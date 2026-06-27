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
extern int nwchemc_embed_set_nwpw_direct(int has_options,
                                         double energy_cutoff,
                                         double wavefunction_cutoff,
                                         double ewald_rcut, int ewald_ncut);
extern int nwchemc_embed_set_brillouin_zone(
    int has_options, const char *zone_name, int zone_name_len,
    int monkhorst_pack_x, int monkhorst_pack_y, int monkhorst_pack_z,
    int max_kpoints_print, const double *kvectors, int kvector_count);
extern int nwchemc_embed_set_pseudopotentials(const char *elements,
                                              const int *library_types,
                                              const char *library_names,
                                              int count);
extern int nwchemc_embed_set_rtdb_strings(const char *keys,
                                          const char *values, int count);
extern int nwchemc_embed_set_rtdb_values(const char *keys,
                                         const int *value_types,
                                         const int *value_counts,
                                         const char *values, int count);
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
extern int nwchemc_embed_dipole(const int *n_atoms,
                                const double *positions_ang,
                                const int *atomic_numbers, const int *charge,
                                const int *multiplicity, double *energy_h,
                                double *dipole_au, char *errmsg,
                                int errmsg_len);
extern int nwchemc_embed_dipole_cell(
    const int *n_atoms, const double *positions_ang, const int *atomic_numbers,
    const double *cell_ang, const int *has_cell, const int *charge,
    const int *multiplicity, double *energy_h, double *dipole_au, char *errmsg,
    int errmsg_len);
extern int nwchemc_embed_quadrupole(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const int *charge, const int *multiplicity,
    double *energy_h, double *quadrupole_au, char *errmsg, int errmsg_len);
extern int nwchemc_embed_quadrupole_cell(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const double *cell_ang, const int *has_cell,
    const int *charge, const int *multiplicity, double *energy_h,
    double *quadrupole_au, char *errmsg, int errmsg_len);
extern int nwchemc_embed_optimize(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const int *charge, const int *multiplicity,
    double *energy_h, double *optimized_positions_ang, char *errmsg,
    int errmsg_len);
extern int nwchemc_embed_optimize_cell(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const double *cell_ang, const int *has_cell,
    const int *charge, const int *multiplicity, double *energy_h,
    double *optimized_positions_ang, char *errmsg, int errmsg_len);
extern int nwchemc_embed_frequencies(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const int *charge, const int *multiplicity,
    double *frequencies_cm1, double *intensities_au, char *errmsg,
    int errmsg_len);
extern int nwchemc_embed_frequencies_cell(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const double *cell_ang, const int *has_cell,
    const int *charge, const int *multiplicity, double *frequencies_cm1,
    double *intensities_au, char *errmsg, int errmsg_len);
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
  /* Session-owned scalar state decoded from installed params. */
  int charge;
  int multiplicity;
  double *step_positions_ang;
  int *step_atomic_numbers;
  size_t step_atom_capacity;
  int *topology_atomic_numbers;
  size_t topology_atom_count;
};

static NWChemCSession *g_active_session = NULL;

enum {
  NWCHEMC_DIRECT_PSP_MAX = 64,
  NWCHEMC_DIRECT_PSP_ELEMENT_LEN = 16,
  NWCHEMC_DIRECT_PSP_NAME_LEN = 256,
  NWCHEMC_DIRECT_SET_MAX = 192,
  NWCHEMC_DIRECT_SET_VALUE_MAX = 64,
  NWCHEMC_DIRECT_SET_KEY_LEN = 128,
  NWCHEMC_DIRECT_SET_VALUE_LEN = 256,
};

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

static int ascii_lower(int c) {
  return c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c;
}

