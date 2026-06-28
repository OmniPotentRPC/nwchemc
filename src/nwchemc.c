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
extern int nwchemc_embed_last_energy(double *energy_h);
extern int nwchemc_embed_reset_rtdb(void);
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
extern int nwchemc_embed_set_brillouin_dos_zones(const char *zone_names,
                                                const int *zone_grids,
                                                int count);
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
extern int nwchemc_embed_energy_only(const int *n_atoms,
                                     const double *positions_ang,
                                     const int *atomic_numbers,
                                     const int *charge,
                                     const int *multiplicity,
                                     double *energy_h, char *errmsg,
                                     int errmsg_len);
extern int nwchemc_embed_energy_only_cell(
    const int *n_atoms, const double *positions_ang, const int *atomic_numbers,
    const double *cell_ang, const int *has_cell, const int *charge,
    const int *multiplicity, double *energy_h, char *errmsg, int errmsg_len);
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
extern int nwchemc_embed_polarizability(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const int *charge, const int *multiplicity,
    double *energy_h, double *polarizability_au, char *errmsg,
    int errmsg_len);
extern int nwchemc_embed_polarizability_cell(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const double *cell_ang, const int *has_cell,
    const int *charge, const int *multiplicity, double *energy_h,
    double *polarizability_au, char *errmsg, int errmsg_len);
extern int nwchemc_embed_quadrupole(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const int *charge, const int *multiplicity,
    double *energy_h, double *quadrupole_au, char *errmsg, int errmsg_len);
extern int nwchemc_embed_quadrupole_cell(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const double *cell_ang, const int *has_cell,
    const int *charge, const int *multiplicity, double *energy_h,
    double *quadrupole_au, char *errmsg, int errmsg_len);
extern int nwchemc_embed_stress(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const int *charge, const int *multiplicity,
    double *energy_h, double *stress_au, char *errmsg, int errmsg_len);
extern int nwchemc_embed_stress_cell(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const double *cell_ang, const int *has_cell,
    const int *charge, const int *multiplicity, double *energy_h,
    double *stress_au, char *errmsg, int errmsg_len);
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
extern int nwchemc_embed_frequencies_modes(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const int *charge, const int *multiplicity,
    double *frequencies_cm1, double *intensities_au, double *normal_modes,
    char *errmsg, int errmsg_len);
extern int nwchemc_embed_frequencies_modes_cell(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const double *cell_ang, const int *has_cell,
    const int *charge, const int *multiplicity, double *frequencies_cm1,
    double *intensities_au, double *normal_modes, char *errmsg,
    int errmsg_len);
extern int nwchemc_embed_frequencies_detail_cell(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const double *cell_ang, const int *has_cell,
    const int *charge, const int *multiplicity, double *frequencies_cm1,
    double *intensities_au, double *normal_modes,
    double *projected_frequencies_cm1, double *projected_intensities_au,
    double *thermochemistry,
    char *errmsg, int errmsg_len);
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
  int step_state_override;
  int step_charge;
  int step_multiplicity;
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
  NWCHEMC_DIRECT_SET_MAX = 512,
  NWCHEMC_DIRECT_SET_VALUE_MAX = 64,
  NWCHEMC_DIRECT_SET_KEY_LEN = 128,
  NWCHEMC_DIRECT_SET_VALUE_LEN = 256,
  NWCHEMC_DIRECT_DOS_ZONE_NAME_LEN = 64,
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

static int append_direct_integer_values(
    capn_text *keys, int *value_types, int *value_counts, capn_text *values,
    size_t key_capacity, size_t value_capacity, size_t *count,
    char key_storage[][NWCHEMC_DIRECT_SET_KEY_LEN],
    char value_storage[][NWCHEMC_DIRECT_SET_VALUE_MAX]
                      [NWCHEMC_DIRECT_SET_VALUE_LEN],
    const char *key, const int *integer_values, size_t nvalues) {
  if (!integer_values || nvalues == 0 ||
      nvalues > NWCHEMC_DIRECT_SET_VALUE_MAX)
    return -1;
  char owned_values[NWCHEMC_DIRECT_SET_VALUE_MAX]
                   [NWCHEMC_DIRECT_SET_VALUE_LEN];
  const char *value_list[NWCHEMC_DIRECT_SET_VALUE_MAX];
  for (size_t i = 0; i < nvalues; ++i) {
    int n = snprintf(owned_values[i], sizeof(owned_values[i]), "%d",
                     integer_values[i]);
    if (n < 0 || (size_t)n >= sizeof(owned_values[i]))
      return -1;
    value_list[i] = owned_values[i];
  }
  return append_direct_typed_values(keys, value_types, value_counts, values,
                                    key_capacity, value_capacity, count,
                                    key_storage, value_storage, key,
                                    NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                                    value_list, (int)nvalues);
}

static int append_direct_double_values(
    capn_text *keys, int *value_types, int *value_counts, capn_text *values,
    size_t key_capacity, size_t value_capacity, size_t *count,
    char key_storage[][NWCHEMC_DIRECT_SET_KEY_LEN],
    char value_storage[][NWCHEMC_DIRECT_SET_VALUE_MAX]
                      [NWCHEMC_DIRECT_SET_VALUE_LEN],
    const char *key, const double *double_values, size_t nvalues) {
  if (!double_values || nvalues == 0 || nvalues > NWCHEMC_DIRECT_SET_VALUE_MAX)
    return -1;
  char owned_values[NWCHEMC_DIRECT_SET_VALUE_MAX]
                   [NWCHEMC_DIRECT_SET_VALUE_LEN];
  const char *value_list[NWCHEMC_DIRECT_SET_VALUE_MAX];
  for (size_t i = 0; i < nvalues; ++i) {
    int n = snprintf(owned_values[i], sizeof(owned_values[i]), "%.17g",
                     double_values[i]);
    if (n < 0 || (size_t)n >= sizeof(owned_values[i]))
      return -1;
    value_list[i] = owned_values[i];
  }
  return append_direct_typed_values(keys, value_types, value_counts, values,
                                    key_capacity, value_capacity, count,
                                    key_storage, value_storage, key,
                                    NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                                    value_list, (int)nvalues);
}

static int append_direct_logical_value(
    capn_text *keys, int *value_types, int *value_counts, capn_text *values,
    size_t key_capacity, size_t value_capacity, size_t *count,
    char key_storage[][NWCHEMC_DIRECT_SET_KEY_LEN],
    char value_storage[][NWCHEMC_DIRECT_SET_VALUE_MAX]
                      [NWCHEMC_DIRECT_SET_VALUE_LEN],
    const char *key, int enabled) {
  const char *value_list[1] = {enabled ? "true" : "false"};
  return append_direct_typed_values(keys, value_types, value_counts, values,
                                    key_capacity, value_capacity, count,
                                    key_storage, value_storage, key,
                                    NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                                    value_list, 1);
}

static int append_direct_string_value(capn_text *keys, capn_text *values,
                                      size_t capacity, size_t *count,
                                      const char *key, capn_text value) {
  if (!keys || !values || !count || !key)
    return -1;
  if (value.len <= 0)
    return 0;
  if (*count >= capacity)
    return -1;
  keys[*count] = text_from_cstr(key);
  values[*count] = value;
  ++*count;
  return 0;
}

static int append_owned_direct_string_value(
    capn_text *keys, capn_text *values, size_t capacity, size_t *count,
    char key_storage[][NWCHEMC_DIRECT_SET_KEY_LEN],
    char value_storage[][NWCHEMC_DIRECT_SET_VALUE_LEN], const char *key,
    const char *value) {
  if (!keys || !values || !count || !key_storage || !value_storage || !key ||
      !value)
    return -1;
  if (*count >= capacity)
    return -1;
  int nkey = snprintf(key_storage[*count], NWCHEMC_DIRECT_SET_KEY_LEN, "%s",
                      key);
  if (nkey < 0 || (size_t)nkey >= NWCHEMC_DIRECT_SET_KEY_LEN)
    return -1;
  int nvalue = snprintf(value_storage[*count], NWCHEMC_DIRECT_SET_VALUE_LEN,
                        "%s", value);
  if (nvalue < 0 || (size_t)nvalue >= NWCHEMC_DIRECT_SET_VALUE_LEN)
    return -1;
  keys[*count] = text_from_cstr(key_storage[*count]);
  values[*count] = text_from_cstr(value_storage[*count]);
  ++*count;
  return 0;
}

