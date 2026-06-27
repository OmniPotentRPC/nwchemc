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

static const char *g_params_path = NULL;
static const char *g_step_eq_path = NULL;
static const char *g_step_stretched_path = NULL;

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

static void assert_close(double actual, double expected, double tolerance) {
  assert_true(actual > expected - tolerance);
  assert_true(actual < expected + tolerance);
}

static double h2_bond_length(const double *positions_ang) {
  double dx = positions_ang[3] - positions_ang[0];
  double dy = positions_ang[4] - positions_ang[1];
  double dz = positions_ang[5] - positions_ang[2];
  return sqrt(dx * dx + dy * dy + dz * dz);
}

static void assert_finite_positions(const double *positions_ang,
                                    int position_count) {
  for (int i = 0; i < position_count; ++i) {
    if (!isfinite(positions_ang[i]))
      fail_msg("non-finite optimized position[%d]", i);
  }
}

static double assert_potential_result(const unsigned char *message,
                                      size_t message_size,
                                      double expected_energy) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);
  struct PotentialResult result;
  read_PotentialResult(&result, root);
  assert_close(result.energy, expected_energy, 1.0e-12);
  capn_resolve(&result.forces.p);
  assert_int_equal(result.forces.p.type, CAPN_LIST);
  assert_int_equal(result.forces.p.datasz, 8);
  assert_int_equal(result.forces.p.len, 6);
  for (int i = 0; i < result.forces.p.len; ++i) {
    double force = capn_to_f64(capn_get64(result.forces, i));
    if (!isfinite(force))
      fail_msg("non-finite force[%d]", i);
  }
  double energy = result.energy;
  capn_free(&arena);
  return energy;
}

static double assert_potential_result_energy_only(const unsigned char *message,
                                                  size_t message_size,
                                                  double expected_energy) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);
  struct PotentialResult result;
  read_PotentialResult(&result, root);
  assert_close(result.energy, expected_energy, 1.0e-12);
  capn_resolve(&result.forces.p);
  if (result.forces.p.type != CAPN_NULL) {
    assert_int_equal(result.forces.p.type, CAPN_LIST);
    assert_int_equal(result.forces.p.datasz, 8);
    assert_int_equal(result.forces.p.len, 0);
  }
  double energy = result.energy;
  capn_free(&arena);
  return energy;
}

static double assert_f64_list(const char *label, capn_list64 list,
                              int expected_len, double *values) {
  capn_resolve(&list.p);
  assert_int_equal(list.p.type, CAPN_LIST);
  assert_int_equal(list.p.datasz, 8);
  assert_int_equal(list.p.len, expected_len);

  double max_abs = 0.0;
  for (int i = 0; i < list.p.len; ++i) {
    double value = capn_to_f64(capn_get64(list, i));
    if (!isfinite(value))
      fail_msg("non-finite %s[%d]", label, i);
    if (values)
      values[i] = value;
    max_abs = fmax(max_abs, fabs(value));
  }
  return max_abs;
}

static void assert_potential_result_hessian(const unsigned char *message,
                                            size_t message_size,
                                            double expected_energy) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);

  struct PotentialResult result;
  read_PotentialResult(&result, root);
  assert_close(result.energy, expected_energy, 1.0e-12);

  const int ncoord = 6;
  double hessian[36];
  double max_abs = assert_f64_list("hessian", result.hessian, 36, hessian);
  for (int i = 0; i < ncoord; ++i) {
    for (int j = 0; j < ncoord; ++j) {
      double hij = hessian[i * ncoord + j];
      double hji = hessian[j * ncoord + i];
      double scale = fmax(1.0, fmax(fabs(hij), fabs(hji)));
      if (fabs(hij - hji) > 1e-7 * scale)
        fail_msg("hessian symmetry mismatch at %d,%d", i, j);
    }
  }
  assert_true(max_abs >= 1e-6);

  capn_free(&arena);
}

static void assert_potential_result_dipole(const unsigned char *message,
                                           size_t message_size,
                                           double expected_energy) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);

  struct PotentialResult result;
  read_PotentialResult(&result, root);
  assert_close(result.energy, expected_energy, 1.0e-12);

  double dipole[3];
  assert_f64_list("dipole", result.dipole, 3, dipole);
  for (int i = 0; i < 3; ++i)
    assert_true(fabs(dipole[i]) < 1.0e-7);

  capn_free(&arena);
}

