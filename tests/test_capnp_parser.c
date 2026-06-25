#include "nwchemc_params.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void store_u32_le(unsigned char *p, uint32_t value) {
  p[0] = (unsigned char)(value & 0xffu);
  p[1] = (unsigned char)((value >> 8) & 0xffu);
  p[2] = (unsigned char)((value >> 16) & 0xffu);
  p[3] = (unsigned char)((value >> 24) & 0xffu);
}

static void store_u64_le(unsigned char *p, uint64_t value) {
  store_u32_le(p, (uint32_t)(value & 0xffffffffu));
  store_u32_le(p + 4, (uint32_t)(value >> 32));
}

static uint64_t struct_ptr(uint32_t data_words, uint32_t pointer_count) {
  return ((uint64_t)data_words << 32) | ((uint64_t)pointer_count << 48);
}

static uint64_t list_ptr(uint32_t pointer_index, uint32_t target_word,
                         uint32_t elem_size, uint32_t elem_count) {
  uint32_t off = target_word - pointer_index - 1u;
  return 1u | ((uint64_t)off << 2) | ((uint64_t)elem_size << 32) |
         ((uint64_t)elem_count << 35);
}

static uint32_t write_text(unsigned char *segment, uint32_t word,
                           const char *text) {
  size_t n = strlen(text) + 1u;
  memcpy(segment + word * 8u, text, n);
  return word + (uint32_t)((n + 7u) / 8u);
}

static uint32_t set_text(unsigned char *segment, uint32_t pointer_index,
                         uint32_t word, const char *text) {
  size_t n = strlen(text) + 1u;
  store_u64_le(segment + pointer_index * 8u,
               list_ptr(pointer_index, word, 2u, (uint32_t)n));
  return write_text(segment, word, text);
}

static uint32_t set_text_list(unsigned char *segment, uint32_t pointer_index,
                              uint32_t word) {
  store_u64_le(segment + pointer_index * 8u,
               list_ptr(pointer_index, word, 6u, 2u));
  uint32_t text_word = word + 2u;
  text_word = set_text(segment, word + 0u, text_word, "dft; xc b3lyp; end");
  text_word = set_text(segment, word + 1u, text_word, "set int:acc_std 1e-8");
  return text_word;
}

int main(void) {
  unsigned char message[512];
  memset(message, 0, sizeof(message));

  store_u32_le(message + 0, 0);  /* one segment minus one */
  store_u32_le(message + 4, 63); /* words in segment */
  unsigned char *segment = message + 8;
  store_u64_le(segment + 0, struct_ptr(2, 10));
  store_u64_le(segment + 8,
               ((uint64_t)(3u) << 32) | (uint32_t)(-1)); /* mult=2, charge=-1 */
  store_u64_le(segment + 16, 1024u);                     /* memoryMb */

  uint32_t word = 13;
  word = set_text(segment, 3, word, "def2-svp");
  word = set_text(segment, 4, word, "dft");
  word = set_text(segment, 5, word, "b3lyp");
  word = set_text(segment, 6, word, "/engine/libnwchemc.so");
  word = set_text(segment, 7, word, "/opt/nwchem");
  word = set_text(segment, 8, word, "gradient");
  word = set_text(segment, 9, word, "water");
  word = set_text(segment, 10, word, "/scratch/nw");
  word = set_text(segment, 11, word, "/perm/nw");
  word = set_text_list(segment, 12, word);

  NWChemCParams params;
  if (nwchemc_params_parse_flat(message, sizeof(message), &params) != 0) {
    fprintf(stderr, "parse failed\n");
    return 1;
  }

  if (strcmp(params.basis, "def2-svp") != 0 ||
      strcmp(params.theory, "dft") != 0 ||
      strcmp(params.scf_type, "b3lyp") != 0 ||
      params.charge != -1 || params.multiplicity != 2 ||
      params.memory_mb != 1024 ||
      strcmp(params.nwchem_root, "/opt/nwchem") != 0 ||
      strcmp(params.scratch_dir, "/scratch/nw") != 0 ||
      strcmp(params.permanent_dir, "/perm/nw") != 0 ||
      strstr(params.input_blocks, "xc b3lyp") == NULL ||
      strstr(params.input_blocks, "acc_std") == NULL) {
    fprintf(stderr, "parsed params mismatch\n");
    return 1;
  }
  return 0;
}
