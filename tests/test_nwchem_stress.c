#include "nwchemc.h"
#include "nwchemc_params.h"
#include "Potentials.capnp.h"

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

static const char *g_config_path = NULL;
static const char *g_force_input_path = NULL;
static const char *g_unit_force_input_path = NULL;

enum { MAX_FORCE_INPUT_ATOMS = 16 };

static const double BOHR_TO_ANGSTROM = 0.529177210903;
static const double HARTREE_TO_EV = 27.211386245988;

typedef struct ParsedForceInput {
  int n_atoms;
  double positions_ang[MAX_FORCE_INPUT_ATOMS * 3];
  int atomic_numbers[MAX_FORCE_INPUT_ATOMS];
  double cell_ang[9];
  int has_cell;
} ParsedForceInput;

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
  if (ensure_dir(NWCHEMC_TEST_SCRATCH_DIR "-pspw-stress") != 0)
    return 1;
  return ensure_dir(NWCHEMC_TEST_PERMANENT_DIR "-pspw-stress");
}

static int teardown_nwchem(void **state) {
  (void)state;
  nwchemc_finalize();
  return 0;
}

static void assert_close_relative_tol(const char *label, int index,
                                      double actual, double expected,
                                      double tolerance) {
  double scale = fmax(1.0, fmax(fabs(actual), fabs(expected)));
  if (fabs(actual - expected) > tolerance * scale)
    fail_msg("%s[%d] mismatch: got %.17g expected %.17g", label, index,
             actual, expected);
}

static void assert_close_relative(const char *label, int index, double actual,
                                  double expected) {
  assert_close_relative_tol(label, index, actual, expected, 1.0e-5);
}

static void parse_force_input_geometry(const unsigned char *force_input,
                                       size_t force_input_size,
                                       ParsedForceInput *parsed) {
  assert_non_null(force_input);
  assert_non_null(parsed);

  struct capn arena;
  ForceInput_ptr root;
  if (nwchemc_force_input_root(force_input, force_input_size, &arena, &root) !=
      0)
    fail_msg("ForceInput parse failed");

  size_t n_atoms = 0;
  int has_cell = 0;
  if (nwchemc_force_input_atom_count(root, &n_atoms, &has_cell) != 0 ||
      n_atoms > MAX_FORCE_INPUT_ATOMS) {
    nwchemc_params_release(&arena);
    fail_msg("ForceInput atom count failed");
  }

  if (nwchemc_force_input_copy_geometry(
          root, parsed->positions_ang, parsed->atomic_numbers,
          MAX_FORCE_INPUT_ATOMS, parsed->cell_ang, &has_cell) != 0) {
    nwchemc_params_release(&arena);
    fail_msg("ForceInput geometry copy failed");
  }
  nwchemc_params_release(&arena);

  parsed->n_atoms = (int)n_atoms;
  parsed->has_cell = has_cell;
  assert_int_equal(parsed->has_cell, 1);
}

static void assert_potential_result_stress(const unsigned char *message,
                                           size_t message_size,
                                           double expected_energy,
                                           const double *native_stress,
                                           double stress_factor,
                                           double stress_tolerance) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);

  struct PotentialResult result;
  read_PotentialResult(&result, root);
  assert_true(isfinite(result.energy));
  assert_close_relative("PotentialResult.energy", 0, result.energy,
                        expected_energy);

  capn_resolve(&result.stress.p);
  assert_int_equal(result.stress.p.type, CAPN_LIST);
  assert_int_equal(result.stress.p.datasz, 8);
  assert_int_equal(result.stress.p.len, 9);
  for (int i = 0; i < result.stress.p.len; ++i) {
    double stress = capn_to_f64(capn_get64(result.stress, i));
    if (!isfinite(stress))
      fail_msg("non-finite PotentialResult.stress[%d]", i);
    if (native_stress) {
      double expected = native_stress[i] * stress_factor;
      assert_close_relative_tol("PotentialResult.stress", i, stress, expected,
                                stress_tolerance);
    }
  }

  capn_free(&arena);
}