static int capn_text_iequals(capn_text text, const char *value) {
  int value_len = cstr_len(value);
  if (!text.str || text.len != value_len)
    return 0;
  for (int i = 0; i < value_len; ++i) {
    if (ascii_lower((unsigned char)text.str[i]) !=
        ascii_lower((unsigned char)value[i]))
      return 0;
  }
  return 1;
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

struct direct_pseudopotential_pack {
  char *elements;
  int *library_types;
  char *library_names;
  size_t capacity;
  size_t count;
};

static int pack_direct_pseudopotential_entry(
    void *user_data, capn_text target,
    const struct NWChemPseudopotentialEntry *entry) {
  struct direct_pseudopotential_pack *pack = user_data;
  if (!pack || !entry || pack->count >= pack->capacity)
    return -1;
  copy_text_record(pack->elements +
                       pack->count * NWCHEMC_DIRECT_PSP_ELEMENT_LEN,
                   NWCHEMC_DIRECT_PSP_ELEMENT_LEN, target);
  pack->library_types[pack->count] = entry->libraryType;
  copy_text_record(pack->library_names +
                       pack->count * NWCHEMC_DIRECT_PSP_NAME_LEN,
                   NWCHEMC_DIRECT_PSP_NAME_LEN, entry->libraryName);
  ++pack->count;
  return 0;
}

static capn_text text_from_cstr(const char *s) {
  capn_text text;
  text.len = s ? (int)strlen(s) : 0;
  text.str = s ? s : "";
  text.seg = NULL;
  return text;
}

static int direct_struct_list_len(capn_ptr *ptr) {
  capn_resolve(ptr);
  if (ptr->type == CAPN_NULL)
    return 0;
  if (ptr->type != CAPN_LIST)
    return -1;
  return ptr->len;
}

static int append_direct_typed_values(
    capn_text *keys, int *value_types, int *value_counts, capn_text *values,
    size_t key_capacity, size_t value_capacity, size_t *count,
    char key_storage[][NWCHEMC_DIRECT_SET_KEY_LEN],
    char value_storage[][NWCHEMC_DIRECT_SET_VALUE_MAX]
                      [NWCHEMC_DIRECT_SET_VALUE_LEN],
    const char *key, int value_type, const char *const *value_list,
    int nvalues) {
  if (!keys || !value_types || !value_counts || !values || !count || !key ||
      !value_list || nvalues <= 0 || *count >= key_capacity ||
      value_capacity == 0 || (size_t)nvalues > value_capacity ||
      nvalues > NWCHEMC_DIRECT_SET_VALUE_MAX)
    return -1;
  int nkey = snprintf(key_storage[*count], NWCHEMC_DIRECT_SET_KEY_LEN, "%s",
                      key);
  if (nkey < 0 || (size_t)nkey >= NWCHEMC_DIRECT_SET_KEY_LEN)
    return -1;
  keys[*count] = text_from_cstr(key_storage[*count]);
  value_types[*count] = value_type;
  value_counts[*count] = nvalues;
  for (int i = 0; i < nvalues; ++i) {
    if (!value_list[i])
      return -1;
    int nvalue = snprintf(value_storage[*count][i],
                          NWCHEMC_DIRECT_SET_VALUE_LEN, "%s", value_list[i]);
    if (nvalue < 0 || (size_t)nvalue >= NWCHEMC_DIRECT_SET_VALUE_LEN)
      return -1;
    values[*count * value_capacity + (size_t)i] =
        text_from_cstr(value_storage[*count][i]);
  }
  ++*count;
  return 0;
}

static int append_direct_typed_value(
    capn_text *keys, int *value_types, int *value_counts, capn_text *values,
    size_t key_capacity, size_t value_capacity, size_t *count,
    char key_storage[][NWCHEMC_DIRECT_SET_KEY_LEN],
    char value_storage[][NWCHEMC_DIRECT_SET_VALUE_MAX]
                      [NWCHEMC_DIRECT_SET_VALUE_LEN],
    const char *key, int value_type, const char *value) {
  const char *value_list[1] = {value};
  return append_direct_typed_values(keys, value_types, value_counts, values,
                                    key_capacity, value_capacity, count,
                                    key_storage, value_storage, key,
                                    value_type, value_list, 1);
}

static int append_nwpw_direct_typed_values(
    capn_text *keys, int *value_types, int *value_counts, capn_text *values,
    size_t key_capacity, size_t value_capacity, size_t *count,
    char key_storage[][NWCHEMC_DIRECT_SET_KEY_LEN],
    char value_storage[][NWCHEMC_DIRECT_SET_VALUE_MAX]
                      [NWCHEMC_DIRECT_SET_VALUE_LEN],
    const char *suffix,
    int value_type, const char *value) {
  static const char *prefixes[] = {"cgsd", "band", "cpsd", "cpmd"};
  char key[NWCHEMC_DIRECT_SET_KEY_LEN];
  for (size_t i = 0; i < sizeof(prefixes) / sizeof(prefixes[0]); ++i) {
    int n = snprintf(key, sizeof(key), "%s:%s", prefixes[i], suffix);
    if (n < 0 || (size_t)n >= sizeof(key))
      return -1;
    if (append_direct_typed_value(keys, value_types, value_counts, values,
                                  key_capacity, value_capacity, count,
                                  key_storage, value_storage, key, value_type,
                                  value) != 0)
      return -1;
  }
  return 0;
}

static int append_nwpw_prefixed_typed_values(
    capn_text *keys, int *value_types, int *value_counts, capn_text *values,
    size_t key_capacity, size_t value_capacity, size_t *count,
    char key_storage[][NWCHEMC_DIRECT_SET_KEY_LEN],
    char value_storage[][NWCHEMC_DIRECT_SET_VALUE_MAX]
                      [NWCHEMC_DIRECT_SET_VALUE_LEN],
    const char *const *prefixes, size_t prefix_count, const char *suffix,
    int value_type, const char *const *value_list, int nvalues) {
  char key[NWCHEMC_DIRECT_SET_KEY_LEN];
  for (size_t i = 0; i < prefix_count; ++i) {
    int n = snprintf(key, sizeof(key), "%s:%s", prefixes[i], suffix);
    if (n < 0 || (size_t)n >= sizeof(key))
      return -1;
    if (append_direct_typed_values(keys, value_types, value_counts, values,
                                   key_capacity, value_capacity, count,
                                   key_storage, value_storage, key,
                                   value_type, value_list, nvalues) != 0)
      return -1;
  }
  return 0;
}

static int append_simulation_cell_direct_value(
    capn_text *keys, int *value_types, int *value_counts, capn_text *values,
    size_t key_capacity, size_t value_capacity, size_t *count,
    char key_storage[][NWCHEMC_DIRECT_SET_KEY_LEN],
    char value_storage[][NWCHEMC_DIRECT_SET_VALUE_MAX]
                      [NWCHEMC_DIRECT_SET_VALUE_LEN],
    capn_text cell_name, const char *suffix, int value_type,
    const char *const *value_list, int nvalues) {
  char key[NWCHEMC_DIRECT_SET_KEY_LEN];
  int n = 0;
  if (cell_name.str && cell_name.len > 0) {
    n = snprintf(key, sizeof(key), "%.*s:%s", cell_name.len, cell_name.str,
                 suffix);
  } else {
    n = snprintf(key, sizeof(key), "cell_default:%s", suffix);
  }
  if (n < 0 || (size_t)n >= sizeof(key))
    return -1;
  return append_direct_typed_values(keys, value_types, value_counts, values,
                                    key_capacity, value_capacity, count,
                                    key_storage, value_storage, key,
                                    value_type, value_list, nvalues);
}

static int append_simulation_cell_direct_scalar(
    capn_text *keys, int *value_types, int *value_counts, capn_text *values,
    size_t key_capacity, size_t value_capacity, size_t *count,
    char key_storage[][NWCHEMC_DIRECT_SET_KEY_LEN],
    char value_storage[][NWCHEMC_DIRECT_SET_VALUE_MAX]
                      [NWCHEMC_DIRECT_SET_VALUE_LEN],
    capn_text cell_name, const char *suffix, int value_type,
    const char *value) {
  const char *value_list[1] = {value};
  return append_simulation_cell_direct_value(
      keys, value_types, value_counts, values, key_capacity, value_capacity,
      count, key_storage, value_storage, cell_name, suffix, value_type,
      value_list, 1);
}

static int tce_reference_value(enum NWChemTceReference reference, int *value) {
  if (!value)
    return 0;
  switch (reference) {
  case NWChemTceReference_dft:
    *value = 0;
    return 1;
  case NWChemTceReference_hf:
  case NWChemTceReference_scf:
    *value = 1;
    return 1;
  case NWChemTceReference_unspecified:
  default:
    return 0;
  }
}

static const char *
tce_two_electron_value(enum NWChemTceTwoElectronStorage storage) {
  switch (storage) {
  case NWChemTceTwoElectronStorage_default:
    return "default";
  case NWChemTceTwoElectronStorage_orbital:
    return "2eorb";
  case NWChemTceTwoElectronStorage_spin:
    return "2espin";
  case NWChemTceTwoElectronStorage_unspecified:
  default:
    return NULL;
  }
}

static int tce_io_value(enum NWChemTceIoAlgorithm io, int *value) {
  if (!value)
    return 0;
  switch (io) {
  case NWChemTceIoAlgorithm_fortran:
    *value = 0;
    return 1;
  case NWChemTceIoAlgorithm_eaf:
    *value = 1;
    return 1;
  case NWChemTceIoAlgorithm_ga:
    *value = 2;
    return 1;
  case NWChemTceIoAlgorithm_sf:
    *value = 3;
    return 1;
  case NWChemTceIoAlgorithm_replicated:
    *value = 4;
    return 1;
  case NWChemTceIoAlgorithm_dra:
    *value = 5;
    return 1;
  case NWChemTceIoAlgorithm_gaEaf:
    *value = 6;
    return 1;
  case NWChemTceIoAlgorithm_unspecified:
  default:
    return 0;
  }
}

/** Parser-equivalent RTDB defaults for a structured TCE method token. */
struct tce_method_defaults {
  /** User-facing NWChem method token from NWChemTceStanza.method. */
  const char *method;
  /** Normalized value for tce:model. */
  const char *model;
  /** Optional normalized value for tce:perturbative. */
  const char *perturbative;
  /** Optional normalized value for tce:ccsdvar. */
  const char *ccsd_variant;
  /** Optional normalized logical value for tce:nts. */
  int no_triples_singles;
  /** Optional normalized logical value for tce:left. */
  int left;
  /** Optional normalized integer value for tce:mrcc; negative means unset. */
  int mrcc;
};

static const struct tce_method_defaults *tce_method_defaults_for(
    capn_text method) {
  static const struct tce_method_defaults defaults[] = {
      {"ccd", "ccd", NULL, NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"lccd", "lccd", NULL, NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"ccsd", "ccsd", NULL, NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"multi", "multi", NULL, NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"eionly", "eionly", NULL, NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"ccsd_act", "ccsd_act", NULL, NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"lccsd", "lccsd", NULL, NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"lccsd(t)", "lccsd", "(t)", NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"cr-lccsd(t)", "lccsd", "cr_(t)", NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"crsd(t)ac", "ccsd_act", "cr_(t)a", NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"ccsdta", "ccsdta", NULL, NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"ccsdt", "ccsdt", NULL, NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"ccsdtq", "ccsdtq", NULL, NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"cc2", "ccsd", NULL, "cc2", NWChemToggle_disabled,
       NWChemToggle_unspecified, -1},
      {"lr-ccsd", "ccsd", NULL, "lr-ccsd", NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"ccsd(t)", "ccsd", "(t)", NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"ccsd[t]", "ccsd", "[t]", NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"qcisd(t)", "qcisd", "(t)", NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"cr-qcisd(t)", "qcisd", "cr_(t)", NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"lambda-ccsd(t)", "ccsd", "lambda(t)", NULL,
       NWChemToggle_unspecified, NWChemToggle_enabled, -1},
      {"cr-ccsd(t)", "ccsd", "cr_(t)", NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"lr-ccsd(t)", "ccsd", "lr_(t)", NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"creomsd(t)", "ccsd", "creom_(t)", NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"creom(t)ac", "ccsd_act", "creom(t)a", NULL,
       NWChemToggle_unspecified, NWChemToggle_unspecified, -1},
      {"r-creom1(t)", "ccsd", "emb1", NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"r-creom2(t)", "ccsd", "emb2", NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"lr-ccsd(tq)-1", "ccsd", "lr_(tq1)", NULL,
       NWChemToggle_unspecified, NWChemToggle_unspecified, -1},
      {"lr-ccsd(tq)-1p", "ccsd", "lr_(tq1p)", NULL,
       NWChemToggle_unspecified, NWChemToggle_unspecified, -1},
      {"cr-ccsd[t]", "ccsd", "cr_[t]", NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"ccsd(2)_t", "ccsd", "2_t", NULL, NWChemToggle_unspecified,
       NWChemToggle_enabled, -1},
      {"ccsd(2)", "ccsd", "2_tq", NULL, NWChemToggle_unspecified,
       NWChemToggle_enabled, -1},
      {"ccsdt(2)_q", "ccsdt", "2_q", NULL, NWChemToggle_unspecified,
       NWChemToggle_enabled, -1},
      {"qcisd", "qcisd", NULL, NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"cis", "cis", NULL, NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"cisd", "cisd", NULL, NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"cisdt", "cisdt", NULL, NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"cisdtq", "cisdtq", NULL, NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"mbpt2", "mbpt2", NULL, NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"mbpt3", "mbpt3", NULL, NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"mbpt4", "mbpt4", NULL, NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"mbpt4(sdq)", "mbpt4sdq", NULL, NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"mbpt4sdq(t)", "mbpt4sdq_t", NULL, NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"mp2", "mbpt2", NULL, NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"mp3", "mbpt3", NULL, NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"mp4sdq", "mbpt4sdq", NULL, NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"mp4sdq(t)", "mbpt4sdq_t", NULL, NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"mp4", "mbpt4", NULL, NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, -1},
      {"bwccsd", "bwccsd", NULL, NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, 1},
      {"mkccsd", "mkccsd", NULL, NULL, NWChemToggle_unspecified,
       NWChemToggle_unspecified, 1},
  };

  if (!method.str || method.len <= 0)
    return NULL;
  for (size_t i = 0; i < sizeof(defaults) / sizeof(defaults[0]); ++i) {
    if (capn_text_iequals(method, defaults[i].method))
      return &defaults[i];
  }
  return NULL;
}

static const char *toggle_logical_value(enum NWChemToggle toggle) {
  switch (toggle) {
  case NWChemToggle_enabled:
    return "true";
  case NWChemToggle_disabled:
    return "false";
  case NWChemToggle_unspecified:
  default:
    return NULL;
  }
}

struct direct_pspspin_rule_append_state {
  capn_text *keys;
  int *value_types;
  int *value_counts;
  capn_text *values;
  size_t key_capacity;
  size_t value_capacity;
  size_t *count;
  char (*key_storage)[NWCHEMC_DIRECT_SET_KEY_LEN];
  char (*value_storage)[NWCHEMC_DIRECT_SET_VALUE_MAX]
                       [NWCHEMC_DIRECT_SET_VALUE_LEN];
};

static int pspspin_index_name(size_t index, char *dst, size_t dst_size) {
  if (index == 0 || index > 999999)
    return -1;
  int n = snprintf(dst, dst_size, "_%06zu", index);
  return n >= 0 && (size_t)n < dst_size ? 0 : -1;
}

static int pspspin_angular_momentum_value(
    enum NWChemPseudopotentialSpinRule_AngularMomentum angular_momentum,
    int *value) {
  if (!value)
    return -1;
  switch (angular_momentum) {
  case NWChemPseudopotentialSpinRule_AngularMomentum_p:
    *value = 1;
    return 0;
  case NWChemPseudopotentialSpinRule_AngularMomentum_d:
    *value = 2;
    return 0;
  case NWChemPseudopotentialSpinRule_AngularMomentum_f:
    *value = 3;
    return 0;
  case NWChemPseudopotentialSpinRule_AngularMomentum_s:
  default:
    *value = 0;
    return 0;
  }
}

static int append_direct_pspspin_rule(
    void *user_data, size_t rule_index,
    const struct NWChemPseudopotentialSpinRule *rule) {
  struct direct_pspspin_rule_append_state *state = user_data;
  if (!state || !rule)
    return -1;

  capn_list32 ion_indices = rule->ionIndices;
  capn_resolve(&ion_indices.p);
  if (ion_indices.p.type == CAPN_NULL ||
      ion_indices.p.type != CAPN_LIST || ion_indices.p.datasz != 4)
    return -1;
  int nions = ion_indices.p.len;
  if (nions <= 0 || nions > NWCHEMC_DIRECT_SET_VALUE_MAX)
    return -1;

  char index_name[8];
  if (pspspin_index_name(rule_index, index_name, sizeof(index_name)) != 0)
    return -1;

  int iam_up = rule->channel != NWChemPseudopotentialSpinRule_Channel_down;
  const char *channel = iam_up ? "up" : "down";
  int angular_momentum = 0;
  if (pspspin_angular_momentum_value(rule->angularMomentum,
                                     &angular_momentum) != 0)
    return -1;

  char key[NWCHEMC_DIRECT_SET_KEY_LEN];
  int n = snprintf(key, sizeof(key), "nwpw:pspspin_iamup:%s", index_name);
  if (n < 0 || (size_t)n >= sizeof(key))
    return -1;
  if (append_direct_typed_value(
          state->keys, state->value_types, state->value_counts, state->values,
          state->key_capacity, state->value_capacity, state->count,
          state->key_storage, state->value_storage, key,
          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, iam_up ? "true" : "false") != 0)
    return -1;

  char scale_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
  snprintf(scale_value, sizeof(scale_value), "%.15g", rule->scale);
  n = snprintf(key, sizeof(key), "nwpw:pspspin_%sscale:%s", channel,
               index_name);
  if (n < 0 || (size_t)n >= sizeof(key))
    return -1;
  if (append_direct_typed_value(
          state->keys, state->value_types, state->value_counts, state->values,
          state->key_capacity, state->value_capacity, state->count,
          state->key_storage, state->value_storage, key,
          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, scale_value) != 0)
    return -1;

  char angular_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
  snprintf(angular_value, sizeof(angular_value), "%d", angular_momentum);
  n = snprintf(key, sizeof(key), "nwpw:pspspin_%sl:%s", channel, index_name);
  if (n < 0 || (size_t)n >= sizeof(key))
    return -1;
  if (append_direct_typed_value(
          state->keys, state->value_types, state->value_counts, state->values,
          state->key_capacity, state->value_capacity, state->count,
          state->key_storage, state->value_storage, key,
          NWCHEMC_DIRECT_SET_VALUE_INTEGER, angular_value) != 0)
    return -1;

  if (rule->hasMagneticQuantumNumber) {
    char magnetic_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    snprintf(magnetic_value, sizeof(magnetic_value), "%d",
             rule->magneticQuantumNumber);
    n = snprintf(key, sizeof(key), "nwpw:pspspin_%sm:%s", channel,
                 index_name);
    if (n < 0 || (size_t)n >= sizeof(key))
      return -1;
    if (append_direct_typed_value(
            state->keys, state->value_types, state->value_counts,
            state->values, state->key_capacity, state->value_capacity,
            state->count, state->key_storage, state->value_storage, key,
            NWCHEMC_DIRECT_SET_VALUE_INTEGER, magnetic_value) != 0)
      return -1;
  }

  char ion_values[NWCHEMC_DIRECT_SET_VALUE_MAX][NWCHEMC_DIRECT_SET_VALUE_LEN];
  const char *ion_value_list[NWCHEMC_DIRECT_SET_VALUE_MAX];
  for (int i = 0; i < nions; ++i) {
    int ion_index = (int)(int32_t)capn_get32(ion_indices, i);
    snprintf(ion_values[i], sizeof(ion_values[i]), "%d", ion_index);
    ion_value_list[i] = ion_values[i];
  }
  n = snprintf(key, sizeof(key), "nwpw:pspspin_%sions:%s", channel,
               index_name);
  if (n < 0 || (size_t)n >= sizeof(key))
    return -1;
  return append_direct_typed_values(
      state->keys, state->value_types, state->value_counts, state->values,
      state->key_capacity, state->value_capacity, state->count,
      state->key_storage, state->value_storage, key,
      NWCHEMC_DIRECT_SET_VALUE_INTEGER, ion_value_list, nions);
}

static int append_simulation_cell_direct_values(
    NWChemParams_ptr params, capn_text *keys, int *value_types,
    int *value_counts, capn_text *values, size_t key_capacity,
    size_t value_capacity, size_t *count,
    char key_storage[][NWCHEMC_DIRECT_SET_KEY_LEN],
    char value_storage[][NWCHEMC_DIRECT_SET_VALUE_MAX]
                      [NWCHEMC_DIRECT_SET_VALUE_LEN]) {
  if (params.p.type == CAPN_NULL || !keys || !value_types || !value_counts ||
      !values || !count)
    return -1;

  struct NWChemParams view;
  read_NWChemParams(&view, params);
  int n = direct_struct_list_len(&view.inputStanzas.p);
  if (n < 0)
    return -1;

  for (int i = 0; i < n; ++i) {
    struct NWChemInputStanza stanza;
    get_NWChemInputStanza(&stanza, view.inputStanzas, i);
    if (stanza.kind != NWChemInputStanza_Kind_simulationCell ||
        stanza.simulationCell.p.type == CAPN_NULL)
      continue;

    struct NWChemSimulationCellStanza cell;
    read_NWChemSimulationCellStanza(&cell, stanza.simulationCell);
    capn_list64 lattice_vectors = cell.latticeVectorsBohr;
    capn_resolve(&lattice_vectors.p);
    int nvector = 0;
    if (lattice_vectors.p.type != CAPN_NULL) {
      if (lattice_vectors.p.type != CAPN_LIST ||
          lattice_vectors.p.datasz != 8)
        return -1;
      nvector = lattice_vectors.p.len;
    }
    if (nvector != 0 && nvector != 9)
      return -1;
    if (cell.boundaryConditions.len > 0) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      copy_text_record(value, sizeof(value), cell.boundaryConditions);
      if (append_simulation_cell_direct_scalar(
              keys, value_types, value_counts, values, key_capacity,
              value_capacity, count, key_storage, value_storage, cell.cellName,
              "boundry", NWCHEMC_DIRECT_SET_VALUE_TEXT, value) != 0)
        return -1;
    }
    if (nvector == 9) {
      char value_text[9][NWCHEMC_DIRECT_SET_VALUE_LEN];
      const char *value_list[9];
      for (int j = 0; j < 9; ++j) {
        snprintf(value_text[j], sizeof(value_text[j]), "%.15g",
                 capn_to_f64(capn_get64(lattice_vectors, j)));
        value_list[j] = value_text[j];
      }
      if (append_simulation_cell_direct_value(
              keys, value_types, value_counts, values, key_capacity,
              value_capacity, count, key_storage, value_storage, cell.cellName,
              "unita", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, value_list, 9) != 0)
        return -1;
    }
    if (cell.ngridX > 0 && cell.ngridY > 0 && cell.ngridZ > 0) {
      char nx[NWCHEMC_DIRECT_SET_VALUE_LEN];
      char ny[NWCHEMC_DIRECT_SET_VALUE_LEN];
      char nz[NWCHEMC_DIRECT_SET_VALUE_LEN];
      const char *value_list[3] = {nx, ny, nz};
      snprintf(nx, sizeof(nx), "%d", cell.ngridX);
      snprintf(ny, sizeof(ny), "%d", cell.ngridY);
      snprintf(nz, sizeof(nz), "%d", cell.ngridZ);
      if (append_simulation_cell_direct_value(
              keys, value_types, value_counts, values, key_capacity,
              value_capacity, count, key_storage, value_storage, cell.cellName,
              "ngrid", NWCHEMC_DIRECT_SET_VALUE_INTEGER, value_list, 3) != 0)
        return -1;
    }
    if (cell.ngridSmallX > 0 && cell.ngridSmallY > 0 &&
        cell.ngridSmallZ > 0) {
      char nx[NWCHEMC_DIRECT_SET_VALUE_LEN];
      char ny[NWCHEMC_DIRECT_SET_VALUE_LEN];
      char nz[NWCHEMC_DIRECT_SET_VALUE_LEN];
      const char *value_list[3] = {nx, ny, nz};
      snprintf(nx, sizeof(nx), "%d", cell.ngridSmallX);
      snprintf(ny, sizeof(ny), "%d", cell.ngridSmallY);
      snprintf(nz, sizeof(nz), "%d", cell.ngridSmallZ);
      if (append_simulation_cell_direct_value(
              keys, value_types, value_counts, values, key_capacity,
              value_capacity, count, key_storage, value_storage, cell.cellName,
              "ngrid_small", NWCHEMC_DIRECT_SET_VALUE_INTEGER, value_list,
              3) != 0)
        return -1;
    }
    if (cell.boxDeltaBohr > 0.0) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(value, sizeof(value), "%.15g", cell.boxDeltaBohr);
      if (append_simulation_cell_direct_scalar(
              keys, value_types, value_counts, values, key_capacity,
              value_capacity, count, key_storage, value_storage, cell.cellName,
              "box_delta", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, value) != 0)
        return -1;
    }
    if (cell.boxOrient &&
        append_simulation_cell_direct_scalar(
            keys, value_types, value_counts, values, key_capacity,
            value_capacity, count, key_storage, value_storage, cell.cellName,
            "box_orient", NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true") != 0)
      return -1;
    if (cell.boxDifferentLengths &&
        append_simulation_cell_direct_scalar(
            keys, value_types, value_counts, values, key_capacity,
            value_capacity, count, key_storage, value_storage, cell.cellName,
            "box_type", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "1") != 0)
      return -1;
  }
  return 0;
}

static int append_tce_direct_values(
    NWChemParams_ptr params, capn_text *keys, int *value_types,
    int *value_counts, capn_text *values, size_t key_capacity,
    size_t value_capacity, size_t *count,
    char key_storage[][NWCHEMC_DIRECT_SET_KEY_LEN],
    char value_storage[][NWCHEMC_DIRECT_SET_VALUE_MAX]
                      [NWCHEMC_DIRECT_SET_VALUE_LEN]) {
  if (params.p.type == CAPN_NULL || !keys || !value_types || !value_counts ||
      !values || !count)
    return -1;

  struct NWChemParams view;
  read_NWChemParams(&view, params);
  int n = direct_struct_list_len(&view.inputStanzas.p);
  if (n < 0)
    return -1;

  for (int i = 0; i < n; ++i) {
    struct NWChemInputStanza stanza;
    get_NWChemInputStanza(&stanza, view.inputStanzas, i);
    if (stanza.kind != NWChemInputStanza_Kind_tce ||
        stanza.tce.p.type == CAPN_NULL)
      continue;

    struct NWChemTceStanza tce;
    read_NWChemTceStanza(&tce, stanza.tce);
    const struct tce_method_defaults *method_defaults =
        tce_method_defaults_for(tce.method);
    if (tce.model.len > 0) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      copy_text_record(value, sizeof(value), tce.model);
      if (append_direct_typed_value(
              keys, value_types, value_counts, values, key_capacity,
              value_capacity, count, key_storage, value_storage, "tce:model",
              NWCHEMC_DIRECT_SET_VALUE_TEXT, value) != 0)
        return -1;
    } else if (method_defaults && method_defaults->model) {
      if (append_direct_typed_value(
              keys, value_types, value_counts, values, key_capacity,
              value_capacity, count, key_storage, value_storage, "tce:model",
              NWCHEMC_DIRECT_SET_VALUE_TEXT, method_defaults->model) != 0)
        return -1;
    }
    const char *model2e = tce_two_electron_value(tce.model2e);
    if (model2e &&
        append_direct_typed_value(keys, value_types, value_counts, values,
                                  key_capacity, value_capacity, count,
                                  key_storage, value_storage, "tce:model2e",
                                  NWCHEMC_DIRECT_SET_VALUE_TEXT, model2e) != 0)
      return -1;
    if (tce.perturbative.len > 0) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      copy_text_record(value, sizeof(value), tce.perturbative);
      if (append_direct_typed_value(
              keys, value_types, value_counts, values, key_capacity,
              value_capacity, count, key_storage, value_storage,
              "tce:perturbative", NWCHEMC_DIRECT_SET_VALUE_TEXT, value) != 0)
        return -1;
    } else if (method_defaults && method_defaults->perturbative) {
      if (append_direct_typed_value(keys, value_types, value_counts, values,
                                    key_capacity, value_capacity, count,
                                    key_storage, value_storage,
                                    "tce:perturbative",
                                    NWCHEMC_DIRECT_SET_VALUE_TEXT,
                                    method_defaults->perturbative) != 0)
        return -1;
    }
    if (tce.ccsdVariant.len > 0) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      copy_text_record(value, sizeof(value), tce.ccsdVariant);
      if (append_direct_typed_value(
              keys, value_types, value_counts, values, key_capacity,
              value_capacity, count, key_storage, value_storage, "tce:ccsdvar",
              NWCHEMC_DIRECT_SET_VALUE_TEXT, value) != 0)
        return -1;
    } else if (method_defaults && method_defaults->ccsd_variant) {
      if (append_direct_typed_value(keys, value_types, value_counts, values,
                                    key_capacity, value_capacity, count,
                                    key_storage, value_storage, "tce:ccsdvar",
                                    NWCHEMC_DIRECT_SET_VALUE_TEXT,
                                    method_defaults->ccsd_variant) != 0)
        return -1;
    }
    const char *nts_value = toggle_logical_value(tce.noTriplesSingles);
    if (!nts_value && method_defaults)
      nts_value = toggle_logical_value(method_defaults->no_triples_singles);
    if (nts_value &&
        append_direct_typed_value(keys, value_types, value_counts, values,
                                  key_capacity, value_capacity, count,
                                  key_storage, value_storage, "tce:nts",
                                  NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                                  nts_value) != 0)
      return -1;
    int int_value = 0;
    if (tce_reference_value(tce.reference, &int_value)) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(value, sizeof(value), "%d", int_value);
      if (append_direct_typed_value(
              keys, value_types, value_counts, values, key_capacity,
              value_capacity, count, key_storage, value_storage,
              "tce:reference", NWCHEMC_DIRECT_SET_VALUE_INTEGER, value) != 0)
        return -1;
    }
    if (tce.frozenCore > 0) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(value, sizeof(value), "%d", tce.frozenCore);
      if (append_direct_typed_value(
              keys, value_types, value_counts, values, key_capacity,
              value_capacity, count, key_storage, value_storage,
              "tce:frozen core", NWCHEMC_DIRECT_SET_VALUE_INTEGER, value) != 0)
        return -1;
    }
    if (tce.frozenVirtual > 0) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(value, sizeof(value), "%d", tce.frozenVirtual);
      if (append_direct_typed_value(keys, value_types, value_counts, values,
                                    key_capacity, value_capacity, count,
                                    key_storage, value_storage,
                                    "tce:frozen virtual",
                                    NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                                    value) != 0)
        return -1;
    }
    if (tce.thresh > 0.0) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(value, sizeof(value), "%.15g", tce.thresh);
      if (append_direct_typed_value(
              keys, value_types, value_counts, values, key_capacity,
              value_capacity, count, key_storage, value_storage, "tce:thresh",
              NWCHEMC_DIRECT_SET_VALUE_DOUBLE, value) != 0)
        return -1;
    }
    if (tce.levelShift > 0.0) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(value, sizeof(value), "%.15g", tce.levelShift);
      if (append_direct_typed_value(keys, value_types, value_counts, values,
                                    key_capacity, value_capacity, count,
                                    key_storage, value_storage, "tce:zlshift",
                                    NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                                    value) != 0)
        return -1;
    }
    if (tce.leftLevelShift > 0.0) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(value, sizeof(value), "%.15g", tce.leftLevelShift);
      if (append_direct_typed_value(keys, value_types, value_counts, values,
                                    key_capacity, value_capacity, count,
                                    key_storage, value_storage, "tce:zlshiftl",
                                    NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                                    value) != 0)
        return -1;
    }
    if (tce.levelShift2Alpha > 0.0 || tce.levelShift2Beta > 0.0) {
      char alpha_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      char beta_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      const char *value_list[2] = {alpha_value, beta_value};
      snprintf(alpha_value, sizeof(alpha_value), "%.15g",
               tce.levelShift2Alpha);
      snprintf(beta_value, sizeof(beta_value), "%.15g",
               tce.levelShift2Beta);
      if (append_direct_typed_values(
              keys, value_types, value_counts, values, key_capacity,
              value_capacity, count, key_storage, value_storage,
              "tce:zlshift2", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, value_list,
              2) != 0)
        return -1;
    }
    if (tce.levelShift3Alpha > 0.0 || tce.levelShift3Beta > 0.0) {
      char alpha_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      char beta_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      const char *value_list[2] = {alpha_value, beta_value};
      snprintf(alpha_value, sizeof(alpha_value), "%.15g",
               tce.levelShift3Alpha);
      snprintf(beta_value, sizeof(beta_value), "%.15g",
               tce.levelShift3Beta);
      if (append_direct_typed_values(
              keys, value_types, value_counts, values, key_capacity,
              value_capacity, count, key_storage, value_storage,
              "tce:zlshift3", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, value_list,
              2) != 0)
        return -1;
    }
    if (tce.maxiter > 0) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(value, sizeof(value), "%d", tce.maxiter);
      if (append_direct_typed_value(
              keys, value_types, value_counts, values, key_capacity,
              value_capacity, count, key_storage, value_storage, "tce:maxiter",
              NWCHEMC_DIRECT_SET_VALUE_INTEGER, value) != 0)
        return -1;
    }
    if (tce_io_value(tce.ioAlgorithm, &int_value)) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(value, sizeof(value), "%d", int_value);
      if (append_direct_typed_value(
              keys, value_types, value_counts, values, key_capacity,
              value_capacity, count, key_storage, value_storage, "tce:ioalg",
              NWCHEMC_DIRECT_SET_VALUE_INTEGER, value) != 0)
        return -1;
    }
    int effective_multipole = tce.multipole;
    if (tce.octupole && effective_multipole < 3)
      effective_multipole = 3;
    if (tce.quadrupole && effective_multipole < 2)
      effective_multipole = 2;
    if (tce.dipole && effective_multipole < 1)
      effective_multipole = 1;
    const struct {
      const char *key;
      int value;
    } int_fields[] = {
        {"tce:diis", tce.diis},
        {"tce:diis2", tce.diis2},
        {"tce:diis3", tce.diis3},
        {"tce:eoms", tce.eomSolver},
        {"tce:hbard", tce.hbarDimension},
        {"tce:nroots", tce.nroots},
        {"tce:target", tce.target},
        {"tce:multipole", effective_multipole},
        {"tce:active_oa", tce.activeOccupiedAlpha},
        {"tce:active_ob", tce.activeOccupiedBeta},
        {"tce:active_va", tce.activeVirtualAlpha},
        {"tce:active_vb", tce.activeVirtualBeta},
        {"tce:oact", tce.activeOccupied},
        {"tce:uact", tce.activeUnoccupied},
        {"tce:act_excit_lvl", tce.activeExcitationLevel},
        {"tce:maxs", tce.atomicTileSize},
        {"tce:ichopx", tce.split},
        {"tce:i4im", tce.twoElectronMethod},
        {"tce:tilesize", tce.tileSize},
        {"tce:cuda", tce.cudaDevices},
    };
    for (size_t j = 0; j < sizeof(int_fields) / sizeof(int_fields[0]); ++j) {
      if (int_fields[j].value <= 0)
        continue;
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(value, sizeof(value), "%d", int_fields[j].value);
      if (append_direct_typed_value(keys, value_types, value_counts, values,
                                    key_capacity, value_capacity, count,
                                    key_storage, value_storage,
                                    int_fields[j].key,
                                    NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                                    value) != 0)
        return -1;
    }
    if (tce.fragment >= 0) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(value, sizeof(value), "%d", tce.fragment);
      if (append_direct_typed_value(
              keys, value_types, value_counts, values, key_capacity,
              value_capacity, count, key_storage, value_storage,
              "tce:fragment", NWCHEMC_DIRECT_SET_VALUE_INTEGER, value) != 0)
        return -1;
    }
    if (tce.diskBackend >= 0) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(value, sizeof(value), "%d", tce.diskBackend);
      if (append_direct_typed_value(
              keys, value_types, value_counts, values, key_capacity,
              value_capacity, count, key_storage, value_storage, "tce:idiskx",
              NWCHEMC_DIRECT_SET_VALUE_INTEGER, value) != 0)
        return -1;
    }
    if (tce.targetSymmetry.len > 0) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      copy_text_record(value, sizeof(value), tce.targetSymmetry);
      if (append_direct_typed_value(keys, value_types, value_counts, values,
                                    key_capacity, value_capacity, count,
                                    key_storage, value_storage,
                                    "tce:targetsym",
                                    NWCHEMC_DIRECT_SET_VALUE_TEXT,
                                    value) != 0)
        return -1;
    }
    if (tce.densityMatrixFile.len > 0) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      copy_text_record(value, sizeof(value), tce.densityMatrixFile);
      if (append_direct_typed_value(keys, value_types, value_counts, values,
                                    key_capacity, value_capacity, count,
                                    key_storage, value_storage,
                                    "tce:file_densmat",
                                    NWCHEMC_DIRECT_SET_VALUE_TEXT,
                                    value) != 0)
        return -1;
    }
    int effective_left = tce.left;
    if (tce.dipole || tce.quadrupole || tce.octupole)
      effective_left = NWChemToggle_enabled;
    if (effective_left == NWChemToggle_unspecified && method_defaults &&
        method_defaults->left != NWChemToggle_unspecified)
      effective_left = method_defaults->left;
    const struct {
      const char *key;
      enum NWChemToggle toggle;
      int value_type;
    } toggle_fields[] = {
        {"tce:symmetry", tce.symmetry, NWCHEMC_DIRECT_SET_VALUE_LOGICAL},
        {"tce:densmat", tce.densityMatrix, NWCHEMC_DIRECT_SET_VALUE_LOGICAL},
        {"tce:left", effective_left, NWCHEMC_DIRECT_SET_VALUE_LOGICAL},
        {"tce:recompf", tce.recomputeFock, NWCHEMC_DIRECT_SET_VALUE_LOGICAL},
        {"tce:ltcc", tce.tccSpaces, NWCHEMC_DIRECT_SET_VALUE_LOGICAL},
        {"tce:eaccsd", tce.eaCcsd, NWCHEMC_DIRECT_SET_VALUE_LOGICAL},
        {"tce:ipccsd", tce.ipCcsd, NWCHEMC_DIRECT_SET_VALUE_LOGICAL},
    };
    for (size_t j = 0; j < sizeof(toggle_fields) / sizeof(toggle_fields[0]);
         ++j) {
      const char *value = toggle_logical_value(toggle_fields[j].toggle);
      if (value &&
          append_direct_typed_value(keys, value_types, value_counts, values,
                                    key_capacity, value_capacity, count,
                                    key_storage, value_storage,
                                    toggle_fields[j].key,
                                    toggle_fields[j].value_type, value) != 0)
        return -1;
    }
    int mrcc_value = -1;
    if (tce.mrcc != NWChemToggle_unspecified)
      mrcc_value = tce.mrcc == NWChemToggle_enabled ? 1 : 0;
    else if (method_defaults)
      mrcc_value = method_defaults->mrcc;
    if (mrcc_value >= 0) {
      const char *value = mrcc_value ? "1" : "0";
      if (append_direct_typed_value(keys, value_types, value_counts, values,
                                    key_capacity, value_capacity, count,
                                    key_storage, value_storage, "tce:mrcc",
                                    NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                                    value) != 0)
        return -1;
    }
    const struct {
      const char *key;
      double value;
    } double_fields[] = {
        {"tce:eactmin", tce.activeEnergyMin},
        {"tce:eactmax", tce.activeEnergyMax},
        {"tce:maxdiff", tce.maxDiff},
    };
    for (size_t j = 0; j < sizeof(double_fields) / sizeof(double_fields[0]);
         ++j) {
      if (double_fields[j].value <= 0.0)
        continue;
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(value, sizeof(value), "%.15g", double_fields[j].value);
      if (append_direct_typed_value(keys, value_types, value_counts, values,
                                    key_capacity, value_capacity, count,
                                    key_storage, value_storage,
                                    double_fields[j].key,
                                    NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                                    value) != 0)
        return -1;
    }
  }

  return 0;
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
  int ccsd_has_options = 0;
  int ccsd_maxiter = 0;
  double ccsd_thresh = 0.0;
  double ccsd_tol2e = 0.0;
  int ccsd_iprt = 0;
  int ccsd_max_diis = 0;
  int ccsd_frozen_core = 0;
  int ccsd_frozen_virtual = 0;
  int ccsd_use_disk = NWChemToggle_unspecified;
  double ccsd_same_spin_scale = 0.0;
  double ccsd_opposite_spin_scale = 0.0;
  int ccsd_use_trpdrv_nb = NWChemToggle_unspecified;
  int ccsd_use_ccsd_omp = NWChemToggle_unspecified;
  int ccsd_use_trpdrv_omp = NWChemToggle_unspecified;
  int ccsd_use_trpdrv_offload = NWChemToggle_unspecified;
  if (nwchemc_params_extract_direct_ccsd(
          params_root, &ccsd_has_options, &ccsd_maxiter, &ccsd_thresh,
          &ccsd_tol2e, &ccsd_iprt, &ccsd_max_diis, &ccsd_frozen_core,
          &ccsd_frozen_virtual, &ccsd_use_disk, &ccsd_same_spin_scale,
          &ccsd_opposite_spin_scale, &ccsd_use_trpdrv_nb, &ccsd_use_ccsd_omp,
          &ccsd_use_trpdrv_omp, &ccsd_use_trpdrv_offload) != 0)
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
  int nwpw_has_options = 0;
  double nwpw_energy_cutoff = 0.0;
  double nwpw_wavefunction_cutoff = 0.0;
  double nwpw_ewald_rcut = 0.0;
  int nwpw_ewald_ncut = 0;
  if (nwchemc_params_extract_direct_nwpw(
          params_root, &nwpw_has_options, &nwpw_energy_cutoff,
          &nwpw_wavefunction_cutoff, &nwpw_ewald_rcut,
          &nwpw_ewald_ncut) != 0)
    return -1;
  int nwpw_state_has_options = 0;
  capn_text nwpw_cell_name = {0};
  capn_text nwpw_input_wavefunction_filename = {0};
  capn_text nwpw_output_wavefunction_filename = {0};
  double nwpw_fake_mass = 0.0;
  double nwpw_time_step = 0.0;
  int nwpw_loop_start = 0;
  int nwpw_loop_end = 0;
  int nwpw_has_tolerances = 0;
  double nwpw_tolerance_energy = 0.0;
  double nwpw_tolerance_density = 0.0;
  double nwpw_tolerance_gradient = 0.0;
  if (nwchemc_params_extract_direct_nwpw_state(
          params_root, &nwpw_state_has_options, &nwpw_cell_name,
          &nwpw_input_wavefunction_filename,
          &nwpw_output_wavefunction_filename, &nwpw_fake_mass,
          &nwpw_time_step, &nwpw_loop_start, &nwpw_loop_end,
          &nwpw_has_tolerances, &nwpw_tolerance_energy,
          &nwpw_tolerance_density, &nwpw_tolerance_gradient) != 0)
    return -1;
  int nwpw_has_xc = 0;
  capn_text nwpw_exchange_correlation = {0};
  if (nwchemc_params_extract_direct_nwpw_xc(
          params_root, &nwpw_has_xc, &nwpw_exchange_correlation) != 0)
    return -1;
  int nwpw_bo_has_options = 0;
  int nwpw_balance_mode = NWChemNwpwBalanceMode_unspecified;
  int nwpw_bo_step_start = 0;
  int nwpw_bo_step_end = 0;
  double nwpw_bo_time_step = 0.0;
  int nwpw_bo_algorithm = NWChemNwpwBoAlgorithm_unspecified;
  double nwpw_bo_fake_mass = 0.0;
  int nwpw_has_scaling = 0;
  double nwpw_scaling_first = 0.0;
  double nwpw_scaling_second = 0.0;
  if (nwchemc_params_extract_direct_nwpw_bo(
          params_root, &nwpw_bo_has_options, &nwpw_balance_mode,
          &nwpw_bo_step_start, &nwpw_bo_step_end, &nwpw_bo_time_step,
          &nwpw_bo_algorithm, &nwpw_bo_fake_mass, &nwpw_has_scaling,
          &nwpw_scaling_first, &nwpw_scaling_second) != 0)
    return -1;
  int nwpw_execution_has_options = 0;
  int nwpw_np_fft = 0;
  int nwpw_np_orbital = 0;
  int nwpw_np_kspace = 0;
  int nwpw_spin_orbit = NWChemNwpwToggle_unspecified;
  int nwpw_parallel_io = NWChemNwpwToggle_unspecified;
  if (nwchemc_params_extract_direct_nwpw_execution(
          params_root, &nwpw_execution_has_options, &nwpw_np_fft,
          &nwpw_np_orbital, &nwpw_np_kspace, &nwpw_spin_orbit,
          &nwpw_parallel_io) != 0)
    return -1;
  int nwpw_filenames_has_options = 0;
  capn_text nwpw_xyz_filename = {0};
  capn_text nwpw_ion_motion_filename = {0};
  capn_text nwpw_electron_motion_filename = {0};
  capn_text nwpw_hamiltonian_motion_filename = {0};
  capn_text nwpw_orbital_motion_filename = {0};
  capn_text nwpw_eigenvalue_motion_filename = {0};
  if (nwchemc_params_extract_direct_nwpw_filenames(
          params_root, &nwpw_filenames_has_options, &nwpw_xyz_filename,
          &nwpw_ion_motion_filename, &nwpw_electron_motion_filename,
          &nwpw_hamiltonian_motion_filename, &nwpw_orbital_motion_filename,
          &nwpw_eigenvalue_motion_filename) != 0)
    return -1;
  int nwpw_has_fractional = 0;
  int nwpw_fractional_orbitals_start = 0;
  int nwpw_fractional_orbitals_end = 0;
  int nwpw_has_smear = 0;
  double nwpw_smear_temperature = 0.0;
  double nwpw_smear_alpha = 0.0;
  int nwpw_smear_type = NWChemNwpwSmearType_unspecified;
  if (nwchemc_params_extract_direct_nwpw_fractional(
          params_root, &nwpw_has_fractional,
          &nwpw_fractional_orbitals_start, &nwpw_fractional_orbitals_end,
          &nwpw_has_smear, &nwpw_smear_temperature, &nwpw_smear_alpha,
          &nwpw_smear_type) != 0)
    return -1;
  int nwpw_orbital_grid_has_options = 0;
  int nwpw_virtual_orbitals_start = 0;
  int nwpw_virtual_orbitals_end = 0;
  int nwpw_lcao_mode = NWChemNwpwLcaoMode_unspecified;
  int nwpw_ewald_grid_x = 0;
  int nwpw_ewald_grid_y = 0;
  int nwpw_ewald_grid_z = 0;
  if (nwchemc_params_extract_direct_nwpw_orbital_grid(
          params_root, &nwpw_orbital_grid_has_options,
          &nwpw_virtual_orbitals_start, &nwpw_virtual_orbitals_end,
          &nwpw_lcao_mode, &nwpw_ewald_grid_x, &nwpw_ewald_grid_y,
          &nwpw_ewald_grid_z) != 0)
    return -1;
  int nwpw_nose_has_options = 0;
  int nwpw_nose_hoover = NWChemNwpwToggle_unspecified;
  int nwpw_nose_restart = NWChemNwpwToggle_unspecified;
  double nwpw_nose_electron_period = 0.0;
  double nwpw_nose_electron_temperature = 0.0;
  double nwpw_nose_ion_period = 0.0;
  double nwpw_nose_ion_temperature = 0.0;
  int nwpw_nose_electron_chain_length = 0;
  int nwpw_nose_ion_chain_length = 0;
  if (nwchemc_params_extract_direct_nwpw_nose(
          params_root, &nwpw_nose_has_options, &nwpw_nose_hoover,
          &nwpw_nose_restart, &nwpw_nose_electron_period,
          &nwpw_nose_electron_temperature, &nwpw_nose_ion_period,
          &nwpw_nose_ion_temperature, &nwpw_nose_electron_chain_length,
          &nwpw_nose_ion_chain_length) != 0)
    return -1;
  int brillouin_has_options = 0;
  capn_text brillouin_zone_name = {0};
  int brillouin_monkhorst_pack[3] = {0, 0, 0};
  int brillouin_max_kpoints_print = 0;
  size_t brillouin_kvector_count = 0;
  if (nwchemc_params_extract_direct_brillouin_zone(
          params_root, &brillouin_has_options, &brillouin_zone_name,
          brillouin_monkhorst_pack, &brillouin_max_kpoints_print, NULL, 0,
          &brillouin_kvector_count) != 0)
    return -1;
  int psp_types[NWCHEMC_DIRECT_PSP_MAX];
  size_t psp_count = 0;
  int psp_spin_has_options = 0;
  int psp_spin_enabled = 0;
  int psp_spin_count = 0;
  char packed_psp_elements[NWCHEMC_DIRECT_PSP_MAX *
                           NWCHEMC_DIRECT_PSP_ELEMENT_LEN];
  char packed_psp_names[NWCHEMC_DIRECT_PSP_MAX *
                        NWCHEMC_DIRECT_PSP_NAME_LEN];
  capn_text set_keys[NWCHEMC_DIRECT_SET_MAX];
  capn_text set_values[NWCHEMC_DIRECT_SET_MAX];
  size_t set_count = 0;
  capn_text typed_set_keys[NWCHEMC_DIRECT_SET_MAX];
  int typed_set_types[NWCHEMC_DIRECT_SET_MAX];
  int typed_set_value_counts[NWCHEMC_DIRECT_SET_MAX];
  capn_text typed_set_values[NWCHEMC_DIRECT_SET_MAX *
                             NWCHEMC_DIRECT_SET_VALUE_MAX];
  size_t typed_set_count = 0;
  char nwpw_direct_keys[NWCHEMC_DIRECT_SET_MAX][NWCHEMC_DIRECT_SET_KEY_LEN];
  static char nwpw_direct_values[NWCHEMC_DIRECT_SET_MAX]
                                [NWCHEMC_DIRECT_SET_VALUE_MAX]
                                [NWCHEMC_DIRECT_SET_VALUE_LEN];
  char packed_set_keys[NWCHEMC_DIRECT_SET_MAX * NWCHEMC_DIRECT_SET_KEY_LEN];
  char packed_set_values[NWCHEMC_DIRECT_SET_MAX *
                         NWCHEMC_DIRECT_SET_VALUE_LEN];
  char packed_typed_set_keys[NWCHEMC_DIRECT_SET_MAX *
                             NWCHEMC_DIRECT_SET_KEY_LEN];
  static char packed_typed_set_values[NWCHEMC_DIRECT_SET_MAX *
                                      NWCHEMC_DIRECT_SET_VALUE_MAX *
                                      NWCHEMC_DIRECT_SET_VALUE_LEN];
  memset(packed_psp_elements, 0, sizeof(packed_psp_elements));
  memset(packed_psp_names, 0, sizeof(packed_psp_names));
  memset(psp_types, 0, sizeof(psp_types));
  struct direct_pseudopotential_pack psp_pack = {
      .elements = packed_psp_elements,
      .library_types = psp_types,
      .library_names = packed_psp_names,
      .capacity = NWCHEMC_DIRECT_PSP_MAX,
      .count = 0,
  };
  if (nwchemc_params_for_each_direct_pseudopotential(
          params_root, pack_direct_pseudopotential_entry, &psp_pack,
          &psp_count) != 0 ||
      psp_count != psp_pack.count)
    return -1;
  if (nwchemc_params_extract_direct_pseudopotential_spin(
          params_root, &psp_spin_has_options, &psp_spin_enabled,
          &psp_spin_count) != 0)
    return -1;
  if (nwchemc_params_extract_direct_set_strings(
          params_root, set_keys, set_values, NWCHEMC_DIRECT_SET_MAX,
          &set_count) != 0)
    return -1;
  if (nwchemc_params_extract_direct_set_values(
          params_root, typed_set_keys, typed_set_types, typed_set_value_counts,
          typed_set_values, NWCHEMC_DIRECT_SET_MAX, NWCHEMC_DIRECT_SET_VALUE_MAX,
          &typed_set_count) != 0)
    return -1;
  static const char *nwpw_cgsd_band_cpsd[] = {"cgsd", "band", "cpsd"};
  static const char *nwpw_cgsd_band[] = {"cgsd", "band"};
  if (ccsd_maxiter > 0) {
    char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    snprintf(value, sizeof(value), "%d", ccsd_maxiter);
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "ccsd:maxiter",
            NWCHEMC_DIRECT_SET_VALUE_INTEGER, value) != 0)
      return -1;
  }
  if (ccsd_thresh > 0.0) {
    char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    snprintf(value, sizeof(value), "%.15g", ccsd_thresh);
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "ccsd:thresh",
            NWCHEMC_DIRECT_SET_VALUE_DOUBLE, value) != 0)
      return -1;
  }
  if (ccsd_tol2e > 0.0) {
    char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    snprintf(value, sizeof(value), "%.15g", ccsd_tol2e);
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "ccsd:tol2e",
            NWCHEMC_DIRECT_SET_VALUE_DOUBLE, value) != 0)
      return -1;
  }
  if (ccsd_iprt > 0) {
    char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    snprintf(value, sizeof(value), "%d", ccsd_iprt);
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "ccsd:iprt",
            NWCHEMC_DIRECT_SET_VALUE_INTEGER, value) != 0)
      return -1;
  }
  if (ccsd_max_diis > 0) {
    char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    snprintf(value, sizeof(value), "%d", ccsd_max_diis);
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "ccsd:maxdiis",
            NWCHEMC_DIRECT_SET_VALUE_INTEGER, value) != 0)
      return -1;
  }
  if (ccsd_frozen_core > 0) {
    char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    snprintf(value, sizeof(value), "%d", ccsd_frozen_core);
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "ccsd:frozen core",
            NWCHEMC_DIRECT_SET_VALUE_INTEGER, value) != 0)
      return -1;
  }
  if (ccsd_frozen_virtual > 0) {
    char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    snprintf(value, sizeof(value), "%d", ccsd_frozen_virtual);
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "ccsd:frozen virtual",
            NWCHEMC_DIRECT_SET_VALUE_INTEGER, value) != 0)
      return -1;
  }
  if (ccsd_use_disk != NWChemToggle_unspecified) {
    const char *value =
        ccsd_use_disk == NWChemToggle_disabled ? "false" : "true";
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "ccsd:usedisk",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, value) != 0)
      return -1;
  }
  if (ccsd_same_spin_scale > 0.0) {
    char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    snprintf(value, sizeof(value), "%.15g", ccsd_same_spin_scale);
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "ccsd:fss", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
            value) != 0)
      return -1;
  }
  if (ccsd_opposite_spin_scale > 0.0) {
    char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    snprintf(value, sizeof(value), "%.15g", ccsd_opposite_spin_scale);
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "ccsd:fos", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
            value) != 0)
      return -1;
  }
  const struct {
    const char *key;
    int toggle;
  } ccsd_toggle_fields[] = {
      {"ccsd:use_trpdrv_nb", ccsd_use_trpdrv_nb},
      {"ccsd:use_ccsd_omp", ccsd_use_ccsd_omp},
      {"ccsd:use_trpdrv_omp", ccsd_use_trpdrv_omp},
      {"ccsd:use_trpdrv_offload", ccsd_use_trpdrv_offload},
  };
  for (size_t i = 0; i < sizeof(ccsd_toggle_fields) /
                             sizeof(ccsd_toggle_fields[0]);
       ++i) {
    const char *value = toggle_logical_value(ccsd_toggle_fields[i].toggle);
    if (value &&
        append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, ccsd_toggle_fields[i].key,
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, value) != 0)
      return -1;
  }
  if (append_tce_direct_values(params_root, typed_set_keys, typed_set_types,
                               typed_set_value_counts, typed_set_values,
                               NWCHEMC_DIRECT_SET_MAX,
                               NWCHEMC_DIRECT_SET_VALUE_MAX,
                               &typed_set_count, nwpw_direct_keys,
                               nwpw_direct_values) != 0)
    return -1;
  if (append_simulation_cell_direct_values(
          params_root, typed_set_keys, typed_set_types, typed_set_value_counts,
          typed_set_values, NWCHEMC_DIRECT_SET_MAX,
          NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
          nwpw_direct_values) != 0)
    return -1;
  if (nwpw_energy_cutoff > 0.0) {
    char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    snprintf(value, sizeof(value), "%.17g", nwpw_energy_cutoff);
    if (append_nwpw_direct_typed_values(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "ecut", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
            value) != 0)
      return -1;
  }
  if (nwpw_wavefunction_cutoff > 0.0) {
    char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    snprintf(value, sizeof(value), "%.17g", nwpw_wavefunction_cutoff);
    if (append_nwpw_direct_typed_values(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "wcut", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
            value) != 0)
      return -1;
  }
  if (nwpw_ewald_rcut > 0.0) {
    char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    snprintf(value, sizeof(value), "%.17g", nwpw_ewald_rcut);
    if (append_nwpw_direct_typed_values(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "rcut", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
            value) != 0)
      return -1;
  }
  if (nwpw_ewald_ncut > 0) {
    char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    snprintf(value, sizeof(value), "%d", nwpw_ewald_ncut);
    if (append_nwpw_direct_typed_values(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "ncut", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
            value) != 0)
      return -1;
  }
  if (nwpw_cell_name.len > 0) {
    char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    const char *value_list[1] = {value};
    copy_text_record(value, sizeof(value), nwpw_cell_name);
    if (append_nwpw_prefixed_typed_values(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, nwpw_cgsd_band_cpsd, 3, "cell_name",
            NWCHEMC_DIRECT_SET_VALUE_TEXT, value_list, 1) != 0)
      return -1;
  }
  if (nwpw_input_wavefunction_filename.len > 0) {
    char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    const char *value_list[1] = {value};
    copy_text_record(value, sizeof(value), nwpw_input_wavefunction_filename);
    if (append_nwpw_prefixed_typed_values(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, nwpw_cgsd_band, 2,
            "input_wavefunction_filename", NWCHEMC_DIRECT_SET_VALUE_TEXT,
            value_list, 1) != 0)
      return -1;
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "pspw:input vectors",
            NWCHEMC_DIRECT_SET_VALUE_TEXT, value) != 0)
      return -1;
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "band:input vectors",
            NWCHEMC_DIRECT_SET_VALUE_TEXT, value) != 0)
      return -1;
  }
  if (nwpw_output_wavefunction_filename.len > 0) {
    char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    const char *value_list[1] = {value};
    copy_text_record(value, sizeof(value), nwpw_output_wavefunction_filename);
    if (append_nwpw_prefixed_typed_values(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, nwpw_cgsd_band, 2,
            "output_wavefunction_filename", NWCHEMC_DIRECT_SET_VALUE_TEXT,
            value_list, 1) != 0)
      return -1;
  }
  if (nwpw_fake_mass > 0.0) {
    char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    const char *value_list[1] = {value};
    snprintf(value, sizeof(value), "%.17g", nwpw_fake_mass);
    if (append_nwpw_prefixed_typed_values(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, nwpw_cgsd_band_cpsd, 3, "fake_mass",
            NWCHEMC_DIRECT_SET_VALUE_DOUBLE, value_list, 1) != 0)
      return -1;
  }
  if (nwpw_time_step > 0.0) {
    char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    const char *value_list[1] = {value};
    snprintf(value, sizeof(value), "%.17g", nwpw_time_step);
    if (append_nwpw_prefixed_typed_values(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, nwpw_cgsd_band_cpsd, 3, "time_step",
            NWCHEMC_DIRECT_SET_VALUE_DOUBLE, value_list, 1) != 0)
      return -1;
  }
  if (nwpw_loop_start > 0 && nwpw_loop_end > 0) {
    char start_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    char end_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    const char *value_list[2] = {start_value, end_value};
    snprintf(start_value, sizeof(start_value), "%d", nwpw_loop_start);
    snprintf(end_value, sizeof(end_value), "%d", nwpw_loop_end);
    if (append_nwpw_prefixed_typed_values(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, nwpw_cgsd_band_cpsd, 3, "loop",
            NWCHEMC_DIRECT_SET_VALUE_INTEGER, value_list, 2) != 0)
      return -1;
  }
  if (nwpw_has_tolerances) {
    char energy_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    char density_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    char gradient_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    const char *value_list[3] = {energy_value, density_value, gradient_value};
    snprintf(energy_value, sizeof(energy_value), "%.17g",
             nwpw_tolerance_energy);
    snprintf(density_value, sizeof(density_value), "%.17g",
             nwpw_tolerance_density);
    snprintf(gradient_value, sizeof(gradient_value), "%.17g",
             nwpw_tolerance_gradient);
    if (append_nwpw_prefixed_typed_values(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, nwpw_cgsd_band, 2, "tolerances",
            NWCHEMC_DIRECT_SET_VALUE_DOUBLE, value_list, 3) != 0)
      return -1;
  }
  if (nwpw_has_xc && nwpw_exchange_correlation.len > 0) {
    char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    copy_text_record(value, sizeof(value), nwpw_exchange_correlation);
    if (append_nwpw_direct_typed_values(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "exchange_correlation",
            NWCHEMC_DIRECT_SET_VALUE_TEXT, value) != 0)
      return -1;
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "pspw:SIC_all",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "false") != 0)
      return -1;
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "pspw:HFX", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
            "false") != 0)
      return -1;
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "band:HFX", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
            "false") != 0)
      return -1;
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "pspw:SIC_xc_parameter",
            NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "1") != 0)
      return -1;
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "pspw:SIC_h_parameter",
            NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "1") != 0)
      return -1;
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "pspw:HFX_parameter",
            NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "1") != 0)
      return -1;
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "band:HFX_parameter",
            NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "1") != 0)
      return -1;
  }
  if (nwpw_bo_has_options &&
      nwpw_balance_mode != NWChemNwpwBalanceMode_unspecified) {
    const char *value =
        nwpw_balance_mode == NWChemNwpwBalanceMode_balance ? "true" : "false";
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:balance",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, value) != 0)
      return -1;
  }
  if (nwpw_bo_step_start > 0 && nwpw_bo_step_end > 0) {
    char start_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    char end_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    const char *value_list[2] = {start_value, end_value};
    snprintf(start_value, sizeof(start_value), "%d", nwpw_bo_step_start);
    snprintf(end_value, sizeof(end_value), "%d", nwpw_bo_step_end);
    if (append_direct_typed_values(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:bo_steps",
            NWCHEMC_DIRECT_SET_VALUE_INTEGER, value_list, 2) != 0)
      return -1;
  }
  if (nwpw_bo_time_step > 0.0) {
    char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    snprintf(value, sizeof(value), "%.17g", nwpw_bo_time_step);
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:bo_time_step",
            NWCHEMC_DIRECT_SET_VALUE_DOUBLE, value) != 0)
      return -1;
  }
  if (nwpw_bo_algorithm != NWChemNwpwBoAlgorithm_unspecified) {
    char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    int algorithm_value = 0;
    if (nwpw_bo_algorithm == NWChemNwpwBoAlgorithm_velocityVerlet)
      algorithm_value = 1;
    else if (nwpw_bo_algorithm == NWChemNwpwBoAlgorithm_leapFrog)
      algorithm_value = 2;
    snprintf(value, sizeof(value), "%d", algorithm_value);
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:bo_algorithm",
            NWCHEMC_DIRECT_SET_VALUE_INTEGER, value) != 0)
      return -1;
  }
  if (nwpw_bo_fake_mass > 0.0) {
    char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    snprintf(value, sizeof(value), "%.17g", nwpw_bo_fake_mass);
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:bo_fake_mass",
            NWCHEMC_DIRECT_SET_VALUE_DOUBLE, value) != 0)
      return -1;
  }
  if (nwpw_has_scaling) {
    char first_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    char second_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    const char *value_list[2] = {first_value, second_value};
    snprintf(first_value, sizeof(first_value), "%.17g", nwpw_scaling_first);
    snprintf(second_value, sizeof(second_value), "%.17g", nwpw_scaling_second);
    if (append_direct_typed_values(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:scaling",
            NWCHEMC_DIRECT_SET_VALUE_DOUBLE, value_list, 2) != 0)
      return -1;
    if (append_direct_typed_values(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "cpmd:scaling",
            NWCHEMC_DIRECT_SET_VALUE_DOUBLE, value_list, 2) != 0)
      return -1;
  }
  if (nwpw_np_fft != 0 || nwpw_np_orbital != 0 || nwpw_np_kspace != 0) {
    char fft_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    char orbital_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    char kspace_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    const char *value_list[3] = {fft_value, orbital_value, kspace_value};
    snprintf(fft_value, sizeof(fft_value), "%d", nwpw_np_fft);
    snprintf(orbital_value, sizeof(orbital_value), "%d", nwpw_np_orbital);
    snprintf(kspace_value, sizeof(kspace_value), "%d", nwpw_np_kspace);
    if (append_direct_typed_values(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:np_dimensions",
            NWCHEMC_DIRECT_SET_VALUE_INTEGER, value_list, 3) != 0)
      return -1;
  }
  if (nwpw_execution_has_options &&
      nwpw_spin_orbit != NWChemNwpwToggle_unspecified) {
    const char *value =
        nwpw_spin_orbit == NWChemNwpwToggle_enabled ? "true" : "false";
    static const char *spin_prefixes[] = {"nwpw", "pspw", "band"};
    const char *value_list[1] = {value};
    if (append_nwpw_prefixed_typed_values(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, spin_prefixes, 3, "spin_orbit",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, value_list, 1) != 0)
      return -1;
  }
  if (nwpw_execution_has_options &&
      nwpw_parallel_io != NWChemNwpwToggle_unspecified) {
    const char *value =
        nwpw_parallel_io == NWChemNwpwToggle_enabled ? "true" : "false";
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:parallel_io",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, value) != 0)
      return -1;
  }
  if (psp_spin_has_options) {
    char count_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    const char *enabled_value = psp_spin_enabled ? "true" : "false";
    snprintf(count_value, sizeof(count_value), "%d", psp_spin_count);
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:pspspin",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, enabled_value) != 0)
      return -1;
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:pspspin_count",
            NWCHEMC_DIRECT_SET_VALUE_INTEGER, count_value) != 0)
      return -1;
    if (psp_spin_enabled && psp_spin_count > 0) {
      struct direct_pspspin_rule_append_state pspspin_rule_state = {
          .keys = typed_set_keys,
          .value_types = typed_set_types,
          .value_counts = typed_set_value_counts,
          .values = typed_set_values,
          .key_capacity = NWCHEMC_DIRECT_SET_MAX,
          .value_capacity = NWCHEMC_DIRECT_SET_VALUE_MAX,
          .count = &typed_set_count,
          .key_storage = nwpw_direct_keys,
          .value_storage = nwpw_direct_values,
      };
      size_t walked_pspspin_rules = 0;
      if (nwchemc_params_for_each_direct_pseudopotential_spin_rule(
              params_root, append_direct_pspspin_rule, &pspspin_rule_state,
              &walked_pspspin_rules) != 0 ||
          walked_pspspin_rules != (size_t)psp_spin_count)
        return -1;
    }
  }
  static const char *nwpw_cpmd_nwpw[] = {"cpmd", "nwpw"};
  const struct {
    capn_text text;
    const char *suffix;
  } nwpw_filename_keys[] = {
      {nwpw_xyz_filename, "xyz_filename"},
      {nwpw_ion_motion_filename, "ion_motion_filename"},
      {nwpw_electron_motion_filename, "emotion_filename"},
      {nwpw_hamiltonian_motion_filename, "hmotion_filename"},
      {nwpw_orbital_motion_filename, "omotion_filename"},
      {nwpw_eigenvalue_motion_filename, "eigmotion_filename"},
  };
  if (nwpw_filenames_has_options) {
    for (size_t i = 0; i < sizeof(nwpw_filename_keys) /
                               sizeof(nwpw_filename_keys[0]);
         ++i) {
      if (nwpw_filename_keys[i].text.len <= 0)
        continue;
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      const char *value_list[1] = {value};
      copy_text_record(value, sizeof(value), nwpw_filename_keys[i].text);
      if (append_nwpw_prefixed_typed_values(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, nwpw_cpmd_nwpw, 2,
              nwpw_filename_keys[i].suffix, NWCHEMC_DIRECT_SET_VALUE_TEXT,
              value_list, 1) != 0)
        return -1;
    }
  }
  if (nwpw_has_fractional) {
    char start_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    char end_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    const char *value_list[2] = {start_value, end_value};
    snprintf(start_value, sizeof(start_value), "%d",
             nwpw_fractional_orbitals_start);
    snprintf(end_value, sizeof(end_value), "%d",
             nwpw_fractional_orbitals_end);
    if (append_direct_typed_values(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:fractional_orbitals",
            NWCHEMC_DIRECT_SET_VALUE_INTEGER, value_list, 2) != 0)
      return -1;
  }
  if (nwpw_has_smear) {
    if (nwpw_smear_temperature > 0.0) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(value, sizeof(value), "%.15g", nwpw_smear_temperature);
      if (append_direct_typed_value(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values,
              "nwpw:fractional_temperature",
              NWCHEMC_DIRECT_SET_VALUE_DOUBLE, value) != 0)
        return -1;
    }
    if (nwpw_smear_alpha > 0.0) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(value, sizeof(value), "%.15g", nwpw_smear_alpha);
      if (append_direct_typed_value(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, "nwpw:fractional_alpha",
              NWCHEMC_DIRECT_SET_VALUE_DOUBLE, value) != 0)
        return -1;
    }
    if (nwpw_smear_type != NWChemNwpwSmearType_unspecified) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      int smear_value = 2;
      if (nwpw_smear_type == NWChemNwpwSmearType_fixed)
        smear_value = -1;
      else if (nwpw_smear_type == NWChemNwpwSmearType_step)
        smear_value = 0;
      else if (nwpw_smear_type == NWChemNwpwSmearType_fermi)
        smear_value = 1;
      else if (nwpw_smear_type == NWChemNwpwSmearType_gaussian)
        smear_value = 2;
      else if (nwpw_smear_type == NWChemNwpwSmearType_marzariVanderbilt)
        smear_value = 4;
      snprintf(value, sizeof(value), "%d", smear_value);
      if (append_direct_typed_value(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values,
              "nwpw:fractional_smeartype",
              NWCHEMC_DIRECT_SET_VALUE_INTEGER, value) != 0)
        return -1;
    }
  }
  if (nwpw_virtual_orbitals_start > 0 && nwpw_virtual_orbitals_end > 0) {
    char start_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    char end_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    const char *value_list[2] = {start_value, end_value};
    snprintf(start_value, sizeof(start_value), "%d",
             nwpw_virtual_orbitals_start);
    snprintf(end_value, sizeof(end_value), "%d", nwpw_virtual_orbitals_end);
    if (append_direct_typed_values(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:excited_ne",
            NWCHEMC_DIRECT_SET_VALUE_INTEGER, value_list, 2) != 0)
      return -1;
  }
  if (nwpw_orbital_grid_has_options &&
      nwpw_lcao_mode != NWChemNwpwLcaoMode_unspecified) {
    const char *value =
        nwpw_lcao_mode == NWChemNwpwLcaoMode_skip ? "true" : "false";
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:lcao_skip",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, value) != 0)
      return -1;
  }
  if (nwpw_ewald_grid_x > 0) {
    char x_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    char y_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    char z_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    const char *value_list[3] = {x_value, y_value, z_value};
    snprintf(x_value, sizeof(x_value), "%d", nwpw_ewald_grid_x);
    snprintf(y_value, sizeof(y_value), "%d", nwpw_ewald_grid_y);
    snprintf(z_value, sizeof(z_value), "%d", nwpw_ewald_grid_z);
    if (append_direct_typed_values(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:ewald_ngrid",
            NWCHEMC_DIRECT_SET_VALUE_INTEGER, value_list, 3) != 0)
      return -1;
  }
  if (nwpw_nose_has_options &&
      nwpw_nose_hoover != NWChemNwpwToggle_unspecified) {
    static const char *nwpw_nose_prefixes[] = {"cpmd", "nwpw"};
    const char *nose_value =
        nwpw_nose_hoover == NWChemNwpwToggle_enabled ? "true" : "false";
    const char *nose_values[1] = {nose_value};
    if (append_nwpw_prefixed_typed_values(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, nwpw_nose_prefixes, 2, "nose",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, nose_values, 1) != 0)
      return -1;
    if (nwpw_nose_hoover == NWChemNwpwToggle_enabled) {
      const char *restart_value =
          nwpw_nose_restart == NWChemNwpwToggle_disabled ? "false" : "true";
      const char *restart_values[1] = {restart_value};
      char electron_period_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      char electron_temperature_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      char ion_period_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      char ion_temperature_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      char electron_chain_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      char ion_chain_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      const char *electron_period_values[1] = {electron_period_value};
      const char *electron_temperature_values[1] = {
          electron_temperature_value};
      const char *ion_period_values[1] = {ion_period_value};
      const char *ion_temperature_values[1] = {ion_temperature_value};
      const char *electron_chain_values[1] = {electron_chain_value};
      const char *ion_chain_values[1] = {ion_chain_value};
      snprintf(electron_period_value, sizeof(electron_period_value), "%.15g",
               nwpw_nose_electron_period);
      snprintf(electron_temperature_value, sizeof(electron_temperature_value),
               "%.15g", nwpw_nose_electron_temperature);
      snprintf(ion_period_value, sizeof(ion_period_value), "%.15g",
               nwpw_nose_ion_period);
      snprintf(ion_temperature_value, sizeof(ion_temperature_value), "%.15g",
               nwpw_nose_ion_temperature);
      snprintf(electron_chain_value, sizeof(electron_chain_value), "%d",
               nwpw_nose_electron_chain_length);
      snprintf(ion_chain_value, sizeof(ion_chain_value), "%d",
               nwpw_nose_ion_chain_length);
      if (append_nwpw_prefixed_typed_values(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, nwpw_nose_prefixes, 2,
              "nose_restart", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
              restart_values, 1) != 0)
        return -1;
      if (append_nwpw_prefixed_typed_values(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, nwpw_nose_prefixes, 2,
              "Pe", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, electron_period_values,
              1) != 0)
        return -1;
      if (append_nwpw_prefixed_typed_values(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, nwpw_nose_prefixes, 2,
              "Te", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
              electron_temperature_values, 1) != 0)
        return -1;
      if (append_nwpw_prefixed_typed_values(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, nwpw_nose_prefixes, 2,
              "Pr", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, ion_period_values, 1) !=
          0)
        return -1;
      if (append_nwpw_prefixed_typed_values(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, nwpw_nose_prefixes, 2,
              "Tr", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, ion_temperature_values,
              1) != 0)
        return -1;
      if (append_nwpw_prefixed_typed_values(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, nwpw_nose_prefixes, 2,
              "Mchain", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
              electron_chain_values, 1) != 0)
        return -1;
      if (append_nwpw_prefixed_typed_values(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, nwpw_nose_prefixes, 2,
              "Nchain", NWCHEMC_DIRECT_SET_VALUE_INTEGER, ion_chain_values,
              1) != 0)
        return -1;
    }
  }
  memset(packed_set_keys, 0, sizeof(packed_set_keys));
  memset(packed_set_values, 0, sizeof(packed_set_values));
  memset(packed_typed_set_keys, 0, sizeof(packed_typed_set_keys));
  memset(packed_typed_set_values, 0, sizeof(packed_typed_set_values));
  for (size_t i = 0; i < set_count; ++i) {
    copy_text_record(packed_set_keys + i * NWCHEMC_DIRECT_SET_KEY_LEN,
                     NWCHEMC_DIRECT_SET_KEY_LEN, set_keys[i]);
    copy_text_record(packed_set_values + i * NWCHEMC_DIRECT_SET_VALUE_LEN,
                     NWCHEMC_DIRECT_SET_VALUE_LEN, set_values[i]);
  }
  for (size_t i = 0; i < typed_set_count; ++i) {
    copy_text_record(packed_typed_set_keys + i * NWCHEMC_DIRECT_SET_KEY_LEN,
                     NWCHEMC_DIRECT_SET_KEY_LEN, typed_set_keys[i]);
    for (int j = 0; j < typed_set_value_counts[i]; ++j) {
      size_t offset = (i * NWCHEMC_DIRECT_SET_VALUE_MAX + (size_t)j) *
                      NWCHEMC_DIRECT_SET_VALUE_LEN;
      copy_text_record(packed_typed_set_values + offset,
                       NWCHEMC_DIRECT_SET_VALUE_LEN,
                       typed_set_values[i * NWCHEMC_DIRECT_SET_VALUE_MAX +
                                        (size_t)j]);
    }
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
  if (nwchemc_embed_set_rtdb_strings(packed_set_keys, packed_set_values,
                                     (int)set_count) != 0)
    return -1;
  if (nwchemc_embed_set_rtdb_values(
          packed_typed_set_keys, typed_set_types, typed_set_value_counts,
          packed_typed_set_values, (int)typed_set_count) != 0)
    return -1;
  int brillouin_zone_name_len = 0;
  const char *brillouin_zone_name_text =
      text_or_with_len(brillouin_zone_name, "zone_default",
                       &brillouin_zone_name_len);
  double no_brillouin_kvectors[1] = {0.0};
  double *brillouin_kvectors = NULL;
  const double *brillouin_kvectors_arg = no_brillouin_kvectors;
  if (brillouin_kvector_count > 0) {
    if (brillouin_kvector_count > (size_t)INT_MAX ||
        brillouin_kvector_count > SIZE_MAX / (4 * sizeof(double)))
      return -1;
    brillouin_kvectors =
        malloc(4 * brillouin_kvector_count * sizeof(*brillouin_kvectors));
    if (!brillouin_kvectors)
      return -1;
    size_t filled_kvectors = 0;
    if (nwchemc_params_extract_direct_brillouin_zone(
            params_root, &brillouin_has_options, &brillouin_zone_name,
            brillouin_monkhorst_pack, &brillouin_max_kpoints_print,
            brillouin_kvectors, brillouin_kvector_count,
            &filled_kvectors) != 0 ||
        filled_kvectors != brillouin_kvector_count) {
      free(brillouin_kvectors);
      return -1;
    }
    brillouin_kvectors_arg = brillouin_kvectors;
  }
  if (nwchemc_embed_set_brillouin_zone(
          brillouin_has_options, brillouin_zone_name_text,
          brillouin_zone_name_len, brillouin_monkhorst_pack[0],
          brillouin_monkhorst_pack[1], brillouin_monkhorst_pack[2],
          brillouin_max_kpoints_print, brillouin_kvectors_arg,
          (int)brillouin_kvector_count) != 0) {
    free(brillouin_kvectors);
    return -1;
  }
  free(brillouin_kvectors);
  if (nwchemc_embed_set_nwpw_direct(nwpw_has_options, nwpw_energy_cutoff,
                                    nwpw_wavefunction_cutoff,
                                    nwpw_ewald_rcut, nwpw_ewald_ncut) != 0)
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

NWChemCResult nwchemc_dipole(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *dipole_au) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';

  if (n_atoms <= 0 || !positions_ang || !atomic_numbers || !dipole_au) {
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
  int rc = nwchemc_embed_dipole(&n, positions_ang, atomic_numbers, &ch, &mult,
                                &eh, dipole_au, errmsg,
                                (int)sizeof(errmsg) - 1);
  nwchemc_params_release(&arena);
  if (rc != 0) {
    snprintf(r.message, sizeof(r.message), "%s",
             errmsg[0] ? errmsg : "nwchem embed dipole failed");
    return r;
  }
  r.ok = 1;
  r.energy_h = eh;
  snprintf(r.message, sizeof(r.message), "ok");
  return r;
}

NWChemCResult nwchemc_quadrupole(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *quadrupole_au) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';

  if (n_atoms <= 0 || !positions_ang || !atomic_numbers || !quadrupole_au) {
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
  int rc = nwchemc_embed_quadrupole(&n, positions_ang, atomic_numbers, &ch,
                                    &mult, &eh, quadrupole_au, errmsg,
                                    (int)sizeof(errmsg) - 1);
  nwchemc_params_release(&arena);
  if (rc != 0) {
    snprintf(r.message, sizeof(r.message), "%s",
             errmsg[0] ? errmsg : "nwchem embed quadrupole failed");
    return r;
  }
  r.ok = 1;
  r.energy_h = eh;
  snprintf(r.message, sizeof(r.message), "ok");
  return r;
}

NWChemCResult nwchemc_optimize(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *optimized_positions_ang) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';

  if (n_atoms <= 0 || !positions_ang || !atomic_numbers ||
      !optimized_positions_ang) {
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
  int rc = nwchemc_embed_optimize(&n, positions_ang, atomic_numbers, &ch,
                                  &mult, &eh, optimized_positions_ang, errmsg,
                                  (int)sizeof(errmsg) - 1);
  nwchemc_params_release(&arena);
  if (rc != 0) {
    snprintf(r.message, sizeof(r.message), "%s",
             errmsg[0] ? errmsg : "nwchem embed optimize failed");
    return r;
  }
  r.ok = 1;
  r.energy_h = eh;
  snprintf(r.message, sizeof(r.message), "ok");
  return r;
}

NWChemCResult nwchemc_frequencies(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *frequencies_cm1, double *intensities_au) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';

  if (n_atoms <= 0 || !positions_ang || !atomic_numbers || !frequencies_cm1 ||
      n_atoms > INT_MAX / 3) {
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

  size_t ndof = (size_t)n_atoms * 3u;
  double *scratch_intensities = NULL;
  double *intensity_arg = intensities_au;
  if (!intensity_arg) {
    scratch_intensities = (double *)calloc(ndof, sizeof(double));
    if (!scratch_intensities) {
      nwchemc_params_release(&arena);
      snprintf(r.message, sizeof(r.message), "out of memory");
      return r;
    }
    intensity_arg = scratch_intensities;
  }

  char errmsg[512];
  memset(errmsg, 0, sizeof(errmsg));
  int n = n_atoms;
  int ch = params.charge;
  int mult = params.multiplicity > 0 ? params.multiplicity : 1;
  int rc = nwchemc_embed_frequencies(&n, positions_ang, atomic_numbers, &ch,
                                     &mult, frequencies_cm1, intensity_arg,
                                     errmsg, (int)sizeof(errmsg) - 1);
  free(scratch_intensities);
  nwchemc_params_release(&arena);
  if (rc != 0) {
    snprintf(r.message, sizeof(r.message), "%s",
             errmsg[0] ? errmsg : "nwchem embed frequencies failed");
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

static void session_clear_topology(NWChemCSession *session) {
  if (!session)
    return;
  free(session->topology_atomic_numbers);
  session->topology_atomic_numbers = NULL;
  session->topology_atom_count = 0;
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

static int session_check_topology(NWChemCSession *session, size_t n_atoms,
                                  const int *atomic_numbers) {
  if (!session || n_atoms == 0 || !atomic_numbers)
    return -1;

  if (session->topology_atom_count == 0) {
    if (n_atoms > SIZE_MAX / sizeof(*session->topology_atomic_numbers))
      return -1;
    int *topology =
        (int *)malloc(n_atoms * sizeof(*session->topology_atomic_numbers));
    if (!topology)
      return -1;
    memcpy(topology, atomic_numbers, n_atoms * sizeof(*topology));
    session->topology_atomic_numbers = topology;
    session->topology_atom_count = n_atoms;
    return 0;
  }

  if (session->topology_atom_count != n_atoms)
    return 1;
  for (size_t i = 0; i < n_atoms; ++i) {
    if (session->topology_atomic_numbers[i] != atomic_numbers[i])
      return 1;
  }
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
  {
    struct NWChemParams decoded;
    read_NWChemParams(&decoded, session->params_root);
    session->charge = decoded.charge;
    session->multiplicity = decoded.multiplicity > 0 ? decoded.multiplicity : 1;
  }
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
  /* Session step calls reuse decoded scalar state. */
  *charge = session->charge;
  *multiplicity = session->multiplicity;
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
  if (session->topology_atom_count != 0)
    return -1;
  return session_install_params(session, params_capnp,
                                params_capnp_size_bytes);
}

void nwchemc_session_destroy(NWChemCSession *session) {
  if (!session)
    return;
  if (g_active_session == session)
    g_active_session = NULL;
  session_clear_params(session);
  session_clear_step_scratch(session);
  session_clear_topology(session);
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
  int topology_status =
      session_check_topology(session, (size_t)n_atoms, atomic_numbers);
  if (topology_status < 0) {
    snprintf(r.message, sizeof(r.message), "out of memory");
    return r;
  }
  if (topology_status > 0) {
    snprintf(r.message, sizeof(r.message), "session topology mismatch");
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

static NWChemCResult session_dipole_cell(NWChemCSession *session, int n_atoms,
                                         const double *positions_ang,
                                         const int *atomic_numbers,
                                         const double *cell_ang, int has_cell,
                                         double *dipole_au) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!session || n_atoms <= 0 || !positions_ang || !atomic_numbers ||
      !dipole_au) {
    snprintf(r.message, sizeof(r.message), "invalid arguments");
    return r;
  }
  int topology_status =
      session_check_topology(session, (size_t)n_atoms, atomic_numbers);
  if (topology_status < 0) {
    snprintf(r.message, sizeof(r.message), "out of memory");
    return r;
  }
  if (topology_status > 0) {
    snprintf(r.message, sizeof(r.message), "session topology mismatch");
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
  int rc = nwchemc_embed_dipole_cell(
      &n, positions_ang, atomic_numbers, cell_arg, &cell_flag, &ch, &mult, &eh,
      dipole_au, errmsg, (int)sizeof(errmsg) - 1);
  if (rc != 0) {
    snprintf(r.message, sizeof(r.message), "%s",
             errmsg[0] ? errmsg : "nwchem embed dipole failed");
    return r;
  }
  r.ok = 1;
  r.energy_h = eh;
  snprintf(r.message, sizeof(r.message), "ok");
  return r;
}

NWChemCResult nwchemc_session_dipole(NWChemCSession *session, int n_atoms,
                                     const double *positions_ang,
                                     const int *atomic_numbers,
                                     double *dipole_au) {
  return session_dipole_cell(session, n_atoms, positions_ang, atomic_numbers,
                             NULL, 0, dipole_au);
}

static NWChemCResult session_quadrupole_cell(
    NWChemCSession *session, int n_atoms, const double *positions_ang,
    const int *atomic_numbers, const double *cell_ang, int has_cell,
    double *quadrupole_au) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!session || n_atoms <= 0 || !positions_ang || !atomic_numbers ||
      !quadrupole_au) {
    snprintf(r.message, sizeof(r.message), "invalid arguments");
    return r;
  }
  int topology_status =
      session_check_topology(session, (size_t)n_atoms, atomic_numbers);
  if (topology_status < 0) {
    snprintf(r.message, sizeof(r.message), "out of memory");
    return r;
  }
  if (topology_status > 0) {
    snprintf(r.message, sizeof(r.message), "session topology mismatch");
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
  int rc = nwchemc_embed_quadrupole_cell(
      &n, positions_ang, atomic_numbers, cell_arg, &cell_flag, &ch, &mult, &eh,
      quadrupole_au, errmsg, (int)sizeof(errmsg) - 1);
  if (rc != 0) {
    snprintf(r.message, sizeof(r.message), "%s",
             errmsg[0] ? errmsg : "nwchem embed quadrupole failed");
    return r;
  }
  r.ok = 1;
  r.energy_h = eh;
  snprintf(r.message, sizeof(r.message), "ok");
  return r;
}

NWChemCResult nwchemc_session_quadrupole(NWChemCSession *session, int n_atoms,
                                         const double *positions_ang,
                                         const int *atomic_numbers,
                                         double *quadrupole_au) {
  return session_quadrupole_cell(session, n_atoms, positions_ang,
                                 atomic_numbers, NULL, 0, quadrupole_au);
}

static NWChemCResult session_optimize_cell(
    NWChemCSession *session, int n_atoms, const double *positions_ang,
    const int *atomic_numbers, const double *cell_ang, int has_cell,
    double *optimized_positions_ang) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!session || n_atoms <= 0 || !positions_ang || !atomic_numbers ||
      !optimized_positions_ang) {
    snprintf(r.message, sizeof(r.message), "invalid arguments");
    return r;
  }
  int topology_status =
      session_check_topology(session, (size_t)n_atoms, atomic_numbers);
  if (topology_status < 0) {
    snprintf(r.message, sizeof(r.message), "out of memory");
    return r;
  }
  if (topology_status > 0) {
    snprintf(r.message, sizeof(r.message), "session topology mismatch");
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
  int rc = nwchemc_embed_optimize_cell(
      &n, positions_ang, atomic_numbers, cell_arg, &cell_flag, &ch, &mult, &eh,
      optimized_positions_ang, errmsg, (int)sizeof(errmsg) - 1);
  if (rc != 0) {
    snprintf(r.message, sizeof(r.message), "%s",
             errmsg[0] ? errmsg : "nwchem embed optimize failed");
    return r;
  }
  r.ok = 1;
  r.energy_h = eh;
  snprintf(r.message, sizeof(r.message), "ok");
  return r;
}

NWChemCResult nwchemc_session_optimize(NWChemCSession *session, int n_atoms,
                                       const double *positions_ang,
                                       const int *atomic_numbers,
                                       double *optimized_positions_ang) {
  return session_optimize_cell(session, n_atoms, positions_ang, atomic_numbers,
                               NULL, 0, optimized_positions_ang);
}

static NWChemCResult session_frequencies_cell(
    NWChemCSession *session, int n_atoms, const double *positions_ang,
    const int *atomic_numbers, const double *cell_ang, int has_cell,
    double *frequencies_cm1, double *intensities_au) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!session || n_atoms <= 0 || !positions_ang || !atomic_numbers ||
      !frequencies_cm1 || n_atoms > INT_MAX / 3) {
    snprintf(r.message, sizeof(r.message), "invalid arguments");
    return r;
  }
  int topology_status =
      session_check_topology(session, (size_t)n_atoms, atomic_numbers);
  if (topology_status < 0) {
    snprintf(r.message, sizeof(r.message), "out of memory");
    return r;
  }
  if (topology_status > 0) {
    snprintf(r.message, sizeof(r.message), "session topology mismatch");
    return r;
  }
  if (session_apply_config(session) != 0) {
    snprintf(r.message, sizeof(r.message), "embed config failed");
    return r;
  }
  ensure_init();
  size_t ndof = (size_t)n_atoms * 3u;
  double *scratch_intensities = NULL;
  double *intensity_arg = intensities_au;
  if (!intensity_arg) {
    scratch_intensities = (double *)calloc(ndof, sizeof(double));
    if (!scratch_intensities) {
      snprintf(r.message, sizeof(r.message), "out of memory");
      return r;
    }
    intensity_arg = scratch_intensities;
  }
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
  int rc = nwchemc_embed_frequencies_cell(
      &n, positions_ang, atomic_numbers, cell_arg, &cell_flag, &ch, &mult,
      frequencies_cm1, intensity_arg, errmsg, (int)sizeof(errmsg) - 1);
  free(scratch_intensities);
  if (rc != 0) {
    snprintf(r.message, sizeof(r.message), "%s",
             errmsg[0] ? errmsg : "nwchem embed frequencies failed");
    return r;
  }
  r.ok = 1;
  snprintf(r.message, sizeof(r.message), "ok");
  return r;
}

NWChemCResult nwchemc_session_frequencies(
    NWChemCSession *session, int n_atoms, const double *positions_ang,
    const int *atomic_numbers, double *frequencies_cm1,
    double *intensities_au) {
  return session_frequencies_cell(session, n_atoms, positions_ang,
                                  atomic_numbers, NULL, 0, frequencies_cm1,
                                  intensities_au);
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

NWChemCResult nwchemc_calculate_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!params_capnp || params_capnp_size_bytes == 0 || !force_input_capnp ||
      force_input_capnp_size_bytes == 0 || !potential_result_capnp_size_bytes) {
    snprintf(r.message, sizeof(r.message), "invalid arguments");
    return r;
  }
  *potential_result_capnp_size_bytes = 0;

  size_t required_size = nwchemc_potential_result_size_for_force_input(
      force_input_capnp, force_input_capnp_size_bytes);
  *potential_result_capnp_size_bytes = required_size;
  if (required_size == 0) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }
  if (!potential_result_capnp ||
      potential_result_capnp_capacity_bytes < required_size) {
    snprintf(r.message, sizeof(r.message), "PotentialResult buffer too small");
    return r;
  }

  NWChemCSession *session =
      nwchemc_session_create(params_capnp, params_capnp_size_bytes);
  if (!session) {
    snprintf(r.message, sizeof(r.message), "embed config failed");
    return r;
  }
  r = nwchemc_session_calculate_result(
      session, force_input_capnp, force_input_capnp_size_bytes,
      potential_result_capnp, potential_result_capnp_capacity_bytes,
      potential_result_capnp_size_bytes);
  nwchemc_session_destroy(session);
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

