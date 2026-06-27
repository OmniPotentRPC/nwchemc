#include "nwchemc.h"
#include "nwchemc_params.h"

#include <cmocka.h>
#include <errno.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *g_params_path = NULL;
static const char *g_config_options_path = NULL;
static const char *g_pspspin_path = NULL;
static const char *g_pspspin_many_path = NULL;
static const char *g_compact_cells_path = NULL;
static const char *g_force_step_a_path = NULL;
static const char *g_force_step_b_path = NULL;
static const char *g_force_step_ev_path = NULL;
static const char *g_force_step_changed_species_path = NULL;
static const char *g_force_step_state_path = NULL;
static const char *g_tce_methods_path = NULL;

static char g_basis[64];
static char g_theory[64];
static char g_scf_type[64];
static char g_dft_xc[64];
static char g_input_blocks[8192];
static char g_psp_elements[8][17];
static char g_psp_names[8][257];
static char g_set_keys[8][129];
static char g_set_values[8][257];
static char g_typed_set_keys[192][129];
static char g_typed_set_values[192][64][257];
static char g_brillouin_zone_name[64];
static double g_brillouin_kvectors[8];
static int g_psp_types[8];
static int g_typed_set_types[192];
static int g_typed_set_value_counts[192];
static int g_psp_count = 0;
static int g_set_string_count = 0;
static int g_typed_set_count = 0;
static int g_set_config_calls = 0;
static int g_set_dft_direct_calls = 0;
static int g_set_scf_direct_calls = 0;
static int g_set_driver_direct_calls = 0;
static int g_set_nwpw_direct_calls = 0;
static int g_set_brillouin_zone_calls = 0;
static int g_set_pseudopotential_calls = 0;
static int g_set_rtdb_strings_calls = 0;
static int g_set_rtdb_values_calls = 0;
static int g_dft_direct_enabled = 0;
static int g_dft_smearing_enabled = 0;
static double g_dft_smear_sigma_hartree = 0.0;
static int g_dft_smearing_spinset = 0;
static int g_scf_has_options = 0;
static int g_scf_maxiter = 0;
static double g_scf_thresh = 0.0;
static double g_scf_tol2e = 0.0;
static int g_driver_has_options = 0;
static int g_driver_maxiter = 0;
static int g_driver_tolerance_mode = 0;
static double g_driver_gmax_tol = 0.0;
static double g_driver_grms_tol = 0.0;
static double g_driver_xmax_tol = 0.0;
static double g_driver_xrms_tol = 0.0;
static int g_nwpw_has_options = 0;
static double g_nwpw_energy_cutoff = 0.0;
static double g_nwpw_wavefunction_cutoff = 0.0;
static double g_nwpw_ewald_rcut = 0.0;
static int g_nwpw_ewald_ncut = 0;
static int g_brillouin_has_options = 0;
static int g_brillouin_monkhorst_pack[3] = {0, 0, 0};
static int g_brillouin_max_kpoints_print = 0;
static int g_brillouin_kvector_count = 0;
static int g_energy_grad_calls = 0;
static int g_hessian_calls = 0;
static int g_hessian_cell_calls = 0;
static int g_dipole_calls = 0;
static int g_dipole_cell_calls = 0;
static int g_quadrupole_calls = 0;
static int g_quadrupole_cell_calls = 0;
static int g_optimize_calls = 0;
static int g_optimize_cell_calls = 0;
static int g_frequency_calls = 0;
static int g_frequency_cell_calls = 0;
static int g_call_n_atoms[8];
static int g_call_has_cell[8];
static int g_call_charge[8];
static int g_call_multiplicity[8];
static int g_call_atomic_numbers[8][8];
static double g_call_positions_ang[8][24];
static double g_call_cell_ang[8][9];
static int g_hessian_n_atoms[8];
static int g_hessian_has_cell[8];
static int g_hessian_charge[8];
static int g_hessian_multiplicity[8];
static int g_hessian_atomic_numbers[8][8];
static double g_hessian_positions_ang[8][24];
static double g_hessian_cell_ang[8][9];
static int g_dipole_n_atoms[8];
static int g_dipole_has_cell[8];
static int g_dipole_charge[8];
static int g_dipole_multiplicity[8];
static int g_dipole_atomic_numbers[8][8];
static double g_dipole_positions_ang[8][24];
static double g_dipole_cell_ang[8][9];
static int g_quadrupole_n_atoms[8];
static int g_quadrupole_has_cell[8];
static int g_quadrupole_charge[8];
static int g_quadrupole_multiplicity[8];
static int g_quadrupole_atomic_numbers[8][8];
static double g_quadrupole_positions_ang[8][24];
static double g_quadrupole_cell_ang[8][9];
static int g_optimize_n_atoms[8];
static int g_optimize_has_cell[8];
static int g_optimize_charge[8];
static int g_optimize_multiplicity[8];
static int g_optimize_atomic_numbers[8][8];
static double g_optimize_positions_ang[8][24];
static double g_optimize_cell_ang[8][9];
static int g_frequency_n_atoms[8];
static int g_frequency_has_cell[8];
static int g_frequency_charge[8];
static int g_frequency_multiplicity[8];
static int g_frequency_atomic_numbers[8][8];
static double g_frequency_positions_ang[8][24];
static double g_frequency_cell_ang[8][9];

#if defined(__GNUC__) || defined(__clang__)
#define NWCHEMC_TEST_WEAK __attribute__((weak))
#else
#define NWCHEMC_TEST_WEAK
#endif

extern NWChemCResult nwchemc_session_calculate_dipole(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *dipole_au,
    size_t dipole_len) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_session_calculate_quadrupole(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *quadrupole_au,
    size_t quadrupole_len) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_session_calculate_optimize(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *optimized_positions_ang,
    size_t optimized_positions_len) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_session_calculate_frequencies(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *frequencies_cm1,
    size_t frequencies_len, double *intensities_au, size_t intensities_len)
    NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_hessian(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *hessian_h_bohr2, size_t hessian_len) NWCHEMC_TEST_WEAK;
