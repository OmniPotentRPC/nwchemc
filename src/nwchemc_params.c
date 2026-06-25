#include "nwchemc_params.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static const capn_text empty_text = {0, "", 0};

const char *nwchemc_params_text_or(capn_text text, const char *fallback) {
  if (text.str && text.len > 0)
    return text.str;
  return fallback;
}

static int append_bytes(char *dst, size_t dst_size, const char *src,
                        size_t n) {
  size_t used = strlen(dst);
  if (n == 0)
    return 0;
  if (used + n >= dst_size)
    return -1;
  memcpy(dst + used, src, n);
  dst[used + n] = '\0';
  return 0;
}

static int append_text(char *dst, size_t dst_size, capn_text text) {
  if (!text.str || text.len <= 0)
    return 0;
  return append_bytes(dst, dst_size, text.str, (size_t)text.len);
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
  return append_bytes(dst, dst_size, block, n);
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

static int pointer_list_len(capn_ptr *ptr) {
  capn_resolve(ptr);
  if (ptr->type == CAPN_NULL)
    return 0;
  if (ptr->type != CAPN_PTR_LIST)
    return -1;
  return ptr->len;
}

static int struct_list_len(capn_ptr *ptr) {
  capn_resolve(ptr);
  if (ptr->type == CAPN_NULL)
    return 0;
  if (ptr->type != CAPN_LIST)
    return -1;
  return ptr->len;
}

static int append_text_args(capn_ptr args, char *dst, size_t dst_size) {
  int n = pointer_list_len(&args);
  if (n < 0)
    return -1;
  for (int i = 0; i < n; ++i) {
    capn_text arg = capn_get_text(args, i, empty_text);
    if (arg.len > 0) {
      if (append_format(dst, dst_size, " ") != 0 ||
          append_text(dst, dst_size, arg) != 0)
        return -1;
    }
  }
  return 0;
}

static int render_directives(NWChemDirective_list directives, char *dst,
                             size_t dst_size, const char *indent) {
  int n = struct_list_len(&directives.p);
  if (n < 0)
    return -1;
  for (int i = 0; i < n; ++i) {
    struct NWChemDirective directive;
    get_NWChemDirective(&directive, directives, i);
    if (directive.keyword.len <= 0)
      continue;
    if (append_format(dst, dst_size, "%s", indent) != 0 ||
        append_text(dst, dst_size, directive.keyword) != 0 ||
        append_text_args(directive.args, dst, dst_size) != 0 ||
        append_format(dst, dst_size, "\n") != 0)
      return -1;
  }
  return 0;
}

static int render_generic_stanza(NWChemGenericStanza_ptr ptr, char *dst,
                                 size_t dst_size) {
  if (ptr.p.type == CAPN_NULL)
    return 0;

  struct NWChemGenericStanza generic;
  char block[2048];
  block[0] = '\0';
  read_NWChemGenericStanza(&generic, ptr);
  if (generic.name.len <= 0)
    return 0;
  if (append_text(block, sizeof(block), generic.name) != 0 ||
      append_format(block, sizeof(block), "\n") != 0 ||
      render_directives(generic.directives, block, sizeof(block), "  ") != 0 ||
      append_format(block, sizeof(block), "end") != 0)
    return -1;
  return append_block(dst, dst_size, block);
}

static int render_set_stanza(NWChemSetDirective_ptr ptr, char *dst,
                             size_t dst_size) {
  if (ptr.p.type == CAPN_NULL)
    return 0;

  struct NWChemSetDirective set;
  char block[512];
  block[0] = '\0';
  read_NWChemSetDirective(&set, ptr);
  if (set.key.len <= 0)
    return 0;
  if (append_format(block, sizeof(block), "set ") != 0 ||
      append_text(block, sizeof(block), set.key) != 0 ||
      append_format(block, sizeof(block), " ") != 0 ||
      append_text(block, sizeof(block), set.value) != 0)
    return -1;
  return append_block(dst, dst_size, block);
}

static int render_dft_stanza(NWChemDftStanza_ptr ptr, char *dst,
                             size_t dst_size) {
  if (ptr.p.type == CAPN_NULL)
    return 0;

  struct NWChemDftStanza dft;
  char block[2048];
  block[0] = '\0';
  read_NWChemDftStanza(&dft, ptr);
  if (append_format(block, sizeof(block), "dft\n") != 0)
    return -1;
  if (dft.direct &&
      append_format(block, sizeof(block), "  direct\n") != 0)
    return -1;
  if (dft.smearing.p.type != CAPN_NULL) {
    struct NWChemDftSmearing smearing;
    read_NWChemDftSmearing(&smearing, dft.smearing);
    if (smearing.sigmaHartree != 0.0 &&
        append_format(block, sizeof(block), "  smear %.12g %s\n",
                      smearing.sigmaHartree,
                      smearing.mode == NWChemDftSmearing_Mode_nofixsz
                          ? "nofixsz"
                          : "fixsz") != 0)
      return -1;
  }
  if (dft.xc.len > 0) {
    if (append_format(block, sizeof(block), "  xc ") != 0 ||
        append_text(block, sizeof(block), dft.xc) != 0 ||
        append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (render_directives(dft.directives, block, sizeof(block), "  ") != 0 ||
      append_format(block, sizeof(block), "end") != 0)
    return -1;
  return append_block(dst, dst_size, block);
}

static int render_input_stanzas(NWChemInputStanza_list stanzas, char *dst,
                                size_t dst_size) {
  int n = struct_list_len(&stanzas.p);
  if (n < 0)
    return -1;
  for (int i = 0; i < n; ++i) {
    struct NWChemInputStanza stanza;
    get_NWChemInputStanza(&stanza, stanzas, i);
    switch (stanza.kind) {
    case NWChemInputStanza_Kind_dft:
      if (render_dft_stanza(stanza.dft, dst, dst_size) != 0)
        return -1;
      break;
    case NWChemInputStanza_Kind_set:
      if (render_set_stanza(stanza.set, dst, dst_size) != 0)
        return -1;
      break;
    case NWChemInputStanza_Kind_raw:
      if (append_block(dst, dst_size,
                       nwchemc_params_text_or(stanza.raw, "")) != 0)
        return -1;
      break;
    case NWChemInputStanza_Kind_generic:
    default:
      if (render_generic_stanza(stanza.generic, dst, dst_size) != 0)
        return -1;
      break;
    }
  }
  return 0;
}

static int render_input_blocks(capn_ptr input_blocks, char *dst,
                               size_t dst_size) {
  int n = pointer_list_len(&input_blocks);
  if (n < 0)
    return -1;
  for (int i = 0; i < n; ++i) {
    capn_text block = capn_get_text(input_blocks, i, empty_text);
    if (append_block(dst, dst_size, nwchemc_params_text_or(block, "")) != 0)
      return -1;
  }
  return 0;
}

int nwchemc_params_render_input_blocks(const struct NWChemParams *params,
                                       char *dst, size_t dst_size) {
  if (!params || !dst || dst_size == 0)
    return -1;
  dst[0] = '\0';
  if (render_input_stanzas(params->inputStanzas, dst, dst_size) != 0)
    return -1;
  return render_input_blocks(params->inputBlocks, dst, dst_size);
}

int nwchemc_params_read(const void *params_capnp,
                        size_t params_capnp_size_bytes, struct capn *arena,
                        struct NWChemParams *params) {
  if (!params_capnp || params_capnp_size_bytes == 0 || !arena || !params)
    return -1;

  memset(arena, 0, sizeof(*arena));
  memset(params, 0, sizeof(*params));
  if (capn_init_mem(arena, (const uint8_t *)params_capnp,
                    params_capnp_size_bytes, 0) != 0)
    return -1;

  NWChemParams_ptr root;
  root.p = capn_getp(capn_root(arena), 0, 1);
  if (root.p.type != CAPN_STRUCT) {
    nwchemc_params_release(arena);
    return -1;
  }
  read_NWChemParams(params, root);
  return 0;
}

void nwchemc_params_release(struct capn *arena) {
  if (!arena)
    return;
  capn_free(arena);
  memset(arena, 0, sizeof(*arena));
}