static int append_brillouin_zone_alias_strings(NWChemParams_ptr params,
                                               capn_text *keys,
                                               capn_text *values,
                                               size_t capacity,
                                               size_t *count) {
  if (params.p.type == CAPN_NULL || !keys || !values || !count)
    return -1;

  struct NWChemParams view;
  read_NWChemParams(&view, params);
  int n = direct_struct_list_len(&view.inputStanzas.p);
  if (n < 0)
    return -1;

  for (int i = 0; i < n; ++i) {
    struct NWChemInputStanza stanza;
    get_NWChemInputStanza(&stanza, view.inputStanzas, i);
    if (stanza.kind != NWChemInputStanza_Kind_brillouinZone ||
        stanza.brillouinZone.p.type == CAPN_NULL)
      continue;

    struct NWChemBrillouinZoneStanza zone;
    read_NWChemBrillouinZoneStanza(&zone, stanza.brillouinZone);
    if (append_direct_string_value(keys, values, capacity, count,
                                   "band_structure:zone_name",
                                   zone.zoneStructureName) != 0)
      return -1;
    if (append_direct_string_value(keys, values, capacity, count,
                                   "band_fft:zone_name",
                                   zone.zoneFftName) != 0)
      return -1;
  }

  return 0;
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

static int compact_lattice_unita(enum NWChemSimulationCellLatticeKind kind,
                                 double length_bohr, double unita[9]) {
  if (!unita || length_bohr <= 0.0)
    return 0;

  double half = 0.5 * length_bohr;
  for (int i = 0; i < 9; ++i)
    unita[i] = 0.0;

  switch (kind) {
  case NWChemSimulationCellLatticeKind_sc:
    unita[0] = length_bohr;
    unita[4] = length_bohr;
    unita[8] = length_bohr;
    return 1;
  case NWChemSimulationCellLatticeKind_fcc:
    unita[0] = half;
    unita[1] = half;
    unita[3] = half;
    unita[5] = half;
    unita[7] = half;
    unita[8] = half;
    return 1;
  case NWChemSimulationCellLatticeKind_bcc:
    unita[0] = -half;
    unita[1] = half;
    unita[2] = half;
    unita[3] = half;
    unita[4] = -half;
    unita[5] = half;
    unita[6] = half;
    unita[7] = half;
    unita[8] = -half;
    return 1;
  case NWChemSimulationCellLatticeKind_unspecified:
  default:
    return 0;
  }
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

static const char *nwpw_toggle_logical_value(enum NWChemNwpwToggle toggle) {
  switch (toggle) {
  case NWChemNwpwToggle_enabled:
    return "true";
  case NWChemNwpwToggle_disabled:
    return "false";
  case NWChemNwpwToggle_unspecified:
  default:
    return NULL;
  }
}

static int nwpw_minimizer_rtdb_value(enum NWChemNwpwMinimizer minimizer) {
  switch (minimizer) {
  case NWChemNwpwMinimizer_cgGrassman:
    return 1;
  case NWChemNwpwMinimizer_cgStiefel:
    return 4;
  case NWChemNwpwMinimizer_cgStich:
    return 9;
  case NWChemNwpwMinimizer_lmbfgsGrassman:
    return 2;
  case NWChemNwpwMinimizer_lmbfgsStiefel:
    return 7;
  case NWChemNwpwMinimizer_lmbfgsStich:
    return 10;
  case NWChemNwpwMinimizer_scfDensity:
    return 8;
  case NWChemNwpwMinimizer_scfPotential:
    return 5;
  case NWChemNwpwMinimizer_unspecified:
  default:
    return 0;
  }
}

static int nwpw_ks_algorithm_rtdb_value(enum NWChemNwpwKsAlgorithm algorithm,
                                        int *value) {
  if (!value)
    return -1;
  switch (algorithm) {
  case NWChemNwpwKsAlgorithm_blockCg:
    *value = -1;
    return 1;
  case NWChemNwpwKsAlgorithm_cg:
    *value = 0;
    return 1;
  case NWChemNwpwKsAlgorithm_rmmDiis:
    *value = 1;
    return 1;
  case NWChemNwpwKsAlgorithm_unspecified:
  default:
    return 0;
  }
}

static int
nwpw_scf_algorithm_rtdb_value(enum NWChemNwpwScfAlgorithm algorithm,
                              int *value) {
  if (!value)
    return -1;
  switch (algorithm) {
  case NWChemNwpwScfAlgorithm_simple:
    *value = 0;
    return 1;
  case NWChemNwpwScfAlgorithm_broyden:
    *value = 1;
    return 1;
  case NWChemNwpwScfAlgorithm_diis:
    *value = 2;
    return 1;
  case NWChemNwpwScfAlgorithm_anderson:
    *value = 3;
    return 1;
  case NWChemNwpwScfAlgorithm_unspecified:
  default:
    return 0;
  }
}

static int nwpw_efield_type_rtdb_value(enum NWChemNwpwEfieldType efield_type,
                                       int *value) {
  if (!value)
    return -1;
  switch (efield_type) {
  case NWChemNwpwEfieldType_periodic:
    *value = 0;
    return 1;
  case NWChemNwpwEfieldType_apc:
    *value = 1;
    return 1;
  case NWChemNwpwEfieldType_rgrid:
    *value = 2;
    return 1;
  case NWChemNwpwEfieldType_unspecified:
  default:
    *value = 0;
    return 0;
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

static int uterm_angular_momentum_value(
    enum NWChemPseudopotentialUtermRule_AngularMomentum angular_momentum,
    int *value) {
  if (!value)
    return -1;
  switch (angular_momentum) {
  case NWChemPseudopotentialUtermRule_AngularMomentum_p:
    *value = 1;
    return 0;
  case NWChemPseudopotentialUtermRule_AngularMomentum_d:
    *value = 2;
    return 0;
  case NWChemPseudopotentialUtermRule_AngularMomentum_f:
    *value = 3;
    return 0;
  case NWChemPseudopotentialUtermRule_AngularMomentum_s:
  default:
    *value = 0;
    return 0;
  }
}

static int append_direct_uterm_rule(
    void *user_data, size_t rule_index,
    const struct NWChemPseudopotentialUtermRule *rule) {
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

  char key[NWCHEMC_DIRECT_SET_KEY_LEN];
  char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
  snprintf(value, sizeof(value), "%.15g", rule->uScale);
  int n = snprintf(key, sizeof(key), "nwpw:uterm_scale:%s", index_name);
  if (n < 0 || (size_t)n >= sizeof(key))
    return -1;
  if (append_direct_typed_value(
          state->keys, state->value_types, state->value_counts, state->values,
          state->key_capacity, state->value_capacity, state->count,
          state->key_storage, state->value_storage, key,
          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, value) != 0)
    return -1;

  snprintf(value, sizeof(value), "%.15g", rule->jScale);
  n = snprintf(key, sizeof(key), "nwpw:jterm_scale:%s", index_name);
  if (n < 0 || (size_t)n >= sizeof(key))
    return -1;
  if (append_direct_typed_value(
          state->keys, state->value_types, state->value_counts, state->values,
          state->key_capacity, state->value_capacity, state->count,
          state->key_storage, state->value_storage, key,
          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, value) != 0)
    return -1;

  int angular_momentum = 0;
  if (uterm_angular_momentum_value(rule->angularMomentum,
                                   &angular_momentum) != 0)
    return -1;
  snprintf(value, sizeof(value), "%d", angular_momentum);
  n = snprintf(key, sizeof(key), "nwpw:uterm_l:%s", index_name);
  if (n < 0 || (size_t)n >= sizeof(key))
    return -1;
  if (append_direct_typed_value(
          state->keys, state->value_types, state->value_counts, state->values,
          state->key_capacity, state->value_capacity, state->count,
          state->key_storage, state->value_storage, key,
          NWCHEMC_DIRECT_SET_VALUE_INTEGER, value) != 0)
    return -1;

  char ion_values[NWCHEMC_DIRECT_SET_VALUE_MAX][NWCHEMC_DIRECT_SET_VALUE_LEN];
  const char *ion_value_list[NWCHEMC_DIRECT_SET_VALUE_MAX];
  for (int i = 0; i < nions; ++i) {
    int ion_index = (int)(int32_t)capn_get32(ion_indices, i);
    snprintf(ion_values[i], sizeof(ion_values[i]), "%d", ion_index);
    ion_value_list[i] = ion_values[i];
  }
  n = snprintf(key, sizeof(key), "nwpw:uterm_ions:%s", index_name);
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
    double compact_unita[9];
    int has_compact_unita = compact_lattice_unita(
        cell.latticeKind, cell.latticeLengthBohr, compact_unita);
    if (cell.boundaryConditions.len > 0) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      copy_text_record(value, sizeof(value), cell.boundaryConditions);
      if (append_simulation_cell_direct_scalar(
              keys, value_types, value_counts, values, key_capacity,
              value_capacity, count, key_storage, value_storage, cell.cellName,
              "boundry", NWCHEMC_DIRECT_SET_VALUE_TEXT, value) != 0)
        return -1;
    }
    if (nvector == 9 && !has_compact_unita) {
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
    if (has_compact_unita) {
      char value_text[9][NWCHEMC_DIRECT_SET_VALUE_LEN];
      const char *value_list[9];
      for (int j = 0; j < 9; ++j) {
        snprintf(value_text[j], sizeof(value_text[j]), "%.15g",
                 compact_unita[j]);
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

/* Hot multi-call path: SocketNWChem reuses one NWChem process across POSDATA.
 * Stateless energy_gradient used to call apply_config_to_embed (reset RTDB +
 * full method setup) on *every* force, which destroys warm SCF and makes the
 * in-process path slower than the socket. Cache last applied params blob and
 * skip re-apply when unchanged (optimize steps / NEB images share method). */
static unsigned char *g_applied_params_blob = NULL;
static size_t g_applied_params_size = 0;
static int g_cached_charge = 0;
static int g_cached_mult = 1;

static int params_blob_matches_applied(const void *params_capnp,
                                       size_t params_capnp_size_bytes) {
  if (!params_capnp || params_capnp_size_bytes == 0 || !g_applied_params_blob ||
      g_applied_params_size != params_capnp_size_bytes)
    return 0;
  return memcmp(g_applied_params_blob, params_capnp, params_capnp_size_bytes) ==
         0;
}

static void remember_applied_params_blob(const void *params_capnp,
                                         size_t params_capnp_size_bytes,
                                         int charge, int mult) {
  if (!params_capnp || params_capnp_size_bytes == 0)
    return;
  if (g_applied_params_size != params_capnp_size_bytes ||
      !g_applied_params_blob) {
    free(g_applied_params_blob);
    g_applied_params_blob =
        (unsigned char *)malloc(params_capnp_size_bytes);
    g_applied_params_size =
        g_applied_params_blob ? params_capnp_size_bytes : 0;
  }
  if (g_applied_params_blob) {
    memcpy(g_applied_params_blob, params_capnp, params_capnp_size_bytes);
    g_applied_params_size = params_capnp_size_bytes;
  }
  g_cached_charge = charge;
  g_cached_mult = mult > 0 ? mult : 1;
}

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
  /* Cap'n Proto owns theory and scfType as separate fields; never rewrite. */
  const char *theory = text_or_with_len(params->theory, "scf", theory_len);
  *scf_type = text_or_with_len(params->scfType, "rhf", scf_len);
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
  if (params && params->enginePath.len > 0)
    return -1;

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
  int dft_iterations = 0;
  if (nwchemc_params_extract_direct_dft(params_root, &dft_xc, &dft_direct,
                                        &dft_smear_on, &dft_smear_sigma,
                                        &dft_smear_spinset,
                                        &dft_iterations) != 0)
    return -1;
  int scf_has_options = 0;
  int scf_maxiter = 0;
  double scf_thresh = 0.0;
  double scf_tol2e = 0.0;
  capn_text scf_wavefunction = {0};
  int scf_nopen = -1;
  int scf_has_nopen = 0;
  if (nwchemc_params_extract_direct_scf(params_root, &scf_has_options,
                                        &scf_maxiter, &scf_thresh, &scf_tol2e,
                                        &scf_wavefunction, &scf_nopen,
                                        &scf_has_nopen) != 0)
    return -1;
  capn_text scf_vectors_in = {0};
  capn_text scf_vectors_out = {0};
  int scf_diis = NWChemToggle_unspecified;
  int scf_diis_bas = 0;
  int scf_maxsub = 0;
  int scf_lock = NWChemToggle_unspecified;
  int scf_adapt = NWChemToggle_unspecified;
  int scf_noscf = NWChemToggle_unspecified;
  int scf_semidirect_filesize = 0;
  int scf_semidirect_memsize = 0;
  if (nwchemc_params_extract_direct_scf_extended(
          params_root, &scf_vectors_in, &scf_vectors_out, &scf_diis,
          &scf_diis_bas, &scf_maxsub, &scf_lock, &scf_adapt, &scf_noscf,
          &scf_semidirect_filesize, &scf_semidirect_memsize) != 0)
    return -1;
  double dft_energy_conv = 0.0;
  double dft_density_conv = 0.0;
  double dft_gradient_conv = 0.0;
  int dft_odft = 0;
  int dft_diis = NWChemToggle_unspecified;
  int dft_nfock = 0;
  double dft_level_shift = 0.0;
  capn_text dft_vectors_in = {0};
  capn_text dft_vectors_out = {0};
  if (nwchemc_params_extract_direct_dft_extended(
          params_root, &dft_energy_conv, &dft_density_conv, &dft_gradient_conv,
          &dft_odft, &dft_diis, &dft_nfock, &dft_level_shift, &dft_vectors_in,
          &dft_vectors_out) != 0)
    return -1;
  int prop_dipole = 0, prop_mulliken = 0, prop_quad = 0, prop_oct = 0;
  int prop_esp = 0, prop_efield = 0, prop_efield_grad = 0, prop_edens = 0;
  int prop_sdens = 0, prop_spop = 0, prop_shield = 0, prop_hyp = 0, prop_pol = 0;
  if (nwchemc_params_extract_direct_property(
          params_root, &prop_dipole, &prop_mulliken, &prop_quad, &prop_oct,
          &prop_esp, &prop_efield, &prop_efield_grad, &prop_edens, &prop_sdens,
          &prop_spop, &prop_shield, &prop_hyp, &prop_pol) != 0)
    return -1;
  int mp2_freeze_core = 0, mp2_freeze_virt = 0, mp2_tight = 0;
  double mp2_aotol2e = 0.0, mp2_aotol2e_fock = 0.0, mp2_backtol = 0.0;
  double mp2_fss = 0.0, mp2_fos = 0.0, mp2_scratch = 0.0;
  int mp2_scs = NWChemToggle_unspecified;
  if (nwchemc_params_extract_direct_mp2(
          params_root, &mp2_freeze_core, &mp2_freeze_virt, &mp2_tight,
          &mp2_aotol2e, &mp2_aotol2e_fock, &mp2_backtol, &mp2_fss, &mp2_fos,
          &mp2_scs, &mp2_scratch) != 0)
    return -1;
  int tddft_nroots = 0, tddft_tda = NWChemToggle_unspecified, tddft_maxiter = 0;
  double tddft_thresh = 0.0;
  int tddft_maxvecs = 0, tddft_singlet = NWChemToggle_unspecified;
  int tddft_triplet = NWChemToggle_unspecified, tddft_target = 0;
  capn_text tddft_target_sym = {0};
  int tddft_symmetry = NWChemToggle_unspecified, tddft_algorithm = 0;
  double tddft_ecut = 0.0;
  if (nwchemc_params_extract_direct_tddft(
          params_root, &tddft_nroots, &tddft_tda, &tddft_maxiter, &tddft_thresh,
          &tddft_maxvecs, &tddft_singlet, &tddft_triplet, &tddft_target,
          &tddft_target_sym, &tddft_symmetry, &tddft_algorithm,
          &tddft_ecut) != 0)
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
  int nwpw_scaling_atoms[NWCHEMC_DIRECT_SET_VALUE_MAX] = {0};
  size_t nwpw_scaling_atom_count = 0;
  if (nwchemc_params_extract_direct_nwpw_bo_with_scaling_atoms(
          params_root, &nwpw_bo_has_options, &nwpw_balance_mode,
          &nwpw_bo_step_start, &nwpw_bo_step_end, &nwpw_bo_time_step,
          &nwpw_bo_algorithm, &nwpw_bo_fake_mass, &nwpw_has_scaling,
          &nwpw_scaling_first, &nwpw_scaling_second, nwpw_scaling_atoms,
          NWCHEMC_DIRECT_SET_VALUE_MAX, &nwpw_scaling_atom_count) != 0)
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
  int nwpw_occupations_has_options = 0;
  double nwpw_occupations[NWCHEMC_DIRECT_SET_VALUE_MAX] = {0.0};
  int nwpw_occupation_states[NWCHEMC_DIRECT_SET_VALUE_MAX] = {0};
  size_t nwpw_occupation_count = 0;
  int nwpw_extra_orbitals = 0;
  if (nwchemc_params_extract_direct_nwpw_occupations(
          params_root, &nwpw_occupations_has_options, nwpw_occupations,
          nwpw_occupation_states, NWCHEMC_DIRECT_SET_VALUE_MAX,
          &nwpw_occupation_count, &nwpw_extra_orbitals) != 0)
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
  int nwpw_lcao_mask_has_options = 0;
  int nwpw_lcao_mask = NWChemNwpwToggle_unspecified;
  int nwpw_lcao_mask_up_orbitals[NWCHEMC_DIRECT_SET_VALUE_MAX] = {0};
  int nwpw_lcao_mask_down_orbitals[NWCHEMC_DIRECT_SET_VALUE_MAX] = {0};
  size_t nwpw_lcao_mask_up_count = 0;
  size_t nwpw_lcao_mask_down_count = 0;
  if (nwchemc_params_extract_direct_nwpw_lcao_mask(
          params_root, &nwpw_lcao_mask_has_options, &nwpw_lcao_mask,
          nwpw_lcao_mask_up_orbitals, NWCHEMC_DIRECT_SET_VALUE_MAX,
          &nwpw_lcao_mask_up_count, nwpw_lcao_mask_down_orbitals,
          NWCHEMC_DIRECT_SET_VALUE_MAX, &nwpw_lcao_mask_down_count) != 0)
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
  int nwpw_electric_field_has_options = 0;
  int nwpw_atom_efield = NWChemNwpwToggle_unspecified;
  int nwpw_atom_efield_gradient = NWChemNwpwToggle_unspecified;
  if (nwchemc_params_extract_direct_nwpw_electric_field(
          params_root, &nwpw_electric_field_has_options, &nwpw_atom_efield,
          &nwpw_atom_efield_gradient) != 0)
    return -1;
  int nwpw_mulliken_has_options = 0;
  int nwpw_mulliken = NWChemNwpwToggle_unspecified;
  int nwpw_mulliken_kawai = NWChemNwpwToggle_unspecified;
  if (nwchemc_params_extract_direct_nwpw_mulliken(
          params_root, &nwpw_mulliken_has_options, &nwpw_mulliken,
          &nwpw_mulliken_kawai) != 0)
    return -1;
  int nwpw_periodic_dipole_has_options = 0;
  int nwpw_periodic_dipole = NWChemNwpwToggle_unspecified;
  if (nwchemc_params_extract_direct_nwpw_periodic_dipole(
          params_root, &nwpw_periodic_dipole_has_options,
          &nwpw_periodic_dipole) != 0)
    return -1;
  int nwpw_efield_has_options = 0;
  int nwpw_efield = NWChemNwpwToggle_unspecified;
  double nwpw_efield_vector[3] = {0.0, 0.0, 0.0};
  int nwpw_efield_has_center = 0;
  double nwpw_efield_center[3] = {0.0, 0.0, 0.0};
  int nwpw_efield_type = NWChemNwpwEfieldType_unspecified;
  if (nwchemc_params_extract_direct_nwpw_efield(
          params_root, &nwpw_efield_has_options, &nwpw_efield,
          nwpw_efield_vector, &nwpw_efield_has_center, nwpw_efield_center,
          &nwpw_efield_type) != 0)
    return -1;
  int nwpw_smooth_cutoff_has_options = 0;
  int nwpw_smooth_cutoff = NWChemNwpwToggle_unspecified;
  double nwpw_smooth_cutoff_values[2] = {0.0, 0.0};
  if (nwchemc_params_extract_direct_nwpw_smooth_cutoff(
          params_root, &nwpw_smooth_cutoff_has_options, &nwpw_smooth_cutoff,
          nwpw_smooth_cutoff_values) != 0)
    return -1;
  int nwpw_cutoff_boot_wavefunction_has_options = 0;
  int nwpw_cutoff_boot_wavefunction = NWChemNwpwToggle_unspecified;
  if (nwchemc_params_extract_direct_nwpw_cutoff_boot_wavefunction(
          params_root, &nwpw_cutoff_boot_wavefunction_has_options,
          &nwpw_cutoff_boot_wavefunction) != 0)
    return -1;
  int nwpw_fast_erf_has_options = 0;
  int nwpw_fast_erf = NWChemNwpwToggle_unspecified;
  if (nwchemc_params_extract_direct_nwpw_fast_erf(
          params_root, &nwpw_fast_erf_has_options, &nwpw_fast_erf) != 0)
    return -1;
  int nwpw_dipole_motion_has_options = 0;
  int nwpw_dipole_motion = NWChemNwpwToggle_unspecified;
  capn_text nwpw_dipole_motion_filename = {0};
  if (nwchemc_params_extract_direct_nwpw_dipole_motion(
          params_root, &nwpw_dipole_motion_has_options, &nwpw_dipole_motion,
          &nwpw_dipole_motion_filename) != 0)
    return -1;
  int nwpw_rho_use_symmetry_has_options = 0;
  int nwpw_rho_use_symmetry = NWChemNwpwToggle_unspecified;
  if (nwchemc_params_extract_direct_nwpw_rho_use_symmetry(
          params_root, &nwpw_rho_use_symmetry_has_options,
          &nwpw_rho_use_symmetry) != 0)
    return -1;
  int nwpw_one_electron_guess_has_options = 0;
  int nwpw_one_electron_guess_it_in = 0;
  int nwpw_one_electron_guess_it_out = 0;
  int nwpw_one_electron_guess_it_ortho = 0;
  if (nwchemc_params_extract_direct_nwpw_one_electron_guess(
          params_root, &nwpw_one_electron_guess_has_options,
          &nwpw_one_electron_guess_it_in, &nwpw_one_electron_guess_it_out,
          &nwpw_one_electron_guess_it_ortho) != 0)
    return -1;
  int nwpw_fmm_has_options = 0;
  int nwpw_fmm = NWChemNwpwToggle_unspecified;
  int nwpw_fmm_lmax = 0;
  int nwpw_fmm_long_range = 0;
  if (nwchemc_params_extract_direct_nwpw_fmm(
          params_root, &nwpw_fmm_has_options, &nwpw_fmm, &nwpw_fmm_lmax,
          &nwpw_fmm_long_range) != 0)
    return -1;
  int nwpw_born_has_options = 0;
  int nwpw_born = NWChemNwpwToggle_unspecified;
  double nwpw_born_dielectric = 0.0;
  int nwpw_born_relax = NWChemNwpwToggle_unspecified;
  double nwpw_born_vradii_angstrom[NWCHEMC_DIRECT_SET_VALUE_MAX] = {0.0};
  size_t nwpw_born_vradii_count = 0;
  if (nwchemc_params_extract_direct_nwpw_born(
          params_root, &nwpw_born_has_options, &nwpw_born,
          &nwpw_born_dielectric, &nwpw_born_relax,
          nwpw_born_vradii_angstrom, NWCHEMC_DIRECT_SET_VALUE_MAX,
          &nwpw_born_vradii_count) != 0)
    return -1;
  int nwpw_vfield_has_options = 0;
  capn_text nwpw_vfield_filenames[NWCHEMC_DIRECT_SET_VALUE_MAX];
  size_t nwpw_vfield_count = 0;
  if (nwchemc_params_extract_direct_nwpw_vfield(
          params_root, &nwpw_vfield_has_options, nwpw_vfield_filenames,
          NWCHEMC_DIRECT_SET_VALUE_MAX, &nwpw_vfield_count) != 0)
    return -1;
  int nwpw_single_precision_hfx_has_options = 0;
  int nwpw_single_precision_hfx = 0;
  if (nwchemc_params_extract_direct_nwpw_single_precision_hfx(
          params_root, &nwpw_single_precision_hfx_has_options,
          &nwpw_single_precision_hfx) != 0)
    return -1;
  int nwpw_geometry_optimize_has_options = 0;
  int nwpw_geometry_optimize = 0;
  if (nwchemc_params_extract_direct_nwpw_geometry_optimize(
          params_root, &nwpw_geometry_optimize_has_options,
          &nwpw_geometry_optimize) != 0)
    return -1;
  int nwpw_auxiliary_potentials_has_options = 0;
  int nwpw_auxiliary_potentials = 0;
  if (nwchemc_params_extract_direct_nwpw_auxiliary_potentials(
          params_root, &nwpw_auxiliary_potentials_has_options,
          &nwpw_auxiliary_potentials) != 0)
    return -1;
  int nwpw_allow_translation_has_options = 0;
  int nwpw_allow_translation = 0;
  if (nwchemc_params_extract_direct_nwpw_allow_translation(
          params_root, &nwpw_allow_translation_has_options,
          &nwpw_allow_translation) != 0)
    return -1;
  int nwpw_multiplicity_has_options = 0;
  int nwpw_multiplicity = 0;
  int nwpw_ispin = 0;
  if (nwchemc_params_extract_direct_nwpw_multiplicity(
          params_root, &nwpw_multiplicity_has_options, &nwpw_multiplicity,
          &nwpw_ispin) != 0)
    return -1;
  int nwpw_spin_ispins[NWCHEMC_DIRECT_SET_VALUE_MAX] = {0};
  size_t nwpw_spin_ispin_count = 0;
  if (nwchemc_params_extract_direct_nwpw_spin_ispins(
          params_root, nwpw_spin_ispins, NWCHEMC_DIRECT_SET_VALUE_MAX,
          &nwpw_spin_ispin_count) != 0)
    return -1;
  int nwpw_dos_has_options = 0;
  int nwpw_dos_alpha_set = 0;
  double nwpw_dos_alpha = 0.0;
  int nwpw_dos_npoints_set = 0;
  int nwpw_dos_npoints = 0;
  int nwpw_dos_emin_set = 0;
  double nwpw_dos_emin = 0.0;
  int nwpw_dos_emax_set = 0;
  double nwpw_dos_emax = 0.0;
  capn_text nwpw_dos_filename = {0};
  if (nwchemc_params_extract_direct_nwpw_dos(
          params_root, &nwpw_dos_has_options, &nwpw_dos_alpha_set,
          &nwpw_dos_alpha, &nwpw_dos_npoints_set, &nwpw_dos_npoints,
          &nwpw_dos_emin_set, &nwpw_dos_emin, &nwpw_dos_emax_set,
          &nwpw_dos_emax, &nwpw_dos_filename) != 0)
    return -1;
  int nwpw_cpmd_grid_has_options = 0;
  int nwpw_cpmd_properties = NWChemNwpwToggle_unspecified;
  int nwpw_use_grid_comparison = NWChemNwpwToggle_unspecified;
  if (nwchemc_params_extract_direct_nwpw_cpmd_grid(
          params_root, &nwpw_cpmd_grid_has_options, &nwpw_cpmd_properties,
          &nwpw_use_grid_comparison) != 0)
    return -1;
  int nwpw_director_has_options = 0;
  int nwpw_director = NWChemNwpwToggle_unspecified;
  capn_text nwpw_director_filename = {0};
  if (nwchemc_params_extract_direct_nwpw_director(
          params_root, &nwpw_director_has_options, &nwpw_director,
          &nwpw_director_filename) != 0)
    return -1;
  int nwpw_cell_mapping_has_options = 0;
  int nwpw_cell_expand[3] = {0, 0, 0};
  int nwpw_mapping = 0;
  if (nwchemc_params_extract_direct_nwpw_cell_mapping(
          params_root, &nwpw_cell_mapping_has_options, nwpw_cell_expand,
          &nwpw_mapping) != 0)
    return -1;
  int nwpw_rotation_multipole_has_options = 0;
  int nwpw_rotation = NWChemNwpwToggle_unspecified;
  int nwpw_lmax_multipole = -1;
  if (nwchemc_params_extract_direct_nwpw_rotation_multipole(
          params_root, &nwpw_rotation_multipole_has_options, &nwpw_rotation,
          &nwpw_lmax_multipole) != 0)
    return -1;
  int nwpw_fei_has_options = 0;
  int nwpw_fei = 0;
  capn_text nwpw_fei_filename = {0};
  if (nwchemc_params_extract_direct_nwpw_fei(
          params_root, &nwpw_fei_has_options, &nwpw_fei,
          &nwpw_fei_filename) != 0)
    return -1;
  int nwpw_et_has_options = 0;
  capn_text nwpw_et_movecs_a = {0};
  capn_text nwpw_et_movecs_b = {0};
  capn_text nwpw_et_ion_a = {0};
  capn_text nwpw_et_ion_b = {0};
  if (nwchemc_params_extract_direct_nwpw_et(
          params_root, &nwpw_et_has_options, &nwpw_et_movecs_a,
          &nwpw_et_movecs_b, &nwpw_et_ion_a, &nwpw_et_ion_b) != 0)
    return -1;
  int nwpw_initial_velocities_has_options = 0;
  double nwpw_initial_velocities_temperature = 0.0;
  int nwpw_initial_velocities_seed = 494;
  if (nwchemc_params_extract_direct_nwpw_initial_velocities(
          params_root, &nwpw_initial_velocities_has_options,
          &nwpw_initial_velocities_temperature,
          &nwpw_initial_velocities_seed) != 0)
    return -1;
  int nwpw_make_hmass2_has_options = 0;
  int nwpw_make_hmass2 = NWChemNwpwToggle_unspecified;
  if (nwchemc_params_extract_direct_nwpw_make_hmass2(
          params_root, &nwpw_make_hmass2_has_options,
          &nwpw_make_hmass2) != 0)
    return -1;
  int nwpw_translate_vector_has_options = 0;
  double nwpw_translate_vector[3] = {0.0, 0.0, 0.0};
  capn_text nwpw_translate_geometry_name = {0};
  int nwpw_translate_reorder = NWChemNwpwToggle_unspecified;
  if (nwchemc_params_extract_direct_nwpw_translate_vector(
          params_root, &nwpw_translate_vector_has_options,
          nwpw_translate_vector, &nwpw_translate_geometry_name,
          &nwpw_translate_reorder) != 0)
    return -1;
  int nwpw_socket_has_options = 0;
  capn_text nwpw_socket_type = {0};
  capn_text nwpw_socket_ip = {0};
  if (nwchemc_params_extract_direct_nwpw_socket(
          params_root, &nwpw_socket_has_options, &nwpw_socket_type,
          &nwpw_socket_ip) != 0)
    return -1;
  int nwpw_apc_has_options = 0;
  double nwpw_apc_gc = 0.0;
  double nwpw_apc_gamma[NWCHEMC_DIRECT_SET_VALUE_MAX] = {0.0};
  size_t nwpw_apc_gamma_count = 0;
  if (nwchemc_params_extract_direct_nwpw_apc(
          params_root, &nwpw_apc_has_options, &nwpw_apc_gc, nwpw_apc_gamma,
          NWCHEMC_DIRECT_SET_VALUE_MAX, &nwpw_apc_gamma_count) != 0)
    return -1;
  int nwpw_translation_has_options = 0;
  int nwpw_translation = NWChemNwpwToggle_unspecified;
  if (nwchemc_params_extract_direct_nwpw_translation(
          params_root, &nwpw_translation_has_options,
          &nwpw_translation) != 0)
    return -1;
  int nwpw_minimizer_has_options = 0;
  int nwpw_minimizer = NWChemNwpwMinimizer_unspecified;
  if (nwchemc_params_extract_direct_nwpw_minimizer(
          params_root, &nwpw_minimizer_has_options, &nwpw_minimizer) != 0)
    return -1;
  int nwpw_scf_algorithms_has_options = 0;
  int nwpw_ks_algorithm = NWChemNwpwKsAlgorithm_unspecified;
  int nwpw_scf_algorithm = NWChemNwpwScfAlgorithm_unspecified;
  int nwpw_precondition = NWChemNwpwToggle_unspecified;
  if (nwchemc_params_extract_direct_nwpw_scf_algorithms(
          params_root, &nwpw_scf_algorithms_has_options, &nwpw_ks_algorithm,
          &nwpw_scf_algorithm, &nwpw_precondition) != 0)
    return -1;
  int nwpw_scf_numeric_has_options = 0;
  NWChemNwpwScfNumericControls nwpw_scf_numeric = {0};
  if (nwchemc_params_extract_direct_nwpw_scf_numeric(
          params_root, &nwpw_scf_numeric_has_options,
          &nwpw_scf_numeric) != 0)
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
  int brillouin_tetrahedron_has_options = 0;
  int brillouin_tetrahedron_grid[3] = {0, 0, 0};
  if (nwchemc_params_extract_direct_brillouin_tetrahedron(
          params_root, &brillouin_tetrahedron_has_options,
          brillouin_tetrahedron_grid) != 0)
    return -1;
  int brillouin_dos_grid_has_options = 0;
  int brillouin_dos_grid[3] = {0, 0, 0};
  if (nwchemc_params_extract_direct_brillouin_dos_grid(
          params_root, &brillouin_dos_grid_has_options,
          brillouin_dos_grid) != 0)
    return -1;
  capn_text brillouin_dos_zone_names[NWCHEMC_DIRECT_SET_MAX];
  int brillouin_dos_zone_grids[3 * NWCHEMC_DIRECT_SET_MAX];
  size_t brillouin_dos_zone_count = 0;
  if (nwchemc_params_extract_direct_brillouin_dos_zones(
          params_root, brillouin_dos_zone_names, brillouin_dos_zone_grids,
          NWCHEMC_DIRECT_SET_MAX, &brillouin_dos_zone_count) != 0 ||
      brillouin_dos_zone_count > (size_t)INT_MAX)
    return -1;
  int psp_types[NWCHEMC_DIRECT_PSP_MAX];
  size_t psp_count = 0;
  int psp_spin_has_options = 0;
  int psp_spin_enabled = 0;
  int psp_spin_count = 0;
  int psp_semicore_small = NWChemToggle_unspecified;
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
  char nwpw_direct_string_keys[NWCHEMC_DIRECT_SET_MAX]
                              [NWCHEMC_DIRECT_SET_KEY_LEN];
  char nwpw_direct_string_values[NWCHEMC_DIRECT_SET_MAX]
                                [NWCHEMC_DIRECT_SET_VALUE_LEN];
  char packed_set_keys[NWCHEMC_DIRECT_SET_MAX * NWCHEMC_DIRECT_SET_KEY_LEN];
  char packed_set_values[NWCHEMC_DIRECT_SET_MAX *
                         NWCHEMC_DIRECT_SET_VALUE_LEN];
  char packed_typed_set_keys[NWCHEMC_DIRECT_SET_MAX *
                             NWCHEMC_DIRECT_SET_KEY_LEN];
  static char packed_typed_set_values[NWCHEMC_DIRECT_SET_MAX *
                                      NWCHEMC_DIRECT_SET_VALUE_MAX *
                                      NWCHEMC_DIRECT_SET_VALUE_LEN];
  char packed_brillouin_dos_zone_names[NWCHEMC_DIRECT_SET_MAX *
                                       NWCHEMC_DIRECT_DOS_ZONE_NAME_LEN];
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
          &psp_spin_count, &psp_semicore_small) != 0)
    return -1;
  int psp_uterm_has_options = 0;
  int psp_uterm_enabled = 0;
  int psp_uterm_count = 0;
  if (nwchemc_params_extract_direct_pseudopotential_uterm(
          params_root, &psp_uterm_has_options, &psp_uterm_enabled,
          &psp_uterm_count) != 0)
    return -1;
  if (nwchemc_params_extract_direct_set_strings(
          params_root, set_keys, set_values, NWCHEMC_DIRECT_SET_MAX,
          &set_count) != 0)
    return -1;
  if (append_brillouin_zone_alias_strings(
          params_root, set_keys, set_values, NWCHEMC_DIRECT_SET_MAX,
          &set_count) != 0)
    return -1;
  if (nwpw_vfield_has_options && nwpw_vfield_count > 0) {
    char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    size_t used = 0;
    value[0] = '\0';
    for (size_t i = 0; i < nwpw_vfield_count; ++i) {
      capn_text filename = nwpw_vfield_filenames[i];
      if (filename.len <= 0)
        continue;
      if (!filename.str)
        return -1;
      size_t len = (size_t)filename.len;
      if (used > 0) {
        if (used + 1 >= sizeof(value))
          return -1;
        value[used++] = ' ';
      }
      if (len >= sizeof(value) - used)
        return -1;
      memcpy(value + used, filename.str, len);
      used += len;
      value[used] = '\0';
    }
    if (used > 0 &&
        append_owned_direct_string_value(
            set_keys, set_values, NWCHEMC_DIRECT_SET_MAX, &set_count,
            nwpw_direct_string_keys, nwpw_direct_string_values,
            "nwpw:vfield_filenames", value) != 0)
      return -1;
  }
  if (nwpw_dos_filename.len > 0 &&
      append_direct_string_value(set_keys, set_values, NWCHEMC_DIRECT_SET_MAX,
                                 &set_count, "nwpw:dos:filename",
                                 nwpw_dos_filename) != 0)
    return -1;
  if (nwpw_et_has_options) {
    if (append_direct_string_value(set_keys, set_values,
                                   NWCHEMC_DIRECT_SET_MAX, &set_count,
                                   "pspw:et:movecs_a",
                                   nwpw_et_movecs_a) != 0)
      return -1;
    if (append_direct_string_value(set_keys, set_values,
                                   NWCHEMC_DIRECT_SET_MAX, &set_count,
                                   "pspw:et:movecs_b",
                                   nwpw_et_movecs_b) != 0)
      return -1;
    if (append_direct_string_value(set_keys, set_values,
                                   NWCHEMC_DIRECT_SET_MAX, &set_count,
                                   "pspw:et:ion_a", nwpw_et_ion_a) != 0)
      return -1;
    if (append_direct_string_value(set_keys, set_values,
                                   NWCHEMC_DIRECT_SET_MAX, &set_count,
                                   "pspw:et:ion_b", nwpw_et_ion_b) != 0)
      return -1;
  }
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
    if (nwpw_scaling_atom_count > 0) {
      char count_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(count_value, sizeof(count_value), "%zu",
               nwpw_scaling_atom_count);
      if (append_direct_typed_value(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
              nwpw_direct_values, "nwpw:scaling_natms",
              NWCHEMC_DIRECT_SET_VALUE_INTEGER, count_value) != 0)
        return -1;
      char atom_storage[NWCHEMC_DIRECT_SET_VALUE_MAX]
                       [NWCHEMC_DIRECT_SET_VALUE_LEN];
      const char *atom_values[NWCHEMC_DIRECT_SET_VALUE_MAX];
      for (size_t i = 0; i < nwpw_scaling_atom_count; ++i) {
        snprintf(atom_storage[i], sizeof(atom_storage[i]), "%d",
                 nwpw_scaling_atoms[i]);
        atom_values[i] = atom_storage[i];
      }
      if (append_direct_typed_values(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
              nwpw_direct_values, "nwpw:scaling_atoms",
              NWCHEMC_DIRECT_SET_VALUE_INTEGER, atom_values,
              (int)nwpw_scaling_atom_count) != 0)
        return -1;
    }
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
  if (psp_semicore_small != NWChemToggle_unspecified) {
    const char *value =
        psp_semicore_small == NWChemToggle_enabled ? "true" : "false";
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:psp:semicore_small",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, value) != 0)
      return -1;
  }
  if (brillouin_tetrahedron_has_options) {
    char x_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    char y_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    char z_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    const char *value_list[3] = {x_value, y_value, z_value};
    snprintf(x_value, sizeof(x_value), "%d", brillouin_tetrahedron_grid[0]);
    snprintf(y_value, sizeof(y_value), "%d", brillouin_tetrahedron_grid[1]);
    snprintf(z_value, sizeof(z_value), "%d", brillouin_tetrahedron_grid[2]);
    if (append_direct_typed_values(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "band:tetrahedron-grid",
            NWCHEMC_DIRECT_SET_VALUE_INTEGER, value_list, 3) != 0)
      return -1;
  }
  if (brillouin_dos_grid_has_options) {
    char x_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    char y_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    char z_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    const char *value_list[3] = {x_value, y_value, z_value};
    snprintf(x_value, sizeof(x_value), "%d", brillouin_dos_grid[0]);
    snprintf(y_value, sizeof(y_value), "%d", brillouin_dos_grid[1]);
    snprintf(z_value, sizeof(z_value), "%d", brillouin_dos_grid[2]);
    if (append_direct_typed_values(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "band:dos-grid",
            NWCHEMC_DIRECT_SET_VALUE_INTEGER, value_list, 3) != 0)
      return -1;
  }
  if (psp_uterm_has_options) {
    char count_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    const char *enabled_value = psp_uterm_enabled ? "true" : "false";
    snprintf(count_value, sizeof(count_value), "%d", psp_uterm_count);
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:uterm",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, enabled_value) != 0)
      return -1;
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:nuterms",
            NWCHEMC_DIRECT_SET_VALUE_INTEGER, count_value) != 0)
      return -1;
    if (psp_uterm_enabled && psp_uterm_count > 0) {
      struct direct_pspspin_rule_append_state uterm_rule_state = {
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
      size_t walked_uterm_rules = 0;
      if (nwchemc_params_for_each_direct_pseudopotential_uterm_rule(
              params_root, append_direct_uterm_rule, &uterm_rule_state,
              &walked_uterm_rules) != 0 ||
          walked_uterm_rules != (size_t)psp_uterm_count)
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
  if (nwpw_occupations_has_options && nwpw_occupation_count > 0) {
    char count_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    snprintf(count_value, sizeof(count_value), "%zu", nwpw_occupation_count);
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:frac_occ:number_states",
            NWCHEMC_DIRECT_SET_VALUE_INTEGER, count_value) != 0)
      return -1;

    char occupation_storage[NWCHEMC_DIRECT_SET_VALUE_MAX]
                           [NWCHEMC_DIRECT_SET_VALUE_LEN];
    char state_storage[NWCHEMC_DIRECT_SET_VALUE_MAX]
                      [NWCHEMC_DIRECT_SET_VALUE_LEN];
    const char *occupation_values[NWCHEMC_DIRECT_SET_VALUE_MAX];
    const char *state_values[NWCHEMC_DIRECT_SET_VALUE_MAX];
    for (size_t i = 0; i < nwpw_occupation_count; ++i) {
      snprintf(occupation_storage[i], sizeof(occupation_storage[i]), "%.15g",
               nwpw_occupations[i]);
      occupation_values[i] = occupation_storage[i];
      snprintf(state_storage[i], sizeof(state_storage[i]), "%d",
               nwpw_occupation_states[i] > 0 ? nwpw_occupation_states[i] : 1);
      state_values[i] = state_storage[i];
    }
    if (append_direct_typed_values(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:frac_occ:occupations",
            NWCHEMC_DIRECT_SET_VALUE_DOUBLE, occupation_values,
            (int)nwpw_occupation_count) != 0)
      return -1;
    if (append_direct_typed_values(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:frac_occ:states",
            NWCHEMC_DIRECT_SET_VALUE_INTEGER, state_values,
            (int)nwpw_occupation_count) != 0)
      return -1;
  }
  if (nwpw_extra_orbitals > 0) {
    char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    snprintf(value, sizeof(value), "%d", nwpw_extra_orbitals);
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:frac_occ:extra_orbitals",
            NWCHEMC_DIRECT_SET_VALUE_INTEGER, value) != 0)
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
  if (nwpw_lcao_mask_has_options &&
      nwpw_lcao_mask != NWChemNwpwToggle_unspecified) {
    const char *value =
        nwpw_lcao_mask == NWChemNwpwToggle_enabled ? "true" : "false";
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:lcao_mask",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, value) != 0)
      return -1;
  }
  if (nwpw_lcao_mask_up_count > 0 &&
      append_direct_integer_values(
          typed_set_keys, typed_set_types, typed_set_value_counts,
          typed_set_values, NWCHEMC_DIRECT_SET_MAX,
          NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
          nwpw_direct_values, "nwpw:lcao_mask_uporbs",
          nwpw_lcao_mask_up_orbitals, nwpw_lcao_mask_up_count) != 0)
    return -1;
  if (nwpw_lcao_mask_down_count > 0 &&
      append_direct_integer_values(
          typed_set_keys, typed_set_types, typed_set_value_counts,
          typed_set_values, NWCHEMC_DIRECT_SET_MAX,
          NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
          nwpw_direct_values, "nwpw:lcao_mask_downorbs",
          nwpw_lcao_mask_down_orbitals, nwpw_lcao_mask_down_count) != 0)
    return -1;
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
  if (nwpw_electric_field_has_options &&
      nwpw_atom_efield != NWChemNwpwToggle_unspecified) {
    const char *value =
        nwpw_atom_efield == NWChemNwpwToggle_enabled ? "true" : "false";
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:atom_efield",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, value) != 0)
      return -1;
  }
  if (nwpw_electric_field_has_options &&
      nwpw_atom_efield_gradient != NWChemNwpwToggle_unspecified) {
    const char *value = nwpw_atom_efield_gradient == NWChemNwpwToggle_enabled
                            ? "true"
                            : "false";
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:atom_efield_grad",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, value) != 0)
      return -1;
  }
  if (nwpw_mulliken_has_options) {
    static const char *nwpw_mulliken_prefixes[] = {"cgsd", "band", "nwpw",
                                                   "cpsd", "cpmd"};
    int effective_mulliken = nwpw_mulliken;
    int effective_kawai = nwpw_mulliken_kawai;
    if (effective_mulliken == NWChemNwpwToggle_disabled)
      effective_kawai = NWChemNwpwToggle_disabled;
    if (effective_mulliken == NWChemNwpwToggle_unspecified &&
        effective_kawai == NWChemNwpwToggle_enabled)
      effective_mulliken = NWChemNwpwToggle_enabled;
    if (effective_kawai == NWChemNwpwToggle_unspecified &&
        effective_mulliken != NWChemNwpwToggle_unspecified)
      effective_kawai = NWChemNwpwToggle_disabled;
    const char *mulliken_value =
        nwpw_toggle_logical_value((enum NWChemNwpwToggle)effective_mulliken);
    const char *kawai_value =
        nwpw_toggle_logical_value((enum NWChemNwpwToggle)effective_kawai);
    if (mulliken_value) {
      const char *value_list[1] = {mulliken_value};
      if (append_nwpw_prefixed_typed_values(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, nwpw_mulliken_prefixes, 5,
              "mulliken", NWCHEMC_DIRECT_SET_VALUE_LOGICAL, value_list, 1) !=
          0)
        return -1;
    }
    if (kawai_value &&
        append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:mulliken_kawai",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, kawai_value) != 0)
      return -1;
  }
  if (nwpw_periodic_dipole_has_options) {
    const char *value = nwpw_toggle_logical_value(
        (enum NWChemNwpwToggle)nwpw_periodic_dipole);
    if (value &&
        append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:periodic_dipole",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, value) != 0)
      return -1;
  }
  if (nwpw_efield_has_options) {
    const char *efield_value =
        nwpw_toggle_logical_value((enum NWChemNwpwToggle)nwpw_efield);
    if (efield_value &&
        append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:efield",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, efield_value) != 0)
      return -1;
    if (nwpw_efield == NWChemNwpwToggle_enabled) {
      char x_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      char y_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      char z_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      const char *vector_values[3] = {x_value, y_value, z_value};
      snprintf(x_value, sizeof(x_value), "%.15g", nwpw_efield_vector[0]);
      snprintf(y_value, sizeof(y_value), "%.15g", nwpw_efield_vector[1]);
      snprintf(z_value, sizeof(z_value), "%.15g", nwpw_efield_vector[2]);
      if (append_direct_typed_values(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, "nwpw:efield_vector",
              NWCHEMC_DIRECT_SET_VALUE_DOUBLE, vector_values, 3) != 0)
        return -1;
      if (nwpw_efield_has_center) {
        char cx_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
        char cy_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
        char cz_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
        const char *center_values[3] = {cx_value, cy_value, cz_value};
        snprintf(cx_value, sizeof(cx_value), "%.15g", nwpw_efield_center[0]);
        snprintf(cy_value, sizeof(cy_value), "%.15g", nwpw_efield_center[1]);
        snprintf(cz_value, sizeof(cz_value), "%.15g", nwpw_efield_center[2]);
        if (append_direct_typed_values(
                typed_set_keys, typed_set_types, typed_set_value_counts,
                typed_set_values, NWCHEMC_DIRECT_SET_MAX,
                NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
                nwpw_direct_keys, nwpw_direct_values, "nwpw:efield_center",
                NWCHEMC_DIRECT_SET_VALUE_DOUBLE, center_values, 3) != 0)
          return -1;
      }
      int efield_type_value = 0;
      int has_efield_type = nwpw_efield_type_rtdb_value(
          (enum NWChemNwpwEfieldType)nwpw_efield_type, &efield_type_value);
      if (has_efield_type < 0)
        return -1;
      if (has_efield_type) {
        char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
        snprintf(value, sizeof(value), "%d", efield_type_value);
        if (append_direct_typed_value(
                typed_set_keys, typed_set_types, typed_set_value_counts,
                typed_set_values, NWCHEMC_DIRECT_SET_MAX,
                NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
                nwpw_direct_keys, nwpw_direct_values, "nwpw:efield_type",
                NWCHEMC_DIRECT_SET_VALUE_INTEGER, value) != 0)
          return -1;
      }
    }
  }
  if (nwpw_smooth_cutoff_has_options &&
      nwpw_smooth_cutoff == NWChemNwpwToggle_enabled) {
    char afac_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    char sigma_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    const char *smooth_cutoff_values[2] = {afac_value, sigma_value};
    snprintf(afac_value, sizeof(afac_value), "%.15g",
             nwpw_smooth_cutoff_values[0]);
    snprintf(sigma_value, sizeof(sigma_value), "%.15g",
             nwpw_smooth_cutoff_values[1]);
    if (append_direct_typed_values(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:smooth_cutoff",
            NWCHEMC_DIRECT_SET_VALUE_DOUBLE, smooth_cutoff_values, 2) != 0)
      return -1;
  }
  if (nwpw_cutoff_boot_wavefunction_has_options) {
    const char *value = nwpw_toggle_logical_value(
        (enum NWChemNwpwToggle)nwpw_cutoff_boot_wavefunction);
    if (value &&
        append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:cutoff_boot_psi",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, value) != 0)
      return -1;
  }
  if (nwpw_fast_erf_has_options) {
    const char *value =
        nwpw_toggle_logical_value((enum NWChemNwpwToggle)nwpw_fast_erf);
    if (value &&
        append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:fast_erf",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, value) != 0)
      return -1;
  }
  if (nwpw_dipole_motion_has_options) {
    const char *value =
        nwpw_toggle_logical_value((enum NWChemNwpwToggle)nwpw_dipole_motion);
    if (value &&
        append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:dipole_motion",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, value) != 0)
      return -1;
    if (nwpw_dipole_motion == NWChemNwpwToggle_enabled &&
        nwpw_dipole_motion_filename.len > 0) {
      char filename[NWCHEMC_DIRECT_SET_VALUE_LEN];
      copy_text_record(filename, sizeof(filename), nwpw_dipole_motion_filename);
      if (append_direct_typed_value(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values,
              "nwpw:dipole_motion_filename", NWCHEMC_DIRECT_SET_VALUE_TEXT,
              filename) != 0)
        return -1;
    }
  }
  if (nwpw_rho_use_symmetry_has_options) {
    const char *value = nwpw_toggle_logical_value(
        (enum NWChemNwpwToggle)nwpw_rho_use_symmetry);
    if (value &&
        append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:rho_use_symmetry",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, value) != 0)
      return -1;
  }
  if (nwpw_one_electron_guess_has_options) {
    char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    snprintf(value, sizeof(value), "%d", nwpw_one_electron_guess_it_in);
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:H1_it_in",
            NWCHEMC_DIRECT_SET_VALUE_INTEGER, value) != 0)
      return -1;
    snprintf(value, sizeof(value), "%d", nwpw_one_electron_guess_it_out);
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:H1_it_out",
            NWCHEMC_DIRECT_SET_VALUE_INTEGER, value) != 0)
      return -1;
    snprintf(value, sizeof(value), "%d", nwpw_one_electron_guess_it_ortho);
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:H1_it_ortho",
            NWCHEMC_DIRECT_SET_VALUE_INTEGER, value) != 0)
      return -1;
  }
  if (nwpw_fmm_has_options) {
    const char *fmm_value =
        nwpw_toggle_logical_value((enum NWChemNwpwToggle)nwpw_fmm);
    if (fmm_value &&
        append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:fmm", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
            fmm_value) != 0)
      return -1;
    char lmax_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    char long_range_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    snprintf(lmax_value, sizeof(lmax_value), "%d", nwpw_fmm_lmax);
    snprintf(long_range_value, sizeof(long_range_value), "%d",
             nwpw_fmm_long_range);
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:fmm_lmax",
            NWCHEMC_DIRECT_SET_VALUE_INTEGER, lmax_value) != 0)
      return -1;
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:fmm_lr",
            NWCHEMC_DIRECT_SET_VALUE_INTEGER, long_range_value) != 0)
      return -1;
  }
  if (nwpw_born_has_options) {
    const char *born_value =
        nwpw_toggle_logical_value((enum NWChemNwpwToggle)nwpw_born);
    if (born_value &&
        append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:born", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
            born_value) != 0)
      return -1;
    if (nwpw_born_dielectric > 0.0) {
      char dielectric_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(dielectric_value, sizeof(dielectric_value), "%.15g",
               nwpw_born_dielectric);
      if (append_direct_typed_value(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, "nwpw:born_dielec",
              NWCHEMC_DIRECT_SET_VALUE_DOUBLE, dielectric_value) != 0)
        return -1;
    }
    if (nwpw_born_relax != NWChemNwpwToggle_unspecified) {
      const char *relax_value =
          nwpw_toggle_logical_value((enum NWChemNwpwToggle)nwpw_born_relax);
      if (relax_value &&
          append_direct_typed_value(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, "nwpw:born_relax",
              NWCHEMC_DIRECT_SET_VALUE_LOGICAL, relax_value) != 0)
        return -1;
    }
    if (nwpw_born == NWChemNwpwToggle_enabled &&
        nwpw_born_vradii_count > 0) {
      char radius_values_storage[NWCHEMC_DIRECT_SET_VALUE_MAX]
                                [NWCHEMC_DIRECT_SET_VALUE_LEN];
      const char *radius_values[NWCHEMC_DIRECT_SET_VALUE_MAX];
      for (size_t i = 0; i < nwpw_born_vradii_count; ++i) {
        double radius_bohr = nwpw_born_vradii_angstrom[i] / 0.529177;
        snprintf(radius_values_storage[i], sizeof(radius_values_storage[i]),
                 "%.15g", radius_bohr);
        radius_values[i] = radius_values_storage[i];
      }
      if (append_direct_typed_values(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, "nwpw:born_vradii",
              NWCHEMC_DIRECT_SET_VALUE_DOUBLE, radius_values,
              nwpw_born_vradii_count) != 0)
        return -1;
    }
  }
  if (nwpw_single_precision_hfx_has_options && nwpw_single_precision_hfx) {
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "pspw:HFX_single_precision",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true") != 0)
      return -1;
  }
  if (nwpw_geometry_optimize_has_options && nwpw_geometry_optimize) {
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "cgsd:geometry_optimize",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true") != 0 ||
        append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "cpsd:geometry_optimize",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true") != 0 ||
        append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "band:geometry_optimize",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true") != 0)
      return -1;
  }
  if (nwpw_auxiliary_potentials_has_options && nwpw_auxiliary_potentials) {
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "pspw_qmmm_auxon",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true") != 0)
      return -1;
  }
  if (nwpw_allow_translation_has_options && nwpw_allow_translation) {
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "cgsd:allow_translation",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true") != 0)
      return -1;
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "band:allow_translation",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true") != 0)
      return -1;
  }
  if (nwpw_multiplicity_has_options) {
    static const char *spin_prefixes[] = {"cgsd", "band", "cpsd"};
    for (size_t i = 0; i < sizeof(spin_prefixes) / sizeof(spin_prefixes[0]);
         ++i) {
      char key[NWCHEMC_DIRECT_SET_KEY_LEN];
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(key, sizeof(key), "%s:ispin", spin_prefixes[i]);
      snprintf(value, sizeof(value), "%d", nwpw_ispin);
      if (append_direct_typed_value(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, key,
              NWCHEMC_DIRECT_SET_VALUE_INTEGER, value) != 0)
        return -1;
      snprintf(key, sizeof(key), "%s:mult", spin_prefixes[i]);
      snprintf(value, sizeof(value), "%d", nwpw_multiplicity);
      if (append_direct_typed_value(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, key,
              NWCHEMC_DIRECT_SET_VALUE_INTEGER, value) != 0)
        return -1;
    }
  }
  for (size_t i = 0; i < nwpw_spin_ispin_count; ++i) {
    static const char *spin_prefixes[] = {"cgsd", "band", "cpsd"};
    for (size_t j = 0; j < sizeof(spin_prefixes) / sizeof(spin_prefixes[0]);
         ++j) {
      char key[NWCHEMC_DIRECT_SET_KEY_LEN];
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(key, sizeof(key), "%s:ispin", spin_prefixes[j]);
      snprintf(value, sizeof(value), "%d", nwpw_spin_ispins[i]);
      if (append_direct_typed_value(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, key,
              NWCHEMC_DIRECT_SET_VALUE_INTEGER, value) != 0)
        return -1;
    }
  }
  if (nwpw_dos_has_options) {
    if (nwpw_dos_alpha_set) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(value, sizeof(value), "%.15g", nwpw_dos_alpha);
      if (append_direct_typed_value(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, "dos:alpha",
              NWCHEMC_DIRECT_SET_VALUE_DOUBLE, value) != 0)
        return -1;
    }
    if (nwpw_dos_npoints_set) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(value, sizeof(value), "%d", nwpw_dos_npoints);
      if (append_direct_typed_value(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, "dos:npoints",
              NWCHEMC_DIRECT_SET_VALUE_INTEGER, value) != 0)
        return -1;
    }
    if (nwpw_dos_emin_set) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(value, sizeof(value), "%.15g", nwpw_dos_emin);
      if (append_direct_typed_value(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, "dos:emin",
              NWCHEMC_DIRECT_SET_VALUE_DOUBLE, value) != 0)
        return -1;
    }
    if (nwpw_dos_emax_set) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(value, sizeof(value), "%.15g", nwpw_dos_emax);
      if (append_direct_typed_value(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, "dos:emax",
              NWCHEMC_DIRECT_SET_VALUE_DOUBLE, value) != 0)
        return -1;
    }
  }
  if (nwpw_cpmd_grid_has_options) {
    const char *cpmd_properties_value = nwpw_toggle_logical_value(
        (enum NWChemNwpwToggle)nwpw_cpmd_properties);
    if (cpmd_properties_value &&
        append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:cpmd_properties",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, cpmd_properties_value) != 0)
      return -1;
    const char *use_grid_comparison_value = nwpw_toggle_logical_value(
        (enum NWChemNwpwToggle)nwpw_use_grid_comparison);
    if (use_grid_comparison_value &&
        append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:use_grid_cmp",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
            use_grid_comparison_value) != 0)
      return -1;
  }
  if (nwpw_director_has_options) {
    const char *director_value =
        nwpw_toggle_logical_value((enum NWChemNwpwToggle)nwpw_director);
    if (director_value &&
        append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:use_director",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, director_value) != 0)
      return -1;
    if (nwpw_director == NWChemNwpwToggle_enabled &&
        nwpw_director_filename.len > 0) {
      char filename[NWCHEMC_DIRECT_SET_VALUE_LEN];
      copy_text_record(filename, sizeof(filename), nwpw_director_filename);
      if (append_direct_typed_value(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, "nwpw:director_filename",
              NWCHEMC_DIRECT_SET_VALUE_TEXT, filename) != 0)
        return -1;
    }
  }
  if (nwpw_cell_mapping_has_options) {
    if (nwpw_cell_expand[0] > 0) {
      char x_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      char y_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      char z_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      const char *cell_expand_values[3] = {x_value, y_value, z_value};
      snprintf(x_value, sizeof(x_value), "%d", nwpw_cell_expand[0]);
      snprintf(y_value, sizeof(y_value), "%d", nwpw_cell_expand[1]);
      snprintf(z_value, sizeof(z_value), "%d", nwpw_cell_expand[2]);
      if (append_direct_typed_values(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, "nwpw:cell_expand",
              NWCHEMC_DIRECT_SET_VALUE_INTEGER, cell_expand_values, 3) != 0)
        return -1;
    }
    if (nwpw_mapping > 0) {
      char mapping_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(mapping_value, sizeof(mapping_value), "%d", nwpw_mapping);
      if (append_direct_typed_value(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, "nwpw:mapping",
              NWCHEMC_DIRECT_SET_VALUE_INTEGER, mapping_value) != 0)
        return -1;
    }
  }
  if (nwpw_rotation_multipole_has_options) {
    const char *rotation_value =
        nwpw_toggle_logical_value((enum NWChemNwpwToggle)nwpw_rotation);
    if (rotation_value &&
        append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:rotation",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, rotation_value) != 0)
      return -1;
    if (nwpw_lmax_multipole >= 0) {
      char lmax_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(lmax_value, sizeof(lmax_value), "%d", nwpw_lmax_multipole);
      if (append_direct_typed_value(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, "nwpw:lmax_multipole",
              NWCHEMC_DIRECT_SET_VALUE_INTEGER, lmax_value) != 0)
        return -1;
    }
  }
  if (nwpw_fei_has_options) {
    if (nwpw_fei_filename.len > 0) {
      char filename[NWCHEMC_DIRECT_SET_VALUE_LEN];
      copy_text_record(filename, sizeof(filename), nwpw_fei_filename);
      if (append_direct_typed_value(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, "cpmd:fei_filename",
              NWCHEMC_DIRECT_SET_VALUE_TEXT, filename) != 0)
        return -1;
      if (append_direct_typed_value(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, "nwpw:fei_filename",
              NWCHEMC_DIRECT_SET_VALUE_TEXT, filename) != 0)
        return -1;
    }
    if (nwpw_fei) {
      if (append_direct_typed_value(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, "cpmd:fei",
              NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true") != 0)
        return -1;
      if (append_direct_typed_value(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, "nwpw:fei",
              NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true") != 0)
        return -1;
    }
  }
  if (nwpw_initial_velocities_has_options) {
    char temperature_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    char seed_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    snprintf(temperature_value, sizeof(temperature_value), "%.15g",
             nwpw_initial_velocities_temperature);
    snprintf(seed_value, sizeof(seed_value), "%d",
             nwpw_initial_velocities_seed);
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:init_velocities_temperature",
            NWCHEMC_DIRECT_SET_VALUE_DOUBLE, temperature_value) != 0)
      return -1;
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:init_velocities_seed",
            NWCHEMC_DIRECT_SET_VALUE_INTEGER, seed_value) != 0)
      return -1;
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:init_velocities",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true") != 0)
      return -1;
  }
  if (nwpw_make_hmass2_has_options) {
    const char *make_hmass2_value =
        nwpw_toggle_logical_value((enum NWChemNwpwToggle)nwpw_make_hmass2);
    if (make_hmass2_value &&
        append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:makehmass2",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, make_hmass2_value) != 0)
      return -1;
  }
  if (nwpw_translate_vector_has_options) {
    char x_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    char y_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    char z_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    const char *translate_values[3] = {x_value, y_value, z_value};
    snprintf(x_value, sizeof(x_value), "%.15g", nwpw_translate_vector[0]);
    snprintf(y_value, sizeof(y_value), "%.15g", nwpw_translate_vector[1]);
    snprintf(z_value, sizeof(z_value), "%.15g", nwpw_translate_vector[2]);
    if (append_direct_typed_values(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:translate_vector",
            NWCHEMC_DIRECT_SET_VALUE_DOUBLE, translate_values, 3) != 0)
      return -1;
    if (nwpw_translate_geometry_name.len > 0) {
      char geometry_name[NWCHEMC_DIRECT_SET_VALUE_LEN];
      copy_text_record(geometry_name, sizeof(geometry_name),
                       nwpw_translate_geometry_name);
      if (append_direct_typed_value(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, "nwpw:translate_geom_name",
              NWCHEMC_DIRECT_SET_VALUE_TEXT, geometry_name) != 0)
        return -1;
    }
    const char *reorder_value = nwpw_toggle_logical_value(
        (enum NWChemNwpwToggle)nwpw_translate_reorder);
    if (reorder_value &&
        append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:translate_reorder",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, reorder_value) != 0)
      return -1;
  }
  if (nwpw_socket_has_options) {
    char socket_type[NWCHEMC_DIRECT_SET_VALUE_LEN];
    copy_text_record(socket_type, sizeof(socket_type), nwpw_socket_type);
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:socket_type",
            NWCHEMC_DIRECT_SET_VALUE_TEXT, socket_type) != 0)
      return -1;
    if (nwpw_socket_ip.len > 0) {
      char socket_ip[NWCHEMC_DIRECT_SET_VALUE_LEN];
      copy_text_record(socket_ip, sizeof(socket_ip), nwpw_socket_ip);
      if (append_direct_typed_value(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, "nwpw:socket_ip",
              NWCHEMC_DIRECT_SET_VALUE_TEXT, socket_ip) != 0)
        return -1;
    }
  }
  if (nwpw_apc_has_options) {
    char gc_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    char count_value[NWCHEMC_DIRECT_SET_VALUE_LEN];
    snprintf(gc_value, sizeof(gc_value), "%.15g", nwpw_apc_gc);
    snprintf(count_value, sizeof(count_value), "%zu", nwpw_apc_gamma_count);
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw_APC:Gc",
            NWCHEMC_DIRECT_SET_VALUE_DOUBLE, gc_value) != 0)
      return -1;
    if (append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw_APC:nga",
            NWCHEMC_DIRECT_SET_VALUE_INTEGER, count_value) != 0)
      return -1;
    char gamma_values_storage[NWCHEMC_DIRECT_SET_VALUE_MAX]
                             [NWCHEMC_DIRECT_SET_VALUE_LEN];
    const char *gamma_values[NWCHEMC_DIRECT_SET_VALUE_MAX];
    for (size_t i = 0; i < nwpw_apc_gamma_count; ++i) {
      snprintf(gamma_values_storage[i], sizeof(gamma_values_storage[i]),
               "%.15g", nwpw_apc_gamma[i]);
      gamma_values[i] = gamma_values_storage[i];
    }
    if (append_direct_typed_values(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw_APC:gamma",
            NWCHEMC_DIRECT_SET_VALUE_DOUBLE, gamma_values,
            nwpw_apc_gamma_count) != 0)
      return -1;
  }
  if (nwpw_translation_has_options) {
    const char *translation_value =
        nwpw_toggle_logical_value((enum NWChemNwpwToggle)nwpw_translation);
    if (translation_value &&
        append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "cgsd:allow_translation",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, translation_value) != 0)
      return -1;
    if (translation_value &&
        append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "band:allow_translation",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, translation_value) != 0)
      return -1;
  }
  if (nwpw_minimizer_has_options) {
    int minimizer_value =
        nwpw_minimizer_rtdb_value((enum NWChemNwpwMinimizer)nwpw_minimizer);
    if (minimizer_value != 0) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(value, sizeof(value), "%d", minimizer_value);
      if (append_direct_typed_value(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, "nwpw:minimizer",
              NWCHEMC_DIRECT_SET_VALUE_INTEGER, value) != 0)
        return -1;
    }
  }
  if (nwpw_scf_algorithms_has_options) {
    int algorithm_value = 0;
    if (nwpw_ks_algorithm_rtdb_value(
            (enum NWChemNwpwKsAlgorithm)nwpw_ks_algorithm,
            &algorithm_value) > 0) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(value, sizeof(value), "%d", algorithm_value);
      if (append_direct_typed_value(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, "nwpw:ks_algorithm",
              NWCHEMC_DIRECT_SET_VALUE_INTEGER, value) != 0)
        return -1;
    }
    if (nwpw_scf_algorithm_rtdb_value(
            (enum NWChemNwpwScfAlgorithm)nwpw_scf_algorithm,
            &algorithm_value) > 0) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(value, sizeof(value), "%d", algorithm_value);
      if (append_direct_typed_value(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, "nwpw:scf_algorithm",
              NWCHEMC_DIRECT_SET_VALUE_INTEGER, value) != 0)
        return -1;
    }
    const char *precondition_value = nwpw_toggle_logical_value(
        (enum NWChemNwpwToggle)nwpw_precondition);
    if (precondition_value &&
        append_direct_typed_value(
            typed_set_keys, typed_set_types, typed_set_value_counts,
            typed_set_values, NWCHEMC_DIRECT_SET_MAX,
            NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count, nwpw_direct_keys,
            nwpw_direct_values, "nwpw:precondition",
            NWCHEMC_DIRECT_SET_VALUE_LOGICAL, precondition_value) != 0)
      return -1;
  }
  if (nwpw_scf_numeric_has_options) {
    if (nwpw_scf_numeric.kerker_g0_set) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(value, sizeof(value), "%.17g", nwpw_scf_numeric.kerker_g0);
      if (append_direct_typed_value(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, "nwpw:kerker_g0",
              NWCHEMC_DIRECT_SET_VALUE_DOUBLE, value) != 0)
        return -1;
    }
    if (nwpw_scf_numeric.ks_alpha_set) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(value, sizeof(value), "%.17g", nwpw_scf_numeric.ks_alpha);
      if (append_direct_typed_value(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, "nwpw:ks_alpha",
              NWCHEMC_DIRECT_SET_VALUE_DOUBLE, value) != 0)
        return -1;
    }
    if (nwpw_scf_numeric.ks_maxit_orb_set) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(value, sizeof(value), "%d", nwpw_scf_numeric.ks_maxit_orb);
      if (append_direct_typed_value(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, "nwpw:ks_maxit_orb",
              NWCHEMC_DIRECT_SET_VALUE_INTEGER, value) != 0)
        return -1;
    }
    if (nwpw_scf_numeric.ks_maxit_orbs_set) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(value, sizeof(value), "%d", nwpw_scf_numeric.ks_maxit_orbs);
      if (append_direct_typed_value(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, "nwpw:ks_maxit_orbs",
              NWCHEMC_DIRECT_SET_VALUE_INTEGER, value) != 0)
        return -1;
    }
    if (nwpw_scf_numeric.diis_histories_set) {
      char value[NWCHEMC_DIRECT_SET_VALUE_LEN];
      snprintf(value, sizeof(value), "%d", nwpw_scf_numeric.diis_histories);
      if (append_direct_typed_value(
              typed_set_keys, typed_set_types, typed_set_value_counts,
              typed_set_values, NWCHEMC_DIRECT_SET_MAX,
              NWCHEMC_DIRECT_SET_VALUE_MAX, &typed_set_count,
              nwpw_direct_keys, nwpw_direct_values, "nwpw:diis_histories",
              NWCHEMC_DIRECT_SET_VALUE_INTEGER, value) != 0)
        return -1;
    }
  }
  memset(packed_set_keys, 0, sizeof(packed_set_keys));
  memset(packed_set_values, 0, sizeof(packed_set_values));
  memset(packed_typed_set_keys, 0, sizeof(packed_typed_set_keys));
  memset(packed_typed_set_values, 0, sizeof(packed_typed_set_values));
  memset(packed_brillouin_dos_zone_names, 0,
         sizeof(packed_brillouin_dos_zone_names));
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
  for (size_t i = 0; i < brillouin_dos_zone_count; ++i) {
    copy_text_record(
        packed_brillouin_dos_zone_names +
            i * NWCHEMC_DIRECT_DOS_ZONE_NAME_LEN,
        NWCHEMC_DIRECT_DOS_ZONE_NAME_LEN, brillouin_dos_zone_names[i]);
  }
  /* DFT stanza XC is a functional label for DFT-class theories only.
   * Never rewrite theory; never overwrite scfType for HF-class theories. */
  {
    int dft_class =
        span_starts_with(theory, theory_len, "dft") ||
        span_starts_with(theory, theory_len, "tddft") ||
        span_starts_with(theory, theory_len, "sodft");
    if (dft_class && dft_xc.len > 0 && dft_xc.str) {
      scf_type = dft_xc.str;
      scf_len = (int)dft_xc.len;
    }
  }

  apply_env_hints(params);
  ensure_init();
  if (nwchemc_embed_reset_rtdb() != 0)
    return -1;
  g_active_session = NULL;
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
  if (nwchemc_embed_set_brillouin_dos_zones(
          packed_brillouin_dos_zone_names, brillouin_dos_zone_grids,
          (int)brillouin_dos_zone_count) != 0)
    return -1;
  if (nwchemc_embed_set_nwpw_direct(nwpw_has_options, nwpw_energy_cutoff,
                                    nwpw_wavefunction_cutoff,
                                    nwpw_ewald_rcut, nwpw_ewald_ncut) != 0)
    return -1;
  if (nwchemc_embed_set_scf_direct(scf_has_options, scf_maxiter, scf_thresh,
                                   scf_tol2e) != 0)
    return -1;
  /* Promote maximally-typed stanza knobs via RTDB (embed render omits them).
   * DFT grid and SCF noprint stay text-only where no stable RTDB key exists.
   * Capacity shares NWCHEMC_DIRECT_SET_MAX with the main typed-set path. */
  {
    enum { PROMO_CAP = NWCHEMC_DIRECT_SET_MAX };
    static char promo_str_key_storage[PROMO_CAP][NWCHEMC_DIRECT_SET_KEY_LEN];
    static char promo_str_val_storage[PROMO_CAP][NWCHEMC_DIRECT_SET_VALUE_LEN];
    static char promo_key_storage[PROMO_CAP][NWCHEMC_DIRECT_SET_KEY_LEN];
    static char promo_value_storage[PROMO_CAP][NWCHEMC_DIRECT_SET_VALUE_MAX]
                                   [NWCHEMC_DIRECT_SET_VALUE_LEN];
    static char promo_str_packed_keys[PROMO_CAP * NWCHEMC_DIRECT_SET_KEY_LEN];
    static char promo_str_packed_values[PROMO_CAP * NWCHEMC_DIRECT_SET_VALUE_LEN];
    static char promo_typed_packed_keys[PROMO_CAP * NWCHEMC_DIRECT_SET_KEY_LEN];
    static char promo_typed_packed_values[PROMO_CAP *
                                          NWCHEMC_DIRECT_SET_VALUE_MAX *
                                          NWCHEMC_DIRECT_SET_VALUE_LEN];
    static capn_text promo_str_keys[PROMO_CAP];
    static capn_text promo_str_vals[PROMO_CAP];
    static capn_text promo_keys[PROMO_CAP];
    static int promo_types[PROMO_CAP];
    static int promo_counts[PROMO_CAP];
    static capn_text promo_vals[PROMO_CAP * NWCHEMC_DIRECT_SET_VALUE_MAX];
    size_t promo_str_count = 0;
    size_t promo_count = 0;
    memset(promo_str_key_storage, 0, sizeof(promo_str_key_storage));
    memset(promo_str_val_storage, 0, sizeof(promo_str_val_storage));
    memset(promo_key_storage, 0, sizeof(promo_key_storage));
    memset(promo_value_storage, 0, sizeof(promo_value_storage));
    memset(promo_str_packed_keys, 0, sizeof(promo_str_packed_keys));
    memset(promo_str_packed_values, 0, sizeof(promo_str_packed_values));
    memset(promo_typed_packed_keys, 0, sizeof(promo_typed_packed_keys));
    memset(promo_typed_packed_values, 0, sizeof(promo_typed_packed_values));
    memset(promo_str_keys, 0, sizeof(promo_str_keys));
    memset(promo_str_vals, 0, sizeof(promo_str_vals));
    memset(promo_keys, 0, sizeof(promo_keys));
    memset(promo_types, 0, sizeof(promo_types));
    memset(promo_counts, 0, sizeof(promo_counts));
    memset(promo_vals, 0, sizeof(promo_vals));
#define PROMO_STR(key, text_val)                                               \
  do {                                                                         \
    if ((text_val).len > 0) {                                                  \
      char _buf[NWCHEMC_DIRECT_SET_VALUE_LEN];                                 \
      size_t _n = (size_t)(text_val).len;                                      \
      if (_n >= sizeof(_buf))                                                  \
        _n = sizeof(_buf) - 1;                                                 \
      if (!(text_val).str)                                                     \
        return -1;                                                             \
      memcpy(_buf, (text_val).str, _n);                                        \
      _buf[_n] = '\0';                                                         \
      if (append_owned_direct_string_value(                                    \
              promo_str_keys, promo_str_vals, PROMO_CAP, &promo_str_count,      \
              promo_str_key_storage, promo_str_val_storage, (key), _buf) != 0)  \
        return -1;                                                             \
    }                                                                          \
  } while (0)
#define PROMO_INT(key, ival)                                                   \
  do {                                                                         \
    int _iv[1] = {(ival)};                                                     \
    if (append_direct_integer_values(                                          \
            promo_keys, promo_types, promo_counts, promo_vals, PROMO_CAP,       \
            NWCHEMC_DIRECT_SET_VALUE_MAX, &promo_count, promo_key_storage,      \
            promo_value_storage, (key), _iv, 1) != 0)                          \
      return -1;                                                               \
  } while (0)
#define PROMO_DBL(key, dval)                                                   \
  do {                                                                         \
    double _dv[1] = {(dval)};                                                  \
    if (append_direct_double_values(                                           \
            promo_keys, promo_types, promo_counts, promo_vals, PROMO_CAP,       \
            NWCHEMC_DIRECT_SET_VALUE_MAX, &promo_count, promo_key_storage,      \
            promo_value_storage, (key), _dv, 1) != 0)                          \
      return -1;                                                               \
  } while (0)
#define PROMO_LOG(key, enabled)                                                \
  do {                                                                         \
    if (append_direct_logical_value(                                           \
            promo_keys, promo_types, promo_counts, promo_vals, PROMO_CAP,       \
            NWCHEMC_DIRECT_SET_VALUE_MAX, &promo_count, promo_key_storage,      \
            promo_value_storage, (key), (enabled)) != 0)                       \
      return -1;                                                               \
  } while (0)
#define PROMO_TOGGLE_LOG(key, toggle)                                          \
  do {                                                                         \
    if ((toggle) == NWChemToggle_enabled)                                      \
      PROMO_LOG((key), 1);                                                     \
    else if ((toggle) == NWChemToggle_disabled)                                \
      PROMO_LOG((key), 0);                                                     \
  } while (0)
#define PROMO_PROP(key, flag)                                                  \
  do {                                                                         \
    if (flag)                                                                  \
      PROMO_INT((key), 1);                                                     \
  } while (0)

    PROMO_STR("scf:scftype", scf_wavefunction);
    PROMO_STR("scf:input vectors", scf_vectors_in);
    PROMO_STR("scf:output vectors", scf_vectors_out);
    if (scf_has_nopen)
      PROMO_INT("scf:nopen", scf_nopen);
    PROMO_TOGGLE_LOG("scf:diis", scf_diis);
    if (scf_diis_bas > 0)
      PROMO_INT("scf:diisbas", scf_diis_bas);
    if (scf_maxsub > 0)
      PROMO_INT("scf:maxsub", scf_maxsub);
    PROMO_TOGGLE_LOG("scf:lock", scf_lock);
    PROMO_TOGGLE_LOG("scf:adapt", scf_adapt);
    if (scf_noscf == NWChemToggle_enabled)
      PROMO_LOG("scf:noscf", 1);
    if (scf_semidirect_filesize > 0)
      PROMO_INT("int2e:filesize", scf_semidirect_filesize);
    if (scf_semidirect_memsize > 0)
      PROMO_INT("int2e:memsize", scf_semidirect_memsize);

    if (dft_iterations > 0)
      PROMO_INT("dft:iterations", dft_iterations);
    if (dft_energy_conv > 0.0)
      PROMO_DBL("dft:e_conv", dft_energy_conv);
    if (dft_density_conv > 0.0)
      PROMO_DBL("dft:d_conv", dft_density_conv);
    if (dft_gradient_conv > 0.0)
      PROMO_DBL("dft:g_conv", dft_gradient_conv);
    if (dft_odft)
      PROMO_INT("dft:ipol", 2);
    PROMO_TOGGLE_LOG("dft:diis", dft_diis);
    if (dft_nfock > 0)
      PROMO_INT("dft:nfock", dft_nfock);
    if (dft_level_shift > 0.0) {
      PROMO_DBL("dft:rlshift", dft_level_shift);
      PROMO_LOG("dft:levelshift", 1);
    }
    PROMO_STR("dft:input vectors", dft_vectors_in);
    PROMO_STR("dft:output vectors", dft_vectors_out);

    PROMO_PROP("prop:dipole", prop_dipole);
    PROMO_PROP("prop:mulliken", prop_mulliken);
    PROMO_PROP("prop:quadrupole", prop_quad);
    PROMO_PROP("prop:octupole", prop_oct);
    PROMO_PROP("prop:esp", prop_esp);
    PROMO_PROP("prop:efield", prop_efield);
    PROMO_PROP("prop:efieldgrad", prop_efield_grad);
    PROMO_PROP("prop:electrondensity", prop_edens);
    PROMO_PROP("prop:spindensity", prop_sdens);
    PROMO_PROP("prop:spinpopulation", prop_spop);
    PROMO_PROP("prop:shldopt", prop_shield);
    PROMO_PROP("prop:hypopt", prop_hyp);
    PROMO_PROP("prop:polfromsos", prop_pol);

    if (mp2_freeze_core > 0)
      PROMO_INT("mp2:number frozen core", mp2_freeze_core);
    if (mp2_freeze_virt > 0)
      PROMO_INT("mp2:number frozen virtual", mp2_freeze_virt);
    if (mp2_aotol2e > 0.0)
      PROMO_DBL("mp2:aotol2e", mp2_aotol2e);
    if (mp2_aotol2e_fock > 0.0)
      PROMO_DBL("mp2:aotol2e fock", mp2_aotol2e_fock);
    if (mp2_backtol > 0.0)
      PROMO_DBL("mp2:backtol", mp2_backtol);
    if (mp2_fss > 0.0)
      PROMO_DBL("mp2:fss", mp2_fss);
    if (mp2_fos > 0.0)
      PROMO_DBL("mp2:fos", mp2_fos);
    if (mp2_scs == NWChemToggle_enabled)
      PROMO_LOG("mp2:scs", 1);
    if (mp2_scratch > 0.0)
      PROMO_DBL("mp2:scratchdisk", mp2_scratch);

    if (tddft_nroots > 0)
      PROMO_INT("tddft:nroots", tddft_nroots);
    PROMO_TOGGLE_LOG("tddft:tda", tddft_tda);
    if (tddft_maxiter > 0)
      PROMO_INT("tddft:maxiter", tddft_maxiter);
    if (tddft_thresh > 0.0)
      PROMO_DBL("tddft:thresh", tddft_thresh);
    if (tddft_maxvecs > 0)
      PROMO_INT("tddft:maxvecs", tddft_maxvecs);
    PROMO_TOGGLE_LOG("tddft:singlet", tddft_singlet);
    PROMO_TOGGLE_LOG("tddft:triplet", tddft_triplet);
    if (tddft_target > 0)
      PROMO_INT("tddft:target", tddft_target);
    PROMO_STR("tddft:targetsym", tddft_target_sym);
    PROMO_TOGGLE_LOG("tddft:symmetry", tddft_symmetry);
    if (tddft_algorithm > 0)
      PROMO_INT("tddft:algorithm", tddft_algorithm);
    if (tddft_ecut > 0.0) {
      PROMO_DBL("tddft:ecut", tddft_ecut);
      PROMO_LOG("tddft:lecut", 1);
    }
    /* TDDFT CI / grad RTDB only when identity theory is tddft and stanza set work. */
    if (span_starts_with(theory, theory_len, "tddft") &&
        (tddft_nroots > 0 || tddft_target > 0)) {
      int grad_root = tddft_target > 0 ? tddft_target : 1;
      PROMO_LOG("tddft:lcivecs", 1);
      PROMO_INT("tddft_grad:isinglet_roots", grad_root);
      PROMO_INT("tddft_grad:itriplet_roots", grad_root);
      PROMO_INT("tddft_grad:iroots", grad_root);
    }

#undef PROMO_STR
#undef PROMO_INT
#undef PROMO_DBL
#undef PROMO_LOG
#undef PROMO_TOGGLE_LOG
#undef PROMO_PROP

    for (size_t i = 0; i < promo_str_count; ++i) {
      copy_text_record(promo_str_packed_keys + i * NWCHEMC_DIRECT_SET_KEY_LEN,
                       NWCHEMC_DIRECT_SET_KEY_LEN, promo_str_keys[i]);
      copy_text_record(
          promo_str_packed_values + i * NWCHEMC_DIRECT_SET_VALUE_LEN,
          NWCHEMC_DIRECT_SET_VALUE_LEN, promo_str_vals[i]);
    }
    if (promo_str_count > 0 &&
        nwchemc_embed_set_rtdb_strings(promo_str_packed_keys,
                                       promo_str_packed_values,
                                       (int)promo_str_count) != 0)
      return -1;
    for (size_t i = 0; i < promo_count; ++i) {
      copy_text_record(promo_typed_packed_keys + i * NWCHEMC_DIRECT_SET_KEY_LEN,
                       NWCHEMC_DIRECT_SET_KEY_LEN, promo_keys[i]);
      for (int j = 0; j < promo_counts[i]; ++j) {
        copy_text_record(promo_typed_packed_values +
                             (i * NWCHEMC_DIRECT_SET_VALUE_MAX + (size_t)j) *
                                 NWCHEMC_DIRECT_SET_VALUE_LEN,
                         NWCHEMC_DIRECT_SET_VALUE_LEN,
                         promo_vals[i * NWCHEMC_DIRECT_SET_VALUE_MAX +
                                    (size_t)j]);
      }
    }
    if (promo_count > 0 &&
        nwchemc_embed_set_rtdb_values(promo_typed_packed_keys, promo_types,
                                      promo_counts, promo_typed_packed_values,
                                      (int)promo_count) != 0)
      return -1;
  }
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
  if (params_blob_matches_applied(params_capnp, params_capnp_size_bytes))
    return 0;
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
  remember_applied_params_blob(params_capnp, params_capnp_size_bytes,
                               params.charge, params.multiplicity);
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

  int ch = g_cached_charge;
  int mult = g_cached_mult;
  if (!params_blob_matches_applied(params_capnp, params_capnp_size_bytes)) {
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
    ch = params.charge;
    mult = params.multiplicity > 0 ? params.multiplicity : 1;
    remember_applied_params_blob(params_capnp, params_capnp_size_bytes, ch,
                                 mult);
    nwchemc_params_release(&arena);
  }

  char errmsg[512];
  memset(errmsg, 0, sizeof(errmsg));
  int n = n_atoms;
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