extern size_t nwchemc_hessian_result_size_for_force_input(
    const void *force_input_capnp,
    size_t force_input_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_session_calculate_hessian_result(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_hessian_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_dipole(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *dipole_au, size_t dipole_len) NWCHEMC_TEST_WEAK;
extern size_t nwchemc_dipole_result_size_for_force_input(
    const void *force_input_capnp,
    size_t force_input_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_session_calculate_dipole_result(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_dipole_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_quadrupole(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *quadrupole_au, size_t quadrupole_len) NWCHEMC_TEST_WEAK;
extern size_t nwchemc_quadrupole_result_size_for_force_input(
    const void *force_input_capnp,
    size_t force_input_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_session_calculate_quadrupole_result(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_quadrupole_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_optimize(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *optimized_positions_ang, size_t optimized_positions_len)
    NWCHEMC_TEST_WEAK;
extern size_t nwchemc_optimize_result_size_for_force_input(
    const void *force_input_capnp,
    size_t force_input_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_session_calculate_optimize_result(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_optimize_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_frequencies(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *frequencies_cm1, size_t frequencies_len, double *intensities_au,
    size_t intensities_len) NWCHEMC_TEST_WEAK;
extern size_t nwchemc_frequencies_result_size_for_force_input(
    const void *force_input_capnp,
    size_t force_input_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_session_calculate_frequencies_result(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_frequencies_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;

static void copy_span(char *dst, size_t dst_size, const char *src, int len) {
  size_t n = len > 0 ? (size_t)len : 0;
  if (n >= dst_size)
    n = dst_size - 1;
  if (n > 0)
    memcpy(dst, src, n);
  dst[n] = '\0';
}

static unsigned char *read_file(const char *path, size_t *size) {
  FILE *fp = fopen(path, "rb");
  if (!fp) {
    fprintf(stderr, "open failed for %s: %s\n", path, strerror(errno));
    return NULL;
  }
  if (fseek(fp, 0, SEEK_END) != 0) {
    fclose(fp);
    return NULL;
  }
  long n = ftell(fp);
  if (n <= 0) {
    fclose(fp);
    return NULL;
  }
  rewind(fp);
  unsigned char *buf = (unsigned char *)malloc((size_t)n);
  if (!buf) {
    fclose(fp);
    return NULL;
  }
  if (fread(buf, 1, (size_t)n, fp) != (size_t)n) {
    free(buf);
    fclose(fp);
    return NULL;
  }
  fclose(fp);
  *size = (size_t)n;
  return buf;
}

void nwchemc_embed_init(void) {}

int nwchemc_embed_available(void) { return 1; }

int nwchemc_embed_set_config(const char *basis, int basis_len,
                             const char *theory, int theory_len,
                             const char *scf_type, int scf_len,
                             const int *charge, const int *mult,
                             const char *input_blocks,
                             int input_blocks_len) {
  (void)charge;
  (void)mult;
  ++g_set_config_calls;
  copy_span(g_basis, sizeof(g_basis), basis, basis_len);
  copy_span(g_theory, sizeof(g_theory), theory, theory_len);
  copy_span(g_scf_type, sizeof(g_scf_type), scf_type, scf_len);
  copy_span(g_input_blocks, sizeof(g_input_blocks), input_blocks,
            input_blocks_len);
  return 0;
}

int nwchemc_embed_set_dft_direct(const char *xc, int xc_len,
                                 int direct_enabled, int smearing_enabled,
                                 double smear_sigma_hartree,
                                 int smearing_spinset) {
  ++g_set_dft_direct_calls;
  copy_span(g_dft_xc, sizeof(g_dft_xc), xc, xc_len);
  g_dft_direct_enabled = direct_enabled;
  g_dft_smearing_enabled = smearing_enabled;
  g_dft_smear_sigma_hartree = smear_sigma_hartree;
  g_dft_smearing_spinset = smearing_spinset;
  return 0;
}

int nwchemc_embed_set_scf_direct(int has_options, int maxiter, double thresh,
                                 double tol2e) {
  ++g_set_scf_direct_calls;
  g_scf_has_options = has_options;
  g_scf_maxiter = maxiter;
  g_scf_thresh = thresh;
  g_scf_tol2e = tol2e;
  return 0;
}

int nwchemc_embed_set_driver_direct(int has_options, int maxiter,
                                    int tolerance_mode, double gmax_tol,
                                    double grms_tol, double xmax_tol,
                                    double xrms_tol) {
  ++g_set_driver_direct_calls;
  g_driver_has_options = has_options;
  g_driver_maxiter = maxiter;
  g_driver_tolerance_mode = tolerance_mode;
  g_driver_gmax_tol = gmax_tol;
  g_driver_grms_tol = grms_tol;
  g_driver_xmax_tol = xmax_tol;
  g_driver_xrms_tol = xrms_tol;
  return 0;
}

int nwchemc_embed_set_nwpw_direct(int has_options, double energy_cutoff,
                                  double wavefunction_cutoff,
                                  double ewald_rcut, int ewald_ncut) {
  ++g_set_nwpw_direct_calls;
  g_nwpw_has_options = has_options;
  g_nwpw_energy_cutoff = energy_cutoff;
  g_nwpw_wavefunction_cutoff = wavefunction_cutoff;
  g_nwpw_ewald_rcut = ewald_rcut;
  g_nwpw_ewald_ncut = ewald_ncut;
  return 0;
}

int nwchemc_embed_set_brillouin_zone(int has_options, const char *zone_name,
                                     int zone_name_len, int monkhorst_pack_x,
                                     int monkhorst_pack_y,
                                     int monkhorst_pack_z,
                                     int max_kpoints_print,
                                     const double *kvectors,
                                     int kvector_count) {
  ++g_set_brillouin_zone_calls;
  g_brillouin_has_options = has_options;
  copy_span(g_brillouin_zone_name, sizeof(g_brillouin_zone_name), zone_name,
            zone_name_len);
  g_brillouin_monkhorst_pack[0] = monkhorst_pack_x;
  g_brillouin_monkhorst_pack[1] = monkhorst_pack_y;
  g_brillouin_monkhorst_pack[2] = monkhorst_pack_z;
  g_brillouin_max_kpoints_print = max_kpoints_print;
  g_brillouin_kvector_count = kvector_count;
  for (int i = 0; i < kvector_count * 4 && i < 8; ++i)
    g_brillouin_kvectors[i] = kvectors[i];
  return 0;
}

int nwchemc_embed_set_pseudopotentials(const char *elements,
                                       const int *library_types,
                                       const char *library_names, int count) {
  ++g_set_pseudopotential_calls;
  g_psp_count = count;
  if (count > 8)
    count = 8;
  for (int i = 0; i < count; ++i) {
    copy_span(g_psp_elements[i], sizeof(g_psp_elements[i]), elements + i * 16,
              16);
    copy_span(g_psp_names[i], sizeof(g_psp_names[i]),
              library_names + i * 256, 256);
    g_psp_types[i] = library_types[i];
  }
  return 0;
}

int nwchemc_embed_set_rtdb_strings(const char *keys, const char *values,
                                   int count) {
  ++g_set_rtdb_strings_calls;
  g_set_string_count = count;
  if (count > 8)
    count = 8;
  for (int i = 0; i < count; ++i) {
    copy_span(g_set_keys[i], sizeof(g_set_keys[i]), keys + i * 128, 128);
    copy_span(g_set_values[i], sizeof(g_set_values[i]), values + i * 256,
              256);
  }
  return 0;
}

int nwchemc_embed_set_rtdb_values(const char *keys, const int *value_types,
                                  const int *value_counts,
                                  const char *values, int count) {
  ++g_set_rtdb_values_calls;
  g_typed_set_count = count;
  if (count > 192)
    count = 192;
  for (int i = 0; i < count; ++i) {
    copy_span(g_typed_set_keys[i], sizeof(g_typed_set_keys[i]),
              keys + i * 128, 128);
    g_typed_set_types[i] = value_types[i];
    g_typed_set_value_counts[i] = value_counts[i];
    int nvalues = value_counts[i] < 64 ? value_counts[i] : 64;
    for (int j = 0; j < nvalues; ++j) {
      copy_span(g_typed_set_values[i][j], sizeof(g_typed_set_values[i][j]),
                values + (i * 64 + j) * 256, 256);
    }
  }
  return 0;
}

int nwchemc_embed_energy_only(const int *n_atoms, const double *positions_ang,
                              const int *atomic_numbers, const int *charge,
                              const int *multiplicity, double *energy_h,
                              char *errmsg, int errmsg_len) {
  (void)n_atoms;
  (void)positions_ang;
  (void)atomic_numbers;
  (void)charge;
  (void)multiplicity;
  *energy_h = -1.0;
  snprintf(errmsg, (size_t)errmsg_len, "ok");
  return 0;
}

static int capture_energy_grad_call(const int *n_atoms,
                                    const double *positions_ang,
                                    const int *atomic_numbers,
                                    const double *cell_ang,
                                    const int *has_cell, const int *charge,
                                    const int *multiplicity,
                                    double *energy_h, double *grad_h_bohr,
                                    char *errmsg, int errmsg_len) {
  int call = g_energy_grad_calls;
  if (call < 8) {
    int ncopy = *n_atoms < 8 ? *n_atoms : 8;
    int ncoord = (*n_atoms) * 3 < 24 ? (*n_atoms) * 3 : 24;
    g_call_n_atoms[call] = *n_atoms;
    g_call_has_cell[call] = has_cell ? *has_cell : 0;
    g_call_charge[call] = charge ? *charge : 0;
    g_call_multiplicity[call] = multiplicity ? *multiplicity : 0;
    for (int i = 0; i < ncopy; ++i)
      g_call_atomic_numbers[call][i] = atomic_numbers[i];
    for (int i = 0; i < ncoord; ++i)
      g_call_positions_ang[call][i] = positions_ang[i];
    for (int i = 0; i < 9; ++i)
      g_call_cell_ang[call][i] = cell_ang && g_call_has_cell[call]
                                     ? cell_ang[i]
                                     : 0.0;
  }
  ++g_energy_grad_calls;
  *energy_h = -1.0;
  for (int i = 0; i < (*n_atoms) * 3; ++i)
    grad_h_bohr[i] = (double)(i + 1);
  snprintf(errmsg, (size_t)errmsg_len, "ok");
  return 0;
}

int nwchemc_embed_energy_grad(const int *n_atoms, const double *positions_ang,
                              const int *atomic_numbers, const int *charge,
                              const int *multiplicity, double *energy_h,
                              double *grad_h_bohr, char *errmsg,
                              int errmsg_len) {
  return capture_energy_grad_call(n_atoms, positions_ang, atomic_numbers, NULL,
                                  NULL, charge, multiplicity, energy_h,
                                  grad_h_bohr, errmsg, errmsg_len);
}

int nwchemc_embed_energy_grad_cell(
    const int *n_atoms, const double *positions_ang, const int *atomic_numbers,
    const double *cell_ang, const int *has_cell, const int *charge,
    const int *multiplicity, double *energy_h, double *grad_h_bohr,
    char *errmsg, int errmsg_len) {
  return capture_energy_grad_call(n_atoms, positions_ang, atomic_numbers,
                                  cell_ang, has_cell, charge, multiplicity,
                                  energy_h, grad_h_bohr, errmsg, errmsg_len);
}

static int capture_hessian_call(const int *n_atoms, const double *positions_ang,
                                const int *atomic_numbers,
                                const double *cell_ang, const int *has_cell,
                                const int *charge, const int *multiplicity,
                                double *hessian_h_bohr2, char *errmsg,
                                int errmsg_len) {
  int call = g_hessian_calls;
  if (call < 8) {
    int ncopy = *n_atoms < 8 ? *n_atoms : 8;
    int ncoord = (*n_atoms) * 3 < 24 ? (*n_atoms) * 3 : 24;
    g_hessian_n_atoms[call] = *n_atoms;
    g_hessian_has_cell[call] = has_cell ? *has_cell : 0;
    g_hessian_charge[call] = charge ? *charge : 0;
    g_hessian_multiplicity[call] = multiplicity ? *multiplicity : 0;
    for (int i = 0; i < ncopy; ++i)
      g_hessian_atomic_numbers[call][i] = atomic_numbers[i];
    for (int i = 0; i < ncoord; ++i)
      g_hessian_positions_ang[call][i] = positions_ang[i];
    for (int i = 0; i < 9; ++i)
      g_hessian_cell_ang[call][i] = cell_ang && g_hessian_has_cell[call]
                                        ? cell_ang[i]
                                        : 0.0;
  }
  ++g_hessian_calls;
  int ndof = (*n_atoms) * 3;
  for (int i = 0; i < ndof * ndof; ++i)
    hessian_h_bohr2[i] = (double)(i + 10);
  snprintf(errmsg, (size_t)errmsg_len, "ok");
  return 0;
}

int nwchemc_embed_hessian(const int *n_atoms, const double *positions_ang,
                          const int *atomic_numbers, const int *charge,
                          const int *multiplicity, double *hessian_h_bohr2,
                          char *errmsg, int errmsg_len) {
  return capture_hessian_call(n_atoms, positions_ang, atomic_numbers, NULL,
                              NULL, charge, multiplicity, hessian_h_bohr2,
                              errmsg, errmsg_len);
}

int nwchemc_embed_hessian_cell(const int *n_atoms, const double *positions_ang,
                               const int *atomic_numbers,
                               const double *cell_ang, const int *has_cell,
                               const int *charge, const int *multiplicity,
                               double *hessian_h_bohr2, char *errmsg,
                               int errmsg_len) {
  ++g_hessian_cell_calls;
  return capture_hessian_call(n_atoms, positions_ang, atomic_numbers, cell_ang,
                              has_cell, charge, multiplicity, hessian_h_bohr2,
                              errmsg, errmsg_len);
}

static int capture_dipole_call(const int *n_atoms, const double *positions_ang,
                               const int *atomic_numbers,
                               const double *cell_ang, const int *has_cell,
                               const int *charge, const int *multiplicity,
                               double *energy_h, double *dipole_au,
                               char *errmsg, int errmsg_len) {
  int call = g_dipole_calls;
  if (call < 8) {
    int ncopy = *n_atoms < 8 ? *n_atoms : 8;
    int ncoord = (*n_atoms) * 3 < 24 ? (*n_atoms) * 3 : 24;
    g_dipole_n_atoms[call] = *n_atoms;
    g_dipole_has_cell[call] = has_cell ? *has_cell : 0;
    g_dipole_charge[call] = charge ? *charge : 0;
    g_dipole_multiplicity[call] = multiplicity ? *multiplicity : 0;
    for (int i = 0; i < ncopy; ++i)
      g_dipole_atomic_numbers[call][i] = atomic_numbers[i];
    for (int i = 0; i < ncoord; ++i)
      g_dipole_positions_ang[call][i] = positions_ang[i];
    for (int i = 0; i < 9; ++i)
      g_dipole_cell_ang[call][i] = cell_ang && g_dipole_has_cell[call]
                                       ? cell_ang[i]
                                       : 0.0;
  }
  ++g_dipole_calls;
  *energy_h = -1.25;
  dipole_au[0] = 0.25;
  dipole_au[1] = 0.5;
  dipole_au[2] = 0.75;
  snprintf(errmsg, (size_t)errmsg_len, "ok");
  return 0;
}

int nwchemc_embed_dipole(const int *n_atoms, const double *positions_ang,
                         const int *atomic_numbers, const int *charge,
                         const int *multiplicity, double *energy_h,
                         double *dipole_au, char *errmsg, int errmsg_len) {
  return capture_dipole_call(n_atoms, positions_ang, atomic_numbers, NULL,
                             NULL, charge, multiplicity, energy_h, dipole_au,
                             errmsg, errmsg_len);
}

int nwchemc_embed_dipole_cell(
    const int *n_atoms, const double *positions_ang, const int *atomic_numbers,
    const double *cell_ang, const int *has_cell, const int *charge,
    const int *multiplicity, double *energy_h, double *dipole_au, char *errmsg,
    int errmsg_len) {
  ++g_dipole_cell_calls;
  return capture_dipole_call(n_atoms, positions_ang, atomic_numbers, cell_ang,
                             has_cell, charge, multiplicity, energy_h,
                             dipole_au, errmsg, errmsg_len);
}

static int capture_quadrupole_call(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const double *cell_ang, const int *has_cell,
    const int *charge, const int *multiplicity, double *energy_h,
    double *quadrupole_au, char *errmsg, int errmsg_len) {
  int call = g_quadrupole_calls;
  if (call < 8) {
    int ncopy = *n_atoms < 8 ? *n_atoms : 8;
    int ncoord = (*n_atoms) * 3 < 24 ? (*n_atoms) * 3 : 24;
    g_quadrupole_n_atoms[call] = *n_atoms;
    g_quadrupole_has_cell[call] = has_cell ? *has_cell : 0;
    g_quadrupole_charge[call] = charge ? *charge : 0;
    g_quadrupole_multiplicity[call] = multiplicity ? *multiplicity : 0;
    for (int i = 0; i < ncopy; ++i)
      g_quadrupole_atomic_numbers[call][i] = atomic_numbers[i];
    for (int i = 0; i < ncoord; ++i)
      g_quadrupole_positions_ang[call][i] = positions_ang[i];
    for (int i = 0; i < 9; ++i)
      g_quadrupole_cell_ang[call][i] =
          cell_ang && g_quadrupole_has_cell[call] ? cell_ang[i] : 0.0;
  }
  ++g_quadrupole_calls;
  *energy_h = -1.5;
  for (int i = 0; i < 6; ++i)
    quadrupole_au[i] = 0.125 * (double)(i + 1);
  snprintf(errmsg, (size_t)errmsg_len, "ok");
  return 0;
}

int nwchemc_embed_quadrupole(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const int *charge, const int *multiplicity,
    double *energy_h, double *quadrupole_au, char *errmsg, int errmsg_len) {
  return capture_quadrupole_call(n_atoms, positions_ang, atomic_numbers, NULL,
                                 NULL, charge, multiplicity, energy_h,
                                 quadrupole_au, errmsg, errmsg_len);
}

int nwchemc_embed_quadrupole_cell(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const double *cell_ang, const int *has_cell,
    const int *charge, const int *multiplicity, double *energy_h,
    double *quadrupole_au, char *errmsg, int errmsg_len) {
  ++g_quadrupole_cell_calls;
  return capture_quadrupole_call(n_atoms, positions_ang, atomic_numbers,
                                 cell_ang, has_cell, charge, multiplicity,
                                 energy_h, quadrupole_au, errmsg, errmsg_len);
}

static int capture_optimize_call(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const double *cell_ang, const int *has_cell,
    const int *charge, const int *multiplicity, double *energy_h,
    double *optimized_positions_ang, char *errmsg, int errmsg_len) {
  int call = g_optimize_calls;
  if (call < 8) {
    int ncopy = *n_atoms < 8 ? *n_atoms : 8;
    int ncoord = (*n_atoms) * 3 < 24 ? (*n_atoms) * 3 : 24;
    g_optimize_n_atoms[call] = *n_atoms;
    g_optimize_has_cell[call] = has_cell ? *has_cell : 0;
    g_optimize_charge[call] = charge ? *charge : 0;
    g_optimize_multiplicity[call] = multiplicity ? *multiplicity : 0;
    for (int i = 0; i < ncopy; ++i)
      g_optimize_atomic_numbers[call][i] = atomic_numbers[i];
    for (int i = 0; i < ncoord; ++i)
      g_optimize_positions_ang[call][i] = positions_ang[i];
    for (int i = 0; i < 9; ++i)
      g_optimize_cell_ang[call][i] =
          cell_ang && g_optimize_has_cell[call] ? cell_ang[i] : 0.0;
  }
  ++g_optimize_calls;
  *energy_h = -1.75;
  for (int i = 0; i < (*n_atoms) * 3; ++i)
    optimized_positions_ang[i] = positions_ang[i] + 0.01 * (double)(i + 1);
  snprintf(errmsg, (size_t)errmsg_len, "ok");
  return 0;
}

int nwchemc_embed_optimize(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const int *charge, const int *multiplicity,
    double *energy_h, double *optimized_positions_ang, char *errmsg,
    int errmsg_len) {
  return capture_optimize_call(n_atoms, positions_ang, atomic_numbers, NULL,
                               NULL, charge, multiplicity, energy_h,
                               optimized_positions_ang, errmsg, errmsg_len);
}

int nwchemc_embed_optimize_cell(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const double *cell_ang, const int *has_cell,
    const int *charge, const int *multiplicity, double *energy_h,
    double *optimized_positions_ang, char *errmsg, int errmsg_len) {
  ++g_optimize_cell_calls;
  return capture_optimize_call(n_atoms, positions_ang, atomic_numbers,
                               cell_ang, has_cell, charge, multiplicity,
                               energy_h, optimized_positions_ang, errmsg,
                               errmsg_len);
}

static int capture_frequency_call(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const double *cell_ang, const int *has_cell,
    const int *charge, const int *multiplicity, double *frequencies_cm1,
    double *intensities_au, char *errmsg, int errmsg_len) {
  int call = g_frequency_calls;
  if (call < 8) {
    int ncopy = *n_atoms < 8 ? *n_atoms : 8;
    int ncoord = (*n_atoms) * 3 < 24 ? (*n_atoms) * 3 : 24;
    g_frequency_n_atoms[call] = *n_atoms;
    g_frequency_has_cell[call] = has_cell ? *has_cell : 0;
    g_frequency_charge[call] = charge ? *charge : 0;
    g_frequency_multiplicity[call] = multiplicity ? *multiplicity : 0;
    for (int i = 0; i < ncopy; ++i)
      g_frequency_atomic_numbers[call][i] = atomic_numbers[i];
    for (int i = 0; i < ncoord; ++i)
      g_frequency_positions_ang[call][i] = positions_ang[i];
    for (int i = 0; i < 9; ++i)
      g_frequency_cell_ang[call][i] =
          cell_ang && g_frequency_has_cell[call] ? cell_ang[i] : 0.0;
  }
  ++g_frequency_calls;
  int ndof = (*n_atoms) * 3;
  for (int i = 0; i < ndof; ++i) {
    frequencies_cm1[i] = 100.0 + (double)i;
    if (intensities_au)
      intensities_au[i] = 0.01 * (double)(i + 1);
  }
  snprintf(errmsg, (size_t)errmsg_len, "ok");
  return 0;
}

int nwchemc_embed_frequencies(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const int *charge, const int *multiplicity,
    double *frequencies_cm1, double *intensities_au, char *errmsg,
    int errmsg_len) {
  return capture_frequency_call(n_atoms, positions_ang, atomic_numbers, NULL,
                                NULL, charge, multiplicity, frequencies_cm1,
                                intensities_au, errmsg, errmsg_len);
}

int nwchemc_embed_frequencies_cell(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const double *cell_ang, const int *has_cell,
    const int *charge, const int *multiplicity, double *frequencies_cm1,
    double *intensities_au, char *errmsg, int errmsg_len) {
  ++g_frequency_cell_calls;
  return capture_frequency_call(n_atoms, positions_ang, atomic_numbers,
                                cell_ang, has_cell, charge, multiplicity,
                                frequencies_cm1, intensities_au, errmsg,
                                errmsg_len);
}

void nwchemc_embed_finalize(void) {}

static void reset_embed_captures(void) {
  g_basis[0] = '\0';
  g_theory[0] = '\0';
  g_scf_type[0] = '\0';
  g_dft_xc[0] = '\0';
  g_input_blocks[0] = '\0';
  for (int i = 0; i < 8; ++i) {
    g_psp_elements[i][0] = '\0';
    g_psp_names[i][0] = '\0';
    g_set_keys[i][0] = '\0';
    g_set_values[i][0] = '\0';
    g_psp_types[i] = -1;
  }
  for (int i = 0; i < 192; ++i) {
    g_typed_set_keys[i][0] = '\0';
    for (int j = 0; j < 64; ++j)
      g_typed_set_values[i][j][0] = '\0';
    g_typed_set_types[i] = -1;
    g_typed_set_value_counts[i] = 0;
  }
  g_psp_count = 0;
  g_set_string_count = 0;
  g_typed_set_count = 0;
  g_set_config_calls = 0;
  g_set_dft_direct_calls = 0;
  g_set_scf_direct_calls = 0;
  g_set_driver_direct_calls = 0;
  g_set_nwpw_direct_calls = 0;
  g_set_brillouin_zone_calls = 0;
  g_set_pseudopotential_calls = 0;
  g_set_rtdb_strings_calls = 0;
  g_set_rtdb_values_calls = 0;
  g_dft_direct_enabled = 0;
  g_dft_smearing_enabled = 0;
  g_dft_smear_sigma_hartree = 0.0;
  g_dft_smearing_spinset = 0;
  g_scf_has_options = 0;
  g_scf_maxiter = 0;
  g_scf_thresh = 0.0;
  g_scf_tol2e = 0.0;
  g_driver_has_options = 0;
  g_driver_maxiter = 0;
  g_driver_tolerance_mode = 0;
  g_driver_gmax_tol = 0.0;
  g_driver_grms_tol = 0.0;
  g_driver_xmax_tol = 0.0;
  g_driver_xrms_tol = 0.0;
  g_nwpw_has_options = 0;
  g_nwpw_energy_cutoff = 0.0;
  g_nwpw_wavefunction_cutoff = 0.0;
  g_nwpw_ewald_rcut = 0.0;
  g_nwpw_ewald_ncut = 0;
  g_brillouin_has_options = 0;
  g_brillouin_zone_name[0] = '\0';
  g_brillouin_monkhorst_pack[0] = 0;
  g_brillouin_monkhorst_pack[1] = 0;
  g_brillouin_monkhorst_pack[2] = 0;
  g_brillouin_max_kpoints_print = 0;
  g_brillouin_kvector_count = 0;
  for (int i = 0; i < 8; ++i)
    g_brillouin_kvectors[i] = 0.0;
  g_energy_grad_calls = 0;
  g_hessian_calls = 0;
  g_hessian_cell_calls = 0;
  g_dipole_calls = 0;
  g_dipole_cell_calls = 0;
  g_quadrupole_calls = 0;
  g_quadrupole_cell_calls = 0;
  g_optimize_calls = 0;
  g_optimize_cell_calls = 0;
  g_frequency_calls = 0;
  g_frequency_cell_calls = 0;
  memset(g_call_n_atoms, 0, sizeof(g_call_n_atoms));
  memset(g_call_has_cell, 0, sizeof(g_call_has_cell));
  memset(g_call_charge, 0, sizeof(g_call_charge));
  memset(g_call_multiplicity, 0, sizeof(g_call_multiplicity));
  memset(g_call_atomic_numbers, 0, sizeof(g_call_atomic_numbers));
  memset(g_call_positions_ang, 0, sizeof(g_call_positions_ang));
  memset(g_call_cell_ang, 0, sizeof(g_call_cell_ang));
  memset(g_hessian_n_atoms, 0, sizeof(g_hessian_n_atoms));
  memset(g_hessian_has_cell, 0, sizeof(g_hessian_has_cell));
  memset(g_hessian_charge, 0, sizeof(g_hessian_charge));
  memset(g_hessian_multiplicity, 0, sizeof(g_hessian_multiplicity));
  memset(g_hessian_atomic_numbers, 0, sizeof(g_hessian_atomic_numbers));
  memset(g_hessian_positions_ang, 0, sizeof(g_hessian_positions_ang));
  memset(g_hessian_cell_ang, 0, sizeof(g_hessian_cell_ang));
  memset(g_dipole_n_atoms, 0, sizeof(g_dipole_n_atoms));
  memset(g_dipole_has_cell, 0, sizeof(g_dipole_has_cell));
  memset(g_dipole_charge, 0, sizeof(g_dipole_charge));
  memset(g_dipole_multiplicity, 0, sizeof(g_dipole_multiplicity));
  memset(g_dipole_atomic_numbers, 0, sizeof(g_dipole_atomic_numbers));
  memset(g_dipole_positions_ang, 0, sizeof(g_dipole_positions_ang));
  memset(g_dipole_cell_ang, 0, sizeof(g_dipole_cell_ang));
  memset(g_quadrupole_n_atoms, 0, sizeof(g_quadrupole_n_atoms));
  memset(g_quadrupole_has_cell, 0, sizeof(g_quadrupole_has_cell));
  memset(g_quadrupole_charge, 0, sizeof(g_quadrupole_charge));
  memset(g_quadrupole_multiplicity, 0, sizeof(g_quadrupole_multiplicity));
  memset(g_quadrupole_atomic_numbers, 0,
         sizeof(g_quadrupole_atomic_numbers));
  memset(g_quadrupole_positions_ang, 0,
         sizeof(g_quadrupole_positions_ang));
  memset(g_quadrupole_cell_ang, 0, sizeof(g_quadrupole_cell_ang));
  memset(g_optimize_n_atoms, 0, sizeof(g_optimize_n_atoms));
  memset(g_optimize_has_cell, 0, sizeof(g_optimize_has_cell));
  memset(g_optimize_charge, 0, sizeof(g_optimize_charge));
  memset(g_optimize_multiplicity, 0, sizeof(g_optimize_multiplicity));
  memset(g_optimize_atomic_numbers, 0, sizeof(g_optimize_atomic_numbers));
  memset(g_optimize_positions_ang, 0, sizeof(g_optimize_positions_ang));
  memset(g_optimize_cell_ang, 0, sizeof(g_optimize_cell_ang));
  memset(g_frequency_n_atoms, 0, sizeof(g_frequency_n_atoms));
  memset(g_frequency_has_cell, 0, sizeof(g_frequency_has_cell));
  memset(g_frequency_charge, 0, sizeof(g_frequency_charge));
  memset(g_frequency_multiplicity, 0, sizeof(g_frequency_multiplicity));
  memset(g_frequency_atomic_numbers, 0, sizeof(g_frequency_atomic_numbers));
  memset(g_frequency_positions_ang, 0, sizeof(g_frequency_positions_ang));
  memset(g_frequency_cell_ang, 0, sizeof(g_frequency_cell_ang));
}

static void assert_close(double actual, double expected, double tolerance) {
  assert_true(actual > expected - tolerance);
  assert_true(actual < expected + tolerance);
}

static void assert_potential_result(const unsigned char *message,
                                    size_t message_size,
                                    double expected_energy,
                                    const double *expected_forces,
                                    size_t expected_force_count,
                                    double tolerance) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);
  struct PotentialResult result;
  read_PotentialResult(&result, root);
  assert_close(result.energy, expected_energy, tolerance);
  capn_resolve(&result.forces.p);
  assert_int_equal(result.forces.p.type, CAPN_LIST);
  assert_int_equal(result.forces.p.datasz, 8);
  assert_int_equal(result.forces.p.len, (int)expected_force_count);
  for (size_t i = 0; i < expected_force_count; ++i) {
    double actual = capn_to_f64(capn_get64(result.forces, (int)i));
    assert_close(actual, expected_forces[i], tolerance);
  }
  capn_free(&arena);
}

static void assert_potential_result_hessian(const unsigned char *message,
                                            size_t message_size,
                                            const double *expected_hessian,
                                            size_t expected_hessian_count,
                                            double tolerance) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);
  struct PotentialResult result;
  read_PotentialResult(&result, root);
  capn_resolve(&result.hessian.p);
  assert_int_equal(result.hessian.p.type, CAPN_LIST);
  assert_int_equal(result.hessian.p.datasz, 8);
  assert_int_equal(result.hessian.p.len, (int)expected_hessian_count);
  for (size_t i = 0; i < expected_hessian_count; ++i) {
    double actual = capn_to_f64(capn_get64(result.hessian, (int)i));
    assert_close(actual, expected_hessian[i], tolerance);
  }
  capn_free(&arena);
}

static void assert_potential_result_dipole(const unsigned char *message,
                                           size_t message_size,
                                           const double *expected_dipole,
                                           double tolerance) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);
  struct PotentialResult result;
  read_PotentialResult(&result, root);
  capn_resolve(&result.dipole.p);
  assert_int_equal(result.dipole.p.type, CAPN_LIST);
  assert_int_equal(result.dipole.p.datasz, 8);
  assert_int_equal(result.dipole.p.len, 3);
  for (int i = 0; i < 3; ++i) {
    double actual = capn_to_f64(capn_get64(result.dipole, i));
    assert_close(actual, expected_dipole[i], tolerance);
  }
  capn_free(&arena);
}

static void assert_potential_result_quadrupole(const unsigned char *message,
                                               size_t message_size,
                                               const double *expected,
                                               double tolerance) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);
  struct PotentialResult result;
  read_PotentialResult(&result, root);
  capn_resolve(&result.quadrupole.p);
  assert_int_equal(result.quadrupole.p.type, CAPN_LIST);
  assert_int_equal(result.quadrupole.p.datasz, 8);
  assert_int_equal(result.quadrupole.p.len, 6);
  for (int i = 0; i < 6; ++i) {
    double actual = capn_to_f64(capn_get64(result.quadrupole, i));
    assert_close(actual, expected[i], tolerance);
  }
  capn_free(&arena);
}

static void assert_potential_result_optimized(
    const unsigned char *message, size_t message_size, double expected_energy,
    const double *expected_positions, size_t expected_position_count,
    double tolerance) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);
  struct PotentialResult result;
  read_PotentialResult(&result, root);
  assert_close(result.energy, expected_energy, tolerance);
  capn_resolve(&result.optimizedPos.p);
  assert_int_equal(result.optimizedPos.p.type, CAPN_LIST);
  assert_int_equal(result.optimizedPos.p.datasz, 8);
  assert_int_equal(result.optimizedPos.p.len, (int)expected_position_count);
  for (size_t i = 0; i < expected_position_count; ++i) {
    double actual = capn_to_f64(capn_get64(result.optimizedPos, (int)i));
    assert_close(actual, expected_positions[i], tolerance);
  }
  capn_free(&arena);
}

