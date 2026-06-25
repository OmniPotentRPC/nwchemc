#pragma once

#include <stddef.h>
#include <stdint.h>

#define NWCHEMC_TEXT 128
#define NWCHEMC_PATH 512
#define NWCHEMC_BLOCKS 8192

typedef struct NWChemCParams {
  char basis[NWCHEMC_TEXT];
  char theory[NWCHEMC_TEXT];
  char scf_type[NWCHEMC_TEXT];
  int32_t charge;
  int32_t multiplicity;
  char engine_path[NWCHEMC_PATH];
  char nwchem_root[NWCHEMC_PATH];
  char task[NWCHEMC_TEXT];
  char title[NWCHEMC_TEXT];
  uint32_t memory_mb;
  char scratch_dir[NWCHEMC_PATH];
  char permanent_dir[NWCHEMC_PATH];
  char input_blocks[NWCHEMC_BLOCKS];
} NWChemCParams;

void nwchemc_params_default(NWChemCParams *params);

int nwchemc_params_parse_flat(const void *params_capnp,
                              size_t params_capnp_size_bytes,
                              NWChemCParams *params);
