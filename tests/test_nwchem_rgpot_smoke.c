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
  if (ensure_dir(NWCHEMC_TEST_SCRATCH_DIR "-config") != 0)
    return 1;
  return ensure_dir(NWCHEMC_TEST_PERMANENT_DIR "-config");
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

static void assert_close_relative(const char *label, int index, double actual,
                                  double expected, double tolerance) {
  double scale = fmax(1.0, fmax(fabs(actual), fabs(expected)));
  if (fabs(actual - expected) > tolerance * scale)
    fail_msg("%s[%d] mismatch: got %.17g expected %.17g", label, index,
             actual, expected);
}

static double h2_bond_length(const double *positions_ang) {
  double dx = positions_ang[3] - positions_ang[0];
  double dy = positions_ang[4] - positions_ang[1];
  double dz = positions_ang[5] - positions_ang[2];
  return sqrt(dx * dx + dy * dy + dz * dz);
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

static void assert_potential_result_forces(const unsigned char *message,
                                           size_t message_size,
                                           double expected_energy) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);

  struct PotentialResult result;
  read_PotentialResult(&result, root);
  assert_true(isfinite(result.energy));
  assert_close(result.energy, expected_energy, 1.0e-12);

  assert_f64_list("force", result.forces, 6, NULL);

  capn_free(&arena);
}

static void assert_potential_result_energy_only(const unsigned char *message,
                                                size_t message_size,
                                                double expected_energy) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);

  struct PotentialResult result;
  read_PotentialResult(&result, root);
  assert_true(isfinite(result.energy));
  assert_close(result.energy, expected_energy, 1.0e-12);

  capn_resolve(&result.forces.p);
  if (result.forces.p.type != CAPN_NULL) {
    assert_int_equal(result.forces.p.type, CAPN_LIST);
    assert_int_equal(result.forces.p.datasz, 8);
    assert_int_equal(result.forces.p.len, 0);
  }

  capn_free(&arena);
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
  assert_true(isfinite(result.energy));
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
                                           double expected_energy,
                                           const double *expected_dipole) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);

  struct PotentialResult result;
  read_PotentialResult(&result, root);
  assert_true(isfinite(result.energy));
  assert_close(result.energy, expected_energy, 1.0e-12);

  double dipole[3];
  assert_f64_list("dipole", result.dipole, 3, dipole);
  for (int i = 0; i < 3; ++i) {
    assert_true(fabs(dipole[i]) < 1.0e-7);
    if (expected_dipole)
      assert_close_relative("PotentialResult.dipole", i, dipole[i],
                            expected_dipole[i], 1.0e-10);
  }

  capn_free(&arena);
}

static void assert_potential_result_polarizability(
    const unsigned char *message, size_t message_size, double expected_energy,
    const double *expected_polarizability) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);

  struct PotentialResult result;
  read_PotentialResult(&result, root);
  assert_true(isfinite(result.energy));
  assert_close(result.energy, expected_energy, 1.0e-12);

  double polarizability[12];
  double max_abs =
      assert_f64_list("polarizability", result.polarizability, 12,
                      polarizability);
  assert_true(max_abs > 1.0e-6);
  assert_true(polarizability[10] > 0.0);
  if (expected_polarizability) {
    for (int i = 0; i < 12; ++i)
      assert_close_relative("PotentialResult.polarizability", i,
                            polarizability[i], expected_polarizability[i],
                            1.0e-10);
  }

  capn_free(&arena);
}

