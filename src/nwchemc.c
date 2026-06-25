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
                                   const char **scf_type) {
  const char *theory = nwchemc_params_text_or(params->theory, "scf");
  *scf_type = nwchemc_params_text_or(params->scfType, "rhf");
  if (strncmp(theory, "blyp", 4) == 0 || strncmp(theory, "b3lyp", 5) == 0 ||
      strncmp(theory, "pbe", 3) == 0) {
    *scf_type = theory;
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
  const char *theory = selected_theory(params, &scf_type);
  if (nwchemc_params_render_input_blocks(params_root, input_blocks,
                                         sizeof(input_blocks)) != 0)
    return -1;
  apply_env_hints(params);
  ensure_init();
  int ch = params->charge;
  int mult = params->multiplicity > 0 ? params->multiplicity : 1;
  const char *basis = nwchemc_params_text_or(params->basis, "sto-3g");
  return nwchemc_embed_set_config(
      basis, cstr_len(basis), theory, cstr_len(theory), scf_type,
      cstr_len(scf_type), &ch, &mult, input_blocks, cstr_len(input_blocks));
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

#endif
