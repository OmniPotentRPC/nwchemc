#include "nwchemc.h"

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>

#include <cmocka.h>

#if defined(__GNUC__) || defined(__clang__)
#define NWCHEMC_TEST_WEAK __attribute__((weak))
#else
#define NWCHEMC_TEST_WEAK
#endif

extern NWChemCResult nwchemc_dipole(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *dipole_au) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_quadrupole(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *quadrupole_au) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_optimize(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *optimized_positions_ang) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_session_dipole(
    NWChemCSession *session, int n_atoms, const double *positions_ang,
    const int *atomic_numbers, double *dipole_au) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_session_quadrupole(
    NWChemCSession *session, int n_atoms, const double *positions_ang,
    const int *atomic_numbers, double *quadrupole_au) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_session_optimize(
    NWChemCSession *session, int n_atoms, const double *positions_ang,
    const int *atomic_numbers, double *optimized_positions_ang)
    NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_session_calculate_dipole(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *dipole_au,
    size_t dipole_len) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_session_calculate_quadrupole(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *quadrupole_au,
    size_t quadrupole_len) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_session_calculate_optimize(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *optimized_positions_ang,
    size_t optimized_positions_len) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_hessian(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *hessian_h_bohr2, size_t hessian_len) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_dipole(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *dipole_au, size_t dipole_len) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_quadrupole(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *quadrupole_au, size_t quadrupole_len) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_optimize(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *optimized_positions_ang, size_t optimized_positions_len)
    NWCHEMC_TEST_WEAK;