size_t nwchemc_hessian_result_size_for_force_input(
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
  size_t ndof = n_atoms * 3u;
  if (ndof > SIZE_MAX / ndof) {
    nwchemc_params_release(&arena);
    return 0;
  }
  size_t hessian_count = ndof * ndof;
  if (hessian_count > (size_t)INT_MAX) {
    nwchemc_params_release(&arena);
    return 0;
  }
  size_t result_size = nwchemc_hessian_result_flat_size(hessian_count);
  nwchemc_params_release(&arena);
  return result_size;
}

static int force_input_step_atom_count(const void *force_input_capnp,
                                       size_t force_input_capnp_size_bytes,
                                       size_t *n_atoms) {
  if (!force_input_capnp || force_input_capnp_size_bytes == 0 || !n_atoms)
    return -1;
  struct capn arena;
  ForceInput_ptr force_input;
  if (nwchemc_force_input_root(force_input_capnp, force_input_capnp_size_bytes,
                               &arena, &force_input) != 0)
    return -1;
  int has_cell = 0;
  int rc = nwchemc_force_input_atom_count(force_input, n_atoms, &has_cell);
  nwchemc_params_release(&arena);
  if (rc != 0 || *n_atoms == 0 || *n_atoms > (size_t)INT_MAX)
    return -1;
  (void)has_cell;
  return 0;
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
  int topology_status =
      session_check_topology(session, (size_t)n_atoms, atomic_numbers);
  if (topology_status < 0) {
    snprintf(r.message, sizeof(r.message), "out of memory");
    return r;
  }
  if (topology_status > 0) {
    snprintf(r.message, sizeof(r.message), "session topology mismatch");
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

NWChemCResult nwchemc_session_calculate_hessian_result(
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
  size_t ndof = n_atoms * 3u;
  if (ndof > SIZE_MAX / ndof) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }
  size_t hessian_count = ndof * ndof;
  size_t required_size = nwchemc_hessian_result_flat_size(hessian_count);
  *potential_result_capnp_size_bytes = required_size;
  if (required_size == 0 || hessian_count > (size_t)INT_MAX) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }

  double energy_factor = 1.0;
  double hessian_factor = 1.0;
  if (nwchemc_force_input_hessian_result_factors(
          force_input, &energy_factor, &hessian_factor) != 0) {
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

  double *hessian = (double *)malloc(hessian_count * sizeof(*hessian));
  if (!hessian) {
    snprintf(r.message, sizeof(r.message), "out of memory");
    return r;
  }
  r = session_hessian_cell(session, (int)n_atoms, session->step_positions_ang,
                           session->step_atomic_numbers, cell_ang, has_cell,
                           hessian);
  if (r.ok) {
    for (size_t i = 0; i < hessian_count; ++i)
      hessian[i] *= hessian_factor;
    if (nwchemc_potential_result_write_hessian(
            r.energy_h * energy_factor, hessian, hessian_count,
            potential_result_capnp, potential_result_capnp_capacity_bytes,
            potential_result_capnp_size_bytes) != 0) {
      r.ok = 0;
      snprintf(r.message, sizeof(r.message), "PotentialResult write failed");
    }
  }
  free(hessian);
  return r;
}

NWChemCResult nwchemc_session_hessian(NWChemCSession *session, int n_atoms,
                                      const double *positions_ang,
                                      const int *atomic_numbers,
                                      double *hessian_h_bohr2) {
  return session_hessian_cell(session, n_atoms, positions_ang, atomic_numbers,
                              NULL, 0, hessian_h_bohr2);
}

NWChemCResult nwchemc_session_calculate_dipole(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *dipole_au,
    size_t dipole_len) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!session || !force_input_capnp || force_input_capnp_size_bytes == 0 ||
      !dipole_au) {
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
      n_atoms > (size_t)INT_MAX || dipole_len < 3u) {
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

  return session_dipole_cell(session, (int)n_atoms, session->step_positions_ang,
                             session->step_atomic_numbers, cell_ang, has_cell,
                             dipole_au);
}