NWChemCResult nwchemc_energy(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (n_atoms <= 0 || !positions_ang || !atomic_numbers) {
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
  remember_applied_params_blob(params_capnp, params_capnp_size_bytes, ch, mult);
  double eh = 0.0;
  int rc = nwchemc_embed_energy_only(&n, positions_ang, atomic_numbers, &ch,
                                     &mult, &eh, errmsg,
                                     (int)sizeof(errmsg) - 1);
  nwchemc_params_release(&arena);
  if (rc != 0) {
    snprintf(r.message, sizeof(r.message), "%s",
             errmsg[0] ? errmsg : "nwchem embed energy failed");
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
  g_active_session = NULL;

  char errmsg[512];
  memset(errmsg, 0, sizeof(errmsg));
  int n = n_atoms;
  int ch = params.charge;
  int mult = params.multiplicity > 0 ? params.multiplicity : 1;
  double eh = 0.0;
  int rc = nwchemc_embed_hessian(&n, positions_ang, atomic_numbers, &ch, &mult,
                                 hessian_h_bohr2, errmsg,
                                 (int)sizeof(errmsg) - 1);
  nwchemc_params_release(&arena);
  if (rc != 0) {
    snprintf(r.message, sizeof(r.message), "%s",
             errmsg[0] ? errmsg : "nwchem embed hessian failed");
    return r;
  }
  if (nwchemc_embed_last_energy(&eh) != 0) {
    snprintf(r.message, sizeof(r.message), "nwchem embed energy unavailable");
    return r;
  }
  r.ok = 1;
  r.energy_h = eh;
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

NWChemCResult nwchemc_polarizability(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *polarizability_au) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';

  if (n_atoms <= 0 || !positions_ang || !atomic_numbers ||
      !polarizability_au) {
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
  int rc = nwchemc_embed_polarizability(
      &n, positions_ang, atomic_numbers, &ch, &mult, &eh,
      polarizability_au, errmsg, (int)sizeof(errmsg) - 1);
  nwchemc_params_release(&arena);
  if (rc != 0) {
    snprintf(r.message, sizeof(r.message), "%s",
             errmsg[0] ? errmsg : "nwchem embed polarizability failed");
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

NWChemCResult nwchemc_stress(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *stress_au) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (n_atoms <= 0 || !positions_ang || !atomic_numbers || !stress_au ||
      !params_capnp || params_capnp_size_bytes == 0) {
    snprintf(r.message, sizeof(r.message), "invalid arguments");
    return r;
  }
  NWChemCSession *session =
      nwchemc_session_create(params_capnp, params_capnp_size_bytes);
  if (!session) {
    snprintf(r.message, sizeof(r.message), "embed config failed");
    return r;
  }
  r = nwchemc_session_stress(session, n_atoms, positions_ang, atomic_numbers,
                             stress_au);
  nwchemc_session_destroy(session);
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
  double eh = 0.0;
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
  if (nwchemc_embed_last_energy(&eh) != 0) {
    snprintf(r.message, sizeof(r.message), "nwchem embed energy unavailable");
    return r;
  }
  r.ok = 1;
  r.energy_h = eh;
  snprintf(r.message, sizeof(r.message), "ok");
  return r;
}

static NWChemCResult coordinate_config_fail(const char *message) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  snprintf(r.message, sizeof(r.message), "%s", message);
  return r;
}

static int coordinate_config_args_invalid(int n_atoms,
                                          const double *positions_ang,
                                          const int *atomic_numbers,
                                          const void *config_capnp,
                                          size_t config_capnp_size_bytes) {
  return n_atoms <= 0 || !positions_ang || !atomic_numbers || !config_capnp ||
         config_capnp_size_bytes == 0;
}

NWChemCResult nwchemc_energy_gradient_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *grad_h_bohr) {
  if (coordinate_config_args_invalid(n_atoms, positions_ang, atomic_numbers,
                                     config_capnp,
                                     config_capnp_size_bytes) ||
      !grad_h_bohr)
    return coordinate_config_fail("invalid arguments");

  NWChemCSession *session =
      nwchemc_session_create_from_config(config_capnp, config_capnp_size_bytes);
  if (!session)
    return coordinate_config_fail("embed config failed");
  NWChemCResult r = nwchemc_session_energy_gradient(
      session, n_atoms, positions_ang, atomic_numbers, grad_h_bohr);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_energy_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes) {
  if (coordinate_config_args_invalid(n_atoms, positions_ang, atomic_numbers,
                                     config_capnp,
                                     config_capnp_size_bytes))
    return coordinate_config_fail("invalid arguments");

  NWChemCSession *session =
      nwchemc_session_create_from_config(config_capnp, config_capnp_size_bytes);
  if (!session)
    return coordinate_config_fail("embed config failed");
  NWChemCResult r =
      nwchemc_session_energy(session, n_atoms, positions_ang, atomic_numbers);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_energy_forces_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *forces_h_bohr) {
  if (coordinate_config_args_invalid(n_atoms, positions_ang, atomic_numbers,
                                     config_capnp,
                                     config_capnp_size_bytes) ||
      !forces_h_bohr)
    return coordinate_config_fail("invalid arguments");

  NWChemCSession *session =
      nwchemc_session_create_from_config(config_capnp, config_capnp_size_bytes);
  if (!session)
    return coordinate_config_fail("embed config failed");
  NWChemCResult r = nwchemc_session_energy_forces(
      session, n_atoms, positions_ang, atomic_numbers, forces_h_bohr);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_hessian_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *hessian_h_bohr2) {
  if (coordinate_config_args_invalid(n_atoms, positions_ang, atomic_numbers,
                                     config_capnp,
                                     config_capnp_size_bytes) ||
      !hessian_h_bohr2)
    return coordinate_config_fail("invalid arguments");

  NWChemCSession *session =
      nwchemc_session_create_from_config(config_capnp, config_capnp_size_bytes);
  if (!session)
    return coordinate_config_fail("embed config failed");
  NWChemCResult r = nwchemc_session_hessian(
      session, n_atoms, positions_ang, atomic_numbers, hessian_h_bohr2);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_dipole_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *dipole_au) {
  if (coordinate_config_args_invalid(n_atoms, positions_ang, atomic_numbers,
                                     config_capnp,
                                     config_capnp_size_bytes) ||
      !dipole_au)
    return coordinate_config_fail("invalid arguments");

  NWChemCSession *session =
      nwchemc_session_create_from_config(config_capnp, config_capnp_size_bytes);
  if (!session)
    return coordinate_config_fail("embed config failed");
  NWChemCResult r = nwchemc_session_dipole(
      session, n_atoms, positions_ang, atomic_numbers, dipole_au);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_polarizability_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *polarizability_au) {
  if (coordinate_config_args_invalid(n_atoms, positions_ang, atomic_numbers,
                                     config_capnp,
                                     config_capnp_size_bytes) ||
      !polarizability_au)
    return coordinate_config_fail("invalid arguments");

  NWChemCSession *session =
      nwchemc_session_create_from_config(config_capnp, config_capnp_size_bytes);
  if (!session)
    return coordinate_config_fail("embed config failed");
  NWChemCResult r = nwchemc_session_polarizability(
      session, n_atoms, positions_ang, atomic_numbers, polarizability_au);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_quadrupole_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *quadrupole_au) {
  if (coordinate_config_args_invalid(n_atoms, positions_ang, atomic_numbers,
                                     config_capnp,
                                     config_capnp_size_bytes) ||
      !quadrupole_au)
    return coordinate_config_fail("invalid arguments");

  NWChemCSession *session =
      nwchemc_session_create_from_config(config_capnp, config_capnp_size_bytes);
  if (!session)
    return coordinate_config_fail("embed config failed");
  NWChemCResult r = nwchemc_session_quadrupole(
      session, n_atoms, positions_ang, atomic_numbers, quadrupole_au);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_stress_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *stress_au) {
  if (coordinate_config_args_invalid(n_atoms, positions_ang, atomic_numbers,
                                     config_capnp,
                                     config_capnp_size_bytes) ||
      !stress_au)
    return coordinate_config_fail("invalid arguments");

  NWChemCSession *session =
      nwchemc_session_create_from_config(config_capnp, config_capnp_size_bytes);
  if (!session)
    return coordinate_config_fail("embed config failed");
  NWChemCResult r = nwchemc_session_stress(
      session, n_atoms, positions_ang, atomic_numbers, stress_au);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_optimize_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *optimized_positions_ang) {
  if (coordinate_config_args_invalid(n_atoms, positions_ang, atomic_numbers,
                                     config_capnp,
                                     config_capnp_size_bytes) ||
      !optimized_positions_ang)
    return coordinate_config_fail("invalid arguments");

  NWChemCSession *session =
      nwchemc_session_create_from_config(config_capnp, config_capnp_size_bytes);
  if (!session)
    return coordinate_config_fail("embed config failed");
  NWChemCResult r = nwchemc_session_optimize(
      session, n_atoms, positions_ang, atomic_numbers, optimized_positions_ang);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_frequencies_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *frequencies_cm1, double *intensities_au) {
  if (coordinate_config_args_invalid(n_atoms, positions_ang, atomic_numbers,
                                     config_capnp,
                                     config_capnp_size_bytes) ||
      !frequencies_cm1 || n_atoms > INT_MAX / 3)
    return coordinate_config_fail("invalid arguments");

  NWChemCSession *session =
      nwchemc_session_create_from_config(config_capnp, config_capnp_size_bytes);
  if (!session)
    return coordinate_config_fail("embed config failed");
  NWChemCResult r = nwchemc_session_frequencies(
      session, n_atoms, positions_ang, atomic_numbers, frequencies_cm1,
      intensities_au);
  nwchemc_session_destroy(session);
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

static int potential_config_root(const void *config_capnp,
                                 size_t config_capnp_size_bytes,
                                 struct capn *arena,
                                 PotentialConfig_ptr *config) {
  if (!config_capnp || config_capnp_size_bytes == 0 || !arena || !config)
    return -1;

  memset(arena, 0, sizeof(*arena));
  memset(config, 0, sizeof(*config));
  if (capn_init_mem(arena, (const uint8_t *)config_capnp,
                    config_capnp_size_bytes, 0) != 0)
    return -1;

  config->p = capn_getp(capn_root(arena), 0, 1);
  if (config->p.type != CAPN_STRUCT) {
    nwchemc_params_release(arena);
    memset(config, 0, sizeof(*config));
    return -1;
  }
  return 0;
}

static int potential_config_nwchem_root(const void *config_capnp,
                                        size_t config_capnp_size_bytes,
                                        struct capn *arena,
                                        NWChemParams_ptr *params_root,
                                        int *is_none) {
  if (!arena || !params_root || !is_none)
    return -1;
  memset(params_root, 0, sizeof(*params_root));
  *is_none = 0;

  PotentialConfig_ptr config_root;
  if (potential_config_root(config_capnp, config_capnp_size_bytes, arena,
                            &config_root) != 0)
    return -1;

  struct PotentialConfig config;
  memset(&config, 0, sizeof(config));
  read_PotentialConfig(&config, config_root);
  if (config.which == PotentialConfig_none) {
    *is_none = 1;
    return 0;
  }
  if (config.which != PotentialConfig_nwchem) {
    nwchemc_params_release(arena);
    return -1;
  }
  capn_resolve(&config.nwchem.p);
  if (config.nwchem.p.type != CAPN_STRUCT) {
    nwchemc_params_release(arena);
    return -1;
  }
  *params_root = config.nwchem;
  return 0;
}

static int write_nwchem_params_root_flat(NWChemParams_ptr params_root,
                                         unsigned char **params_capnp,
                                         size_t *params_capnp_size_bytes) {
  if (params_root.p.type == CAPN_NULL || !params_capnp ||
      !params_capnp_size_bytes)
    return -1;
  *params_capnp = NULL;
  *params_capnp_size_bytes = 0;

  struct capn arena;
  capn_init_malloc(&arena);
  capn_ptr root = capn_root(&arena);
  if (root.type == CAPN_NULL) {
    capn_free(&arena);
    return -1;
  }
  if (capn_setp(root, 0, params_root.p) != 0) {
    capn_free(&arena);
    return -1;
  }

  size_t capacity = 4096u;
  unsigned char *buffer = NULL;
  int written = -1;
  for (int attempt = 0; attempt < 16 && written < 0; ++attempt) {
    unsigned char *next = (unsigned char *)realloc(buffer, capacity);
    if (!next) {
      free(buffer);
      capn_free(&arena);
      return -1;
    }
    buffer = next;
    written = capn_write_mem(&arena, (uint8_t *)buffer, capacity, 0);
    if (written < 0) {
      if (capacity > SIZE_MAX / 2u) {
        free(buffer);
        capn_free(&arena);
        return -1;
      }
      capacity *= 2u;
    }
  }
  capn_free(&arena);
  if (written < 0) {
    free(buffer);
    return -1;
  }
  *params_capnp = buffer;
  *params_capnp_size_bytes = (size_t)written;
  return 0;
}

int nwchemc_configure(const void *config_capnp,
                      size_t config_capnp_size_bytes) {
  struct capn arena;
  NWChemParams_ptr params_root;
  int is_none = 0;
  if (potential_config_nwchem_root(config_capnp, config_capnp_size_bytes,
                                   &arena, &params_root, &is_none) != 0)
    return -1;
  if (is_none) {
    nwchemc_params_release(&arena);
    return 0;
  }
  int rc = apply_root_to_embed(params_root);
  if (rc == 0)
    g_active_session = NULL;
  nwchemc_params_release(&arena);
  return rc == 0 ? 0 : -1;
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
  *charge =
      session->step_state_override ? session->step_charge : session->charge;
  *multiplicity = session->step_state_override ? session->step_multiplicity
                                               : session->multiplicity;
}

static void session_set_step_state(NWChemCSession *session, int charge,
                                   int multiplicity) {
  session->step_state_override = 1;
  session->step_charge = charge;
  session->step_multiplicity = multiplicity;
}

static void session_clear_step_state(NWChemCSession *session) {
  session->step_state_override = 0;
}

static int force_input_electronic_state(NWChemCSession *session,
                                        ForceInput_ptr force_input,
                                        int *charge, int *multiplicity) {
  if (!session || force_input.p.type == CAPN_NULL || !charge || !multiplicity)
    return -1;

  struct ForceInput view;
  read_ForceInput(&view, force_input);
  *charge = session->charge;
  *multiplicity = session->multiplicity;
  if (view.hasCharge)
    *charge = view.charge;
  if (view.hasMultiplicity) {
    if (view.multiplicity <= 0)
      return -1;
    *multiplicity = view.multiplicity;
  }
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
  if (session_install_params(session, params_capnp, params_capnp_size_bytes) !=
      0) {
    free(session);
    return NULL;
  }
  return session;
}

NWChemCSession *
nwchemc_session_create_from_config(const void *config_capnp,
                                   size_t config_capnp_size_bytes) {
  struct capn arena;
  NWChemParams_ptr params_root;
  int is_none = 0;
  if (potential_config_nwchem_root(config_capnp, config_capnp_size_bytes,
                                   &arena, &params_root, &is_none) != 0)
    return NULL;
  if (is_none) {
    nwchemc_params_release(&arena);
    return NULL;
  }

  unsigned char *params_bytes = NULL;
  size_t params_size = 0;
  if (write_nwchem_params_root_flat(params_root, &params_bytes,
                                    &params_size) != 0) {
    nwchemc_params_release(&arena);
    return NULL;
  }
  nwchemc_params_release(&arena);

  NWChemCSession *session =
      (NWChemCSession *)calloc(1, sizeof(NWChemCSession));
  if (!session) {
    free(params_bytes);
    return NULL;
  }
  if (session_install_params(session, params_bytes, params_size) != 0) {
    free(params_bytes);
    free(session);
    return NULL;
  }
  free(params_bytes);
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

int nwchemc_session_configure(NWChemCSession *session,
                              const void *config_capnp,
                              size_t config_capnp_size_bytes) {
  if (!session || !config_capnp || config_capnp_size_bytes == 0)
    return -1;
  if (session->topology_atom_count != 0)
    return -1;

  struct capn arena;
  NWChemParams_ptr params_root;
  int is_none = 0;
  if (potential_config_nwchem_root(config_capnp, config_capnp_size_bytes,
                                   &arena, &params_root, &is_none) != 0)
    return -1;
  if (is_none) {
    nwchemc_params_release(&arena);
    return 0;
  }

  unsigned char *params_bytes = NULL;
  size_t params_size = 0;
  if (write_nwchem_params_root_flat(params_root, &params_bytes,
                                    &params_size) != 0) {
    nwchemc_params_release(&arena);
    return -1;
  }
  nwchemc_params_release(&arena);
  int rc = session_install_params(session, params_bytes, params_size);
  free(params_bytes);
  return rc;
}

int nwchemc_session_reset_topology(NWChemCSession *session) {
  if (!session)
    return -1;
  session_clear_step_state(session);
  session_clear_topology(session);
  return 0;
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

static NWChemCResult session_energy_only_cell(NWChemCSession *session,
                                              int n_atoms,
                                              const double *positions_ang,
                                              const int *atomic_numbers,
                                              const double *cell_ang,
                                              int has_cell) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!session || n_atoms <= 0 || !positions_ang || !atomic_numbers) {
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
  int rc = nwchemc_embed_energy_only_cell(
      &n, positions_ang, atomic_numbers, cell_arg, &cell_flag, &ch, &mult, &eh,
      errmsg, (int)sizeof(errmsg) - 1);
  if (rc != 0) {
    snprintf(r.message, sizeof(r.message), "%s",
             errmsg[0] ? errmsg : "nwchem embed energy failed");
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
  return session_energy_only_cell(session, n_atoms, positions_ang,
                                  atomic_numbers, NULL, 0);
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

static NWChemCResult session_polarizability_cell(
    NWChemCSession *session, int n_atoms, const double *positions_ang,
    const int *atomic_numbers, const double *cell_ang, int has_cell,
    double *polarizability_au) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!session || n_atoms <= 0 || !positions_ang || !atomic_numbers ||
      !polarizability_au) {
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
  int rc = nwchemc_embed_polarizability_cell(
      &n, positions_ang, atomic_numbers, cell_arg, &cell_flag, &ch, &mult, &eh,
      polarizability_au, errmsg, (int)sizeof(errmsg) - 1);
  if (rc != 0) {
    snprintf(r.message, sizeof(r.message), "%s",
             errmsg[0] ? errmsg : "nwchem embed polarizability failed");
    return r;
  }
  r.ok = 1;
  r.energy_h = eh;
  snprintf(r.message, sizeof(r.message), "ok");
  return r;
}

NWChemCResult nwchemc_session_polarizability(
    NWChemCSession *session, int n_atoms, const double *positions_ang,
    const int *atomic_numbers, double *polarizability_au) {
  return session_polarizability_cell(session, n_atoms, positions_ang,
                                     atomic_numbers, NULL, 0,
                                     polarizability_au);
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

static NWChemCResult session_stress_cell(NWChemCSession *session, int n_atoms,
                                         const double *positions_ang,
                                         const int *atomic_numbers,
                                         const double *cell_ang, int has_cell,
                                         double *stress_au) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!session || n_atoms <= 0 || !positions_ang || !atomic_numbers ||
      !stress_au) {
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
  int rc = nwchemc_embed_stress_cell(
      &n, positions_ang, atomic_numbers, cell_arg, &cell_flag, &ch, &mult, &eh,
      stress_au, errmsg, (int)sizeof(errmsg) - 1);
  if (rc != 0) {
    snprintf(r.message, sizeof(r.message), "%s",
             errmsg[0] ? errmsg : "nwchem embed stress failed");
    return r;
  }
  r.ok = 1;
  r.energy_h = eh;
  snprintf(r.message, sizeof(r.message), "ok");
  return r;
}

NWChemCResult nwchemc_session_stress(NWChemCSession *session, int n_atoms,
                                     const double *positions_ang,
                                     const int *atomic_numbers,
                                     double *stress_au) {
  return session_stress_cell(session, n_atoms, positions_ang, atomic_numbers,
                             NULL, 0, stress_au);
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
    double *frequencies_cm1, double *intensities_au,
    double *normal_modes, double *projected_frequencies_cm1,
    double *projected_intensities_au, double *thermochemistry) {
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
  double eh = 0.0;
  int rc = 0;
  if (normal_modes && thermochemistry) {
    rc = nwchemc_embed_frequencies_detail_cell(
        &n, positions_ang, atomic_numbers, cell_arg, &cell_flag, &ch, &mult,
        frequencies_cm1, intensity_arg, normal_modes, projected_frequencies_cm1,
        projected_intensities_au, thermochemistry, errmsg,
        (int)sizeof(errmsg) - 1);
  } else if (normal_modes) {
    rc = nwchemc_embed_frequencies_modes_cell(
        &n, positions_ang, atomic_numbers, cell_arg, &cell_flag, &ch, &mult,
        frequencies_cm1, intensity_arg, normal_modes, errmsg,
        (int)sizeof(errmsg) - 1);
  } else {
    rc = nwchemc_embed_frequencies_cell(
        &n, positions_ang, atomic_numbers, cell_arg, &cell_flag, &ch, &mult,
        frequencies_cm1, intensity_arg, errmsg, (int)sizeof(errmsg) - 1);
  }
  free(scratch_intensities);
  if (rc != 0) {
    snprintf(r.message, sizeof(r.message), "%s",
             errmsg[0] ? errmsg : "nwchem embed frequencies failed");
    return r;
  }
  if (nwchemc_embed_last_energy(&eh) != 0) {
    snprintf(r.message, sizeof(r.message), "nwchem embed energy unavailable");
    return r;
  }
  r.ok = 1;
  r.energy_h = eh;
  snprintf(r.message, sizeof(r.message), "ok");
  return r;
}

NWChemCResult nwchemc_session_frequencies(
    NWChemCSession *session, int n_atoms, const double *positions_ang,
    const int *atomic_numbers, double *frequencies_cm1,
    double *intensities_au) {
  return session_frequencies_cell(session, n_atoms, positions_ang,
                                  atomic_numbers, NULL, 0, frequencies_cm1,
                                  intensities_au, NULL, NULL, NULL, NULL);
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
  int step_charge = 0;
  int step_multiplicity = 1;
  if (force_input_electronic_state(session, force_input, &step_charge,
                                   &step_multiplicity) != 0) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput state");
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

  session_set_step_state(session, step_charge, step_multiplicity);
  r = session_energy_gradient_cell(session, (int)n_atoms,
                                   session->step_positions_ang,
                                   session->step_atomic_numbers, cell_ang,
                                   has_cell, forces_h_bohr);
  session_clear_step_state(session);
  if (r.ok) {
    for (size_t i = 0; i < n_atoms * 3u; ++i)
      forces_h_bohr[i] = -forces_h_bohr[i];
  }
  return r;
}

NWChemCResult nwchemc_session_calculate_gradient(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *gradient_h_bohr,
    size_t gradient_len) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!session || !force_input_capnp || force_input_capnp_size_bytes == 0 ||
      !gradient_h_bohr) {
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
      n_atoms > (size_t)INT_MAX || gradient_len < n_atoms * 3u) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }
  if (session_reserve_step_atoms(session, n_atoms) != 0) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "out of memory");
    return r;
  }
  int step_charge = 0;
  int step_multiplicity = 1;
  if (force_input_electronic_state(session, force_input, &step_charge,
                                   &step_multiplicity) != 0) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput state");
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

  session_set_step_state(session, step_charge, step_multiplicity);
  r = session_energy_gradient_cell(session, (int)n_atoms,
                                   session->step_positions_ang,
                                   session->step_atomic_numbers, cell_ang,
                                   has_cell, gradient_h_bohr);
  session_clear_step_state(session);
  return r;
}

