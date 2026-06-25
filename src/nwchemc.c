#define _POSIX_C_SOURCE 200112L

#include "nwchemc.h"

#include "nwchemc_params.h"

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
extern int nwchemc_embed_energy_grad(const int *n_atoms,
                                     const double *positions_ang,
                                     const int *atomic_numbers,
                                     const int *charge,
                                     const int *multiplicity,
                                     double *energy_h, double *grad_h_bohr,
                                     char *errmsg, int errmsg_len);
extern int nwchemc_embed_hessian(const int *n_atoms,
                                 const double *positions_ang,
                                 const int *atomic_numbers,
                                 const int *charge,
                                 const int *multiplicity,
                                 double *hessian_h_bohr2, char *errmsg,
                                 int errmsg_len);
extern void nwchemc_embed_finalize(void);

static int g_initialized = 0;
static int g_atexit_registered = 0;

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

/* Persistent session: config applied once; geometry varies per call. */
struct NWChemCSession {
  void *params_copy;
  size_t params_size;
  int charge;
  int multiplicity;
  int config_applied;
};

static int session_apply_config(NWChemCSession *session) {
  if (!session || !session->params_copy || session->params_size == 0)
    return -1;
  if (session->config_applied)
    return 0;
  struct capn arena;
  NWChemParams_ptr params_root;
  if (nwchemc_params_root(session->params_copy, session->params_size, &arena,
                          &params_root) != 0)
    return -1;
  struct NWChemParams params;
  read_NWChemParams(&params, params_root);
  if (apply_config_to_embed(params_root, &params) != 0) {
    nwchemc_params_release(&arena);
    return -1;
  }
  session->charge = params.charge;
  session->multiplicity = params.multiplicity > 0 ? params.multiplicity : 1;
  nwchemc_params_release(&arena);
  session->config_applied = 1;
  return 0;
}

NWChemCSession *nwchemc_session_create(const void *params_capnp,
                                       size_t params_capnp_size_bytes) {
  if (!params_capnp || params_capnp_size_bytes == 0)
    return NULL;
  NWChemCSession *session =
      (NWChemCSession *)calloc(1, sizeof(NWChemCSession));
  if (!session)
    return NULL;
  session->params_copy = malloc(params_capnp_size_bytes);
  if (!session->params_copy) {
    free(session);
    return NULL;
  }
  memcpy(session->params_copy, params_capnp, params_capnp_size_bytes);
  session->params_size = params_capnp_size_bytes;
  session->multiplicity = 1;
  if (session_apply_config(session) != 0) {
    free(session->params_copy);
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
  void *copy = malloc(params_capnp_size_bytes);
  if (!copy)
    return -1;
  memcpy(copy, params_capnp, params_capnp_size_bytes);
  free(session->params_copy);
  session->params_copy = copy;
  session->params_size = params_capnp_size_bytes;
  session->config_applied = 0;
  return session_apply_config(session);
}

void nwchemc_session_destroy(NWChemCSession *session) {
  if (!session)
    return;
  free(session->params_copy);
  free(session);
}

NWChemCResult nwchemc_session_energy_gradient(NWChemCSession *session,
                                              int n_atoms,
                                              const double *positions_ang,
                                              const int *atomic_numbers,
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
  int ch = session->charge;
  int mult = session->multiplicity;
  double eh = 0.0;
  int rc = nwchemc_embed_energy_grad(&n, positions_ang, atomic_numbers, &ch,
                                     &mult, &eh, grad_h_bohr, errmsg,
                                     (int)sizeof(errmsg) - 1);
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

NWChemCResult nwchemc_session_hessian(NWChemCSession *session, int n_atoms,
                                      const double *positions_ang,
                                      const int *atomic_numbers,
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
  int ch = session->charge;
  int mult = session->multiplicity;
  int rc = nwchemc_embed_hessian(&n, positions_ang, atomic_numbers, &ch, &mult,
                                 hessian_h_bohr2, errmsg,
                                 (int)sizeof(errmsg) - 1);
  if (rc != 0) {
    snprintf(r.message, sizeof(r.message), "%s",
             errmsg[0] ? errmsg : "nwchem embed hessian failed");
    return r;
  }
  r.ok = 1;
  snprintf(r.message, sizeof(r.message), "ok");
  return r;
}

const char *nwchemc_version(void) { return "nwchemc/0.1.0"; }

int nwchemc_available(void) {
  ensure_init();
  return nwchemc_embed_available() ? 1 : 0;
}

void nwchemc_finalize(void) {
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
