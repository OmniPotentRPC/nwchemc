#include "nwchemc_params.h"

#include <errno.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cmocka.h>

static const char *g_params_path = NULL;

static unsigned char *read_file(const char *path, size_t *size) {
  FILE *fp = fopen(path, "rb");
  if (!fp) {
    fprintf(stderr, "open failed for %s: %s\n", path, strerror(errno));
    return NULL;
  }
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

static int text_equals(capn_text text, const char *expected) {
  size_t n = strlen(expected);
  return text.len == (int)n && text.str && memcmp(text.str, expected, n) == 0;
}

static void test_parser_renders_structured_input(void **state) {
  (void)state;

  size_t message_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  struct NWChemParams params;
  read_NWChemParams(&params, params_root);
  char input_blocks[NWCHEMC_BLOCKS];
  assert_true(text_equals(params.basis, "def2-svp"));
  assert_true(text_equals(params.theory, "dft"));
  assert_true(text_equals(params.scfType, "pbe0"));
  assert_int_equal(params.charge, -1);
  assert_int_equal(params.multiplicity, 3);
  assert_int_equal(params.memoryMb, 1024);
  assert_true(text_equals(params.nwchemRoot, "/opt/nwchem"));
  assert_true(text_equals(params.scratchDir, "/scratch/nw"));
  assert_true(text_equals(params.permanentDir, "/perm/nw"));
  assert_int_equal(nwchemc_params_render_input_blocks(
                       params_root, input_blocks, sizeof(input_blocks)),
                   0);
  assert_non_null(strstr(input_blocks, "smear 0.001 fixsz"));
  assert_non_null(strstr(input_blocks, "xc pbe0"));
  assert_non_null(strstr(input_blocks, "iterations 40"));
  assert_non_null(strstr(input_blocks, "driver"));
  assert_non_null(strstr(input_blocks, "maxiter 20"));
  assert_non_null(strstr(input_blocks, "scf"));
  assert_non_null(strstr(input_blocks, "thresh 1e-6"));
  assert_non_null(strstr(input_blocks, "tol2e 1e-9"));
  assert_non_null(strstr(input_blocks, "acc_std"));
  assert_non_null(strstr(input_blocks, "pseudopotentials"));
  assert_non_null(strstr(input_blocks, "Si library sg15"));
  assert_non_null(strstr(input_blocks, "O paw_library paw_default"));
  assert_non_null(strstr(input_blocks, "pspspin off"));

  nwchemc_params_release(&arena);
  free(message);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s PARAMS_BIN\n", argv[0]);
    return 2;
  }
  g_params_path = argv[1];
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_parser_renders_structured_input),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
