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

static void assert_capn_text_equals(capn_text text, const char *expected) {
  assert_non_null(expected);
  assert_int_equal(text.len, (int)strlen(expected));
  assert_memory_equal(text.str, expected, (size_t)text.len);
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
  assert_render_contains(blocks, "tol2e");
  assert_render_contains(blocks, "driver\n");
  assert_render_contains(blocks, "maxiter 40");
  assert_render_contains(blocks, "tight");
  assert_render_contains(blocks, "gmax 2.5e-05");
  assert_render_contains(blocks, "grms 1.5e-05");
  assert_render_contains(blocks, "xmax 7.5e-05");
  assert_render_contains(blocks, "xrms 5.5e-05");
  assert_render_contains(blocks, "property\n");
  assert_render_contains(blocks, "dipole");
  assert_render_contains(blocks, "mulliken");
  assert_render_contains(blocks, "task scf energy");
  assert_render_contains(blocks, "set dft:grid xfine");
  assert_render_contains(blocks, "set dft:nopen integer 2");
  assert_render_contains(blocks, "set dft:smear_sigma double 0.0015");
  assert_render_contains(blocks, "set dft:spinset logical false");
  assert_render_contains(blocks, "print low");

  capn_text set_keys[4];
  capn_text set_values[4];
  size_t set_count = 0;
  assert_int_equal(nwchemc_params_extract_direct_set_strings(
                       params_root, set_keys, set_values, 4, &set_count),
                   0);
  assert_int_equal(set_count, 1);
  assert_capn_text_equals(set_keys[0], "dft:grid");
  assert_capn_text_equals(set_values[0], "xfine");

  capn_text typed_set_keys[4];
  int typed_set_types[4];
  int typed_set_counts[4];
  capn_text typed_set_values[16];
  size_t typed_set_count = 0;
  assert_int_equal(nwchemc_params_extract_direct_set_values(
                       params_root, typed_set_keys, typed_set_types,
                       typed_set_counts, typed_set_values, 4, 4,
                       &typed_set_count),
                   0);
  assert_int_equal(typed_set_count, 3);
  assert_capn_text_equals(typed_set_keys[0], "dft:nopen");
  assert_int_equal(typed_set_types[0], NWCHEMC_DIRECT_SET_VALUE_INTEGER);
  assert_int_equal(typed_set_counts[0], 1);
  assert_capn_text_equals(typed_set_values[0], "2");
  assert_capn_text_equals(typed_set_keys[1], "dft:smear_sigma");
  assert_int_equal(typed_set_types[1], NWCHEMC_DIRECT_SET_VALUE_DOUBLE);
  assert_int_equal(typed_set_counts[1], 1);
  assert_capn_text_equals(typed_set_values[4], "0.0015");
  assert_capn_text_equals(typed_set_keys[2], "dft:spinset");
  assert_int_equal(typed_set_types[2], NWCHEMC_DIRECT_SET_VALUE_LOGICAL);
  assert_int_equal(typed_set_counts[2], 1);
  assert_capn_text_equals(typed_set_values[8], "false");

  int scf_maxiter = 0;
  double scf_thresh = 0.0;
  double scf_tol2e = 0.0;
  int has_scf = 0;
  capn_text scf_wf = {0};
  int scf_nopen = -1;
  int scf_has_nopen = 0;
  assert_int_equal(nwchemc_params_extract_direct_scf(
                       params_root, &has_scf, &scf_maxiter, &scf_thresh,
                       &scf_tol2e, &scf_wf, &scf_nopen, &scf_has_nopen),
                   0);
  assert_int_equal(has_scf, 1);
  assert_int_equal(scf_maxiter, 50);
  assert_true(scf_thresh > 0.999e-6);
  assert_true(scf_thresh < 1.001e-6);
  assert_true(scf_tol2e > 0.999e-9);
  assert_true(scf_tol2e < 1.001e-9);
  int has_driver = 0;
  int driver_maxiter = 0;
  int driver_tolerance_mode = 0;
  double driver_gmax_tol = 0.0;
  double driver_grms_tol = 0.0;
  double driver_xmax_tol = 0.0;
  double driver_xrms_tol = 0.0;
  assert_int_equal(nwchemc_params_extract_direct_driver(
                       params_root, &has_driver, &driver_maxiter,
                       &driver_tolerance_mode, &driver_gmax_tol,
                       &driver_grms_tol, &driver_xmax_tol, &driver_xrms_tol),
                   0);
  assert_int_equal(has_driver, 1);
  assert_int_equal(driver_maxiter, 40);
  assert_int_equal(driver_tolerance_mode, NWCHEMC_DRIVER_TOLERANCE_TIGHT);
  assert_true(driver_gmax_tol > 2.499e-5);
  assert_true(driver_gmax_tol < 2.501e-5);
  assert_true(driver_grms_tol > 1.499e-5);
  assert_true(driver_grms_tol < 1.501e-5);
  assert_true(driver_xmax_tol > 7.499e-5);
  assert_true(driver_xmax_tol < 7.501e-5);
  assert_true(driver_xrms_tol > 5.499e-5);
  assert_true(driver_xrms_tol < 5.501e-5);

  char embed_blocks[NWCHEMC_BLOCKS];
  assert_int_equal(nwchemc_params_render_embed_input_blocks(
                       params_root, embed_blocks, sizeof(embed_blocks)),
                   0);
  assert_null(strstr(embed_blocks, "scf\n"));
  assert_null(strstr(embed_blocks, "maxiter 50"));
  assert_null(strstr(embed_blocks, "thresh 1e-06"));
  assert_null(strstr(embed_blocks, "tol2e 1e-09"));
  assert_null(strstr(embed_blocks, "driver\n"));
  assert_null(strstr(embed_blocks, "maxiter 40"));
  assert_null(strstr(embed_blocks, "tight"));
  assert_null(strstr(embed_blocks, "gmax 2.5e-05"));
  assert_null(strstr(embed_blocks, "grms 1.5e-05"));
  assert_null(strstr(embed_blocks, "xmax 7.5e-05"));
  assert_null(strstr(embed_blocks, "xrms 5.5e-05"));
  assert_null(strstr(embed_blocks, "set dft:grid xfine"));
  assert_null(strstr(embed_blocks, "set dft:nopen integer 2"));
  assert_null(strstr(embed_blocks, "set dft:smear_sigma double 0.0015"));
  assert_null(strstr(embed_blocks, "set dft:spinset logical false"));

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
