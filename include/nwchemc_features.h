#pragma once
/**
 * @file nwchemc_features.h
 * @brief Machine-readable intern table for NWChemParams / module / stanza features.
 *
 * Kept in sync with schema/inventory/nwchem_features.json and schema/Potentials.capnp.
 * Regenerate via tools/gen_feature_inventory.py when schema changes.
 */

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NWChemCFeatureClass {
  NWCHEMC_FEATURE_MODULE = 0,
  NWCHEMC_FEATURE_STANZA = 1,
  NWCHEMC_FEATURE_PARAMS_FIELD = 2,
  NWCHEMC_FEATURE_ABI = 3,
} NWChemCFeatureClass;

typedef struct NWChemCFeatureEntry {
  const char *feature_id;
  const char *schema_path;
  const char *nwchem_text_or_role;
  NWChemCFeatureClass klass;
  int enum_or_field_id;
  int stub_applicable;
  int embed_applicable;
} NWChemCFeatureEntry;

size_t nwchemc_feature_count(void);
const NWChemCFeatureEntry *nwchemc_feature_table(void);
const NWChemCFeatureEntry *nwchemc_feature_find(const char *feature_id);
size_t nwchemc_feature_count_class(NWChemCFeatureClass klass);
const char *nwchemc_module_nwchem_name(int module_enum_id);
size_t nwchemc_module_feature_count(void);

#ifdef __cplusplus
}
#endif
