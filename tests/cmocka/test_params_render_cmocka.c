/**
 * Drive shipped nwchemc_params_* render/extract paths on multi-stanza fixtures.
 * No reimplementation: asserts substrings from real renderer output only.
 */
#include "nwchemc_params.h"

#include <cmocka.h>
#include <errno.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *g_config_options_path = NULL;
static const char *g_structured_path = NULL;

static unsigned char *read_file(const char *path, size_t *size) {
  FILE *fp = fopen(path, "rb");
  if (!fp)
    return NULL;
  if (fseek(fp, 0, SEEK_END) != 0) {
    fclose(fp);
    return NULL;
  }
  long n = ftell(fp);
  if (n <= 0) {
    fclose(fp);
    return NULL;
  }
  rewind(fp);
  unsigned char *buf = (unsigned char *)malloc((size_t)n);
  if (!buf) {
    fclose(fp);
    return NULL;
  }
  if (fread(buf, 1, (size_t)n, fp) != (size_t)n) {
    free(buf);
    fclose(fp);
    return NULL;
  }
  fclose(fp);
  *size = (size_t)n;
  return buf;
}

static void assert_render_contains(const char *blocks, const char *needle) {
  assert_non_null(blocks);
  assert_non_null(strstr(blocks, needle));
}

static void test_render_config_options_stanzas(void **state) {
  (void)state;
  assert_non_null(g_config_options_path);

  size_t message_size = 0;
  unsigned char *message = read_file(g_config_options_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  char blocks[NWCHEMC_BLOCKS];
  assert_int_equal(
      nwchemc_params_render_input_blocks(params_root, blocks, sizeof(blocks)),
      0);

  /* Shipped renderers for geometry/scf/driver/property/task/set/raw. */
  assert_render_contains(blocks, "geometry");
  assert_render_contains(blocks, "units angstrom");
  assert_render_contains(blocks, "symmetry c1");
  assert_render_contains(blocks, "scf\n");
  assert_render_contains(blocks, "maxiter 50");
  assert_render_contains(blocks, "thresh");
  assert_render_contains(blocks, "driver\n");
  assert_render_contains(blocks, "tight");
  assert_render_contains(blocks, "property\n");
  assert_render_contains(blocks, "dipole");
  assert_render_contains(blocks, "mulliken");
  assert_render_contains(blocks, "task scf energy");
  assert_render_contains(blocks, "set dft:grid xfine");
  assert_render_contains(blocks, "print low");

  nwchemc_params_release(&arena);
  free(message);
}

static void test_render_structured_dft_full_and_embed(void **state) {
  (void)state;
  if (!g_structured_path)
    skip();

  size_t message_size = 0;
  unsigned char *message = read_file(g_structured_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  char full_blocks[NWCHEMC_BLOCKS];
  char embed_blocks[NWCHEMC_BLOCKS];
  assert_int_equal(nwchemc_params_render_input_blocks(params_root, full_blocks,
                                                      sizeof(full_blocks)),
                   0);
  assert_int_equal(nwchemc_params_render_embed_input_blocks(
                       params_root, embed_blocks, sizeof(embed_blocks)),
                   0);

  /* Full render includes promoted DFT fields. */
  assert_render_contains(full_blocks, "dft\n");
  assert_render_contains(full_blocks, "xc pbe0");
  assert_render_contains(full_blocks, "direct");

  /* Embed render omits promoted fields when only those were set. */
  assert_null(strstr(embed_blocks, "xc pbe0"));
  assert_null(strstr(embed_blocks, "  direct"));

  capn_text xc = {0};
  int direct_enabled = 0;
  int smearing_enabled = 0;
  double smear_sigma = 0.0;
  int spinset = 0;
  assert_int_equal(nwchemc_params_extract_direct_dft(
                       params_root, &xc, &direct_enabled, &smearing_enabled,
                       &smear_sigma, &spinset),
                   0);
  assert_true(xc.len > 0);
  assert_int_equal(direct_enabled, 1);

  nwchemc_params_release(&arena);
  free(message);
}

static void test_render_rejects_null_dst(void **state) {
  (void)state;
  assert_non_null(g_config_options_path);
  size_t message_size = 0;
  unsigned char *message = read_file(g_config_options_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);
  assert_int_equal(nwchemc_params_render_input_blocks(params_root, NULL, 0),
                   -1);
  assert_int_equal(
      nwchemc_params_render_embed_input_blocks(params_root, NULL, 0), -1);

  nwchemc_params_release(&arena);
  free(message);
}

static void test_params_root_rejects_empty(void **state) {
  (void)state;
  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(nwchemc_params_root(NULL, 0, &arena, &params_root), -1);
  assert_int_equal(nwchemc_params_root((const void *)"x", 1, NULL, &params_root),
                   -1);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s CONFIG_OPTIONS_BIN [STRUCTURED_BIN]\n", argv[0]);
    return 2;
  }
  g_config_options_path = argv[1];
  g_structured_path = argc >= 3 ? argv[2] : NULL;

  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_render_config_options_stanzas),
      cmocka_unit_test(test_render_structured_dft_full_and_embed),
      cmocka_unit_test(test_render_rejects_null_dst),
      cmocka_unit_test(test_params_root_rejects_empty),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
