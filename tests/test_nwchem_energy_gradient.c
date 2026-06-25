#include "nwchemc.h"

#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

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

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s PARAMS_BIN\n", argv[0]);
    return 2;
  }
  if (ensure_dir(NWCHEMC_TEST_SCRATCH_DIR) != 0 ||
      ensure_dir(NWCHEMC_TEST_PERMANENT_DIR) != 0)
    return 1;

  size_t params_size = 0;
  unsigned char *params = read_file(argv[1], &params_size);
  if (!params)
    return 1;

  if (!nwchemc_available()) {
    fprintf(stderr, "nwchemc_available returned false\n");
    free(params);
    return 1;
  }

  const int n_atoms = 2;
  const int atomic_numbers[2] = {1, 1};
  const double positions_ang[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.74};
  double grad[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

  NWChemCResult result = nwchemc_energy_gradient(
      n_atoms, positions_ang, atomic_numbers, params, params_size, grad);
  free(params);

  if (!result.ok) {
    fprintf(stderr, "nwchemc_energy_gradient failed: %s\n", result.message);
    return 1;
  }
  if (!isfinite(result.energy_h) || result.energy_h >= -0.5 ||
      result.energy_h <= -2.0) {
    fprintf(stderr, "unexpected H2 energy: %.16g (%s)\n", result.energy_h,
            result.message);
    return 1;
  }
  for (int i = 0; i < 6; ++i) {
    if (!isfinite(grad[i])) {
      fprintf(stderr, "non-finite gradient[%d]\n", i);
      return 1;
    }
  }
  return 0;
}