static void assert_potential_result_quadrupole(
    const unsigned char *message, size_t message_size, double expected_energy,
    const double *expected_quadrupole) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);

  struct PotentialResult result;
  read_PotentialResult(&result, root);
  assert_true(isfinite(result.energy));
  assert_close(result.energy, expected_energy, 1.0e-12);

  double quadrupole[6];
  double max_abs =
      assert_f64_list("quadrupole", result.quadrupole, 6, quadrupole);
  assert_true(max_abs > 1.0e-6);
  double trace = quadrupole[0] + quadrupole[3] + quadrupole[5];
  assert_true(fabs(trace) < 1.0e-7 * fmax(1.0, max_abs));
  if (expected_quadrupole) {
    for (int i = 0; i < 6; ++i)
      assert_close_relative("PotentialResult.quadrupole", i, quadrupole[i],
                            expected_quadrupole[i], 1.0e-10);
  }

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
  assert_true(isfinite(result.energy));
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
  assert_true(isfinite(result.energy));
  assert_close(result.energy, expected_energy, 1.0e-12);

  double max_frequency_abs =
      assert_f64_list("frequency", result.frequencies, 6, NULL);
  assert_f64_list("intensity", result.intensities, 6, NULL);
  assert_true(max_frequency_abs > 1.0);

  capn_free(&arena);
}

static void assert_status_energy(const char *label, NWChemCResult status) {
  if (!status.ok)
    fail_msg("%s failed: %s", label, status.message);
  assert_true(isfinite(status.energy_h));
  assert_true(status.energy_h < -0.9);
}

static void assert_result_capacity(const char *label, size_t result_capacity) {
  if (result_capacity == 0)
    fail_msg("%s returned zero PotentialResult capacity", label);
}

static void test_rgpot_config_forceinput_result_carrier(void **state) {
  (void)state;
  assert_true(nwchemc_available());

  size_t config_size = 0;
  size_t force_input_size = 0;
  unsigned char *config = read_file(g_config_path, &config_size);
  unsigned char *force_input = read_file(g_force_input_path, &force_input_size);
  assert_non_null(config);
  assert_non_null(force_input);

  size_t result_capacity =
      nwchemc_potential_result_size_for_force_input(force_input,
                                                    force_input_size);
  assert_true(result_capacity > 0);
  unsigned char *result_bytes =
      (unsigned char *)malloc(result_capacity);
  assert_non_null(result_bytes);

  size_t result_size = 0;
  NWChemCResult one_shot = nwchemc_calculate_result_from_config(
      config, config_size, force_input, force_input_size, result_bytes,
      result_capacity, &result_size);
  assert_status_energy("nwchemc_calculate_result_from_config", one_shot);
  assert_int_equal(result_size, result_capacity);
  assert_potential_result_forces(result_bytes, result_size, one_shot.energy_h);

  NWChemCSession *session =
      nwchemc_session_create_from_config(config, config_size);
  assert_non_null(session);
  memset(result_bytes, 0, result_capacity);
  result_size = 0;
  NWChemCResult session_result = nwchemc_session_calculate_result(
      session, force_input, force_input_size, result_bytes, result_capacity,
      &result_size);
  assert_status_energy("nwchemc_session_calculate_result", session_result);
  assert_int_equal(result_size, result_capacity);
  assert_potential_result_forces(result_bytes, result_size,
                                 session_result.energy_h);

  nwchemc_session_destroy(session);
  free(result_bytes);
  free(force_input);
  free(config);
}

