#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

#include <cmocka.h>

extern void nwchemc_test_nwpw_rtdb(int *result);

static void test_nwpw_controls_reach_rtdb(void **state) {
  (void)state;
  int result = -1;
  nwchemc_test_nwpw_rtdb(&result);
  assert_int_equal(result, 0);
}

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_nwpw_controls_reach_rtdb),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
