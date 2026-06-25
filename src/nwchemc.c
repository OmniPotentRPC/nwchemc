#define _POSIX_C_SOURCE 200112L

#include "nwchemc.h"

#include "nwchemc_params.h"

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef NWCHEMC_HAS_NWCHEM

extern void nwchemc_embed_init(void);
extern int nwchemc_embed_available(void);
extern int nwchemc_embed_set_config(const char *basis, int basis_len,
                                    const char *theory, int theory_len,
                                    const char *scf_type, int scf_len,
                                    const int *charge, const int *mult,
                                    const char *input_blocks,
                                    int input_blocks_len);
extern int nwchemc_embed_set_dft_direct(const char *xc, int xc_len,
                                        int direct_enabled,
                                        int smearing_enabled,
                                        double smear_sigma_hartree,
                                        int smearing_spinset);
extern int nwchemc_embed_set_scf_direct(int has_options, int maxiter,
                                        double thresh, double tol2e);
extern int nwchemc_embed_set_driver_direct(int has_options, int maxiter,
                                           int tolerance_mode,
                                           double gmax_tol, double grms_tol,
                                           double xmax_tol, double xrms_tol);
extern int nwchemc_embed_set_pseudopotentials(const char *elements,
                                              const int *library_types,
                                              const char *library_names,
                                              int count);
extern int nwchemc_embed_energy_grad(const int *n_atoms,
                                     const double *positions_ang,
                                     const int *atomic_numbers,
                                     const int *charge,
                                     const int *multiplicity,
                                     double *energy_h, double *grad_h_bohr,
                                     char *errmsg, int errmsg_len);
extern int nwchemc_embed_energy_grad_cell(
    const int *n_atoms, const double *positions_ang, const int *atomic_numbers,
    const double *cell_ang, const int *has_cell, const int *charge,
    const int *multiplicity, double *energy_h, double *grad_h_bohr,
    char *errmsg, int errmsg_len);
extern int nwchemc_embed_hessian(const int *n_atoms,
                                 const double *positions_ang,
                                 const int *atomic_numbers,
                                 const int *charge,
                                 const int *multiplicity,
                                 double *hessian_h_bohr2, char *errmsg,
                                 int errmsg_len);
extern int nwchemc_embed_hessian_cell(
    const int *n_atoms, const double *positions_ang, const int *atomic_numbers,
    const double *cell_ang, const int *has_cell, const int *charge,
    const int *multiplicity, double *hessian_h_bohr2, char *errmsg,
    int errmsg_len);
extern void nwchemc_embed_finalize(void);

static int g_initialized = 0;
static int g_atexit_registered = 0;

struct NWChemCSession {
  unsigned char *params_bytes;
  size_t params_size;
  struct capn arena;
  NWChemParams_ptr params_root;
  int has_params;
  int configured;
  double *step_positions_ang;
  int *step_atomic_numbers;
  size_t step_atom_capacity;
};

static NWChemCSession *g_active_session = NULL;

static int cstr_len(const char *s) { return s ? (int)strlen(s) : 0; }

static const char *text_or_with_len(capn_text text, const char *fallback,
                                    int *len) {
  if (text.str && text.len > 0) {
    *len = text.len;
    return text.str;
  }
  *len = cstr_len(fallback);
  return fallback;
}

static int span_starts_with(const char *s, int len, const char *prefix) {
  int prefix_len = cstr_len(prefix);
  return s && len >= prefix_len && memcmp(s, prefix, (size_t)prefix_len) == 0;
}

static void copy_text_record(char *dst, size_t dst_size, capn_text text) {
  size_t n = 0;
  memset(dst, 0, dst_size);
  if (!text.str || text.len <= 0 || dst_size == 0)
    return;
  n = (size_t)text.len;
  if (n >= dst_size)
    n = dst_size - 1;
  memcpy(dst, text.str, n);
}

static void finalize_at_exit(void) { nwchemc_finalize(); }

static void ensure_init(void) {
  if (!g_initialized) {
    nwchemc_embed_init();
    g_initialized = 1;
    if (!g_atexit_registered) {
      atexit(finalize_at_exit);
      g_atexit_registered = 1;
    }
  }
}

static const char *selected_theory(const struct NWChemParams *params,
                                   int *theory_len, const char **scf_type,
                                   int *scf_len) {
  const char *theory = text_or_with_len(params->theory, "scf", theory_len);
  *scf_type = text_or_with_len(params->scfType, "rhf", scf_len);
  if (span_starts_with(theory, *theory_len, "blyp") ||
      span_starts_with(theory, *theory_len, "b3lyp") ||
      span_starts_with(theory, *theory_len, "pbe")) {
    *scf_type = theory;
    *scf_len = *theory_len;
    *theory_len = 3;
    return "dft";
  }
  return theory;
}