static int force_input_step_atom_count(const void *force_input_capnp,
                                       size_t force_input_capnp_size_bytes,
                                       size_t *n_atoms);
static int force_input_result_energy_factor(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *energy_factor);

NWChemCResult nwchemc_session_calculate_energy(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!session || !force_input_capnp || force_input_capnp_size_bytes == 0) {
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
      n_atoms > (size_t)INT_MAX) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }
  if (session_reserve_step_atoms(session, n_atoms) != 0) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "out of memory");
    return r;
  }
  int step_charge = 0;
  int step_multiplicity = 1;
  if (force_input_electronic_state(session, force_input, &step_charge,
                                   &step_multiplicity) != 0) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput state");
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

  session_set_step_state(session, step_charge, step_multiplicity);
  r = session_energy_only_cell(session, (int)n_atoms, session->step_positions_ang,
                               session->step_atomic_numbers, cell_ang,
                               has_cell);
  session_clear_step_state(session);
  return r;
}

size_t nwchemc_energy_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes) {
  struct capn arena;
  ForceInput_ptr force_input;
  if (nwchemc_force_input_root(force_input_capnp, force_input_capnp_size_bytes,
                               &arena, &force_input) != 0)
    return 0;

  size_t n_atoms = 0;
  int has_cell = 0;
  if (nwchemc_force_input_atom_count(force_input, &n_atoms, &has_cell) != 0 ||
      n_atoms > (size_t)INT_MAX) {
    nwchemc_params_release(&arena);
    return 0;
  }
  (void)n_atoms;
  (void)has_cell;
  size_t result_size = nwchemc_potential_result_flat_size(0);
  nwchemc_params_release(&arena);
  return result_size;
}