static void test_rgpot_config_named_result_carriers(void **state) {
  (void)state;
  assert_true(nwchemc_available());

  size_t config_size = 0;
  size_t force_input_size = 0;
  unsigned char *config = read_file(g_config_path, &config_size);
  unsigned char *force_input = read_file(g_force_input_path, &force_input_size);
  assert_non_null(config);
  assert_non_null(force_input);

  size_t energy_capacity =
      nwchemc_energy_result_size_for_force_input(force_input,
                                                 force_input_size);
  assert_result_capacity("nwchemc_energy_result_size_for_force_input",
                         energy_capacity);
  unsigned char *energy_bytes = (unsigned char *)malloc(energy_capacity);
  assert_non_null(energy_bytes);
  size_t energy_size = 0;
  NWChemCResult energy_status = nwchemc_calculate_energy_result_from_config(
      config, config_size, force_input, force_input_size, energy_bytes,
      energy_capacity, &energy_size);
  assert_status_energy("nwchemc_calculate_energy_result_from_config",
                       energy_status);
  assert_int_equal(energy_size, energy_capacity);
  assert_potential_result_energy_only(energy_bytes, energy_size,
                                      energy_status.energy_h);

  size_t forces_capacity =
      nwchemc_forces_result_size_for_force_input(force_input,
                                                 force_input_size);
  assert_result_capacity("nwchemc_forces_result_size_for_force_input",
                         forces_capacity);
  unsigned char *forces_bytes = (unsigned char *)malloc(forces_capacity);
  assert_non_null(forces_bytes);
  size_t forces_size = 0;
  NWChemCResult forces_status = nwchemc_calculate_forces_result_from_config(
      config, config_size, force_input, force_input_size, forces_bytes,
      forces_capacity, &forces_size);
  assert_status_energy("nwchemc_calculate_forces_result_from_config",
                       forces_status);
  assert_int_equal(forces_size, forces_capacity);
  assert_potential_result_forces(forces_bytes, forces_size,
                                 forces_status.energy_h);

  size_t hessian_capacity =
      nwchemc_hessian_result_size_for_force_input(force_input,
                                                  force_input_size);
  assert_result_capacity("nwchemc_hessian_result_size_for_force_input",
                         hessian_capacity);
  unsigned char *hessian_bytes = (unsigned char *)malloc(hessian_capacity);
  assert_non_null(hessian_bytes);
  size_t hessian_size = 0;
  NWChemCResult hessian_status = nwchemc_calculate_hessian_result_from_config(
      config, config_size, force_input, force_input_size, hessian_bytes,
      hessian_capacity, &hessian_size);
  assert_status_energy("nwchemc_calculate_hessian_result_from_config",
                       hessian_status);
  assert_int_equal(hessian_size, hessian_capacity);
  assert_potential_result_hessian(hessian_bytes, hessian_size,
                                  hessian_status.energy_h);

  size_t dipole_capacity =
      nwchemc_dipole_result_size_for_force_input(force_input,
                                                 force_input_size);
  assert_result_capacity("nwchemc_dipole_result_size_for_force_input",
                         dipole_capacity);
  double raw_dipole[3] = {0.0, 0.0, 0.0};
  NWChemCResult raw_dipole_status = nwchemc_calculate_dipole_from_config(
      config, config_size, force_input, force_input_size, raw_dipole, 3);
  assert_status_energy("nwchemc_calculate_dipole_from_config",
                       raw_dipole_status);
  unsigned char *dipole_bytes = (unsigned char *)malloc(dipole_capacity);
  assert_non_null(dipole_bytes);
  size_t dipole_size = 0;
  NWChemCResult dipole_status = nwchemc_calculate_dipole_result_from_config(
      config, config_size, force_input, force_input_size, dipole_bytes,
      dipole_capacity, &dipole_size);
  assert_status_energy("nwchemc_calculate_dipole_result_from_config",
                       dipole_status);
  assert_int_equal(dipole_size, dipole_capacity);
  assert_potential_result_dipole(dipole_bytes, dipole_size,
                                 dipole_status.energy_h, raw_dipole);

  size_t polarizability_capacity =
      nwchemc_polarizability_result_size_for_force_input(force_input,
                                                         force_input_size);
  assert_result_capacity("nwchemc_polarizability_result_size_for_force_input",
                         polarizability_capacity);
  double raw_polarizability[12] = {0.0};
  NWChemCResult raw_polarizability_status =
      nwchemc_calculate_polarizability_from_config(
          config, config_size, force_input, force_input_size,
          raw_polarizability, 12);
  assert_status_energy("nwchemc_calculate_polarizability_from_config",
                       raw_polarizability_status);
  unsigned char *polarizability_bytes =
      (unsigned char *)malloc(polarizability_capacity);
  assert_non_null(polarizability_bytes);
  size_t polarizability_size = 0;
  NWChemCResult polarizability_status =
      nwchemc_calculate_polarizability_result_from_config(
          config, config_size, force_input, force_input_size,
          polarizability_bytes, polarizability_capacity, &polarizability_size);
  assert_status_energy("nwchemc_calculate_polarizability_result_from_config",
                       polarizability_status);
  assert_int_equal(polarizability_size, polarizability_capacity);
  assert_potential_result_polarizability(
      polarizability_bytes, polarizability_size,
      polarizability_status.energy_h, raw_polarizability);

  size_t quadrupole_capacity =
      nwchemc_quadrupole_result_size_for_force_input(force_input,
                                                     force_input_size);
  assert_result_capacity("nwchemc_quadrupole_result_size_for_force_input",
                         quadrupole_capacity);
  double raw_quadrupole[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult raw_quadrupole_status =
      nwchemc_calculate_quadrupole_from_config(
          config, config_size, force_input, force_input_size, raw_quadrupole,
          6);
  assert_status_energy("nwchemc_calculate_quadrupole_from_config",
                       raw_quadrupole_status);
  unsigned char *quadrupole_bytes =
      (unsigned char *)malloc(quadrupole_capacity);
  assert_non_null(quadrupole_bytes);
  size_t quadrupole_size = 0;
  NWChemCResult quadrupole_status =
      nwchemc_calculate_quadrupole_result_from_config(
          config, config_size, force_input, force_input_size, quadrupole_bytes,
          quadrupole_capacity, &quadrupole_size);
  assert_status_energy("nwchemc_calculate_quadrupole_result_from_config",
                       quadrupole_status);
  assert_int_equal(quadrupole_size, quadrupole_capacity);
  assert_potential_result_quadrupole(quadrupole_bytes, quadrupole_size,
                                     quadrupole_status.energy_h,
                                     raw_quadrupole);

  size_t optimize_capacity =
      nwchemc_optimize_result_size_for_force_input(force_input,
                                                   force_input_size);
  assert_result_capacity("nwchemc_optimize_result_size_for_force_input",
                         optimize_capacity);
  unsigned char *optimize_bytes = (unsigned char *)malloc(optimize_capacity);
  assert_non_null(optimize_bytes);
  size_t optimize_size = 0;
  NWChemCResult optimize_status = nwchemc_calculate_optimize_result_from_config(
      config, config_size, force_input, force_input_size, optimize_bytes,
      optimize_capacity, &optimize_size);
  assert_status_energy("nwchemc_calculate_optimize_result_from_config",
                       optimize_status);
  assert_int_equal(optimize_size, optimize_capacity);
  assert_potential_result_optimized(optimize_bytes, optimize_size,
                                    optimize_status.energy_h);

  size_t frequencies_capacity =
      nwchemc_frequencies_result_size_for_force_input(force_input,
                                                      force_input_size);
  assert_result_capacity("nwchemc_frequencies_result_size_for_force_input",
                         frequencies_capacity);
  unsigned char *frequencies_bytes =
      (unsigned char *)malloc(frequencies_capacity);
  assert_non_null(frequencies_bytes);
  size_t frequencies_size = 0;
  NWChemCResult frequencies_status =
      nwchemc_calculate_frequencies_result_from_config(
          config, config_size, force_input, force_input_size, frequencies_bytes,
          frequencies_capacity, &frequencies_size);
  assert_status_energy("nwchemc_calculate_frequencies_result_from_config",
                       frequencies_status);
  assert_int_equal(frequencies_size, frequencies_capacity);
  assert_potential_result_frequencies(frequencies_bytes, frequencies_size,
                                      frequencies_status.energy_h);

  free(frequencies_bytes);
  free(optimize_bytes);
  free(quadrupole_bytes);
  free(polarizability_bytes);
  free(dipole_bytes);
  free(hessian_bytes);
  free(forces_bytes);
  free(energy_bytes);
  free(force_input);
  free(config);
}

static void test_rgpot_config_session_named_result_carriers(void **state) {
  (void)state;
  assert_true(nwchemc_available());

  size_t config_size = 0;
  size_t force_input_size = 0;
  unsigned char *config = read_file(g_config_path, &config_size);
  unsigned char *force_input = read_file(g_force_input_path, &force_input_size);
  assert_non_null(config);
  assert_non_null(force_input);

  NWChemCSession *session =
      nwchemc_session_create_from_config(config, config_size);
  assert_non_null(session);

  size_t energy_capacity =
      nwchemc_energy_result_size_for_force_input(force_input,
                                                 force_input_size);
  assert_result_capacity("nwchemc_energy_result_size_for_force_input",
                         energy_capacity);
  unsigned char *energy_bytes = (unsigned char *)malloc(energy_capacity);
  assert_non_null(energy_bytes);
  size_t energy_size = 0;
  NWChemCResult energy_status = nwchemc_session_calculate_energy_result(
      session, force_input, force_input_size, energy_bytes, energy_capacity,
      &energy_size);
  assert_status_energy("nwchemc_session_calculate_energy_result",
                       energy_status);
  assert_int_equal(energy_size, energy_capacity);
  assert_potential_result_energy_only(energy_bytes, energy_size,
                                      energy_status.energy_h);

  size_t forces_capacity =
      nwchemc_forces_result_size_for_force_input(force_input,
                                                 force_input_size);
  assert_result_capacity("nwchemc_forces_result_size_for_force_input",
                         forces_capacity);
  unsigned char *forces_bytes = (unsigned char *)malloc(forces_capacity);
  assert_non_null(forces_bytes);
  size_t forces_size = 0;
  NWChemCResult forces_status = nwchemc_session_calculate_forces_result(
      session, force_input, force_input_size, forces_bytes, forces_capacity,
      &forces_size);
  assert_status_energy("nwchemc_session_calculate_forces_result",
                       forces_status);
  assert_int_equal(forces_size, forces_capacity);
  assert_potential_result_forces(forces_bytes, forces_size,
                                 forces_status.energy_h);

  size_t hessian_capacity =
      nwchemc_hessian_result_size_for_force_input(force_input,
                                                  force_input_size);
  assert_result_capacity("nwchemc_hessian_result_size_for_force_input",
                         hessian_capacity);
  unsigned char *hessian_bytes = (unsigned char *)malloc(hessian_capacity);
  assert_non_null(hessian_bytes);
  size_t hessian_size = 0;
  NWChemCResult hessian_status = nwchemc_session_calculate_hessian_result(
      session, force_input, force_input_size, hessian_bytes, hessian_capacity,
      &hessian_size);
  assert_status_energy("nwchemc_session_calculate_hessian_result",
                       hessian_status);
  assert_int_equal(hessian_size, hessian_capacity);
  assert_potential_result_hessian(hessian_bytes, hessian_size,
                                  hessian_status.energy_h);

  size_t dipole_capacity =
      nwchemc_dipole_result_size_for_force_input(force_input,
                                                 force_input_size);
  assert_result_capacity("nwchemc_dipole_result_size_for_force_input",
                         dipole_capacity);
  double raw_dipole[3] = {0.0, 0.0, 0.0};
  NWChemCResult raw_dipole_status = nwchemc_session_calculate_dipole(
      session, force_input, force_input_size, raw_dipole, 3);
  assert_status_energy("nwchemc_session_calculate_dipole", raw_dipole_status);
  unsigned char *dipole_bytes = (unsigned char *)malloc(dipole_capacity);
  assert_non_null(dipole_bytes);
  size_t dipole_size = 0;
  NWChemCResult dipole_status = nwchemc_session_calculate_dipole_result(
      session, force_input, force_input_size, dipole_bytes, dipole_capacity,
      &dipole_size);
  assert_status_energy("nwchemc_session_calculate_dipole_result",
                       dipole_status);
  assert_int_equal(dipole_size, dipole_capacity);
  assert_potential_result_dipole(dipole_bytes, dipole_size,
                                 dipole_status.energy_h, raw_dipole);

  size_t polarizability_capacity =
      nwchemc_polarizability_result_size_for_force_input(force_input,
                                                         force_input_size);
  assert_result_capacity("nwchemc_polarizability_result_size_for_force_input",
                         polarizability_capacity);
  double raw_polarizability[12] = {0.0};
  NWChemCResult raw_polarizability_status =
      nwchemc_session_calculate_polarizability(
          session, force_input, force_input_size, raw_polarizability, 12);
  assert_status_energy("nwchemc_session_calculate_polarizability",
                       raw_polarizability_status);
  unsigned char *polarizability_bytes =
      (unsigned char *)malloc(polarizability_capacity);
  assert_non_null(polarizability_bytes);
  size_t polarizability_size = 0;
  NWChemCResult polarizability_status =
      nwchemc_session_calculate_polarizability_result(
          session, force_input, force_input_size, polarizability_bytes,
          polarizability_capacity, &polarizability_size);
  assert_status_energy("nwchemc_session_calculate_polarizability_result",
                       polarizability_status);
  assert_int_equal(polarizability_size, polarizability_capacity);
  assert_potential_result_polarizability(
      polarizability_bytes, polarizability_size,
      polarizability_status.energy_h, raw_polarizability);

  size_t quadrupole_capacity =
      nwchemc_quadrupole_result_size_for_force_input(force_input,
                                                     force_input_size);
  assert_result_capacity("nwchemc_quadrupole_result_size_for_force_input",
                         quadrupole_capacity);
  double raw_quadrupole[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult raw_quadrupole_status =
      nwchemc_session_calculate_quadrupole(
          session, force_input, force_input_size, raw_quadrupole, 6);
  assert_status_energy("nwchemc_session_calculate_quadrupole",
                       raw_quadrupole_status);
  unsigned char *quadrupole_bytes =
      (unsigned char *)malloc(quadrupole_capacity);
  assert_non_null(quadrupole_bytes);
  size_t quadrupole_size = 0;
  NWChemCResult quadrupole_status =
      nwchemc_session_calculate_quadrupole_result(
          session, force_input, force_input_size, quadrupole_bytes,
          quadrupole_capacity, &quadrupole_size);
  assert_status_energy("nwchemc_session_calculate_quadrupole_result",
                       quadrupole_status);
  assert_int_equal(quadrupole_size, quadrupole_capacity);
  assert_potential_result_quadrupole(quadrupole_bytes, quadrupole_size,
                                     quadrupole_status.energy_h,
                                     raw_quadrupole);

  size_t optimize_capacity =
      nwchemc_optimize_result_size_for_force_input(force_input,
                                                   force_input_size);
  assert_result_capacity("nwchemc_optimize_result_size_for_force_input",
                         optimize_capacity);
  unsigned char *optimize_bytes = (unsigned char *)malloc(optimize_capacity);
  assert_non_null(optimize_bytes);
  size_t optimize_size = 0;
  NWChemCResult optimize_status = nwchemc_session_calculate_optimize_result(
      session, force_input, force_input_size, optimize_bytes,
      optimize_capacity, &optimize_size);
  assert_status_energy("nwchemc_session_calculate_optimize_result",
                       optimize_status);
  assert_int_equal(optimize_size, optimize_capacity);
  assert_potential_result_optimized(optimize_bytes, optimize_size,
                                    optimize_status.energy_h);

  size_t frequencies_capacity =
      nwchemc_frequencies_result_size_for_force_input(force_input,
                                                      force_input_size);
  assert_result_capacity("nwchemc_frequencies_result_size_for_force_input",
                         frequencies_capacity);
  unsigned char *frequencies_bytes =
      (unsigned char *)malloc(frequencies_capacity);
  assert_non_null(frequencies_bytes);
  size_t frequencies_size = 0;
  NWChemCResult frequencies_status =
      nwchemc_session_calculate_frequencies_result(
          session, force_input, force_input_size, frequencies_bytes,
          frequencies_capacity, &frequencies_size);
  assert_status_energy("nwchemc_session_calculate_frequencies_result",
                       frequencies_status);
  assert_int_equal(frequencies_size, frequencies_capacity);
  assert_potential_result_frequencies(frequencies_bytes, frequencies_size,
                                      frequencies_status.energy_h);

  free(frequencies_bytes);
  free(optimize_bytes);
  free(quadrupole_bytes);
  free(polarizability_bytes);
  free(dipole_bytes);
  free(hessian_bytes);
  free(forces_bytes);
  free(energy_bytes);
  nwchemc_session_destroy(session);
  free(force_input);
  free(config);
}

static void test_rgpot_config_stress_rejects_short_buffers(void **state) {
  (void)state;
  assert_true(nwchemc_available());

  size_t config_size = 0;
  size_t force_input_size = 0;
  unsigned char *config = read_file(g_config_path, &config_size);
  unsigned char *force_input = read_file(g_force_input_path, &force_input_size);
  assert_non_null(config);
  assert_non_null(force_input);

  double short_stress[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult raw_config = nwchemc_calculate_stress_from_config(
      config, config_size, force_input, force_input_size, short_stress, 8);
  assert_int_equal(raw_config.ok, 0);

  size_t stress_capacity =
      nwchemc_stress_result_size_for_force_input(force_input,
                                                 force_input_size);
  assert_result_capacity("nwchemc_stress_result_size_for_force_input",
                         stress_capacity);
  assert_true(stress_capacity > 1);
  unsigned char *stress_bytes = (unsigned char *)malloc(stress_capacity - 1);
  assert_non_null(stress_bytes);

  size_t required_size = 0;
  NWChemCResult config_result =
      nwchemc_calculate_stress_result_from_config(
          config, config_size, force_input, force_input_size, stress_bytes,
          stress_capacity - 1, &required_size);
  assert_int_equal(config_result.ok, 0);
  assert_int_equal(required_size, stress_capacity);

  NWChemCSession *session =
      nwchemc_session_create_from_config(config, config_size);
  assert_non_null(session);

  NWChemCResult raw_session = nwchemc_session_calculate_stress(
      session, force_input, force_input_size, short_stress, 8);
  assert_int_equal(raw_session.ok, 0);

  required_size = 0;
  NWChemCResult session_result = nwchemc_session_calculate_stress_result(
      session, force_input, force_input_size, stress_bytes, stress_capacity - 1,
      &required_size);
  assert_int_equal(session_result.ok, 0);
  assert_int_equal(required_size, stress_capacity);

  nwchemc_session_destroy(session);
  free(stress_bytes);
  free(force_input);
  free(config);
}

static void test_rgpot_config_raw_forceinput_operations(void **state) {
  (void)state;
  assert_true(nwchemc_available());

  size_t config_size = 0;
  size_t force_input_size = 0;
  unsigned char *config = read_file(g_config_path, &config_size);
  unsigned char *force_input = read_file(g_force_input_path, &force_input_size);
  assert_non_null(config);
  assert_non_null(force_input);

  NWChemCResult energy_status = nwchemc_calculate_energy_from_config(
      config, config_size, force_input, force_input_size);
  assert_status_energy("nwchemc_calculate_energy_from_config", energy_status);

  double forces[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult forces_status = nwchemc_calculate_forces_from_config(
      config, config_size, force_input, force_input_size, forces, 6);
  assert_status_energy("nwchemc_calculate_forces_from_config", forces_status);
  for (int i = 0; i < 6; ++i) {
    if (!isfinite(forces[i]))
      fail_msg("non-finite raw force[%d]", i);
  }

  double hessian[36] = {0.0};
  NWChemCResult hessian_status = nwchemc_calculate_hessian_from_config(
      config, config_size, force_input, force_input_size, hessian, 36);
  assert_status_energy("nwchemc_calculate_hessian_from_config",
                       hessian_status);
  const int ncoord = 6;
  double max_hessian_abs = 0.0;
  for (int i = 0; i < ncoord; ++i) {
    for (int j = 0; j < ncoord; ++j) {
      double hij = hessian[i * ncoord + j];
      double hji = hessian[j * ncoord + i];
      if (!isfinite(hij))
        fail_msg("non-finite raw hessian[%d,%d]", i, j);
      double scale = fmax(1.0, fmax(fabs(hij), fabs(hji)));
      if (fabs(hij - hji) > 1e-7 * scale)
        fail_msg("raw hessian symmetry mismatch at %d,%d", i, j);
      max_hessian_abs = fmax(max_hessian_abs, fabs(hij));
    }
  }
  assert_true(max_hessian_abs >= 1e-6);

  double dipole[3] = {0.0, 0.0, 0.0};
  NWChemCResult dipole_status = nwchemc_calculate_dipole_from_config(
      config, config_size, force_input, force_input_size, dipole, 3);
  assert_status_energy("nwchemc_calculate_dipole_from_config", dipole_status);
  for (int i = 0; i < 3; ++i) {
    if (!isfinite(dipole[i]))
      fail_msg("non-finite raw dipole[%d]", i);
    assert_true(fabs(dipole[i]) < 1.0e-7);
  }

  double polarizability[12] = {0.0};
  NWChemCResult polarizability_status =
      nwchemc_calculate_polarizability_from_config(
          config, config_size, force_input, force_input_size, polarizability,
          12);
  assert_status_energy("nwchemc_calculate_polarizability_from_config",
                       polarizability_status);
  double max_polarizability_abs = 0.0;
  for (int i = 0; i < 12; ++i) {
    if (!isfinite(polarizability[i]))
      fail_msg("non-finite raw polarizability[%d]", i);
    max_polarizability_abs =
        fmax(max_polarizability_abs, fabs(polarizability[i]));
  }
  assert_true(max_polarizability_abs > 1.0e-6);
  assert_true(polarizability[10] > 0.0);

  double quadrupole[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult quadrupole_status =
      nwchemc_calculate_quadrupole_from_config(
          config, config_size, force_input, force_input_size, quadrupole, 6);
  assert_status_energy("nwchemc_calculate_quadrupole_from_config",
                       quadrupole_status);
  double max_quadrupole_abs = 0.0;
  for (int i = 0; i < 6; ++i) {
    if (!isfinite(quadrupole[i]))
      fail_msg("non-finite raw quadrupole[%d]", i);
    max_quadrupole_abs = fmax(max_quadrupole_abs, fabs(quadrupole[i]));
  }
  assert_true(max_quadrupole_abs > 1.0e-6);
  double trace = quadrupole[0] + quadrupole[3] + quadrupole[5];
  assert_true(fabs(trace) < 1.0e-7 * fmax(1.0, max_quadrupole_abs));

  double optimized_positions[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult optimize_status = nwchemc_calculate_optimize_from_config(
      config, config_size, force_input, force_input_size, optimized_positions,
      6);
  assert_status_energy("nwchemc_calculate_optimize_from_config",
                       optimize_status);
  for (int i = 0; i < 6; ++i) {
    if (!isfinite(optimized_positions[i]))
      fail_msg("non-finite raw optimized position[%d]", i);
  }
  double bond = h2_bond_length(optimized_positions);
  assert_true(bond > 0.5);
  assert_true(bond < 1.0);

  double frequencies[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  double intensities[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult frequencies_status =
      nwchemc_calculate_frequencies_from_config(
          config, config_size, force_input, force_input_size, frequencies, 6,
          intensities, 6);
  assert_status_energy("nwchemc_calculate_frequencies_from_config",
                       frequencies_status);
  double max_frequency_abs = 0.0;
  for (int i = 0; i < 6; ++i) {
    if (!isfinite(frequencies[i]))
      fail_msg("non-finite raw frequency[%d]", i);
    if (!isfinite(intensities[i]))
      fail_msg("non-finite raw intensity[%d]", i);
    max_frequency_abs = fmax(max_frequency_abs, fabs(frequencies[i]));
  }
  assert_true(max_frequency_abs > 1.0);

  free(force_input);
  free(config);
}

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s POTENTIAL_CONFIG_BIN FORCE_INPUT_BIN\n",
            argv[0]);
    return 2;
  }
  g_config_path = argv[1];
  g_force_input_path = argv[2];
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_rgpot_config_forceinput_result_carrier),
      cmocka_unit_test(test_rgpot_config_named_result_carriers),
      cmocka_unit_test(test_rgpot_config_session_named_result_carriers),
      cmocka_unit_test(test_rgpot_config_stress_rejects_short_buffers),
      cmocka_unit_test(test_rgpot_config_raw_forceinput_operations),
  };
  return cmocka_run_group_tests(tests, setup_nwchem_dirs, teardown_nwchem);
}