static void apply_env_hints(const struct NWChemParams *params) {
#if !defined(_WIN32)
  if (params->nwchemRoot.len > 0)
    setenv("NWCHEM_TOP", params->nwchemRoot.str, 1);
  if (params->scratchDir.len > 0)
    setenv("NWCHEM_SCRATCH_DIR", params->scratchDir.str, 1);
  if (params->permanentDir.len > 0)
    setenv("NWCHEM_PERMANENT_DIR", params->permanentDir.str, 1);
#else
  (void)params;
#endif
}

static int apply_config_to_embed(NWChemParams_ptr params_root,
                                 const struct NWChemParams *params) {
  char input_blocks[NWCHEMC_BLOCKS];
  const char *scf_type = NULL;
  int theory_len = 0;
  int scf_len = 0;
  const char *theory = selected_theory(params, &theory_len, &scf_type, &scf_len);
  /* Embed path: strip promoted DFT fields from text; apply via RTDB/direct API. */
  if (nwchemc_params_render_embed_input_blocks(params_root, input_blocks,
                                               sizeof(input_blocks)) != 0)
    return -1;

  capn_text dft_xc = {0};
  int dft_direct = 0;
  int dft_smear_on = 0;
  double dft_smear_sigma = 0.0;
  int dft_smear_spinset = 1;
  if (nwchemc_params_extract_direct_dft(params_root, &dft_xc, &dft_direct,
                                        &dft_smear_on, &dft_smear_sigma,
                                        &dft_smear_spinset) != 0)
    return -1;
  int scf_has_options = 0;
  int scf_maxiter = 0;
  double scf_thresh = 0.0;
  double scf_tol2e = 0.0;
  if (nwchemc_params_extract_direct_scf(params_root, &scf_has_options,
                                        &scf_maxiter, &scf_thresh,
                                        &scf_tol2e) != 0)
    return -1;
  int driver_has_options = 0;
  int driver_maxiter = 0;
  int driver_tolerance_mode = NWCHEMC_DRIVER_TOLERANCE_NONE;
  double driver_gmax_tol = 0.0;
  double driver_grms_tol = 0.0;
  double driver_xmax_tol = 0.0;
  double driver_xrms_tol = 0.0;
  if (nwchemc_params_extract_direct_driver(params_root, &driver_has_options,
                                           &driver_maxiter,
                                           &driver_tolerance_mode,
                                           &driver_gmax_tol, &driver_grms_tol,
                                           &driver_xmax_tol,
                                           &driver_xrms_tol) != 0)
    return -1;
  capn_text psp_elements[64];
  capn_text psp_names[64];
  int psp_types[64];
  size_t psp_count = 0;
  char packed_psp_elements[64 * 16];
  char packed_psp_names[64 * 256];
  if (nwchemc_params_extract_direct_pseudopotentials(
          params_root, psp_elements, psp_types, psp_names, 64, &psp_count) != 0)
    return -1;
  memset(packed_psp_elements, 0, sizeof(packed_psp_elements));
  memset(packed_psp_names, 0, sizeof(packed_psp_names));
  for (size_t i = 0; i < psp_count; ++i) {
    copy_text_record(packed_psp_elements + i * 16, 16, psp_elements[i]);
    copy_text_record(packed_psp_names + i * 256, 256, psp_names[i]);
  }
  if (dft_xc.len > 0 && dft_xc.str) {
    scf_type = dft_xc.str;
    scf_len = (int)dft_xc.len;
    if (!span_starts_with(theory, theory_len, "dft") &&
        !span_starts_with(theory, theory_len, "blyp") &&
        !span_starts_with(theory, theory_len, "b3lyp") &&
        !span_starts_with(theory, theory_len, "pbe")) {
      theory = "dft";
      theory_len = 3;
    }
  }

  apply_env_hints(params);
  ensure_init();
  int ch = params->charge;
  int mult = params->multiplicity > 0 ? params->multiplicity : 1;
  int basis_len = 0;
  const char *basis = text_or_with_len(params->basis, "sto-3g", &basis_len);
  if (nwchemc_embed_set_config(basis, basis_len, theory, theory_len, scf_type,
                               scf_len, &ch, &mult, input_blocks,
                               cstr_len(input_blocks)) != 0)
    return -1;
  if (nwchemc_embed_set_pseudopotentials(
          packed_psp_elements, psp_types, packed_psp_names, (int)psp_count) !=
      0)
    return -1;
  if (nwchemc_embed_set_scf_direct(scf_has_options, scf_maxiter, scf_thresh,
                                   scf_tol2e) != 0)
    return -1;
  if (nwchemc_embed_set_driver_direct(driver_has_options, driver_maxiter,
                                      driver_tolerance_mode, driver_gmax_tol,
                                      driver_grms_tol, driver_xmax_tol,
                                      driver_xrms_tol) != 0)
    return -1;
  return nwchemc_embed_set_dft_direct(
      dft_xc.str ? dft_xc.str : "", dft_xc.str ? (int)dft_xc.len : 0,
      dft_direct, dft_smear_on, dft_smear_sigma, dft_smear_spinset);
}