NWChemCResult nwchemc_session_calculate_energy_result(
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

  size_t required_size = nwchemc_energy_result_size_for_force_input(
      force_input_capnp, force_input_capnp_size_bytes);
  *potential_result_capnp_size_bytes = required_size;
  if (required_size == 0) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }

  double energy_factor = 1.0;
  if (force_input_result_energy_factor(force_input_capnp,
                                       force_input_capnp_size_bytes,
                                       &energy_factor) != 0) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput result units");
    return r;
  }
  if (!potential_result_capnp ||
      potential_result_capnp_capacity_bytes < required_size) {
    snprintf(r.message, sizeof(r.message), "PotentialResult buffer too small");
    return r;
  }

  r = nwchemc_session_calculate_energy(
      session, force_input_capnp, force_input_capnp_size_bytes);
  if (r.ok &&
      nwchemc_potential_result_write(
          r.energy_h * energy_factor, NULL, 0, potential_result_capnp,
          potential_result_capnp_capacity_bytes,
          potential_result_capnp_size_bytes) != 0) {
    r.ok = 0;
    snprintf(r.message, sizeof(r.message), "PotentialResult write failed");
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
  int step_charge = 0;
  int step_multiplicity = 1;
  if (force_input_electronic_state(session, force_input, &step_charge,
                                   &step_multiplicity) != 0) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput state");
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
  session_set_step_state(session, step_charge, step_multiplicity);
  r = session_energy_gradient_cell(session, (int)n_atoms,
                                   session->step_positions_ang,
                                   session->step_atomic_numbers, cell_ang,
                                   has_cell, forces);
  session_clear_step_state(session);
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

NWChemCResult nwchemc_session_calculate_forces_result(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) {
  return nwchemc_session_calculate_result(
      session, force_input_capnp, force_input_capnp_size_bytes,
      potential_result_capnp, potential_result_capnp_capacity_bytes,
      potential_result_capnp_size_bytes);
}

typedef size_t (*nwchemc_result_size_fn)(const void *, size_t);
typedef NWChemCResult (*nwchemc_session_result_fn)(
    NWChemCSession *, const void *, size_t, void *, size_t, size_t *);

static NWChemCResult calculate_config_result(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes,
    nwchemc_result_size_fn size_fn,
    nwchemc_session_result_fn session_fn) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!config_capnp || config_capnp_size_bytes == 0 || !force_input_capnp ||
      force_input_capnp_size_bytes == 0 || !potential_result_capnp_size_bytes ||
      !size_fn || !session_fn) {
    snprintf(r.message, sizeof(r.message), "invalid arguments");
    return r;
  }
  *potential_result_capnp_size_bytes = 0;

  size_t required_size =
      size_fn(force_input_capnp, force_input_capnp_size_bytes);
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
      nwchemc_session_create_from_config(config_capnp, config_capnp_size_bytes);
  if (!session) {
    snprintf(r.message, sizeof(r.message), "embed config failed");
    return r;
  }
  r = session_fn(session, force_input_capnp, force_input_capnp_size_bytes,
                 potential_result_capnp, potential_result_capnp_capacity_bytes,
                 potential_result_capnp_size_bytes);
  nwchemc_session_destroy(session);
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