static void test_pspw_stress_from_config(void **state) {
  (void)state;
  assert_true(nwchemc_available());

  size_t config_size = 0;
  size_t force_input_size = 0;
  size_t unit_force_input_size = 0;
  unsigned char *config = read_file(g_config_path, &config_size);
  unsigned char *force_input = read_file(g_force_input_path, &force_input_size);
  unsigned char *unit_force_input =
      read_file(g_unit_force_input_path, &unit_force_input_size);
  assert_non_null(config);
  assert_non_null(force_input);
  assert_non_null(unit_force_input);

  ParsedForceInput geometry;
  parse_force_input_geometry(force_input, force_input_size, &geometry);
  double stress_hartree_per_ang3_factor =
      1.0 / (BOHR_TO_ANGSTROM * BOHR_TO_ANGSTROM * BOHR_TO_ANGSTROM);

  double stress[9] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult raw_status = nwchemc_calculate_stress_from_config(
      config, config_size, force_input, force_input_size, stress, 9);
  if (!raw_status.ok)
    fail_msg("nwchemc_calculate_stress_from_config failed: %s",
             raw_status.message);
  assert_true(isfinite(raw_status.energy_h));
  for (int i = 0; i < 9; ++i) {
    if (!isfinite(stress[i]))
      fail_msg("non-finite stress[%d]", i);
  }

  double coordinate_stress[9] = {0.0, 0.0, 0.0, 0.0, 0.0,
                                 0.0, 0.0, 0.0, 0.0};
  NWChemCResult coordinate_status = nwchemc_stress_from_config(
      geometry.n_atoms, geometry.positions_ang, geometry.atomic_numbers,
      config, config_size, coordinate_stress);
  if (!coordinate_status.ok)
    fail_msg("nwchemc_stress_from_config failed: %s",
             coordinate_status.message);
  assert_true(isfinite(coordinate_status.energy_h));
  for (int i = 0; i < 9; ++i) {
    if (!isfinite(coordinate_stress[i]))
      fail_msg("non-finite coordinate_stress[%d]", i);
  }

  size_t result_capacity =
      nwchemc_stress_result_size_for_force_input(force_input, force_input_size);
  assert_true(result_capacity > 0);
  unsigned char *result_bytes = (unsigned char *)malloc(result_capacity);
  assert_non_null(result_bytes);
  size_t result_size = 0;
  NWChemCResult result_status = nwchemc_calculate_stress_result_from_config(
      config, config_size, force_input, force_input_size, result_bytes,
      result_capacity, &result_size);
  if (!result_status.ok)
    fail_msg("nwchemc_calculate_stress_result_from_config failed: %s",
             result_status.message);
  assert_true(isfinite(result_status.energy_h));
  assert_close_relative("stress result energy", 0, result_status.energy_h,
                        raw_status.energy_h);
  assert_int_equal(result_size, result_capacity);
  assert_potential_result_stress(result_bytes, result_size,
                                 result_status.energy_h, stress,
                                 stress_hartree_per_ang3_factor, 1.0e-5);

  double unit_stress[9] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult unit_raw_status = nwchemc_calculate_stress_from_config(
      config, config_size, unit_force_input, unit_force_input_size,
      unit_stress, 9);
  if (!unit_raw_status.ok)
    fail_msg("unit nwchemc_calculate_stress_from_config failed: %s",
             unit_raw_status.message);
  assert_true(isfinite(unit_raw_status.energy_h));

  size_t unit_result_capacity = nwchemc_stress_result_size_for_force_input(
      unit_force_input, unit_force_input_size);
  assert_true(unit_result_capacity > 0);
  unsigned char *unit_result_bytes =
      (unsigned char *)malloc(unit_result_capacity);
  assert_non_null(unit_result_bytes);
  size_t unit_result_size = 0;
  NWChemCResult unit_result_status =
      nwchemc_calculate_stress_result_from_config(
          config, config_size, unit_force_input, unit_force_input_size,
          unit_result_bytes, unit_result_capacity, &unit_result_size);
  if (!unit_result_status.ok)
    fail_msg("unit nwchemc_calculate_stress_result_from_config failed: %s",
             unit_result_status.message);
  assert_true(isfinite(unit_result_status.energy_h));
  assert_close_relative("unit stress result energy", 0,
                        unit_result_status.energy_h,
                        unit_raw_status.energy_h);
  assert_int_equal(unit_result_size, unit_result_capacity);
  assert_potential_result_stress(unit_result_bytes, unit_result_size,
                                 unit_result_status.energy_h * HARTREE_TO_EV,
                                 unit_stress, HARTREE_TO_EV, 5.0e-5);

  NWChemCSession *session =
      nwchemc_session_create_from_config(config, config_size);
  assert_non_null(session);

  double session_coordinate_stress[9] = {0.0, 0.0, 0.0, 0.0, 0.0,
                                         0.0, 0.0, 0.0, 0.0};
  NWChemCResult session_coordinate_status = nwchemc_session_stress(
      session, geometry.n_atoms, geometry.positions_ang,
      geometry.atomic_numbers, session_coordinate_stress);
  if (!session_coordinate_status.ok)
    fail_msg("nwchemc_session_stress failed: %s",
             session_coordinate_status.message);
  assert_true(isfinite(session_coordinate_status.energy_h));
  for (int i = 0; i < 9; ++i) {
    if (!isfinite(session_coordinate_stress[i]))
      fail_msg("non-finite session_coordinate_stress[%d]", i);
  }

  double session_stress[9] = {0.0, 0.0, 0.0, 0.0, 0.0,
                              0.0, 0.0, 0.0, 0.0};
  NWChemCResult session_raw_status = nwchemc_session_calculate_stress(
      session, force_input, force_input_size, session_stress, 9);
  if (!session_raw_status.ok)
    fail_msg("nwchemc_session_calculate_stress failed: %s",
             session_raw_status.message);
  assert_true(isfinite(session_raw_status.energy_h));
  for (int i = 0; i < 9; ++i) {
    if (!isfinite(session_stress[i]))
      fail_msg("non-finite session_stress[%d]", i);
  }

  memset(result_bytes, 0, result_capacity);
  result_size = 0;
  NWChemCResult session_result_status =
      nwchemc_session_calculate_stress_result(
          session, force_input, force_input_size, result_bytes,
          result_capacity, &result_size);
  if (!session_result_status.ok)
    fail_msg("nwchemc_session_calculate_stress_result failed: %s",
             session_result_status.message);
  assert_true(isfinite(session_result_status.energy_h));
  assert_close_relative("session stress result energy", 0,
                        session_result_status.energy_h,
                        session_raw_status.energy_h);
  assert_int_equal(result_size, result_capacity);
  assert_potential_result_stress(result_bytes, result_size,
                                 session_result_status.energy_h,
                                 session_stress,
                                 stress_hartree_per_ang3_factor, 1.0e-5);

  double session_unit_stress[9] = {0.0, 0.0, 0.0, 0.0, 0.0,
                                   0.0, 0.0, 0.0, 0.0};
  NWChemCResult session_unit_raw_status =
      nwchemc_session_calculate_stress(session, unit_force_input,
                                       unit_force_input_size,
                                       session_unit_stress, 9);
  if (!session_unit_raw_status.ok)
    fail_msg("unit nwchemc_session_calculate_stress failed: %s",
             session_unit_raw_status.message);
  assert_true(isfinite(session_unit_raw_status.energy_h));

  memset(unit_result_bytes, 0, unit_result_capacity);
  unit_result_size = 0;
  NWChemCResult session_unit_result_status =
      nwchemc_session_calculate_stress_result(
          session, unit_force_input, unit_force_input_size, unit_result_bytes,
          unit_result_capacity, &unit_result_size);
  if (!session_unit_result_status.ok)
    fail_msg("unit nwchemc_session_calculate_stress_result failed: %s",
             session_unit_result_status.message);
  assert_true(isfinite(session_unit_result_status.energy_h));
  assert_close_relative("session unit stress result energy", 0,
                        session_unit_result_status.energy_h,
                        session_unit_raw_status.energy_h);
  assert_int_equal(unit_result_size, unit_result_capacity);
  assert_potential_result_stress(
      unit_result_bytes, unit_result_size,
      session_unit_result_status.energy_h * HARTREE_TO_EV,
      session_unit_stress, HARTREE_TO_EV, 5.0e-5);

  nwchemc_session_destroy(session);
  free(unit_result_bytes);
  free(result_bytes);
  free(unit_force_input);
  free(force_input);
  free(config);
}

int main(int argc, char **argv) {
  if (argc != 4) {
    fprintf(stderr,
            "usage: %s potential-config.bin force-input.bin unit-force-input.bin\n",
            argv[0]);
    return 2;
  }
  g_config_path = argv[1];
  g_force_input_path = argv[2];
  g_unit_force_input_path = argv[3];

  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_pspw_stress_from_config),
  };
  return cmocka_run_group_tests(tests, setup_nwchem_dirs, teardown_nwchem);
}
