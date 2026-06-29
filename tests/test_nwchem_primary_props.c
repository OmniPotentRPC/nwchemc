/* Drive shipped primary property/hessian entry points for CLI compare. */
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

static void write_json_array(FILE *fp, const char *key, const double *v, int n) {
  fprintf(fp, "  \"%s\": [", key);
  for (int i = 0; i < n; ++i) {
    if (i)
      fputc(',', fp);
    fprintf(fp, "%.17g", v[i]);
  }
  fprintf(fp, "]");
}

static void test_primary(void **state) {
  (void)state;
  assert_true(nwchemc_available());
  size_t params_size = 0;
  unsigned char *params = read_file(g_params_path, &params_size);
  assert_non_null(params);
  const int n_atoms = 2;
  const int atomic_numbers[2] = {1, 1};
  const double positions_ang[6] = {0.0, 0.0, -0.3707, 0.0, 0.0, 0.3707};
  const char *jpath = getenv("NWCHEMC_COMPARE_JSON");

  if (strcmp(g_mode, "dipole") == 0) {
    double dipole[3] = {0};
    NWChemCResult r = nwchemc_dipole(n_atoms, positions_ang, atomic_numbers,
                                     params, params_size, dipole);
    if (!r.ok)
      fail_msg("nwchemc_dipole: %s", r.message);
    assert_true(isfinite(r.energy_h));
    for (int i = 0; i < 3; ++i)
      assert_true(isfinite(dipole[i]));
    fprintf(stderr, "dipole energy_h=%.12g d=[%.6g,%.6g,%.6g]\n", r.energy_h,
            dipole[0], dipole[1], dipole[2]);
    if (jpath && jpath[0]) {
      FILE *fp = fopen(jpath, "w");
      if (fp) {
        fprintf(fp, "{\n  \"source\": \"nwchemc_dipole\",\n  \"n_atoms\": 2,\n");
        fprintf(fp, "  \"energy_ha\": %.17g,\n", r.energy_h);
        write_json_array(fp, "dipole_au", dipole, 3);
        fprintf(fp, "\n}\n");
        fclose(fp);
      }
    }
  } else if (strcmp(g_mode, "polarizability") == 0) {
    double alpha[12] = {0};
    NWChemCResult r = nwchemc_polarizability(
        n_atoms, positions_ang, atomic_numbers, params, params_size, alpha);
    if (!r.ok)
      fail_msg("nwchemc_polarizability: %s", r.message);
    assert_true(isfinite(r.energy_h));
    double max_abs = 0.0;
    for (int i = 0; i < 12; ++i) {
      assert_true(isfinite(alpha[i]));
      max_abs = fmax(max_abs, fabs(alpha[i]));
    }
    /* Non-trivial polarizability response for H2 (not energy-only theater). */
    assert_true(max_abs > 1.0e-3);
    fprintf(stderr, "polarizability energy_h=%.12g max_abs=%.6g\n", r.energy_h,
            max_abs);
    if (jpath && jpath[0]) {
      FILE *fp = fopen(jpath, "w");
      if (fp) {
        fprintf(fp,
                "{\n  \"source\": \"nwchemc_polarizability\",\n  \"n_atoms\": 2,\n");
        fprintf(fp, "  \"energy_ha\": %.17g,\n", r.energy_h);
        write_json_array(fp, "polarizability_au", alpha, 12);
        fprintf(fp, "\n}\n");
        fclose(fp);
      }
    }
  } else if (strcmp(g_mode, "quadrupole") == 0) {
    double q[6] = {0};
    NWChemCResult r = nwchemc_quadrupole(n_atoms, positions_ang, atomic_numbers,
                                         params, params_size, q);
    if (!r.ok)
      fail_msg("nwchemc_quadrupole: %s", r.message);
    assert_true(isfinite(r.energy_h));
    double max_abs = 0.0;
    for (int i = 0; i < 6; ++i) {
      assert_true(isfinite(q[i]));
      max_abs = fmax(max_abs, fabs(q[i]));
    }
    assert_true(max_abs > 1.0e-4);
    fprintf(stderr, "quadrupole energy_h=%.12g max_abs=%.6g\n", r.energy_h,
            max_abs);
    if (jpath && jpath[0]) {
      FILE *fp = fopen(jpath, "w");
      if (fp) {
        fprintf(fp,
                "{\n  \"source\": \"nwchemc_quadrupole\",\n  \"n_atoms\": 2,\n");
        fprintf(fp, "  \"energy_ha\": %.17g,\n", r.energy_h);
        write_json_array(fp, "quadrupole_au", q, 6);
        fprintf(fp, "\n}\n");
        fclose(fp);
      }
    }
  } else if (strcmp(g_mode, "hessian") == 0) {
    double hess[36] = {0};
    NWChemCResult r = nwchemc_hessian(n_atoms, positions_ang, atomic_numbers,
                                      params, params_size, hess);
    if (!r.ok)
      fail_msg("nwchemc_hessian: %s", r.message);
    assert_true(isfinite(r.energy_h));
    double max_abs = 0.0;
    for (int i = 0; i < 36; ++i) {
      assert_true(isfinite(hess[i]));
      max_abs = fmax(max_abs, fabs(hess[i]));
    }
    assert_true(max_abs > 1.0e-4);
    fprintf(stderr, "hessian energy_h=%.12g max_abs=%.6g\n", r.energy_h,
            max_abs);
    if (jpath && jpath[0]) {
      FILE *fp = fopen(jpath, "w");
      if (fp) {
        fprintf(fp, "{\n  \"source\": \"nwchemc_hessian\",\n  \"n_atoms\": 2,\n");
        fprintf(fp, "  \"energy_ha\": %.17g,\n", r.energy_h);
        write_json_array(fp, "hessian_ha_bohr2", hess, 36);
        fprintf(fp, "\n}\n");
        fclose(fp);
      }
    }
  } else {
    fail_msg("unknown mode %s", g_mode);
  }
  free(params);
}

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr,
            "usage: %s dipole|polarizability|quadrupole|hessian PARAMS_BIN\n",
            argv[0]);
    return 2;
  }
  g_mode = argv[1];
  g_params_path = argv[2];
  const struct CMUnitTest tests[] = {cmocka_unit_test(test_primary)};
  return cmocka_run_group_tests(tests, NULL, teardown_nwchem);
}
