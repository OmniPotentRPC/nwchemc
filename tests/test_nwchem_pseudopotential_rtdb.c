#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include <cmocka.h>

extern void nwchemc_test_pseudopotential_rtdb(int *result);
extern void nwchemc_test_pseudopotential_text_rtdb(int *result);

static void test_pseudopotential_entries_reach_rtdb(void **state) {
  (void)state;
  int result = -1;
  nwchemc_test_pseudopotential_rtdb(&result);
  assert_int_equal(result, 0);
}

static void test_pseudopotential_text_entries_reach_rtdb(void **state) {
  (void)state;
  int result = -1;
  nwchemc_test_pseudopotential_text_rtdb(&result);
  assert_int_equal(result, 0);
}

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_pseudopotential_entries_reach_rtdb),
      cmocka_unit_test(test_pseudopotential_text_entries_reach_rtdb),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
