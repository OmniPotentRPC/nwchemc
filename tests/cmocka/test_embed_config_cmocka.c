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
static const char *g_force_step_a_path = NULL;
static const char *g_force_step_b_path = NULL;

static char g_basis[64];
static char g_theory[64];
static char g_scf_type[64];
static char g_dft_xc[64];
static char g_input_blocks[8192];
static char g_psp_elements[8][17];
static char g_psp_names[8][257];
static int g_psp_types[8];
static int g_psp_count = 0;
static int g_set_config_calls = 0;
static int g_set_dft_direct_calls = 0;
static int g_set_pseudopotential_calls = 0;
static int g_dft_direct_enabled = 0;
static int g_dft_smearing_enabled = 0;
static double g_dft_smear_sigma_hartree = 0.0;
static int g_dft_smearing_spinset = 0;
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
    g_psp_types[i] = -1;
  }
  g_psp_count = 0;
  g_set_config_calls = 0;
  g_set_dft_direct_calls = 0;
  g_set_pseudopotential_calls = 0;
  g_dft_direct_enabled = 0;
  g_dft_smearing_enabled = 0;
  g_dft_smear_sigma_hartree = 0.0;
  g_dft_smearing_spinset = 0;
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
  assert_non_null(strstr(g_input_blocks, "pspspin off"));
  assert_non_null(strstr(g_input_blocks, "iterations 40"));
  assert_non_null(strstr(g_input_blocks, "set int:acc_std 1e-8"));
  assert_int_equal(g_set_pseudopotential_calls, 1);
  assert_int_equal(g_psp_count, 5);
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
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  unsigned char *step_b = read_file(g_force_step_b_path, &step_b_size);
  assert_non_null(message);
  assert_non_null(step_a);
  assert_non_null(step_b);

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

  NWChemCResult short_output = nwchemc_session_calculate_forces(
      session, step_b, step_b_size, forces, 5);
  assert_int_equal(short_output.ok, 0);
  assert_int_equal(g_energy_grad_calls, 2);

  nwchemc_session_destroy(session);
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
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  assert_non_null(message);
  assert_non_null(step_a);

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

  NWChemCResult short_output = nwchemc_session_calculate_hessian(
      session, step_a, step_a_size, hessian, 35);
  assert_int_equal(short_output.ok, 0);
  assert_int_equal(g_hessian_calls, 1);

  nwchemc_session_destroy(session);
  free(step_a);
  free(message);
}

int main(int argc, char **argv) {
  if (argc != 4) {
    fprintf(stderr, "usage: %s PARAMS_BIN FORCE_STEP_A_BIN FORCE_STEP_B_BIN\n",
            argv[0]);
    return 2;
  }
  g_params_path = argv[1];
  g_force_step_a_path = argv[2];
  g_force_step_b_path = argv[3];
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_embed_config_uses_direct_dft_values),
      cmocka_unit_test(test_session_reuses_config_across_geometry_steps),
      cmocka_unit_test(test_session_reapplies_after_one_shot_config),
      cmocka_unit_test(test_session_calculate_forces_accepts_force_input_steps),
      cmocka_unit_test(test_session_calculate_hessian_accepts_force_input_step),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
