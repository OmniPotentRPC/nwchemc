#include "nwchemc_params.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s PARAMS_BIN\n", argv[0]);
    return 2;
  }

  size_t message_size = 0;
  unsigned char *message = read_file(argv[1], &message_size);
  if (!message)
    return 1;

  struct capn arena;
  NWChemParams_ptr params_root;
  if (nwchemc_params_root(message, message_size, &arena, &params_root) != 0) {
    fprintf(stderr, "parse failed\n");
    free(message);
    return 1;
  }

  struct NWChemParams params;
  read_NWChemParams(&params, params_root);
  char input_blocks[NWCHEMC_BLOCKS];
  int ok = text_equals(params.basis, "def2-svp") &&
           text_equals(params.theory, "dft") &&
           text_equals(params.scfType, "pbe0") && params.charge == -1 &&
           params.multiplicity == 3 && params.memoryMb == 1024 &&
           text_equals(params.nwchemRoot, "/opt/nwchem") &&
           text_equals(params.scratchDir, "/scratch/nw") &&
           text_equals(params.permanentDir, "/perm/nw") &&
           nwchemc_params_render_input_blocks(params_root, input_blocks,
                                              sizeof(input_blocks)) == 0 &&
           strstr(input_blocks, "smear 0.001 fixsz") != NULL &&
           strstr(input_blocks, "xc pbe0") != NULL &&
           strstr(input_blocks, "iterations 40") != NULL &&
           strstr(input_blocks, "driver") != NULL &&
           strstr(input_blocks, "maxiter 20") != NULL &&
           strstr(input_blocks, "acc_std") != NULL;

  nwchemc_params_release(&arena);
  free(message);
  if (!ok) {
    fprintf(stderr, "parsed params mismatch\n");
    return 1;
  }
  return 0;
}