static void assert_potential_result_quadrupole(const unsigned char *message,
                                               size_t message_size,
                                               double expected_energy) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);

  struct PotentialResult result;
  read_PotentialResult(&result, root);
  assert_close(result.energy, expected_energy, 1.0e-12);

  double quadrupole[6];
  double max_abs =
      assert_f64_list("quadrupole", result.quadrupole, 6, quadrupole);
  assert_true(max_abs > 1.0e-6);
  double trace = quadrupole[0] + quadrupole[3] + quadrupole[5];
  assert_true(fabs(trace) < 1.0e-7 * fmax(1.0, max_abs));

  capn_free(&arena);
}

static void assert_potential_result_optimized(const unsigned char *message,
                                              size_t message_size,
                                              double expected_energy) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);

  struct PotentialResult result;
  read_PotentialResult(&result, root);
  assert_close(result.energy, expected_energy, 1.0e-12);

  double optimized_positions[6];
  assert_f64_list("optimized position", result.optimizedPos, 6,
                  optimized_positions);
  double bond = h2_bond_length(optimized_positions);
  assert_true(bond > 0.5);
  assert_true(bond < 1.0);

  capn_free(&arena);
}

static void assert_potential_result_frequencies(const unsigned char *message,
                                                size_t message_size,
                                                double expected_energy) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);

  struct PotentialResult result;
  read_PotentialResult(&result, root);
  assert_close(result.energy, expected_energy, 1.0e-12);

  double max_frequency_abs =
      assert_f64_list("frequency", result.frequencies, 6, NULL);
  assert_f64_list("intensity", result.intensities, 6, NULL);
  assert_true(max_frequency_abs > 1.0);

  capn_free(&arena);
}

static NWChemCResult calculate_result_step(
    NWChemCSession *session, const unsigned char *step, size_t step_size,
    unsigned char *result_bytes, size_t result_capacity, size_t *result_size,
    double *result_energy) {
  *result_size = 0;
  NWChemCResult result =
      nwchemc_session_calculate_result(session, step, step_size, result_bytes,
                                       result_capacity, result_size);
  if (!result.ok)
    fail_msg("nwchemc_session_calculate_result failed: %s", result.message);
  *result_energy =
      assert_potential_result(result_bytes, *result_size, result.energy_h);
  return result;
}

static void test_session_calculate_result_accepts_multiple_steps(void **state) {
  (void)state;
  assert_true(nwchemc_available());

  size_t params_size = 0;
  size_t step_eq_size = 0;
  size_t step_stretched_size = 0;
  unsigned char *params = read_file(g_params_path, &params_size);
  unsigned char *step_eq = read_file(g_step_eq_path, &step_eq_size);
  unsigned char *step_stretched =
      read_file(g_step_stretched_path, &step_stretched_size);
  assert_non_null(params);
  assert_non_null(step_eq);
  assert_non_null(step_stretched);

  size_t expected_eq_size =
      nwchemc_potential_result_size_for_force_input(step_eq, step_eq_size);
  size_t expected_stretched_size = nwchemc_potential_result_size_for_force_input(
      step_stretched, step_stretched_size);
  assert_true(expected_eq_size > 0);
  assert_int_equal(expected_stretched_size, expected_eq_size);

  NWChemCSession *session = nwchemc_session_create(params, params_size);
  assert_non_null(session);

  unsigned char result_bytes[256];
  size_t eq_result_size = 0;
  size_t stretched_result_size = 0;
  size_t repeated_result_size = 0;
  double eq_energy = 0.0;
  double stretched_energy = 0.0;
  double repeated_energy = 0.0;

  NWChemCResult eq_status = calculate_result_step(
      session, step_eq, step_eq_size, result_bytes, sizeof(result_bytes),
      &eq_result_size, &eq_energy);
  NWChemCResult stretched_status = calculate_result_step(
      session, step_stretched, step_stretched_size, result_bytes,
      sizeof(result_bytes), &stretched_result_size, &stretched_energy);
  NWChemCResult repeated_status = calculate_result_step(
      session, step_eq, step_eq_size, result_bytes, sizeof(result_bytes),
      &repeated_result_size, &repeated_energy);

  assert_int_equal(eq_result_size, expected_eq_size);
  assert_int_equal(stretched_result_size, expected_stretched_size);
  assert_int_equal(repeated_result_size, expected_eq_size);
  assert_true(isfinite(eq_status.energy_h));
  assert_true(isfinite(stretched_status.energy_h));
  assert_true(isfinite(repeated_status.energy_h));
  assert_close(repeated_energy, eq_energy, 1.0e-10);
  assert_true(fabs(stretched_energy - eq_energy) > 1.0e-5);

  nwchemc_session_destroy(session);
  free(step_stretched);
  free(step_eq);
  free(params);
}

