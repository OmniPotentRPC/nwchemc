#include "nwchemc.h"

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

extern void nwchemc_test_configured_pseudopotential_rtdb(int *result);

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
  if (ensure_dir(NWCHEMC_TEST_SCRATCH_DIR "-config-psp") != 0)
    return 1;
  return ensure_dir(NWCHEMC_TEST_PERMANENT_DIR "-config-psp");
}

static int teardown_nwchem(void **state) {
  (void)state;
  nwchemc_finalize();
  return 0;
}

static void test_potential_config_pseudopotentials_reach_rtdb(void **state) {
  (void)state;
  assert_true(nwchemc_available());

  size_t config_size = 0;
  size_t force_input_size = 0;
  unsigned char *config = read_file(g_config_path, &config_size);
  unsigned char *force_input = read_file(g_force_input_path, &force_input_size);
  assert_non_null(config);
  assert_non_null(force_input);

  const double positions_ang[6] = {0.0, 0.0, -0.3707, 0.0, 0.0, 0.3707};
  const int atomic_numbers[2] = {1, 1};
  NWChemCResult status = nwchemc_energy_from_config(
      2, positions_ang, atomic_numbers, config, config_size);
  if (!status.ok)
    fail_msg("nwchemc_energy_from_config failed: %s", status.message);
  assert_true(isfinite(status.energy_h));

  size_t result_capacity =
      nwchemc_potential_result_size_for_force_input(force_input,
                                                    force_input_size);
  assert_true(result_capacity > 0);
  unsigned char *result_bytes = (unsigned char *)malloc(result_capacity);
  assert_non_null(result_bytes);
  size_t result_size = 0;
  NWChemCResult result_status = nwchemc_calculate_result_from_config(
      config, config_size, force_input, force_input_size, result_bytes,
      result_capacity, &result_size);
  if (!result_status.ok)
    fail_msg("nwchemc_calculate_result_from_config failed: %s",
             result_status.message);
  assert_true(isfinite(result_status.energy_h));
  assert_int_equal(result_size, result_capacity);

  int probe_result = -1;
  nwchemc_test_configured_pseudopotential_rtdb(&probe_result);
  assert_int_equal(probe_result, 0);

  free(result_bytes);
  free(force_input);
  free(config);
}

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s potential-config.bin force-input.bin\n",
            argv[0]);
    return 2;
  }
  g_config_path = argv[1];
  g_force_input_path = argv[2];

  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_potential_config_pseudopotentials_reach_rtdb),
  };
  return cmocka_run_group_tests(tests, setup_nwchem_dirs, teardown_nwchem);
}