int nwchemc_set_params(const void *params_capnp,
                       size_t params_capnp_size_bytes) {
  struct capn arena;
  NWChemParams_ptr params_root;
  if (nwchemc_params_root(params_capnp, params_capnp_size_bytes, &arena,
                          &params_root) != 0)
    return -1;
  struct NWChemParams params;
  read_NWChemParams(&params, params_root);
  if (apply_config_to_embed(params_root, &params) != 0) {
    nwchemc_params_release(&arena);
    return -1;
  }
  g_active_session = NULL;
  nwchemc_params_release(&arena);
  return 0;
}

NWChemCResult nwchemc_energy_gradient(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *grad_h_bohr) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';

  if (n_atoms <= 0 || !positions_ang || !atomic_numbers || !grad_h_bohr) {
    snprintf(r.message, sizeof(r.message), "invalid arguments");
    return r;
  }

  struct capn arena;
  NWChemParams_ptr params_root;
  if (nwchemc_params_root(params_capnp, params_capnp_size_bytes, &arena,
                          &params_root) != 0) {
    snprintf(r.message, sizeof(r.message), "invalid NWChemParams message");
    return r;
  }

  struct NWChemParams params;
  read_NWChemParams(&params, params_root);
  if (apply_config_to_embed(params_root, &params) != 0) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "embed config failed");
    return r;
  }
  g_active_session = NULL;

  char errmsg[512];
  memset(errmsg, 0, sizeof(errmsg));
  int n = n_atoms;
  int ch = params.charge;
  int mult = params.multiplicity > 0 ? params.multiplicity : 1;
  double eh = 0.0;
  int rc = nwchemc_embed_energy_grad(&n, positions_ang, atomic_numbers, &ch,
                                     &mult, &eh, grad_h_bohr, errmsg,
                                     (int)sizeof(errmsg) - 1);
  nwchemc_params_release(&arena);
  if (rc != 0) {
    snprintf(r.message, sizeof(r.message), "%s",
             errmsg[0] ? errmsg : "nwchem embed energy/grad failed");
    return r;
  }
  r.ok = 1;
  r.energy_h = eh;
  snprintf(r.message, sizeof(r.message), "ok");
  return r;
}

NWChemCResult nwchemc_energy(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes) {
  /* Energy-only public ABI; uses gradient path internally, discards grad. */
  double *scratch = NULL;
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (n_atoms <= 0 || !positions_ang || !atomic_numbers) {
    snprintf(r.message, sizeof(r.message), "invalid arguments");
    return r;
  }
  scratch = (double *)calloc((size_t)n_atoms * 3u, sizeof(double));
  if (!scratch) {
    snprintf(r.message, sizeof(r.message), "out of memory");
    return r;
  }
  r = nwchemc_energy_gradient(n_atoms, positions_ang, atomic_numbers,
                              params_capnp, params_capnp_size_bytes, scratch);
  free(scratch);
  return r;
}

NWChemCResult nwchemc_energy_forces(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *forces_h_bohr) {
  NWChemCResult r = nwchemc_energy_gradient(
      n_atoms, positions_ang, atomic_numbers, params_capnp,
      params_capnp_size_bytes, forces_h_bohr);
  if (!r.ok || !forces_h_bohr || n_atoms <= 0)
    return r;
  int i;
  for (i = 0; i < n_atoms * 3; ++i)
    forces_h_bohr[i] = -forces_h_bohr[i];
  return r;
}