NWChemCResult nwchemc_session_calculate_quadrupole(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *quadrupole_au,
    size_t quadrupole_len) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!session || !force_input_capnp || force_input_capnp_size_bytes == 0 ||
      !quadrupole_au) {
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
      n_atoms > (size_t)INT_MAX || quadrupole_len < 6u) {
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

  return session_quadrupole_cell(
      session, (int)n_atoms, session->step_positions_ang,
      session->step_atomic_numbers, cell_ang, has_cell, quadrupole_au);
}

NWChemCResult nwchemc_session_calculate_optimize(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *optimized_positions_ang,
    size_t optimized_positions_len) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!session || !force_input_capnp || force_input_capnp_size_bytes == 0 ||
      !optimized_positions_ang) {
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
      n_atoms > (size_t)INT_MAX || n_atoms > SIZE_MAX / 3u ||
      optimized_positions_len < n_atoms * 3u) {
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

  return session_optimize_cell(
      session, (int)n_atoms, session->step_positions_ang,
      session->step_atomic_numbers, cell_ang, has_cell,
      optimized_positions_ang);
}

NWChemCResult nwchemc_session_calculate_frequencies(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *frequencies_cm1,
    size_t frequencies_len, double *intensities_au, size_t intensities_len) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!session || !force_input_capnp || force_input_capnp_size_bytes == 0 ||
      !frequencies_cm1) {
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
      n_atoms > (size_t)INT_MAX || n_atoms > SIZE_MAX / 3u ||
      frequencies_len < n_atoms * 3u ||
      (intensities_au && intensities_len < n_atoms * 3u)) {
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

  return session_frequencies_cell(
      session, (int)n_atoms, session->step_positions_ang,
      session->step_atomic_numbers, cell_ang, has_cell, frequencies_cm1,
      intensities_au);
}

