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

typedef struct NWChemNwpwScfNumericControls {
  int kerker_g0_set;
  double kerker_g0;
  int ks_alpha_set;
  double ks_alpha;
  int ks_maxit_orb_set;
  int ks_maxit_orb;
  int ks_maxit_orbs_set;
  int ks_maxit_orbs;
  int diis_histories_set;
  int diis_histories;
} NWChemNwpwScfNumericControls;

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

int nwchemc_params_extract_direct_nwpw_electric_field(
    NWChemParams_ptr params, int *has_options, int *atom_efield,
    int *atom_efield_gradient);

int nwchemc_params_extract_direct_nwpw_mulliken(
    NWChemParams_ptr params, int *has_options, int *mulliken,
    int *mulliken_kawai);

int nwchemc_params_extract_direct_nwpw_periodic_dipole(
    NWChemParams_ptr params, int *has_options, int *periodic_dipole);

int nwchemc_params_extract_direct_nwpw_efield(
    NWChemParams_ptr params, int *has_options, int *efield,
    double efield_vector[3], int *has_center, double efield_center[3],
    int *efield_type);

int nwchemc_params_extract_direct_nwpw_smooth_cutoff(
    NWChemParams_ptr params, int *has_options, int *smooth_cutoff,
    double values[2]);

int nwchemc_params_extract_direct_nwpw_cutoff_boot_wavefunction(
    NWChemParams_ptr params, int *has_options, int *cutoff_boot_wavefunction);

int nwchemc_params_extract_direct_nwpw_fast_erf(NWChemParams_ptr params,
                                                int *has_options,
                                                int *fast_erf);

int nwchemc_params_extract_direct_nwpw_dipole_motion(
    NWChemParams_ptr params, int *has_options, int *dipole_motion,
    capn_text *filename);

int nwchemc_params_extract_direct_nwpw_rho_use_symmetry(
    NWChemParams_ptr params, int *has_options, int *rho_use_symmetry);

int nwchemc_params_extract_direct_nwpw_one_electron_guess(
    NWChemParams_ptr params, int *has_options, int *it_in, int *it_out,
    int *it_ortho);

int nwchemc_params_extract_direct_nwpw_fmm(NWChemParams_ptr params,
                                           int *has_options, int *fmm,
                                           int *fmm_lmax,
                                           int *fmm_long_range);

int nwchemc_params_extract_direct_nwpw_born(
    NWChemParams_ptr params, int *has_options, int *born, double *dielectric,
    int *relax, double *vradii_angstrom, size_t vradii_capacity,
    size_t *vradii_count);

int nwchemc_params_extract_direct_nwpw_vfield(
    NWChemParams_ptr params, int *has_options, capn_text *filenames,
    size_t filename_capacity, size_t *filename_count);

int nwchemc_params_extract_direct_nwpw_single_precision_hfx(
    NWChemParams_ptr params, int *has_options, int *single_precision_hfx);

int nwchemc_params_extract_direct_nwpw_geometry_optimize(
    NWChemParams_ptr params, int *has_options, int *geometry_optimize);

int nwchemc_params_extract_direct_nwpw_auxiliary_potentials(
    NWChemParams_ptr params, int *has_options, int *auxiliary_potentials);

int nwchemc_params_extract_direct_nwpw_multiplicity(
    NWChemParams_ptr params, int *has_options, int *multiplicity, int *ispin);

int nwchemc_params_extract_direct_nwpw_allow_translation(
    NWChemParams_ptr params, int *has_options, int *allow_translation);

int nwchemc_params_extract_direct_nwpw_spin_mode(NWChemParams_ptr params,
                                                 int *has_options,
                                                 int *spin_mode, int *ispin);

int nwchemc_params_extract_direct_nwpw_spin_ispins(
    NWChemParams_ptr params, int *ispins, size_t capacity, size_t *count);

int nwchemc_params_extract_direct_nwpw_dos(
    NWChemParams_ptr params, int *has_options, int *dos_alpha_set,
    double *dos_alpha, int *dos_npoints_set, int *dos_npoints,
    int *dos_emin_set, double *dos_emin, int *dos_emax_set, double *dos_emax,
    capn_text *dos_filename);