NWChemCResult nwchemc_hessian(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *hessian_h_bohr2) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';

  if (n_atoms <= 0 || !positions_ang || !atomic_numbers || !hessian_h_bohr2) {
    snprintf(r.message, sizeof(r.message), "invalid arguments");
    return r;
  }

  struct capn arena;
  NWChemParams_ptr params_root;
  if (nwchemc_params_root(params_capnp, params_capnp_size_bytes, &arena,
                          &params_root) != 0) {
    snprintf(r.message, sizeof(r.message), "invalid NWChemParams message");
    return r;
  }

  struct NWChemParams params;
  read_NWChemParams(&params, params_root);
  if (apply_config_to_embed(params_root, &params) != 0) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "embed config failed");
    return r;
  }
  g_active_session = NULL;

  char errmsg[512];
  memset(errmsg, 0, sizeof(errmsg));
  int n = n_atoms;
  int ch = params.charge;
  int mult = params.multiplicity > 0 ? params.multiplicity : 1;
  int rc = nwchemc_embed_hessian(&n, positions_ang, atomic_numbers, &ch, &mult,
                                 hessian_h_bohr2, errmsg,
                                 (int)sizeof(errmsg) - 1);
  nwchemc_params_release(&arena);
  if (rc != 0) {
    snprintf(r.message, sizeof(r.message), "%s",
             errmsg[0] ? errmsg : "nwchem embed hessian failed");
    return r;
  }
  r.ok = 1;
  snprintf(r.message, sizeof(r.message), "ok");
  return r;
}

static void session_clear_params(NWChemCSession *session) {
  if (!session)
    return;
  if (g_active_session == session)
    g_active_session = NULL;
  if (session->has_params)
    nwchemc_params_release(&session->arena);
  free(session->params_bytes);
  session->params_bytes = NULL;
  session->params_size = 0;
  memset(&session->arena, 0, sizeof(session->arena));
  memset(&session->params_root, 0, sizeof(session->params_root));
  session->has_params = 0;
  session->configured = 0;
}

static void session_clear_step_scratch(NWChemCSession *session) {
  if (!session)
    return;
  free(session->step_positions_ang);
  free(session->step_atomic_numbers);
  session->step_positions_ang = NULL;
  session->step_atomic_numbers = NULL;
  session->step_atom_capacity = 0;
}

static int session_reserve_step_atoms(NWChemCSession *session,
                                      size_t n_atoms) {
  if (!session || n_atoms == 0)
    return -1;
  if (n_atoms <= session->step_atom_capacity)
    return 0;
  if (n_atoms > (SIZE_MAX / 3u) / sizeof(double))
    return -1;
  double *positions =
      (double *)malloc(n_atoms * 3u * sizeof(*session->step_positions_ang));
  if (!positions)
    return -1;
  int *atomic_numbers =
      (int *)malloc(n_atoms * sizeof(*session->step_atomic_numbers));
  if (!atomic_numbers) {
    free(positions);
    return -1;
  }
  free(session->step_positions_ang);
  free(session->step_atomic_numbers);
  session->step_positions_ang = positions;
  session->step_atomic_numbers = atomic_numbers;
  session->step_atom_capacity = n_atoms;
  return 0;
}

static int apply_root_to_embed(NWChemParams_ptr params_root) {
  struct NWChemParams params;
  read_NWChemParams(&params, params_root);
  return apply_config_to_embed(params_root, &params);
}

static int apply_message_to_embed(const void *params_capnp,
                                  size_t params_capnp_size_bytes) {
  struct capn arena;
  NWChemParams_ptr params_root;
  if (nwchemc_params_root(params_capnp, params_capnp_size_bytes, &arena,
                          &params_root) != 0)
    return -1;
  int rc = apply_root_to_embed(params_root);
  nwchemc_params_release(&arena);
  return rc;
}

static int session_install_params(NWChemCSession *session,
                                  const void *params_capnp,
                                  size_t params_capnp_size_bytes) {
  if (!session || !params_capnp || params_capnp_size_bytes == 0)
    return -1;

  unsigned char *copy = (unsigned char *)malloc(params_capnp_size_bytes);
  if (!copy)
    return -1;
  memcpy(copy, params_capnp, params_capnp_size_bytes);

  if (apply_message_to_embed(copy, params_capnp_size_bytes) != 0) {
    free(copy);
    return -1;
  }

  session_clear_params(session);
  session->params_bytes = copy;
  session->params_size = params_capnp_size_bytes;
  if (nwchemc_params_root(session->params_bytes, session->params_size,
                          &session->arena, &session->params_root) != 0) {
    session_clear_params(session);
    return -1;
  }
  session->has_params = 1;
  session->configured = 1;
  g_active_session = session;
  return 0;
}

static int session_apply_config(NWChemCSession *session) {
  if (!session || !session->has_params)
    return -1;
  if (session->configured && g_active_session == session)
    return 0;
  if (apply_root_to_embed(session->params_root) != 0) {
    session->configured = 0;
    if (g_active_session == session)
      g_active_session = NULL;
    return -1;
  }
  session->configured = 1;
  g_active_session = session;
  return 0;
}