static void assert_potential_result_frequencies(
    const unsigned char *message, size_t message_size,
    const double *expected_frequencies, const double *expected_intensities,
    size_t expected_count, double tolerance) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);
  struct PotentialResult result;
  read_PotentialResult(&result, root);
  capn_resolve(&result.frequencies.p);
  assert_int_equal(result.frequencies.p.type, CAPN_LIST);
  assert_int_equal(result.frequencies.p.datasz, 8);
  assert_int_equal(result.frequencies.p.len, (int)expected_count);
  capn_resolve(&result.intensities.p);
  assert_int_equal(result.intensities.p.type, CAPN_LIST);
  assert_int_equal(result.intensities.p.datasz, 8);
  assert_int_equal(result.intensities.p.len, (int)expected_count);
  for (size_t i = 0; i < expected_count; ++i) {
    double actual_frequency =
        capn_to_f64(capn_get64(result.frequencies, (int)i));
    double actual_intensity =
        capn_to_f64(capn_get64(result.intensities, (int)i));
    assert_close(actual_frequency, expected_frequencies[i], tolerance);
    assert_close(actual_intensity, expected_intensities[i], tolerance);
  }
  capn_free(&arena);
}

static int find_typed_set_key(const char *key) {
  for (int i = 0; i < g_typed_set_count && i < 192; ++i) {
    if (strcmp(g_typed_set_keys[i], key) == 0)
      return i;
  }
  return -1;
}

static void assert_typed_set_scalar(const char *key, int value_type,
                                    const char *value) {
  int index = find_typed_set_key(key);
  assert_true(index >= 0);
  assert_int_equal(g_typed_set_types[index], value_type);
  assert_int_equal(g_typed_set_value_counts[index], 1);
  assert_string_equal(g_typed_set_values[index][0], value);
}

static void assert_typed_set_scalar_entry(const char *key, int value_type,
                                          const char *value) {
  for (int i = 0; i < g_typed_set_count && i < 192; ++i) {
    if (strcmp(g_typed_set_keys[i], key) != 0)
      continue;
    if (g_typed_set_types[i] == value_type &&
        g_typed_set_value_counts[i] == 1 &&
        strcmp(g_typed_set_values[i][0], value) == 0)
      return;
  }
  fail_msg("missing typed RTDB scalar %s=%s", key, value);
}

static void assert_typed_set_pair(const char *key, int value_type,
                                  const char *first, const char *second) {
  int index = find_typed_set_key(key);
  assert_true(index >= 0);
  assert_int_equal(g_typed_set_types[index], value_type);
  assert_int_equal(g_typed_set_value_counts[index], 2);
  assert_string_equal(g_typed_set_values[index][0], first);
  assert_string_equal(g_typed_set_values[index][1], second);
}

static void assert_typed_set_triple(const char *key, int value_type,
                                    const char *first, const char *second,
                                    const char *third) {
  int index = find_typed_set_key(key);
  assert_true(index >= 0);
  assert_int_equal(g_typed_set_types[index], value_type);
  assert_int_equal(g_typed_set_value_counts[index], 3);
  assert_string_equal(g_typed_set_values[index][0], first);
  assert_string_equal(g_typed_set_values[index][1], second);
  assert_string_equal(g_typed_set_values[index][2], third);
}

static void assert_typed_set_values(const char *key, int value_type,
                                    int nvalues,
                                    const char *const *expected) {
  int index = find_typed_set_key(key);
  assert_true(index >= 0);
  assert_int_equal(g_typed_set_types[index], value_type);
  assert_int_equal(g_typed_set_value_counts[index], nvalues);
  for (int i = 0; i < nvalues; ++i)
    assert_string_equal(g_typed_set_values[index][i], expected[i]);
}

