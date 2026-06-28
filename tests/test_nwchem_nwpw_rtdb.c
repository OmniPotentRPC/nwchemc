#include "nwchemc.h"

#include <errno.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <cmocka.h>

static const char *g_config_path = NULL;

extern void nwchemc_test_nwpw_rtdb(int *result);
extern void nwchemc_test_configured_nwpw_rtdb(int *result);

static int ensure_dir(const char *path) {
  if (mkdir(path, 0777) == 0 || errno == EEXIST)
    return 0;
  fprintf(stderr, "mkdir failed for %s: %s\n", path, strerror(errno));
  return -1;
}

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

static int setup_nwchem_dirs(void **state) {
  (void)state;
  if (ensure_dir(NWCHEMC_TEST_SCRATCH_DIR) != 0)
    return 1;
  if (ensure_dir(NWCHEMC_TEST_PERMANENT_DIR) != 0)
    return 1;
  if (ensure_dir(NWCHEMC_TEST_SCRATCH_DIR "-nwpw-controls") != 0)
    return 1;
  return ensure_dir(NWCHEMC_TEST_PERMANENT_DIR "-nwpw-controls");
}

static int teardown_nwchem(void **state) {
  (void)state;
  nwchemc_finalize();
  return 0;
}

static void test_nwpw_controls_reach_rtdb(void **state) {
  (void)state;
  int result = -1;
  nwchemc_test_nwpw_rtdb(&result);
  assert_int_equal(result, 0);
}

static void test_configured_nwpw_controls_reach_rtdb(void **state) {
  (void)state;
  assert_true(nwchemc_available());

  size_t config_size = 0;
  unsigned char *config = read_file(g_config_path, &config_size);
  assert_non_null(config);

  assert_int_equal(nwchemc_configure(config, config_size), 0);

  int result = -1;
  nwchemc_test_configured_nwpw_rtdb(&result);
  assert_int_equal(result, 0);

  free(config);
}

int main(int argc, char **argv) {
  if (argc >= 2 && strcmp(argv[1], "direct") == 0) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_nwpw_controls_reach_rtdb),
    };
    return cmocka_run_group_tests(tests, setup_nwchem_dirs, teardown_nwchem);
  }
  if (argc == 3 && strcmp(argv[1], "configured") == 0) {
    g_config_path = argv[2];
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_configured_nwpw_controls_reach_rtdb),
    };
    return cmocka_run_group_tests(tests, setup_nwchem_dirs, teardown_nwchem);
  }
  fprintf(stderr, "usage: %s direct | configured potential-config.bin\n",
          argv[0]);
  return 2;
}
