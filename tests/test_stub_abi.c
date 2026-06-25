#include "nwchemc.h"

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>

#include <cmocka.h>

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
