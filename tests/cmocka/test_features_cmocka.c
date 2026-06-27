#include "nwchemc_features.h"

#include <cmocka.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* Drives shipped nwchemc_feature_* accessors; counts must match inventory. */

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

static void test_find_tce_schema_fields(void **state) {
  (void)state;
  static const char *ids[] = {
      "field.NWChemTceStanza.dipole",
      "field.NWChemTceStanza.quadrupole",
      "field.NWChemTceStanza.octupole",
      "field.PotentialResult.hessian",
      "field.PotentialResult.dipole",
      "field.PotentialResult.quadrupole",
      "field.PotentialResult.optimizedPos",
      "field.PotentialResult.frequencies",
      "field.PotentialResult.intensities",
      "field.PotentialResult.stress",
      "field.ForceInput.hasCharge",
      "field.ForceInput.charge",
      "field.ForceInput.hasMultiplicity",
      "field.ForceInput.multiplicity",
  };
  size_t i;
  for (i = 0; i < sizeof(ids) / sizeof(ids[0]); ++i) {
    const NWChemCFeatureEntry *e = nwchemc_feature_find(ids[i]);
    assert_non_null(e);
    assert_int_equal(e->klass, NWCHEMC_FEATURE_SCHEMA_FIELD);
  }
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

static void test_class_partition_counts(void **state) {
  (void)state;
  size_t mods = nwchemc_feature_count_class(NWCHEMC_FEATURE_MODULE);
  size_t stanzas = nwchemc_feature_count_class(NWCHEMC_FEATURE_STANZA);
  size_t fields = nwchemc_feature_count_class(NWCHEMC_FEATURE_PARAMS_FIELD);
  size_t schema_fields =
      nwchemc_feature_count_class(NWCHEMC_FEATURE_SCHEMA_FIELD);
  size_t abis = nwchemc_feature_count_class(NWCHEMC_FEATURE_ABI);
  assert_int_equal((int)mods, 88);
  assert_int_equal((int)stanzas, 18);
  assert_int_equal((int)fields, 14);
  assert_int_equal((int)schema_fields, 319);
  assert_int_equal((int)abis, 59);
  assert_int_equal((int)(mods + stanzas + fields + schema_fields + abis),
                   (int)nwchemc_feature_count());
}

static void test_feature_table_integrity(void **state) {
  (void)state;
  const NWChemCFeatureEntry *table = nwchemc_feature_table();
  size_t n = nwchemc_feature_count();
  size_t i;
  assert_non_null(table);
  assert_true(n > 0);
  for (i = 0; i < n; ++i) {
    assert_non_null(table[i].feature_id);
    assert_non_null(table[i].schema_path);
    assert_non_null(table[i].nwchem_text_or_role);
    assert_true(table[i].klass >= NWCHEMC_FEATURE_MODULE &&
                table[i].klass <= NWCHEMC_FEATURE_SCHEMA_FIELD);
    assert_non_null(nwchemc_feature_find(table[i].feature_id));
  }
}

static void test_all_modules_enum_ids_unique(void **state) {
  (void)state;
  const NWChemCFeatureEntry *table = nwchemc_feature_table();
  size_t n = nwchemc_feature_count();
  int seen[128];
  size_t i;
  memset(seen, 0, sizeof(seen));
  for (i = 0; i < n; ++i) {
    if (table[i].klass != NWCHEMC_FEATURE_MODULE)
      continue;
    assert_true(table[i].enum_or_field_id >= 0);
    assert_true(table[i].enum_or_field_id < 128);
    assert_int_equal(seen[table[i].enum_or_field_id], 0);
    seen[table[i].enum_or_field_id] = 1;
  }
  for (i = 0; i < 88; ++i)
    assert_int_equal(seen[i], 1);
}

static void test_abi_entrypoints_interned(void **state) {
  (void)state;
  static const char *ids[] = {
      "abi.nwchemc_set_params",
      "abi.nwchemc_energy_gradient",
      "abi.nwchemc_energy",
      "abi.nwchemc_energy_forces",
      "abi.nwchemc_hessian",
      "abi.nwchemc_dipole",
      "abi.nwchemc_quadrupole",
      "abi.nwchemc_optimize",
      "abi.nwchemc_frequencies",
      "abi.nwchemc_stress",
      "abi.nwchemc_session_create",
      "abi.nwchemc_session_set_params",
      "abi.nwchemc_session_destroy",
      "abi.nwchemc_session_energy_gradient",
      "abi.nwchemc_session_energy",
      "abi.nwchemc_session_energy_forces",
      "abi.nwchemc_session_dipole",
      "abi.nwchemc_session_quadrupole",
      "abi.nwchemc_session_optimize",
      "abi.nwchemc_session_frequencies",
      "abi.nwchemc_session_stress",
      "abi.nwchemc_session_calculate_forces",
      "abi.nwchemc_session_calculate_result",
      "abi.nwchemc_calculate_result",
      "abi.nwchemc_calculate_hessian",
      "abi.nwchemc_hessian_result_size_for_force_input",
      "abi.nwchemc_session_calculate_hessian_result",
      "abi.nwchemc_calculate_hessian_result",
      "abi.nwchemc_calculate_dipole",
      "abi.nwchemc_dipole_result_size_for_force_input",
      "abi.nwchemc_session_calculate_dipole_result",
      "abi.nwchemc_calculate_dipole_result",
      "abi.nwchemc_calculate_quadrupole",
      "abi.nwchemc_quadrupole_result_size_for_force_input",
      "abi.nwchemc_session_calculate_quadrupole_result",
      "abi.nwchemc_calculate_quadrupole_result",
      "abi.nwchemc_calculate_optimize",
      "abi.nwchemc_optimize_result_size_for_force_input",
      "abi.nwchemc_session_calculate_optimize_result",
      "abi.nwchemc_calculate_optimize_result",
      "abi.nwchemc_calculate_frequencies",
      "abi.nwchemc_frequencies_result_size_for_force_input",
      "abi.nwchemc_session_calculate_frequencies_result",
      "abi.nwchemc_calculate_frequencies_result",
      "abi.nwchemc_calculate_stress",
      "abi.nwchemc_stress_result_size_for_force_input",
      "abi.nwchemc_session_calculate_stress_result",
      "abi.nwchemc_calculate_stress_result",
      "abi.nwchemc_potential_result_size_for_force_input",
      "abi.nwchemc_session_calculate_hessian",
      "abi.nwchemc_session_calculate_dipole",
      "abi.nwchemc_session_calculate_quadrupole",
      "abi.nwchemc_session_calculate_optimize",
      "abi.nwchemc_session_calculate_frequencies",
      "abi.nwchemc_session_calculate_stress",
      "abi.nwchemc_session_hessian",
      "abi.nwchemc_available",
      "abi.nwchemc_version",
      "abi.nwchemc_finalize",
  };
  size_t i;
  for (i = 0; i < sizeof(ids) / sizeof(ids[0]); ++i) {
    const NWChemCFeatureEntry *e = nwchemc_feature_find(ids[i]);
    assert_non_null(e);
    assert_int_equal(e->klass, NWCHEMC_FEATURE_ABI);
    assert_int_equal(e->stub_applicable, 1);
  }
}

static void test_stanza_kinds_interned(void **state) {
  (void)state;
  static const char *ids[] = {
      "stanza.generic", "stanza.dft",        "stanza.set",
      "stanza.raw",     "stanza.module",     "stanza.pseudopotential",
      "stanza.scf",     "stanza.task",       "stanza.driver",
      "stanza.property", "stanza.basis",     "stanza.geometry",
      "stanza.mrccData", "stanza.brillouinZone",
  };
  size_t i;
  for (i = 0; i < sizeof(ids) / sizeof(ids[0]); ++i) {
    const NWChemCFeatureEntry *e = nwchemc_feature_find(ids[i]);
    assert_non_null(e);
    assert_int_equal(e->klass, NWCHEMC_FEATURE_STANZA);
  }
}

static void test_params_fields_interned(void **state) {
  (void)state;
  static const char *ids[] = {
      "params.basis",        "params.theory",       "params.scfType",
      "params.charge",       "params.multiplicity", "params.enginePath",
      "params.nwchemRoot",   "params.task",         "params.title",
      "params.memoryMb",     "params.scratchDir",   "params.permanentDir",
      "params.inputBlocks",  "params.inputStanzas",
  };
  size_t i;
  for (i = 0; i < sizeof(ids) / sizeof(ids[0]); ++i) {
    const NWChemCFeatureEntry *e = nwchemc_feature_find(ids[i]);
    assert_non_null(e);
    assert_int_equal(e->klass, NWCHEMC_FEATURE_PARAMS_FIELD);
  }
}

static void test_module_nwchem_name_bounds(void **state) {
  (void)state;
  assert_null(nwchemc_module_nwchem_name(-1));
  assert_null(nwchemc_module_nwchem_name(9999));
  /* module.custom uses parenthetical role text; lookup returns NULL. */
  assert_null(nwchemc_module_nwchem_name(0));
  assert_string_equal(nwchemc_module_nwchem_name(1), "basis");
  assert_string_equal(nwchemc_module_nwchem_name(5), "dft");
}

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_feature_count_nonzero),
      cmocka_unit_test(test_module_count),
      cmocka_unit_test(test_find_module_dft),
      cmocka_unit_test(test_find_stanza_pseudopotential),
      cmocka_unit_test(test_find_abi_hessian),
      cmocka_unit_test(test_find_params_basis),
      cmocka_unit_test(test_find_tce_schema_fields),
      cmocka_unit_test(test_unknown_null),
      cmocka_unit_test(test_module_name_lookup),
      cmocka_unit_test(test_class_partition_counts),
      cmocka_unit_test(test_feature_table_integrity),
      cmocka_unit_test(test_all_modules_enum_ids_unique),
      cmocka_unit_test(test_abi_entrypoints_interned),
      cmocka_unit_test(test_stanza_kinds_interned),
      cmocka_unit_test(test_params_fields_interned),
      cmocka_unit_test(test_module_nwchem_name_bounds),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
