#include "nwchemc_params.h"

#include <errno.h>
#include <limits.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cmocka.h>

static const char *g_params_path = NULL;

extern void nwchemc_test_pseudopotential_text_rtdb(const char *input_blocks,
                                                  const int *input_len,
                                                  int *result);
extern void nwchemc_test_psp_text_reset_rtdb(
    const char *input_blocks, const int *input_len, int *result);

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

static void test_rendered_pseudopotential_deck_reaches_rtdb(void **state) {
  (void)state;

  size_t params_size = 0;
  unsigned char *params_bytes = read_file(g_params_path, &params_size);
  assert_non_null(params_bytes);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(nwchemc_params_root(params_bytes, params_size, &arena,
                                       &params_root),
                   0);

  char input_blocks[NWCHEMC_BLOCKS];
  assert_int_equal(nwchemc_params_render_input_blocks(
                       params_root, input_blocks, sizeof(input_blocks)),
                   0);
  assert_non_null(strstr(input_blocks, "nwpw"));
  assert_non_null(strstr(input_blocks, "pseudopotential_libraries"));
  assert_non_null(strstr(input_blocks, "Si library sg15"));
  assert_non_null(strstr(input_blocks, "H pspw_library hgh_lda"));
  assert_non_null(strstr(input_blocks, "O paw_library paw_default"));
  assert_non_null(strstr(input_blocks, "C cpi C.cpi"));
  assert_non_null(strstr(input_blocks, "N teter N.teter"));
  assert_non_null(strstr(input_blocks, "* pspw_library pspw_default"));
  assert_null(strstr(input_blocks, "pspspin on"));
  assert_non_null(strstr(input_blocks, "pspspin up p 1.25 2 3"));
  assert_non_null(strstr(input_blocks, "pspspin not_m 0 down d 0.75 2"));
  assert_non_null(strstr(input_blocks, "uterm d 1.4 0.2 4 6"));
  assert_non_null(
      strstr(input_blocks, "set nwpw:psp:semicore_small logical true"));

  size_t input_len_size = strlen(input_blocks);
  assert_true(input_len_size > 0);
  assert_true(input_len_size <= (size_t)INT_MAX);
  int input_len = (int)input_len_size;
  int probe_result = -1;
  nwchemc_test_pseudopotential_text_rtdb(input_blocks, &input_len,
                                         &probe_result);
  assert_int_equal(probe_result, 0);

  nwchemc_params_release(&arena);
  free(params_bytes);
}

static void test_rendered_pseudopotential_reset_deck_reaches_rtdb(
    void **state) {
  (void)state;

  size_t params_size = 0;
  unsigned char *params_bytes = read_file(g_params_path, &params_size);
  assert_non_null(params_bytes);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(nwchemc_params_root(params_bytes, params_size, &arena,
                                       &params_root),
                   0);

  char input_blocks[NWCHEMC_BLOCKS];
  assert_int_equal(nwchemc_params_render_input_blocks(
                       params_root, input_blocks, sizeof(input_blocks)),
                   0);
  assert_non_null(strstr(input_blocks, "nwpw"));
  assert_non_null(strstr(input_blocks, "pspspin off"));
  assert_non_null(strstr(input_blocks, "uterm off"));
  assert_non_null(
      strstr(input_blocks, "set nwpw:psp:semicore_small logical false"));
  assert_null(strstr(input_blocks, "pseudopotentials"));

  size_t input_len_size = strlen(input_blocks);
  assert_true(input_len_size > 0);
  assert_true(input_len_size <= (size_t)INT_MAX);
  int input_len = (int)input_len_size;
  int probe_result = -1;
  nwchemc_test_psp_text_reset_rtdb(input_blocks, &input_len, &probe_result);
  assert_int_equal(probe_result, 0);

  nwchemc_params_release(&arena);
  free(params_bytes);
}

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s enabled|reset nwchem-params.bin\n", argv[0]);
    return 2;
  }
  const char *mode = argv[1];
  g_params_path = argv[2];

  if (strcmp(mode, "enabled") == 0) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_rendered_pseudopotential_deck_reaches_rtdb),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
  }

  if (strcmp(mode, "reset") == 0) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_rendered_pseudopotential_reset_deck_reaches_rtdb),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
  }

  fprintf(stderr, "unknown pseudopotential text RTDB mode: %s\n", mode);
  return 2;
}
