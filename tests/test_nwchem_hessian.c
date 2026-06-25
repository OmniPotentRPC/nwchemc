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

static void call_gradient(const double *positions_ang,
                          const int *atomic_numbers,
                          const unsigned char *params, size_t params_size,
                          double *grad) {
  NWChemCResult result = nwchemc_energy_gradient(
      2, positions_ang, atomic_numbers, params, params_size, grad);
  if (!result.ok)
    fail_msg("nwchemc_energy_gradient failed: %s", result.message);
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
  unsigned char *params = read_file(g_params_path, &params_size);
  assert_non_null(params);

  const int n_atoms = 2;
  const int ncoord = 3 * n_atoms;
  const int atomic_numbers[2] = {1, 1};
  const double base_positions[6] = {0.0, 0.0, -0.3707,
                                    0.0, 0.0, 0.3707};
  double hessian[36];
  memset(hessian, 0, sizeof(hessian));

  NWChemCResult result = nwchemc_hessian(
      n_atoms, base_positions, atomic_numbers, params, params_size, hessian);
  if (!result.ok)
    fail_msg("nwchemc_hessian failed: %s", result.message);

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

  free(params);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s PARAMS_BIN\n", argv[0]);
    return 2;
  }
  g_params_path = argv[1];
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_h2_hessian),
  };
  return cmocka_run_group_tests(tests, NULL, teardown_nwchem);
}