int nwchemc_params_extract_direct_nwpw_cpmd_grid(
    NWChemParams_ptr params, int *has_options, int *cpmd_properties,
    int *use_grid_comparison);

int nwchemc_params_extract_direct_nwpw_director(
    NWChemParams_ptr params, int *has_options, int *director,
    capn_text *filename);

int nwchemc_params_extract_direct_nwpw_cell_mapping(
    NWChemParams_ptr params, int *has_options, int cell_expand[3],
    int *mapping);

int nwchemc_params_extract_direct_nwpw_rotation_multipole(
    NWChemParams_ptr params, int *has_options, int *rotation,
    int *lmax_multipole);

int nwchemc_params_extract_direct_nwpw_fei(
    NWChemParams_ptr params, int *has_options, int *fei, capn_text *filename);

int nwchemc_params_extract_direct_nwpw_et(
    NWChemParams_ptr params, int *has_options, capn_text *movecs_a,
    capn_text *movecs_b, capn_text *ion_a, capn_text *ion_b);

int nwchemc_params_extract_direct_nwpw_initial_velocities(
    NWChemParams_ptr params, int *has_options, double *temperature, int *seed);

int nwchemc_params_extract_direct_nwpw_make_hmass2(
    NWChemParams_ptr params, int *has_options, int *make_hmass2);

int nwchemc_params_extract_direct_nwpw_translate_vector(
    NWChemParams_ptr params, int *has_options, double vector[3],
    capn_text *geometry_name, int *reorder);

int nwchemc_params_extract_direct_nwpw_socket(
    NWChemParams_ptr params, int *has_options, capn_text *socket_type,
    capn_text *socket_ip);

int nwchemc_params_extract_direct_nwpw_apc(
    NWChemParams_ptr params, int *has_options, double *gc, double *gamma,
    size_t gamma_capacity, size_t *gamma_count);

int nwchemc_params_extract_direct_nwpw_translation(
    NWChemParams_ptr params, int *has_options, int *translation);

int nwchemc_params_extract_direct_nwpw_minimizer(
    NWChemParams_ptr params, int *has_options, int *minimizer);

int nwchemc_params_extract_direct_nwpw_scf_algorithms(
    NWChemParams_ptr params, int *has_options, int *ks_algorithm,
    int *scf_algorithm, int *precondition);

int nwchemc_params_extract_direct_nwpw_scf_numeric(
    NWChemParams_ptr params, int *has_options,
    NWChemNwpwScfNumericControls *controls);

int nwchemc_params_extract_direct_brillouin_zone(
    NWChemParams_ptr params, int *has_options, capn_text *zone_name,
    int monkhorst_pack[3], int *max_kpoints_print, double *kvectors,
    size_t kvector_capacity, size_t *kvector_count);

int nwchemc_params_extract_direct_brillouin_tetrahedron(
    NWChemParams_ptr params, int *has_options, int tetrahedron_grid[3]);

int nwchemc_params_extract_direct_brillouin_dos_grid(
    NWChemParams_ptr params, int *has_options, int dos_grid[3]);

typedef int (*nwchemc_params_direct_pseudopotential_fn)(
    void *user_data, capn_text target,
    const struct NWChemPseudopotentialEntry *entry);

int nwchemc_params_for_each_direct_pseudopotential(
    NWChemParams_ptr params, nwchemc_params_direct_pseudopotential_fn callback,
    void *user_data, size_t *count);

typedef int (*nwchemc_params_pseudopotential_spin_rule_fn)(
    void *user_data, size_t rule_index,
    const struct NWChemPseudopotentialSpinRule *rule);

int nwchemc_params_for_each_direct_pseudopotential_spin_rule(
    NWChemParams_ptr params,
    nwchemc_params_pseudopotential_spin_rule_fn callback, void *user_data,
    size_t *count);