static void test_stub_reports_unavailable(void **state) {
  (void)state;
  assert_int_equal(nwchemc_available(), 0);
  const char *version = nwchemc_version();
  assert_non_null(version);
  assert_non_null(strstr(version, "stub"));
  assert_int_not_equal(nwchemc_set_params(NULL, 0), 0);
  NWChemCResult energy_result = nwchemc_energy(0, NULL, NULL, NULL, 0);
  assert_int_equal(energy_result.ok, 0);
  NWChemCResult result =
      nwchemc_energy_gradient(0, NULL, NULL, NULL, 0, NULL);
  assert_int_equal(result.ok, 0);
  NWChemCResult forces_result =
      nwchemc_energy_forces(0, NULL, NULL, NULL, 0, NULL);
  assert_int_equal(forces_result.ok, 0);
  NWChemCResult hessian_result =
      nwchemc_hessian(0, NULL, NULL, NULL, 0, NULL);
  assert_int_equal(hessian_result.ok, 0);
  assert_true(nwchemc_dipole != NULL);
  double dipole_au[3] = {0.0, 0.0, 0.0};
  NWChemCResult dipole_result =
      nwchemc_dipole(0, NULL, NULL, NULL, 0, dipole_au);
  assert_int_equal(dipole_result.ok, 0);
  assert_true(nwchemc_quadrupole != NULL);
  double quadrupole_au[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult quadrupole_result =
      nwchemc_quadrupole(0, NULL, NULL, NULL, 0, quadrupole_au);
  assert_int_equal(quadrupole_result.ok, 0);
  assert_true(nwchemc_optimize != NULL);
  double optimized_positions_ang[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult optimize_result =
      nwchemc_optimize(0, NULL, NULL, NULL, 0, optimized_positions_ang);
  assert_int_equal(optimize_result.ok, 0);
  assert_null(nwchemc_session_create(NULL, 0));
  assert_int_not_equal(nwchemc_session_set_params(NULL, NULL, 0), 0);
  nwchemc_session_destroy(NULL);
  NWChemCResult session_energy =
      nwchemc_session_energy(NULL, 0, NULL, NULL);
  assert_int_equal(session_energy.ok, 0);
  NWChemCResult session_gradient =
      nwchemc_session_energy_gradient(NULL, 0, NULL, NULL, NULL);
  assert_int_equal(session_gradient.ok, 0);
  NWChemCResult session_forces =
      nwchemc_session_energy_forces(NULL, 0, NULL, NULL, NULL);
  assert_int_equal(session_forces.ok, 0);
  assert_true(nwchemc_session_dipole != NULL);
  NWChemCResult session_dipole =
      nwchemc_session_dipole(NULL, 0, NULL, NULL, dipole_au);
  assert_int_equal(session_dipole.ok, 0);
  assert_true(nwchemc_session_quadrupole != NULL);
  NWChemCResult session_quadrupole =
      nwchemc_session_quadrupole(NULL, 0, NULL, NULL, quadrupole_au);
  assert_int_equal(session_quadrupole.ok, 0);
  assert_true(nwchemc_session_optimize != NULL);
  NWChemCResult session_optimize =
      nwchemc_session_optimize(NULL, 0, NULL, NULL, optimized_positions_ang);
  assert_int_equal(session_optimize.ok, 0);
  NWChemCResult session_step =
      nwchemc_session_calculate_forces(NULL, NULL, 0, NULL, 0);
  assert_int_equal(session_step.ok, 0);
  NWChemCResult session_result =
      nwchemc_session_calculate_result(NULL, NULL, 0, NULL, 0, NULL);
  assert_int_equal(session_result.ok, 0);
  NWChemCResult capnp_result =
      nwchemc_calculate_result(NULL, 0, NULL, 0, NULL, 0, NULL);
  assert_int_equal(capnp_result.ok, 0);
  assert_int_equal(nwchemc_potential_result_size_for_force_input(NULL, 0), 0);
  NWChemCResult session_step_hessian =
      nwchemc_session_calculate_hessian(NULL, NULL, 0, NULL, 0);
  assert_int_equal(session_step_hessian.ok, 0);
  assert_true(nwchemc_calculate_hessian != NULL);
  double hessian_h_bohr2[1] = {0.0};
  NWChemCResult one_shot_hessian =
      nwchemc_calculate_hessian(NULL, 0, NULL, 0, hessian_h_bohr2, 1);
  assert_int_equal(one_shot_hessian.ok, 0);
  assert_true(nwchemc_session_calculate_dipole != NULL);
  NWChemCResult session_step_dipole =
      nwchemc_session_calculate_dipole(NULL, NULL, 0, dipole_au, 3);
  assert_int_equal(session_step_dipole.ok, 0);
  assert_true(nwchemc_session_calculate_quadrupole != NULL);
  NWChemCResult session_step_quadrupole =
      nwchemc_session_calculate_quadrupole(NULL, NULL, 0, quadrupole_au, 6);
  assert_int_equal(session_step_quadrupole.ok, 0);
  assert_true(nwchemc_session_calculate_optimize != NULL);
  NWChemCResult session_step_optimize = nwchemc_session_calculate_optimize(
      NULL, NULL, 0, optimized_positions_ang, 6);
  assert_int_equal(session_step_optimize.ok, 0);
  assert_true(nwchemc_calculate_dipole != NULL);
  NWChemCResult one_shot_dipole =
      nwchemc_calculate_dipole(NULL, 0, NULL, 0, dipole_au, 3);
  assert_int_equal(one_shot_dipole.ok, 0);
  assert_true(nwchemc_calculate_quadrupole != NULL);
  NWChemCResult one_shot_quadrupole =
      nwchemc_calculate_quadrupole(NULL, 0, NULL, 0, quadrupole_au, 6);
  assert_int_equal(one_shot_quadrupole.ok, 0);
  assert_true(nwchemc_calculate_optimize != NULL);
  NWChemCResult one_shot_optimize =
      nwchemc_calculate_optimize(NULL, 0, NULL, 0, optimized_positions_ang, 6);
  assert_int_equal(one_shot_optimize.ok, 0);
  NWChemCResult session_hessian =
      nwchemc_session_hessian(NULL, 0, NULL, NULL, NULL);
  assert_int_equal(session_hessian.ok, 0);
  nwchemc_finalize();
}

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_stub_reports_unavailable),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
