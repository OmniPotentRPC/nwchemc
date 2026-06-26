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
#include <sys/stat.h>

#include <cmocka.h>

static const char *g_params_path = NULL;
static const char *g_step_eq_path = NULL;
static const char *g_step_stretched_path = NULL;

static int ensure_dir(const char *path) {
  if (mkdir(path, 0777) == 0 || errno == EEXIST)
    return 0;
  fprintf(stderr, "mkdir failed for %s: %s\n", path, strerror(errno));
  return -1;
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

static int setup_nwchem_dirs(void **state) {
  (void)state;
  if (ensure_dir(NWCHEMC_TEST_SCRATCH_DIR) != 0)
    return 1;
  return ensure_dir(NWCHEMC_TEST_PERMANENT_DIR);
}

static int teardown_nwchem(void **state) {
  (void)state;
  nwchemc_finalize();
  return 0;
}

static void assert_close(double actual, double expected, double tolerance) {
  assert_true(actual > expected - tolerance);
  assert_true(actual < expected + tolerance);
}

static double assert_potential_result(const unsigned char *message,
                                      size_t message_size,
                                      double expected_energy) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);
  struct PotentialResult result;
  read_PotentialResult(&result, root);
  assert_close(result.energy, expected_energy, 1.0e-12);
  capn_resolve(&result.forces.p);
  assert_int_equal(result.forces.p.type, CAPN_LIST);
  assert_int_equal(result.forces.p.datasz, 8);
  assert_int_equal(result.forces.p.len, 6);
  for (int i = 0; i < result.forces.p.len; ++i) {
    double force = capn_to_f64(capn_get64(result.forces, i));
    if (!isfinite(force))
      fail_msg("non-finite force[%d]", i);
  }
  double energy = result.energy;
  capn_free(&arena);
  return energy;
}

static NWChemCResult calculate_result_step(
    NWChemCSession *session, const unsigned char *step, size_t step_size,
    unsigned char *result_bytes, size_t result_capacity, size_t *result_size,
    double *result_energy) {
  *result_size = 0;
  NWChemCResult result =
      nwchemc_session_calculate_result(session, step, step_size, result_bytes,
                                       result_capacity, result_size);
  if (!result.ok)
    fail_msg("nwchemc_session_calculate_result failed: %s", result.message);
  *result_energy =
      assert_potential_result(result_bytes, *result_size, result.energy_h);
  return result;
}

static void test_session_calculate_result_accepts_multiple_steps(void **state) {
  (void)state;
  assert_true(nwchemc_available());

  size_t params_size = 0;
  size_t step_eq_size = 0;
  size_t step_stretched_size = 0;
  unsigned char *params = read_file(g_params_path, &params_size);
  unsigned char *step_eq = read_file(g_step_eq_path, &step_eq_size);
  unsigned char *step_stretched =
      read_file(g_step_stretched_path, &step_stretched_size);
  assert_non_null(params);
  assert_non_null(step_eq);
  assert_non_null(step_stretched);

  size_t expected_eq_size =
      nwchemc_potential_result_size_for_force_input(step_eq, step_eq_size);
  size_t expected_stretched_size = nwchemc_potential_result_size_for_force_input(
      step_stretched, step_stretched_size);
  assert_true(expected_eq_size > 0);
  assert_int_equal(expected_stretched_size, expected_eq_size);

  NWChemCSession *session = nwchemc_session_create(params, params_size);
  assert_non_null(session);

  unsigned char result_bytes[256];
  size_t eq_result_size = 0;
  size_t stretched_result_size = 0;
  size_t repeated_result_size = 0;
  double eq_energy = 0.0;
  double stretched_energy = 0.0;
  double repeated_energy = 0.0;

  NWChemCResult eq_status = calculate_result_step(
      session, step_eq, step_eq_size, result_bytes, sizeof(result_bytes),
      &eq_result_size, &eq_energy);
  NWChemCResult stretched_status = calculate_result_step(
      session, step_stretched, step_stretched_size, result_bytes,
      sizeof(result_bytes), &stretched_result_size, &stretched_energy);
  NWChemCResult repeated_status = calculate_result_step(
      session, step_eq, step_eq_size, result_bytes, sizeof(result_bytes),
      &repeated_result_size, &repeated_energy);

  assert_int_equal(eq_result_size, expected_eq_size);
  assert_int_equal(stretched_result_size, expected_stretched_size);
  assert_int_equal(repeated_result_size, expected_eq_size);
  assert_true(isfinite(eq_status.energy_h));
  assert_true(isfinite(stretched_status.energy_h));
  assert_true(isfinite(repeated_status.energy_h));
  assert_close(repeated_energy, eq_energy, 1.0e-10);
  assert_true(fabs(stretched_energy - eq_energy) > 1.0e-5);

  nwchemc_session_destroy(session);
  free(step_stretched);
  free(step_eq);
  free(params);
}

int main(int argc, char **argv) {
  if (argc != 4) {
    fprintf(stderr,
            "usage: %s PARAMS_BIN FORCE_STEP_EQ_BIN FORCE_STEP_STRETCHED_BIN\n",
            argv[0]);
    return 2;
  }
  g_params_path = argv[1];
  g_step_eq_path = argv[2];
  g_step_stretched_path = argv[3];
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_session_calculate_result_accepts_multiple_steps),
  };
  return cmocka_run_group_tests(tests, setup_nwchem_dirs, teardown_nwchem);
}