static void session_charge_multiplicity(NWChemCSession *session, int *charge,
                                        int *multiplicity) {
  struct NWChemParams params;
  read_NWChemParams(&params, session->params_root);
  *charge = params.charge;
  *multiplicity = params.multiplicity > 0 ? params.multiplicity : 1;
}

NWChemCSession *nwchemc_session_create(const void *params_capnp,
                                       size_t params_capnp_size_bytes) {
  if (!params_capnp || params_capnp_size_bytes == 0)
    return NULL;
  NWChemCSession *session =
      (NWChemCSession *)calloc(1, sizeof(NWChemCSession));
  if (!session)
    return NULL;
  if (session_install_params(session, params_capnp, params_capnp_size_bytes) !=
      0) {
    free(session);
    return NULL;
  }
  return session;
}

int nwchemc_session_set_params(NWChemCSession *session,
                               const void *params_capnp,
                               size_t params_capnp_size_bytes) {
  if (!session || !params_capnp || params_capnp_size_bytes == 0)
    return -1;
  return session_install_params(session, params_capnp,
                                params_capnp_size_bytes);
}

void nwchemc_session_destroy(NWChemCSession *session) {
  if (!session)
    return;
  session_clear_params(session);
  session_clear_step_scratch(session);
  free(session);
}

static NWChemCResult session_energy_gradient_cell(NWChemCSession *session,
                                                  int n_atoms,
                                                  const double *positions_ang,
                                                  const int *atomic_numbers,
                                                  const double *cell_ang,
                                                  int has_cell,
                                                  double *grad_h_bohr) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!session || n_atoms <= 0 || !positions_ang || !atomic_numbers ||
      !grad_h_bohr) {
    snprintf(r.message, sizeof(r.message), "invalid arguments");
    return r;
  }
  if (session_apply_config(session) != 0) {
    snprintf(r.message, sizeof(r.message), "embed config failed");
    return r;
  }
  ensure_init();
  char errmsg[512];
  memset(errmsg, 0, sizeof(errmsg));
  int n = n_atoms;
  int ch = 0;
  int mult = 1;
  session_charge_multiplicity(session, &ch, &mult);
  double eh = 0.0;
  int cell_flag = has_cell ? 1 : 0;
  double empty_cell[9] = {0.0, 0.0, 0.0, 0.0, 0.0,
                          0.0, 0.0, 0.0, 0.0};
  const double *cell_arg = cell_flag ? cell_ang : empty_cell;
  int rc = nwchemc_embed_energy_grad_cell(
      &n, positions_ang, atomic_numbers, cell_arg, &cell_flag, &ch, &mult, &eh,
      grad_h_bohr, errmsg, (int)sizeof(errmsg) - 1);
  if (rc != 0) {
    snprintf(r.message, sizeof(r.message), "%s",
             errmsg[0] ? errmsg : "nwchem embed energy/grad failed");
    return r;
  }
  r.ok = 1;
  r.energy_h = eh;
  snprintf(r.message, sizeof(r.message), "ok");
  return r;
}

NWChemCResult nwchemc_session_energy_gradient(NWChemCSession *session,
                                              int n_atoms,
                                              const double *positions_ang,
                                              const int *atomic_numbers,
                                              double *grad_h_bohr) {
  return session_energy_gradient_cell(session, n_atoms, positions_ang,
                                      atomic_numbers, NULL, 0, grad_h_bohr);
}

NWChemCResult nwchemc_session_energy(NWChemCSession *session, int n_atoms,
                                     const double *positions_ang,
                                     const int *atomic_numbers) {
  double *scratch = NULL;
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!session || n_atoms <= 0 || !positions_ang || !atomic_numbers) {
    snprintf(r.message, sizeof(r.message), "invalid arguments");
    return r;
  }
  scratch = (double *)calloc((size_t)n_atoms * 3u, sizeof(double));
  if (!scratch) {
    snprintf(r.message, sizeof(r.message), "out of memory");
    return r;
  }
  r = nwchemc_session_energy_gradient(session, n_atoms, positions_ang,
                                      atomic_numbers, scratch);
  free(scratch);
  return r;
}

NWChemCResult nwchemc_session_energy_forces(NWChemCSession *session,
                                            int n_atoms,
                                            const double *positions_ang,
                                            const int *atomic_numbers,
                                            double *forces_h_bohr) {
  NWChemCResult r =
      nwchemc_session_energy_gradient(session, n_atoms, positions_ang,
                                      atomic_numbers, forces_h_bohr);
  if (r.ok && forces_h_bohr && n_atoms > 0) {
    for (int i = 0; i < n_atoms * 3; ++i)
      forces_h_bohr[i] = -forces_h_bohr[i];
  }
  return r;
}

