#pragma once

#include "Potentials.capnp.h"

#include <stddef.h>

#define NWCHEMC_BLOCKS 8192

int nwchemc_params_root(const void *params_capnp,
                        size_t params_capnp_size_bytes, struct capn *arena,
                        NWChemParams_ptr *params);

void nwchemc_params_release(struct capn *arena);

const char *nwchemc_params_text_or(capn_text text, const char *fallback);

int nwchemc_params_render_input_blocks(NWChemParams_ptr params, char *dst,
                                       size_t dst_size);