NWChemCResult nwchemc_calculate_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) {
  return calculate_config_result(
      config_capnp, config_capnp_size_bytes, force_input_capnp,
      force_input_capnp_size_bytes, potential_result_capnp,
      potential_result_capnp_capacity_bytes, potential_result_capnp_size_bytes,
      nwchemc_potential_result_size_for_force_input,
      nwchemc_session_calculate_result);
}

NWChemCResult nwchemc_calculate_forces_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) {
  return nwchemc_calculate_result(
      params_capnp, params_capnp_size_bytes, force_input_capnp,
      force_input_capnp_size_bytes, potential_result_capnp,
      potential_result_capnp_capacity_bytes, potential_result_capnp_size_bytes);
}

NWChemCResult nwchemc_calculate_forces_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) {
  return calculate_config_result(
      config_capnp, config_capnp_size_bytes, force_input_capnp,
      force_input_capnp_size_bytes, potential_result_capnp,
      potential_result_capnp_capacity_bytes, potential_result_capnp_size_bytes,
      nwchemc_forces_result_size_for_force_input,
      nwchemc_session_calculate_forces_result);
}

NWChemCResult nwchemc_calculate_energy_result(
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

  size_t required_size = nwchemc_energy_result_size_for_force_input(
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
  r = nwchemc_session_calculate_energy_result(
      session, force_input_capnp, force_input_capnp_size_bytes,
      potential_result_capnp, potential_result_capnp_capacity_bytes,
      potential_result_capnp_size_bytes);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_calculate_energy_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) {
  return calculate_config_result(
      config_capnp, config_capnp_size_bytes, force_input_capnp,
      force_input_capnp_size_bytes, potential_result_capnp,
      potential_result_capnp_capacity_bytes, potential_result_capnp_size_bytes,
      nwchemc_energy_result_size_for_force_input,
      nwchemc_session_calculate_energy_result);
}

NWChemCResult nwchemc_calculate_forces(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *forces_h_bohr, size_t forces_len) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!params_capnp || params_capnp_size_bytes == 0 || !force_input_capnp ||
      force_input_capnp_size_bytes == 0 || !forces_h_bohr) {
    snprintf(r.message, sizeof(r.message), "invalid arguments");
    return r;
  }
  size_t n_atoms = 0;
  if (force_input_step_atom_count(force_input_capnp,
                                  force_input_capnp_size_bytes,
                                  &n_atoms) != 0 ||
      n_atoms > SIZE_MAX / 3u || forces_len < n_atoms * 3u) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }

  NWChemCSession *session =
      nwchemc_session_create(params_capnp, params_capnp_size_bytes);
  if (!session) {
    snprintf(r.message, sizeof(r.message), "embed config failed");
    return r;
  }
  r = nwchemc_session_calculate_forces(
      session, force_input_capnp, force_input_capnp_size_bytes, forces_h_bohr,
      forces_len);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_calculate_forces_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *forces_h_bohr, size_t forces_len) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!config_capnp || config_capnp_size_bytes == 0 || !force_input_capnp ||
      force_input_capnp_size_bytes == 0 || !forces_h_bohr) {
    snprintf(r.message, sizeof(r.message), "invalid arguments");
    return r;
  }
  size_t n_atoms = 0;
  if (force_input_step_atom_count(force_input_capnp,
                                  force_input_capnp_size_bytes,
                                  &n_atoms) != 0 ||
      n_atoms > SIZE_MAX / 3u || forces_len < n_atoms * 3u) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }

  NWChemCSession *session =
      nwchemc_session_create_from_config(config_capnp, config_capnp_size_bytes);
  if (!session) {
    snprintf(r.message, sizeof(r.message), "embed config failed");
    return r;
  }
  r = nwchemc_session_calculate_forces(
      session, force_input_capnp, force_input_capnp_size_bytes, forces_h_bohr,
      forces_len);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_calculate_gradient(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *gradient_h_bohr, size_t gradient_len) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!params_capnp || params_capnp_size_bytes == 0 || !force_input_capnp ||
      force_input_capnp_size_bytes == 0 || !gradient_h_bohr) {
    snprintf(r.message, sizeof(r.message), "invalid arguments");
    return r;
  }
  size_t n_atoms = 0;
  if (force_input_step_atom_count(force_input_capnp,
                                  force_input_capnp_size_bytes,
                                  &n_atoms) != 0 ||
      n_atoms > SIZE_MAX / 3u || gradient_len < n_atoms * 3u) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }

  NWChemCSession *session =
      nwchemc_session_create(params_capnp, params_capnp_size_bytes);
  if (!session) {
    snprintf(r.message, sizeof(r.message), "embed config failed");
    return r;
  }
  r = nwchemc_session_calculate_gradient(
      session, force_input_capnp, force_input_capnp_size_bytes, gradient_h_bohr,
      gradient_len);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_calculate_gradient_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *gradient_h_bohr, size_t gradient_len) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!config_capnp || config_capnp_size_bytes == 0 || !force_input_capnp ||
      force_input_capnp_size_bytes == 0 || !gradient_h_bohr) {
    snprintf(r.message, sizeof(r.message), "invalid arguments");
    return r;
  }
  size_t n_atoms = 0;
  if (force_input_step_atom_count(force_input_capnp,
                                  force_input_capnp_size_bytes,
                                  &n_atoms) != 0 ||
      n_atoms > SIZE_MAX / 3u || gradient_len < n_atoms * 3u) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }

  NWChemCSession *session =
      nwchemc_session_create_from_config(config_capnp, config_capnp_size_bytes);
  if (!session) {
    snprintf(r.message, sizeof(r.message), "embed config failed");
    return r;
  }
  r = nwchemc_session_calculate_gradient(
      session, force_input_capnp, force_input_capnp_size_bytes, gradient_h_bohr,
      gradient_len);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_calculate_energy(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!params_capnp || params_capnp_size_bytes == 0 || !force_input_capnp ||
      force_input_capnp_size_bytes == 0) {
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

  NWChemCSession *session =
      nwchemc_session_create(params_capnp, params_capnp_size_bytes);
  if (!session) {
    snprintf(r.message, sizeof(r.message), "embed config failed");
    return r;
  }
  r = nwchemc_session_calculate_energy(
      session, force_input_capnp, force_input_capnp_size_bytes);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_calculate_energy_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!config_capnp || config_capnp_size_bytes == 0 || !force_input_capnp ||
      force_input_capnp_size_bytes == 0) {
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

  NWChemCSession *session =
      nwchemc_session_create_from_config(config_capnp, config_capnp_size_bytes);
  if (!session) {
    snprintf(r.message, sizeof(r.message), "embed config failed");
    return r;
  }
  r = nwchemc_session_calculate_energy(
      session, force_input_capnp, force_input_capnp_size_bytes);
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

size_t nwchemc_forces_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes) {
  return nwchemc_potential_result_size_for_force_input(
      force_input_capnp, force_input_capnp_size_bytes);
}

size_t nwchemc_gradient_result_size_for_force_input(
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
  size_t gradient_count = n_atoms * 3u;
  if (gradient_count > (size_t)INT_MAX) {
    nwchemc_params_release(&arena);
    return 0;
  }
  size_t result_size = nwchemc_gradient_result_flat_size(gradient_count);
  nwchemc_params_release(&arena);
  return result_size;
}

NWChemCResult nwchemc_session_calculate_gradient_result(
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
  size_t gradient_count = n_atoms * 3u;
  size_t required_size = nwchemc_gradient_result_flat_size(gradient_count);
  *potential_result_capnp_size_bytes = required_size;
  if (required_size == 0 || gradient_count > (size_t)INT_MAX) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }

  double energy_factor = 1.0;
  double gradient_factor = 1.0;
  if (nwchemc_force_input_result_factors(force_input, &energy_factor,
                                         &gradient_factor) != 0) {
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
  int step_charge = 0;
  int step_multiplicity = 1;
  if (force_input_electronic_state(session, force_input, &step_charge,
                                   &step_multiplicity) != 0) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput state");
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

  double *gradient = (double *)malloc(gradient_count * sizeof(*gradient));
  if (!gradient) {
    snprintf(r.message, sizeof(r.message), "out of memory");
    return r;
  }
  session_set_step_state(session, step_charge, step_multiplicity);
  r = session_energy_gradient_cell(session, (int)n_atoms,
                                   session->step_positions_ang,
                                   session->step_atomic_numbers, cell_ang,
                                   has_cell, gradient);
  session_clear_step_state(session);
  if (r.ok) {
    for (size_t i = 0; i < gradient_count; ++i)
      gradient[i] *= gradient_factor;
    if (nwchemc_potential_result_write_gradient(
            r.energy_h * energy_factor, gradient, gradient_count,
            potential_result_capnp, potential_result_capnp_capacity_bytes,
            potential_result_capnp_size_bytes) != 0) {
      r.ok = 0;
      snprintf(r.message, sizeof(r.message), "PotentialResult write failed");
    }
  }
  free(gradient);
  return r;
}

NWChemCResult nwchemc_calculate_gradient_result(
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

  size_t required_size = nwchemc_gradient_result_size_for_force_input(
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
  r = nwchemc_session_calculate_gradient_result(
      session, force_input_capnp, force_input_capnp_size_bytes,
      potential_result_capnp, potential_result_capnp_capacity_bytes,
      potential_result_capnp_size_bytes);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_calculate_gradient_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) {
  return calculate_config_result(
      config_capnp, config_capnp_size_bytes, force_input_capnp,
      force_input_capnp_size_bytes, potential_result_capnp,
      potential_result_capnp_capacity_bytes, potential_result_capnp_size_bytes,
      nwchemc_gradient_result_size_for_force_input,
      nwchemc_session_calculate_gradient_result);
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

static size_t fixed_property_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    size_t result_size) {
  struct capn arena;
  ForceInput_ptr force_input;
  if (nwchemc_force_input_root(force_input_capnp, force_input_capnp_size_bytes,
                               &arena, &force_input) != 0)
    return 0;

  size_t n_atoms = 0;
  int has_cell = 0;
  if (nwchemc_force_input_atom_count(force_input, &n_atoms, &has_cell) != 0 ||
      n_atoms > (size_t)INT_MAX) {
    nwchemc_params_release(&arena);
    return 0;
  }
  (void)n_atoms;
  (void)has_cell;
  nwchemc_params_release(&arena);
  return result_size;
}

size_t nwchemc_dipole_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes) {
  return fixed_property_result_size_for_force_input(
      force_input_capnp, force_input_capnp_size_bytes,
      nwchemc_dipole_result_flat_size());
}

size_t nwchemc_polarizability_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes) {
  return fixed_property_result_size_for_force_input(
      force_input_capnp, force_input_capnp_size_bytes,
      nwchemc_polarizability_result_flat_size());
}

size_t nwchemc_quadrupole_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes) {
  return fixed_property_result_size_for_force_input(
      force_input_capnp, force_input_capnp_size_bytes,
      nwchemc_quadrupole_result_flat_size());
}

size_t nwchemc_stress_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes) {
  return fixed_property_result_size_for_force_input(
      force_input_capnp, force_input_capnp_size_bytes,
      nwchemc_stress_result_flat_size());
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

static int force_input_result_energy_factor(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *energy_factor) {
  if (!force_input_capnp || force_input_capnp_size_bytes == 0 ||
      !energy_factor)
    return -1;
  struct capn arena;
  ForceInput_ptr force_input;
  if (nwchemc_force_input_root(force_input_capnp, force_input_capnp_size_bytes,
                               &arena, &force_input) != 0)
    return -1;
  double secondary_factor = 1.0;
  int rc = nwchemc_force_input_result_factors(force_input, energy_factor,
                                              &secondary_factor);
  nwchemc_params_release(&arena);
  return rc;
}

size_t nwchemc_optimize_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes) {
  size_t n_atoms = 0;
  if (force_input_step_atom_count(force_input_capnp,
                                  force_input_capnp_size_bytes,
                                  &n_atoms) != 0 ||
      n_atoms > SIZE_MAX / 3u)
    return 0;
  return nwchemc_optimize_result_flat_size(n_atoms * 3u);
}

