#pragma once

#include "Potentials.capnp.h"

#include <stddef.h>

#define NWCHEMC_BLOCKS 8192
#define NWCHEMC_DRIVER_TOLERANCE_NONE 0
#define NWCHEMC_DRIVER_TOLERANCE_TIGHT 1
#define NWCHEMC_DRIVER_TOLERANCE_LOOSE 2
#define NWCHEMC_DIRECT_SET_VALUE_AUTO 0
#define NWCHEMC_DIRECT_SET_VALUE_TEXT 1
#define NWCHEMC_DIRECT_SET_VALUE_DOUBLE 2
#define NWCHEMC_DIRECT_SET_VALUE_INTEGER 3
#define NWCHEMC_DIRECT_SET_VALUE_LOGICAL 4

int nwchemc_params_root(const void *params_capnp,
                        size_t params_capnp_size_bytes, struct capn *arena,
                        NWChemParams_ptr *params);

void nwchemc_params_release(struct capn *arena);

const char *nwchemc_params_text_or(capn_text text, const char *fallback);

int nwchemc_params_render_input_blocks(NWChemParams_ptr params, char *dst,
                                       size_t dst_size);

int nwchemc_params_render_embed_input_blocks(NWChemParams_ptr params, char *dst,
                                             size_t dst_size);

int nwchemc_params_extract_direct_dft(NWChemParams_ptr params, capn_text *xc,
                                      int *direct_enabled,
                                      int *smearing_enabled,
                                      double *smear_sigma_hartree,
                                      int *smearing_spinset);

int nwchemc_params_extract_direct_scf(NWChemParams_ptr params, int *has_options,
                                      int *maxiter, double *thresh,
                                      double *tol2e);

int nwchemc_params_extract_direct_driver(NWChemParams_ptr params,
                                         int *has_options, int *maxiter,
                                         int *tolerance_mode,
                                         double *gmax_tol,
                                         double *grms_tol,
                                         double *xmax_tol,
                                         double *xrms_tol);

int nwchemc_params_extract_direct_pseudopotentials(
    NWChemParams_ptr params, capn_text *elements, int *library_types,
    capn_text *library_names, size_t capacity, size_t *count);

int nwchemc_params_extract_direct_set_strings(NWChemParams_ptr params,
                                              capn_text *keys,
                                              capn_text *values,
                                              size_t capacity, size_t *count);

int nwchemc_params_extract_direct_set_values(
    NWChemParams_ptr params, capn_text *keys, int *value_types,
    int *value_counts, capn_text *values, size_t set_capacity,
    size_t value_capacity, size_t *count);

int nwchemc_force_input_root(const void *force_input_capnp,
                             size_t force_input_capnp_size_bytes,
                             struct capn *arena, ForceInput_ptr *force_input);

int nwchemc_force_input_atom_count(ForceInput_ptr force_input,
                                   size_t *n_atoms, int *has_cell);

int nwchemc_force_input_copy_geometry(ForceInput_ptr force_input,
                                      double *positions_ang,
                                      int *atomic_numbers,
                                      size_t atom_capacity, double *cell_ang,
                                      int *has_cell);

int nwchemc_force_input_result_factors(ForceInput_ptr force_input,
                                       double *energy_factor,
                                       double *force_factor);

size_t nwchemc_potential_result_flat_size(size_t force_count);

int nwchemc_potential_result_write(double energy, const double *forces,
                                   size_t force_count,
                                   void *potential_result_capnp,
                                   size_t potential_result_capacity_bytes,
                                   size_t *potential_result_size_bytes);
