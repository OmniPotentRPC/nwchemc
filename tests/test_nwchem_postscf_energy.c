/* Drive shipped nwchemc_energy for post-SCF theories (ccsd/mp2/tce). */
#include "nwchemc.h"

#include <errno.h>
#include <math.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cmocka.h>

static const char *g_label = NULL;
static const char *g_params_path = NULL;

static unsigned char *read_file(const char *path, size_t *size) {
  FILE *fp = fopen(path, "rb");
  if (!fp)
    return NULL;
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

static int teardown_nwchem(void **state) {
  (void)state;
  nwchemc_finalize();
  return 0;
}

static void test_h2_postscf_energy_finite(void **state) {
  (void)state;
  assert_true(nwchemc_available());
  size_t params_size = 0;
  unsigned char *params = read_file(g_params_path, &params_size);
  assert_non_null(params);

  const int atomic_numbers[2] = {1, 1};
  const double positions_ang[6] = {0.0, 0.0, -0.3707, 0.0, 0.0, 0.3707};

  NWChemCResult result = nwchemc_energy(2, positions_ang, atomic_numbers,
                                        params, params_size);
  if (!result.ok)
    fail_msg("%s nwchemc_energy failed: %s", g_label ? g_label : "postscf",
             result.message);
  if (!isfinite(result.energy_h))
    fail_msg("%s non-finite energy_h", g_label ? g_label : "postscf");
  /* H2 STO-3G correlated energies are negative and finite. */
  assert_true(result.energy_h < 0.0);
  fprintf(stderr, "%s energy_h=%.12g\n", g_label ? g_label : "postscf",
          result.energy_h);

  free(params);
}

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s LABEL PARAMS_BIN\n", argv[0]);
    return 2;
  }
  g_label = argv[1];
  g_params_path = argv[2];
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_h2_postscf_energy_finite),
  };
  return cmocka_run_group_tests(tests, NULL, teardown_nwchem);
}
