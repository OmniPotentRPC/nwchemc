#include "nwchemc.h"

#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <cmocka.h>

static const char *g_params_path = NULL;

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

static void test_h2_energy_gradient(void **state) {
  (void)state;
  size_t params_size = 0;
  unsigned char *params = read_file(g_params_path, &params_size);
  assert_non_null(params);

  assert_true(nwchemc_available());

  const int n_atoms = 2;
  const int atomic_numbers[2] = {1, 1};
  const double positions_ang[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.74};
  double grad[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

  NWChemCResult result = nwchemc_energy_gradient(
      n_atoms, positions_ang, atomic_numbers, params, params_size, grad);
  free(params);

  if (!result.ok)
    fail_msg("nwchemc_energy_gradient failed: %s", result.message);
  assert_true(isfinite(result.energy_h));
  assert_true(result.energy_h < -0.5);
  assert_true(result.energy_h > -2.0);
  for (int i = 0; i < 6; ++i) {
    if (!isfinite(grad[i]))
      fail_msg("non-finite gradient[%d]", i);
  }
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s PARAMS_BIN\n", argv[0]);
    return 2;
  }
  g_params_path = argv[1];
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_h2_energy_gradient),
  };
  return cmocka_run_group_tests(tests, setup_nwchem_dirs, teardown_nwchem);
}
