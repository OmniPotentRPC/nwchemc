#include "nwchemc.h"
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

static const double BOHR_TO_ANGSTROM = 0.529177210903;

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
  if (ensure_dir(NWCHEMC_TEST_SCRATCH_DIR "-pspw-forces") != 0)
    return 1;
  return ensure_dir(NWCHEMC_TEST_PERMANENT_DIR "-pspw-forces");
}

static int teardown_nwchem(void **state) {
  (void)state;
  nwchemc_finalize();
  return 0;
}

static void assert_close_force(const char *label, int index, double actual,
                               double expected) {
  double scale = fmax(1.0, fmax(fabs(actual), fabs(expected)));
  if (fabs(actual - expected) > 1.0e-5 * scale)
    fail_msg("%s[%d] mismatch: got %.17g expected %.17g", label, index,
             actual, expected);
}

static void assert_close_energy(const char *label, double actual,
                                double expected) {
  double scale = fmax(1.0, fmax(fabs(actual), fabs(expected)));
  if (fabs(actual - expected) > 1.0e-5 * scale)
    fail_msg("%s mismatch: got %.17g expected %.17g", label, actual, expected);
}

static void assert_potential_result_forces(const unsigned char *message,
                                           size_t message_size,
                                           double native_energy_h,
                                           const double *native_forces) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);

  struct PotentialResult result;
  read_PotentialResult(&result, root);
  assert_true(isfinite(result.energy));
  assert_close_energy("PotentialResult.energy", result.energy,
                      native_energy_h);

  capn_resolve(&result.forces.p);
  assert_int_equal(result.forces.p.type, CAPN_LIST);
  assert_int_equal(result.forces.p.datasz, 8);
  assert_int_equal(result.forces.p.len, 6);
  for (int i = 0; i < result.forces.p.len; ++i) {
    double force = capn_to_f64(capn_get64(result.forces, i));
    if (!isfinite(force))
      fail_msg("non-finite force[%d]", i);
    if (native_forces) {
      double expected = native_forces[i] / BOHR_TO_ANGSTROM;
      assert_close_force("PotentialResult.force", i, force, expected);
    }
  }

  capn_free(&arena);
}

static void assert_potential_result_energy_only(const unsigned char *message,
                                                size_t message_size,
                                                double native_energy_h) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);

  struct PotentialResult result;
  read_PotentialResult(&result, root);
  assert_true(isfinite(result.energy));
  assert_close_energy("PotentialResult.energy", result.energy,
                      native_energy_h);

  capn_resolve(&result.forces.p);
  if (result.forces.p.type != CAPN_NULL) {
    assert_int_equal(result.forces.p.type, CAPN_LIST);
    assert_int_equal(result.forces.p.datasz, 8);
    assert_int_equal(result.forces.p.len, 0);
  }

  capn_free(&arena);
}

static void assert_force_buffer(const char *label, const double *forces,
                                int force_count) {
  for (int i = 0; i < force_count; ++i) {
    if (!isfinite(forces[i]))
      fail_msg("non-finite %s[%d]", label, i);
  }
}

static void assert_matching_native_forces(const double *actual,
                                          const double *expected,
                                          int force_count) {
  for (int i = 0; i < force_count; ++i)
    assert_close_force("native force", i, actual[i], expected[i]);
}