size_t nwchemc_frequencies_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes) {
  size_t n_atoms = 0;
  if (force_input_step_atom_count(force_input_capnp,
                                  force_input_capnp_size_bytes,
                                  &n_atoms) != 0 ||
      n_atoms > SIZE_MAX / 3u)
    return 0;
  return nwchemc_frequencies_result_flat_size(n_atoms * 3u);
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
  double eh = 0.0;
  int rc = nwchemc_embed_hessian_cell(
      &n, positions_ang, atomic_numbers, cell_arg, &cell_flag, &ch, &mult,
      hessian_h_bohr2, errmsg, (int)sizeof(errmsg) - 1);
  if (rc != 0) {
    snprintf(r.message, sizeof(r.message), "%s",
             errmsg[0] ? errmsg : "nwchem embed hessian failed");
    return r;
  }
  if (nwchemc_embed_last_energy(&eh) != 0) {
    snprintf(r.message, sizeof(r.message), "nwchem embed energy unavailable");
    return r;
  }
  r.ok = 1;
  r.energy_h = eh;
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
  int step_charge = 0;
  int step_multiplicity = 1;
  if (force_input_electronic_state(session, force_input, &step_charge,
                                   &step_multiplicity) != 0) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput state");
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

  session_set_step_state(session, step_charge, step_multiplicity);
  r = session_hessian_cell(session, (int)n_atoms, session->step_positions_ang,
                           session->step_atomic_numbers, cell_ang, has_cell,
                           hessian_h_bohr2);
  session_clear_step_state(session);
  return r;
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
  int step_charge = 0;
  int step_multiplicity = 1;
  if (force_input_electronic_state(session, force_input, &step_charge,
                                   &step_multiplicity) != 0) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput state");
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
  session_set_step_state(session, step_charge, step_multiplicity);
  r = session_hessian_cell(session, (int)n_atoms, session->step_positions_ang,
                           session->step_atomic_numbers, cell_ang, has_cell,
                           hessian);
  session_clear_step_state(session);
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
  int step_charge = 0;
  int step_multiplicity = 1;
  if (force_input_electronic_state(session, force_input, &step_charge,
                                   &step_multiplicity) != 0) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput state");
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

  session_set_step_state(session, step_charge, step_multiplicity);
  r = session_dipole_cell(session, (int)n_atoms, session->step_positions_ang,
                          session->step_atomic_numbers, cell_ang, has_cell,
                          dipole_au);
  session_clear_step_state(session);
  return r;
}

NWChemCResult nwchemc_session_calculate_dipole_result(
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
  *potential_result_capnp_size_bytes =
      nwchemc_dipole_result_size_for_force_input(force_input_capnp,
                                                 force_input_capnp_size_bytes);
  if (*potential_result_capnp_size_bytes == 0) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }
  double energy_factor = 1.0;
  if (force_input_result_energy_factor(force_input_capnp,
                                       force_input_capnp_size_bytes,
                                       &energy_factor) != 0) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput result units");
    return r;
  }
  if (!potential_result_capnp ||
      potential_result_capnp_capacity_bytes <
          *potential_result_capnp_size_bytes) {
    snprintf(r.message, sizeof(r.message), "PotentialResult buffer too small");
    return r;
  }

  double dipole[3] = {0.0, 0.0, 0.0};
  r = nwchemc_session_calculate_dipole(
      session, force_input_capnp, force_input_capnp_size_bytes, dipole, 3);
  if (r.ok &&
      nwchemc_potential_result_write_dipole(
          r.energy_h * energy_factor, dipole, potential_result_capnp,
          potential_result_capnp_capacity_bytes,
          potential_result_capnp_size_bytes) != 0) {
    r.ok = 0;
    snprintf(r.message, sizeof(r.message), "PotentialResult write failed");
  }
  return r;
}

NWChemCResult nwchemc_session_calculate_polarizability(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *polarizability_au,
    size_t polarizability_len) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!session || !force_input_capnp || force_input_capnp_size_bytes == 0 ||
      !polarizability_au) {
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
      n_atoms > (size_t)INT_MAX || polarizability_len < 12u) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }
  if (session_reserve_step_atoms(session, n_atoms) != 0) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "out of memory");
    return r;
  }
  int step_charge = 0;
  int step_multiplicity = 1;
  if (force_input_electronic_state(session, force_input, &step_charge,
                                   &step_multiplicity) != 0) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput state");
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

  session_set_step_state(session, step_charge, step_multiplicity);
  r = session_polarizability_cell(
      session, (int)n_atoms, session->step_positions_ang,
      session->step_atomic_numbers, cell_ang, has_cell, polarizability_au);
  session_clear_step_state(session);
  return r;
}

NWChemCResult nwchemc_session_calculate_polarizability_result(
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
  *potential_result_capnp_size_bytes =
      nwchemc_polarizability_result_size_for_force_input(
          force_input_capnp, force_input_capnp_size_bytes);
  if (*potential_result_capnp_size_bytes == 0) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }
  double energy_factor = 1.0;
  if (force_input_result_energy_factor(force_input_capnp,
                                       force_input_capnp_size_bytes,
                                       &energy_factor) != 0) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput result units");
    return r;
  }
  if (!potential_result_capnp ||
      potential_result_capnp_capacity_bytes <
          *potential_result_capnp_size_bytes) {
    snprintf(r.message, sizeof(r.message), "PotentialResult buffer too small");
    return r;
  }

  double polarizability[12] = {0.0};
  r = nwchemc_session_calculate_polarizability(
      session, force_input_capnp, force_input_capnp_size_bytes,
      polarizability, 12);
  if (r.ok &&
      nwchemc_potential_result_write_polarizability(
          r.energy_h * energy_factor, polarizability, potential_result_capnp,
          potential_result_capnp_capacity_bytes,
          potential_result_capnp_size_bytes) != 0) {
    r.ok = 0;
    snprintf(r.message, sizeof(r.message), "PotentialResult write failed");
  }
  return r;
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
  int step_charge = 0;
  int step_multiplicity = 1;
  if (force_input_electronic_state(session, force_input, &step_charge,
                                   &step_multiplicity) != 0) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput state");
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

  session_set_step_state(session, step_charge, step_multiplicity);
  r = session_quadrupole_cell(
      session, (int)n_atoms, session->step_positions_ang,
      session->step_atomic_numbers, cell_ang, has_cell, quadrupole_au);
  session_clear_step_state(session);
  return r;
}

NWChemCResult nwchemc_session_calculate_quadrupole_result(
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
  *potential_result_capnp_size_bytes =
      nwchemc_quadrupole_result_size_for_force_input(
          force_input_capnp, force_input_capnp_size_bytes);
  if (*potential_result_capnp_size_bytes == 0) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }
  double energy_factor = 1.0;
  if (force_input_result_energy_factor(force_input_capnp,
                                       force_input_capnp_size_bytes,
                                       &energy_factor) != 0) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput result units");
    return r;
  }
  if (!potential_result_capnp ||
      potential_result_capnp_capacity_bytes <
          *potential_result_capnp_size_bytes) {
    snprintf(r.message, sizeof(r.message), "PotentialResult buffer too small");
    return r;
  }

  double quadrupole[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  r = nwchemc_session_calculate_quadrupole(
      session, force_input_capnp, force_input_capnp_size_bytes, quadrupole, 6);
  if (r.ok &&
      nwchemc_potential_result_write_quadrupole(
          r.energy_h * energy_factor, quadrupole, potential_result_capnp,
          potential_result_capnp_capacity_bytes,
          potential_result_capnp_size_bytes) != 0) {
    r.ok = 0;
    snprintf(r.message, sizeof(r.message), "PotentialResult write failed");
  }
  return r;
}

NWChemCResult nwchemc_session_calculate_stress(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *stress_au,
    size_t stress_len) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!session || !force_input_capnp || force_input_capnp_size_bytes == 0 ||
      !stress_au) {
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
      n_atoms > (size_t)INT_MAX || stress_len < 9u) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }
  if (session_reserve_step_atoms(session, n_atoms) != 0) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "out of memory");
    return r;
  }
  int step_charge = 0;
  int step_multiplicity = 1;
  if (force_input_electronic_state(session, force_input, &step_charge,
                                   &step_multiplicity) != 0) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput state");
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

  session_set_step_state(session, step_charge, step_multiplicity);
  r = session_stress_cell(session, (int)n_atoms, session->step_positions_ang,
                          session->step_atomic_numbers, cell_ang, has_cell,
                          stress_au);
  session_clear_step_state(session);
  return r;
}

NWChemCResult nwchemc_session_calculate_stress_result(
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
      n_atoms > (size_t)INT_MAX) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }
  (void)n_atoms;
  (void)has_cell;
  size_t required_size = nwchemc_stress_result_flat_size();
  *potential_result_capnp_size_bytes = required_size;
  if (required_size == 0) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }

  double energy_factor = 1.0;
  double stress_factor = 1.0;
  if (nwchemc_force_input_stress_result_factors(
          force_input, &energy_factor, &stress_factor) != 0) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput result units");
    return r;
  }
  nwchemc_params_release(&arena);

  if (!potential_result_capnp ||
      potential_result_capnp_capacity_bytes < required_size) {
    snprintf(r.message, sizeof(r.message), "PotentialResult buffer too small");
    return r;
  }

  double stress[9] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  r = nwchemc_session_calculate_stress(
      session, force_input_capnp, force_input_capnp_size_bytes, stress, 9);
  if (r.ok) {
    for (size_t i = 0; i < 9u; ++i)
      stress[i] *= stress_factor;
    if (nwchemc_potential_result_write_stress(
            r.energy_h * energy_factor, stress, potential_result_capnp,
            potential_result_capnp_capacity_bytes,
            potential_result_capnp_size_bytes) != 0) {
      r.ok = 0;
      snprintf(r.message, sizeof(r.message), "PotentialResult write failed");
    }
  }
  return r;
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
  int step_charge = 0;
  int step_multiplicity = 1;
  if (force_input_electronic_state(session, force_input, &step_charge,
                                   &step_multiplicity) != 0) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput state");
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

  session_set_step_state(session, step_charge, step_multiplicity);
  r = session_optimize_cell(
      session, (int)n_atoms, session->step_positions_ang,
      session->step_atomic_numbers, cell_ang, has_cell,
      optimized_positions_ang);
  session_clear_step_state(session);
  return r;
}

NWChemCResult nwchemc_session_calculate_optimize_result(
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
  (void)has_cell;
  size_t position_count = n_atoms * 3u;
  size_t required_size = nwchemc_optimize_result_flat_size(position_count);
  *potential_result_capnp_size_bytes = required_size;
  if (required_size == 0 || position_count > (size_t)INT_MAX) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }

  double energy_factor = 1.0;
  double position_factor = 1.0;
  if (nwchemc_force_input_position_result_factors(
          force_input, &energy_factor, &position_factor) != 0) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput result units");
    return r;
  }
  nwchemc_params_release(&arena);

  if (!potential_result_capnp ||
      potential_result_capnp_capacity_bytes < required_size) {
    snprintf(r.message, sizeof(r.message), "PotentialResult buffer too small");
    return r;
  }

  double *optimized_positions =
      (double *)malloc(position_count * sizeof(*optimized_positions));
  if (!optimized_positions) {
    snprintf(r.message, sizeof(r.message), "out of memory");
    return r;
  }
  r = nwchemc_session_calculate_optimize(
      session, force_input_capnp, force_input_capnp_size_bytes,
      optimized_positions, position_count);
  if (r.ok) {
    for (size_t i = 0; i < position_count; ++i)
      optimized_positions[i] *= position_factor;
    if (nwchemc_potential_result_write_optimized(
            r.energy_h * energy_factor, optimized_positions, position_count,
            potential_result_capnp, potential_result_capnp_capacity_bytes,
            potential_result_capnp_size_bytes) != 0) {
      r.ok = 0;
      snprintf(r.message, sizeof(r.message), "PotentialResult write failed");
    }
  }
  free(optimized_positions);
  return r;
}

static NWChemCResult session_calculate_frequencies_impl(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *frequencies_cm1,
    size_t frequencies_len, double *intensities_au, size_t intensities_len,
    double *normal_modes, size_t normal_modes_len,
    double *projected_frequencies_cm1, size_t projected_frequencies_len,
    double *projected_intensities_au, size_t projected_intensities_len,
    double *thermochemistry, size_t thermochemistry_len) {
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
      n_atoms == 0 || n_atoms > (size_t)INT_MAX ||
      n_atoms > SIZE_MAX / 3u) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }
  size_t frequency_count = n_atoms * 3u;
  if (frequencies_len < frequency_count ||
      (intensities_au && intensities_len < frequency_count) ||
      (normal_modes && frequency_count > SIZE_MAX / frequency_count) ||
      (normal_modes &&
       normal_modes_len < frequency_count * frequency_count) ||
      (projected_frequencies_cm1 &&
       projected_frequencies_len < frequency_count) ||
      (projected_intensities_au &&
       projected_intensities_len < frequency_count) ||
      (thermochemistry && thermochemistry_len < 5u)) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }
  if (session_reserve_step_atoms(session, n_atoms) != 0) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "out of memory");
    return r;
  }
  int step_charge = 0;
  int step_multiplicity = 1;
  if (force_input_electronic_state(session, force_input, &step_charge,
                                   &step_multiplicity) != 0) {
    nwchemc_params_release(&arena);
    snprintf(r.message, sizeof(r.message), "invalid ForceInput state");
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

  session_set_step_state(session, step_charge, step_multiplicity);
  r = session_frequencies_cell(
      session, (int)n_atoms, session->step_positions_ang,
      session->step_atomic_numbers, cell_ang, has_cell, frequencies_cm1,
      intensities_au, normal_modes, projected_frequencies_cm1,
      projected_intensities_au, thermochemistry);
  session_clear_step_state(session);
  return r;
}

NWChemCResult nwchemc_session_calculate_frequencies(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *frequencies_cm1,
    size_t frequencies_len, double *intensities_au, size_t intensities_len) {
  return session_calculate_frequencies_impl(
      session, force_input_capnp, force_input_capnp_size_bytes,
      frequencies_cm1, frequencies_len, intensities_au, intensities_len, NULL,
      0, NULL, 0, NULL, 0, NULL, 0);
}

NWChemCResult nwchemc_session_calculate_frequencies_detail(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *frequencies_cm1,
    size_t frequencies_len, double *intensities_au, size_t intensities_len,
    double *normal_modes, size_t normal_modes_len,
    double *projected_frequencies_cm1, size_t projected_frequencies_len,
    double *projected_intensities_au, size_t projected_intensities_len,
    double *thermochemistry, size_t thermochemistry_len) {
  return session_calculate_frequencies_impl(
      session, force_input_capnp, force_input_capnp_size_bytes,
      frequencies_cm1, frequencies_len, intensities_au, intensities_len,
      normal_modes, normal_modes_len, projected_frequencies_cm1,
      projected_frequencies_len, projected_intensities_au,
      projected_intensities_len, thermochemistry, thermochemistry_len);
}

NWChemCResult nwchemc_session_calculate_frequencies_result(
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

  size_t n_atoms = 0;
  if (force_input_step_atom_count(force_input_capnp,
                                  force_input_capnp_size_bytes,
                                  &n_atoms) != 0 ||
      n_atoms == 0 || n_atoms > SIZE_MAX / 3u) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }
  size_t frequency_count = n_atoms * 3u;
  size_t required_size =
      nwchemc_frequencies_result_flat_size(frequency_count);
  *potential_result_capnp_size_bytes = required_size;
  if (required_size == 0 || frequency_count > (size_t)INT_MAX) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }
  if (frequency_count > 0 && frequency_count > SIZE_MAX / frequency_count) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }
  size_t normal_mode_count = frequency_count * frequency_count;
  double energy_factor = 1.0;
  if (force_input_result_energy_factor(force_input_capnp,
                                       force_input_capnp_size_bytes,
                                       &energy_factor) != 0) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput result units");
    return r;
  }
  if (!potential_result_capnp ||
      potential_result_capnp_capacity_bytes < required_size) {
    snprintf(r.message, sizeof(r.message), "PotentialResult buffer too small");
    return r;
  }

  double *frequencies =
      (double *)malloc(frequency_count * sizeof(*frequencies));
  double *intensities =
      (double *)malloc(frequency_count * sizeof(*intensities));
  double *normal_modes =
      (double *)malloc(normal_mode_count * sizeof(*normal_modes));
  double *projected_frequencies =
      (double *)calloc(frequency_count, sizeof(*projected_frequencies));
  double *projected_intensities =
      (double *)calloc(frequency_count, sizeof(*projected_intensities));
  double thermochemistry[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
  if (!frequencies || !intensities || !normal_modes ||
      !projected_frequencies || !projected_intensities) {
    free(projected_intensities);
    free(projected_frequencies);
    free(normal_modes);
    free(intensities);
    free(frequencies);
    snprintf(r.message, sizeof(r.message), "out of memory");
    return r;
  }
  r = session_calculate_frequencies_impl(
      session, force_input_capnp, force_input_capnp_size_bytes, frequencies,
      frequency_count, intensities, frequency_count, normal_modes,
      normal_mode_count, projected_frequencies, frequency_count,
      projected_intensities, frequency_count, thermochemistry, 5u);
  if (r.ok) {
    double result_thermochemistry[5] = {
        thermochemistry[0] * energy_factor, thermochemistry[1] * energy_factor,
        thermochemistry[2] * energy_factor, thermochemistry[3],
        thermochemistry[4]};
    if (nwchemc_potential_result_write_frequencies(
            r.energy_h * energy_factor, frequencies, intensities, normal_modes,
            result_thermochemistry, projected_frequencies,
            projected_intensities, frequency_count, potential_result_capnp,
            potential_result_capnp_capacity_bytes,
            potential_result_capnp_size_bytes) != 0) {
      r.ok = 0;
      snprintf(r.message, sizeof(r.message), "PotentialResult write failed");
    }
  }
  free(projected_intensities);
  free(projected_frequencies);
  free(normal_modes);
  free(intensities);
  free(frequencies);
  return r;
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

NWChemCResult nwchemc_calculate_hessian_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) {
  return calculate_config_result(
      config_capnp, config_capnp_size_bytes, force_input_capnp,
      force_input_capnp_size_bytes, potential_result_capnp,
      potential_result_capnp_capacity_bytes, potential_result_capnp_size_bytes,
      nwchemc_hessian_result_size_for_force_input,
      nwchemc_session_calculate_hessian_result);
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

NWChemCResult nwchemc_calculate_hessian_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *hessian_h_bohr2, size_t hessian_len) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!config_capnp || config_capnp_size_bytes == 0 || !force_input_capnp ||
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
      nwchemc_session_create_from_config(config_capnp, config_capnp_size_bytes);
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

NWChemCResult nwchemc_calculate_dipole_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *dipole_au, size_t dipole_len) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!config_capnp || config_capnp_size_bytes == 0 || !force_input_capnp ||
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
  (void)n_atoms;

  NWChemCSession *session =
      nwchemc_session_create_from_config(config_capnp, config_capnp_size_bytes);
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

NWChemCResult nwchemc_calculate_dipole_result(
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
  *potential_result_capnp_size_bytes =
      nwchemc_dipole_result_size_for_force_input(force_input_capnp,
                                                 force_input_capnp_size_bytes);
  if (*potential_result_capnp_size_bytes == 0) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }
  if (!potential_result_capnp ||
      potential_result_capnp_capacity_bytes <
          *potential_result_capnp_size_bytes) {
    snprintf(r.message, sizeof(r.message), "PotentialResult buffer too small");
    return r;
  }

  NWChemCSession *session =
      nwchemc_session_create(params_capnp, params_capnp_size_bytes);
  if (!session) {
    snprintf(r.message, sizeof(r.message), "embed config failed");
    return r;
  }
  r = nwchemc_session_calculate_dipole_result(
      session, force_input_capnp, force_input_capnp_size_bytes,
      potential_result_capnp, potential_result_capnp_capacity_bytes,
      potential_result_capnp_size_bytes);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_calculate_dipole_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) {
  return calculate_config_result(
      config_capnp, config_capnp_size_bytes, force_input_capnp,
      force_input_capnp_size_bytes, potential_result_capnp,
      potential_result_capnp_capacity_bytes, potential_result_capnp_size_bytes,
      nwchemc_dipole_result_size_for_force_input,
      nwchemc_session_calculate_dipole_result);
}

NWChemCResult nwchemc_calculate_polarizability(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *polarizability_au, size_t polarizability_len) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!params_capnp || params_capnp_size_bytes == 0 || !force_input_capnp ||
      force_input_capnp_size_bytes == 0 || !polarizability_au) {
    snprintf(r.message, sizeof(r.message), "invalid arguments");
    return r;
  }
  size_t n_atoms = 0;
  if (force_input_step_atom_count(force_input_capnp,
                                  force_input_capnp_size_bytes,
                                  &n_atoms) != 0 ||
      polarizability_len < 12u) {
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
  r = nwchemc_session_calculate_polarizability(
      session, force_input_capnp, force_input_capnp_size_bytes,
      polarizability_au, polarizability_len);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_calculate_polarizability_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *polarizability_au, size_t polarizability_len) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!config_capnp || config_capnp_size_bytes == 0 || !force_input_capnp ||
      force_input_capnp_size_bytes == 0 || !polarizability_au) {
    snprintf(r.message, sizeof(r.message), "invalid arguments");
    return r;
  }
  size_t n_atoms = 0;
  if (force_input_step_atom_count(force_input_capnp,
                                  force_input_capnp_size_bytes,
                                  &n_atoms) != 0 ||
      polarizability_len < 12u) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }
  (void)n_atoms;

  NWChemCSession *session =
      nwchemc_session_create_from_config(config_capnp, config_capnp_size_bytes);
  if (!session) {
    snprintf(r.message, sizeof(r.message), "embed config failed");
    return r;
  }
  r = nwchemc_session_calculate_polarizability(
      session, force_input_capnp, force_input_capnp_size_bytes,
      polarizability_au, polarizability_len);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_calculate_polarizability_result(
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
  *potential_result_capnp_size_bytes =
      nwchemc_polarizability_result_size_for_force_input(
          force_input_capnp, force_input_capnp_size_bytes);
  if (*potential_result_capnp_size_bytes == 0) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }
  if (!potential_result_capnp ||
      potential_result_capnp_capacity_bytes <
          *potential_result_capnp_size_bytes) {
    snprintf(r.message, sizeof(r.message), "PotentialResult buffer too small");
    return r;
  }

  NWChemCSession *session =
      nwchemc_session_create(params_capnp, params_capnp_size_bytes);
  if (!session) {
    snprintf(r.message, sizeof(r.message), "embed config failed");
    return r;
  }
  r = nwchemc_session_calculate_polarizability_result(
      session, force_input_capnp, force_input_capnp_size_bytes,
      potential_result_capnp, potential_result_capnp_capacity_bytes,
      potential_result_capnp_size_bytes);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_calculate_polarizability_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) {
  return calculate_config_result(
      config_capnp, config_capnp_size_bytes, force_input_capnp,
      force_input_capnp_size_bytes, potential_result_capnp,
      potential_result_capnp_capacity_bytes, potential_result_capnp_size_bytes,
      nwchemc_polarizability_result_size_for_force_input,
      nwchemc_session_calculate_polarizability_result);
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

NWChemCResult nwchemc_calculate_quadrupole_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *quadrupole_au, size_t quadrupole_len) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!config_capnp || config_capnp_size_bytes == 0 || !force_input_capnp ||
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
      nwchemc_session_create_from_config(config_capnp, config_capnp_size_bytes);
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

