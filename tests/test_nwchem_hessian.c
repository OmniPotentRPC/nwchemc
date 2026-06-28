#include "nwchemc.h"
#include "Potentials.capnp.h"

#include <errno.h>
#include <math.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cmocka.h>

static const char *g_params_path = NULL;
static const char *g_config_path = NULL;
static const char *g_force_input_path = NULL;

static const double BOHR_TO_ANGSTROM = 0.529177210903;

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

static void call_gradient(const double *positions_ang,
                          const int *atomic_numbers,
                          const unsigned char *params, size_t params_size,
                          double *grad) {
  NWChemCResult result = nwchemc_energy_gradient(
      2, positions_ang, atomic_numbers, params, params_size, grad);
  if (!result.ok)
    fail_msg("nwchemc_energy_gradient failed: %s", result.message);
}

static void assert_close_relative(const char *label, int index, double actual,
                                  double expected, double tolerance) {
  double scale = fmax(1.0, fmax(fabs(actual), fabs(expected)));
  if (fabs(actual - expected) > tolerance * scale)
    fail_msg("%s[%d] mismatch: got %.17g expected %.17g", label, index,
             actual, expected);
}

static void assert_success(const char *label, NWChemCResult result) {
  if (!result.ok)
    fail_msg("%s failed: %s", label, result.message);
  assert_true(isfinite(result.energy_h));
}

static void assert_matching_hessian(const char *label, const double *actual,
                                    const double *expected, int ncoord,
                                    double scale_factor) {
  for (int i = 0; i < ncoord * ncoord; ++i)
    assert_close_relative(label, i, actual[i], expected[i] * scale_factor,
                          1.0e-5);
}

static void assert_potential_result_hessian(const unsigned char *message,
                                            size_t message_size,
                                            const double *native_hessian,
                                            int ncoord) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);

  struct PotentialResult result;
  read_PotentialResult(&result, root);
  assert_true(isfinite(result.energy));

  capn_resolve(&result.hessian.p);
  assert_int_equal(result.hessian.p.type, CAPN_LIST);
  assert_int_equal(result.hessian.p.datasz, 8);
  assert_int_equal(result.hessian.p.len, ncoord * ncoord);

  double conversion = 1.0 / (BOHR_TO_ANGSTROM * BOHR_TO_ANGSTROM);
  for (int i = 0; i < result.hessian.p.len; ++i) {
    double actual = capn_to_f64(capn_get64(result.hessian, i));
    if (!isfinite(actual))
      fail_msg("non-finite PotentialResult.hessian[%d]", i);
    assert_close_relative("PotentialResult.hessian", i, actual,
                          native_hessian[i] * conversion, 1.0e-5);
  }

  capn_free(&arena);
}

static int teardown_nwchem(void **state) {
  (void)state;
  nwchemc_finalize();
  return 0;
}