static void test_embed_config_uses_direct_dft_values(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_int_equal(g_set_config_calls, 1);
  assert_string_equal(g_basis, "sto-3g");
  assert_string_equal(g_theory, "dft");
  assert_string_equal(g_scf_type, "pbe0");
  assert_int_equal(g_set_dft_direct_calls, 1);
  assert_string_equal(g_dft_xc, "pbe0");
  assert_int_equal(g_dft_direct_enabled, 1);
  assert_int_equal(g_dft_smearing_enabled, 1);
  assert_true(g_dft_smear_sigma_hartree > 0.000999);
  assert_true(g_dft_smear_sigma_hartree < 0.001001);
  assert_int_equal(g_dft_smearing_spinset, 0);
  assert_null(strstr(g_input_blocks, "smear 0.001"));
  assert_null(strstr(g_input_blocks, "xc pbe0"));
  assert_null(strstr(g_input_blocks, "  direct"));
  assert_null(strstr(g_input_blocks, "pseudopotentials"));
  assert_null(strstr(g_input_blocks, "Si library sg15"));
  assert_null(strstr(g_input_blocks, "H pspw_library hgh_lda"));
  assert_null(strstr(g_input_blocks, "O paw_library paw_default"));
  assert_null(strstr(g_input_blocks, "C cpi C.cpi"));
  assert_null(strstr(g_input_blocks, "N teter N.teter"));
  assert_null(strstr(g_input_blocks, "* pspw_library pspw_default"));
  assert_null(strstr(g_input_blocks, "energy_cutoff 12.5"));
  assert_null(strstr(g_input_blocks, "wavefunction_cutoff 6.25"));
  assert_null(strstr(g_input_blocks, "ewald_rcut 3.5"));
  assert_null(strstr(g_input_blocks, "ewald_ncut 9"));
  assert_null(strstr(g_input_blocks, "cell_name cellA"));
  assert_null(strstr(g_input_blocks, "input_wavefunction_filename psi.in"));
  assert_null(strstr(g_input_blocks, "output_wavefunction_filename psi.out"));
  assert_null(strstr(g_input_blocks, "fake_mass 2.5"));
  assert_null(strstr(g_input_blocks, "time_step 4.5"));
  assert_null(strstr(g_input_blocks, "loop 3 7"));
  assert_null(strstr(g_input_blocks, "tolerances 0.125 0.25 0.5"));
  assert_null(strstr(g_input_blocks, "exchange_correlation pbe96"));
  assert_null(strstr(g_input_blocks, "nobalance"));
  assert_null(strstr(g_input_blocks, "bo_steps 11 22"));
  assert_null(strstr(g_input_blocks, "bo_time_step 0.75"));
  assert_null(strstr(g_input_blocks, "bo_algorithm velocity-verlet"));
  assert_null(strstr(g_input_blocks, "bo_fake_mass 333"));
  assert_null(strstr(g_input_blocks, "scaling 1.5 2.5"));
  assert_null(strstr(g_input_blocks, "np_dimensions 2 3 4"));
  assert_null(strstr(g_input_blocks, "spin_orbit off"));
  assert_null(strstr(g_input_blocks, "parallel_io on"));
  assert_null(strstr(g_input_blocks, "xyz_filename traj.xyz"));
  assert_null(strstr(g_input_blocks, "ion_motion_filename ion.mov"));
  assert_null(strstr(g_input_blocks, "emotion_filename electron.mov"));
  assert_null(strstr(g_input_blocks, "hmotion_filename h.mov"));
  assert_null(strstr(g_input_blocks, "omotion_filename orb.mov"));
  assert_null(strstr(g_input_blocks, "eigmotion_filename eig.mov"));
  assert_null(strstr(g_input_blocks, "fractional_orbitals 5 6"));
  assert_null(strstr(g_input_blocks,
                     "smear temperature 0.02 alpha 0.7 fermi orbitals 5 6"));
  assert_null(strstr(g_input_blocks, "virtual_orbitals 7 8"));
  assert_null(strstr(g_input_blocks, "lcao_skip"));
  assert_null(strstr(g_input_blocks, "ewald_ngrid 9 10 11"));
  assert_null(strstr(g_input_blocks, "Nose-Hoover 12 300 34 400 start 2 3"));
  assert_null(strstr(g_input_blocks, "monkhorst-pack 3 4 -5 zoneA"));
  assert_null(strstr(g_input_blocks, "zone_name zoneA"));
  assert_null(strstr(g_input_blocks, "max_kpoints_print 12"));
  assert_null(strstr(g_input_blocks, "simulation_cell"));
  assert_null(strstr(g_input_blocks, "boundary_conditions periodic"));
  assert_null(strstr(g_input_blocks, "lattice_vectors"));
  assert_null(strstr(g_input_blocks, "ngrid 20 22 24"));
  assert_null(strstr(g_input_blocks, "ngrid_small 10 11 12"));
  assert_null(strstr(g_input_blocks, "box_delta 1"));
  assert_null(strstr(g_input_blocks, "box_orient"));
  assert_null(strstr(g_input_blocks, "box_different_lengths"));
  assert_null(strstr(g_input_blocks, "ccsd\n  maxiter 20"));
  assert_null(strstr(g_input_blocks, "freeze 1 virtual 2"));
  assert_null(strstr(g_input_blocks, "nodisk"));
  assert_non_null(strstr(
      g_input_blocks,
      "ccsd\n"
      "  print high reference\n"
      "  noprint byproduct energies\n"
      "  doa 1 0 1\n"
      "  dob 2 0\n"
      "  dog 3\n"
      "  doh 4 5\n"
      "  dojk 6\n"
      "  dos 7 8 9\n"
      "  dod 10\n"
      "end"));
  assert_null(strstr(g_input_blocks, "tce\n  dft"));
  assert_null(strstr(g_input_blocks, "cr-ccsd(t)"));
  assert_null(strstr(g_input_blocks, "lshift 0.01"));
  assert_null(strstr(g_input_blocks, "lshift2 0.03 0.04"));
  assert_null(strstr(g_input_blocks, "io ga_eaf"));
  assert_null(strstr(g_input_blocks, "densmat dens.dat"));
  assert_null(strstr(g_input_blocks, "nofock"));
  assert_null(strstr(g_input_blocks, "active_oa 5"));
  assert_null(strstr(g_input_blocks, "tcc_spaces"));
  assert_non_null(strstr(g_input_blocks, "tce\n  print debug tile time\nend"));
  assert_non_null(strstr(g_input_blocks, "mrccdata\n  se4t"));
  assert_non_null(strstr(g_input_blocks, "  cas 2 2\n"));
  assert_non_null(strstr(g_input_blocks, "  2222ba\n"));
  assert_non_null(strstr(g_input_blocks, "tce\n  freeze core atomic\nend"));
  assert_null(strstr(g_input_blocks, "dipole"));
  assert_null(strstr(g_input_blocks, "pspspin off"));
  assert_non_null(strstr(g_input_blocks, "print debug tile time"));
  assert_non_null(strstr(g_input_blocks, "iterations 40"));
  assert_non_null(strstr(g_input_blocks, "set int:acc_std 1e-8"));
  assert_int_equal(g_set_rtdb_values_calls, 1);
  assert_int_equal(g_typed_set_count, 164);
  assert_typed_set_scalar("cgsd:ecut", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "12.5");
  assert_typed_set_scalar("band:wcut", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "6.25");
  assert_typed_set_scalar("cpsd:rcut", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "3.5");
  assert_typed_set_scalar("cpmd:ncut", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "9");
  assert_typed_set_scalar("cgsd:cell_name", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                          "cellA");
  assert_typed_set_scalar("pspw:input vectors", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                          "psi.in");
  assert_typed_set_scalar("band:input vectors", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                          "psi.in");
  assert_typed_set_scalar("band:output_wavefunction_filename",
                          NWCHEMC_DIRECT_SET_VALUE_TEXT, "psi.out");
  assert_typed_set_scalar("cpsd:fake_mass", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "2.5");
  assert_typed_set_scalar("band:time_step", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "4.5");
  assert_typed_set_pair("cgsd:loop", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "3",
                        "7");
  assert_typed_set_triple("band:tolerances", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "0.125", "0.25", "0.5");
  assert_typed_set_scalar("cgsd:exchange_correlation",
                          NWCHEMC_DIRECT_SET_VALUE_TEXT, "pbe96");
  assert_typed_set_scalar("cpmd:exchange_correlation",
                          NWCHEMC_DIRECT_SET_VALUE_TEXT, "pbe96");
  assert_typed_set_scalar("pspw:SIC_all", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "false");
  assert_typed_set_scalar("pspw:HFX", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "false");
  assert_typed_set_scalar("band:HFX", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "false");
  assert_typed_set_scalar("pspw:SIC_xc_parameter",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "1");
  assert_typed_set_scalar("band:HFX_parameter",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "1");
  assert_typed_set_scalar("nwpw:balance", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "false");
  assert_typed_set_pair("nwpw:bo_steps", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                        "11", "22");
  assert_typed_set_scalar("nwpw:bo_time_step",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "0.75");
  assert_typed_set_scalar("nwpw:bo_algorithm",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "1");
  assert_typed_set_scalar("nwpw:bo_fake_mass",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "333");
  assert_typed_set_pair("nwpw:scaling", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "1.5",
                        "2.5");
  assert_typed_set_pair("cpmd:scaling", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "1.5",
                        "2.5");
  assert_typed_set_triple("nwpw:np_dimensions",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "2", "3", "4");
  assert_typed_set_scalar("nwpw:spin_orbit", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "false");
  assert_typed_set_scalar("pspw:spin_orbit", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "false");
  assert_typed_set_scalar("band:spin_orbit", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "false");
  assert_typed_set_scalar("nwpw:parallel_io", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_typed_set_scalar("nwpw:pspspin", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "false");
  assert_typed_set_scalar("nwpw:pspspin_count",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "0");
  assert_typed_set_scalar("cpmd:xyz_filename", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                          "traj.xyz");
  assert_typed_set_scalar("nwpw:xyz_filename", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                          "traj.xyz");
  assert_typed_set_scalar("cpmd:ion_motion_filename",
                          NWCHEMC_DIRECT_SET_VALUE_TEXT, "ion.mov");
  assert_typed_set_scalar("nwpw:ion_motion_filename",
                          NWCHEMC_DIRECT_SET_VALUE_TEXT, "ion.mov");
  assert_typed_set_scalar("cpmd:emotion_filename",
                          NWCHEMC_DIRECT_SET_VALUE_TEXT, "electron.mov");
  assert_typed_set_scalar("nwpw:emotion_filename",
                          NWCHEMC_DIRECT_SET_VALUE_TEXT, "electron.mov");
  assert_typed_set_scalar("cpmd:hmotion_filename",
                          NWCHEMC_DIRECT_SET_VALUE_TEXT, "h.mov");
  assert_typed_set_scalar("nwpw:hmotion_filename",
                          NWCHEMC_DIRECT_SET_VALUE_TEXT, "h.mov");
  assert_typed_set_scalar("cpmd:omotion_filename",
                          NWCHEMC_DIRECT_SET_VALUE_TEXT, "orb.mov");
  assert_typed_set_scalar("nwpw:omotion_filename",
                          NWCHEMC_DIRECT_SET_VALUE_TEXT, "orb.mov");
  assert_typed_set_scalar("cpmd:eigmotion_filename",
                          NWCHEMC_DIRECT_SET_VALUE_TEXT, "eig.mov");
  assert_typed_set_scalar("nwpw:eigmotion_filename",
                          NWCHEMC_DIRECT_SET_VALUE_TEXT, "eig.mov");
  assert_typed_set_pair("nwpw:fractional_orbitals",
                        NWCHEMC_DIRECT_SET_VALUE_INTEGER, "5", "6");
  assert_typed_set_scalar("nwpw:fractional_temperature",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "0.02");
  assert_typed_set_scalar("nwpw:fractional_alpha",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "0.7");
  assert_typed_set_scalar("nwpw:fractional_smeartype",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "1");
  assert_typed_set_pair("nwpw:excited_ne", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                        "7", "8");
  assert_typed_set_scalar("nwpw:lcao_skip", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_typed_set_triple("nwpw:ewald_ngrid",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "9", "10",
                          "11");
  assert_typed_set_scalar("cpmd:nose", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_typed_set_scalar("nwpw:nose", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_typed_set_scalar("cpmd:nose_restart",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "false");
  assert_typed_set_scalar("nwpw:nose_restart",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "false");
  assert_typed_set_scalar("cpmd:Pe", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "12");
  assert_typed_set_scalar("nwpw:Pe", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "12");
  assert_typed_set_scalar("cpmd:Te", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "300");
  assert_typed_set_scalar("nwpw:Te", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "300");
  assert_typed_set_scalar("cpmd:Pr", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "34");
  assert_typed_set_scalar("nwpw:Pr", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "34");
  assert_typed_set_scalar("cpmd:Tr", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "400");
  assert_typed_set_scalar("nwpw:Tr", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "400");
  assert_typed_set_scalar("cpmd:Mchain", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "2");
  assert_typed_set_scalar("nwpw:Mchain", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "2");
  assert_typed_set_scalar("cpmd:Nchain", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "3");
  assert_typed_set_scalar("nwpw:Nchain", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "3");
  assert_typed_set_scalar("cellA:boundry", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                          "periodic");
  const char *unita_values[9] = {"1", "0", "0", "0", "2",
                                 "0", "0", "0", "3"};
  assert_typed_set_values("cellA:unita", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, 9,
                          unita_values);
  assert_typed_set_triple("cellA:ngrid", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "20", "22", "24");
  assert_typed_set_triple("cellA:ngrid_small",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "10", "11",
                          "12");
  assert_typed_set_scalar("cellA:box_delta",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "1");
  assert_typed_set_scalar("cellA:box_orient",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true");
  assert_typed_set_scalar("cellA:box_type", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "1");
  assert_typed_set_scalar("ccsd:maxiter", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "20");
  assert_typed_set_scalar("ccsd:thresh", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "1e-07");
  assert_typed_set_scalar("ccsd:tol2e", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "1e-09");
  assert_typed_set_scalar("ccsd:iprt", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "2");
  assert_typed_set_scalar("ccsd:maxdiis", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "6");
  assert_typed_set_scalar("ccsd:frozen core",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "1");
  assert_typed_set_scalar("ccsd:frozen virtual",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "2");
  assert_typed_set_scalar("ccsd:usedisk", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "false");
  assert_typed_set_scalar("ccsd:fss", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "1.2");
  assert_typed_set_scalar("ccsd:fos", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "0.8");
  assert_typed_set_scalar("ccsd:use_trpdrv_nb",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true");
  assert_typed_set_scalar("ccsd:use_ccsd_omp",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true");
  assert_typed_set_scalar("ccsd:use_trpdrv_omp",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "false");
  assert_typed_set_scalar("ccsd:use_trpdrv_offload",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true");
  assert_typed_set_scalar("tce:model", NWCHEMC_DIRECT_SET_VALUE_TEXT, "ccsd");
  assert_typed_set_scalar("tce:model2e", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                          "2eorb");
  assert_typed_set_scalar("tce:perturbative", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                          "cr_(t)");
  assert_typed_set_scalar("tce:reference", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "0");
  assert_typed_set_scalar("tce:frozen core",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "1");
  assert_typed_set_scalar("tce:frozen virtual",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "2");
  assert_typed_set_scalar("tce:thresh", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "1e-08");
  assert_typed_set_scalar("tce:zlshift", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "0.01");
  assert_typed_set_scalar("tce:zlshiftl", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "0.02");
  assert_typed_set_pair("tce:zlshift2", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                        "0.03", "0.04");
  assert_typed_set_pair("tce:zlshift3", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                        "0.05", "0.06");
  assert_typed_set_scalar("tce:maxiter", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "30");
  assert_typed_set_scalar("tce:ioalg", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "6");
  assert_typed_set_scalar("tce:diis", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "7");
  assert_typed_set_scalar("tce:diis2", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "8");
  assert_typed_set_scalar("tce:diis3", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "9");
  assert_typed_set_scalar("tce:eoms", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "2");
  assert_typed_set_scalar("tce:hbard", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "600");
  assert_typed_set_scalar("tce:nroots", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "4");
  assert_typed_set_scalar("tce:target", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "3");
  assert_typed_set_scalar("tce:targetsym", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                          "b2");
  assert_typed_set_scalar("tce:symmetry", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_typed_set_scalar("tce:densmat", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_typed_set_scalar("tce:file_densmat", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                          "dens.dat");
  assert_typed_set_scalar("tce:left", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_typed_set_scalar("tce:multipole", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "3");
  assert_typed_set_scalar_entry("tce:left", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                                "true");
  assert_typed_set_scalar_entry("tce:multipole",
                                NWCHEMC_DIRECT_SET_VALUE_INTEGER, "1");
  assert_typed_set_scalar("tce:fragment", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "2");
  assert_typed_set_scalar("tce:recompf", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "false");
  assert_typed_set_scalar("tce:active_oa",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "5");
  assert_typed_set_scalar("tce:active_ob",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "4");
  assert_typed_set_scalar("tce:active_va",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "6");
  assert_typed_set_scalar("tce:active_vb",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "7");
  assert_typed_set_scalar("tce:oact", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "8");
  assert_typed_set_scalar("tce:uact", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "9");
  assert_typed_set_scalar("tce:eactmin", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "0.1");
  assert_typed_set_scalar("tce:eactmax", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "0.9");
  assert_typed_set_scalar("tce:act_excit_lvl",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "2");
  assert_typed_set_scalar("tce:maxdiff", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "0.25");
  assert_typed_set_scalar("tce:maxs", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "44");
  assert_typed_set_scalar("tce:ichopx", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "2");
  assert_typed_set_scalar("tce:i4im", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "3");
  assert_typed_set_scalar("tce:idiskx", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "1");
  assert_typed_set_scalar("tce:tilesize", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "40");
  assert_typed_set_scalar("tce:cuda", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "1");
  assert_typed_set_scalar("tce:ltcc", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_int_equal(g_set_nwpw_direct_calls, 1);
  assert_int_equal(g_nwpw_has_options, 1);
  assert_close(g_nwpw_energy_cutoff, 12.5, 1e-12);
  assert_close(g_nwpw_wavefunction_cutoff, 6.25, 1e-12);
  assert_close(g_nwpw_ewald_rcut, 3.5, 1e-12);
  assert_int_equal(g_nwpw_ewald_ncut, 9);
  assert_int_equal(g_set_brillouin_zone_calls, 1);
  assert_int_equal(g_brillouin_has_options, 1);
  assert_string_equal(g_brillouin_zone_name, "zoneA");
  assert_int_equal(g_brillouin_monkhorst_pack[0], 3);
  assert_int_equal(g_brillouin_monkhorst_pack[1], 4);
  assert_int_equal(g_brillouin_monkhorst_pack[2], -5);
  assert_int_equal(g_brillouin_max_kpoints_print, 12);
  assert_int_equal(g_brillouin_kvector_count, 2);
  assert_close(g_brillouin_kvectors[0], 0.0, 1e-12);
  assert_close(g_brillouin_kvectors[1], 0.0, 1e-12);
  assert_close(g_brillouin_kvectors[2], 0.0, 1e-12);
  assert_close(g_brillouin_kvectors[3], 0.5, 1e-12);
  assert_close(g_brillouin_kvectors[4], 0.5, 1e-12);
  assert_close(g_brillouin_kvectors[5], 0.0, 1e-12);
  assert_close(g_brillouin_kvectors[6], 0.0, 1e-12);
  assert_close(g_brillouin_kvectors[7], 0.5, 1e-12);
  assert_int_equal(g_set_pseudopotential_calls, 1);
  assert_int_equal(g_psp_count, 6);
  assert_string_equal(g_psp_elements[0], "Si");
  assert_int_equal(g_psp_types[0],
                   NWChemPseudopotentialEntry_LibraryType_library);
  assert_string_equal(g_psp_names[0], "sg15");
  assert_string_equal(g_psp_elements[1], "H");
  assert_int_equal(g_psp_types[1],
                   NWChemPseudopotentialEntry_LibraryType_pspwLibrary);
  assert_string_equal(g_psp_names[1], "hgh_lda");
  assert_string_equal(g_psp_elements[2], "O");
  assert_int_equal(g_psp_types[2],
                   NWChemPseudopotentialEntry_LibraryType_pawLibrary);
  assert_string_equal(g_psp_names[2], "paw_default");
  assert_string_equal(g_psp_elements[3], "C");
  assert_int_equal(g_psp_types[3],
                   NWChemPseudopotentialEntry_LibraryType_cpi);
  assert_string_equal(g_psp_names[3], "C.cpi");
  assert_string_equal(g_psp_elements[4], "N");
  assert_int_equal(g_psp_types[4],
                   NWChemPseudopotentialEntry_LibraryType_teter);
  assert_string_equal(g_psp_names[4], "N.teter");
  assert_string_equal(g_psp_elements[5], "*");
  assert_int_equal(g_psp_types[5],
                   NWChemPseudopotentialEntry_LibraryType_pspwLibrary);
  assert_string_equal(g_psp_names[5], "pspw_default");

  free(message);
}

static void test_embed_config_promotes_compact_simulation_cells(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message = read_file(g_compact_cells_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_null(strstr(g_input_blocks, "simulation_cell"));
  assert_int_equal(g_set_rtdb_values_calls, 1);
  const char *sc_values[9] = {"6", "0", "0", "0", "6", "0", "0", "0", "6"};
  const char *fcc_values[9] = {"4", "4", "0", "4", "0",
                               "4", "0", "4", "4"};
  const char *bcc_values[9] = {"-5", "5", "5", "5", "-5",
                               "5",  "5", "5", "-5"};
  assert_typed_set_values("scCell:unita", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, 9,
                          sc_values);
  assert_typed_set_values("fccCell:unita", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, 9,
                          fcc_values);
  assert_typed_set_values("bccCell:unita", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, 9,
                          bcc_values);
  free(message);
}

static void test_embed_config_promotes_tce_method_tokens(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message = read_file(g_tce_methods_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_int_equal(g_set_rtdb_values_calls, 1);
  assert_int_equal(g_typed_set_count, 16);
  assert_null(strstr(g_input_blocks, "dipole"));
  assert_null(strstr(g_input_blocks, "quadrupole"));
  assert_null(strstr(g_input_blocks, "octupole"));
  assert_typed_set_scalar_entry("tce:model", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                                "multi");
  assert_typed_set_scalar_entry("tce:model", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                                "eionly");
  assert_typed_set_scalar_entry("tce:model", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                                "ccsd");
  assert_typed_set_scalar_entry("tce:ccsdvar", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                                "cc2");
  assert_typed_set_scalar_entry("tce:nts", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                                "false");
  assert_typed_set_scalar_entry("tce:perturbative",
                                NWCHEMC_DIRECT_SET_VALUE_TEXT, "lambda(t)");
  assert_typed_set_scalar_entry("tce:left", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                                "true");
  assert_typed_set_scalar_entry("tce:multipole",
                                NWCHEMC_DIRECT_SET_VALUE_INTEGER, "1");
  assert_typed_set_scalar_entry("tce:multipole",
                                NWCHEMC_DIRECT_SET_VALUE_INTEGER, "2");
  assert_typed_set_scalar_entry("tce:multipole",
                                NWCHEMC_DIRECT_SET_VALUE_INTEGER, "3");
  assert_typed_set_scalar_entry("tce:model", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                                "bwccsd");
  assert_typed_set_scalar_entry("tce:mrcc", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                                "1");
  free(message);
}

static void test_embed_config_uses_direct_scf_values(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message = read_file(g_config_options_path, &message_size);
  assert_non_null(message);

  assert_int_equal(nwchemc_set_params(message, message_size), 0);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_set_scf_direct_calls, 1);
  assert_int_equal(g_scf_has_options, 1);
  assert_int_equal(g_scf_maxiter, 50);
  assert_close(g_scf_thresh, 1.0e-6, 1.0e-12);
  assert_close(g_scf_tol2e, 1.0e-9, 1.0e-15);
  assert_int_equal(g_set_driver_direct_calls, 1);
  assert_int_equal(g_driver_has_options, 1);
  assert_int_equal(g_driver_maxiter, 40);
  assert_int_equal(g_driver_tolerance_mode, NWCHEMC_DRIVER_TOLERANCE_TIGHT);
  assert_close(g_driver_gmax_tol, 2.5e-5, 1.0e-12);
  assert_close(g_driver_grms_tol, 1.5e-5, 1.0e-12);
  assert_close(g_driver_xmax_tol, 7.5e-5, 1.0e-12);
  assert_close(g_driver_xrms_tol, 5.5e-5, 1.0e-12);
  assert_int_equal(g_set_rtdb_strings_calls, 1);
  assert_int_equal(g_set_string_count, 1);
  assert_string_equal(g_set_keys[0], "dft:grid");
  assert_string_equal(g_set_values[0], "xfine");
  assert_int_equal(g_set_rtdb_values_calls, 1);
  assert_int_equal(g_typed_set_count, 3);
  assert_string_equal(g_typed_set_keys[0], "dft:nopen");
  assert_int_equal(g_typed_set_types[0], NWCHEMC_DIRECT_SET_VALUE_INTEGER);
  assert_int_equal(g_typed_set_value_counts[0], 1);
  assert_string_equal(g_typed_set_values[0][0], "2");
  assert_string_equal(g_typed_set_keys[1], "dft:smear_sigma");
  assert_int_equal(g_typed_set_types[1], NWCHEMC_DIRECT_SET_VALUE_DOUBLE);
  assert_int_equal(g_typed_set_value_counts[1], 1);
  assert_string_equal(g_typed_set_values[1][0], "0.0015");
  assert_string_equal(g_typed_set_keys[2], "dft:spinset");
  assert_int_equal(g_typed_set_types[2], NWCHEMC_DIRECT_SET_VALUE_LOGICAL);
  assert_int_equal(g_typed_set_value_counts[2], 1);
  assert_string_equal(g_typed_set_values[2][0], "false");
  assert_null(strstr(g_input_blocks, "scf\n"));
  assert_null(strstr(g_input_blocks, "maxiter 50"));
  assert_null(strstr(g_input_blocks, "tol2e 1e-09"));
  assert_null(strstr(g_input_blocks, "driver\n"));
  assert_null(strstr(g_input_blocks, "maxiter 40"));
  assert_null(strstr(g_input_blocks, "tight"));
  assert_null(strstr(g_input_blocks, "gmax 2.5e-05"));
  assert_null(strstr(g_input_blocks, "grms 1.5e-05"));
  assert_null(strstr(g_input_blocks, "xmax 7.5e-05"));
  assert_null(strstr(g_input_blocks, "xrms 5.5e-05"));
  assert_null(strstr(g_input_blocks, "task scf energy"));
  assert_null(strstr(g_input_blocks, "set dft:grid xfine"));
  assert_null(strstr(g_input_blocks, "set dft:nopen integer 2"));
  assert_null(strstr(g_input_blocks, "set dft:smear_sigma double 0.0015"));
  assert_null(strstr(g_input_blocks, "set dft:spinset logical false"));

  free(message);
}

static void test_embed_config_promotes_pspspin_rules(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message = read_file(g_pspspin_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_null(strstr(g_input_blocks, "pspspin"));
  assert_int_equal(g_set_rtdb_values_calls, 1);
  assert_int_equal(g_typed_set_count, 11);
  assert_typed_set_scalar("nwpw:pspspin", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_typed_set_scalar("nwpw:pspspin_count",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "2");
  assert_typed_set_scalar("nwpw:pspspin_iamup:_000001",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true");
  assert_typed_set_scalar("nwpw:pspspin_upscale:_000001",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "1.25");
  assert_typed_set_scalar("nwpw:pspspin_upl:_000001",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "1");
  const char *up_ions[] = {"1", "3"};
  assert_typed_set_values("nwpw:pspspin_upions:_000001",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, 2, up_ions);
  assert_typed_set_scalar("nwpw:pspspin_iamup:_000002",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "false");
  assert_typed_set_scalar("nwpw:pspspin_downscale:_000002",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "0.75");
  assert_typed_set_scalar("nwpw:pspspin_downl:_000002",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "2");
  assert_typed_set_scalar("nwpw:pspspin_downm:_000002",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "0");
  const char *down_ions[] = {"2"};
  assert_typed_set_values("nwpw:pspspin_downions:_000002",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, 1, down_ions);

  free(message);
}

static void test_embed_config_promotes_large_pspspin_ion_list(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message = read_file(g_pspspin_many_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_null(strstr(g_input_blocks, "pspspin"));
  assert_int_equal(g_set_rtdb_values_calls, 1);
  assert_int_equal(g_typed_set_count, 6);
  assert_typed_set_scalar("nwpw:pspspin", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_typed_set_scalar("nwpw:pspspin_count",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "1");
  assert_typed_set_scalar("nwpw:pspspin_iamup:_000001",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true");
  assert_typed_set_scalar("nwpw:pspspin_upscale:_000001",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "2.5");
  assert_typed_set_scalar("nwpw:pspspin_upl:_000001",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "3");
  const char *ions[] = {
      "1",  "2",  "3",  "4",  "5",  "6",  "7",
      "8",  "9",  "10", "11", "12", "13", "14",
      "15", "16", "17", "18", "19", "20"};
  assert_typed_set_values("nwpw:pspspin_upions:_000001",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, 20, ions);

  free(message);
}

static void test_session_reuses_config_across_geometry_steps(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  assert_non_null(message);

  NWChemCSession *session =
      nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_set_dft_direct_calls, 1);
  assert_int_equal(g_set_pseudopotential_calls, 1);

  double pos_a[3] = {0.0, 0.0, 0.0};
  double pos_b[3] = {0.0, 0.0, 0.1};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};

  NWChemCResult first =
      nwchemc_session_energy_gradient(session, 1, pos_a, z, grad);
  NWChemCResult second =
      nwchemc_session_energy_gradient(session, 1, pos_b, z, grad);

  assert_int_equal(first.ok, 1);
  assert_int_equal(second.ok, 1);
  assert_int_equal(g_energy_grad_calls, 2);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_set_dft_direct_calls, 1);

  int changed_z[1] = {8};
  NWChemCResult changed_species =
      nwchemc_session_energy_gradient(session, 1, pos_b, changed_z, grad);
  assert_int_equal(changed_species.ok, 0);
  assert_non_null(strstr(changed_species.message, "topology"));
  assert_int_equal(g_energy_grad_calls, 2);
  assert_int_equal(g_set_config_calls, 1);

  nwchemc_session_destroy(session);
  free(message);
}

static void test_session_reapplies_after_one_shot_config(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  assert_non_null(message);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};

  NWChemCResult first =
      nwchemc_session_energy_gradient(session, 1, pos, z, grad);
  assert_int_equal(first.ok, 1);
  assert_int_equal(g_set_config_calls, 1);

  NWChemCResult one_shot =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);
  assert_int_equal(one_shot.ok, 1);
  assert_int_equal(g_set_config_calls, 2);

  NWChemCResult second =
      nwchemc_session_energy_gradient(session, 1, pos, z, grad);
  assert_int_equal(second.ok, 1);
  assert_int_equal(g_set_config_calls, 3);

  NWChemCResult third =
      nwchemc_session_energy_gradient(session, 1, pos, z, grad);
  assert_int_equal(third.ok, 1);
  assert_int_equal(g_set_config_calls, 3);

  nwchemc_session_destroy(session);
  free(message);
}

static void test_session_rejects_param_replacement_after_topology(
    void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  size_t replacement_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *replacement =
      read_file(g_config_options_path, &replacement_size);
  assert_non_null(message);
  assert_non_null(replacement);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult first =
      nwchemc_session_energy_gradient(session, 1, pos, z, grad);
  assert_int_equal(first.ok, 1);
  assert_int_equal(g_set_config_calls, 1);

  assert_int_not_equal(
      nwchemc_session_set_params(session, replacement, replacement_size), 0);
  assert_int_equal(g_set_config_calls, 1);

  nwchemc_session_destroy(session);
  free(replacement);
  free(message);
}

static void test_session_calculate_forces_accepts_force_input_steps(
    void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  size_t step_a_size = 0;
  size_t step_b_size = 0;
  size_t step_changed_species_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  unsigned char *step_b = read_file(g_force_step_b_path, &step_b_size);
  unsigned char *step_changed_species = read_file(
      g_force_step_changed_species_path, &step_changed_species_size);
  assert_non_null(message);
  assert_non_null(step_a);
  assert_non_null(step_b);
  assert_non_null(step_changed_species);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);

  double forces[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult first = nwchemc_session_calculate_forces(
      session, step_a, step_a_size, forces, 6);
  assert_int_equal(first.ok, 1);
  assert_int_equal(g_energy_grad_calls, 1);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_call_n_atoms[0], 2);
  assert_int_equal(g_call_atomic_numbers[0][0], 1);
  assert_int_equal(g_call_atomic_numbers[0][1], 8);
  assert_close(g_call_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_call_has_cell[0], 1);
  assert_close(g_call_cell_ang[0][0], 10.0, 1.0e-12);
  assert_close(forces[0], -1.0, 1.0e-12);
  assert_close(forces[5], -6.0, 1.0e-12);

  NWChemCResult second = nwchemc_session_calculate_forces(
      session, step_b, step_b_size, forces, 6);
  assert_int_equal(second.ok, 1);
  assert_int_equal(g_energy_grad_calls, 2);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_call_n_atoms[1], 2);
  assert_int_equal(g_call_atomic_numbers[1][0], 1);
  assert_int_equal(g_call_atomic_numbers[1][1], 8);
  assert_close(g_call_positions_ang[1][5], 1.058354421806, 1.0e-12);
  assert_int_equal(g_call_has_cell[1], 1);
  assert_close(g_call_cell_ang[1][0], 10.58354421806, 1.0e-11);

  NWChemCResult changed_species = nwchemc_session_calculate_forces(
      session, step_changed_species, step_changed_species_size, forces, 6);
  assert_int_equal(changed_species.ok, 0);
  assert_non_null(strstr(changed_species.message, "topology"));
  assert_int_equal(g_energy_grad_calls, 2);

  NWChemCResult short_output = nwchemc_session_calculate_forces(
      session, step_b, step_b_size, forces, 5);
  assert_int_equal(short_output.ok, 0);
  assert_int_equal(g_energy_grad_calls, 2);

  nwchemc_session_destroy(session);
  free(step_changed_species);
  free(step_b);
  free(step_a);
  free(message);
}

static void test_session_calculate_hessian_accepts_force_input_step(
    void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  size_t step_a_size = 0;
  size_t step_changed_species_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  unsigned char *step_changed_species = read_file(
      g_force_step_changed_species_path, &step_changed_species_size);
  assert_non_null(message);
  assert_non_null(step_a);
  assert_non_null(step_changed_species);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);

  double hessian[36] = {0.0};
  NWChemCResult first = nwchemc_session_calculate_hessian(
      session, step_a, step_a_size, hessian, 36);
  assert_int_equal(first.ok, 1);
  assert_int_equal(g_hessian_calls, 1);
  assert_int_equal(g_hessian_cell_calls, 1);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_hessian_n_atoms[0], 2);
  assert_int_equal(g_hessian_atomic_numbers[0][0], 1);
  assert_int_equal(g_hessian_atomic_numbers[0][1], 8);
  assert_close(g_hessian_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_hessian_has_cell[0], 1);
  assert_close(g_hessian_cell_ang[0][0], 10.0, 1.0e-12);
  assert_close(hessian[0], 10.0, 1.0e-12);
  assert_close(hessian[35], 45.0, 1.0e-12);

  NWChemCResult changed_species = nwchemc_session_calculate_hessian(
      session, step_changed_species, step_changed_species_size, hessian, 36);
  assert_int_equal(changed_species.ok, 0);
  assert_non_null(strstr(changed_species.message, "topology"));
  assert_int_equal(g_hessian_calls, 1);

  NWChemCResult short_output = nwchemc_session_calculate_hessian(
      session, step_a, step_a_size, hessian, 35);
  assert_int_equal(short_output.ok, 0);
  assert_int_equal(g_hessian_calls, 1);

  nwchemc_session_destroy(session);
  free(step_changed_species);
  free(step_a);
  free(message);
}

static void test_session_calculate_dipole_accepts_force_input_step(
    void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_session_calculate_dipole != NULL);
  size_t message_size = 0;
  size_t step_a_size = 0;
  size_t step_changed_species_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  unsigned char *step_changed_species = read_file(
      g_force_step_changed_species_path, &step_changed_species_size);
  assert_non_null(message);
  assert_non_null(step_a);
  assert_non_null(step_changed_species);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);

  double dipole[3] = {0.0, 0.0, 0.0};
  NWChemCResult first = nwchemc_session_calculate_dipole(
      session, step_a, step_a_size, dipole, 3);
  assert_int_equal(first.ok, 1);
  assert_close(first.energy_h, -1.25, 1.0e-12);
  assert_int_equal(g_dipole_calls, 1);
  assert_int_equal(g_dipole_cell_calls, 1);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_dipole_n_atoms[0], 2);
  assert_int_equal(g_dipole_atomic_numbers[0][0], 1);
  assert_int_equal(g_dipole_atomic_numbers[0][1], 8);
  assert_close(g_dipole_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_dipole_has_cell[0], 1);
  assert_close(g_dipole_cell_ang[0][0], 10.0, 1.0e-12);
  assert_close(dipole[0], 0.25, 1.0e-12);
  assert_close(dipole[1], 0.5, 1.0e-12);
  assert_close(dipole[2], 0.75, 1.0e-12);

  NWChemCResult changed_species = nwchemc_session_calculate_dipole(
      session, step_changed_species, step_changed_species_size, dipole, 3);
  assert_int_equal(changed_species.ok, 0);
  assert_non_null(strstr(changed_species.message, "topology"));
  assert_int_equal(g_dipole_calls, 1);

  NWChemCResult short_output = nwchemc_session_calculate_dipole(
      session, step_a, step_a_size, dipole, 2);
  assert_int_equal(short_output.ok, 0);
  assert_int_equal(g_dipole_calls, 1);

  nwchemc_session_destroy(session);
  free(step_changed_species);
  free(step_a);
  free(message);
}

static void test_session_calculate_quadrupole_accepts_force_input_step(
    void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_session_calculate_quadrupole != NULL);
  size_t message_size = 0;
  size_t step_a_size = 0;
  size_t step_changed_species_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  unsigned char *step_changed_species = read_file(
      g_force_step_changed_species_path, &step_changed_species_size);
  assert_non_null(message);
  assert_non_null(step_a);
  assert_non_null(step_changed_species);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);

  double quadrupole[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult first = nwchemc_session_calculate_quadrupole(
      session, step_a, step_a_size, quadrupole, 6);
  assert_int_equal(first.ok, 1);
  assert_close(first.energy_h, -1.5, 1.0e-12);
  assert_int_equal(g_quadrupole_calls, 1);
  assert_int_equal(g_quadrupole_cell_calls, 1);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_quadrupole_n_atoms[0], 2);
  assert_int_equal(g_quadrupole_atomic_numbers[0][0], 1);
  assert_int_equal(g_quadrupole_atomic_numbers[0][1], 8);
  assert_close(g_quadrupole_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_quadrupole_has_cell[0], 1);
  assert_close(g_quadrupole_cell_ang[0][0], 10.0, 1.0e-12);
  assert_close(quadrupole[0], 0.125, 1.0e-12);
  assert_close(quadrupole[5], 0.75, 1.0e-12);

  NWChemCResult changed_species = nwchemc_session_calculate_quadrupole(
      session, step_changed_species, step_changed_species_size, quadrupole, 6);
  assert_int_equal(changed_species.ok, 0);
  assert_non_null(strstr(changed_species.message, "topology"));
  assert_int_equal(g_quadrupole_calls, 1);

  NWChemCResult short_output = nwchemc_session_calculate_quadrupole(
      session, step_a, step_a_size, quadrupole, 5);
  assert_int_equal(short_output.ok, 0);
  assert_int_equal(g_quadrupole_calls, 1);

  nwchemc_session_destroy(session);
  free(step_changed_species);
  free(step_a);
  free(message);
}

static void test_session_calculate_optimize_accepts_force_input_step(
    void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_session_calculate_optimize != NULL);
  size_t message_size = 0;
  size_t step_a_size = 0;
  size_t step_changed_species_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  unsigned char *step_changed_species = read_file(
      g_force_step_changed_species_path, &step_changed_species_size);
  assert_non_null(message);
  assert_non_null(step_a);
  assert_non_null(step_changed_species);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);

  double optimized_positions[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult first = nwchemc_session_calculate_optimize(
      session, step_a, step_a_size, optimized_positions, 6);
  assert_int_equal(first.ok, 1);
  assert_close(first.energy_h, -1.75, 1.0e-12);
  assert_int_equal(g_optimize_calls, 1);
  assert_int_equal(g_optimize_cell_calls, 1);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_optimize_n_atoms[0], 2);
  assert_int_equal(g_optimize_atomic_numbers[0][0], 1);
  assert_int_equal(g_optimize_atomic_numbers[0][1], 8);
  assert_close(g_optimize_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_optimize_has_cell[0], 1);
  assert_close(g_optimize_cell_ang[0][0], 10.0, 1.0e-12);
  assert_close(optimized_positions[0], 0.01, 1.0e-12);
  assert_close(optimized_positions[5], 0.8014, 1.0e-12);

  NWChemCResult changed_species = nwchemc_session_calculate_optimize(
      session, step_changed_species, step_changed_species_size,
      optimized_positions, 6);
  assert_int_equal(changed_species.ok, 0);
  assert_non_null(strstr(changed_species.message, "topology"));
  assert_int_equal(g_optimize_calls, 1);

  NWChemCResult short_output = nwchemc_session_calculate_optimize(
      session, step_a, step_a_size, optimized_positions, 5);
  assert_int_equal(short_output.ok, 0);
  assert_int_equal(g_optimize_calls, 1);

  nwchemc_session_destroy(session);
  free(step_changed_species);
  free(step_a);
  free(message);
}

static void test_session_calculate_frequencies_accepts_force_input_step(
    void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_session_calculate_frequencies != NULL);
  size_t message_size = 0;
  size_t step_a_size = 0;
  size_t step_changed_species_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  unsigned char *step_changed_species = read_file(
      g_force_step_changed_species_path, &step_changed_species_size);
  assert_non_null(message);
  assert_non_null(step_a);
  assert_non_null(step_changed_species);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);

  double frequencies[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  double intensities[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult first = nwchemc_session_calculate_frequencies(
      session, step_a, step_a_size, frequencies, 6, intensities, 6);
  assert_int_equal(first.ok, 1);
  assert_int_equal(g_frequency_calls, 1);
  assert_int_equal(g_frequency_cell_calls, 1);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_frequency_n_atoms[0], 2);
  assert_int_equal(g_frequency_atomic_numbers[0][0], 1);
  assert_int_equal(g_frequency_atomic_numbers[0][1], 8);
  assert_close(g_frequency_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_frequency_has_cell[0], 1);
  assert_close(g_frequency_cell_ang[0][0], 10.0, 1.0e-12);
  assert_close(frequencies[0], 100.0, 1.0e-12);
  assert_close(frequencies[5], 105.0, 1.0e-12);
  assert_close(intensities[0], 0.01, 1.0e-12);
  assert_close(intensities[5], 0.06, 1.0e-12);

  NWChemCResult changed_species = nwchemc_session_calculate_frequencies(
      session, step_changed_species, step_changed_species_size, frequencies, 6,
      intensities, 6);
  assert_int_equal(changed_species.ok, 0);
  assert_non_null(strstr(changed_species.message, "topology"));
  assert_int_equal(g_frequency_calls, 1);

  NWChemCResult short_frequency_output = nwchemc_session_calculate_frequencies(
      session, step_a, step_a_size, frequencies, 5, intensities, 6);
  assert_int_equal(short_frequency_output.ok, 0);
  assert_int_equal(g_frequency_calls, 1);

  NWChemCResult short_intensity_output = nwchemc_session_calculate_frequencies(
      session, step_a, step_a_size, frequencies, 6, intensities, 5);
  assert_int_equal(short_intensity_output.ok, 0);
  assert_int_equal(g_frequency_calls, 1);

  nwchemc_session_destroy(session);
  free(step_changed_species);
  free(step_a);
  free(message);
}

static void test_session_force_input_state_overrides_params(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  size_t step_a_size = 0;
  size_t step_state_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  unsigned char *step_state = read_file(g_force_step_state_path,
                                        &step_state_size);
  assert_non_null(message);
  assert_non_null(step_a);
  assert_non_null(step_state);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);

  double forces[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult default_forces =
      nwchemc_session_calculate_forces(session, step_a, step_a_size, forces, 6);
  assert_int_equal(default_forces.ok, 1);
  assert_int_equal(g_call_charge[0], 0);
  assert_int_equal(g_call_multiplicity[0], 1);

  NWChemCResult override_forces = nwchemc_session_calculate_forces(
      session, step_state, step_state_size, forces, 6);
  assert_int_equal(override_forces.ok, 1);
  assert_int_equal(g_call_charge[1], -2);
  assert_int_equal(g_call_multiplicity[1], 5);

  unsigned char result_bytes[512];
  size_t result_size = 0;
  NWChemCResult force_result = nwchemc_session_calculate_result(
      session, step_state, step_state_size, result_bytes, sizeof(result_bytes),
      &result_size);
  assert_int_equal(force_result.ok, 1);
  assert_int_equal(g_call_charge[2], -2);
  assert_int_equal(g_call_multiplicity[2], 5);

  double hessian[36] = {0.0};
  NWChemCResult hessian_result = nwchemc_session_calculate_hessian(
      session, step_state, step_state_size, hessian, 36);
  assert_int_equal(hessian_result.ok, 1);
  assert_int_equal(g_hessian_charge[0], -2);
  assert_int_equal(g_hessian_multiplicity[0], 5);

  result_size = 0;
  NWChemCResult hessian_carrier = nwchemc_session_calculate_hessian_result(
      session, step_state, step_state_size, result_bytes, sizeof(result_bytes),
      &result_size);
  assert_int_equal(hessian_carrier.ok, 1);
  assert_int_equal(g_hessian_charge[1], -2);
  assert_int_equal(g_hessian_multiplicity[1], 5);

  result_size = 0;
  NWChemCResult dipole_carrier = nwchemc_session_calculate_dipole_result(
      session, step_state, step_state_size, result_bytes, sizeof(result_bytes),
      &result_size);
  assert_int_equal(dipole_carrier.ok, 1);
  assert_int_equal(g_dipole_charge[0], -2);
  assert_int_equal(g_dipole_multiplicity[0], 5);

  result_size = 0;
  NWChemCResult quadrupole_carrier =
      nwchemc_session_calculate_quadrupole_result(
          session, step_state, step_state_size, result_bytes,
          sizeof(result_bytes), &result_size);
  assert_int_equal(quadrupole_carrier.ok, 1);
  assert_int_equal(g_quadrupole_charge[0], -2);
  assert_int_equal(g_quadrupole_multiplicity[0], 5);

  result_size = 0;
  NWChemCResult optimize_carrier = nwchemc_session_calculate_optimize_result(
      session, step_state, step_state_size, result_bytes, sizeof(result_bytes),
      &result_size);
  assert_int_equal(optimize_carrier.ok, 1);
  assert_int_equal(g_optimize_charge[0], -2);
  assert_int_equal(g_optimize_multiplicity[0], 5);

  result_size = 0;
  NWChemCResult frequency_carrier =
      nwchemc_session_calculate_frequencies_result(
          session, step_state, step_state_size, result_bytes,
          sizeof(result_bytes), &result_size);
  assert_int_equal(frequency_carrier.ok, 1);
  assert_int_equal(g_frequency_charge[0], -2);
  assert_int_equal(g_frequency_multiplicity[0], 5);

  nwchemc_session_destroy(session);
  free(step_state);
  free(step_a);
  free(message);
}

static void test_session_calculate_result_writes_potential_result(
    void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  size_t step_a_size = 0;
  size_t step_b_size = 0;
  size_t step_ev_size = 0;
  size_t step_changed_species_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  unsigned char *step_b = read_file(g_force_step_b_path, &step_b_size);
  unsigned char *step_ev = read_file(g_force_step_ev_path, &step_ev_size);
  unsigned char *step_changed_species = read_file(
      g_force_step_changed_species_path, &step_changed_species_size);
  assert_non_null(message);
  assert_non_null(step_a);
  assert_non_null(step_b);
  assert_non_null(step_ev);
  assert_non_null(step_changed_species);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);

  unsigned char result_bytes[256];
  size_t result_size = 0;
  size_t expected_step_a_size =
      nwchemc_potential_result_size_for_force_input(step_a, step_a_size);
  assert_true(expected_step_a_size > 0);
  NWChemCResult native = nwchemc_session_calculate_result(
      session, step_a, step_a_size, result_bytes, sizeof(result_bytes),
      &result_size);
  assert_int_equal(native.ok, 1);
  assert_close(native.energy_h, -1.0, 1.0e-12);
  assert_int_equal(result_size, expected_step_a_size);
  assert_true(result_size < sizeof(result_bytes));
  const double native_forces[6] = {-1.0, -2.0, -3.0, -4.0, -5.0, -6.0};
  const double bohr_to_angstrom = 0.529177210903;
  double hartree_angstrom_forces[6];
  for (int i = 0; i < 6; ++i)
    hartree_angstrom_forces[i] = native_forces[i] / bohr_to_angstrom;
  assert_potential_result(result_bytes, result_size, -1.0,
                          hartree_angstrom_forces, 6,
                          1.0e-12);
  assert_int_equal(g_energy_grad_calls, 1);
  assert_int_equal(g_set_config_calls, 1);

  size_t bohr_result_size = 0;
  size_t expected_step_b_size =
      nwchemc_potential_result_size_for_force_input(step_b, step_b_size);
  assert_int_equal(expected_step_b_size, expected_step_a_size);
  NWChemCResult bohr = nwchemc_session_calculate_result(
      session, step_b, step_b_size, result_bytes, sizeof(result_bytes),
      &bohr_result_size);
  assert_int_equal(bohr.ok, 1);
  assert_close(bohr.energy_h, -1.0, 1.0e-12);
  assert_int_equal(bohr_result_size, result_size);
  assert_potential_result(result_bytes, bohr_result_size, -1.0,
                          native_forces, 6, 1.0e-12);
  assert_int_equal(g_energy_grad_calls, 2);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_call_n_atoms[1], 2);
  assert_close(g_call_positions_ang[1][5], 1.058354421806, 1.0e-12);
  assert_int_equal(g_call_has_cell[1], 1);
  assert_close(g_call_cell_ang[1][0], 10.58354421806, 1.0e-11);

  size_t changed_result_size = 0;
  NWChemCResult changed_species = nwchemc_session_calculate_result(
      session, step_changed_species, step_changed_species_size, result_bytes,
      sizeof(result_bytes), &changed_result_size);
  assert_int_equal(changed_species.ok, 0);
  assert_non_null(strstr(changed_species.message, "topology"));
  assert_int_equal(changed_result_size, result_size);
  assert_int_equal(g_energy_grad_calls, 2);

  unsigned char short_result[79];
  size_t required_size = 0;
  NWChemCResult short_output = nwchemc_session_calculate_result(
      session, step_a, step_a_size, short_result, sizeof(short_result),
      &required_size);
  assert_int_equal(short_output.ok, 0);
  assert_int_equal(required_size, result_size);
  assert_int_equal(g_energy_grad_calls, 2);

  result_size = 0;
  NWChemCResult ev = nwchemc_session_calculate_result(
      session, step_ev, step_ev_size, result_bytes, sizeof(result_bytes),
      &result_size);
  assert_int_equal(ev.ok, 1);
  assert_close(ev.energy_h, -1.0, 1.0e-12);
  const double hartree_to_ev = 27.211386245988;
  double ev_forces[6];
  for (int i = 0; i < 6; ++i)
    ev_forces[i] = native_forces[i] * hartree_to_ev / bohr_to_angstrom;
  assert_potential_result(result_bytes, result_size, -hartree_to_ev, ev_forces,
                          6, 1.0e-10);
  assert_int_equal(g_energy_grad_calls, 3);
  assert_int_equal(g_set_config_calls, 1);

  nwchemc_session_destroy(session);
  free(step_changed_species);
  free(step_ev);
  free(step_b);
  free(step_a);
  free(message);
}

