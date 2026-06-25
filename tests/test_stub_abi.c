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
  NWChemCResult result =
      nwchemc_energy_gradient(0, NULL, NULL, NULL, 0, NULL);
  assert_int_equal(result.ok, 0);
  NWChemCResult forces_result =
      nwchemc_energy_forces(0, NULL, NULL, NULL, 0, NULL);
  assert_int_equal(forces_result.ok, 0);
  NWChemCResult hessian_result =
      nwchemc_hessian(0, NULL, NULL, NULL, 0, NULL);
  assert_int_equal(hessian_result.ok, 0);
  nwchemc_finalize();
}

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_stub_reports_unavailable),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