NWChemCResult nwchemc_session_calculate_forces(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *forces_h_bohr,
    size_t forces_len) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!session || !force_input_capnp || force_input_capnp_size_bytes == 0 ||
      !forces_h_bohr) {
    snprintf(r.message, sizeof(r.message), "invalid arguments");
    return r;
  }

  struct capn arena;
  ForceInput_ptr force_input;
  if (nwchemc_force_input_root(force_input_capnp, force_input_capnp_size_bytes,
                               &arena, &force_input) != 0) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput message");
    return r;
  }

  size_t n_atoms = 0;
  int has_cell = 0;
  if (nwchemc_force_input_atom_count(force_input, &n_atoms, &has_cell) != 0 ||
      n_atoms > (size_t)INT_MAX || forces_len < n_atoms * 3u) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }
  if (session_reserve_step_atoms(session, n_atoms) != 0) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "out of memory");
    return r;
  }

  double cell_ang[9];
  if (nwchemc_force_input_copy_geometry(
          force_input, session->step_positions_ang, session->step_atomic_numbers,
          session->step_atom_capacity, cell_ang, &has_cell) != 0) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }
  nwchemc_params_release(&arena);

  r = session_energy_gradient_cell(session, (int)n_atoms,
                                   session->step_positions_ang,
                                   session->step_atomic_numbers, cell_ang,
                                   has_cell, forces_h_bohr);
  if (r.ok) {
    for (size_t i = 0; i < n_atoms * 3u; ++i)
      forces_h_bohr[i] = -forces_h_bohr[i];
  }
  return r;
}

NWChemCResult nwchemc_session_calculate_result(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!session || !force_input_capnp || force_input_capnp_size_bytes == 0 ||
      !potential_result_capnp_size_bytes) {
    snprintf(r.message, sizeof(r.message), "invalid arguments");
    return r;
  }
  *potential_result_capnp_size_bytes = 0;

  struct capn arena;
  ForceInput_ptr force_input;
  if (nwchemc_force_input_root(force_input_capnp, force_input_capnp_size_bytes,
                               &arena, &force_input) != 0) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput message");
    return r;
  }

  size_t n_atoms = 0;
  int has_cell = 0;
  if (nwchemc_force_input_atom_count(force_input, &n_atoms, &has_cell) != 0 ||
      n_atoms > (size_t)INT_MAX || n_atoms > SIZE_MAX / 3u) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }
  size_t force_count = n_atoms * 3u;
  size_t required_size = nwchemc_potential_result_flat_size(force_count);
  *potential_result_capnp_size_bytes = required_size;
  if (required_size == 0 || force_count > (size_t)INT_MAX) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }

  double energy_factor = 1.0;
  double force_factor = 1.0;
  if (nwchemc_force_input_result_factors(force_input, &energy_factor,
                                         &force_factor) != 0) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput result units");
    return r;
  }
  if (!potential_result_capnp ||
      potential_result_capnp_capacity_bytes < required_size) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "PotentialResult buffer too small");
    return r;
  }
  if (session_reserve_step_atoms(session, n_atoms) != 0) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "out of memory");
    return r;
  }

  double cell_ang[9];
  if (nwchemc_force_input_copy_geometry(
          force_input, session->step_positions_ang, session->step_atomic_numbers,
          session->step_atom_capacity, cell_ang, &has_cell) != 0) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }
  nwchemc_params_release(&arena);

  double *forces = (double *)malloc(force_count * sizeof(*forces));
  if (!forces) {
    snprintf(r.message, sizeof(r.message), "out of memory");
    return r;
  }
  r = session_energy_gradient_cell(session, (int)n_atoms,
                                   session->step_positions_ang,
                                   session->step_atomic_numbers, cell_ang,
                                   has_cell, forces);
  if (r.ok) {
    for (size_t i = 0; i < force_count; ++i)
      forces[i] = -forces[i] * force_factor;
    if (nwchemc_potential_result_write(r.energy_h * energy_factor, forces,
                                       force_count, potential_result_capnp,
                                       potential_result_capnp_capacity_bytes,
                                       potential_result_capnp_size_bytes) != 0) {
      r.ok = 0;
      snprintf(r.message, sizeof(r.message), "PotentialResult write failed");
    }
  }
  free(forces);
  return r;
}

