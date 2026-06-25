/**
 * C driver: exercise interned feature classes through shipped entry points.
 *
 * Modes:
 *   inventory  - intern table counts/core rows
 *   stub-abi   - nwchemc_* stub ABI (set_params/gradient/hessian/...)
 *   params     - load Cap'n Proto fixture and drive nwchemc_params_* parser/render
 *   validate   - inventory + all module enum ids interned
 *   all        - validate + stub-abi (+ params if NWCHEMC_PARSER_FIXTURE set)
 *
 * Params mode requires argv[2] or env NWCHEMC_PARSER_FIXTURE pointing at an
 * unpacked flat NWChemParams message (same fixture as test_capnp_parser).
 */
#include "nwchemc.h"
#include "nwchemc_features.h"
#include "nwchemc_params.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char *load_file(const char *path, size_t *out_len) {
  FILE *f = fopen(path, "rb");
  if (!f)
    return NULL;
  if (fseek(f, 0, SEEK_END) != 0) {
    fclose(f);
    return NULL;
  }
  long sz = ftell(f);
  if (sz <= 0) {
    fclose(f);
    return NULL;
  }
  if (fseek(f, 0, SEEK_SET) != 0) {
    fclose(f);
    return NULL;
  }
  unsigned char *buf = malloc((size_t)sz);
  if (!buf) {
    fclose(f);
    return NULL;
  }
  if (fread(buf, 1, (size_t)sz, f) != (size_t)sz) {
    free(buf);
    fclose(f);
    return NULL;
  }
  fclose(f);
  *out_len = (size_t)sz;
  return buf;
}

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

static int run_params(const char *fixture_path) {
  printf("nwchemc_feature_driver params\n");
  if (!fixture_path || !*fixture_path) {
    fprintf(stderr, "params mode needs fixture path (argv or NWCHEMC_PARSER_FIXTURE)\n");
    return 1;
  }
  size_t len = 0;
  unsigned char *msg = load_file(fixture_path, &len);
  if (!msg) {
    fprintf(stderr, "failed to read fixture: %s\n", fixture_path);
    return 1;
  }

  /* Empty/invalid must fail through shipped parser entry. */
  struct capn arena_bad;
  NWChemParams_ptr params_bad;
  if (nwchemc_params_root(NULL, 0, &arena_bad, &params_bad) == 0) {
    fprintf(stderr, "params_root should reject empty message\n");
    free(msg);
    return 1;
  }

  struct capn arena;
  NWChemParams_ptr params;
  if (nwchemc_params_root(msg, len, &arena, &params) != 0) {
    fprintf(stderr, "params_root failed on fixture %s\n", fixture_path);
    free(msg);
    return 1;
  }

  char blocks[NWCHEMC_BLOCKS];
  blocks[0] = '\0';
  if (nwchemc_params_render_input_blocks(params, blocks, sizeof(blocks)) != 0) {
    fprintf(stderr, "render_input_blocks failed\n");
    nwchemc_params_release(&arena);
    free(msg);
    return 1;
  }

  /* Interned stanza/field classes must be representable in schema; fixture
   * may not emit every block, but render must succeed without buffer overflow. */
  printf("  fixture=%s bytes=%zu rendered_blocks_len=%zu\n", fixture_path, len,
         strlen(blocks));
  printf("  interned stanza classes: generic/dft/set/raw/module/pseudopotential\n");
  printf("  interned params.basis role: %s\n",
         nwchemc_feature_find("params.basis")->nwchem_text_or_role);

  nwchemc_params_release(&arena);
  free(msg);
  printf("  params parser/render path ok via shipped nwchemc_params_*\n");
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
  const char *fixture = (argc > 2) ? argv[2] : getenv("NWCHEMC_PARSER_FIXTURE");

  if (strcmp(mode, "inventory") == 0)
    return run_inventory();
  if (strcmp(mode, "stub-abi") == 0)
    return run_stub_abi();
  if (strcmp(mode, "params") == 0)
    return run_params(fixture);
  if (strcmp(mode, "validate") == 0)
    return run_validate();
  if (strcmp(mode, "all") == 0) {
    int rc = run_validate();
    if (rc != 0)
      return rc;
    rc = run_stub_abi();
    if (rc != 0)
      return rc;
    if (fixture && *fixture)
      return run_params(fixture);
    printf("nwchemc_feature_driver all: skipped params (no fixture path)\n");
    return 0;
  }
  fprintf(stderr,
          "usage: %s [inventory|stub-abi|params|validate|all] [fixture.bin]\n",
          argv[0]);
  return 2;
}