static void test_session_calculate_hessian_result_writes_potential_result(
    void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_hessian_result_size_for_force_input != NULL);
  assert_true(nwchemc_session_calculate_hessian_result != NULL);
  size_t message_size = 0;
  size_t step_a_size = 0;
  size_t step_changed_species_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  unsigned char *step_changed_species = read_file(
      g_force_step_changed_species_path, &step_changed_species_size);
  assert_non_null(message);
  assert_non_null(step_a);
  assert_non_null(step_changed_species);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);

  unsigned char result_bytes[512];
  size_t result_size = 0;
  size_t expected_size =
      nwchemc_hessian_result_size_for_force_input(step_a, step_a_size);
  assert_true(expected_size > 0);
  NWChemCResult result = nwchemc_session_calculate_hessian_result(
      session, step_a, step_a_size, result_bytes, sizeof(result_bytes),
      &result_size);
  assert_int_equal(result.ok, 1);
  assert_int_equal(result_size, expected_size);
  assert_int_equal(g_hessian_calls, 1);
  assert_int_equal(g_hessian_cell_calls, 1);
  double expected_hessian[36];
  const double bohr_to_angstrom = 0.529177210903;
  for (int i = 0; i < 36; ++i)
    expected_hessian[i] =
        (double)(i + 10) / (bohr_to_angstrom * bohr_to_angstrom);
  assert_potential_result_hessian(result_bytes, result_size, expected_hessian,
                                  36, 1.0e-10);

  size_t changed_result_size = 0;
  NWChemCResult changed_species = nwchemc_session_calculate_hessian_result(
      session, step_changed_species, step_changed_species_size, result_bytes,
      sizeof(result_bytes), &changed_result_size);
  assert_int_equal(changed_species.ok, 0);
  assert_non_null(strstr(changed_species.message, "topology"));
  assert_int_equal(changed_result_size, expected_size);
  assert_int_equal(g_hessian_calls, 1);

  unsigned char short_result[127];
  size_t required_size = 0;
  NWChemCResult short_output = nwchemc_session_calculate_hessian_result(
      session, step_a, step_a_size, short_result, sizeof(short_result),
      &required_size);
  assert_int_equal(short_output.ok, 0);
  assert_int_equal(required_size, expected_size);
  assert_int_equal(g_hessian_calls, 1);

  nwchemc_session_destroy(session);
  free(step_changed_species);
  free(step_a);
  free(message);
}

