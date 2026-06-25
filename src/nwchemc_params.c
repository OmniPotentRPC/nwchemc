#include "nwchemc_params.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct CapnpStructRef {
  size_t data_index;
  uint32_t data_words;
  size_t pointer_index;
  uint32_t pointer_count;
} CapnpStructRef;

typedef struct CapnpStructList {
  size_t first_element;
  uint32_t element_count;
  uint32_t data_words;
  uint32_t pointer_count;
} CapnpStructList;

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

static double decode_f64(uint64_t stored, uint64_t default_value) {
  uint64_t value = stored ^ default_value;
  double out;
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

static int parse_struct_pointer(const unsigned char *segment,
                                size_t segment_words, size_t pointer_index,
                                CapnpStructRef *ref) {
  const uint64_t ptr = load_u64_le(segment + pointer_index * 8u);
  if (ptr == 0)
    return 1;
  if ((ptr & 3u) != 0u)
    return -1;

  const int32_t off = sign_extend_30((uint32_t)((ptr >> 2) & 0x3fffffffu));
  const uint32_t data_words = (uint32_t)((ptr >> 32) & 0xffffu);
  const uint32_t pointer_count = (uint32_t)((ptr >> 48) & 0xffffu);
  const int64_t data_index_signed = (int64_t)pointer_index + 1 + (int64_t)off;
  if (data_index_signed < 0)
    return -1;
  const size_t data_index = (size_t)data_index_signed;
  if ((uint64_t)data_index + data_words + pointer_count >
      (uint64_t)segment_words)
    return -1;

  ref->data_index = data_index;
  ref->data_words = data_words;
  ref->pointer_index = data_index + data_words;
  ref->pointer_count = pointer_count;
  return 0;
}

static int parse_struct_list_pointer(const unsigned char *segment,
                                     size_t segment_words, size_t pointer_index,
                                     CapnpStructList *list) {
  memset(list, 0, sizeof(*list));
  const uint64_t ptr = load_u64_le(segment + pointer_index * 8u);
  if (ptr == 0)
    return 0;
  if ((ptr & 3u) != 1u)
    return -1;

  const int32_t off = sign_extend_30((uint32_t)((ptr >> 2) & 0x3fffffffu));
  const uint32_t elem_size = (uint32_t)((ptr >> 32) & 7u);
  const uint64_t word_count = ptr >> 35;
  if (elem_size != 7u)
    return -1;

  const int64_t tag_index_signed = (int64_t)pointer_index + 1 + (int64_t)off;
  if (tag_index_signed < 0)
    return -1;
  const size_t tag_index = (size_t)tag_index_signed;
  if ((uint64_t)tag_index >= (uint64_t)segment_words)
    return -1;

  const uint64_t tag = load_u64_le(segment + tag_index * 8u);
  if ((tag & 3u) != 0u)
    return -1;
  const uint32_t element_count = (uint32_t)((tag >> 2) & 0x3fffffffu);
  const uint32_t data_words = (uint32_t)((tag >> 32) & 0xffffu);
  const uint32_t pointer_count = (uint32_t)((tag >> 48) & 0xffffu);
  const uint64_t words_per_element =
      (uint64_t)data_words + (uint64_t)pointer_count;
  const uint64_t actual_words = (uint64_t)element_count * words_per_element;
  if (word_count != actual_words)
    return -1;
  if ((uint64_t)tag_index + 1u + actual_words > (uint64_t)segment_words)
    return -1;

  list->first_element = tag_index + 1u;
  list->element_count = element_count;
  list->data_words = data_words;
  list->pointer_count = pointer_count;
  return 0;
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

static int append_format(char *dst, size_t dst_size, const char *fmt, ...) {
  size_t used = strlen(dst);
  if (used >= dst_size)
    return -1;
  va_list ap;
  va_start(ap, fmt);
  int n = vsnprintf(dst + used, dst_size - used, fmt, ap);
  va_end(ap);
  if (n < 0 || (size_t)n >= dst_size - used)
    return -1;
  return 0;
}

static int append_text_args(const unsigned char *segment, size_t segment_words,
                            size_t pointer_index, char *dst,
                            size_t dst_size) {
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

  for (uint64_t i = 0; i < elem_count; ++i) {
    char arg[128];
    arg[0] = '\0';
    if (parse_text_pointer(segment, segment_words, (size_t)start + (size_t)i,
                           arg, sizeof(arg)) != 0)
      return -1;
    if (arg[0] != '\0' && append_format(dst, dst_size, " %s", arg) != 0)
      return -1;
  }
  return 0;
}

static int render_directives(const unsigned char *segment, size_t segment_words,
                             size_t pointer_index, char *dst, size_t dst_size,
                             const char *indent) {
  CapnpStructList list;
  if (parse_struct_list_pointer(segment, segment_words, pointer_index, &list) !=
      0)
    return -1;
  for (uint32_t i = 0; i < list.element_count; ++i) {
    const uint64_t stride =
        (uint64_t)list.data_words + (uint64_t)list.pointer_count;
    const size_t data_index = list.first_element + (size_t)i * (size_t)stride;
    const size_t ptr_index = data_index + list.data_words;
    if (list.pointer_count == 0)
      continue;
    char keyword[128];
    keyword[0] = '\0';
    if (parse_text_pointer(segment, segment_words, ptr_index + 0u, keyword,
                           sizeof(keyword)) != 0)
      return -1;
    if (keyword[0] == '\0')
      continue;
    if (append_format(dst, dst_size, "%s%s", indent, keyword) != 0)
      return -1;
    if (list.pointer_count > 1u &&
        append_text_args(segment, segment_words, ptr_index + 1u, dst,
                         dst_size) != 0)
      return -1;
    if (append_format(dst, dst_size, "\n") != 0)
      return -1;
  }
  return 0;
}

static int render_dft_stanza(const unsigned char *segment, size_t segment_words,
                             size_t pointer_index, char *dst,
                             size_t dst_size) {
  CapnpStructRef ref;
  int rc = parse_struct_pointer(segment, segment_words, pointer_index, &ref);
  if (rc != 0)
    return rc < 0 ? -1 : 0;

  char block[2048];
  char xc[128];
  block[0] = '\0';
  xc[0] = '\0';
  if (append_format(block, sizeof(block), "dft\n") != 0)
    return -1;
  if (ref.data_words > 0u) {
    const uint64_t data0 = load_u64_le(segment + ref.data_index * 8u);
    if ((data0 & 1u) != 0u &&
        append_format(block, sizeof(block), "  direct\n") != 0)
      return -1;
  }
  if (ref.pointer_count > 1u) {
    CapnpStructRef smear;
    rc = parse_struct_pointer(segment, segment_words, ref.pointer_index + 1u,
                              &smear);
    if (rc < 0)
      return -1;
    if (rc == 0 && smear.data_words > 0u) {
      const uint64_t sigma_bits =
          load_u64_le(segment + smear.data_index * 8u);
      const double sigma = decode_f64(sigma_bits, 0u);
      uint32_t mode = 0u;
      if (smear.data_words > 1u) {
        const uint64_t mode_word =
            load_u64_le(segment + (smear.data_index + 1u) * 8u);
        mode = (uint32_t)(mode_word & 0xffffu);
      }
      if (sigma != 0.0 &&
          append_format(block, sizeof(block), "  smear %.12g %s\n", sigma,
                        mode == 1u ? "nofixsz" : "fixsz") != 0)
        return -1;
    }
  }
  if (ref.pointer_count > 0u &&
      parse_text_pointer(segment, segment_words, ref.pointer_index + 0u, xc,
                         sizeof(xc)) != 0)
    return -1;
  if (xc[0] != '\0' &&
      append_format(block, sizeof(block), "  xc %s\n", xc) != 0)
    return -1;
  if (ref.pointer_count > 2u &&
      render_directives(segment, segment_words, ref.pointer_index + 2u, block,
                        sizeof(block), "  ") != 0)
    return -1;
  if (append_format(block, sizeof(block), "end") != 0)
    return -1;
  return append_block(dst, dst_size, block);
}

static int render_generic_stanza(const unsigned char *segment,
                                 size_t segment_words, size_t pointer_index,
                                 char *dst, size_t dst_size) {
  CapnpStructRef ref;
  int rc = parse_struct_pointer(segment, segment_words, pointer_index, &ref);
  if (rc != 0)
    return rc < 0 ? -1 : 0;
  if (ref.pointer_count == 0)
    return 0;

  char name[128];
  char block[2048];
  name[0] = '\0';
  block[0] = '\0';
  if (parse_text_pointer(segment, segment_words, ref.pointer_index + 0u, name,
                         sizeof(name)) != 0)
    return -1;
  if (name[0] == '\0')
    return 0;
  if (append_format(block, sizeof(block), "%s\n", name) != 0)
    return -1;
  if (ref.pointer_count > 1u &&
      render_directives(segment, segment_words, ref.pointer_index + 1u, block,
                        sizeof(block), "  ") != 0)
    return -1;
  if (append_format(block, sizeof(block), "end") != 0)
    return -1;
  return append_block(dst, dst_size, block);
}

static int render_set_stanza(const unsigned char *segment, size_t segment_words,
                             size_t pointer_index, char *dst,
                             size_t dst_size) {
  CapnpStructRef ref;
  int rc = parse_struct_pointer(segment, segment_words, pointer_index, &ref);
  if (rc != 0)
    return rc < 0 ? -1 : 0;
  if (ref.pointer_count < 2u)
    return 0;

  char key[128];
  char value[128];
  char line[320];
  key[0] = '\0';
  value[0] = '\0';
  line[0] = '\0';
  if (parse_text_pointer(segment, segment_words, ref.pointer_index + 0u, key,
                         sizeof(key)) != 0 ||
      parse_text_pointer(segment, segment_words, ref.pointer_index + 1u, value,
                         sizeof(value)) != 0)
    return -1;
  if (key[0] == '\0')
    return 0;
  if (append_format(line, sizeof(line), "set %s %s", key, value) != 0)
    return -1;
  return append_block(dst, dst_size, line);
}

static int render_input_stanzas(const unsigned char *segment,
                                size_t segment_words, size_t pointer_index,
                                char *dst, size_t dst_size) {
  CapnpStructList list;
  if (parse_struct_list_pointer(segment, segment_words, pointer_index, &list) !=
      0)
    return -1;
  for (uint32_t i = 0; i < list.element_count; ++i) {
    const uint64_t stride =
        (uint64_t)list.data_words + (uint64_t)list.pointer_count;
    const size_t data_index = list.first_element + (size_t)i * (size_t)stride;
    const size_t ptr_index = data_index + list.data_words;
    uint32_t kind = 0u;
    if (list.data_words > 0u) {
      const uint64_t data0 = load_u64_le(segment + data_index * 8u);
      kind = (uint32_t)(data0 & 0xffffu);
    }
    if (kind == 1u) {
      if (list.pointer_count > 1u &&
          render_dft_stanza(segment, segment_words, ptr_index + 1u, dst,
                            dst_size) != 0)
        return -1;
    } else if (kind == 2u) {
      if (list.pointer_count > 2u &&
          render_set_stanza(segment, segment_words, ptr_index + 2u, dst,
                            dst_size) != 0)
        return -1;
    } else if (kind == 3u) {
      if (list.pointer_count > 3u) {
        char block[1024];
        block[0] = '\0';
        if (parse_text_pointer(segment, segment_words, ptr_index + 3u, block,
                               sizeof(block)) != 0)
          return -1;
        if (append_block(dst, dst_size, block) != 0)
          return -1;
      }
    } else if (list.pointer_count > 0u &&
               render_generic_stanza(segment, segment_words, ptr_index + 0u,
                                     dst, dst_size) != 0) {
      return -1;
    }
  }
  return 0;
}

static int parse_text_list_pointer(const unsigned char *segment,
                                   size_t segment_words, size_t pointer_index,
                                   char *dst, size_t dst_size,
                                   int clear_first) {
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

  if (clear_first)
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
  params->input_blocks[0] = '\0';
  if (pointer_count > 10u &&
      render_input_stanzas(segment, segment_words, pointer_index + 10u,
                           params->input_blocks,
                           sizeof(params->input_blocks)) != 0)
    return -1;
  if (pointer_count > 9u &&
      parse_text_list_pointer(segment, segment_words, pointer_index + 9u,
                              params->input_blocks,
                              sizeof(params->input_blocks), 0) != 0)
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