typedef int (*nwchemc_params_pseudopotential_uterm_rule_fn)(
    void *user_data, size_t rule_index,
    const struct NWChemPseudopotentialUtermRule *rule);

int nwchemc_params_for_each_direct_pseudopotential_uterm_rule(
    NWChemParams_ptr params,
    nwchemc_params_pseudopotential_uterm_rule_fn callback, void *user_data,
    size_t *count);

int nwchemc_params_extract_direct_pseudopotentials(
    NWChemParams_ptr params, capn_text *elements, int *library_types,
    capn_text *library_names, size_t capacity, size_t *count);

int nwchemc_params_extract_direct_pseudopotential_spin(
    NWChemParams_ptr params, int *has_options, int *pspspin_enabled,
    int *pspspin_count, int *semicore_small);

int nwchemc_params_extract_direct_pseudopotential_uterm(
    NWChemParams_ptr params, int *has_options, int *uterm_enabled,
    int *uterm_count);

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

int nwchemc_force_input_hessian_result_factors(ForceInput_ptr force_input,
                                               double *energy_factor,
                                               double *hessian_factor);

int nwchemc_force_input_stress_result_factors(ForceInput_ptr force_input,
                                              double *energy_factor,
                                              double *stress_factor);

int nwchemc_force_input_position_result_factors(ForceInput_ptr force_input,
                                                double *energy_factor,
                                                double *position_factor);

size_t nwchemc_potential_result_flat_size(size_t force_count);

size_t nwchemc_gradient_result_flat_size(size_t gradient_count);

size_t nwchemc_hessian_result_flat_size(size_t hessian_count);

size_t nwchemc_dipole_result_flat_size(void);

size_t nwchemc_quadrupole_result_flat_size(void);

size_t nwchemc_stress_result_flat_size(void);

size_t nwchemc_polarizability_result_flat_size(void);

size_t nwchemc_optimize_result_flat_size(size_t position_count);

size_t nwchemc_frequencies_result_flat_size(size_t frequency_count);

int nwchemc_potential_result_write(double energy, const double *forces,
                                   size_t force_count,
                                   void *potential_result_capnp,
                                   size_t potential_result_capacity_bytes,
                                   size_t *potential_result_size_bytes);

int nwchemc_potential_result_write_gradient(
    double energy, const double *gradient, size_t gradient_count,
    void *potential_result_capnp, size_t potential_result_capacity_bytes,
    size_t *potential_result_size_bytes);

int nwchemc_potential_result_write_hessian(
    double energy, const double *hessian, size_t hessian_count,
    void *potential_result_capnp, size_t potential_result_capacity_bytes,
    size_t *potential_result_size_bytes);

int nwchemc_potential_result_write_dipole(
    double energy, const double *dipole, void *potential_result_capnp,
    size_t potential_result_capacity_bytes,
    size_t *potential_result_size_bytes);

int nwchemc_potential_result_write_polarizability(
    double energy, const double *polarizability,
    void *potential_result_capnp, size_t potential_result_capacity_bytes,
    size_t *potential_result_size_bytes);

int nwchemc_potential_result_write_quadrupole(
    double energy, const double *quadrupole, void *potential_result_capnp,
    size_t potential_result_capacity_bytes,
    size_t *potential_result_size_bytes);

int nwchemc_potential_result_write_stress(
    double energy, const double *stress, void *potential_result_capnp,
    size_t potential_result_capacity_bytes,
    size_t *potential_result_size_bytes);

int nwchemc_potential_result_write_optimized(
    double energy, const double *optimized_positions, size_t position_count,
    void *potential_result_capnp, size_t potential_result_capacity_bytes,
    size_t *potential_result_size_bytes);

int nwchemc_potential_result_write_frequencies(
    double energy, const double *frequencies, const double *intensities,
    const double *normal_modes, const double *thermochemistry,
    const double *projected_frequencies,
    const double *projected_intensities, size_t frequency_count,
    void *potential_result_capnp, size_t potential_result_capacity_bytes,
    size_t *potential_result_size_bytes);