static void test_session_calculate_property_results_write_potential_result(
    void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_dipole_result_size_for_force_input != NULL);
  assert_true(nwchemc_session_calculate_dipole_result != NULL);
  assert_true(nwchemc_quadrupole_result_size_for_force_input != NULL);
  assert_true(nwchemc_session_calculate_quadrupole_result != NULL);
  size_t message_size = 0;
  size_t step_a_size = 0;
  size_t step_changed_species_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  unsigned char *step_changed_species = read_file(
      g_force_step_changed_species_path, &step_changed_species_size);
  assert_non_null(message);
  assert_non_null(step_a);
  assert_non_null(step_changed_species);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);

  unsigned char result_bytes[256];
  size_t dipole_result_size = 0;
  size_t expected_dipole_size =
      nwchemc_dipole_result_size_for_force_input(step_a, step_a_size);
  assert_true(expected_dipole_size > 0);
  NWChemCResult dipole_result = nwchemc_session_calculate_dipole_result(
      session, step_a, step_a_size, result_bytes, sizeof(result_bytes),
      &dipole_result_size);
  assert_int_equal(dipole_result.ok, 1);
  assert_close(dipole_result.energy_h, -1.25, 1.0e-12);
  assert_int_equal(dipole_result_size, expected_dipole_size);
  assert_int_equal(g_dipole_calls, 1);
  assert_int_equal(g_dipole_cell_calls, 1);
  const double expected_dipole[3] = {0.25, 0.5, 0.75};
  assert_potential_result_dipole(result_bytes, dipole_result_size,
                                 expected_dipole, 1.0e-12);

  size_t changed_dipole_size = 0;
  NWChemCResult changed_dipole =
      nwchemc_session_calculate_dipole_result(
          session, step_changed_species, step_changed_species_size,
          result_bytes, sizeof(result_bytes), &changed_dipole_size);
  assert_int_equal(changed_dipole.ok, 0);
  assert_non_null(strstr(changed_dipole.message, "topology"));
  assert_int_equal(changed_dipole_size, expected_dipole_size);
  assert_int_equal(g_dipole_calls, 1);

  unsigned char short_result[63];
  size_t required_size = 0;
  NWChemCResult short_dipole = nwchemc_session_calculate_dipole_result(
      session, step_a, step_a_size, short_result, sizeof(short_result),
      &required_size);
  assert_int_equal(short_dipole.ok, 0);
  assert_int_equal(required_size, expected_dipole_size);
  assert_int_equal(g_dipole_calls, 1);

  size_t quadrupole_result_size = 0;
  size_t expected_quadrupole_size =
      nwchemc_quadrupole_result_size_for_force_input(step_a, step_a_size);
  assert_true(expected_quadrupole_size > 0);
  NWChemCResult quadrupole_result =
      nwchemc_session_calculate_quadrupole_result(
          session, step_a, step_a_size, result_bytes, sizeof(result_bytes),
          &quadrupole_result_size);
  assert_int_equal(quadrupole_result.ok, 1);
  assert_close(quadrupole_result.energy_h, -1.5, 1.0e-12);
  assert_int_equal(quadrupole_result_size, expected_quadrupole_size);
  assert_int_equal(g_quadrupole_calls, 1);
  assert_int_equal(g_quadrupole_cell_calls, 1);
  const double expected_quadrupole[6] = {0.125, 0.25, 0.375,
                                         0.5,   0.625, 0.75};
  assert_potential_result_quadrupole(result_bytes, quadrupole_result_size,
                                     expected_quadrupole, 1.0e-12);

  size_t changed_quadrupole_size = 0;
  NWChemCResult changed_quadrupole =
      nwchemc_session_calculate_quadrupole_result(
          session, step_changed_species, step_changed_species_size,
          result_bytes, sizeof(result_bytes), &changed_quadrupole_size);
  assert_int_equal(changed_quadrupole.ok, 0);
  assert_non_null(strstr(changed_quadrupole.message, "topology"));
  assert_int_equal(changed_quadrupole_size, expected_quadrupole_size);
  assert_int_equal(g_quadrupole_calls, 1);

  required_size = 0;
  NWChemCResult short_quadrupole =
      nwchemc_session_calculate_quadrupole_result(
          session, step_a, step_a_size, short_result, sizeof(short_result),
          &required_size);
  assert_int_equal(short_quadrupole.ok, 0);
  assert_int_equal(required_size, expected_quadrupole_size);
  assert_int_equal(g_quadrupole_calls, 1);

  nwchemc_session_destroy(session);
  free(step_changed_species);
  free(step_a);
  free(message);
}

