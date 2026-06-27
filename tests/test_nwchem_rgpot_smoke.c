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

static const char *g_config_path = NULL;
static const char *g_force_input_path = NULL;

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
  if (ensure_dir(NWCHEMC_TEST_PERMANENT_DIR) != 0)
    return 1;
  if (ensure_dir(NWCHEMC_TEST_SCRATCH_DIR "-config") != 0)
    return 1;
  return ensure_dir(NWCHEMC_TEST_PERMANENT_DIR "-config");
}

static int teardown_nwchem(void **state) {
  (void)state;
  nwchemc_finalize();
  return 0;
}

static void assert_potential_result_forces(const unsigned char *message,
                                           size_t message_size,
                                           double expected_energy) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);

  struct PotentialResult result;
  read_PotentialResult(&result, root);
  assert_true(isfinite(result.energy));
  assert_true(fabs(result.energy - expected_energy) < 1.0e-12);

  capn_resolve(&result.forces.p);
  assert_int_equal(result.forces.p.type, CAPN_LIST);
  assert_int_equal(result.forces.p.datasz, 8);
  assert_int_equal(result.forces.p.len, 6);
  for (int i = 0; i < result.forces.p.len; ++i) {
    double force = capn_to_f64(capn_get64(result.forces, i));
    if (!isfinite(force))
      fail_msg("non-finite force[%d]", i);
  }

  capn_free(&arena);
}

static void assert_status_energy(const char *label, NWChemCResult status) {
  if (!status.ok)
    fail_msg("%s failed: %s", label, status.message);
  assert_true(isfinite(status.energy_h));
  assert_true(status.energy_h < -0.9);
}

static void test_rgpot_config_forceinput_result_carrier(void **state) {
  (void)state;
  assert_true(nwchemc_available());

  size_t config_size = 0;
  size_t force_input_size = 0;
  unsigned char *config = read_file(g_config_path, &config_size);
  unsigned char *force_input = read_file(g_force_input_path, &force_input_size);
  assert_non_null(config);
  assert_non_null(force_input);

  size_t result_capacity =
      nwchemc_potential_result_size_for_force_input(force_input,
                                                    force_input_size);
  assert_true(result_capacity > 0);
  unsigned char *result_bytes =
      (unsigned char *)malloc(result_capacity);
  assert_non_null(result_bytes);

  size_t result_size = 0;
  NWChemCResult one_shot = nwchemc_calculate_result_from_config(
      config, config_size, force_input, force_input_size, result_bytes,
      result_capacity, &result_size);
  assert_status_energy("nwchemc_calculate_result_from_config", one_shot);
  assert_int_equal(result_size, result_capacity);
  assert_potential_result_forces(result_bytes, result_size, one_shot.energy_h);

  NWChemCSession *session =
      nwchemc_session_create_from_config(config, config_size);
  assert_non_null(session);
  memset(result_bytes, 0, result_capacity);
  result_size = 0;
  NWChemCResult session_result = nwchemc_session_calculate_result(
      session, force_input, force_input_size, result_bytes, result_capacity,
      &result_size);
  assert_status_energy("nwchemc_session_calculate_result", session_result);
  assert_int_equal(result_size, result_capacity);
  assert_potential_result_forces(result_bytes, result_size,
                                 session_result.energy_h);

  nwchemc_session_destroy(session);
  free(result_bytes);
  free(force_input);
  free(config);
}

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s POTENTIAL_CONFIG_BIN FORCE_INPUT_BIN\n",
            argv[0]);
    return 2;
  }
  g_config_path = argv[1];
  g_force_input_path = argv[2];
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_rgpot_config_forceinput_result_carrier),
  };
  return cmocka_run_group_tests(tests, setup_nwchem_dirs, teardown_nwchem);
}
