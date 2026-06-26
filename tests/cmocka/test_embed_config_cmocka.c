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
static const char *g_force_step_a_path = NULL;
static const char *g_force_step_b_path = NULL;
static const char *g_force_step_ev_path = NULL;
static const char *g_force_step_changed_species_path = NULL;

static char g_basis[64];
static char g_theory[64];
static char g_scf_type[64];
static char g_dft_xc[64];
static char g_input_blocks[8192];
static char g_psp_elements[8][17];
static char g_psp_names[8][257];
static char g_set_keys[8][129];
static char g_set_values[8][257];
static char g_typed_set_keys[8][129];
static char g_typed_set_values[8][4][257];
static int g_psp_types[8];
static int g_typed_set_types[8];
static int g_typed_set_value_counts[8];
static int g_psp_count = 0;
static int g_set_string_count = 0;
static int g_typed_set_count = 0;
static int g_set_config_calls = 0;
static int g_set_dft_direct_calls = 0;
static int g_set_scf_direct_calls = 0;
static int g_set_driver_direct_calls = 0;
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
static int g_energy_grad_calls = 0;
static int g_hessian_calls = 0;
static int g_hessian_cell_calls = 0;
static int g_call_n_atoms[8];
static int g_call_has_cell[8];
static int g_call_atomic_numbers[8][8];
static double g_call_positions_ang[8][24];
static double g_call_cell_ang[8][9];
static int g_hessian_n_atoms[8];
static int g_hessian_has_cell[8];
static int g_hessian_atomic_numbers[8][8];
static double g_hessian_positions_ang[8][24];
static double g_hessian_cell_ang[8][9];

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
  if (count > 8)
    count = 8;
  for (int i = 0; i < count; ++i) {
    copy_span(g_typed_set_keys[i], sizeof(g_typed_set_keys[i]),
              keys + i * 128, 128);
    g_typed_set_types[i] = value_types[i];
    g_typed_set_value_counts[i] = value_counts[i];
    int nvalues = value_counts[i] < 4 ? value_counts[i] : 4;
    for (int j = 0; j < nvalues; ++j) {
      copy_span(g_typed_set_values[i][j], sizeof(g_typed_set_values[i][j]),
                values + (i * 16 + j) * 256, 256);
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
  (void)charge;
  (void)multiplicity;
  if (call < 8) {
    int ncopy = *n_atoms < 8 ? *n_atoms : 8;
    int ncoord = (*n_atoms) * 3 < 24 ? (*n_atoms) * 3 : 24;
    g_call_n_atoms[call] = *n_atoms;
    g_call_has_cell[call] = has_cell ? *has_cell : 0;
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
  (void)charge;
  (void)multiplicity;
  if (call < 8) {
    int ncopy = *n_atoms < 8 ? *n_atoms : 8;
    int ncoord = (*n_atoms) * 3 < 24 ? (*n_atoms) * 3 : 24;
    g_hessian_n_atoms[call] = *n_atoms;
    g_hessian_has_cell[call] = has_cell ? *has_cell : 0;
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
    g_typed_set_keys[i][0] = '\0';
    for (int j = 0; j < 4; ++j)
      g_typed_set_values[i][j][0] = '\0';
    g_psp_types[i] = -1;
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
  g_energy_grad_calls = 0;
  g_hessian_calls = 0;
  g_hessian_cell_calls = 0;
  memset(g_call_n_atoms, 0, sizeof(g_call_n_atoms));
  memset(g_call_has_cell, 0, sizeof(g_call_has_cell));
  memset(g_call_atomic_numbers, 0, sizeof(g_call_atomic_numbers));
  memset(g_call_positions_ang, 0, sizeof(g_call_positions_ang));
  memset(g_call_cell_ang, 0, sizeof(g_call_cell_ang));
  memset(g_hessian_n_atoms, 0, sizeof(g_hessian_n_atoms));
  memset(g_hessian_has_cell, 0, sizeof(g_hessian_has_cell));
  memset(g_hessian_atomic_numbers, 0, sizeof(g_hessian_atomic_numbers));
  memset(g_hessian_positions_ang, 0, sizeof(g_hessian_positions_ang));
  memset(g_hessian_cell_ang, 0, sizeof(g_hessian_cell_ang));
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
  assert_non_null(strstr(g_input_blocks, "pspspin off"));
  assert_non_null(strstr(g_input_blocks, "iterations 40"));
  assert_non_null(strstr(g_input_blocks, "set int:acc_std 1e-8"));
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
  assert_null(strstr(g_input_blocks, "set dft:grid xfine"));
  assert_null(strstr(g_input_blocks, "set dft:nopen integer 2"));
  assert_null(strstr(g_input_blocks, "set dft:smear_sigma double 0.0015"));
  assert_null(strstr(g_input_blocks, "set dft:spinset logical false"));

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

int main(int argc, char **argv) {
  if (argc != 7) {
    fprintf(stderr,
            "usage: %s PARAMS_BIN CONFIG_OPTIONS_BIN FORCE_STEP_A_BIN FORCE_STEP_B_BIN "
            "FORCE_STEP_EV_BIN FORCE_STEP_CHANGED_SPECIES_BIN\n",
            argv[0]);
    return 2;
  }
  g_params_path = argv[1];
  g_config_options_path = argv[2];
  g_force_step_a_path = argv[3];
  g_force_step_b_path = argv[4];
  g_force_step_ev_path = argv[5];
  g_force_step_changed_species_path = argv[6];
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_embed_config_uses_direct_dft_values),
      cmocka_unit_test(test_embed_config_uses_direct_scf_values),
      cmocka_unit_test(test_session_reuses_config_across_geometry_steps),
      cmocka_unit_test(test_session_reapplies_after_one_shot_config),
      cmocka_unit_test(test_session_calculate_forces_accepts_force_input_steps),
      cmocka_unit_test(test_session_calculate_hessian_accepts_force_input_step),
      cmocka_unit_test(test_session_calculate_result_writes_potential_result),
      cmocka_unit_test(test_calculate_result_one_shot_writes_potential_result),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