size_t nwchemc_potential_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes) {
  struct capn arena;
  ForceInput_ptr force_input;
  if (nwchemc_force_input_root(force_input_capnp, force_input_capnp_size_bytes,
                               &arena, &force_input) != 0)
    return 0;

  size_t n_atoms = 0;
  int has_cell = 0;
  if (nwchemc_force_input_atom_count(force_input, &n_atoms, &has_cell) != 0 ||
      n_atoms > (size_t)INT_MAX || n_atoms > SIZE_MAX / 3u) {
    nwchemc_params_release(&arena);
    return 0;
  }
  (void)has_cell;
  size_t force_count = n_atoms * 3u;
  if (force_count > (size_t)INT_MAX) {
    nwchemc_params_release(&arena);
    return 0;
  }
  size_t result_size = nwchemc_potential_result_flat_size(force_count);
  nwchemc_params_release(&arena);
  return result_size;
}

static NWChemCResult session_hessian_cell(NWChemCSession *session, int n_atoms,
                                          const double *positions_ang,
                                          const int *atomic_numbers,
                                          const double *cell_ang, int has_cell,
                                          double *hessian_h_bohr2) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!session || n_atoms <= 0 || !positions_ang || !atomic_numbers ||
      !hessian_h_bohr2) {
    snprintf(r.message, sizeof(r.message), "invalid arguments");
    return r;
  }
  if (session_apply_config(session) != 0) {
    snprintf(r.message, sizeof(r.message), "embed config failed");
    return r;
  }
  ensure_init();
  char errmsg[512];
  memset(errmsg, 0, sizeof(errmsg));
  int n = n_atoms;
  int ch = 0;
  int mult = 1;
  session_charge_multiplicity(session, &ch, &mult);
  int cell_flag = has_cell ? 1 : 0;
  double empty_cell[9] = {0.0, 0.0, 0.0, 0.0, 0.0,
                          0.0, 0.0, 0.0, 0.0};
  const double *cell_arg = cell_flag ? cell_ang : empty_cell;
  int rc = nwchemc_embed_hessian_cell(
      &n, positions_ang, atomic_numbers, cell_arg, &cell_flag, &ch, &mult,
      hessian_h_bohr2, errmsg, (int)sizeof(errmsg) - 1);
  if (rc != 0) {
    snprintf(r.message, sizeof(r.message), "%s",
             errmsg[0] ? errmsg : "nwchem embed hessian failed");
    return r;
  }
  r.ok = 1;
  snprintf(r.message, sizeof(r.message), "ok");
  return r;
}

NWChemCResult nwchemc_session_calculate_hessian(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *hessian_h_bohr2,
    size_t hessian_len) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!session || !force_input_capnp || force_input_capnp_size_bytes == 0 ||
      !hessian_h_bohr2) {
    snprintf(r.message, sizeof(r.message), "invalid arguments");
    return r;
  }

  struct capn arena;
  ForceInput_ptr force_input;
  if (nwchemc_force_input_root(force_input_capnp, force_input_capnp_size_bytes,
                               &arena, &force_input) != 0) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput message");
    return r;
  }

  size_t n_atoms = 0;
  int has_cell = 0;
  if (nwchemc_force_input_atom_count(force_input, &n_atoms, &has_cell) != 0 ||
      n_atoms > (size_t)INT_MAX || n_atoms > SIZE_MAX / 3u) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }
  size_t ndof = n_atoms * 3u;
  if (ndof > SIZE_MAX / ndof || hessian_len < ndof * ndof) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }
  if (session_reserve_step_atoms(session, n_atoms) != 0) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "out of memory");
    return r;
  }

  double cell_ang[9];
  if (nwchemc_force_input_copy_geometry(
          force_input, session->step_positions_ang, session->step_atomic_numbers,
          session->step_atom_capacity, cell_ang, &has_cell) != 0) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }
  nwchemc_params_release(&arena);

  return session_hessian_cell(session, (int)n_atoms, session->step_positions_ang,
                              session->step_atomic_numbers, cell_ang, has_cell,
                              hessian_h_bohr2);
}

NWChemCResult nwchemc_session_hessian(NWChemCSession *session, int n_atoms,
                                      const double *positions_ang,
                                      const int *atomic_numbers,
                                      double *hessian_h_bohr2) {
  return session_hessian_cell(session, n_atoms, positions_ang, atomic_numbers,
                              NULL, 0, hessian_h_bohr2);
}

const char *nwchemc_version(void) { return "nwchemc/0.1.0"; }

int nwchemc_available(void) {
  ensure_init();
  return nwchemc_embed_available() ? 1 : 0;
}

void nwchemc_finalize(void) {
  g_active_session = NULL;
  if (!g_initialized)
    return;
  nwchemc_embed_finalize();
  g_initialized = 0;
}