static void test_session_calculate_structural_results_write_potential_result(
    void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_optimize_result_size_for_force_input != NULL);
  assert_true(nwchemc_session_calculate_optimize_result != NULL);
  assert_true(nwchemc_frequencies_result_size_for_force_input != NULL);
  assert_true(nwchemc_session_calculate_frequencies_result != NULL);
  size_t message_size = 0;
  size_t step_a_size = 0;
  size_t step_b_size = 0;
  size_t step_changed_species_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  unsigned char *step_b = read_file(g_force_step_b_path, &step_b_size);
  unsigned char *step_changed_species = read_file(
      g_force_step_changed_species_path, &step_changed_species_size);
  assert_non_null(message);
  assert_non_null(step_a);
  assert_non_null(step_b);
  assert_non_null(step_changed_species);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);

  unsigned char result_bytes[512];
  unsigned char short_result[63];
  size_t result_size = 0;
  size_t required_size = 0;
  const double expected_optimized_ang[6] = {0.01, 0.02, 0.03,
                                            0.04, 0.05, 0.8014};
  size_t expected_optimize_size =
      nwchemc_optimize_result_size_for_force_input(step_a, step_a_size);
  assert_true(expected_optimize_size > 0);

  NWChemCResult short_optimize =
      nwchemc_session_calculate_optimize_result(
          session, step_a, step_a_size, short_result, sizeof(short_result),
          &required_size);
  assert_int_equal(short_optimize.ok, 0);
  assert_int_equal(required_size, expected_optimize_size);
  assert_int_equal(g_optimize_calls, 0);

  NWChemCResult optimize_result = nwchemc_session_calculate_optimize_result(
      session, step_a, step_a_size, result_bytes, sizeof(result_bytes),
      &result_size);
  assert_int_equal(optimize_result.ok, 1);
  assert_close(optimize_result.energy_h, -1.75, 1.0e-12);
  assert_int_equal(result_size, expected_optimize_size);
  assert_int_equal(g_optimize_calls, 1);
  assert_int_equal(g_optimize_cell_calls, 1);
  assert_potential_result_optimized(result_bytes, result_size, -1.75,
                                    expected_optimized_ang, 6, 1.0e-12);

  const double bohr_to_angstrom = 0.529177210903;
  double expected_optimized_bohr[6];
  for (int i = 0; i < 6; ++i) {
    double input_ang = i == 5 ? 2.0 * bohr_to_angstrom : 0.0;
    expected_optimized_bohr[i] =
        (input_ang + 0.01 * (double)(i + 1)) / bohr_to_angstrom;
  }
  size_t expected_bohr_optimize_size =
      nwchemc_optimize_result_size_for_force_input(step_b, step_b_size);
  assert_int_equal(expected_bohr_optimize_size, expected_optimize_size);
  result_size = 0;
  NWChemCResult optimize_bohr = nwchemc_session_calculate_optimize_result(
      session, step_b, step_b_size, result_bytes, sizeof(result_bytes),
      &result_size);
  assert_int_equal(optimize_bohr.ok, 1);
  assert_int_equal(result_size, expected_optimize_size);
  assert_int_equal(g_optimize_calls, 2);
  assert_close(g_optimize_positions_ang[1][5], 1.058354421806, 1.0e-12);
  assert_potential_result_optimized(result_bytes, result_size, -1.75,
                                    expected_optimized_bohr, 6, 1.0e-12);

  size_t changed_optimize_size = 0;
  NWChemCResult changed_optimize =
      nwchemc_session_calculate_optimize_result(
          session, step_changed_species, step_changed_species_size,
          result_bytes, sizeof(result_bytes), &changed_optimize_size);
  assert_int_equal(changed_optimize.ok, 0);
  assert_non_null(strstr(changed_optimize.message, "topology"));
  assert_int_equal(changed_optimize_size, expected_optimize_size);
  assert_int_equal(g_optimize_calls, 2);

  size_t expected_frequencies_size =
      nwchemc_frequencies_result_size_for_force_input(step_a, step_a_size);
  assert_true(expected_frequencies_size > 0);
  required_size = 0;
  NWChemCResult short_frequencies =
      nwchemc_session_calculate_frequencies_result(
          session, step_a, step_a_size, short_result, sizeof(short_result),
          &required_size);
  assert_int_equal(short_frequencies.ok, 0);
  assert_int_equal(required_size, expected_frequencies_size);
  assert_int_equal(g_frequency_calls, 0);

  result_size = 0;
  NWChemCResult frequencies_result =
      nwchemc_session_calculate_frequencies_result(
          session, step_a, step_a_size, result_bytes, sizeof(result_bytes),
          &result_size);
  assert_int_equal(frequencies_result.ok, 1);
  assert_int_equal(result_size, expected_frequencies_size);
  assert_int_equal(g_frequency_calls, 1);
  assert_int_equal(g_frequency_cell_calls, 1);
  const double expected_frequencies[6] = {100.0, 101.0, 102.0,
                                          103.0, 104.0, 105.0};
  const double expected_intensities[6] = {0.01, 0.02, 0.03,
                                          0.04, 0.05, 0.06};
  assert_potential_result_frequencies(result_bytes, result_size,
                                      expected_frequencies,
                                      expected_intensities, 6, 1.0e-12);

  size_t changed_frequencies_size = 0;
  NWChemCResult changed_frequencies =
      nwchemc_session_calculate_frequencies_result(
          session, step_changed_species, step_changed_species_size,
          result_bytes, sizeof(result_bytes), &changed_frequencies_size);
  assert_int_equal(changed_frequencies.ok, 0);
  assert_non_null(strstr(changed_frequencies.message, "topology"));
  assert_int_equal(changed_frequencies_size, expected_frequencies_size);
  assert_int_equal(g_frequency_calls, 1);

  nwchemc_session_destroy(session);
  free(step_changed_species);
  free(step_b);
  free(step_a);
  free(message);
}

static void test_calculate_result_one_shot_writes_potential_result(
    void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  size_t step_a_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  assert_non_null(message);
  assert_non_null(step_a);

  unsigned char result_bytes[256];
  size_t result_size = 0;
  size_t expected_size =
      nwchemc_potential_result_size_for_force_input(step_a, step_a_size);
  assert_true(expected_size > 0);

  NWChemCResult one_shot = nwchemc_calculate_result(
      message, message_size, step_a, step_a_size, result_bytes,
      sizeof(result_bytes), &result_size);
  assert_int_equal(one_shot.ok, 1);
  assert_close(one_shot.energy_h, -1.0, 1.0e-12);
  assert_int_equal(result_size, expected_size);
  const double native_forces[6] = {-1.0, -2.0, -3.0, -4.0, -5.0, -6.0};
  const double bohr_to_angstrom = 0.529177210903;
  double hartree_angstrom_forces[6];
  for (int i = 0; i < 6; ++i)
    hartree_angstrom_forces[i] = native_forces[i] / bohr_to_angstrom;
  assert_potential_result(result_bytes, result_size, -1.0,
                          hartree_angstrom_forces, 6, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_energy_grad_calls, 1);

  reset_embed_captures();
  unsigned char short_result[79];
  size_t required_size = 0;
  NWChemCResult short_output = nwchemc_calculate_result(
      message, message_size, step_a, step_a_size, short_result,
      sizeof(short_result), &required_size);
  assert_int_equal(short_output.ok, 0);
  assert_int_equal(required_size, expected_size);
  assert_int_equal(g_set_config_calls, 0);
  assert_int_equal(g_energy_grad_calls, 0);

  free(step_a);
  free(message);
}

static void test_calculate_hessian_result_one_shot_writes_potential_result(
    void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_calculate_hessian_result != NULL);
  size_t message_size = 0;
  size_t step_a_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  assert_non_null(message);
  assert_non_null(step_a);

  unsigned char result_bytes[512];
  size_t result_size = 0;
  size_t expected_size =
      nwchemc_hessian_result_size_for_force_input(step_a, step_a_size);
  assert_true(expected_size > 0);
  NWChemCResult result = nwchemc_calculate_hessian_result(
      message, message_size, step_a, step_a_size, result_bytes,
      sizeof(result_bytes), &result_size);
  assert_int_equal(result.ok, 1);
  assert_int_equal(result_size, expected_size);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_hessian_calls, 1);
  assert_int_equal(g_hessian_cell_calls, 1);
  double expected_hessian[36];
  const double bohr_to_angstrom = 0.529177210903;
  for (int i = 0; i < 36; ++i)
    expected_hessian[i] =
        (double)(i + 10) / (bohr_to_angstrom * bohr_to_angstrom);
  assert_potential_result_hessian(result_bytes, result_size, expected_hessian,
                                  36, 1.0e-10);

  reset_embed_captures();
  unsigned char short_result[127];
  size_t required_size = 0;
  NWChemCResult short_output = nwchemc_calculate_hessian_result(
      message, message_size, step_a, step_a_size, short_result,
      sizeof(short_result), &required_size);
  assert_int_equal(short_output.ok, 0);
  assert_int_equal(required_size, expected_size);
  assert_int_equal(g_set_config_calls, 0);
  assert_int_equal(g_hessian_calls, 0);

  free(step_a);
  free(message);
}

