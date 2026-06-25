#include "nwchemc_params.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static uint32_t load_u32_le(const unsigned char *p) {
  return ((uint32_t)p[0]) | ((uint32_t)p[1] << 8) |
         ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static uint64_t load_u64_le(const unsigned char *p) {
  return ((uint64_t)load_u32_le(p)) | ((uint64_t)load_u32_le(p + 4) << 32);
}

static int32_t sign_extend_30(uint32_t value) {
  if (value & 0x20000000u)
    return (int32_t)(value | 0xc0000000u);
  return (int32_t)value;
}

static int32_t decode_i32(uint32_t stored, uint32_t default_value) {
  uint32_t value = stored ^ default_value;
  int32_t out;
  memcpy(&out, &value, sizeof(out));
  return out;
}

static void copy_text(char *dst, size_t dst_size, const char *src) {
  if (!dst || dst_size == 0)
    return;
  if (!src) {
    dst[0] = '\0';
    return;
  }
  snprintf(dst, dst_size, "%s", src);
}

void nwchemc_params_default(NWChemCParams *params) {
  if (!params)
    return;
  memset(params, 0, sizeof(*params));
  copy_text(params->basis, sizeof(params->basis), "sto-3g");
  copy_text(params->theory, sizeof(params->theory), "scf");
  copy_text(params->scf_type, sizeof(params->scf_type), "rhf");
  params->charge = 0;
  params->multiplicity = 1;
  copy_text(params->task, sizeof(params->task), "gradient");
}

static int parse_text_pointer(const unsigned char *segment,
                              size_t segment_words, size_t pointer_index,
                              char *dst, size_t dst_size) {
  const uint64_t ptr = load_u64_le(segment + pointer_index * 8u);
  if (ptr == 0)
    return 0;
  if ((ptr & 3u) != 1u)
    return -1;

  const int32_t off = sign_extend_30((uint32_t)((ptr >> 2) & 0x3fffffffu));
  const uint32_t elem_size = (uint32_t)((ptr >> 32) & 7u);
  const uint64_t elem_count = ptr >> 35;
  if (elem_size != 2u || elem_count == 0)
    return -1;

  const int64_t start = (int64_t)pointer_index + 1 + (int64_t)off;
  if (start < 0 || (uint64_t)start > (uint64_t)segment_words)
    return -1;
  if (elem_count > (uint64_t)segment_words * 8u)
    return -1;

  const uint64_t byte_start = (uint64_t)start * 8u;
  const uint64_t byte_end = byte_start + elem_count;
  if (byte_end > (uint64_t)segment_words * 8u)
    return -1;

  size_t n = (size_t)elem_count - 1u;
  if (n >= dst_size)
    n = dst_size - 1u;
  memcpy(dst, segment + byte_start, n);
  dst[n] = '\0';
  return 0;
}

static int append_block(char *dst, size_t dst_size, const char *block) {
  size_t used = strlen(dst);
  size_t n = strlen(block);
  if (n == 0)
    return 0;
  if (used != 0) {
    if (used + 1 >= dst_size)
      return -1;
    dst[used++] = '\n';
    dst[used] = '\0';
  }
  if (used + n >= dst_size)
    return -1;
  memcpy(dst + used, block, n + 1u);
  return 0;
}

static int parse_text_list_pointer(const unsigned char *segment,
                                   size_t segment_words, size_t pointer_index,
                                   char *dst, size_t dst_size) {
  const uint64_t ptr = load_u64_le(segment + pointer_index * 8u);
  if (ptr == 0)
    return 0;
  if ((ptr & 3u) != 1u)
    return -1;

  const int32_t off = sign_extend_30((uint32_t)((ptr >> 2) & 0x3fffffffu));
  const uint32_t elem_size = (uint32_t)((ptr >> 32) & 7u);
  const uint64_t elem_count = ptr >> 35;
  if (elem_size != 6u)
    return -1;

  const int64_t start = (int64_t)pointer_index + 1 + (int64_t)off;
  if (start < 0 || (uint64_t)start > (uint64_t)segment_words)
    return -1;
  if ((uint64_t)start + elem_count > (uint64_t)segment_words)
    return -1;

  dst[0] = '\0';
  for (uint64_t i = 0; i < elem_count; ++i) {
    char block[1024];
    block[0] = '\0';
    if (parse_text_pointer(segment, segment_words, (size_t)start + (size_t)i,
                           block, sizeof(block)) != 0)
      return -1;
    if (append_block(dst, dst_size, block) != 0)
      return -1;
  }
  return 0;
}

