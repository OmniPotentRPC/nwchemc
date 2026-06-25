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

int nwchemc_params_extract_direct_pseudopotentials(
    NWChemParams_ptr params, capn_text *elements, int *library_types,
    capn_text *library_names, size_t capacity, size_t *count);

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
