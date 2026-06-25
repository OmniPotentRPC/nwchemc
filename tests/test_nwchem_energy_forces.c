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

static const char *g_params_path = NULL;

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

static int teardown_nwchem(void **state) {
  (void)state;
  nwchemc_finalize();
  return 0;
}

static void test_h2_energy_forces_match_negative_gradient(void **state) {
  (void)state;
  assert_true(nwchemc_available());
  size_t params_size = 0;
  unsigned char *params = read_file(g_params_path, &params_size);
  assert_non_null(params);

  const int n_atoms = 2;
  const int ncoord = 3 * n_atoms;
  const int atomic_numbers[2] = {1, 1};
  const double positions_ang[6] = {0.0, 0.0, -0.3707,
                                   0.0, 0.0, 0.3707};
  double grad[6];
  double forces[6];
  memset(grad, 0, sizeof(grad));
  memset(forces, 0, sizeof(forces));

  NWChemCResult grad_result = nwchemc_energy_gradient(
      n_atoms, positions_ang, atomic_numbers, params, params_size, grad);
  if (!grad_result.ok)
    fail_msg("nwchemc_energy_gradient failed: %s", grad_result.message);

  NWChemCResult force_result = nwchemc_energy_forces(
      n_atoms, positions_ang, atomic_numbers, params, params_size, forces);
  if (!force_result.ok)
    fail_msg("nwchemc_energy_forces failed: %s", force_result.message);

  assert_true(isfinite(force_result.energy_h));
  assert_true(fabs(force_result.energy_h - grad_result.energy_h) < 1.0e-10);
  for (int i = 0; i < ncoord; ++i) {
    if (!isfinite(forces[i]))
      fail_msg("non-finite force[%d]", i);
    if (fabs(forces[i] + grad[i]) > 1.0e-10)
      fail_msg("force/gradient mismatch at %d: force=%.17g grad=%.17g", i,
               forces[i], grad[i]);
  }

  free(params);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s PARAMS_BIN\n", argv[0]);
    return 2;
  }
  g_params_path = argv[1];
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_h2_energy_forces_match_negative_gradient),
  };
  return cmocka_run_group_tests(tests, NULL, teardown_nwchem);
}