static void test_session_named_energy_and_forces_results_reuse_config(
    void **state) {
  (void)state;
  assert_true(nwchemc_available());

  size_t params_size = 0;
  size_t step_eq_size = 0;
  size_t step_stretched_size = 0;
  unsigned char *params = read_file(g_params_path, &params_size);
  unsigned char *step_eq = read_file(g_step_eq_path, &step_eq_size);
  unsigned char *step_stretched =
      read_file(g_step_stretched_path, &step_stretched_size);
  assert_non_null(params);
  assert_non_null(step_eq);
  assert_non_null(step_stretched);

  size_t energy_size_expected =
      nwchemc_energy_result_size_for_force_input(step_eq, step_eq_size);
  size_t force_size_expected =
      nwchemc_forces_result_size_for_force_input(step_eq, step_eq_size);
  assert_true(energy_size_expected > 0);
  assert_true(force_size_expected > energy_size_expected);
  assert_int_equal(nwchemc_energy_result_size_for_force_input(
                       step_stretched, step_stretched_size),
                   energy_size_expected);
  assert_int_equal(nwchemc_forces_result_size_for_force_input(
                       step_stretched, step_stretched_size),
                   force_size_expected);

  NWChemCSession *session = nwchemc_session_create(params, params_size);
  assert_non_null(session);

  unsigned char result_bytes[256];
  size_t result_size = 0;
  NWChemCResult eq_energy_status = nwchemc_session_calculate_energy_result(
      session, step_eq, step_eq_size, result_bytes, sizeof(result_bytes),
      &result_size);
  if (!eq_energy_status.ok)
    fail_msg("nwchemc_session_calculate_energy_result failed: %s",
             eq_energy_status.message);
  assert_int_equal(result_size, energy_size_expected);
  double eq_energy = assert_potential_result_energy_only(
      result_bytes, result_size, eq_energy_status.energy_h);

  memset(result_bytes, 0, sizeof(result_bytes));
  result_size = 0;
  NWChemCResult stretched_forces_status =
      nwchemc_session_calculate_forces_result(
          session, step_stretched, step_stretched_size, result_bytes,
          sizeof(result_bytes), &result_size);
  if (!stretched_forces_status.ok)
    fail_msg("nwchemc_session_calculate_forces_result failed: %s",
             stretched_forces_status.message);
  assert_int_equal(result_size, force_size_expected);
  double stretched_energy = assert_potential_result(
      result_bytes, result_size, stretched_forces_status.energy_h);
  assert_true(fabs(stretched_energy - eq_energy) > 1.0e-5);

  memset(result_bytes, 0, sizeof(result_bytes));
  result_size = 0;
  NWChemCResult repeated_energy_status =
      nwchemc_session_calculate_energy_result(
          session, step_eq, step_eq_size, result_bytes, sizeof(result_bytes),
          &result_size);
  if (!repeated_energy_status.ok)
    fail_msg("repeated nwchemc_session_calculate_energy_result failed: %s",
             repeated_energy_status.message);
  assert_int_equal(result_size, energy_size_expected);
  double repeated_energy = assert_potential_result_energy_only(
      result_bytes, result_size, repeated_energy_status.energy_h);
  assert_close(repeated_energy, eq_energy, 1.0e-10);

  nwchemc_session_destroy(session);
  free(step_stretched);
  free(step_eq);
  free(params);
}