#else

int nwchemc_set_params(const void *params_capnp,
                       size_t params_capnp_size_bytes) {
  (void)params_capnp;
  (void)params_capnp_size_bytes;
  return -1;
}

NWChemCResult nwchemc_energy_gradient(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *grad_h_bohr) {
  (void)n_atoms;
  (void)positions_ang;
  (void)atomic_numbers;
  (void)params_capnp;
  (void)params_capnp_size_bytes;
  (void)grad_h_bohr;
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  snprintf(r.message, sizeof(r.message), "compiled without NWCHEMC_HAS_NWCHEM");
  return r;
}

NWChemCResult nwchemc_energy(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes) {
  (void)n_atoms;
  (void)positions_ang;
  (void)atomic_numbers;
  (void)params_capnp;
  (void)params_capnp_size_bytes;
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  snprintf(r.message, sizeof(r.message), "compiled without NWCHEMC_HAS_NWCHEM");
  return r;
}

NWChemCResult nwchemc_energy_forces(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *forces_h_bohr) {
  (void)n_atoms;
  (void)positions_ang;
  (void)atomic_numbers;
  (void)params_capnp;
  (void)params_capnp_size_bytes;
  (void)forces_h_bohr;
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  snprintf(r.message, sizeof(r.message), "compiled without NWCHEMC_HAS_NWCHEM");
  return r;
}

NWChemCResult nwchemc_hessian(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *hessian_h_bohr2) {
  (void)n_atoms;
  (void)positions_ang;
  (void)atomic_numbers;
  (void)params_capnp;
  (void)params_capnp_size_bytes;
  (void)hessian_h_bohr2;
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  snprintf(r.message, sizeof(r.message), "compiled without NWCHEMC_HAS_NWCHEM");
  return r;
}

const char *nwchemc_version(void) { return "nwchemc/unavailable"; }

int nwchemc_available(void) { return 0; }

void nwchemc_finalize(void) {}

NWChemCSession *nwchemc_session_create(const void *params_capnp,
                                       size_t params_capnp_size_bytes) {
  (void)params_capnp;
  (void)params_capnp_size_bytes;
  return NULL;
}

int nwchemc_session_set_params(NWChemCSession *session,
                               const void *params_capnp,
                               size_t params_capnp_size_bytes) {
  (void)session;
  (void)params_capnp;
  (void)params_capnp_size_bytes;
  return -1;
}

void nwchemc_session_destroy(NWChemCSession *session) { (void)session; }

static NWChemCResult no_nwchem_fail(void) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  snprintf(r.message, sizeof(r.message), "compiled without NWCHEMC_HAS_NWCHEM");
  return r;
}

NWChemCResult nwchemc_session_energy(NWChemCSession *session, int n_atoms,
                                     const double *positions_ang,
                                     const int *atomic_numbers) {
  (void)session;
  (void)n_atoms;
  (void)positions_ang;
  (void)atomic_numbers;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_session_energy_gradient(NWChemCSession *session,
                                              int n_atoms,
                                              const double *positions_ang,
                                              const int *atomic_numbers,
                                              double *grad_h_bohr) {
  (void)session;
  (void)n_atoms;
  (void)positions_ang;
  (void)atomic_numbers;
  (void)grad_h_bohr;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_session_energy_forces(NWChemCSession *session,
                                            int n_atoms,
                                            const double *positions_ang,
                                            const int *atomic_numbers,
                                            double *forces_h_bohr) {
  (void)session;
  (void)n_atoms;
  (void)positions_ang;
  (void)atomic_numbers;
  (void)forces_h_bohr;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_session_calculate_forces(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *forces_h_bohr,
    size_t forces_len) {
  (void)session;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)forces_h_bohr;
  (void)forces_len;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_session_calculate_result(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) {
  (void)session;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)potential_result_capnp;
  (void)potential_result_capnp_capacity_bytes;
  (void)potential_result_capnp_size_bytes;
  return no_nwchem_fail();
}

size_t nwchemc_potential_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes) {
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  return 0;
}

NWChemCResult nwchemc_session_calculate_hessian(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *hessian_h_bohr2,
    size_t hessian_len) {
  (void)session;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)hessian_h_bohr2;
  (void)hessian_len;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_session_hessian(NWChemCSession *session, int n_atoms,
                                      const double *positions_ang,
                                      const int *atomic_numbers,
                                      double *hessian_h_bohr2) {
  (void)session;
  (void)n_atoms;
  (void)positions_ang;
  (void)atomic_numbers;
  (void)hessian_h_bohr2;
  return no_nwchem_fail();
}

#endif
