#include "nwchemc_features.h"

#include <cmocka.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static void test_feature_count_nonzero(void **state) {
  (void)state;
  assert_true(nwchemc_feature_count() > 100);
}

static void test_module_count(void **state) {
  (void)state;
  assert_int_equal((int)nwchemc_module_feature_count(), 88);
}

static void test_find_module_dft(void **state) {
  (void)state;
  const NWChemCFeatureEntry *e = nwchemc_feature_find("module.dft");
  assert_non_null(e);
  assert_int_equal(e->klass, NWCHEMC_FEATURE_MODULE);
  assert_string_equal(e->nwchem_text_or_role, "dft");
}

static void test_find_stanza_pseudopotential(void **state) {
  (void)state;
  const NWChemCFeatureEntry *e = nwchemc_feature_find("stanza.pseudopotential");
  assert_non_null(e);
  assert_int_equal(e->klass, NWCHEMC_FEATURE_STANZA);
}

static void test_find_abi_hessian(void **state) {
  (void)state;
  const NWChemCFeatureEntry *e = nwchemc_feature_find("abi.nwchemc_hessian");
  assert_non_null(e);
  assert_int_equal(e->klass, NWCHEMC_FEATURE_ABI);
}

static void test_find_params_basis(void **state) {
  (void)state;
  const NWChemCFeatureEntry *e = nwchemc_feature_find("params.basis");
  assert_non_null(e);
  assert_int_equal(e->klass, NWCHEMC_FEATURE_PARAMS_FIELD);
}

static void test_unknown_null(void **state) {
  (void)state;
  assert_null(nwchemc_feature_find("module.nope"));
  assert_null(nwchemc_feature_find(NULL));
}

static void test_module_name_lookup(void **state) {
  (void)state;
  const NWChemCFeatureEntry *e = nwchemc_feature_find("module.pspwWannier");
  assert_non_null(e);
  assert_string_equal(nwchemc_module_nwchem_name(e->enum_or_field_id),
                      "pspw_wannier");
}

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_feature_count_nonzero),
      cmocka_unit_test(test_module_count),
      cmocka_unit_test(test_find_module_dft),
      cmocka_unit_test(test_find_stanza_pseudopotential),
      cmocka_unit_test(test_find_abi_hessian),
      cmocka_unit_test(test_find_params_basis),
      cmocka_unit_test(test_unknown_null),
      cmocka_unit_test(test_module_name_lookup),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
