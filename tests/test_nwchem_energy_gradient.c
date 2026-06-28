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

/* Write machine-readable results for tools/compare_nwchem_cli.py embed leg. */
static void maybe_write_compare_json(double energy_h, const double *grad,
                                     int ncoord) {
  const char *path = getenv("NWCHEMC_COMPARE_JSON");
  if (!path || !path[0])
    return;
  FILE *fp = fopen(path, "w");
  if (!fp)
    return;
  fprintf(fp, "{\n  \"source\": \"nwchemc_energy_gradient\",\n");
  fprintf(fp, "  \"n_atoms\": 2,\n");
  fprintf(fp, "  \"energy_ha\": %.17g,\n", energy_h);
  fprintf(fp, "  \"gradient_ha_bohr\": [");
  for (int i = 0; i < ncoord; ++i) {
    if (i)
      fputc(',', fp);
    fprintf(fp, "%.17g", grad[i]);
  }
  fprintf(fp, "],\n  \"n_gradient\": %d\n}\n", ncoord);
  fclose(fp);
}

static void test_h2_energy_gradient(void **state) {
  (void)state;
  size_t params_size = 0;
  unsigned char *params = read_file(g_params_path, &params_size);
  assert_non_null(params);

  assert_true(nwchemc_available());

  const int n_atoms = 2;
  const int ncoord = n_atoms * 3;
  const int atomic_numbers[2] = {1, 1};
  /* Match tests/integration/nw/h2_scf_*.nw geometry (bond 0.7414 A). */
  const double positions_ang[6] = {0.0, 0.0, -0.3707, 0.0, 0.0, 0.3707};
  double grad[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

  NWChemCResult result = nwchemc_energy_gradient(
      n_atoms, positions_ang, atomic_numbers, params, params_size, grad);
  free(params);

  if (!result.ok)
    fail_msg("nwchemc_energy_gradient failed: %s", result.message);
  assert_true(isfinite(result.energy_h));
  assert_true(result.energy_h < -0.5);
  assert_true(result.energy_h > -2.0);
  for (int i = 0; i < ncoord; ++i) {
    if (!isfinite(grad[i]))
      fail_msg("non-finite gradient[%d]", i);
  }
  maybe_write_compare_json(result.energy_h, grad, ncoord);
}

static void test_h2_dipole(void **state) {
  (void)state;
  size_t params_size = 0;
  unsigned char *params = read_file(g_params_path, &params_size);
  assert_non_null(params);

  assert_true(nwchemc_available());

  const int n_atoms = 2;
  const int atomic_numbers[2] = {1, 1};
  const double positions_ang[6] = {0.0, 0.0, -0.3707, 0.0, 0.0, 0.3707};
  double dipole_au[3] = {0.0, 0.0, 0.0};

  NWChemCResult result = nwchemc_dipole(
      n_atoms, positions_ang, atomic_numbers, params, params_size, dipole_au);
  free(params);

  if (!result.ok)
    fail_msg("nwchemc_dipole failed: %s", result.message);
  assert_true(isfinite(result.energy_h));
  assert_true(result.energy_h < -0.5);
  assert_true(result.energy_h > -2.0);
  for (int i = 0; i < 3; ++i) {
    if (!isfinite(dipole_au[i]))
      fail_msg("non-finite dipole[%d]", i);
    assert_true(fabs(dipole_au[i]) < 1.0e-7);
  }
}

static void test_h2_polarizability(void **state) {
  (void)state;
  size_t params_size = 0;
  unsigned char *params = read_file(g_params_path, &params_size);
  assert_non_null(params);

  assert_true(nwchemc_available());

  const int n_atoms = 2;
  const int atomic_numbers[2] = {1, 1};
  const double positions_ang[6] = {0.0, 0.0, -0.3707, 0.0, 0.0, 0.3707};
  double polarizability_au[12] = {0.0};

  NWChemCResult result =
      nwchemc_polarizability(n_atoms, positions_ang, atomic_numbers, params,
                             params_size, polarizability_au);
  free(params);

  if (!result.ok)
    fail_msg("nwchemc_polarizability failed: %s", result.message);
  assert_true(isfinite(result.energy_h));
  assert_true(result.energy_h < -0.5);
  assert_true(result.energy_h > -2.0);
  double max_abs = 0.0;
  for (int i = 0; i < 12; ++i) {
    if (!isfinite(polarizability_au[i]))
      fail_msg("non-finite polarizability[%d]", i);
    max_abs = fmax(max_abs, fabs(polarizability_au[i]));
  }
  assert_true(max_abs > 1.0e-6);
  assert_true(polarizability_au[10] > 0.0);
}

static double h2_bond_length(const double *positions_ang) {
  double dx = positions_ang[3] - positions_ang[0];
  double dy = positions_ang[4] - positions_ang[1];
  double dz = positions_ang[5] - positions_ang[2];
  return sqrt(dx * dx + dy * dy + dz * dz);
}

static void test_h2_quadrupole(void **state) {
  (void)state;
  size_t params_size = 0;
  unsigned char *params = read_file(g_params_path, &params_size);
  assert_non_null(params);

  assert_true(nwchemc_available());

  const int n_atoms = 2;
  const int atomic_numbers[2] = {1, 1};
  const double positions_ang[6] = {0.0, 0.0, -0.3707, 0.0, 0.0, 0.3707};
  double quadrupole_au[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

  NWChemCResult result = nwchemc_quadrupole(
      n_atoms, positions_ang, atomic_numbers, params, params_size,
      quadrupole_au);
  free(params);

  if (!result.ok)
    fail_msg("nwchemc_quadrupole failed: %s", result.message);
  assert_true(isfinite(result.energy_h));
  double max_abs = 0.0;
  for (int i = 0; i < 6; ++i) {
    if (!isfinite(quadrupole_au[i]))
      fail_msg("non-finite quadrupole[%d]", i);
    max_abs = fmax(max_abs, fabs(quadrupole_au[i]));
  }
  assert_true(max_abs > 1.0e-6);
  double trace = quadrupole_au[0] + quadrupole_au[3] + quadrupole_au[5];
  assert_true(fabs(trace) < 1.0e-7 * fmax(1.0, max_abs));
}

static void test_h2_optimize(void **state) {
  (void)state;
  size_t params_size = 0;
  unsigned char *params = read_file(g_params_path, &params_size);
  assert_non_null(params);

  assert_true(nwchemc_available());

  const int n_atoms = 2;
  const int atomic_numbers[2] = {1, 1};
  const double positions_ang[6] = {0.0, 0.0, -0.6500, 0.0, 0.0, 0.6500};
  double optimized_positions[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

  NWChemCResult result =
      nwchemc_optimize(n_atoms, positions_ang, atomic_numbers, params,
                       params_size, optimized_positions);
  free(params);

  if (!result.ok)
    fail_msg("nwchemc_optimize failed: %s", result.message);
  assert_true(isfinite(result.energy_h));
  for (int i = 0; i < 6; ++i) {
    if (!isfinite(optimized_positions[i]))
      fail_msg("non-finite optimized position[%d]", i);
  }
  double bond = h2_bond_length(optimized_positions);
  assert_true(bond > 0.5);
  assert_true(bond < 1.0);
}

static void test_h2_frequencies(void **state) {
  (void)state;
  size_t params_size = 0;
  unsigned char *params = read_file(g_params_path, &params_size);
  assert_non_null(params);

  assert_true(nwchemc_available());

  const int n_atoms = 2;
  const int atomic_numbers[2] = {1, 1};
  const double positions_ang[6] = {0.0, 0.0, -0.3707, 0.0, 0.0, 0.3707};
  double frequencies[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  double intensities[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

  NWChemCResult result =
      nwchemc_frequencies(n_atoms, positions_ang, atomic_numbers, params,
                          params_size, frequencies, intensities);
  free(params);

  if (!result.ok)
    fail_msg("nwchemc_frequencies failed: %s", result.message);
  assert_true(isfinite(result.energy_h));
  double max_frequency_abs = 0.0;
  for (int i = 0; i < 6; ++i) {
    if (!isfinite(frequencies[i]))
      fail_msg("non-finite frequency[%d]", i);
    if (!isfinite(intensities[i]))
      fail_msg("non-finite intensity[%d]", i);
    max_frequency_abs = fmax(max_frequency_abs, fabs(frequencies[i]));
  }
  assert_true(max_frequency_abs > 1.0);
}

static void test_cd_energy_gradient_uses_heavy_element_symbol(void **state) {
  (void)state;
  size_t params_size = 0;
  unsigned char *params = read_file(g_params_path, &params_size);
  assert_non_null(params);

  assert_true(nwchemc_available());

  const int n_atoms = 1;
  const int ncoord = n_atoms * 3;
  const int atomic_numbers[1] = {48};
  const double positions_ang[3] = {0.0, 0.0, 0.0};
  double grad[3] = {0.0, 0.0, 0.0};

  NWChemCResult result = nwchemc_energy_gradient(
      n_atoms, positions_ang, atomic_numbers, params, params_size, grad);
  free(params);

  if (!result.ok)
    fail_msg("nwchemc_energy_gradient Cd failed: %s", result.message);
  assert_true(isfinite(result.energy_h));
  assert_true(result.energy_h < -10.0);
  for (int i = 0; i < ncoord; ++i) {
    if (!isfinite(grad[i]))
      fail_msg("non-finite Xe gradient[%d]", i);
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
      cmocka_unit_test(test_h2_dipole),
      cmocka_unit_test(test_h2_polarizability),
      cmocka_unit_test(test_h2_quadrupole),
      cmocka_unit_test(test_h2_optimize),
      cmocka_unit_test(test_h2_frequencies),
      cmocka_unit_test(test_cd_energy_gradient_uses_heavy_element_symbol),
  };
  return cmocka_run_group_tests(tests, setup_nwchem_dirs, teardown_nwchem);
}
