#include "nwchemc.h"

#include <cmocka.h>
#include <errno.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *g_params_path = NULL;

static char g_basis[64];
static char g_theory[64];
static char g_scf_type[64];
static char g_dft_xc[64];
static char g_input_blocks[8192];
static int g_set_config_calls = 0;
static int g_set_dft_direct_calls = 0;
static int g_dft_direct_enabled = 0;
static int g_dft_smearing_enabled = 0;
static double g_dft_smear_sigma_hartree = 0.0;
static int g_dft_smearing_spinset = 0;
static int g_energy_grad_calls = 0;

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

int nwchemc_embed_energy_grad(const int *n_atoms, const double *positions_ang,
                              const int *atomic_numbers, const int *charge,
                              const int *multiplicity, double *energy_h,
                              double *grad_h_bohr, char *errmsg,
                              int errmsg_len) {
  ++g_energy_grad_calls;
  (void)positions_ang;
  (void)atomic_numbers;
  (void)charge;
  (void)multiplicity;
  *energy_h = -1.0;
  for (int i = 0; i < (*n_atoms) * 3; ++i)
    grad_h_bohr[i] = 0.0;
  snprintf(errmsg, (size_t)errmsg_len, "ok");
  return 0;
}

int nwchemc_embed_hessian(const int *n_atoms, const double *positions_ang,
                          const int *atomic_numbers, const int *charge,
                          const int *multiplicity, double *hessian_h_bohr2,
                          char *errmsg, int errmsg_len) {
  (void)n_atoms;
  (void)positions_ang;
  (void)atomic_numbers;
  (void)charge;
  (void)multiplicity;
  (void)hessian_h_bohr2;
  snprintf(errmsg, (size_t)errmsg_len, "not exercised");
  return -1;
}

void nwchemc_embed_finalize(void) {}

static void reset_embed_captures(void) {
  g_basis[0] = '\0';
  g_theory[0] = '\0';
  g_scf_type[0] = '\0';
  g_dft_xc[0] = '\0';
  g_input_blocks[0] = '\0';
  g_set_config_calls = 0;
  g_set_dft_direct_calls = 0;
  g_dft_direct_enabled = 0;
  g_dft_smearing_enabled = 0;
  g_dft_smear_sigma_hartree = 0.0;
  g_dft_smearing_spinset = 0;
  g_energy_grad_calls = 0;
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
  assert_non_null(strstr(g_input_blocks, "iterations 40"));
  assert_non_null(strstr(g_input_blocks, "set int:acc_std 1e-8"));

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

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s PARAMS_BIN\n", argv[0]);
    return 2;
  }
  g_params_path = argv[1];
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_embed_config_uses_direct_dft_values),
      cmocka_unit_test(test_session_reuses_config_across_geometry_steps),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