NWChemCResult nwchemc_calculate_hessian_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!params_capnp || params_capnp_size_bytes == 0 || !force_input_capnp ||
      force_input_capnp_size_bytes == 0 || !potential_result_capnp_size_bytes) {
    snprintf(r.message, sizeof(r.message), "invalid arguments");
    return r;
  }
  *potential_result_capnp_size_bytes = 0;

  size_t required_size = nwchemc_hessian_result_size_for_force_input(
      force_input_capnp, force_input_capnp_size_bytes);
  *potential_result_capnp_size_bytes = required_size;
  if (required_size == 0) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }
  if (!potential_result_capnp ||
      potential_result_capnp_capacity_bytes < required_size) {
    snprintf(r.message, sizeof(r.message), "PotentialResult buffer too small");
    return r;
  }

  NWChemCSession *session =
      nwchemc_session_create(params_capnp, params_capnp_size_bytes);
  if (!session) {
    snprintf(r.message, sizeof(r.message), "embed config failed");
    return r;
  }
  r = nwchemc_session_calculate_hessian_result(
      session, force_input_capnp, force_input_capnp_size_bytes,
      potential_result_capnp, potential_result_capnp_capacity_bytes,
      potential_result_capnp_size_bytes);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_calculate_hessian(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *hessian_h_bohr2, size_t hessian_len) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!params_capnp || params_capnp_size_bytes == 0 || !force_input_capnp ||
      force_input_capnp_size_bytes == 0 || !hessian_h_bohr2) {
    snprintf(r.message, sizeof(r.message), "invalid arguments");
    return r;
  }
  size_t n_atoms = 0;
  if (force_input_step_atom_count(force_input_capnp,
                                  force_input_capnp_size_bytes,
                                  &n_atoms) != 0 ||
      n_atoms > SIZE_MAX / 3u) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }
  size_t ndof = n_atoms * 3u;
  if (ndof > SIZE_MAX / ndof || hessian_len < ndof * ndof) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }

  NWChemCSession *session =
      nwchemc_session_create(params_capnp, params_capnp_size_bytes);
  if (!session) {
    snprintf(r.message, sizeof(r.message), "embed config failed");
    return r;
  }
  r = nwchemc_session_calculate_hessian(
      session, force_input_capnp, force_input_capnp_size_bytes,
      hessian_h_bohr2, hessian_len);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_calculate_dipole(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *dipole_au, size_t dipole_len) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!params_capnp || params_capnp_size_bytes == 0 || !force_input_capnp ||
      force_input_capnp_size_bytes == 0 || !dipole_au) {
    snprintf(r.message, sizeof(r.message), "invalid arguments");
    return r;
  }
  size_t n_atoms = 0;
  if (force_input_step_atom_count(force_input_capnp,
                                  force_input_capnp_size_bytes,
                                  &n_atoms) != 0 ||
      dipole_len < 3u) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }

  NWChemCSession *session =
      nwchemc_session_create(params_capnp, params_capnp_size_bytes);
  if (!session) {
    snprintf(r.message, sizeof(r.message), "embed config failed");
    return r;
  }
  r = nwchemc_session_calculate_dipole(
      session, force_input_capnp, force_input_capnp_size_bytes, dipole_au,
      dipole_len);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_calculate_quadrupole(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *quadrupole_au, size_t quadrupole_len) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!params_capnp || params_capnp_size_bytes == 0 || !force_input_capnp ||
      force_input_capnp_size_bytes == 0 || !quadrupole_au) {
    snprintf(r.message, sizeof(r.message), "invalid arguments");
    return r;
  }
  size_t n_atoms = 0;
  if (force_input_step_atom_count(force_input_capnp,
                                  force_input_capnp_size_bytes,
                                  &n_atoms) != 0 ||
      quadrupole_len < 6u) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }
  (void)n_atoms;

  NWChemCSession *session =
      nwchemc_session_create(params_capnp, params_capnp_size_bytes);
  if (!session) {
    snprintf(r.message, sizeof(r.message), "embed config failed");
    return r;
  }
  r = nwchemc_session_calculate_quadrupole(
      session, force_input_capnp, force_input_capnp_size_bytes, quadrupole_au,
      quadrupole_len);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_calculate_optimize(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *optimized_positions_ang, size_t optimized_positions_len) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!params_capnp || params_capnp_size_bytes == 0 || !force_input_capnp ||
      force_input_capnp_size_bytes == 0 || !optimized_positions_ang) {
    snprintf(r.message, sizeof(r.message), "invalid arguments");
    return r;
  }
  size_t n_atoms = 0;
  if (force_input_step_atom_count(force_input_capnp,
                                  force_input_capnp_size_bytes,
                                  &n_atoms) != 0 ||
      n_atoms > SIZE_MAX / 3u || optimized_positions_len < n_atoms * 3u) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }

  NWChemCSession *session =
      nwchemc_session_create(params_capnp, params_capnp_size_bytes);
  if (!session) {
    snprintf(r.message, sizeof(r.message), "embed config failed");
    return r;
  }
  r = nwchemc_session_calculate_optimize(
      session, force_input_capnp, force_input_capnp_size_bytes,
      optimized_positions_ang, optimized_positions_len);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_calculate_frequencies(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *frequencies_cm1, size_t frequencies_len, double *intensities_au,
    size_t intensities_len) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!params_capnp || params_capnp_size_bytes == 0 || !force_input_capnp ||
      force_input_capnp_size_bytes == 0 || !frequencies_cm1) {
    snprintf(r.message, sizeof(r.message), "invalid arguments");
    return r;
  }
  size_t n_atoms = 0;
  if (force_input_step_atom_count(force_input_capnp,
                                  force_input_capnp_size_bytes,
                                  &n_atoms) != 0 ||
      n_atoms > SIZE_MAX / 3u || frequencies_len < n_atoms * 3u ||
      (intensities_au && intensities_len < n_atoms * 3u)) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }
  NWChemCSession *session =
      nwchemc_session_create(params_capnp, params_capnp_size_bytes);
  if (!session) {
    snprintf(r.message, sizeof(r.message), "invalid NWChemParams message");
    return r;
  }
  r = nwchemc_session_calculate_frequencies(
      session, force_input_capnp, force_input_capnp_size_bytes, frequencies_cm1,
      frequencies_len, intensities_au, intensities_len);
  nwchemc_session_destroy(session);
  return r;
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