static void test_session_named_property_and_structural_results(void **state) {
  (void)state;
  assert_true(nwchemc_available());

  size_t params_size = 0;
  size_t step_eq_size = 0;
  size_t step_stretched_size = 0;
  unsigned char *params = read_file(g_params_path, &params_size);
  unsigned char *step_eq = read_file(g_step_eq_path, &step_eq_size);
  unsigned char *step_stretched =
      read_file(g_step_stretched_path, &step_stretched_size);
  assert_non_null(params);
  assert_non_null(step_eq);
  assert_non_null(step_stretched);

  NWChemCSession *session = nwchemc_session_create(params, params_size);
  assert_non_null(session);

  size_t hessian_capacity =
      nwchemc_hessian_result_size_for_force_input(step_eq, step_eq_size);
  assert_true(hessian_capacity > 0);
  unsigned char *hessian_bytes = (unsigned char *)malloc(hessian_capacity);
  assert_non_null(hessian_bytes);
  size_t hessian_size = 0;
  NWChemCResult hessian_status = nwchemc_session_calculate_hessian_result(
      session, step_eq, step_eq_size, hessian_bytes, hessian_capacity,
      &hessian_size);
  if (!hessian_status.ok)
    fail_msg("nwchemc_session_calculate_hessian_result failed: %s",
             hessian_status.message);
  assert_int_equal(hessian_size, hessian_capacity);
  assert_potential_result_hessian(hessian_bytes, hessian_size,
                                  hessian_status.energy_h);

  size_t dipole_capacity =
      nwchemc_dipole_result_size_for_force_input(step_eq, step_eq_size);
  assert_true(dipole_capacity > 0);
  unsigned char *dipole_bytes = (unsigned char *)malloc(dipole_capacity);
  assert_non_null(dipole_bytes);
  size_t dipole_size = 0;
  NWChemCResult dipole_status = nwchemc_session_calculate_dipole_result(
      session, step_eq, step_eq_size, dipole_bytes, dipole_capacity,
      &dipole_size);
  if (!dipole_status.ok)
    fail_msg("nwchemc_session_calculate_dipole_result failed: %s",
             dipole_status.message);
  assert_int_equal(dipole_size, dipole_capacity);
  assert_potential_result_dipole(dipole_bytes, dipole_size,
                                 dipole_status.energy_h);

  size_t quadrupole_capacity =
      nwchemc_quadrupole_result_size_for_force_input(step_eq, step_eq_size);
  assert_true(quadrupole_capacity > 0);
  unsigned char *quadrupole_bytes =
      (unsigned char *)malloc(quadrupole_capacity);
  assert_non_null(quadrupole_bytes);
  size_t quadrupole_size = 0;
  NWChemCResult quadrupole_status =
      nwchemc_session_calculate_quadrupole_result(
          session, step_eq, step_eq_size, quadrupole_bytes,
          quadrupole_capacity, &quadrupole_size);
  if (!quadrupole_status.ok)
    fail_msg("nwchemc_session_calculate_quadrupole_result failed: %s",
             quadrupole_status.message);
  assert_int_equal(quadrupole_size, quadrupole_capacity);
  assert_potential_result_quadrupole(quadrupole_bytes, quadrupole_size,
                                     quadrupole_status.energy_h);

  size_t optimize_capacity =
      nwchemc_optimize_result_size_for_force_input(step_stretched,
                                                   step_stretched_size);
  assert_true(optimize_capacity > 0);
  unsigned char *optimize_bytes = (unsigned char *)malloc(optimize_capacity);
  assert_non_null(optimize_bytes);
  size_t optimize_size = 0;
  NWChemCResult optimize_status = nwchemc_session_calculate_optimize_result(
      session, step_stretched, step_stretched_size, optimize_bytes,
      optimize_capacity, &optimize_size);
  if (!optimize_status.ok)
    fail_msg("nwchemc_session_calculate_optimize_result failed: %s",
             optimize_status.message);
  assert_int_equal(optimize_size, optimize_capacity);
  assert_potential_result_optimized(optimize_bytes, optimize_size,
                                    optimize_status.energy_h);

  size_t frequencies_capacity =
      nwchemc_frequencies_result_size_for_force_input(step_eq, step_eq_size);
  assert_true(frequencies_capacity > 0);
  unsigned char *frequencies_bytes =
      (unsigned char *)malloc(frequencies_capacity);
  assert_non_null(frequencies_bytes);
  size_t frequencies_size = 0;
  NWChemCResult frequencies_status =
      nwchemc_session_calculate_frequencies_result(
          session, step_eq, step_eq_size, frequencies_bytes,
          frequencies_capacity, &frequencies_size);
  if (!frequencies_status.ok)
    fail_msg("nwchemc_session_calculate_frequencies_result failed: %s",
             frequencies_status.message);
  assert_int_equal(frequencies_size, frequencies_capacity);
  assert_potential_result_frequencies(frequencies_bytes, frequencies_size,
                                      frequencies_status.energy_h);

  free(frequencies_bytes);
  free(optimize_bytes);
  free(quadrupole_bytes);
  free(dipole_bytes);
  free(hessian_bytes);
  nwchemc_session_destroy(session);
  free(step_stretched);
  free(step_eq);
  free(params);
}

