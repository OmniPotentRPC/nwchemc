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

static const char *g_text_params_path = NULL;
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

static void assert_close_value(const char *label, double actual,
                               double expected, double tolerance) {
  if (!isfinite(actual))
    fail_msg("non-finite %s", label);
  if (fabs(actual - expected) > tolerance)
    fail_msg("%s mismatch: actual=%.17g expected=%.17g tolerance=%.3g", label,
             actual, expected, tolerance);
}

static void test_structured_config_matches_text_blocks(void **state) {
  (void)state;
  assert_true(nwchemc_available());

  size_t text_params_size = 0;
  size_t config_size = 0;
  size_t force_input_size = 0;
  unsigned char *text_params = read_file(g_text_params_path, &text_params_size);
  unsigned char *config = read_file(g_config_path, &config_size);
  unsigned char *force_input = read_file(g_force_input_path, &force_input_size);
  assert_non_null(text_params);
  assert_non_null(config);
  assert_non_null(force_input);

  const size_t ncoord = 6;
  double text_forces[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  double config_forces[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

  NWChemCResult text_result =
      nwchemc_calculate_forces(text_params, text_params_size, force_input,
                               force_input_size, text_forces, ncoord);
  if (!text_result.ok)
    fail_msg("nwchemc_calculate_forces failed: %s", text_result.message);

  NWChemCResult config_result = nwchemc_calculate_forces_from_config(
      config, config_size, force_input, force_input_size, config_forces,
      ncoord);
  if (!config_result.ok)
    fail_msg("nwchemc_calculate_forces_from_config failed: %s",
             config_result.message);

  assert_close_value("energy", config_result.energy_h, text_result.energy_h,
                     1.0e-10);
  for (size_t i = 0; i < ncoord; ++i) {
    char label[32];
    snprintf(label, sizeof(label), "force[%zu]", i);
    assert_close_value(label, config_forces[i], text_forces[i], 1.0e-8);
  }

  free(force_input);
  free(config);
  free(text_params);
}

int main(int argc, char **argv) {
  if (argc != 4) {
    fprintf(stderr,
            "usage: %s TEXT_PARAMS_BIN POTENTIAL_CONFIG_BIN FORCE_INPUT_BIN\n",
            argv[0]);
    return 2;
  }
  g_text_params_path = argv[1];
  g_config_path = argv[2];
  g_force_input_path = argv[3];
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_structured_config_matches_text_blocks),
  };
  return cmocka_run_group_tests(tests, setup_nwchem_dirs, teardown_nwchem);
}