int nwchemc_params_parse_flat(const void *params_capnp,
                              size_t params_capnp_size_bytes,
                              NWChemCParams *params) {
  if (!params_capnp || params_capnp_size_bytes < 16u ||
      (params_capnp_size_bytes % 8u) != 0u || !params) {
    return -1;
  }

  const unsigned char *bytes = (const unsigned char *)params_capnp;
  const uint32_t segment_count = load_u32_le(bytes) + 1u;
  if (segment_count != 1u)
    return -1;

  const uint32_t segment_words = load_u32_le(bytes + 4u);
  if (segment_words == 0u)
    return -1;
  if (params_capnp_size_bytes < 8u + (size_t)segment_words * 8u)
    return -1;

  const unsigned char *segment = bytes + 8u;
  const uint64_t root = load_u64_le(segment);
  if ((root & 3u) != 0u)
    return -1;

  const int32_t off = sign_extend_30((uint32_t)((root >> 2) & 0x3fffffffu));
  const uint32_t data_words = (uint32_t)((root >> 32) & 0xffffu);
  const uint32_t pointer_count = (uint32_t)((root >> 48) & 0xffffu);
  const int64_t data_index_signed = 1 + (int64_t)off;
  if (data_index_signed < 0)
    return -1;
  const size_t data_index = (size_t)data_index_signed;
  if ((uint64_t)data_index + data_words + pointer_count >
      (uint64_t)segment_words)
    return -1;
  if (data_words < 1u || pointer_count < 5u)
    return -1;

  nwchemc_params_default(params);
  const uint64_t data0 = load_u64_le(segment + data_index * 8u);
  params->charge = decode_i32((uint32_t)(data0 & 0xffffffffu), 0u);
  params->multiplicity = decode_i32((uint32_t)(data0 >> 32), 1u);
  if (params->multiplicity <= 0)
    params->multiplicity = 1;

  if (data_words >= 2u) {
    const uint64_t data1 = load_u64_le(segment + (data_index + 1u) * 8u);
    params->memory_mb = (uint32_t)(data1 & 0xffffffffu);
  }

  const size_t pointer_index = data_index + data_words;
  if (pointer_count > 0u &&
      parse_text_pointer(segment, segment_words, pointer_index + 0u,
                         params->basis, sizeof(params->basis)) != 0)
    return -1;
  if (pointer_count > 1u &&
      parse_text_pointer(segment, segment_words, pointer_index + 1u,
                         params->theory, sizeof(params->theory)) != 0)
    return -1;
  if (pointer_count > 2u &&
      parse_text_pointer(segment, segment_words, pointer_index + 2u,
                         params->scf_type, sizeof(params->scf_type)) != 0)
    return -1;
  if (pointer_count > 3u &&
      parse_text_pointer(segment, segment_words, pointer_index + 3u,
                         params->engine_path, sizeof(params->engine_path)) != 0)
    return -1;
  if (pointer_count > 4u &&
      parse_text_pointer(segment, segment_words, pointer_index + 4u,
                         params->nwchem_root, sizeof(params->nwchem_root)) != 0)
    return -1;
  if (pointer_count > 5u &&
      parse_text_pointer(segment, segment_words, pointer_index + 5u,
                         params->task, sizeof(params->task)) != 0)
    return -1;
  if (pointer_count > 6u &&
      parse_text_pointer(segment, segment_words, pointer_index + 6u,
                         params->title, sizeof(params->title)) != 0)
    return -1;
  if (pointer_count > 7u &&
      parse_text_pointer(segment, segment_words, pointer_index + 7u,
                         params->scratch_dir, sizeof(params->scratch_dir)) != 0)
    return -1;
  if (pointer_count > 8u &&
      parse_text_pointer(segment, segment_words, pointer_index + 8u,
                         params->permanent_dir,
                         sizeof(params->permanent_dir)) != 0)
    return -1;
  if (pointer_count > 9u &&
      parse_text_list_pointer(segment, segment_words, pointer_index + 9u,
                              params->input_blocks,
                              sizeof(params->input_blocks)) != 0)
    return -1;

  if (params->basis[0] == '\0')
    copy_text(params->basis, sizeof(params->basis), "sto-3g");
  if (params->theory[0] == '\0')
    copy_text(params->theory, sizeof(params->theory), "scf");
  if (params->scf_type[0] == '\0')
    copy_text(params->scf_type, sizeof(params->scf_type), "rhf");
  if (params->task[0] == '\0')
    copy_text(params->task, sizeof(params->task), "gradient");
  return 0;
}