static void test_calculate_property_results_one_shot_write_potential_result(
    void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_calculate_dipole_result != NULL);
  assert_true(nwchemc_calculate_quadrupole_result != NULL);
  size_t message_size = 0;
  size_t step_a_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  assert_non_null(message);
  assert_non_null(step_a);

  unsigned char result_bytes[256];
  size_t result_size = 0;
  size_t expected_dipole_size =
      nwchemc_dipole_result_size_for_force_input(step_a, step_a_size);
  NWChemCResult dipole_result = nwchemc_calculate_dipole_result(
      message, message_size, step_a, step_a_size, result_bytes,
      sizeof(result_bytes), &result_size);
  assert_int_equal(dipole_result.ok, 1);
  assert_int_equal(result_size, expected_dipole_size);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_dipole_calls, 1);
  const double expected_dipole[3] = {0.25, 0.5, 0.75};
  assert_potential_result_dipole(result_bytes, result_size, expected_dipole,
                                 1.0e-12);

  reset_embed_captures();
  unsigned char short_result[63];
  size_t required_size = 0;
  NWChemCResult short_dipole = nwchemc_calculate_dipole_result(
      message, message_size, step_a, step_a_size, short_result,
      sizeof(short_result), &required_size);
  assert_int_equal(short_dipole.ok, 0);
  assert_int_equal(required_size, expected_dipole_size);
  assert_int_equal(g_set_config_calls, 0);
  assert_int_equal(g_dipole_calls, 0);

  reset_embed_captures();
  size_t expected_quadrupole_size =
      nwchemc_quadrupole_result_size_for_force_input(step_a, step_a_size);
  NWChemCResult quadrupole_result = nwchemc_calculate_quadrupole_result(
      message, message_size, step_a, step_a_size, result_bytes,
      sizeof(result_bytes), &result_size);
  assert_int_equal(quadrupole_result.ok, 1);
  assert_int_equal(result_size, expected_quadrupole_size);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_quadrupole_calls, 1);
  const double expected_quadrupole[6] = {0.125, 0.25, 0.375,
                                         0.5,   0.625, 0.75};
  assert_potential_result_quadrupole(result_bytes, result_size,
                                     expected_quadrupole, 1.0e-12);

  reset_embed_captures();
  required_size = 0;
  NWChemCResult short_quadrupole = nwchemc_calculate_quadrupole_result(
      message, message_size, step_a, step_a_size, short_result,
      sizeof(short_result), &required_size);
  assert_int_equal(short_quadrupole.ok, 0);
  assert_int_equal(required_size, expected_quadrupole_size);
  assert_int_equal(g_set_config_calls, 0);
  assert_int_equal(g_quadrupole_calls, 0);

  free(step_a);
  free(message);
}

static void test_calculate_structural_results_one_shot_write_potential_result(
    void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_calculate_optimize_result != NULL);
  assert_true(nwchemc_calculate_frequencies_result != NULL);
  size_t message_size = 0;
  size_t step_a_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  assert_non_null(message);
  assert_non_null(step_a);

  unsigned char result_bytes[512];
  unsigned char short_result[63];
  size_t result_size = 0;
  size_t required_size = 0;
  size_t expected_optimize_size =
      nwchemc_optimize_result_size_for_force_input(step_a, step_a_size);
  assert_true(expected_optimize_size > 0);

  NWChemCResult short_optimize = nwchemc_calculate_optimize_result(
      message, message_size, step_a, step_a_size, short_result,
      sizeof(short_result), &required_size);
  assert_int_equal(short_optimize.ok, 0);
  assert_int_equal(required_size, expected_optimize_size);
  assert_int_equal(g_set_config_calls, 0);
  assert_int_equal(g_optimize_calls, 0);

  NWChemCResult optimize_result = nwchemc_calculate_optimize_result(
      message, message_size, step_a, step_a_size, result_bytes,
      sizeof(result_bytes), &result_size);
  assert_int_equal(optimize_result.ok, 1);
  assert_close(optimize_result.energy_h, -1.75, 1.0e-12);
  assert_int_equal(result_size, expected_optimize_size);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_optimize_calls, 1);
  const double expected_optimized_ang[6] = {0.01, 0.02, 0.03,
                                            0.04, 0.05, 0.8014};
  assert_potential_result_optimized(result_bytes, result_size, -1.75,
                                    expected_optimized_ang, 6, 1.0e-12);

  reset_embed_captures();
  size_t expected_frequencies_size =
      nwchemc_frequencies_result_size_for_force_input(step_a, step_a_size);
  assert_true(expected_frequencies_size > 0);
  required_size = 0;
  NWChemCResult short_frequencies = nwchemc_calculate_frequencies_result(
      message, message_size, step_a, step_a_size, short_result,
      sizeof(short_result), &required_size);
  assert_int_equal(short_frequencies.ok, 0);
  assert_int_equal(required_size, expected_frequencies_size);
  assert_int_equal(g_set_config_calls, 0);
  assert_int_equal(g_frequency_calls, 0);

  result_size = 0;
  NWChemCResult frequencies_result = nwchemc_calculate_frequencies_result(
      message, message_size, step_a, step_a_size, result_bytes,
      sizeof(result_bytes), &result_size);
  assert_int_equal(frequencies_result.ok, 1);
  assert_int_equal(result_size, expected_frequencies_size);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_frequency_calls, 1);
  const double expected_frequencies[6] = {100.0, 101.0, 102.0,
                                          103.0, 104.0, 105.0};
  const double expected_intensities[6] = {0.01, 0.02, 0.03,
                                          0.04, 0.05, 0.06};
  assert_potential_result_frequencies(result_bytes, result_size,
                                      expected_frequencies,
                                      expected_intensities, 6, 1.0e-12);

  free(step_a);
  free(message);
}

static void test_calculate_hessian_and_dipole_one_shot_accept_force_input(
    void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_calculate_hessian != NULL);
  assert_true(nwchemc_calculate_dipole != NULL);
  assert_true(nwchemc_calculate_quadrupole != NULL);
  assert_true(nwchemc_calculate_optimize != NULL);
  size_t message_size = 0;
  size_t step_a_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  assert_non_null(message);
  assert_non_null(step_a);

  double hessian[36] = {0.0};
  NWChemCResult hessian_result = nwchemc_calculate_hessian(
      message, message_size, step_a, step_a_size, hessian, 36);
  assert_int_equal(hessian_result.ok, 1);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_hessian_calls, 1);
  assert_int_equal(g_hessian_cell_calls, 1);
  assert_int_equal(g_hessian_n_atoms[0], 2);
  assert_int_equal(g_hessian_atomic_numbers[0][0], 1);
  assert_int_equal(g_hessian_atomic_numbers[0][1], 8);
  assert_close(g_hessian_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_hessian_has_cell[0], 1);
  assert_close(hessian[0], 10.0, 1.0e-12);
  assert_close(hessian[35], 45.0, 1.0e-12);

  reset_embed_captures();
  NWChemCResult short_hessian = nwchemc_calculate_hessian(
      message, message_size, step_a, step_a_size, hessian, 35);
  assert_int_equal(short_hessian.ok, 0);
  assert_int_equal(g_set_config_calls, 0);
  assert_int_equal(g_hessian_calls, 0);

  reset_embed_captures();
  double dipole[3] = {0.0, 0.0, 0.0};
  NWChemCResult dipole_result = nwchemc_calculate_dipole(
      message, message_size, step_a, step_a_size, dipole, 3);
  assert_int_equal(dipole_result.ok, 1);
  assert_close(dipole_result.energy_h, -1.25, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_dipole_calls, 1);
  assert_int_equal(g_dipole_cell_calls, 1);
  assert_int_equal(g_dipole_n_atoms[0], 2);
  assert_int_equal(g_dipole_atomic_numbers[0][0], 1);
  assert_int_equal(g_dipole_atomic_numbers[0][1], 8);
  assert_close(g_dipole_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_dipole_has_cell[0], 1);
  assert_close(dipole[0], 0.25, 1.0e-12);
  assert_close(dipole[1], 0.5, 1.0e-12);
  assert_close(dipole[2], 0.75, 1.0e-12);

  reset_embed_captures();
  NWChemCResult short_dipole = nwchemc_calculate_dipole(
      message, message_size, step_a, step_a_size, dipole, 2);
  assert_int_equal(short_dipole.ok, 0);
  assert_int_equal(g_set_config_calls, 0);
  assert_int_equal(g_dipole_calls, 0);

  reset_embed_captures();
  double quadrupole[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult quadrupole_result = nwchemc_calculate_quadrupole(
      message, message_size, step_a, step_a_size, quadrupole, 6);
  assert_int_equal(quadrupole_result.ok, 1);
  assert_close(quadrupole_result.energy_h, -1.5, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_quadrupole_calls, 1);
  assert_int_equal(g_quadrupole_cell_calls, 1);
  assert_int_equal(g_quadrupole_n_atoms[0], 2);
  assert_int_equal(g_quadrupole_atomic_numbers[0][0], 1);
  assert_int_equal(g_quadrupole_atomic_numbers[0][1], 8);
  assert_close(g_quadrupole_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_quadrupole_has_cell[0], 1);
  assert_close(quadrupole[0], 0.125, 1.0e-12);
  assert_close(quadrupole[5], 0.75, 1.0e-12);

  reset_embed_captures();
  NWChemCResult short_quadrupole = nwchemc_calculate_quadrupole(
      message, message_size, step_a, step_a_size, quadrupole, 5);
  assert_int_equal(short_quadrupole.ok, 0);
  assert_int_equal(g_set_config_calls, 0);
  assert_int_equal(g_quadrupole_calls, 0);

  reset_embed_captures();
  double optimized_positions[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult optimize_result = nwchemc_calculate_optimize(
      message, message_size, step_a, step_a_size, optimized_positions, 6);
  assert_int_equal(optimize_result.ok, 1);
  assert_close(optimize_result.energy_h, -1.75, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_optimize_calls, 1);
  assert_int_equal(g_optimize_cell_calls, 1);
  assert_int_equal(g_optimize_n_atoms[0], 2);
  assert_int_equal(g_optimize_atomic_numbers[0][0], 1);
  assert_int_equal(g_optimize_atomic_numbers[0][1], 8);
  assert_close(g_optimize_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_optimize_has_cell[0], 1);
  assert_close(optimized_positions[0], 0.01, 1.0e-12);
  assert_close(optimized_positions[5], 0.8014, 1.0e-12);

  reset_embed_captures();
  NWChemCResult short_optimize = nwchemc_calculate_optimize(
      message, message_size, step_a, step_a_size, optimized_positions, 5);
  assert_int_equal(short_optimize.ok, 0);
  assert_int_equal(g_set_config_calls, 0);
  assert_int_equal(g_optimize_calls, 0);

  free(step_a);
  free(message);
}

static void test_calculate_frequencies_one_shot_accepts_force_input(
    void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_calculate_frequencies != NULL);
  size_t message_size = 0;
  size_t step_a_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  assert_non_null(message);
  assert_non_null(step_a);

  double frequencies[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  double intensities[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult frequency_result = nwchemc_calculate_frequencies(
      message, message_size, step_a, step_a_size, frequencies, 6, intensities,
      6);
  assert_int_equal(frequency_result.ok, 1);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_frequency_calls, 1);
  assert_int_equal(g_frequency_cell_calls, 1);
  assert_int_equal(g_frequency_n_atoms[0], 2);
  assert_int_equal(g_frequency_atomic_numbers[0][0], 1);
  assert_int_equal(g_frequency_atomic_numbers[0][1], 8);
  assert_close(g_frequency_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_frequency_has_cell[0], 1);
  assert_close(frequencies[0], 100.0, 1.0e-12);
  assert_close(frequencies[5], 105.0, 1.0e-12);
  assert_close(intensities[0], 0.01, 1.0e-12);
  assert_close(intensities[5], 0.06, 1.0e-12);

  reset_embed_captures();
  NWChemCResult short_frequency = nwchemc_calculate_frequencies(
      message, message_size, step_a, step_a_size, frequencies, 5, intensities,
      6);
  assert_int_equal(short_frequency.ok, 0);
  assert_int_equal(g_set_config_calls, 0);
  assert_int_equal(g_frequency_calls, 0);

  reset_embed_captures();
  NWChemCResult short_intensity = nwchemc_calculate_frequencies(
      message, message_size, step_a, step_a_size, frequencies, 6, intensities,
      5);
  assert_int_equal(short_intensity.ok, 0);
  assert_int_equal(g_set_config_calls, 0);
  assert_int_equal(g_frequency_calls, 0);

  free(step_a);
  free(message);
}

int main(int argc, char **argv) {
  if (argc != 12) {
    fprintf(stderr,
            "usage: %s PARAMS_BIN CONFIG_OPTIONS_BIN PSPSPIN_PARAMS_BIN "
            "PSPSPIN_MANY_PARAMS_BIN FORCE_STEP_A_BIN FORCE_STEP_B_BIN "
            "FORCE_STEP_EV_BIN FORCE_STEP_CHANGED_SPECIES_BIN "
            "FORCE_STEP_STATE_BIN TCE_METHODS_BIN COMPACT_CELLS_BIN\n",
            argv[0]);
    return 2;
  }
  g_params_path = argv[1];
  g_config_options_path = argv[2];
  g_pspspin_path = argv[3];
  g_pspspin_many_path = argv[4];
  g_force_step_a_path = argv[5];
  g_force_step_b_path = argv[6];
  g_force_step_ev_path = argv[7];
  g_force_step_changed_species_path = argv[8];
  g_force_step_state_path = argv[9];
  g_tce_methods_path = argv[10];
  g_compact_cells_path = argv[11];
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_embed_config_uses_direct_dft_values),
      cmocka_unit_test(test_embed_config_promotes_compact_simulation_cells),
      cmocka_unit_test(test_embed_config_promotes_tce_method_tokens),
      cmocka_unit_test(test_embed_config_uses_direct_scf_values),
      cmocka_unit_test(test_embed_config_promotes_pspspin_rules),
      cmocka_unit_test(test_embed_config_promotes_large_pspspin_ion_list),
      cmocka_unit_test(test_session_reuses_config_across_geometry_steps),
      cmocka_unit_test(test_session_reapplies_after_one_shot_config),
      cmocka_unit_test(test_session_rejects_param_replacement_after_topology),
      cmocka_unit_test(test_session_calculate_forces_accepts_force_input_steps),
      cmocka_unit_test(test_session_calculate_hessian_accepts_force_input_step),
      cmocka_unit_test(test_session_calculate_dipole_accepts_force_input_step),
      cmocka_unit_test(
          test_session_calculate_quadrupole_accepts_force_input_step),
      cmocka_unit_test(test_session_calculate_optimize_accepts_force_input_step),
      cmocka_unit_test(
          test_session_calculate_frequencies_accepts_force_input_step),
      cmocka_unit_test(test_session_force_input_state_overrides_params),
      cmocka_unit_test(test_session_calculate_result_writes_potential_result),
      cmocka_unit_test(
          test_session_calculate_hessian_result_writes_potential_result),
      cmocka_unit_test(
          test_session_calculate_property_results_write_potential_result),
      cmocka_unit_test(
          test_session_calculate_structural_results_write_potential_result),
      cmocka_unit_test(test_calculate_result_one_shot_writes_potential_result),
      cmocka_unit_test(
          test_calculate_hessian_result_one_shot_writes_potential_result),
      cmocka_unit_test(
          test_calculate_property_results_one_shot_write_potential_result),
      cmocka_unit_test(
          test_calculate_structural_results_one_shot_write_potential_result),
      cmocka_unit_test(
          test_calculate_hessian_and_dipole_one_shot_accept_force_input),
      cmocka_unit_test(test_calculate_frequencies_one_shot_accepts_force_input),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