NWChemCResult nwchemc_quadrupole(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *quadrupole_au) {
  (void)n_atoms;
  (void)positions_ang;
  (void)atomic_numbers;
  (void)params_capnp;
  (void)params_capnp_size_bytes;
  (void)quadrupole_au;
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  snprintf(r.message, sizeof(r.message), "compiled without NWCHEMC_HAS_NWCHEM");
  return r;
}

NWChemCResult nwchemc_optimize(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *optimized_positions_ang) {
  (void)n_atoms;
  (void)positions_ang;
  (void)atomic_numbers;
  (void)params_capnp;
  (void)params_capnp_size_bytes;
  (void)optimized_positions_ang;
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  snprintf(r.message, sizeof(r.message), "compiled without NWCHEMC_HAS_NWCHEM");
  return r;
}

NWChemCResult nwchemc_frequencies(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *frequencies_cm1, double *intensities_au) {
  (void)n_atoms;
  (void)positions_ang;
  (void)atomic_numbers;
  (void)params_capnp;
  (void)params_capnp_size_bytes;
  (void)frequencies_cm1;
  (void)intensities_au;
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

NWChemCResult nwchemc_dipole(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *dipole_au) {
  (void)n_atoms;
  (void)positions_ang;
  (void)atomic_numbers;
  (void)params_capnp;
  (void)params_capnp_size_bytes;
  (void)dipole_au;
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

NWChemCResult nwchemc_session_dipole(NWChemCSession *session, int n_atoms,
                                     const double *positions_ang,
                                     const int *atomic_numbers,
                                     double *dipole_au) {
  (void)session;
  (void)n_atoms;
  (void)positions_ang;
  (void)atomic_numbers;
  (void)dipole_au;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_session_quadrupole(NWChemCSession *session, int n_atoms,
                                         const double *positions_ang,
                                         const int *atomic_numbers,
                                         double *quadrupole_au) {
  (void)session;
  (void)n_atoms;
  (void)positions_ang;
  (void)atomic_numbers;
  (void)quadrupole_au;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_session_optimize(NWChemCSession *session, int n_atoms,
                                       const double *positions_ang,
                                       const int *atomic_numbers,
                                       double *optimized_positions_ang) {
  (void)session;
  (void)n_atoms;
  (void)positions_ang;
  (void)atomic_numbers;
  (void)optimized_positions_ang;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_session_frequencies(
    NWChemCSession *session, int n_atoms, const double *positions_ang,
    const int *atomic_numbers, double *frequencies_cm1,
    double *intensities_au) {
  (void)session;
  (void)n_atoms;
  (void)positions_ang;
  (void)atomic_numbers;
  (void)frequencies_cm1;
  (void)intensities_au;
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

NWChemCResult nwchemc_calculate_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) {
  (void)params_capnp;
  (void)params_capnp_size_bytes;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)potential_result_capnp;
  (void)potential_result_capnp_capacity_bytes;
  (void)potential_result_capnp_size_bytes;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_calculate_hessian(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *hessian_h_bohr2, size_t hessian_len) {
  (void)params_capnp;
  (void)params_capnp_size_bytes;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)hessian_h_bohr2;
  (void)hessian_len;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_calculate_dipole(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *dipole_au, size_t dipole_len) {
  (void)params_capnp;
  (void)params_capnp_size_bytes;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)dipole_au;
  (void)dipole_len;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_calculate_quadrupole(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *quadrupole_au, size_t quadrupole_len) {
  (void)params_capnp;
  (void)params_capnp_size_bytes;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)quadrupole_au;
  (void)quadrupole_len;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_calculate_optimize(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *optimized_positions_ang, size_t optimized_positions_len) {
  (void)params_capnp;
  (void)params_capnp_size_bytes;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)optimized_positions_ang;
  (void)optimized_positions_len;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_calculate_frequencies(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *frequencies_cm1, size_t frequencies_len, double *intensities_au,
    size_t intensities_len) {
  (void)params_capnp;
  (void)params_capnp_size_bytes;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)frequencies_cm1;
  (void)frequencies_len;
  (void)intensities_au;
  (void)intensities_len;
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

NWChemCResult nwchemc_session_calculate_dipole(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *dipole_au,
    size_t dipole_len) {
  (void)session;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)dipole_au;
  (void)dipole_len;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_session_calculate_quadrupole(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *quadrupole_au,
    size_t quadrupole_len) {
  (void)session;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)quadrupole_au;
  (void)quadrupole_len;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_session_calculate_optimize(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *optimized_positions_ang,
    size_t optimized_positions_len) {
  (void)session;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)optimized_positions_ang;
  (void)optimized_positions_len;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_session_calculate_frequencies(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *frequencies_cm1,
    size_t frequencies_len, double *intensities_au, size_t intensities_len) {
  (void)session;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)frequencies_cm1;
  (void)frequencies_len;
  (void)intensities_au;
  (void)intensities_len;
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
