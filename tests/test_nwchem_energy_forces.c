/* Drive shipped nwchemc_energy_forces / nwchemc_energy_gradient for SP theories. */
#define _POSIX_C_SOURCE 200809L
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

static int isolate_embed_dirs(const char *label) {
  char scratch[512];
  char permanent[512];
  const char *lab = (label && label[0]) ? label : "sp";
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
  assert_int_equal(isolate_embed_dirs(g_label), 0);
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
    fail_msg("%s nwchemc_energy_gradient failed: %s",
             g_label ? g_label : "sp", grad_result.message);

  NWChemCResult force_result = nwchemc_energy_forces(
      n_atoms, positions_ang, atomic_numbers, params, params_size, forces);
  if (!force_result.ok)
    fail_msg("%s nwchemc_energy_forces failed: %s",
             g_label ? g_label : "sp", force_result.message);

  assert_true(isfinite(force_result.energy_h));
  /* Separate gradient/forces embeds can differ (numerical grads / prior suite
   * movecs); gate forces = -gradient rather than micro-Ha energy identity. */
  assert_true(fabs(force_result.energy_h - grad_result.energy_h) < 5.0e-3);
  for (int i = 0; i < ncoord; ++i) {
    if (!isfinite(forces[i]))
      fail_msg("%s non-finite force[%d]", g_label ? g_label : "sp", i);
    if (!isfinite(grad[i]))
      fail_msg("%s non-finite grad[%d]", g_label ? g_label : "sp", i);
    /* Consecutive gradient vs forces embeds can differ slightly for CCSD
     * (numerical gradients + leftover movecs/state from prior suite cases). */
    if (fabs(forces[i] + grad[i]) > 1.0e-4)
      fail_msg("%s force/gradient mismatch at %d: force=%.17g grad=%.17g",
               g_label ? g_label : "sp", i, forces[i], grad[i]);
  }
  fprintf(stderr, "%s energy_h=%.12g forces=[", g_label ? g_label : "sp",
          force_result.energy_h);
  for (int i = 0; i < ncoord; ++i)
    fprintf(stderr, "%s%.6g", i ? "," : "", forces[i]);
  fprintf(stderr, "]\n");

  /* Machine-readable embed leg for tools/compare_nwchem_cli.py */
  {
    const char *path = getenv("NWCHEMC_COMPARE_JSON");
    if (path && path[0]) {
      FILE *fp = fopen(path, "w");
      if (fp) {
        fprintf(fp, "{\n  \"source\": \"nwchemc_energy_forces\",\n");
        fprintf(fp, "  \"label\": \"%s\",\n", g_label ? g_label : "sp");
        fprintf(fp, "  \"n_atoms\": %d,\n", n_atoms);
        fprintf(fp, "  \"energy_ha\": %.17g,\n", grad_result.energy_h);
        fprintf(fp, "  \"gradient_ha_bohr\": [");
        for (int i = 0; i < ncoord; ++i) {
          if (i)
            fputc(',', fp);
          fprintf(fp, "%.17g", grad[i]);
        }
        fprintf(fp, "],\n  \"forces_ha_bohr\": [");
        for (int i = 0; i < ncoord; ++i) {
          if (i)
            fputc(',', fp);
          fprintf(fp, "%.17g", forces[i]);
        }
        fprintf(fp, "],\n  \"n_gradient\": %d\n}\n", ncoord);
        fclose(fp);
      }
    }
  }

  free(params);
}

int main(int argc, char **argv) {
  if (argc == 2) {
    g_label = "scf";
    g_params_path = argv[1];
  } else if (argc == 3) {
    g_label = argv[1];
    g_params_path = argv[2];
  } else {
    fprintf(stderr, "usage: %s [LABEL] PARAMS_BIN\n", argv[0]);
    return 2;
  }
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_h2_energy_forces_match_negative_gradient),
  };
  return cmocka_run_group_tests(tests, NULL, teardown_nwchem);
}
