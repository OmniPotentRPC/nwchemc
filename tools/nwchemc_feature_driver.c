/**
 * C driver: exercise interned feature classes through shipped entry points.
 *
 * Modes: inventory | stub-abi | validate | all
 */
#include "nwchemc.h"
#include "nwchemc_features.h"

#include <stdio.h>
#include <string.h>

static int run_inventory(void) {
  size_t total = nwchemc_feature_count();
  size_t mods = nwchemc_feature_count_class(NWCHEMC_FEATURE_MODULE);
  size_t stanzas = nwchemc_feature_count_class(NWCHEMC_FEATURE_STANZA);
  size_t fields = nwchemc_feature_count_class(NWCHEMC_FEATURE_PARAMS_FIELD);
  size_t abis = nwchemc_feature_count_class(NWCHEMC_FEATURE_ABI);
  printf("nwchemc_feature_driver inventory\n");
  printf("  total=%zu modules=%zu stanzas=%zu params_fields=%zu abi=%zu\n",
         total, mods, stanzas, fields, abis);
  if (mods < 88 || stanzas < 6 || fields < 14 || abis < 6) {
    fprintf(stderr, "inventory counts below expected floor\n");
    return 1;
  }
  if (!nwchemc_feature_find("module.dft") ||
      !nwchemc_feature_find("params.basis") ||
      !nwchemc_feature_find("stanza.pseudopotential") ||
      !nwchemc_feature_find("abi.nwchemc_hessian")) {
    fprintf(stderr, "missing core intern rows\n");
    return 1;
  }
  printf("  core intern rows present\n");
  return 0;
}

static int run_stub_abi(void) {
  printf("nwchemc_feature_driver stub-abi\n");
  if (nwchemc_available() != 0) {
    fprintf(stderr, "expected stub unavailable\n");
    return 1;
  }
  const char *ver = nwchemc_version();
  if (!ver || !strstr(ver, "stub")) {
    fprintf(stderr, "unexpected version: %s\n", ver ? ver : "(null)");
    return 1;
  }
  if (nwchemc_set_params(NULL, 0) == 0) {
    fprintf(stderr, "set_params should fail on stub\n");
    return 1;
  }
  NWChemCResult g = nwchemc_energy_gradient(0, NULL, NULL, NULL, 0, NULL);
  NWChemCResult h = nwchemc_hessian(0, NULL, NULL, NULL, 0, NULL);
  if (g.ok != 0 || h.ok != 0) {
    fprintf(stderr, "gradient/hessian should fail on stub\n");
    return 1;
  }
  nwchemc_finalize();
  printf("  stub ABI ok (grad_msg=%s hess_msg=%s)\n", g.message, h.message);
  return 0;
}

static int run_validate(void) {
  if (run_inventory() != 0)
    return 1;
  for (int id = 0; id < 88; ++id) {
    int found = 0;
    const NWChemCFeatureEntry *tab = nwchemc_feature_table();
    size_t n = nwchemc_feature_count();
    for (size_t i = 0; i < n; ++i) {
      if (tab[i].klass == NWCHEMC_FEATURE_MODULE &&
          tab[i].enum_or_field_id == id) {
        found = 1;
        break;
      }
    }
    if (!found) {
      fprintf(stderr, "missing module enum id %d\n", id);
      return 1;
    }
  }
  printf("  validate: all 88 module enum ids interned\n");
  return 0;
}

int main(int argc, char **argv) {
  const char *mode = (argc > 1) ? argv[1] : "inventory";
  if (strcmp(mode, "inventory") == 0)
    return run_inventory();
  if (strcmp(mode, "stub-abi") == 0)
    return run_stub_abi();
  if (strcmp(mode, "validate") == 0)
    return run_validate();
  if (strcmp(mode, "all") == 0) {
    int rc = run_validate();
    if (rc != 0)
      return rc;
    return run_stub_abi();
  }
  fprintf(stderr, "usage: %s [inventory|stub-abi|validate|all]\n", argv[0]);
  return 2;
}
