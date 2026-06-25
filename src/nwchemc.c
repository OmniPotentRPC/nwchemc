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

static int g_initialized = 0;
static NWChemCParams g_params;

static int cstr_len(const char *s) { return s ? (int)strlen(s) : 0; }

static void ensure_init(void) {
  if (!g_initialized) {
    nwchemc_embed_init();
    nwchemc_params_default(&g_params);
    g_initialized = 1;
  }
}

static void normalize_params(NWChemCParams *params) {
  if (strncmp(params->theory, "blyp", 4) == 0 ||
      strncmp(params->theory, "b3lyp", 5) == 0 ||
      strncmp(params->theory, "pbe", 3) == 0) {
    snprintf(params->scf_type, sizeof(params->scf_type), "%s", params->theory);
    snprintf(params->theory, sizeof(params->theory), "%s", "dft");
  }
}

static void apply_env_hints(const NWChemCParams *params) {
#if !defined(_WIN32)
  if (params->nwchem_root[0])
    setenv("NWCHEM_TOP", params->nwchem_root, 1);
  if (params->scratch_dir[0])
    setenv("NWCHEM_SCRATCH_DIR", params->scratch_dir, 1);
  if (params->permanent_dir[0])
    setenv("NWCHEM_PERMANENT_DIR", params->permanent_dir, 1);
#else
  (void)params;
#endif
}

static int apply_config_to_embed(const NWChemCParams *params) {
  NWChemCParams p = *params;
  normalize_params(&p);
  apply_env_hints(&p);
  ensure_init();
  int ch = p.charge;
  int mult = p.multiplicity > 0 ? p.multiplicity : 1;
  return nwchemc_embed_set_config(p.basis, cstr_len(p.basis), p.theory,
                                  cstr_len(p.theory), p.scf_type,
                                  cstr_len(p.scf_type), &ch, &mult,
                                  p.input_blocks, cstr_len(p.input_blocks));
}

int nwchemc_set_params(const void *params_capnp,
                       size_t params_capnp_size_bytes) {
  NWChemCParams parsed;
  if (nwchemc_params_parse_flat(params_capnp, params_capnp_size_bytes,
                                &parsed) != 0)
    return -1;
  g_params = parsed;
  return apply_config_to_embed(&g_params);
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

  NWChemCParams parsed;
  if (nwchemc_params_parse_flat(params_capnp, params_capnp_size_bytes,
                                &parsed) != 0) {
    snprintf(r.message, sizeof(r.message), "invalid NWChemParams message");
    return r;
  }
  g_params = parsed;

  if (apply_config_to_embed(&g_params) != 0) {
    snprintf(r.message, sizeof(r.message), "embed config failed");
    return r;
  }

  char errmsg[512];
  memset(errmsg, 0, sizeof(errmsg));
  int n = n_atoms;
  int ch = g_params.charge;
  int mult = g_params.multiplicity > 0 ? g_params.multiplicity : 1;
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

const char *nwchemc_version(void) { return "nwchemc/0.1.0"; }

int nwchemc_available(void) {
  ensure_init();
  return nwchemc_embed_available() ? 1 : 0;
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

const char *nwchemc_version(void) { return "nwchemc/unavailable"; }

int nwchemc_available(void) { return 0; }

#endif