static void test_h2_hessian(void **state) {
  (void)state;
  assert_true(nwchemc_available());
  size_t params_size = 0;
  size_t config_size = 0;
  size_t force_input_size = 0;
  unsigned char *params = read_file(g_params_path, &params_size);
  unsigned char *config = read_file(g_config_path, &config_size);
  unsigned char *force_input = read_file(g_force_input_path, &force_input_size);
  assert_non_null(params);
  assert_non_null(config);
  assert_non_null(force_input);

  const int n_atoms = 2;
  const int ncoord = 3 * n_atoms;
  const int atomic_numbers[2] = {1, 1};
  const double base_positions[6] = {0.0, 0.0, -0.3707,
                                    0.0, 0.0, 0.3707};
  double hessian[36];
  memset(hessian, 0, sizeof(hessian));

  NWChemCResult result = nwchemc_hessian(
      n_atoms, base_positions, atomic_numbers, params, params_size, hessian);
  assert_success("nwchemc_hessian", result);

  double max_abs = 0.0;
  for (int i = 0; i < ncoord; ++i) {
    for (int j = 0; j < ncoord; ++j) {
      double hij = hessian[i * ncoord + j];
      double hji = hessian[j * ncoord + i];
      if (!isfinite(hij))
        fail_msg("non-finite hessian[%d,%d]", i, j);
      double scale = fmax(1.0, fmax(fabs(hij), fabs(hji)));
      if (fabs(hij - hji) > 1e-7 * scale)
        fail_msg("hessian symmetry mismatch at %d,%d: %.17g %.17g", i, j,
                 hij, hji);
      max_abs = fmax(max_abs, fabs(hij));
    }
  }
  assert_true(max_abs >= 1e-6);

  const double bohr_to_ang = 0.529177210903;
  const double delta_bohr = 1.0e-3;
  const int displaced_coord = 2;
  double plus_positions[6];
  double minus_positions[6];
  memcpy(plus_positions, base_positions, sizeof(base_positions));
  memcpy(minus_positions, base_positions, sizeof(base_positions));
  plus_positions[displaced_coord] += delta_bohr * bohr_to_ang;
  minus_positions[displaced_coord] -= delta_bohr * bohr_to_ang;

  double grad_plus[6];
  double grad_minus[6];
  call_gradient(plus_positions, atomic_numbers, params, params_size, grad_plus);
  call_gradient(minus_positions, atomic_numbers, params, params_size,
                grad_minus);

  for (int i = 0; i < ncoord; ++i) {
    double finite_diff = (grad_plus[i] - grad_minus[i]) / (2.0 * delta_bohr);
    double analytic = hessian[i * ncoord + displaced_coord];
    double scale = fmax(1.0, fmax(fabs(finite_diff), fabs(analytic)));
    if (fabs(finite_diff - analytic) > 2.0e-2 * scale)
      fail_msg("hessian/gradient mismatch row %d: h=%.17g fd=%.17g", i,
               analytic, finite_diff);
  }

  double force_input_hessian[36];
  memset(force_input_hessian, 0, sizeof(force_input_hessian));
  NWChemCResult force_input_status = nwchemc_calculate_hessian(
      params, params_size, force_input, force_input_size, force_input_hessian,
      36);
  assert_success("nwchemc_calculate_hessian", force_input_status);
  assert_matching_hessian("ForceInput hessian", force_input_hessian, hessian,
                          ncoord, 1.0);

  double config_hessian[36];
  memset(config_hessian, 0, sizeof(config_hessian));
  NWChemCResult config_status = nwchemc_calculate_hessian_from_config(
      config, config_size, force_input, force_input_size, config_hessian, 36);
  assert_success("nwchemc_calculate_hessian_from_config", config_status);
  assert_matching_hessian("PotentialConfig hessian", config_hessian, hessian,
                          ncoord, 1.0);

  size_t hessian_capacity =
      nwchemc_hessian_result_size_for_force_input(force_input,
                                                  force_input_size);
  assert_true(hessian_capacity > 0);
  unsigned char *hessian_bytes = (unsigned char *)malloc(hessian_capacity);
  assert_non_null(hessian_bytes);

  size_t hessian_size = 0;
  NWChemCResult result_status = nwchemc_calculate_hessian_result(
      params, params_size, force_input, force_input_size, hessian_bytes,
      hessian_capacity, &hessian_size);
  assert_success("nwchemc_calculate_hessian_result", result_status);
  assert_int_equal(hessian_size, hessian_capacity);
  assert_potential_result_hessian(hessian_bytes, hessian_size,
                                  force_input_hessian, ncoord);

  memset(hessian_bytes, 0, hessian_capacity);
  hessian_size = 0;
  NWChemCResult config_result_status =
      nwchemc_calculate_hessian_result_from_config(
          config, config_size, force_input, force_input_size, hessian_bytes,
          hessian_capacity, &hessian_size);
  assert_success("nwchemc_calculate_hessian_result_from_config",
                 config_result_status);
  assert_int_equal(hessian_size, hessian_capacity);
  assert_potential_result_hessian(hessian_bytes, hessian_size, config_hessian,
                                  ncoord);

  NWChemCSession *session =
      nwchemc_session_create_from_config(config, config_size);
  assert_non_null(session);
  memset(hessian_bytes, 0, hessian_capacity);
  hessian_size = 0;
  NWChemCResult session_result_status =
      nwchemc_session_calculate_hessian_result(
          session, force_input, force_input_size, hessian_bytes,
          hessian_capacity, &hessian_size);
  assert_success("nwchemc_session_calculate_hessian_result",
                 session_result_status);
  assert_int_equal(hessian_size, hessian_capacity);
  assert_potential_result_hessian(hessian_bytes, hessian_size, config_hessian,
                                  ncoord);

  nwchemc_session_destroy(session);
  free(hessian_bytes);
  free(force_input);
  free(config);
  free(params);
}

int main(int argc, char **argv) {
  if (argc != 4) {
    fprintf(stderr, "usage: %s PARAMS_BIN CONFIG_BIN FORCE_INPUT_BIN\n",
            argv[0]);
    return 2;
  }
  g_params_path = argv[1];
  g_config_path = argv[2];
  g_force_input_path = argv[3];
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_h2_hessian),
  };
  return cmocka_run_group_tests(tests, NULL, teardown_nwchem);
}
