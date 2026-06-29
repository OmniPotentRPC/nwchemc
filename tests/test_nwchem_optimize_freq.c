/* Drive shipped nwchemc_optimize / nwchemc_frequencies for CLI compare. */
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

static const char *g_mode = NULL;
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

static void write_compare_json_optimize(double energy_h, int n_atoms,
                                        const double *pos_ang) {
  const char *path = getenv("NWCHEMC_COMPARE_JSON");
  if (!path || !path[0])
    return;
  FILE *fp = fopen(path, "w");
  if (!fp)
    return;
  int ncoord = 3 * n_atoms;
  fprintf(fp, "{\n  \"source\": \"nwchemc_optimize\",\n");
  fprintf(fp, "  \"label\": \"scf\",\n");
  fprintf(fp, "  \"n_atoms\": %d,\n", n_atoms);
  fprintf(fp, "  \"energy_ha\": %.17g,\n", energy_h);
  fprintf(fp, "  \"optimized_positions_ang\": [");
  for (int i = 0; i < ncoord; ++i) {
    if (i)
      fputc(',', fp);
    fprintf(fp, "%.17g", pos_ang[i]);
  }
  fprintf(fp, "],\n  \"n_positions\": %d\n}\n", ncoord);
  fclose(fp);
}

static void write_compare_json_frequencies(double energy_h, int n_atoms,
                                           const double *freq_cm1) {
  const char *path = getenv("NWCHEMC_COMPARE_JSON");
  if (!path || !path[0])
    return;
  FILE *fp = fopen(path, "w");
  if (!fp)
    return;
  int nmode = 3 * n_atoms;
  fprintf(fp, "{\n  \"source\": \"nwchemc_frequencies\",\n");
  fprintf(fp, "  \"label\": \"scf\",\n");
  fprintf(fp, "  \"n_atoms\": %d,\n", n_atoms);
  fprintf(fp, "  \"energy_ha\": %.17g,\n", energy_h);
  fprintf(fp, "  \"frequencies_cm1\": [");
  for (int i = 0; i < nmode; ++i) {
    if (i)
      fputc(',', fp);
    fprintf(fp, "%.17g", freq_cm1[i]);
  }
  fprintf(fp, "],\n  \"n_frequencies\": %d\n}\n", nmode);
  fclose(fp);
}

/* Same starting geometry as tests/integration/nw/h2_scf_*.nw CLI fixtures. */
static void test_h2_optimize_or_freq(void **state) {
  (void)state;
  assert_true(nwchemc_available());
  size_t params_size = 0;
  unsigned char *params = read_file(g_params_path, &params_size);
  assert_non_null(params);

  const int n_atoms = 2;
  const int ncoord = 3 * n_atoms;
  const int atomic_numbers[2] = {1, 1};
  const double positions_ang[6] = {0.0, 0.0, -0.3707, 0.0, 0.0, 0.3707};

  if (strcmp(g_mode, "optimize") == 0) {
    double optimized[6];
    memset(optimized, 0, sizeof(optimized));
    NWChemCResult result = nwchemc_optimize(
        n_atoms, positions_ang, atomic_numbers, params, params_size, optimized);
    if (!result.ok)
      fail_msg("nwchemc_optimize failed: %s", result.message);
    assert_true(isfinite(result.energy_h));
    for (int i = 0; i < ncoord; ++i) {
      if (!isfinite(optimized[i]))
        fail_msg("non-finite optimized position[%d]", i);
    }
    fprintf(stderr, "optimize energy_h=%.12g positions_ang=[", result.energy_h);
    for (int i = 0; i < ncoord; ++i)
      fprintf(stderr, "%s%.8g", i ? "," : "", optimized[i]);
    fprintf(stderr, "]\n");
    write_compare_json_optimize(result.energy_h, n_atoms, optimized);
  } else if (strcmp(g_mode, "frequencies") == 0) {
    double frequencies[6];
    double intensities[6];
    memset(frequencies, 0, sizeof(frequencies));
    memset(intensities, 0, sizeof(intensities));
    NWChemCResult result = nwchemc_frequencies(
        n_atoms, positions_ang, atomic_numbers, params, params_size,
        frequencies, intensities);
    if (!result.ok)
      fail_msg("nwchemc_frequencies failed: %s", result.message);
    assert_true(isfinite(result.energy_h));
    double max_abs = 0.0;
    for (int i = 0; i < ncoord; ++i) {
      if (!isfinite(frequencies[i]))
        fail_msg("non-finite frequency[%d]", i);
      max_abs = fmax(max_abs, fabs(frequencies[i]));
    }
    assert_true(max_abs > 1.0);
    fprintf(stderr, "frequencies energy_h=%.12g freq_cm1=[", result.energy_h);
    for (int i = 0; i < ncoord; ++i)
      fprintf(stderr, "%s%.6g", i ? "," : "", frequencies[i]);
    fprintf(stderr, "]\n");
    write_compare_json_frequencies(result.energy_h, n_atoms, frequencies);
  } else {
    fail_msg("unknown mode %s", g_mode);
  }
  free(params);
}

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s optimize|frequencies PARAMS_BIN\n", argv[0]);
    return 2;
  }
  g_mode = argv[1];
  g_params_path = argv[2];
  if (strcmp(g_mode, "optimize") != 0 && strcmp(g_mode, "frequencies") != 0) {
    fprintf(stderr, "usage: %s optimize|frequencies PARAMS_BIN\n", argv[0]);
    return 2;
  }
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_h2_optimize_or_freq),
  };
  return cmocka_run_group_tests(tests, NULL, teardown_nwchem);
}
