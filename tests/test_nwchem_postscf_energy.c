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
#include <sys/stat.h>

#include <cmocka.h>

static const char *g_label = NULL;
static const char *g_params_path = NULL;

/* Per-label dirs so Meson suite order cannot leave movecs/RTDB that break
 * classic CCSD after rimp2/TCE (shared compile-time scratch path). */
static int isolate_embed_dirs(const char *label) {
  char scratch[512];
  char permanent[512];
  const char *lab = (label && label[0]) ? label : "postscf";
  snprintf(scratch, sizeof(scratch), "%s-%s", NWCHEMC_TEST_SCRATCH_DIR, lab);
  snprintf(permanent, sizeof(permanent), "%s-%s", NWCHEMC_TEST_PERMANENT_DIR,
           lab);
  if (mkdir(scratch, 0755) != 0 && errno != EEXIST)
    return -1;
  if (mkdir(permanent, 0755) != 0 && errno != EEXIST)
    return -1;
  if (setenv("NWCHEM_SCRATCH_DIR", scratch, 1) != 0)
    return -1;
  if (setenv("NWCHEM_PERMANENT_DIR", permanent, 1) != 0)
    return -1;
  return 0;
}

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
  assert_int_equal(isolate_embed_dirs(g_label), 0);
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

  /* Machine-readable embed leg for tools/compare_nwchem_cli.py */
  {
    const char *path = getenv("NWCHEMC_COMPARE_JSON");
    if (path && path[0]) {
      FILE *fp = fopen(path, "w");
      if (fp) {
        fprintf(fp, "{\n  \"source\": \"nwchemc_energy\",\n");
        fprintf(fp, "  \"label\": \"%s\",\n", g_label ? g_label : "postscf");
        fprintf(fp, "  \"n_atoms\": 2,\n");
        fprintf(fp, "  \"energy_ha\": %.17g,\n", result.energy_h);
        fprintf(fp, "  \"gradient_ha_bohr\": null,\n");
        fprintf(fp, "  \"n_gradient\": 0\n}\n");
        fclose(fp);
      }
    }
  }

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