static void test_pspw_pseudopotential_forces_result(void **state) {
  (void)state;
  assert_true(nwchemc_available());

  size_t config_size = 0;
  size_t force_input_size = 0;
  unsigned char *config = read_file(g_config_path, &config_size);
  unsigned char *force_input = read_file(g_force_input_path, &force_input_size);
  assert_non_null(config);
  assert_non_null(force_input);

  NWChemCResult raw_energy_status = nwchemc_calculate_energy_from_config(
      config, config_size, force_input, force_input_size);
  if (!raw_energy_status.ok)
    fail_msg("nwchemc_calculate_energy_from_config failed: %s",
             raw_energy_status.message);
  assert_true(isfinite(raw_energy_status.energy_h));

  double raw_forces[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult raw_status = nwchemc_calculate_forces_from_config(
      config, config_size, force_input, force_input_size, raw_forces, 6);
  if (!raw_status.ok)
    fail_msg("nwchemc_calculate_forces_from_config failed: %s",
             raw_status.message);
  assert_true(isfinite(raw_status.energy_h));
  assert_close_energy("raw forces energy", raw_status.energy_h,
                      raw_energy_status.energy_h);
  assert_force_buffer("raw force", raw_forces, 6);

  size_t energy_capacity =
      nwchemc_energy_result_size_for_force_input(force_input,
                                                 force_input_size);
  assert_true(energy_capacity > 0);
  unsigned char *energy_bytes = (unsigned char *)malloc(energy_capacity);
  assert_non_null(energy_bytes);
  size_t energy_size = 0;
  NWChemCResult energy_status = nwchemc_calculate_energy_result_from_config(
      config, config_size, force_input, force_input_size, energy_bytes,
      energy_capacity, &energy_size);
  if (!energy_status.ok)
    fail_msg("nwchemc_calculate_energy_result_from_config failed: %s",
             energy_status.message);
  assert_true(isfinite(energy_status.energy_h));
  assert_close_energy("energy result status", energy_status.energy_h,
                      raw_energy_status.energy_h);
  assert_int_equal(energy_size, energy_capacity);
  assert_potential_result_energy_only(energy_bytes, energy_size,
                                      energy_status.energy_h);

  size_t result_capacity =
      nwchemc_potential_result_size_for_force_input(force_input,
                                                    force_input_size);
  assert_true(result_capacity > 0);
  unsigned char *result_bytes = (unsigned char *)malloc(result_capacity);
  assert_non_null(result_bytes);
  size_t result_size = 0;
  NWChemCResult result_status = nwchemc_calculate_result_from_config(
      config, config_size, force_input, force_input_size, result_bytes,
      result_capacity, &result_size);
  if (!result_status.ok)
    fail_msg("nwchemc_calculate_result_from_config failed: %s",
             result_status.message);
  assert_true(isfinite(result_status.energy_h));
  assert_close_energy("compatibility result status", result_status.energy_h,
                      raw_energy_status.energy_h);
  assert_int_equal(result_size, result_capacity);
  assert_potential_result_forces(result_bytes, result_size,
                                 result_status.energy_h, raw_forces);

  size_t forces_capacity =
      nwchemc_forces_result_size_for_force_input(force_input,
                                                 force_input_size);
  assert_true(forces_capacity > 0);
  unsigned char *forces_bytes = (unsigned char *)malloc(forces_capacity);
  assert_non_null(forces_bytes);
  size_t forces_size = 0;
  NWChemCResult one_shot_forces_status =
      nwchemc_calculate_forces_result_from_config(
          config, config_size, force_input, force_input_size, forces_bytes,
          forces_capacity, &forces_size);
  if (!one_shot_forces_status.ok)
    fail_msg("nwchemc_calculate_forces_result_from_config failed: %s",
             one_shot_forces_status.message);
  assert_true(isfinite(one_shot_forces_status.energy_h));
  assert_close_energy("forces result status",
                      one_shot_forces_status.energy_h,
                      raw_energy_status.energy_h);
  assert_int_equal(forces_size, forces_capacity);
  assert_potential_result_forces(forces_bytes, forces_size,
                                 one_shot_forces_status.energy_h,
                                 raw_forces);

  NWChemCSession *session =
      nwchemc_session_create_from_config(config, config_size);
  assert_non_null(session);

  NWChemCResult session_raw_energy_status =
      nwchemc_session_calculate_energy(session, force_input, force_input_size);
  if (!session_raw_energy_status.ok)
    fail_msg("nwchemc_session_calculate_energy failed: %s",
             session_raw_energy_status.message);
  assert_true(isfinite(session_raw_energy_status.energy_h));
  assert_close_energy("session raw energy", session_raw_energy_status.energy_h,
                      raw_energy_status.energy_h);

  double session_raw_forces[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult session_raw_status = nwchemc_session_calculate_forces(
      session, force_input, force_input_size, session_raw_forces, 6);
  if (!session_raw_status.ok)
    fail_msg("nwchemc_session_calculate_forces failed: %s",
             session_raw_status.message);
  assert_true(isfinite(session_raw_status.energy_h));
  assert_close_energy("session raw forces energy", session_raw_status.energy_h,
                      raw_energy_status.energy_h);
  assert_force_buffer("session raw force", session_raw_forces, 6);
  assert_matching_native_forces(session_raw_forces, raw_forces, 6);

  memset(energy_bytes, 0, energy_capacity);
  energy_size = 0;
  NWChemCResult session_energy_status =
      nwchemc_session_calculate_energy_result(
          session, force_input, force_input_size, energy_bytes,
          energy_capacity, &energy_size);
  if (!session_energy_status.ok)
    fail_msg("nwchemc_session_calculate_energy_result failed: %s",
             session_energy_status.message);
  assert_true(isfinite(session_energy_status.energy_h));
  assert_close_energy("session energy result status",
                      session_energy_status.energy_h,
                      raw_energy_status.energy_h);
  assert_int_equal(energy_size, energy_capacity);
  assert_potential_result_energy_only(energy_bytes, energy_size,
                                      session_energy_status.energy_h);

  memset(result_bytes, 0, result_capacity);
  result_size = 0;
  NWChemCResult session_result_status = nwchemc_session_calculate_result(
      session, force_input, force_input_size, result_bytes, result_capacity,
      &result_size);
  if (!session_result_status.ok)
    fail_msg("nwchemc_session_calculate_result failed: %s",
             session_result_status.message);
  assert_true(isfinite(session_result_status.energy_h));
  assert_close_energy("session compatibility result status",
                      session_result_status.energy_h,
                      raw_energy_status.energy_h);
  assert_int_equal(result_size, result_capacity);
  assert_potential_result_forces(result_bytes, result_size,
                                 session_result_status.energy_h,
                                 session_raw_forces);

  memset(forces_bytes, 0, forces_capacity);
  forces_size = 0;
  NWChemCResult session_forces_status =
      nwchemc_session_calculate_forces_result(
          session, force_input, force_input_size, forces_bytes,
          forces_capacity, &forces_size);
  if (!session_forces_status.ok)
    fail_msg("nwchemc_session_calculate_forces_result failed: %s",
             session_forces_status.message);
  assert_true(isfinite(session_forces_status.energy_h));
  assert_close_energy("session forces result status",
                      session_forces_status.energy_h,
                      raw_energy_status.energy_h);
  assert_int_equal(forces_size, forces_capacity);
  assert_potential_result_forces(forces_bytes, forces_size,
                                 session_forces_status.energy_h,
                                 session_raw_forces);

  nwchemc_session_destroy(session);
  free(forces_bytes);
  free(result_bytes);
  free(energy_bytes);
  free(force_input);
  free(config);
}

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s potential-config.bin force-input.bin\n",
            argv[0]);
    return 2;
  }
  g_config_path = argv[1];
  g_force_input_path = argv[2];

  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_pspw_pseudopotential_forces_result),
  };
  return cmocka_run_group_tests(tests, setup_nwchem_dirs, teardown_nwchem);
}