static void test_session_raw_property_and_structural_buffers(void **state) {
  (void)state;
  assert_true(nwchemc_available());

  size_t params_size = 0;
  size_t step_eq_size = 0;
  size_t step_stretched_size = 0;
  unsigned char *params = read_file(g_params_path, &params_size);
  unsigned char *step_eq = read_file(g_step_eq_path, &step_eq_size);
  unsigned char *step_stretched =
      read_file(g_step_stretched_path, &step_stretched_size);
  assert_non_null(params);
  assert_non_null(step_eq);
  assert_non_null(step_stretched);

  NWChemCSession *session = nwchemc_session_create(params, params_size);
  assert_non_null(session);

  const int ncoord = 6;
  double hessian[36];
  memset(hessian, 0, sizeof(hessian));
  NWChemCResult hessian_status = nwchemc_session_calculate_hessian(
      session, step_eq, step_eq_size, hessian, 36);
  if (!hessian_status.ok)
    fail_msg("nwchemc_session_calculate_hessian failed: %s",
             hessian_status.message);

  double max_hessian_abs = 0.0;
  for (int i = 0; i < ncoord; ++i) {
    for (int j = 0; j < ncoord; ++j) {
      double hij = hessian[i * ncoord + j];
      double hji = hessian[j * ncoord + i];
      if (!isfinite(hij))
        fail_msg("non-finite session hessian[%d,%d]", i, j);
      double scale = fmax(1.0, fmax(fabs(hij), fabs(hji)));
      if (fabs(hij - hji) > 1e-7 * scale)
        fail_msg("session hessian symmetry mismatch at %d,%d", i, j);
      max_hessian_abs = fmax(max_hessian_abs, fabs(hij));
    }
  }
  assert_true(max_hessian_abs >= 1e-6);

  double dipole[3] = {0.0, 0.0, 0.0};
  NWChemCResult dipole_status = nwchemc_session_calculate_dipole(
      session, step_eq, step_eq_size, dipole, 3);
  if (!dipole_status.ok)
    fail_msg("nwchemc_session_calculate_dipole failed: %s",
             dipole_status.message);
  for (int i = 0; i < 3; ++i) {
    if (!isfinite(dipole[i]))
      fail_msg("non-finite session dipole[%d]", i);
    assert_true(fabs(dipole[i]) < 1.0e-7);
  }

  double quadrupole[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult quadrupole_status = nwchemc_session_calculate_quadrupole(
      session, step_eq, step_eq_size, quadrupole, 6);
  if (!quadrupole_status.ok)
    fail_msg("nwchemc_session_calculate_quadrupole failed: %s",
             quadrupole_status.message);
  double max_quadrupole_abs = 0.0;
  for (int i = 0; i < 6; ++i) {
    if (!isfinite(quadrupole[i]))
      fail_msg("non-finite session quadrupole[%d]", i);
    max_quadrupole_abs = fmax(max_quadrupole_abs, fabs(quadrupole[i]));
  }
  assert_true(max_quadrupole_abs > 1.0e-6);
  double trace = quadrupole[0] + quadrupole[3] + quadrupole[5];
  assert_true(fabs(trace) < 1.0e-7 * fmax(1.0, max_quadrupole_abs));

  double optimized_positions[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult optimize_status = nwchemc_session_calculate_optimize(
      session, step_stretched, step_stretched_size, optimized_positions, 6);
  if (!optimize_status.ok)
    fail_msg("nwchemc_session_calculate_optimize failed: %s",
             optimize_status.message);
  assert_finite_positions(optimized_positions, 6);
  double optimized_bond = h2_bond_length(optimized_positions);
  assert_true(optimized_bond > 0.5);
  assert_true(optimized_bond < 1.0);

  double frequencies[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  double intensities[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult frequencies_status = nwchemc_session_calculate_frequencies(
      session, step_eq, step_eq_size, frequencies, 6, intensities, 6);
  if (!frequencies_status.ok)
    fail_msg("nwchemc_session_calculate_frequencies failed: %s",
             frequencies_status.message);
  double max_frequency_abs = 0.0;
  for (int i = 0; i < 6; ++i) {
    if (!isfinite(frequencies[i]))
      fail_msg("non-finite session frequency[%d]", i);
    if (!isfinite(intensities[i]))
      fail_msg("non-finite session intensity[%d]", i);
    max_frequency_abs = fmax(max_frequency_abs, fabs(frequencies[i]));
  }
  assert_true(max_frequency_abs > 1.0);

  nwchemc_session_destroy(session);
  free(step_stretched);
  free(step_eq);
  free(params);
}

static void test_calculate_hessian_and_dipole_one_shot(void **state) {
  (void)state;
  assert_true(nwchemc_available());

  size_t params_size = 0;
  size_t step_eq_size = 0;
  unsigned char *params = read_file(g_params_path, &params_size);
  unsigned char *step_eq = read_file(g_step_eq_path, &step_eq_size);
  assert_non_null(params);
  assert_non_null(step_eq);

  const int ncoord = 6;
  double hessian[36];
  memset(hessian, 0, sizeof(hessian));
  NWChemCResult hessian_status = nwchemc_calculate_hessian(
      params, params_size, step_eq, step_eq_size, hessian, 36);
  if (!hessian_status.ok)
    fail_msg("nwchemc_calculate_hessian failed: %s", hessian_status.message);

  double max_abs = 0.0;
  for (int i = 0; i < ncoord; ++i) {
    for (int j = 0; j < ncoord; ++j) {
      double hij = hessian[i * ncoord + j];
      double hji = hessian[j * ncoord + i];
      if (!isfinite(hij))
        fail_msg("non-finite one-shot hessian[%d,%d]", i, j);
      double scale = fmax(1.0, fmax(fabs(hij), fabs(hji)));
      if (fabs(hij - hji) > 1e-7 * scale)
        fail_msg("one-shot hessian symmetry mismatch at %d,%d", i, j);
      max_abs = fmax(max_abs, fabs(hij));
    }
  }
  assert_true(max_abs >= 1e-6);

  double dipole[3] = {0.0, 0.0, 0.0};
  NWChemCResult dipole_status = nwchemc_calculate_dipole(
      params, params_size, step_eq, step_eq_size, dipole, 3);
  if (!dipole_status.ok)
    fail_msg("nwchemc_calculate_dipole failed: %s", dipole_status.message);
  assert_true(isfinite(dipole_status.energy_h));
  for (int i = 0; i < 3; ++i) {
    if (!isfinite(dipole[i]))
      fail_msg("non-finite one-shot dipole[%d]", i);
    assert_true(fabs(dipole[i]) < 1.0e-7);
  }

  double quadrupole[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult quadrupole_status = nwchemc_calculate_quadrupole(
      params, params_size, step_eq, step_eq_size, quadrupole, 6);
  if (!quadrupole_status.ok)
    fail_msg("nwchemc_calculate_quadrupole failed: %s",
             quadrupole_status.message);
  assert_true(isfinite(quadrupole_status.energy_h));
  double max_quadrupole_abs = 0.0;
  for (int i = 0; i < 6; ++i) {
    if (!isfinite(quadrupole[i]))
      fail_msg("non-finite one-shot quadrupole[%d]", i);
    max_quadrupole_abs = fmax(max_quadrupole_abs, fabs(quadrupole[i]));
  }
  assert_true(max_quadrupole_abs > 1.0e-6);
  double trace = quadrupole[0] + quadrupole[3] + quadrupole[5];
  assert_true(fabs(trace) < 1.0e-7 * fmax(1.0, max_quadrupole_abs));

  double frequencies[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  double intensities[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult frequency_status = nwchemc_calculate_frequencies(
      params, params_size, step_eq, step_eq_size, frequencies, 6, intensities,
      6);
  if (!frequency_status.ok)
    fail_msg("nwchemc_calculate_frequencies failed: %s",
             frequency_status.message);
  double max_frequency_abs = 0.0;
  for (int i = 0; i < 6; ++i) {
    if (!isfinite(frequencies[i]))
      fail_msg("non-finite one-shot frequency[%d]", i);
    if (!isfinite(intensities[i]))
      fail_msg("non-finite one-shot intensity[%d]", i);
    max_frequency_abs = fmax(max_frequency_abs, fabs(frequencies[i]));
  }
  assert_true(max_frequency_abs > 1.0);

  free(step_eq);
  free(params);
}

static void test_calculate_optimize_returns_final_geometry(void **state) {
  (void)state;
  assert_true(nwchemc_available());

  size_t params_size = 0;
  size_t step_stretched_size = 0;
  unsigned char *params = read_file(g_params_path, &params_size);
  unsigned char *step_stretched =
      read_file(g_step_stretched_path, &step_stretched_size);
  assert_non_null(params);
  assert_non_null(step_stretched);

  double short_output[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult short_status = nwchemc_calculate_optimize(
      params, params_size, step_stretched, step_stretched_size, short_output,
      5);
  assert_int_equal(short_status.ok, 0);

  double optimized_positions[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult optimize_status = nwchemc_calculate_optimize(
      params, params_size, step_stretched, step_stretched_size,
      optimized_positions, 6);
  if (!optimize_status.ok)
    fail_msg("nwchemc_calculate_optimize failed: %s",
             optimize_status.message);
  assert_true(isfinite(optimize_status.energy_h));
  assert_true(optimize_status.energy_h < -0.9);
  assert_finite_positions(optimized_positions, 6);
  double one_shot_bond = h2_bond_length(optimized_positions);
  assert_true(one_shot_bond > 0.5);
  assert_true(one_shot_bond < 1.0);
  assert_true(one_shot_bond < 1.3);

  NWChemCSession *session = nwchemc_session_create(params, params_size);
  assert_non_null(session);
  double session_positions[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult session_status = nwchemc_session_calculate_optimize(
      session, step_stretched, step_stretched_size, session_positions, 6);
  if (!session_status.ok)
    fail_msg("nwchemc_session_calculate_optimize failed: %s",
             session_status.message);
  assert_true(isfinite(session_status.energy_h));
  assert_true(session_status.energy_h < -0.9);
  assert_finite_positions(session_positions, 6);
  double session_bond = h2_bond_length(session_positions);
  assert_true(session_bond > 0.5);
  assert_true(session_bond < 1.0);
  assert_true(session_bond < 1.3);
  assert_close(session_bond, one_shot_bond, 1.0e-5);

  nwchemc_session_destroy(session);
  free(step_stretched);
  free(params);
}

int main(int argc, char **argv) {
  if (argc != 4) {
    fprintf(stderr,
            "usage: %s PARAMS_BIN FORCE_STEP_EQ_BIN FORCE_STEP_STRETCHED_BIN\n",
            argv[0]);
    return 2;
  }
  g_params_path = argv[1];
  g_step_eq_path = argv[2];
  g_step_stretched_path = argv[3];
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_session_calculate_result_accepts_multiple_steps),
      cmocka_unit_test(
          test_session_named_energy_and_forces_results_reuse_config),
      cmocka_unit_test(test_session_named_property_and_structural_results),
      cmocka_unit_test(test_session_raw_property_and_structural_buffers),
      cmocka_unit_test(test_calculate_hessian_and_dipole_one_shot),
      cmocka_unit_test(test_calculate_optimize_returns_final_geometry),
  };
  return cmocka_run_group_tests(tests, setup_nwchem_dirs, teardown_nwchem);
}