NWChemCResult nwchemc_calculate_quadrupole_result(
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
  *potential_result_capnp_size_bytes =
      nwchemc_quadrupole_result_size_for_force_input(
          force_input_capnp, force_input_capnp_size_bytes);
  if (*potential_result_capnp_size_bytes == 0) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }
  if (!potential_result_capnp ||
      potential_result_capnp_capacity_bytes <
          *potential_result_capnp_size_bytes) {
    snprintf(r.message, sizeof(r.message), "PotentialResult buffer too small");
    return r;
  }

  NWChemCSession *session =
      nwchemc_session_create(params_capnp, params_capnp_size_bytes);
  if (!session) {
    snprintf(r.message, sizeof(r.message), "embed config failed");
    return r;
  }
  r = nwchemc_session_calculate_quadrupole_result(
      session, force_input_capnp, force_input_capnp_size_bytes,
      potential_result_capnp, potential_result_capnp_capacity_bytes,
      potential_result_capnp_size_bytes);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_calculate_quadrupole_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) {
  return calculate_config_result(
      config_capnp, config_capnp_size_bytes, force_input_capnp,
      force_input_capnp_size_bytes, potential_result_capnp,
      potential_result_capnp_capacity_bytes, potential_result_capnp_size_bytes,
      nwchemc_quadrupole_result_size_for_force_input,
      nwchemc_session_calculate_quadrupole_result);
}

NWChemCResult nwchemc_calculate_stress(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *stress_au, size_t stress_len) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!params_capnp || params_capnp_size_bytes == 0 || !force_input_capnp ||
      force_input_capnp_size_bytes == 0 || !stress_au) {
    snprintf(r.message, sizeof(r.message), "invalid arguments");
    return r;
  }
  size_t n_atoms = 0;
  if (force_input_step_atom_count(force_input_capnp,
                                  force_input_capnp_size_bytes,
                                  &n_atoms) != 0 ||
      stress_len < 9u) {
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
  r = nwchemc_session_calculate_stress(
      session, force_input_capnp, force_input_capnp_size_bytes, stress_au,
      stress_len);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_calculate_stress_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *stress_au, size_t stress_len) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!config_capnp || config_capnp_size_bytes == 0 || !force_input_capnp ||
      force_input_capnp_size_bytes == 0 || !stress_au) {
    snprintf(r.message, sizeof(r.message), "invalid arguments");
    return r;
  }
  size_t n_atoms = 0;
  if (force_input_step_atom_count(force_input_capnp,
                                  force_input_capnp_size_bytes,
                                  &n_atoms) != 0 ||
      stress_len < 9u) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }
  (void)n_atoms;

  NWChemCSession *session =
      nwchemc_session_create_from_config(config_capnp, config_capnp_size_bytes);
  if (!session) {
    snprintf(r.message, sizeof(r.message), "embed config failed");
    return r;
  }
  r = nwchemc_session_calculate_stress(
      session, force_input_capnp, force_input_capnp_size_bytes, stress_au,
      stress_len);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_calculate_stress_result(
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
  *potential_result_capnp_size_bytes =
      nwchemc_stress_result_size_for_force_input(force_input_capnp,
                                                 force_input_capnp_size_bytes);
  if (*potential_result_capnp_size_bytes == 0) {
    snprintf(r.message, sizeof(r.message), "invalid ForceInput geometry");
    return r;
  }
  if (!potential_result_capnp ||
      potential_result_capnp_capacity_bytes <
          *potential_result_capnp_size_bytes) {
    snprintf(r.message, sizeof(r.message), "PotentialResult buffer too small");
    return r;
  }

  NWChemCSession *session =
      nwchemc_session_create(params_capnp, params_capnp_size_bytes);
  if (!session) {
    snprintf(r.message, sizeof(r.message), "embed config failed");
    return r;
  }
  r = nwchemc_session_calculate_stress_result(
      session, force_input_capnp, force_input_capnp_size_bytes,
      potential_result_capnp, potential_result_capnp_capacity_bytes,
      potential_result_capnp_size_bytes);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_calculate_stress_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) {
  return calculate_config_result(
      config_capnp, config_capnp_size_bytes, force_input_capnp,
      force_input_capnp_size_bytes, potential_result_capnp,
      potential_result_capnp_capacity_bytes, potential_result_capnp_size_bytes,
      nwchemc_stress_result_size_for_force_input,
      nwchemc_session_calculate_stress_result);
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

NWChemCResult nwchemc_calculate_optimize_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *optimized_positions_ang, size_t optimized_positions_len) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!config_capnp || config_capnp_size_bytes == 0 || !force_input_capnp ||
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
      nwchemc_session_create_from_config(config_capnp, config_capnp_size_bytes);
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

NWChemCResult nwchemc_calculate_optimize_result(
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

  size_t required_size = nwchemc_optimize_result_size_for_force_input(
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
  r = nwchemc_session_calculate_optimize_result(
      session, force_input_capnp, force_input_capnp_size_bytes,
      potential_result_capnp, potential_result_capnp_capacity_bytes,
      potential_result_capnp_size_bytes);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_calculate_optimize_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) {
  return calculate_config_result(
      config_capnp, config_capnp_size_bytes, force_input_capnp,
      force_input_capnp_size_bytes, potential_result_capnp,
      potential_result_capnp_capacity_bytes, potential_result_capnp_size_bytes,
      nwchemc_optimize_result_size_for_force_input,
      nwchemc_session_calculate_optimize_result);
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

NWChemCResult nwchemc_calculate_frequencies_detail(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *frequencies_cm1, size_t frequencies_len, double *intensities_au,
    size_t intensities_len, double *normal_modes, size_t normal_modes_len,
    double *projected_frequencies_cm1, size_t projected_frequencies_len,
    double *projected_intensities_au, size_t projected_intensities_len,
    double *thermochemistry, size_t thermochemistry_len) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!params_capnp || params_capnp_size_bytes == 0 || !force_input_capnp ||
      force_input_capnp_size_bytes == 0 || !frequencies_cm1) {
    snprintf(r.message, sizeof(r.message), "invalid arguments");
    return r;
  }
  NWChemCSession *session =
      nwchemc_session_create(params_capnp, params_capnp_size_bytes);
  if (!session) {
    snprintf(r.message, sizeof(r.message), "invalid NWChemParams message");
    return r;
  }
  r = nwchemc_session_calculate_frequencies_detail(
      session, force_input_capnp, force_input_capnp_size_bytes, frequencies_cm1,
      frequencies_len, intensities_au, intensities_len, normal_modes,
      normal_modes_len, projected_frequencies_cm1, projected_frequencies_len,
      projected_intensities_au, projected_intensities_len, thermochemistry,
      thermochemistry_len);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_calculate_frequencies_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *frequencies_cm1, size_t frequencies_len, double *intensities_au,
    size_t intensities_len) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!config_capnp || config_capnp_size_bytes == 0 || !force_input_capnp ||
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
      nwchemc_session_create_from_config(config_capnp, config_capnp_size_bytes);
  if (!session) {
    snprintf(r.message, sizeof(r.message), "embed config failed");
    return r;
  }
  r = nwchemc_session_calculate_frequencies(
      session, force_input_capnp, force_input_capnp_size_bytes, frequencies_cm1,
      frequencies_len, intensities_au, intensities_len);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_calculate_frequencies_detail_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *frequencies_cm1, size_t frequencies_len, double *intensities_au,
    size_t intensities_len, double *normal_modes, size_t normal_modes_len,
    double *projected_frequencies_cm1, size_t projected_frequencies_len,
    double *projected_intensities_au, size_t projected_intensities_len,
    double *thermochemistry, size_t thermochemistry_len) {
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  r.message[0] = '\0';
  if (!config_capnp || config_capnp_size_bytes == 0 || !force_input_capnp ||
      force_input_capnp_size_bytes == 0 || !frequencies_cm1) {
    snprintf(r.message, sizeof(r.message), "invalid arguments");
    return r;
  }
  NWChemCSession *session =
      nwchemc_session_create_from_config(config_capnp, config_capnp_size_bytes);
  if (!session) {
    snprintf(r.message, sizeof(r.message), "embed config failed");
    return r;
  }
  r = nwchemc_session_calculate_frequencies_detail(
      session, force_input_capnp, force_input_capnp_size_bytes, frequencies_cm1,
      frequencies_len, intensities_au, intensities_len, normal_modes,
      normal_modes_len, projected_frequencies_cm1, projected_frequencies_len,
      projected_intensities_au, projected_intensities_len, thermochemistry,
      thermochemistry_len);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_calculate_frequencies_result(
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

  size_t required_size = nwchemc_frequencies_result_size_for_force_input(
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
  r = nwchemc_session_calculate_frequencies_result(
      session, force_input_capnp, force_input_capnp_size_bytes,
      potential_result_capnp, potential_result_capnp_capacity_bytes,
      potential_result_capnp_size_bytes);
  nwchemc_session_destroy(session);
  return r;
}

NWChemCResult nwchemc_calculate_frequencies_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) {
  return calculate_config_result(
      config_capnp, config_capnp_size_bytes, force_input_capnp,
      force_input_capnp_size_bytes, potential_result_capnp,
      potential_result_capnp_capacity_bytes, potential_result_capnp_size_bytes,
      nwchemc_frequencies_result_size_for_force_input,
      nwchemc_session_calculate_frequencies_result);
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

int nwchemc_configure(const void *config_capnp,
                      size_t config_capnp_size_bytes) {
  (void)config_capnp;
  (void)config_capnp_size_bytes;
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

NWChemCResult nwchemc_polarizability(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *polarizability_au) {
  (void)n_atoms;
  (void)positions_ang;
  (void)atomic_numbers;
  (void)params_capnp;
  (void)params_capnp_size_bytes;
  (void)polarizability_au;
  NWChemCResult r;
  r.ok = 0;
  r.energy_h = 0.0;
  snprintf(r.message, sizeof(r.message), "compiled without NWCHEMC_HAS_NWCHEM");
  return r;
}

NWChemCResult nwchemc_stress(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *stress_au) {
  (void)n_atoms;
  (void)positions_ang;
  (void)atomic_numbers;
  (void)params_capnp;
  (void)params_capnp_size_bytes;
  (void)stress_au;
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

NWChemCSession *
nwchemc_session_create_from_config(const void *config_capnp,
                                   size_t config_capnp_size_bytes) {
  (void)config_capnp;
  (void)config_capnp_size_bytes;
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

int nwchemc_session_configure(NWChemCSession *session,
                              const void *config_capnp,
                              size_t config_capnp_size_bytes) {
  (void)session;
  (void)config_capnp;
  (void)config_capnp_size_bytes;
  return -1;
}

int nwchemc_session_reset_topology(NWChemCSession *session) {
  (void)session;
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

NWChemCResult nwchemc_energy_gradient_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *grad_h_bohr) {
  (void)n_atoms;
  (void)positions_ang;
  (void)atomic_numbers;
  (void)config_capnp;
  (void)config_capnp_size_bytes;
  (void)grad_h_bohr;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_energy_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes) {
  (void)n_atoms;
  (void)positions_ang;
  (void)atomic_numbers;
  (void)config_capnp;
  (void)config_capnp_size_bytes;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_energy_forces_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *forces_h_bohr) {
  (void)n_atoms;
  (void)positions_ang;
  (void)atomic_numbers;
  (void)config_capnp;
  (void)config_capnp_size_bytes;
  (void)forces_h_bohr;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_hessian_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *hessian_h_bohr2) {
  (void)n_atoms;
  (void)positions_ang;
  (void)atomic_numbers;
  (void)config_capnp;
  (void)config_capnp_size_bytes;
  (void)hessian_h_bohr2;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_dipole_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *dipole_au) {
  (void)n_atoms;
  (void)positions_ang;
  (void)atomic_numbers;
  (void)config_capnp;
  (void)config_capnp_size_bytes;
  (void)dipole_au;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_polarizability_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *polarizability_au) {
  (void)n_atoms;
  (void)positions_ang;
  (void)atomic_numbers;
  (void)config_capnp;
  (void)config_capnp_size_bytes;
  (void)polarizability_au;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_quadrupole_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *quadrupole_au) {
  (void)n_atoms;
  (void)positions_ang;
  (void)atomic_numbers;
  (void)config_capnp;
  (void)config_capnp_size_bytes;
  (void)quadrupole_au;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_stress_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *stress_au) {
  (void)n_atoms;
  (void)positions_ang;
  (void)atomic_numbers;
  (void)config_capnp;
  (void)config_capnp_size_bytes;
  (void)stress_au;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_optimize_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *optimized_positions_ang) {
  (void)n_atoms;
  (void)positions_ang;
  (void)atomic_numbers;
  (void)config_capnp;
  (void)config_capnp_size_bytes;
  (void)optimized_positions_ang;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_frequencies_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *frequencies_cm1, double *intensities_au) {
  (void)n_atoms;
  (void)positions_ang;
  (void)atomic_numbers;
  (void)config_capnp;
  (void)config_capnp_size_bytes;
  (void)frequencies_cm1;
  (void)intensities_au;
  return no_nwchem_fail();
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

NWChemCResult nwchemc_session_polarizability(
    NWChemCSession *session, int n_atoms, const double *positions_ang,
    const int *atomic_numbers, double *polarizability_au) {
  (void)session;
  (void)n_atoms;
  (void)positions_ang;
  (void)atomic_numbers;
  (void)polarizability_au;
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

NWChemCResult nwchemc_session_stress(NWChemCSession *session, int n_atoms,
                                     const double *positions_ang,
                                     const int *atomic_numbers,
                                     double *stress_au) {
  (void)session;
  (void)n_atoms;
  (void)positions_ang;
  (void)atomic_numbers;
  (void)stress_au;
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

NWChemCResult nwchemc_session_calculate_gradient(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *gradient_h_bohr,
    size_t gradient_len) {
  (void)session;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)gradient_h_bohr;
  (void)gradient_len;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_session_calculate_energy(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes) {
  (void)session;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_session_calculate_energy_result(
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

NWChemCResult nwchemc_session_calculate_forces_result(
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

NWChemCResult nwchemc_session_calculate_gradient_result(
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

NWChemCResult nwchemc_calculate_forces_result(
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

NWChemCResult nwchemc_calculate_gradient_result(
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

NWChemCResult nwchemc_calculate_energy_result(
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

NWChemCResult nwchemc_calculate_energy_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) {
  (void)config_capnp;
  (void)config_capnp_size_bytes;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)potential_result_capnp;
  (void)potential_result_capnp_capacity_bytes;
  (void)potential_result_capnp_size_bytes;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_calculate_forces_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) {
  (void)config_capnp;
  (void)config_capnp_size_bytes;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)potential_result_capnp;
  (void)potential_result_capnp_capacity_bytes;
  (void)potential_result_capnp_size_bytes;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_calculate_gradient_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) {
  (void)config_capnp;
  (void)config_capnp_size_bytes;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)potential_result_capnp;
  (void)potential_result_capnp_capacity_bytes;
  (void)potential_result_capnp_size_bytes;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_calculate_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) {
  (void)config_capnp;
  (void)config_capnp_size_bytes;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)potential_result_capnp;
  (void)potential_result_capnp_capacity_bytes;
  (void)potential_result_capnp_size_bytes;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_calculate_hessian_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) {
  (void)config_capnp;
  (void)config_capnp_size_bytes;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)potential_result_capnp;
  (void)potential_result_capnp_capacity_bytes;
  (void)potential_result_capnp_size_bytes;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_calculate_dipole_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) {
  (void)config_capnp;
  (void)config_capnp_size_bytes;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)potential_result_capnp;
  (void)potential_result_capnp_capacity_bytes;
  (void)potential_result_capnp_size_bytes;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_calculate_polarizability_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) {
  (void)config_capnp;
  (void)config_capnp_size_bytes;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)potential_result_capnp;
  (void)potential_result_capnp_capacity_bytes;
  (void)potential_result_capnp_size_bytes;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_calculate_quadrupole_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) {
  (void)config_capnp;
  (void)config_capnp_size_bytes;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)potential_result_capnp;
  (void)potential_result_capnp_capacity_bytes;
  (void)potential_result_capnp_size_bytes;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_calculate_stress_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) {
  (void)config_capnp;
  (void)config_capnp_size_bytes;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)potential_result_capnp;
  (void)potential_result_capnp_capacity_bytes;
  (void)potential_result_capnp_size_bytes;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_calculate_optimize_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) {
  (void)config_capnp;
  (void)config_capnp_size_bytes;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)potential_result_capnp;
  (void)potential_result_capnp_capacity_bytes;
  (void)potential_result_capnp_size_bytes;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_calculate_frequencies_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) {
  (void)config_capnp;
  (void)config_capnp_size_bytes;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)potential_result_capnp;
  (void)potential_result_capnp_capacity_bytes;
  (void)potential_result_capnp_size_bytes;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_calculate_forces(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *forces_h_bohr, size_t forces_len) {
  (void)params_capnp;
  (void)params_capnp_size_bytes;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)forces_h_bohr;
  (void)forces_len;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_calculate_gradient(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *gradient_h_bohr, size_t gradient_len) {
  (void)params_capnp;
  (void)params_capnp_size_bytes;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)gradient_h_bohr;
  (void)gradient_len;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_calculate_forces_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *forces_h_bohr, size_t forces_len) {
  (void)config_capnp;
  (void)config_capnp_size_bytes;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)forces_h_bohr;
  (void)forces_len;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_calculate_gradient_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *gradient_h_bohr, size_t gradient_len) {
  (void)config_capnp;
  (void)config_capnp_size_bytes;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)gradient_h_bohr;
  (void)gradient_len;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_calculate_energy(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes) {
  (void)params_capnp;
  (void)params_capnp_size_bytes;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_calculate_energy_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes) {
  (void)config_capnp;
  (void)config_capnp_size_bytes;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
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

NWChemCResult nwchemc_calculate_hessian_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *hessian_h_bohr2, size_t hessian_len) {
  (void)config_capnp;
  (void)config_capnp_size_bytes;
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

NWChemCResult nwchemc_calculate_dipole_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *dipole_au, size_t dipole_len) {
  (void)config_capnp;
  (void)config_capnp_size_bytes;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)dipole_au;
  (void)dipole_len;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_calculate_polarizability(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *polarizability_au, size_t polarizability_len) {
  (void)params_capnp;
  (void)params_capnp_size_bytes;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)polarizability_au;
  (void)polarizability_len;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_calculate_polarizability_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *polarizability_au, size_t polarizability_len) {
  (void)config_capnp;
  (void)config_capnp_size_bytes;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)polarizability_au;
  (void)polarizability_len;
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

NWChemCResult nwchemc_calculate_quadrupole_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *quadrupole_au, size_t quadrupole_len) {
  (void)config_capnp;
  (void)config_capnp_size_bytes;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)quadrupole_au;
  (void)quadrupole_len;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_calculate_stress(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *stress_au, size_t stress_len) {
  (void)params_capnp;
  (void)params_capnp_size_bytes;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)stress_au;
  (void)stress_len;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_calculate_stress_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *stress_au, size_t stress_len) {
  (void)config_capnp;
  (void)config_capnp_size_bytes;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)stress_au;
  (void)stress_len;
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

NWChemCResult nwchemc_calculate_optimize_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *optimized_positions_ang, size_t optimized_positions_len) {
  (void)config_capnp;
  (void)config_capnp_size_bytes;
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

NWChemCResult nwchemc_calculate_frequencies_detail(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *frequencies_cm1, size_t frequencies_len, double *intensities_au,
    size_t intensities_len, double *normal_modes, size_t normal_modes_len,
    double *projected_frequencies_cm1, size_t projected_frequencies_len,
    double *projected_intensities_au, size_t projected_intensities_len,
    double *thermochemistry, size_t thermochemistry_len) {
  (void)params_capnp;
  (void)params_capnp_size_bytes;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)frequencies_cm1;
  (void)frequencies_len;
  (void)intensities_au;
  (void)intensities_len;
  (void)normal_modes;
  (void)normal_modes_len;
  (void)projected_frequencies_cm1;
  (void)projected_frequencies_len;
  (void)projected_intensities_au;
  (void)projected_intensities_len;
  (void)thermochemistry;
  (void)thermochemistry_len;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_calculate_frequencies_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *frequencies_cm1, size_t frequencies_len, double *intensities_au,
    size_t intensities_len) {
  (void)config_capnp;
  (void)config_capnp_size_bytes;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)frequencies_cm1;
  (void)frequencies_len;
  (void)intensities_au;
  (void)intensities_len;
  return no_nwchem_fail();
}

NWChemCResult nwchemc_calculate_frequencies_detail_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *frequencies_cm1, size_t frequencies_len, double *intensities_au,
    size_t intensities_len, double *normal_modes, size_t normal_modes_len,
    double *projected_frequencies_cm1, size_t projected_frequencies_len,
    double *projected_intensities_au, size_t projected_intensities_len,
    double *thermochemistry, size_t thermochemistry_len) {
  (void)config_capnp;
  (void)config_capnp_size_bytes;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)frequencies_cm1;
  (void)frequencies_len;
  (void)intensities_au;
  (void)intensities_len;
  (void)normal_modes;
  (void)normal_modes_len;
  (void)projected_frequencies_cm1;
  (void)projected_frequencies_len;
  (void)projected_intensities_au;
  (void)projected_intensities_len;
  (void)thermochemistry;
  (void)thermochemistry_len;
  return no_nwchem_fail();
}

size_t nwchemc_energy_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes) {
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  return 0;
}

size_t nwchemc_forces_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes) {
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  return 0;
}

size_t nwchemc_gradient_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes) {
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  return 0;
}

size_t nwchemc_potential_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes) {
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  return 0;
}

size_t nwchemc_stress_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes) {
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  return 0;
}

size_t nwchemc_polarizability_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes) {
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  return 0;
}

NWChemCResult nwchemc_session_calculate_polarizability_result(
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

NWChemCResult nwchemc_calculate_polarizability_result(
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

NWChemCResult nwchemc_session_calculate_stress_result(
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

NWChemCResult nwchemc_calculate_stress_result(
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

NWChemCResult nwchemc_session_calculate_polarizability(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *polarizability_au,
    size_t polarizability_len) {
  (void)session;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)polarizability_au;
  (void)polarizability_len;
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

NWChemCResult nwchemc_session_calculate_stress(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *stress_au,
    size_t stress_len) {
  (void)session;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)stress_au;
  (void)stress_len;
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

NWChemCResult nwchemc_session_calculate_frequencies_detail(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *frequencies_cm1,
    size_t frequencies_len, double *intensities_au, size_t intensities_len,
    double *normal_modes, size_t normal_modes_len,
    double *projected_frequencies_cm1, size_t projected_frequencies_len,
    double *projected_intensities_au, size_t projected_intensities_len,
    double *thermochemistry, size_t thermochemistry_len) {
  (void)session;
  (void)force_input_capnp;
  (void)force_input_capnp_size_bytes;
  (void)frequencies_cm1;
  (void)frequencies_len;
  (void)intensities_au;
  (void)intensities_len;
  (void)normal_modes;
  (void)normal_modes_len;
  (void)projected_frequencies_cm1;
  (void)projected_frequencies_len;
  (void)projected_intensities_au;
  (void)projected_intensities_len;
  (void)thermochemistry;
  (void)thermochemistry_len;
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
