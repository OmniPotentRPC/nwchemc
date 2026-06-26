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

int nwchemc_params_extract_direct_ccsd(
    NWChemParams_ptr params, int *has_options, int *maxiter, double *thresh,
    double *tol2e, int *iprt, int *max_diis, int *frozen_core,
    int *frozen_virtual, int *use_disk, double *same_spin_scale,
    double *opposite_spin_scale, int *use_trpdrv_nb, int *use_ccsd_omp,
    int *use_trpdrv_omp, int *use_trpdrv_offload);

int nwchemc_params_extract_direct_driver(NWChemParams_ptr params,
                                         int *has_options, int *maxiter,
                                         int *tolerance_mode,
                                         double *gmax_tol,
                                         double *grms_tol,
                                         double *xmax_tol,
                                         double *xrms_tol);

int nwchemc_params_extract_direct_nwpw(NWChemParams_ptr params,
                                       int *has_options,
                                       double *energy_cutoff,
                                       double *wavefunction_cutoff,
                                       double *ewald_rcut,
                                       int *ewald_ncut);

int nwchemc_params_extract_direct_nwpw_state(
    NWChemParams_ptr params, int *has_options, capn_text *cell_name,
    capn_text *input_wavefunction_filename,
    capn_text *output_wavefunction_filename, double *fake_mass,
    double *time_step, int *loop_start, int *loop_end, int *has_tolerances,
    double *tolerance_energy, double *tolerance_density,
    double *tolerance_gradient);

int nwchemc_params_extract_direct_nwpw_xc(NWChemParams_ptr params,
                                          int *has_options,
                                          capn_text *exchange_correlation);

int nwchemc_params_extract_direct_nwpw_bo(
    NWChemParams_ptr params, int *has_options, int *balance_mode,
    int *bo_step_start, int *bo_step_end, double *bo_time_step,
    int *bo_algorithm, double *bo_fake_mass, int *has_scaling,
    double *scaling_first, double *scaling_second);

int nwchemc_params_extract_direct_nwpw_execution(
    NWChemParams_ptr params, int *has_options, int *np_fft,
    int *np_orbital, int *np_kspace, int *spin_orbit, int *parallel_io);

int nwchemc_params_extract_direct_nwpw_filenames(
    NWChemParams_ptr params, int *has_options, capn_text *xyz_filename,
    capn_text *ion_motion_filename, capn_text *electron_motion_filename,
    capn_text *hamiltonian_motion_filename,
    capn_text *orbital_motion_filename,
    capn_text *eigenvalue_motion_filename);

int nwchemc_params_extract_direct_nwpw_fractional(
    NWChemParams_ptr params, int *has_fractional,
    int *fractional_orbitals_start, int *fractional_orbitals_end,
    int *has_smear, double *smear_temperature, double *smear_alpha,
    int *smear_type);

int nwchemc_params_extract_direct_nwpw_orbital_grid(
    NWChemParams_ptr params, int *has_options, int *virtual_orbitals_start,
    int *virtual_orbitals_end, int *lcao_mode, int *ewald_grid_x,
    int *ewald_grid_y, int *ewald_grid_z);

int nwchemc_params_extract_direct_nwpw_nose(
    NWChemParams_ptr params, int *has_options, int *nose_hoover,
    int *nose_restart, double *electron_period, double *electron_temperature,
    double *ion_period, double *ion_temperature, int *electron_chain_length,
    int *ion_chain_length);

int nwchemc_params_extract_direct_brillouin_zone(
    NWChemParams_ptr params, int *has_options, capn_text *zone_name,
    int monkhorst_pack[3], int *max_kpoints_print, double *kvectors,
    size_t kvector_capacity, size_t *kvector_count);

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
