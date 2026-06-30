#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include <cmocka.h>

extern void nwchemc_test_geometry_basis_rtdb(int *result);

/* Large-H store plus Cl2 lanl2dz_ecp ECP association in one GA session. */
static void test_geometry_basis_and_ecp_association(void **state) {
  (void)state;
  int result = -1;
  nwchemc_test_geometry_basis_rtdb(&result);
  assert_int_equal(result, 0);
}

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_geometry_basis_and_ecp_association),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
