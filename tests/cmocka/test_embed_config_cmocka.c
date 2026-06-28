#include "nwchemc.h"
#include "nwchemc_params.h"

#include <cmocka.h>
#include <errno.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *g_params_path = NULL;
static const char *g_config_options_path = NULL;
static const char *g_pspspin_path = NULL;
static const char *g_pspspin_many_path = NULL;
static const char *g_nwpw_spin_mode_path = NULL;
static const char *g_nwpw_allow_translation_path = NULL;
static const char *g_nwpw_cutoff_alias_path = NULL;
static const char *g_nwpw_mc_steps_path = NULL;
static const char *g_nwpw_bo_steps_default_path = NULL;
static const char *g_nwpw_bo_time_step_default_path = NULL;
static const char *g_nwpw_bo_fake_mass_default_path = NULL;
static const char *g_nwpw_scaling_default_path = NULL;
static const char *g_nwpw_np_dimensions_default_path = NULL;
static const char *g_nwpw_tolerances_default_path = NULL;
static const char *g_nwpw_mc_steps_default_path = NULL;
static const char *g_brillouin_tetrahedron_path = NULL;
static const char *g_brillouin_dos_grid_path = NULL;
static const char *g_nwpw_et_path = NULL;
static const char *g_nwpw_temperature_path = NULL;
static const char *g_nwpw_dos_default_path = NULL;
static const char *g_nwpw_mapping_alias_path = NULL;
static const char *g_nwpw_mapping_default_path = NULL;
static const char *g_nwpw_virtual_alias_path = NULL;
static const char *g_nwpw_one_electron_guess_defaults_path = NULL;
static const char *g_nwpw_fractional_orbitals_default_path = NULL;
static const char *g_nwpw_smear_orbitals_default_path = NULL;
static const char *g_nwpw_virtual_orbitals_default_path = NULL;
static const char *g_nwpw_translate_vector_default_path = NULL;
static const char *g_brillouin_monkhorst_default_path = NULL;
static const char *g_brillouin_dos_grid_default_path = NULL;
static const char *g_compact_cells_path = NULL;
static const char *g_force_step_a_path = NULL;
static const char *g_force_step_b_path = NULL;
static const char *g_force_step_ev_path = NULL;
static const char *g_force_step_changed_species_path = NULL;
static const char *g_force_step_state_path = NULL;
static const char *g_tce_methods_path = NULL;

static char g_basis[64];
static char g_theory[64];
static char g_scf_type[64];
static char g_dft_xc[64];
static char g_input_blocks[8192];
static char g_psp_elements[8][17];
static char g_psp_names[8][257];
static char g_set_keys[64][129];
static char g_set_values[64][257];
static char g_typed_set_keys[512][129];
static char g_typed_set_values[512][64][257];
static char g_brillouin_zone_name[64];
static char g_brillouin_dos_zone_names[256][65];
static double g_brillouin_kvectors[8];
static int g_psp_types[8];
static int g_typed_set_types[512];
static int g_typed_set_value_counts[512];
static int g_brillouin_dos_zone_grids[256][3];
static int g_psp_count = 0;
static int g_set_string_count = 0;
static int g_typed_set_count = 0;
static int g_brillouin_dos_zone_count = 0;
static int g_set_config_calls = 0;
static int g_reset_rtdb_calls = 0;
static int g_set_dft_direct_calls = 0;
static int g_set_scf_direct_calls = 0;
static int g_set_driver_direct_calls = 0;
static int g_set_nwpw_direct_calls = 0;
static int g_set_brillouin_zone_calls = 0;
static int g_set_brillouin_dos_zones_calls = 0;
static int g_set_pseudopotential_calls = 0;
static int g_set_rtdb_strings_calls = 0;
static int g_set_rtdb_values_calls = 0;
static int g_dft_direct_enabled = 0;
static int g_dft_smearing_enabled = 0;
static double g_dft_smear_sigma_hartree = 0.0;
static int g_dft_smearing_spinset = 0;
static int g_scf_has_options = 0;
static int g_scf_maxiter = 0;
static double g_scf_thresh = 0.0;
static double g_scf_tol2e = 0.0;
static int g_driver_has_options = 0;
static int g_driver_maxiter = 0;
static int g_driver_tolerance_mode = 0;
static double g_driver_gmax_tol = 0.0;
static double g_driver_grms_tol = 0.0;
static double g_driver_xmax_tol = 0.0;
static double g_driver_xrms_tol = 0.0;
static int g_nwpw_has_options = 0;
static double g_nwpw_energy_cutoff = 0.0;
static double g_nwpw_wavefunction_cutoff = 0.0;
static double g_nwpw_ewald_rcut = 0.0;
static int g_nwpw_ewald_ncut = 0;
static int g_brillouin_has_options = 0;
static int g_brillouin_monkhorst_pack[3] = {0, 0, 0};
static int g_brillouin_max_kpoints_print = 0;
static int g_brillouin_kvector_count = 0;
static int g_energy_grad_calls = 0;
static int g_energy_only_calls = 0;
static int g_energy_only_cell_calls = 0;
static int g_hessian_calls = 0;
static int g_hessian_cell_calls = 0;
static int g_dipole_calls = 0;
static int g_dipole_cell_calls = 0;
static int g_polarizability_calls = 0;
static int g_polarizability_cell_calls = 0;
static int g_quadrupole_calls = 0;
static int g_quadrupole_cell_calls = 0;
static int g_optimize_calls = 0;
static int g_optimize_cell_calls = 0;
static int g_frequency_calls = 0;
static int g_frequency_cell_calls = 0;
static int g_frequency_modes_calls = 0;
static int g_frequency_modes_cell_calls = 0;
static int g_frequency_detail_calls = 0;
static int g_frequency_detail_cell_calls = 0;
static int g_stress_calls = 0;
static int g_stress_cell_calls = 0;
static double g_last_energy_h = 0.0;
static int g_call_n_atoms[8];
static int g_call_has_cell[8];
static int g_call_charge[8];
static int g_call_multiplicity[8];
static int g_call_atomic_numbers[8][8];
static double g_call_positions_ang[8][24];
static double g_call_cell_ang[8][9];
static int g_hessian_n_atoms[8];
static int g_hessian_has_cell[8];
static int g_hessian_charge[8];
static int g_hessian_multiplicity[8];
static int g_hessian_atomic_numbers[8][8];
static double g_hessian_positions_ang[8][24];
static double g_hessian_cell_ang[8][9];
static int g_dipole_n_atoms[8];
static int g_dipole_has_cell[8];
static int g_dipole_charge[8];
static int g_dipole_multiplicity[8];
static int g_dipole_atomic_numbers[8][8];
static double g_dipole_positions_ang[8][24];
static double g_dipole_cell_ang[8][9];
static int g_polarizability_n_atoms[8];
static int g_polarizability_has_cell[8];
static int g_polarizability_charge[8];
static int g_polarizability_multiplicity[8];
static int g_polarizability_atomic_numbers[8][8];
static double g_polarizability_positions_ang[8][24];
static double g_polarizability_cell_ang[8][9];
static int g_quadrupole_n_atoms[8];
static int g_quadrupole_has_cell[8];
static int g_quadrupole_charge[8];
static int g_quadrupole_multiplicity[8];
static int g_quadrupole_atomic_numbers[8][8];
static double g_quadrupole_positions_ang[8][24];
static double g_quadrupole_cell_ang[8][9];
static int g_optimize_n_atoms[8];
static int g_optimize_has_cell[8];
static int g_optimize_charge[8];
static int g_optimize_multiplicity[8];
static int g_optimize_atomic_numbers[8][8];
static double g_optimize_positions_ang[8][24];
static double g_optimize_cell_ang[8][9];
static int g_frequency_n_atoms[8];
static int g_frequency_has_cell[8];
static int g_frequency_charge[8];
static int g_frequency_multiplicity[8];
static int g_frequency_atomic_numbers[8][8];
static double g_frequency_positions_ang[8][24];
static double g_frequency_cell_ang[8][9];
static int g_stress_n_atoms[8];
static int g_stress_has_cell[8];
static int g_stress_charge[8];
static int g_stress_multiplicity[8];
static int g_stress_atomic_numbers[8][8];
static double g_stress_positions_ang[8][24];
static double g_stress_cell_ang[8][9];

#if defined(__GNUC__) || defined(__clang__)
#define NWCHEMC_TEST_WEAK __attribute__((weak))
#else
#define NWCHEMC_TEST_WEAK
#endif

extern NWChemCResult nwchemc_session_calculate_dipole(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *dipole_au,
    size_t dipole_len) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_session_calculate_quadrupole(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *quadrupole_au,
    size_t quadrupole_len) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_session_calculate_optimize(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *optimized_positions_ang,
    size_t optimized_positions_len) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_session_calculate_frequencies(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *frequencies_cm1,
    size_t frequencies_len, double *intensities_au, size_t intensities_len)
    NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_session_calculate_frequencies_detail(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *frequencies_cm1,
    size_t frequencies_len, double *intensities_au, size_t intensities_len,
    double *normal_modes, size_t normal_modes_len,
    double *projected_frequencies_cm1, size_t projected_frequencies_len,
    double *projected_intensities_au, size_t projected_intensities_len,
    double *thermochemistry, size_t thermochemistry_len) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_session_calculate_stress(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *stress_au,
    size_t stress_len) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_session_calculate_energy(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_forces(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *forces_h_bohr, size_t forces_len) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_forces_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *forces_h_bohr, size_t forces_len) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_energy(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp,
    size_t force_input_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_energy_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp,
    size_t force_input_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern size_t nwchemc_energy_result_size_for_force_input(
    const void *force_input_capnp,
    size_t force_input_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_session_calculate_energy_result(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_energy_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern size_t nwchemc_forces_result_size_for_force_input(
    const void *force_input_capnp,
    size_t force_input_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_session_calculate_forces_result(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_forces_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_hessian(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *hessian_h_bohr2, size_t hessian_len) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_hessian_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *hessian_h_bohr2, size_t hessian_len) NWCHEMC_TEST_WEAK;
extern size_t nwchemc_hessian_result_size_for_force_input(
    const void *force_input_capnp,
    size_t force_input_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_session_calculate_hessian_result(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_hessian_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_dipole(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *dipole_au, size_t dipole_len) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_dipole_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *dipole_au, size_t dipole_len) NWCHEMC_TEST_WEAK;
extern size_t nwchemc_dipole_result_size_for_force_input(
    const void *force_input_capnp,
    size_t force_input_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_session_calculate_dipole_result(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_dipole_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_quadrupole(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *quadrupole_au, size_t quadrupole_len) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_quadrupole_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *quadrupole_au, size_t quadrupole_len) NWCHEMC_TEST_WEAK;
extern size_t nwchemc_quadrupole_result_size_for_force_input(
    const void *force_input_capnp,
    size_t force_input_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_session_calculate_quadrupole_result(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_quadrupole_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_optimize(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *optimized_positions_ang, size_t optimized_positions_len)
    NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_optimize_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *optimized_positions_ang, size_t optimized_positions_len)
    NWCHEMC_TEST_WEAK;
extern size_t nwchemc_optimize_result_size_for_force_input(
    const void *force_input_capnp,
    size_t force_input_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_session_calculate_optimize_result(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_optimize_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_frequencies(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *frequencies_cm1, size_t frequencies_len, double *intensities_au,
    size_t intensities_len) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_frequencies_detail(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *frequencies_cm1, size_t frequencies_len, double *intensities_au,
    size_t intensities_len, double *normal_modes, size_t normal_modes_len,
    double *projected_frequencies_cm1, size_t projected_frequencies_len,
    double *projected_intensities_au, size_t projected_intensities_len,
    double *thermochemistry, size_t thermochemistry_len) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_frequencies_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *frequencies_cm1, size_t frequencies_len, double *intensities_au,
    size_t intensities_len) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_frequencies_detail_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *frequencies_cm1, size_t frequencies_len, double *intensities_au,
    size_t intensities_len, double *normal_modes, size_t normal_modes_len,
    double *projected_frequencies_cm1, size_t projected_frequencies_len,
    double *projected_intensities_au, size_t projected_intensities_len,
    double *thermochemistry, size_t thermochemistry_len) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_stress(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *stress_au, size_t stress_len) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_stress_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *stress_au, size_t stress_len) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_energy_gradient_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *grad_h_bohr) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_energy_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes)
    NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_energy_forces_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *forces_h_bohr) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_hessian_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *hessian_h_bohr2) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_dipole_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *dipole_au) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_quadrupole_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *quadrupole_au) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_stress_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *stress_au) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_optimize_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *optimized_positions_ang) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_frequencies_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *frequencies_cm1, double *intensities_au) NWCHEMC_TEST_WEAK;
extern size_t nwchemc_stress_result_size_for_force_input(
    const void *force_input_capnp,
    size_t force_input_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_session_calculate_stress_result(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_stress_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern size_t nwchemc_frequencies_result_size_for_force_input(
    const void *force_input_capnp,
    size_t force_input_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_session_calculate_frequencies_result(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_frequencies_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern int nwchemc_configure(const void *config_capnp,
                             size_t config_capnp_size_bytes)
    NWCHEMC_TEST_WEAK;
extern NWChemCSession *nwchemc_session_create_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes)
    NWCHEMC_TEST_WEAK;
extern int nwchemc_session_configure(NWChemCSession *session,
                                     const void *config_capnp,
                                     size_t config_capnp_size_bytes)
    NWCHEMC_TEST_WEAK;
extern int nwchemc_session_reset_topology(NWChemCSession *session)
    NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_energy_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_forces_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_hessian_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_dipole_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_quadrupole_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_stress_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_optimize_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;
extern NWChemCResult nwchemc_calculate_frequencies_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes) NWCHEMC_TEST_WEAK;

static void copy_span(char *dst, size_t dst_size, const char *src, int len) {
  size_t n = len > 0 ? (size_t)len : 0;
  if (n >= dst_size)
    n = dst_size - 1;
  if (n > 0)
    memcpy(dst, src, n);
  dst[n] = '\0';
}

static unsigned char *read_file(const char *path, size_t *size) {
  FILE *fp = fopen(path, "rb");
  if (!fp) {
    fprintf(stderr, "open failed for %s: %s\n", path, strerror(errno));
    return NULL;
  }
  if (fseek(fp, 0, SEEK_END) != 0) {
    fclose(fp);
    return NULL;
  }
  long n = ftell(fp);
  if (n <= 0) {
    fclose(fp);
    return NULL;
  }
  rewind(fp);
  unsigned char *buf = (unsigned char *)malloc((size_t)n);
  if (!buf) {
    fclose(fp);
    return NULL;
  }
  if (fread(buf, 1, (size_t)n, fp) != (size_t)n) {
    free(buf);
    fclose(fp);
    return NULL;
  }
  fclose(fp);
  *size = (size_t)n;
  return buf;
}

static unsigned char *wrap_params_in_config(const unsigned char *params_bytes,
                                            size_t params_size,
                                            size_t *config_size) {
  struct capn source_arena;
  NWChemParams_ptr params_root;
  assert_int_equal(nwchemc_params_root(params_bytes, params_size,
                                       &source_arena, &params_root),
                   0);

  struct capn config_arena;
  capn_init_malloc(&config_arena);
  capn_ptr root = capn_root(&config_arena);
  assert_int_not_equal(root.type, CAPN_NULL);
  PotentialConfig_ptr config = new_PotentialConfig(root.seg);
  assert_int_not_equal(config.p.type, CAPN_NULL);

  struct PotentialConfig config_view;
  memset(&config_view, 0, sizeof(config_view));
  config_view.which = PotentialConfig_nwchem;
  config_view.nwchem = params_root;
  write_PotentialConfig(&config_view, config);
  assert_int_equal(capn_setp(root, 0, config.p), 0);

  size_t capacity = params_size + 1024u;
  if (capacity < 4096u)
    capacity = 4096u;
  unsigned char *buffer = NULL;
  int written = -1;
  for (int attempt = 0; attempt < 6 && written < 0; ++attempt) {
    unsigned char *next = (unsigned char *)realloc(buffer, capacity);
    assert_non_null(next);
    buffer = next;
    written = capn_write_mem(&config_arena, buffer, capacity, 0);
    capacity *= 2u;
  }
  assert_true(written > 0);
  *config_size = (size_t)written;

  capn_free(&config_arena);
  nwchemc_params_release(&source_arena);
  return buffer;
}

static capn_text test_text_from_cstr(const char *s) {
  capn_text text;
  text.len = s ? (int)strlen(s) : 0;
  text.str = s ? s : "";
  text.seg = NULL;
  return text;
}

static unsigned char *make_params_with_engine_path(const char *engine_path,
                                                   size_t *params_size) {
  struct capn arena;
  capn_init_malloc(&arena);
  capn_ptr root = capn_root(&arena);
  assert_int_not_equal(root.type, CAPN_NULL);
  NWChemParams_ptr params = new_NWChemParams(root.seg);
  assert_int_not_equal(params.p.type, CAPN_NULL);

  struct NWChemParams view;
  memset(&view, 0, sizeof(view));
  view.basis = test_text_from_cstr("sto-3g");
  view.theory = test_text_from_cstr("scf");
  view.scfType = test_text_from_cstr("rhf");
  view.multiplicity = 1;
  view.enginePath = test_text_from_cstr(engine_path);
  view.task = test_text_from_cstr("gradient");
  write_NWChemParams(&view, params);
  assert_int_equal(capn_setp(root, 0, params.p), 0);

  size_t capacity = 4096u;
  unsigned char *buffer = NULL;
  int written = -1;
  for (int attempt = 0; attempt < 6 && written < 0; ++attempt) {
    unsigned char *next = (unsigned char *)realloc(buffer, capacity);
    assert_non_null(next);
    buffer = next;
    written = capn_write_mem(&arena, buffer, capacity, 0);
    capacity *= 2u;
  }
  assert_true(written > 0);
  *params_size = (size_t)written;

  capn_free(&arena);
  return buffer;
}

void nwchemc_embed_init(void) {}

int nwchemc_embed_available(void) { return 1; }

int nwchemc_embed_reset_rtdb(void) {
  ++g_reset_rtdb_calls;
  return 0;
}

int nwchemc_embed_set_config(const char *basis, int basis_len,
                             const char *theory, int theory_len,
                             const char *scf_type, int scf_len,
                             const int *charge, const int *mult,
                             const char *input_blocks,
                             int input_blocks_len) {
  (void)charge;
  (void)mult;
  ++g_set_config_calls;
  copy_span(g_basis, sizeof(g_basis), basis, basis_len);
  copy_span(g_theory, sizeof(g_theory), theory, theory_len);
  copy_span(g_scf_type, sizeof(g_scf_type), scf_type, scf_len);
  copy_span(g_input_blocks, sizeof(g_input_blocks), input_blocks,
            input_blocks_len);
  return 0;
}

int nwchemc_embed_set_dft_direct(const char *xc, int xc_len,
                                 int direct_enabled, int smearing_enabled,
                                 double smear_sigma_hartree,
                                 int smearing_spinset) {
  ++g_set_dft_direct_calls;
  copy_span(g_dft_xc, sizeof(g_dft_xc), xc, xc_len);
  g_dft_direct_enabled = direct_enabled;
  g_dft_smearing_enabled = smearing_enabled;
  g_dft_smear_sigma_hartree = smear_sigma_hartree;
  g_dft_smearing_spinset = smearing_spinset;
  return 0;
}

int nwchemc_embed_set_scf_direct(int has_options, int maxiter, double thresh,
                                 double tol2e) {
  ++g_set_scf_direct_calls;
  g_scf_has_options = has_options;
  g_scf_maxiter = maxiter;
  g_scf_thresh = thresh;
  g_scf_tol2e = tol2e;
  return 0;
}

int nwchemc_embed_set_driver_direct(int has_options, int maxiter,
                                    int tolerance_mode, double gmax_tol,
                                    double grms_tol, double xmax_tol,
                                    double xrms_tol) {
  ++g_set_driver_direct_calls;
  g_driver_has_options = has_options;
  g_driver_maxiter = maxiter;
  g_driver_tolerance_mode = tolerance_mode;
  g_driver_gmax_tol = gmax_tol;
  g_driver_grms_tol = grms_tol;
  g_driver_xmax_tol = xmax_tol;
  g_driver_xrms_tol = xrms_tol;
  return 0;
}

int nwchemc_embed_set_nwpw_direct(int has_options, double energy_cutoff,
                                  double wavefunction_cutoff,
                                  double ewald_rcut, int ewald_ncut) {
  ++g_set_nwpw_direct_calls;
  g_nwpw_has_options = has_options;
  g_nwpw_energy_cutoff = energy_cutoff;
  g_nwpw_wavefunction_cutoff = wavefunction_cutoff;
  g_nwpw_ewald_rcut = ewald_rcut;
  g_nwpw_ewald_ncut = ewald_ncut;
  return 0;
}

int nwchemc_embed_set_brillouin_zone(int has_options, const char *zone_name,
                                     int zone_name_len, int monkhorst_pack_x,
                                     int monkhorst_pack_y,
                                     int monkhorst_pack_z,
                                     int max_kpoints_print,
                                     const double *kvectors,
                                     int kvector_count) {
  ++g_set_brillouin_zone_calls;
  g_brillouin_has_options = has_options;
  copy_span(g_brillouin_zone_name, sizeof(g_brillouin_zone_name), zone_name,
            zone_name_len);
  g_brillouin_monkhorst_pack[0] = monkhorst_pack_x;
  g_brillouin_monkhorst_pack[1] = monkhorst_pack_y;
  g_brillouin_monkhorst_pack[2] = monkhorst_pack_z;
  g_brillouin_max_kpoints_print = max_kpoints_print;
  g_brillouin_kvector_count = kvector_count;
  for (int i = 0; i < kvector_count * 4 && i < 8; ++i)
    g_brillouin_kvectors[i] = kvectors[i];
  return 0;
}

int nwchemc_embed_set_brillouin_dos_zones(const char *zone_names,
                                          const int *zone_grids, int count) {
  ++g_set_brillouin_dos_zones_calls;
  g_brillouin_dos_zone_count = count;
  if (count > 256)
    count = 256;
  for (int i = 0; i < count; ++i) {
    copy_span(g_brillouin_dos_zone_names[i],
              sizeof(g_brillouin_dos_zone_names[i]),
              zone_names + i * 64, 64);
    g_brillouin_dos_zone_grids[i][0] = zone_grids[3 * i];
    g_brillouin_dos_zone_grids[i][1] = zone_grids[3 * i + 1];
    g_brillouin_dos_zone_grids[i][2] = zone_grids[3 * i + 2];
  }
  return 0;
}

int nwchemc_embed_set_pseudopotentials(const char *elements,
                                       const int *library_types,
                                       const char *library_names, int count) {
  ++g_set_pseudopotential_calls;
  g_psp_count = count;
  if (count > 8)
    count = 8;
  for (int i = 0; i < count; ++i) {
    copy_span(g_psp_elements[i], sizeof(g_psp_elements[i]), elements + i * 16,
              16);
    copy_span(g_psp_names[i], sizeof(g_psp_names[i]),
              library_names + i * 256, 256);
    g_psp_types[i] = library_types[i];
  }
  return 0;
}

int nwchemc_embed_set_rtdb_strings(const char *keys, const char *values,
                                   int count) {
  ++g_set_rtdb_strings_calls;
  /* Accumulate across promo + explicit set stanza calls. */
  for (int i = 0; i < count; ++i) {
    if (g_set_string_count >= 64)
      break;
    int dst = g_set_string_count++;
    copy_span(g_set_keys[dst], sizeof(g_set_keys[dst]), keys + i * 128, 128);
    copy_span(g_set_values[dst], sizeof(g_set_values[dst]), values + i * 256,
              256);
  }
  return 0;
}

int nwchemc_embed_set_rtdb_values(const char *keys, const int *value_types,
                                  const int *value_counts,
                                  const char *values, int count) {
  ++g_set_rtdb_values_calls;
  /* Accumulate across promo + explicit set stanza calls. */
  for (int i = 0; i < count; ++i) {
    if (g_typed_set_count >= 512)
      break;
    int dst = g_typed_set_count++;
    copy_span(g_typed_set_keys[dst], sizeof(g_typed_set_keys[dst]),
              keys + i * 128, 128);
    g_typed_set_types[dst] = value_types[i];
    g_typed_set_value_counts[dst] = value_counts[i];
    int nvalues = value_counts[i] < 64 ? value_counts[i] : 64;
    for (int j = 0; j < nvalues; ++j) {
      copy_span(g_typed_set_values[dst][j],
                sizeof(g_typed_set_values[dst][j]),
                values + (i * 64 + j) * 256, 256);
    }
  }
  return 0;
}

static int capture_energy_only_call(const int *n_atoms,
                                    const double *positions_ang,
                                    const int *atomic_numbers,
                                    const double *cell_ang,
                                    const int *has_cell, const int *charge,
                                    const int *multiplicity, double *energy_h,
                                    char *errmsg, int errmsg_len) {
  int call = g_energy_only_calls;
  if (call < 8) {
    int ncopy = *n_atoms < 8 ? *n_atoms : 8;
    int ncoord = (*n_atoms) * 3 < 24 ? (*n_atoms) * 3 : 24;
    g_call_n_atoms[call] = *n_atoms;
    g_call_has_cell[call] = has_cell ? *has_cell : 0;
    g_call_charge[call] = charge ? *charge : 0;
    g_call_multiplicity[call] = multiplicity ? *multiplicity : 0;
    for (int i = 0; i < ncopy; ++i)
      g_call_atomic_numbers[call][i] = atomic_numbers[i];
    for (int i = 0; i < ncoord; ++i)
      g_call_positions_ang[call][i] = positions_ang[i];
    for (int i = 0; i < 9; ++i)
      g_call_cell_ang[call][i] = cell_ang && g_call_has_cell[call]
                                     ? cell_ang[i]
                                     : 0.0;
  }
  ++g_energy_only_calls;
  *energy_h = -1.0;
  snprintf(errmsg, (size_t)errmsg_len, "ok");
  return 0;
}

int nwchemc_embed_energy_only(const int *n_atoms, const double *positions_ang,
                              const int *atomic_numbers, const int *charge,
                              const int *multiplicity, double *energy_h,
                              char *errmsg, int errmsg_len) {
  return capture_energy_only_call(n_atoms, positions_ang, atomic_numbers, NULL,
                                  NULL, charge, multiplicity, energy_h, errmsg,
                                  errmsg_len);
}

int nwchemc_embed_energy_only_cell(
    const int *n_atoms, const double *positions_ang, const int *atomic_numbers,
    const double *cell_ang, const int *has_cell, const int *charge,
    const int *multiplicity, double *energy_h, char *errmsg, int errmsg_len) {
  ++g_energy_only_cell_calls;
  return capture_energy_only_call(n_atoms, positions_ang, atomic_numbers,
                                  cell_ang, has_cell, charge, multiplicity,
                                  energy_h, errmsg, errmsg_len);
}

static int capture_energy_grad_call(const int *n_atoms,
                                    const double *positions_ang,
                                    const int *atomic_numbers,
                                    const double *cell_ang,
                                    const int *has_cell, const int *charge,
                                    const int *multiplicity,
                                    double *energy_h, double *grad_h_bohr,
                                    char *errmsg, int errmsg_len) {
  int call = g_energy_grad_calls;
  if (call < 8) {
    int ncopy = *n_atoms < 8 ? *n_atoms : 8;
    int ncoord = (*n_atoms) * 3 < 24 ? (*n_atoms) * 3 : 24;
    g_call_n_atoms[call] = *n_atoms;
    g_call_has_cell[call] = has_cell ? *has_cell : 0;
    g_call_charge[call] = charge ? *charge : 0;
    g_call_multiplicity[call] = multiplicity ? *multiplicity : 0;
    for (int i = 0; i < ncopy; ++i)
      g_call_atomic_numbers[call][i] = atomic_numbers[i];
    for (int i = 0; i < ncoord; ++i)
      g_call_positions_ang[call][i] = positions_ang[i];
    for (int i = 0; i < 9; ++i)
      g_call_cell_ang[call][i] = cell_ang && g_call_has_cell[call]
                                     ? cell_ang[i]
                                     : 0.0;
  }
  ++g_energy_grad_calls;
  *energy_h = -1.0;
  for (int i = 0; i < (*n_atoms) * 3; ++i)
    grad_h_bohr[i] = (double)(i + 1);
  snprintf(errmsg, (size_t)errmsg_len, "ok");
  return 0;
}

int nwchemc_embed_energy_grad(const int *n_atoms, const double *positions_ang,
                              const int *atomic_numbers, const int *charge,
                              const int *multiplicity, double *energy_h,
                              double *grad_h_bohr, char *errmsg,
                              int errmsg_len) {
  return capture_energy_grad_call(n_atoms, positions_ang, atomic_numbers, NULL,
                                  NULL, charge, multiplicity, energy_h,
                                  grad_h_bohr, errmsg, errmsg_len);
}

int nwchemc_embed_energy_grad_cell(
    const int *n_atoms, const double *positions_ang, const int *atomic_numbers,
    const double *cell_ang, const int *has_cell, const int *charge,
    const int *multiplicity, double *energy_h, double *grad_h_bohr,
    char *errmsg, int errmsg_len) {
  return capture_energy_grad_call(n_atoms, positions_ang, atomic_numbers,
                                  cell_ang, has_cell, charge, multiplicity,
                                  energy_h, grad_h_bohr, errmsg, errmsg_len);
}

static int capture_hessian_call(const int *n_atoms, const double *positions_ang,
                                const int *atomic_numbers,
                                const double *cell_ang, const int *has_cell,
                                const int *charge, const int *multiplicity,
                                double *hessian_h_bohr2, char *errmsg,
                                int errmsg_len) {
  int call = g_hessian_calls;
  if (call < 8) {
    int ncopy = *n_atoms < 8 ? *n_atoms : 8;
    int ncoord = (*n_atoms) * 3 < 24 ? (*n_atoms) * 3 : 24;
    g_hessian_n_atoms[call] = *n_atoms;
    g_hessian_has_cell[call] = has_cell ? *has_cell : 0;
    g_hessian_charge[call] = charge ? *charge : 0;
    g_hessian_multiplicity[call] = multiplicity ? *multiplicity : 0;
    for (int i = 0; i < ncopy; ++i)
      g_hessian_atomic_numbers[call][i] = atomic_numbers[i];
    for (int i = 0; i < ncoord; ++i)
      g_hessian_positions_ang[call][i] = positions_ang[i];
    for (int i = 0; i < 9; ++i)
      g_hessian_cell_ang[call][i] = cell_ang && g_hessian_has_cell[call]
                                        ? cell_ang[i]
                                        : 0.0;
  }
  ++g_hessian_calls;
  g_last_energy_h = -1.125;
  int ndof = (*n_atoms) * 3;
  for (int i = 0; i < ndof * ndof; ++i)
    hessian_h_bohr2[i] = (double)(i + 10);
  snprintf(errmsg, (size_t)errmsg_len, "ok");
  return 0;
}

int nwchemc_embed_hessian(const int *n_atoms, const double *positions_ang,
                          const int *atomic_numbers, const int *charge,
                          const int *multiplicity, double *hessian_h_bohr2,
                          char *errmsg, int errmsg_len) {
  return capture_hessian_call(n_atoms, positions_ang, atomic_numbers, NULL,
                              NULL, charge, multiplicity, hessian_h_bohr2,
                              errmsg, errmsg_len);
}

int nwchemc_embed_hessian_cell(const int *n_atoms, const double *positions_ang,
                               const int *atomic_numbers,
                               const double *cell_ang, const int *has_cell,
                               const int *charge, const int *multiplicity,
                               double *hessian_h_bohr2, char *errmsg,
                               int errmsg_len) {
  ++g_hessian_cell_calls;
  return capture_hessian_call(n_atoms, positions_ang, atomic_numbers, cell_ang,
                              has_cell, charge, multiplicity, hessian_h_bohr2,
                              errmsg, errmsg_len);
}

int nwchemc_embed_last_energy(double *energy_h) {
  if (!energy_h)
    return -1;
  *energy_h = g_last_energy_h;
  return 0;
}

static int capture_dipole_call(const int *n_atoms, const double *positions_ang,
                               const int *atomic_numbers,
                               const double *cell_ang, const int *has_cell,
                               const int *charge, const int *multiplicity,
                               double *energy_h, double *dipole_au,
                               char *errmsg, int errmsg_len) {
  int call = g_dipole_calls;
  if (call < 8) {
    int ncopy = *n_atoms < 8 ? *n_atoms : 8;
    int ncoord = (*n_atoms) * 3 < 24 ? (*n_atoms) * 3 : 24;
    g_dipole_n_atoms[call] = *n_atoms;
    g_dipole_has_cell[call] = has_cell ? *has_cell : 0;
    g_dipole_charge[call] = charge ? *charge : 0;
    g_dipole_multiplicity[call] = multiplicity ? *multiplicity : 0;
    for (int i = 0; i < ncopy; ++i)
      g_dipole_atomic_numbers[call][i] = atomic_numbers[i];
    for (int i = 0; i < ncoord; ++i)
      g_dipole_positions_ang[call][i] = positions_ang[i];
    for (int i = 0; i < 9; ++i)
      g_dipole_cell_ang[call][i] = cell_ang && g_dipole_has_cell[call]
                                       ? cell_ang[i]
                                       : 0.0;
  }
  ++g_dipole_calls;
  *energy_h = -1.25;
  dipole_au[0] = 0.25;
  dipole_au[1] = 0.5;
  dipole_au[2] = 0.75;
  snprintf(errmsg, (size_t)errmsg_len, "ok");
  return 0;
}

int nwchemc_embed_dipole(const int *n_atoms, const double *positions_ang,
                         const int *atomic_numbers, const int *charge,
                         const int *multiplicity, double *energy_h,
                         double *dipole_au, char *errmsg, int errmsg_len) {
  return capture_dipole_call(n_atoms, positions_ang, atomic_numbers, NULL,
                             NULL, charge, multiplicity, energy_h, dipole_au,
                             errmsg, errmsg_len);
}

int nwchemc_embed_dipole_cell(
    const int *n_atoms, const double *positions_ang, const int *atomic_numbers,
    const double *cell_ang, const int *has_cell, const int *charge,
    const int *multiplicity, double *energy_h, double *dipole_au, char *errmsg,
    int errmsg_len) {
  ++g_dipole_cell_calls;
  return capture_dipole_call(n_atoms, positions_ang, atomic_numbers, cell_ang,
                             has_cell, charge, multiplicity, energy_h,
                             dipole_au, errmsg, errmsg_len);
}

static int capture_polarizability_call(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const double *cell_ang, const int *has_cell,
    const int *charge, const int *multiplicity, double *energy_h,
    double *polarizability_au, char *errmsg, int errmsg_len) {
  int call = g_polarizability_calls;
  if (call < 8) {
    int ncopy = *n_atoms < 8 ? *n_atoms : 8;
    int ncoord = (*n_atoms) * 3 < 24 ? (*n_atoms) * 3 : 24;
    g_polarizability_n_atoms[call] = *n_atoms;
    g_polarizability_has_cell[call] = has_cell ? *has_cell : 0;
    g_polarizability_charge[call] = charge ? *charge : 0;
    g_polarizability_multiplicity[call] =
        multiplicity ? *multiplicity : 0;
    for (int i = 0; i < ncopy; ++i)
      g_polarizability_atomic_numbers[call][i] = atomic_numbers[i];
    for (int i = 0; i < ncoord; ++i)
      g_polarizability_positions_ang[call][i] = positions_ang[i];
    for (int i = 0; i < 9; ++i)
      g_polarizability_cell_ang[call][i] =
          cell_ang && g_polarizability_has_cell[call] ? cell_ang[i] : 0.0;
  }
  ++g_polarizability_calls;
  *energy_h = -1.375;
  for (int i = 0; i < 12; ++i)
    polarizability_au[i] = 0.0625 * (double)(i + 1);
  snprintf(errmsg, (size_t)errmsg_len, "ok");
  return 0;
}

int nwchemc_embed_polarizability(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const int *charge, const int *multiplicity,
    double *energy_h, double *polarizability_au, char *errmsg,
    int errmsg_len) {
  return capture_polarizability_call(
      n_atoms, positions_ang, atomic_numbers, NULL, NULL, charge,
      multiplicity, energy_h, polarizability_au, errmsg, errmsg_len);
}

int nwchemc_embed_polarizability_cell(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const double *cell_ang, const int *has_cell,
    const int *charge, const int *multiplicity, double *energy_h,
    double *polarizability_au, char *errmsg, int errmsg_len) {
  ++g_polarizability_cell_calls;
  return capture_polarizability_call(
      n_atoms, positions_ang, atomic_numbers, cell_ang, has_cell, charge,
      multiplicity, energy_h, polarizability_au, errmsg, errmsg_len);
}

static int capture_quadrupole_call(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const double *cell_ang, const int *has_cell,
    const int *charge, const int *multiplicity, double *energy_h,
    double *quadrupole_au, char *errmsg, int errmsg_len) {
  int call = g_quadrupole_calls;
  if (call < 8) {
    int ncopy = *n_atoms < 8 ? *n_atoms : 8;
    int ncoord = (*n_atoms) * 3 < 24 ? (*n_atoms) * 3 : 24;
    g_quadrupole_n_atoms[call] = *n_atoms;
    g_quadrupole_has_cell[call] = has_cell ? *has_cell : 0;
    g_quadrupole_charge[call] = charge ? *charge : 0;
    g_quadrupole_multiplicity[call] = multiplicity ? *multiplicity : 0;
    for (int i = 0; i < ncopy; ++i)
      g_quadrupole_atomic_numbers[call][i] = atomic_numbers[i];
    for (int i = 0; i < ncoord; ++i)
      g_quadrupole_positions_ang[call][i] = positions_ang[i];
    for (int i = 0; i < 9; ++i)
      g_quadrupole_cell_ang[call][i] =
          cell_ang && g_quadrupole_has_cell[call] ? cell_ang[i] : 0.0;
  }
  ++g_quadrupole_calls;
  *energy_h = -1.5;
  for (int i = 0; i < 6; ++i)
    quadrupole_au[i] = 0.125 * (double)(i + 1);
  snprintf(errmsg, (size_t)errmsg_len, "ok");
  return 0;
}

int nwchemc_embed_quadrupole(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const int *charge, const int *multiplicity,
    double *energy_h, double *quadrupole_au, char *errmsg, int errmsg_len) {
  return capture_quadrupole_call(n_atoms, positions_ang, atomic_numbers, NULL,
                                 NULL, charge, multiplicity, energy_h,
                                 quadrupole_au, errmsg, errmsg_len);
}

int nwchemc_embed_quadrupole_cell(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const double *cell_ang, const int *has_cell,
    const int *charge, const int *multiplicity, double *energy_h,
    double *quadrupole_au, char *errmsg, int errmsg_len) {
  ++g_quadrupole_cell_calls;
  return capture_quadrupole_call(n_atoms, positions_ang, atomic_numbers,
                                 cell_ang, has_cell, charge, multiplicity,
                                 energy_h, quadrupole_au, errmsg, errmsg_len);
}

static int capture_optimize_call(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const double *cell_ang, const int *has_cell,
    const int *charge, const int *multiplicity, double *energy_h,
    double *optimized_positions_ang, char *errmsg, int errmsg_len) {
  int call = g_optimize_calls;
  if (call < 8) {
    int ncopy = *n_atoms < 8 ? *n_atoms : 8;
    int ncoord = (*n_atoms) * 3 < 24 ? (*n_atoms) * 3 : 24;
    g_optimize_n_atoms[call] = *n_atoms;
    g_optimize_has_cell[call] = has_cell ? *has_cell : 0;
    g_optimize_charge[call] = charge ? *charge : 0;
    g_optimize_multiplicity[call] = multiplicity ? *multiplicity : 0;
    for (int i = 0; i < ncopy; ++i)
      g_optimize_atomic_numbers[call][i] = atomic_numbers[i];
    for (int i = 0; i < ncoord; ++i)
      g_optimize_positions_ang[call][i] = positions_ang[i];
    for (int i = 0; i < 9; ++i)
      g_optimize_cell_ang[call][i] =
          cell_ang && g_optimize_has_cell[call] ? cell_ang[i] : 0.0;
  }
  ++g_optimize_calls;
  *energy_h = -1.75;
  for (int i = 0; i < (*n_atoms) * 3; ++i)
    optimized_positions_ang[i] = positions_ang[i] + 0.01 * (double)(i + 1);
  snprintf(errmsg, (size_t)errmsg_len, "ok");
  return 0;
}

int nwchemc_embed_optimize(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const int *charge, const int *multiplicity,
    double *energy_h, double *optimized_positions_ang, char *errmsg,
    int errmsg_len) {
  return capture_optimize_call(n_atoms, positions_ang, atomic_numbers, NULL,
                               NULL, charge, multiplicity, energy_h,
                               optimized_positions_ang, errmsg, errmsg_len);
}

int nwchemc_embed_optimize_cell(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const double *cell_ang, const int *has_cell,
    const int *charge, const int *multiplicity, double *energy_h,
    double *optimized_positions_ang, char *errmsg, int errmsg_len) {
  ++g_optimize_cell_calls;
  return capture_optimize_call(n_atoms, positions_ang, atomic_numbers,
                               cell_ang, has_cell, charge, multiplicity,
                               energy_h, optimized_positions_ang, errmsg,
                               errmsg_len);
}

static int capture_frequency_call(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const double *cell_ang, const int *has_cell,
    const int *charge, const int *multiplicity, double *frequencies_cm1,
    double *intensities_au, char *errmsg, int errmsg_len) {
  int call = g_frequency_calls;
  if (call < 8) {
    int ncopy = *n_atoms < 8 ? *n_atoms : 8;
    int ncoord = (*n_atoms) * 3 < 24 ? (*n_atoms) * 3 : 24;
    g_frequency_n_atoms[call] = *n_atoms;
    g_frequency_has_cell[call] = has_cell ? *has_cell : 0;
    g_frequency_charge[call] = charge ? *charge : 0;
    g_frequency_multiplicity[call] = multiplicity ? *multiplicity : 0;
    for (int i = 0; i < ncopy; ++i)
      g_frequency_atomic_numbers[call][i] = atomic_numbers[i];
    for (int i = 0; i < ncoord; ++i)
      g_frequency_positions_ang[call][i] = positions_ang[i];
    for (int i = 0; i < 9; ++i)
      g_frequency_cell_ang[call][i] =
          cell_ang && g_frequency_has_cell[call] ? cell_ang[i] : 0.0;
  }
  ++g_frequency_calls;
  g_last_energy_h = -1.625;
  int ndof = (*n_atoms) * 3;
  for (int i = 0; i < ndof; ++i) {
    frequencies_cm1[i] = 100.0 + (double)i;
    if (intensities_au)
      intensities_au[i] = 0.01 * (double)(i + 1);
  }
  snprintf(errmsg, (size_t)errmsg_len, "ok");
  return 0;
}

int nwchemc_embed_frequencies(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const int *charge, const int *multiplicity,
    double *frequencies_cm1, double *intensities_au, char *errmsg,
    int errmsg_len) {
  return capture_frequency_call(n_atoms, positions_ang, atomic_numbers, NULL,
                                NULL, charge, multiplicity, frequencies_cm1,
                                intensities_au, errmsg, errmsg_len);
}

int nwchemc_embed_frequencies_cell(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const double *cell_ang, const int *has_cell,
    const int *charge, const int *multiplicity, double *frequencies_cm1,
    double *intensities_au, char *errmsg, int errmsg_len) {
  ++g_frequency_cell_calls;
  return capture_frequency_call(n_atoms, positions_ang, atomic_numbers,
                                cell_ang, has_cell, charge, multiplicity,
                                frequencies_cm1, intensities_au, errmsg,
                                errmsg_len);
}

static int capture_frequency_modes_call(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const double *cell_ang, const int *has_cell,
    const int *charge, const int *multiplicity, double *frequencies_cm1,
    double *intensities_au, double *normal_modes, char *errmsg,
    int errmsg_len) {
  ++g_frequency_modes_calls;
  int rc = capture_frequency_call(n_atoms, positions_ang, atomic_numbers,
                                  cell_ang, has_cell, charge, multiplicity,
                                  frequencies_cm1, intensities_au, errmsg,
                                  errmsg_len);
  int ndof = (*n_atoms) * 3;
  for (int i = 0; normal_modes && i < ndof * ndof; ++i)
    normal_modes[i] = 0.001 * (double)(i + 1);
  return rc;
}

int nwchemc_embed_frequencies_modes(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const int *charge, const int *multiplicity,
    double *frequencies_cm1, double *intensities_au, double *normal_modes,
    char *errmsg, int errmsg_len) {
  return capture_frequency_modes_call(
      n_atoms, positions_ang, atomic_numbers, NULL, NULL, charge,
      multiplicity, frequencies_cm1, intensities_au, normal_modes, errmsg,
      errmsg_len);
}

int nwchemc_embed_frequencies_modes_cell(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const double *cell_ang, const int *has_cell,
    const int *charge, const int *multiplicity, double *frequencies_cm1,
    double *intensities_au, double *normal_modes, char *errmsg,
    int errmsg_len) {
  ++g_frequency_modes_cell_calls;
  return capture_frequency_modes_call(
      n_atoms, positions_ang, atomic_numbers, cell_ang, has_cell, charge,
      multiplicity, frequencies_cm1, intensities_au, normal_modes, errmsg,
      errmsg_len);
}

int nwchemc_embed_frequencies_detail_cell(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const double *cell_ang, const int *has_cell,
    const int *charge, const int *multiplicity, double *frequencies_cm1,
    double *intensities_au, double *normal_modes,
    double *projected_frequencies_cm1, double *projected_intensities_au,
    double *thermochemistry,
    char *errmsg, int errmsg_len) {
  ++g_frequency_detail_calls;
  ++g_frequency_detail_cell_calls;
  int rc = capture_frequency_modes_call(
      n_atoms, positions_ang, atomic_numbers, cell_ang, has_cell, charge,
      multiplicity, frequencies_cm1, intensities_au, normal_modes, errmsg,
      errmsg_len);
  if (thermochemistry) {
    thermochemistry[0] = 0.001;
    thermochemistry[1] = 0.002;
    thermochemistry[2] = 0.003;
    thermochemistry[3] = 4.0;
    thermochemistry[4] = 5.0;
  }
  int ndof = (*n_atoms) * 3;
  for (int i = 0; projected_frequencies_cm1 && i < ndof; ++i)
    projected_frequencies_cm1[i] = 90.0 + (double)i;
  for (int i = 0; projected_intensities_au && i < ndof; ++i)
    projected_intensities_au[i] = 0.02 * (double)(i + 1);
  return rc;
}

static int capture_stress_call(const int *n_atoms,
                               const double *positions_ang,
                               const int *atomic_numbers,
                               const double *cell_ang, const int *has_cell,
                               const int *charge, const int *multiplicity,
                               double *energy_h, double *stress_au,
                               char *errmsg, int errmsg_len) {
  int call = g_stress_calls;
  if (call < 8) {
    int ncopy = *n_atoms < 8 ? *n_atoms : 8;
    int ncoord = (*n_atoms) * 3 < 24 ? (*n_atoms) * 3 : 24;
    g_stress_n_atoms[call] = *n_atoms;
    g_stress_has_cell[call] = has_cell ? *has_cell : 0;
    g_stress_charge[call] = charge ? *charge : 0;
    g_stress_multiplicity[call] = multiplicity ? *multiplicity : 0;
    for (int i = 0; i < ncopy; ++i)
      g_stress_atomic_numbers[call][i] = atomic_numbers[i];
    for (int i = 0; i < ncoord; ++i)
      g_stress_positions_ang[call][i] = positions_ang[i];
    for (int i = 0; i < 9; ++i)
      g_stress_cell_ang[call][i] =
          cell_ang && g_stress_has_cell[call] ? cell_ang[i] : 0.0;
  }
  ++g_stress_calls;
  *energy_h = -2.0;
  for (int i = 0; i < 9; ++i)
    stress_au[i] = 0.03125 * (double)(i + 1);
  snprintf(errmsg, (size_t)errmsg_len, "ok");
  return 0;
}

int nwchemc_embed_stress(const int *n_atoms, const double *positions_ang,
                         const int *atomic_numbers, const int *charge,
                         const int *multiplicity, double *energy_h,
                         double *stress_au, char *errmsg, int errmsg_len) {
  return capture_stress_call(n_atoms, positions_ang, atomic_numbers, NULL,
                             NULL, charge, multiplicity, energy_h, stress_au,
                             errmsg, errmsg_len);
}

int nwchemc_embed_stress_cell(
    const int *n_atoms, const double *positions_ang, const int *atomic_numbers,
    const double *cell_ang, const int *has_cell, const int *charge,
    const int *multiplicity, double *energy_h, double *stress_au, char *errmsg,
    int errmsg_len) {
  ++g_stress_cell_calls;
  return capture_stress_call(n_atoms, positions_ang, atomic_numbers, cell_ang,
                             has_cell, charge, multiplicity, energy_h,
                             stress_au, errmsg, errmsg_len);
}

void nwchemc_embed_finalize(void) {}

static void reset_embed_captures(void) {
  g_basis[0] = '\0';
  g_theory[0] = '\0';
  g_scf_type[0] = '\0';
  g_dft_xc[0] = '\0';
  g_input_blocks[0] = '\0';
  for (int i = 0; i < 8; ++i) {
    g_psp_elements[i][0] = '\0';
    g_psp_names[i][0] = '\0';
    g_psp_types[i] = -1;
  }
  for (int i = 0; i < 64; ++i) {
    g_set_keys[i][0] = '\0';
    g_set_values[i][0] = '\0';
  }
  for (int i = 0; i < 512; ++i) {
    g_typed_set_keys[i][0] = '\0';
    for (int j = 0; j < 64; ++j)
      g_typed_set_values[i][j][0] = '\0';
    g_typed_set_types[i] = -1;
    g_typed_set_value_counts[i] = 0;
  }
  g_psp_count = 0;
  g_set_string_count = 0;
  g_typed_set_count = 0;
  g_set_config_calls = 0;
  g_reset_rtdb_calls = 0;
  g_set_dft_direct_calls = 0;
  g_set_scf_direct_calls = 0;
  g_set_driver_direct_calls = 0;
  g_set_nwpw_direct_calls = 0;
  g_set_brillouin_zone_calls = 0;
  g_set_brillouin_dos_zones_calls = 0;
  g_set_pseudopotential_calls = 0;
  g_set_rtdb_strings_calls = 0;
  g_set_rtdb_values_calls = 0;
  g_dft_direct_enabled = 0;
  g_dft_smearing_enabled = 0;
  g_dft_smear_sigma_hartree = 0.0;
  g_dft_smearing_spinset = 0;
  g_scf_has_options = 0;
  g_scf_maxiter = 0;
  g_scf_thresh = 0.0;
  g_scf_tol2e = 0.0;
  g_driver_has_options = 0;
  g_driver_maxiter = 0;
  g_driver_tolerance_mode = 0;
  g_driver_gmax_tol = 0.0;
  g_driver_grms_tol = 0.0;
  g_driver_xmax_tol = 0.0;
  g_driver_xrms_tol = 0.0;
  g_nwpw_has_options = 0;
  g_nwpw_energy_cutoff = 0.0;
  g_nwpw_wavefunction_cutoff = 0.0;
  g_nwpw_ewald_rcut = 0.0;
  g_nwpw_ewald_ncut = 0;
  g_brillouin_has_options = 0;
  g_brillouin_zone_name[0] = '\0';
  g_brillouin_dos_zone_count = 0;
  g_brillouin_monkhorst_pack[0] = 0;
  g_brillouin_monkhorst_pack[1] = 0;
  g_brillouin_monkhorst_pack[2] = 0;
  g_brillouin_max_kpoints_print = 0;
  g_brillouin_kvector_count = 0;
  for (int i = 0; i < 8; ++i)
    g_brillouin_kvectors[i] = 0.0;
  g_energy_grad_calls = 0;
  g_energy_only_calls = 0;
  g_energy_only_cell_calls = 0;
  g_hessian_calls = 0;
  g_hessian_cell_calls = 0;
  g_dipole_calls = 0;
  g_dipole_cell_calls = 0;
  g_polarizability_calls = 0;
  g_polarizability_cell_calls = 0;
  g_quadrupole_calls = 0;
  g_quadrupole_cell_calls = 0;
  g_optimize_calls = 0;
  g_optimize_cell_calls = 0;
  g_frequency_calls = 0;
  g_frequency_cell_calls = 0;
  g_frequency_modes_calls = 0;
  g_frequency_modes_cell_calls = 0;
  g_frequency_detail_calls = 0;
  g_frequency_detail_cell_calls = 0;
  g_stress_calls = 0;
  g_stress_cell_calls = 0;
  g_last_energy_h = 0.0;
  memset(g_call_n_atoms, 0, sizeof(g_call_n_atoms));
  memset(g_call_has_cell, 0, sizeof(g_call_has_cell));
  memset(g_call_charge, 0, sizeof(g_call_charge));
  memset(g_call_multiplicity, 0, sizeof(g_call_multiplicity));
  memset(g_call_atomic_numbers, 0, sizeof(g_call_atomic_numbers));
  memset(g_call_positions_ang, 0, sizeof(g_call_positions_ang));
  memset(g_call_cell_ang, 0, sizeof(g_call_cell_ang));
  memset(g_hessian_n_atoms, 0, sizeof(g_hessian_n_atoms));
  memset(g_hessian_has_cell, 0, sizeof(g_hessian_has_cell));
  memset(g_hessian_charge, 0, sizeof(g_hessian_charge));
  memset(g_hessian_multiplicity, 0, sizeof(g_hessian_multiplicity));
  memset(g_hessian_atomic_numbers, 0, sizeof(g_hessian_atomic_numbers));
  memset(g_hessian_positions_ang, 0, sizeof(g_hessian_positions_ang));
  memset(g_hessian_cell_ang, 0, sizeof(g_hessian_cell_ang));
  memset(g_dipole_n_atoms, 0, sizeof(g_dipole_n_atoms));
  memset(g_dipole_has_cell, 0, sizeof(g_dipole_has_cell));
  memset(g_dipole_charge, 0, sizeof(g_dipole_charge));
  memset(g_dipole_multiplicity, 0, sizeof(g_dipole_multiplicity));
  memset(g_dipole_atomic_numbers, 0, sizeof(g_dipole_atomic_numbers));
  memset(g_dipole_positions_ang, 0, sizeof(g_dipole_positions_ang));
  memset(g_dipole_cell_ang, 0, sizeof(g_dipole_cell_ang));
  memset(g_polarizability_n_atoms, 0, sizeof(g_polarizability_n_atoms));
  memset(g_polarizability_has_cell, 0, sizeof(g_polarizability_has_cell));
  memset(g_polarizability_charge, 0, sizeof(g_polarizability_charge));
  memset(g_polarizability_multiplicity, 0,
         sizeof(g_polarizability_multiplicity));
  memset(g_polarizability_atomic_numbers, 0,
         sizeof(g_polarizability_atomic_numbers));
  memset(g_polarizability_positions_ang, 0,
         sizeof(g_polarizability_positions_ang));
  memset(g_polarizability_cell_ang, 0, sizeof(g_polarizability_cell_ang));
  memset(g_quadrupole_n_atoms, 0, sizeof(g_quadrupole_n_atoms));
  memset(g_quadrupole_has_cell, 0, sizeof(g_quadrupole_has_cell));
  memset(g_quadrupole_charge, 0, sizeof(g_quadrupole_charge));
  memset(g_quadrupole_multiplicity, 0, sizeof(g_quadrupole_multiplicity));
  memset(g_quadrupole_atomic_numbers, 0,
         sizeof(g_quadrupole_atomic_numbers));
  memset(g_quadrupole_positions_ang, 0,
         sizeof(g_quadrupole_positions_ang));
  memset(g_quadrupole_cell_ang, 0, sizeof(g_quadrupole_cell_ang));
  memset(g_optimize_n_atoms, 0, sizeof(g_optimize_n_atoms));
  memset(g_optimize_has_cell, 0, sizeof(g_optimize_has_cell));
  memset(g_optimize_charge, 0, sizeof(g_optimize_charge));
  memset(g_optimize_multiplicity, 0, sizeof(g_optimize_multiplicity));
  memset(g_optimize_atomic_numbers, 0, sizeof(g_optimize_atomic_numbers));
  memset(g_optimize_positions_ang, 0, sizeof(g_optimize_positions_ang));
  memset(g_optimize_cell_ang, 0, sizeof(g_optimize_cell_ang));
  memset(g_frequency_n_atoms, 0, sizeof(g_frequency_n_atoms));
  memset(g_frequency_has_cell, 0, sizeof(g_frequency_has_cell));
  memset(g_frequency_charge, 0, sizeof(g_frequency_charge));
  memset(g_frequency_multiplicity, 0, sizeof(g_frequency_multiplicity));
  memset(g_frequency_atomic_numbers, 0, sizeof(g_frequency_atomic_numbers));
  memset(g_frequency_positions_ang, 0, sizeof(g_frequency_positions_ang));
  memset(g_frequency_cell_ang, 0, sizeof(g_frequency_cell_ang));
  memset(g_stress_n_atoms, 0, sizeof(g_stress_n_atoms));
  memset(g_stress_has_cell, 0, sizeof(g_stress_has_cell));
  memset(g_stress_charge, 0, sizeof(g_stress_charge));
  memset(g_stress_multiplicity, 0, sizeof(g_stress_multiplicity));
  memset(g_stress_atomic_numbers, 0, sizeof(g_stress_atomic_numbers));
  memset(g_stress_positions_ang, 0, sizeof(g_stress_positions_ang));
  memset(g_stress_cell_ang, 0, sizeof(g_stress_cell_ang));
}

static void assert_close(double actual, double expected, double tolerance) {
  assert_true(actual > expected - tolerance);
  assert_true(actual < expected + tolerance);
}

static void assert_potential_result(const unsigned char *message,
                                    size_t message_size,
                                    double expected_energy,
                                    const double *expected_forces,
                                    size_t expected_force_count,
                                    double tolerance) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);
  struct PotentialResult result;
  read_PotentialResult(&result, root);
  assert_close(result.energy, expected_energy, tolerance);
  capn_resolve(&result.forces.p);
  assert_int_equal(result.forces.p.type, CAPN_LIST);
  assert_int_equal(result.forces.p.datasz, 8);
  assert_int_equal(result.forces.p.len, (int)expected_force_count);
  for (size_t i = 0; i < expected_force_count; ++i) {
    double actual = capn_to_f64(capn_get64(result.forces, (int)i));
    assert_close(actual, expected_forces[i], tolerance);
  }
  capn_free(&arena);
}

static void assert_potential_result_energy_only(const unsigned char *message,
                                                size_t message_size,
                                                double expected_energy,
                                                double tolerance) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);
  struct PotentialResult result;
  read_PotentialResult(&result, root);
  assert_close(result.energy, expected_energy, tolerance);
  capn_resolve(&result.forces.p);
  if (result.forces.p.type != CAPN_NULL) {
    assert_int_equal(result.forces.p.type, CAPN_LIST);
    assert_int_equal(result.forces.p.datasz, 8);
    assert_int_equal(result.forces.p.len, 0);
  }
  capn_free(&arena);
}

static void assert_potential_result_gradient(const unsigned char *message,
                                             size_t message_size,
                                             double expected_energy,
                                             const double *expected_gradient,
                                             size_t expected_gradient_count,
                                             double tolerance) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);
  struct PotentialResult result;
  read_PotentialResult(&result, root);
  assert_close(result.energy, expected_energy, tolerance);
  capn_resolve(&result.gradient.p);
  assert_int_equal(result.gradient.p.type, CAPN_LIST);
  assert_int_equal(result.gradient.p.datasz, 8);
  assert_int_equal(result.gradient.p.len, (int)expected_gradient_count);
  for (size_t i = 0; i < expected_gradient_count; ++i) {
    double actual = capn_to_f64(capn_get64(result.gradient, (int)i));
    assert_close(actual, expected_gradient[i], tolerance);
  }
  capn_free(&arena);
}

static void assert_potential_result_hessian(const unsigned char *message,
                                            size_t message_size,
                                            double expected_energy,
                                            const double *expected_hessian,
                                            size_t expected_hessian_count,
                                            double tolerance) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);
  struct PotentialResult result;
  read_PotentialResult(&result, root);
  assert_close(result.energy, expected_energy, tolerance);
  capn_resolve(&result.hessian.p);
  assert_int_equal(result.hessian.p.type, CAPN_LIST);
  assert_int_equal(result.hessian.p.datasz, 8);
  assert_int_equal(result.hessian.p.len, (int)expected_hessian_count);
  for (size_t i = 0; i < expected_hessian_count; ++i) {
    double actual = capn_to_f64(capn_get64(result.hessian, (int)i));
    assert_close(actual, expected_hessian[i], tolerance);
  }
  capn_free(&arena);
}

static void assert_potential_result_dipole(const unsigned char *message,
                                           size_t message_size,
                                           double expected_energy,
                                           const double *expected_dipole,
                                           double tolerance) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);
  struct PotentialResult result;
  read_PotentialResult(&result, root);
  assert_close(result.energy, expected_energy, tolerance);
  capn_resolve(&result.dipole.p);
  assert_int_equal(result.dipole.p.type, CAPN_LIST);
  assert_int_equal(result.dipole.p.datasz, 8);
  assert_int_equal(result.dipole.p.len, 3);
  for (int i = 0; i < 3; ++i) {
    double actual = capn_to_f64(capn_get64(result.dipole, i));
    assert_close(actual, expected_dipole[i], tolerance);
  }
  capn_free(&arena);
}

static void assert_potential_result_quadrupole(const unsigned char *message,
                                               size_t message_size,
                                               double expected_energy,
                                               const double *expected,
                                               double tolerance) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);
  struct PotentialResult result;
  read_PotentialResult(&result, root);
  assert_close(result.energy, expected_energy, tolerance);
  capn_resolve(&result.quadrupole.p);
  assert_int_equal(result.quadrupole.p.type, CAPN_LIST);
  assert_int_equal(result.quadrupole.p.datasz, 8);
  assert_int_equal(result.quadrupole.p.len, 6);
  for (int i = 0; i < 6; ++i) {
    double actual = capn_to_f64(capn_get64(result.quadrupole, i));
    assert_close(actual, expected[i], tolerance);
  }
  capn_free(&arena);
}

static void assert_potential_result_stress(const unsigned char *message,
                                           size_t message_size,
                                           double expected_energy,
                                           const double *expected,
                                           double tolerance) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);
  struct PotentialResult result;
  read_PotentialResult(&result, root);
  assert_close(result.energy, expected_energy, tolerance);
  capn_resolve(&result.stress.p);
  assert_int_equal(result.stress.p.type, CAPN_LIST);
  assert_int_equal(result.stress.p.datasz, 8);
  assert_int_equal(result.stress.p.len, 9);
  for (int i = 0; i < 9; ++i) {
    double actual = capn_to_f64(capn_get64(result.stress, i));
    assert_close(actual, expected[i], tolerance);
  }
  capn_free(&arena);
}

static void assert_potential_result_optimized(
    const unsigned char *message, size_t message_size, double expected_energy,
    const double *expected_positions, size_t expected_position_count,
    double tolerance) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);
  struct PotentialResult result;
  read_PotentialResult(&result, root);
  assert_close(result.energy, expected_energy, tolerance);
  capn_resolve(&result.optimizedPos.p);
  assert_int_equal(result.optimizedPos.p.type, CAPN_LIST);
  assert_int_equal(result.optimizedPos.p.datasz, 8);
  assert_int_equal(result.optimizedPos.p.len, (int)expected_position_count);
  for (size_t i = 0; i < expected_position_count; ++i) {
    double actual = capn_to_f64(capn_get64(result.optimizedPos, (int)i));
    assert_close(actual, expected_positions[i], tolerance);
  }
  capn_free(&arena);
}

static void assert_potential_result_frequencies(
    const unsigned char *message, size_t message_size, double expected_energy,
    const double *expected_frequencies, const double *expected_intensities,
    const double *expected_projected_frequencies,
    const double *expected_projected_intensities,
    size_t expected_count, double tolerance) {
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, message, message_size, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(&arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);
  struct PotentialResult result;
  read_PotentialResult(&result, root);
  assert_close(result.energy, expected_energy, tolerance);
  assert_close(result.zeroPointEnergy, 0.001, tolerance);
  assert_close(result.thermalEnergy, 0.002, tolerance);
  assert_close(result.thermalEnthalpy, 0.003, tolerance);
  assert_close(result.entropy, 4.0, tolerance);
  assert_close(result.heatCapacityCv, 5.0, tolerance);
  capn_resolve(&result.frequencies.p);
  assert_int_equal(result.frequencies.p.type, CAPN_LIST);
  assert_int_equal(result.frequencies.p.datasz, 8);
  assert_int_equal(result.frequencies.p.len, (int)expected_count);
  capn_resolve(&result.intensities.p);
  assert_int_equal(result.intensities.p.type, CAPN_LIST);
  assert_int_equal(result.intensities.p.datasz, 8);
  assert_int_equal(result.intensities.p.len, (int)expected_count);
  capn_resolve(&result.normalModes.p);
  assert_int_equal(result.normalModes.p.type, CAPN_LIST);
  assert_int_equal(result.normalModes.p.datasz, 8);
  assert_int_equal(result.normalModes.p.len,
                   (int)(expected_count * expected_count));
  capn_resolve(&result.projectedFrequencies.p);
  assert_int_equal(result.projectedFrequencies.p.type, CAPN_LIST);
  assert_int_equal(result.projectedFrequencies.p.datasz, 8);
  assert_int_equal(result.projectedFrequencies.p.len, (int)expected_count);
  capn_resolve(&result.projectedIntensities.p);
  assert_int_equal(result.projectedIntensities.p.type, CAPN_LIST);
  assert_int_equal(result.projectedIntensities.p.datasz, 8);
  assert_int_equal(result.projectedIntensities.p.len, (int)expected_count);
  for (size_t i = 0; i < expected_count; ++i) {
    double actual_frequency =
        capn_to_f64(capn_get64(result.frequencies, (int)i));
    double actual_intensity =
        capn_to_f64(capn_get64(result.intensities, (int)i));
    assert_close(actual_frequency, expected_frequencies[i], tolerance);
    assert_close(actual_intensity, expected_intensities[i], tolerance);
    double actual_projected_frequency =
        capn_to_f64(capn_get64(result.projectedFrequencies, (int)i));
    double actual_projected_intensity =
        capn_to_f64(capn_get64(result.projectedIntensities, (int)i));
    assert_close(actual_projected_frequency,
                 expected_projected_frequencies[i], tolerance);
    assert_close(actual_projected_intensity,
                 expected_projected_intensities[i], tolerance);
  }
  for (size_t i = 0; i < expected_count * expected_count; ++i) {
    double actual_mode = capn_to_f64(capn_get64(result.normalModes, (int)i));
    assert_close(actual_mode, 0.001 * (double)(i + 1), tolerance);
  }
  capn_free(&arena);
}

static int find_typed_set_key(const char *key) {
  for (int i = 0; i < g_typed_set_count && i < 512; ++i) {
    if (strcmp(g_typed_set_keys[i], key) == 0)
      return i;
  }
  return -1;
}

static void assert_set_string(const char *key, const char *value) {
  for (int i = 0; i < g_set_string_count && i < 8; ++i) {
    if (strcmp(g_set_keys[i], key) == 0) {
      assert_string_equal(g_set_values[i], value);
      return;
    }
  }
  fail_msg("missing set string key: %s", key);
}

static void assert_typed_set_scalar(const char *key, int value_type,
                                    const char *value) {
  int index = find_typed_set_key(key);
  assert_true(index >= 0);
  assert_int_equal(g_typed_set_types[index], value_type);
  assert_int_equal(g_typed_set_value_counts[index], 1);
  assert_string_equal(g_typed_set_values[index][0], value);
}

static void assert_typed_set_scalar_entry(const char *key, int value_type,
                                          const char *value) {
  for (int i = 0; i < g_typed_set_count && i < 512; ++i) {
    if (strcmp(g_typed_set_keys[i], key) != 0)
      continue;
    if (g_typed_set_types[i] == value_type &&
        g_typed_set_value_counts[i] == 1 &&
        strcmp(g_typed_set_values[i][0], value) == 0)
      return;
  }
  fail_msg("missing typed RTDB scalar %s=%s", key, value);
}

static void assert_typed_set_pair(const char *key, int value_type,
                                  const char *first, const char *second) {
  int index = find_typed_set_key(key);
  assert_true(index >= 0);
  assert_int_equal(g_typed_set_types[index], value_type);
  assert_int_equal(g_typed_set_value_counts[index], 2);
  assert_string_equal(g_typed_set_values[index][0], first);
  assert_string_equal(g_typed_set_values[index][1], second);
}

static void assert_typed_set_triple(const char *key, int value_type,
                                    const char *first, const char *second,
                                    const char *third) {
  int index = find_typed_set_key(key);
  assert_true(index >= 0);
  assert_int_equal(g_typed_set_types[index], value_type);
  assert_int_equal(g_typed_set_value_counts[index], 3);
  assert_string_equal(g_typed_set_values[index][0], first);
  assert_string_equal(g_typed_set_values[index][1], second);
  assert_string_equal(g_typed_set_values[index][2], third);
}

static void assert_typed_set_values(const char *key, int value_type,
                                    int nvalues,
                                    const char *const *expected) {
  int index = find_typed_set_key(key);
  assert_true(index >= 0);
  assert_int_equal(g_typed_set_types[index], value_type);
  assert_int_equal(g_typed_set_value_counts[index], nvalues);
  for (int i = 0; i < nvalues; ++i)
    assert_string_equal(g_typed_set_values[index][i], expected[i]);
}

static void assert_brillouin_dos_zone(int index, const char *zone_name, int x,
                                      int y, int z) {
  assert_true(index >= 0);
  assert_true(index < g_brillouin_dos_zone_count);
  assert_string_equal(g_brillouin_dos_zone_names[index], zone_name);
  assert_int_equal(g_brillouin_dos_zone_grids[index][0], x);
  assert_int_equal(g_brillouin_dos_zone_grids[index][1], y);
  assert_int_equal(g_brillouin_dos_zone_grids[index][2], z);
}

static void test_embed_config_uses_direct_dft_values(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_reset_rtdb_calls, 1);
  assert_string_equal(g_basis, "sto-3g");
  assert_string_equal(g_theory, "dft");
  assert_string_equal(g_scf_type, "pbe0");
  assert_int_equal(g_set_dft_direct_calls, 1);
  assert_string_equal(g_dft_xc, "pbe0");
  assert_int_equal(g_dft_direct_enabled, 1);
  assert_int_equal(g_dft_smearing_enabled, 1);
  assert_true(g_dft_smear_sigma_hartree > 0.000999);
  assert_true(g_dft_smear_sigma_hartree < 0.001001);
  assert_int_equal(g_dft_smearing_spinset, 0);
  assert_null(strstr(g_input_blocks, "smear 0.001"));
  assert_null(strstr(g_input_blocks, "xc pbe0"));
  assert_null(strstr(g_input_blocks, "  direct"));
  assert_null(strstr(g_input_blocks, "pseudopotentials"));
  assert_null(strstr(g_input_blocks, "Si library sg15"));
  assert_null(strstr(g_input_blocks, "H pspw_library hgh_lda"));
  assert_null(strstr(g_input_blocks, "O paw_library paw_default"));
  assert_null(strstr(g_input_blocks, "C cpi C.cpi"));
  assert_null(strstr(g_input_blocks, "N teter N.teter"));
  assert_null(strstr(g_input_blocks, "* pspw_library pspw_default"));
  assert_null(strstr(g_input_blocks, "uterm d 1.4"));
  assert_null(strstr(g_input_blocks, "energy_cutoff 12.5"));
  assert_null(strstr(g_input_blocks, "wavefunction_cutoff 6.25"));
  assert_null(strstr(g_input_blocks, "ewald_rcut 3.5"));
  assert_null(strstr(g_input_blocks, "ewald_ncut 9"));
  assert_null(strstr(g_input_blocks, "cell_name cellA"));
  assert_null(strstr(g_input_blocks, "input_wavefunction_filename psi.in"));
  assert_null(strstr(g_input_blocks, "output_wavefunction_filename psi.out"));
  assert_null(strstr(g_input_blocks, "fake_mass 2.5"));
  assert_null(strstr(g_input_blocks, "time_step 4.5"));
  assert_null(strstr(g_input_blocks, "loop 3 7"));
  assert_null(strstr(g_input_blocks, "tolerances 0.125 0.25 0.5"));
  assert_null(strstr(g_input_blocks, "exchange_correlation pbe96"));
  assert_null(strstr(g_input_blocks, "nobalance"));
  assert_null(strstr(g_input_blocks, "bo_steps 11 22"));
  assert_null(strstr(g_input_blocks, "bo_time_step 0.75"));
  assert_null(strstr(g_input_blocks, "bo_algorithm velocity-verlet"));
  assert_null(strstr(g_input_blocks, "bo_fake_mass 333"));
  assert_null(strstr(g_input_blocks, "scaling 1.5 2.5"));
  assert_null(strstr(g_input_blocks, "np_dimensions 2 3 4"));
  assert_null(strstr(g_input_blocks, "spin_orbit off"));
  assert_null(strstr(g_input_blocks, "parallel_io on"));
  assert_null(strstr(g_input_blocks, "xyz_filename traj.xyz"));
  assert_null(strstr(g_input_blocks, "ion_motion_filename ion.mov"));
  assert_null(strstr(g_input_blocks, "emotion_filename electron.mov"));
  assert_null(strstr(g_input_blocks, "hmotion_filename h.mov"));
  assert_null(strstr(g_input_blocks, "omotion_filename orb.mov"));
  assert_null(strstr(g_input_blocks, "eigmotion_filename eig.mov"));
  assert_null(strstr(g_input_blocks, "fractional_orbitals 5 6"));
  assert_null(strstr(g_input_blocks, "occupation 1.75 2"));
  assert_null(strstr(g_input_blocks, "extra_orbitals 3"));
  assert_null(strstr(g_input_blocks,
                     "smear temperature 0.02 alpha 0.7 fermi orbitals 5 6"));
  assert_null(strstr(g_input_blocks, "virtual 7 8"));
  assert_null(strstr(g_input_blocks, "virtual_orbitals 7 8"));
  assert_null(strstr(g_input_blocks, "lcao_skip"));
  assert_null(strstr(g_input_blocks, "lcao_mask up 1 3 5"));
  assert_null(strstr(g_input_blocks, "lcao_mask down 2 4"));
  assert_null(strstr(g_input_blocks, "ewald_ngrid 9 10 11"));
  assert_null(strstr(g_input_blocks, "Nose-Hoover 12 300 34 400 start 2 3"));
  assert_null(strstr(g_input_blocks, "monkhorst-pack 3 4 -5 zoneA"));
  assert_null(strstr(g_input_blocks, "zone_name zoneA"));
  assert_null(strstr(g_input_blocks, "max_kpoints_print 12"));
  assert_null(strstr(g_input_blocks, "simulation_cell"));
  assert_null(strstr(g_input_blocks, "boundary_conditions periodic"));
  assert_null(strstr(g_input_blocks, "lattice_vectors"));
  assert_null(strstr(g_input_blocks, "ngrid 20 22 24"));
  assert_null(strstr(g_input_blocks, "ngrid_small 10 11 12"));
  assert_null(strstr(g_input_blocks, "box_delta 1"));
  assert_null(strstr(g_input_blocks, "box_orient"));
  assert_null(strstr(g_input_blocks, "box_different_lengths"));
  assert_null(strstr(g_input_blocks, "ccsd\n  maxiter 20"));
  assert_null(strstr(g_input_blocks, "freeze 1 virtual 2"));
  assert_null(strstr(g_input_blocks, "nodisk"));
  assert_non_null(strstr(
      g_input_blocks,
      "ccsd\n"
      "  print high reference\n"
      "  noprint byproduct energies\n"
      "  doa 1 0 1\n"
      "  dob 2 0\n"
      "  dog 3\n"
      "  doh 4 5\n"
      "  dojk 6\n"
      "  dos 7 8 9\n"
      "  dod 10\n"
      "end"));
  assert_null(strstr(g_input_blocks, "tce\n  dft"));
  assert_null(strstr(g_input_blocks, "cr-ccsd(t)"));
  assert_null(strstr(g_input_blocks, "lshift 0.01"));
  assert_null(strstr(g_input_blocks, "lshift2 0.03 0.04"));
  assert_null(strstr(g_input_blocks, "io ga_eaf"));
  assert_null(strstr(g_input_blocks, "densmat dens.dat"));
  assert_null(strstr(g_input_blocks, "nofock"));
  assert_null(strstr(g_input_blocks, "active_oa 5"));
  assert_null(strstr(g_input_blocks, "tcc_spaces"));
  assert_non_null(strstr(g_input_blocks, "tce\n  print debug tile time\nend"));
  assert_non_null(strstr(g_input_blocks, "mrccdata\n  se4t"));
  assert_non_null(strstr(g_input_blocks, "  cas 2 2\n"));
  assert_non_null(strstr(g_input_blocks, "  2222ba\n"));
  assert_non_null(strstr(g_input_blocks, "tce\n  freeze core atomic\nend"));
  assert_null(strstr(g_input_blocks, "dipole"));
  assert_null(strstr(g_input_blocks, "atom_efield"));
  assert_null(strstr(g_input_blocks, "atom_efield_grad"));
  assert_null(strstr(g_input_blocks, "mulliken kawai"));
  assert_null(strstr(g_input_blocks, "nwpw:mulliken"));
  assert_null(strstr(g_input_blocks, "periodic_dipole"));
  assert_null(strstr(g_input_blocks, "nwpw:periodic_dipole"));
  assert_null(strstr(g_input_blocks, "efield true"));
  assert_null(strstr(g_input_blocks, "nwpw:efield"));
  assert_null(strstr(g_input_blocks, "smooth_cutoff"));
  assert_null(strstr(g_input_blocks, "nwpw:smooth_cutoff"));
  assert_null(strstr(g_input_blocks, "cutoff_boot_wavefunction"));
  assert_null(strstr(g_input_blocks, "nwpw:cutoff_boot_psi"));
  assert_null(strstr(g_input_blocks, "fast_erf"));
  assert_null(strstr(g_input_blocks, "nwpw:fast_erf"));
  assert_null(strstr(g_input_blocks, "dipole_motion"));
  assert_null(strstr(g_input_blocks, "nwpw:dipole_motion"));
  assert_null(strstr(g_input_blocks, "symmetry false"));
  assert_null(strstr(g_input_blocks, "nwpw:rho_use_symmetry"));
  assert_null(strstr(g_input_blocks, "one_electron_guess 25"));
  assert_null(strstr(g_input_blocks, "nwpw:H1_it_in"));
  assert_null(strstr(g_input_blocks, "fmm true"));
  assert_null(strstr(g_input_blocks, "nwpw:fmm"));
  assert_null(strstr(g_input_blocks, "born 78.4"));
  assert_null(strstr(g_input_blocks, "nwpw:born"));
  assert_null(strstr(g_input_blocks, "vfield vf_a.ascii"));
  assert_null(strstr(g_input_blocks, "nwpw:vfield_filenames"));
  assert_null(strstr(g_input_blocks, "single_precision_hfx"));
  assert_null(strstr(g_input_blocks, "pspw:HFX_single_precision"));
  assert_null(strstr(g_input_blocks, "geometry_optimize"));
  assert_null(strstr(g_input_blocks, "cgsd:geometry_optimize"));
  assert_null(strstr(g_input_blocks, "auxiliary_potentials"));
  assert_null(strstr(g_input_blocks, "pspw_qmmm_auxon"));
  assert_null(strstr(g_input_blocks, "mult 3"));
  assert_null(strstr(g_input_blocks, "cgsd:mult"));
  assert_null(strstr(g_input_blocks, "dos 0.0025"));
  assert_null(strstr(g_input_blocks, "dos_filename dos.dat"));
  assert_null(strstr(g_input_blocks, "dos:alpha"));
  assert_null(strstr(g_input_blocks, "nwpw:dos:filename"));
  assert_null(strstr(g_input_blocks, "cpmd_properties true"));
  assert_null(strstr(g_input_blocks, "nwpw:cpmd_properties"));
  assert_null(strstr(g_input_blocks, "use_grid_cmp false"));
  assert_null(strstr(g_input_blocks, "nwpw:use_grid_cmp"));
  assert_null(strstr(g_input_blocks, "director director.dat"));
  assert_null(strstr(g_input_blocks, "nwpw:use_director"));
  assert_null(strstr(g_input_blocks, "expand_cell 2 3 4"));
  assert_null(strstr(g_input_blocks, "nwpw:cell_expand"));
  assert_null(strstr(g_input_blocks, "mapping 3"));
  assert_null(strstr(g_input_blocks, "nwpw:mapping"));
  assert_null(strstr(g_input_blocks, "rotation false"));
  assert_null(strstr(g_input_blocks, "nwpw:rotation"));
  assert_null(strstr(g_input_blocks, "integrate_mult_l 4"));
  assert_null(strstr(g_input_blocks, "nwpw:lmax_multipole"));
  assert_null(strstr(g_input_blocks, "Fei fei.dat"));
  assert_null(strstr(g_input_blocks, "nwpw:fei"));
  assert_null(strstr(g_input_blocks, "cpmd:fei"));
  assert_null(strstr(g_input_blocks, "initial_velocities 298.15"));
  assert_null(strstr(g_input_blocks, "nwpw:init_velocities"));
  assert_null(strstr(g_input_blocks, "makehmass2 false"));
  assert_null(strstr(g_input_blocks, "nwpw:makehmass2"));
  assert_null(strstr(g_input_blocks, "translate_vector 0.1"));
  assert_null(strstr(g_input_blocks, "nwpw:translate_vector"));
  assert_null(strstr(g_input_blocks, "socket ipi_client"));
  assert_null(strstr(g_input_blocks, "nwpw:socket_type"));
  assert_null(strstr(g_input_blocks, "apc 1.25"));
  assert_null(strstr(g_input_blocks, "nwpw_APC:Gc"));
  assert_null(strstr(g_input_blocks, "translation false"));
  assert_null(strstr(g_input_blocks, "cgsd:allow_translation"));
  assert_null(strstr(g_input_blocks, "lmbfgs stiefel"));
  assert_null(strstr(g_input_blocks, "scf density rmm-diis"));
  assert_null(strstr(g_input_blocks, "nwpw:minimizer"));
  assert_null(strstr(g_input_blocks, "nwpw:ks_algorithm"));
  assert_null(strstr(g_input_blocks, "nwpw:kerker_g0"));
  assert_null(strstr(g_input_blocks, "pspspin off"));
  assert_null(strstr(g_input_blocks, "nwpw:psp:semicore_small"));
  assert_non_null(strstr(g_input_blocks, "print debug tile time"));
  assert_non_null(strstr(g_input_blocks, "iterations 40"));
  assert_non_null(strstr(g_input_blocks, "set int:acc_std 1e-8"));
  assert_int_equal(g_set_rtdb_strings_calls, 1);
  assert_int_equal(g_set_string_count, 4);
  assert_set_string("band_structure:zone_name", "structureA");
  assert_set_string("band_fft:zone_name", "fftA");
  assert_set_string("nwpw:vfield_filenames", "vf_a.ascii vf_b.ascii");
  assert_set_string("nwpw:dos:filename", "dos.dat");
  assert_int_equal(g_set_rtdb_values_calls, 1);
  assert_int_equal(g_typed_set_count, 259);
  assert_typed_set_scalar("cgsd:ecut", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "12.5");
  assert_typed_set_scalar("band:wcut", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "6.25");
  assert_typed_set_scalar("cpsd:rcut", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "3.5");
  assert_typed_set_scalar("cpmd:ncut", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "9");
  assert_typed_set_scalar("cgsd:cell_name", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                          "cellA");
  assert_typed_set_scalar("pspw:input vectors", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                          "psi.in");
  assert_typed_set_scalar("band:input vectors", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                          "psi.in");
  assert_typed_set_scalar("band:output_wavefunction_filename",
                          NWCHEMC_DIRECT_SET_VALUE_TEXT, "psi.out");
  assert_typed_set_scalar("cpsd:fake_mass", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "2.5");
  assert_typed_set_scalar("band:time_step", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "4.5");
  assert_typed_set_pair("cgsd:loop", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "3",
                        "7");
  assert_typed_set_triple("band:tolerances", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "0.125", "0.25", "0.5");
  assert_typed_set_scalar("cgsd:exchange_correlation",
                          NWCHEMC_DIRECT_SET_VALUE_TEXT, "pbe96");
  assert_typed_set_scalar("cpmd:exchange_correlation",
                          NWCHEMC_DIRECT_SET_VALUE_TEXT, "pbe96");
  assert_typed_set_scalar("pspw:SIC_all", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "false");
  assert_typed_set_scalar("pspw:HFX", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "false");
  assert_typed_set_scalar("band:HFX", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "false");
  assert_typed_set_scalar("pspw:SIC_xc_parameter",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "1");
  assert_typed_set_scalar("band:HFX_parameter",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "1");
  assert_typed_set_scalar("nwpw:balance", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "false");
  assert_typed_set_pair("nwpw:bo_steps", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                        "11", "22");
  assert_typed_set_scalar("nwpw:bo_time_step",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "0.75");
  assert_typed_set_scalar("nwpw:bo_algorithm",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "1");
  assert_typed_set_scalar("nwpw:bo_fake_mass",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "333");
  assert_typed_set_pair("nwpw:scaling", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "1.5",
                        "2.5");
  assert_typed_set_pair("cpmd:scaling", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "1.5",
                        "2.5");
  assert_typed_set_scalar("nwpw:scaling_natms",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "3");
  const char *scaling_atoms[3] = {"1", "3", "5"};
  assert_typed_set_values("nwpw:scaling_atoms",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, 3,
                          scaling_atoms);
  assert_typed_set_triple("nwpw:np_dimensions",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "2", "3", "4");
  assert_typed_set_scalar("nwpw:spin_orbit", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "false");
  assert_typed_set_scalar("pspw:spin_orbit", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "false");
  assert_typed_set_scalar("band:spin_orbit", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "false");
  assert_typed_set_scalar("nwpw:parallel_io", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_typed_set_scalar("nwpw:pspspin", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "false");
  assert_typed_set_scalar("nwpw:pspspin_count",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "0");
  assert_typed_set_scalar("nwpw:psp:semicore_small",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true");
  assert_typed_set_scalar("nwpw:uterm", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_typed_set_scalar("nwpw:nuterms", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "1");
  assert_typed_set_scalar("nwpw:uterm_scale:_000001",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "1.4");
  assert_typed_set_scalar("nwpw:jterm_scale:_000001",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "0.2");
  assert_typed_set_scalar("nwpw:uterm_l:_000001",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "2");
  const char *uterm_ions[2] = {"4", "6"};
  assert_typed_set_values("nwpw:uterm_ions:_000001",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, 2, uterm_ions);
  assert_typed_set_scalar("cpmd:xyz_filename", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                          "traj.xyz");
  assert_typed_set_scalar("nwpw:xyz_filename", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                          "traj.xyz");
  assert_typed_set_scalar("cpmd:ion_motion_filename",
                          NWCHEMC_DIRECT_SET_VALUE_TEXT, "ion.mov");
  assert_typed_set_scalar("nwpw:ion_motion_filename",
                          NWCHEMC_DIRECT_SET_VALUE_TEXT, "ion.mov");
  assert_typed_set_scalar("cpmd:emotion_filename",
                          NWCHEMC_DIRECT_SET_VALUE_TEXT, "electron.mov");
  assert_typed_set_scalar("nwpw:emotion_filename",
                          NWCHEMC_DIRECT_SET_VALUE_TEXT, "electron.mov");
  assert_typed_set_scalar("cpmd:hmotion_filename",
                          NWCHEMC_DIRECT_SET_VALUE_TEXT, "h.mov");
  assert_typed_set_scalar("nwpw:hmotion_filename",
                          NWCHEMC_DIRECT_SET_VALUE_TEXT, "h.mov");
  assert_typed_set_scalar("cpmd:omotion_filename",
                          NWCHEMC_DIRECT_SET_VALUE_TEXT, "orb.mov");
  assert_typed_set_scalar("nwpw:omotion_filename",
                          NWCHEMC_DIRECT_SET_VALUE_TEXT, "orb.mov");
  assert_typed_set_scalar("cpmd:eigmotion_filename",
                          NWCHEMC_DIRECT_SET_VALUE_TEXT, "eig.mov");
  assert_typed_set_scalar("nwpw:eigmotion_filename",
                          NWCHEMC_DIRECT_SET_VALUE_TEXT, "eig.mov");
  assert_typed_set_pair("nwpw:fractional_orbitals",
                        NWCHEMC_DIRECT_SET_VALUE_INTEGER, "5", "6");
  assert_typed_set_scalar("nwpw:frac_occ:number_states",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "3");
  const char *occupation_values[3] = {"1.75", "0.5", "0.25"};
  assert_typed_set_values("nwpw:frac_occ:occupations",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, 3,
                          occupation_values);
  const char *occupation_states[3] = {"2", "4", "7"};
  assert_typed_set_values("nwpw:frac_occ:states",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, 3,
                          occupation_states);
  assert_typed_set_scalar("nwpw:frac_occ:extra_orbitals",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "3");
  assert_typed_set_scalar("nwpw:fractional_temperature",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "0.02");
  assert_typed_set_scalar("nwpw:fractional_alpha",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "0.7");
  assert_typed_set_scalar("nwpw:fractional_smeartype",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "1");
  assert_typed_set_pair("nwpw:excited_ne", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                        "7", "8");
  assert_typed_set_scalar("nwpw:lcao_skip", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_typed_set_scalar("nwpw:lcao_mask", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  const char *lcao_mask_up_orbitals[3] = {"1", "3", "5"};
  assert_typed_set_values("nwpw:lcao_mask_uporbs",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, 3,
                          lcao_mask_up_orbitals);
  const char *lcao_mask_down_orbitals[2] = {"2", "4"};
  assert_typed_set_values("nwpw:lcao_mask_downorbs",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, 2,
                          lcao_mask_down_orbitals);
  assert_typed_set_triple("nwpw:ewald_ngrid",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "9", "10",
                          "11");
  assert_typed_set_scalar("cpmd:nose", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_typed_set_scalar("nwpw:nose", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_typed_set_scalar("cpmd:nose_restart",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "false");
  assert_typed_set_scalar("nwpw:nose_restart",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "false");
  assert_typed_set_scalar("cpmd:Pe", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "12");
  assert_typed_set_scalar("nwpw:Pe", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "12");
  assert_typed_set_scalar("cpmd:Te", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "300");
  assert_typed_set_scalar("nwpw:Te", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "300");
  assert_typed_set_scalar("cpmd:Pr", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "34");
  assert_typed_set_scalar("nwpw:Pr", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "34");
  assert_typed_set_scalar("cpmd:Tr", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "400");
  assert_typed_set_scalar("nwpw:Tr", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "400");
  assert_typed_set_scalar("cpmd:Mchain", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "2");
  assert_typed_set_scalar("nwpw:Mchain", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "2");
  assert_typed_set_scalar("cpmd:Nchain", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "3");
  assert_typed_set_scalar("nwpw:Nchain", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "3");
  assert_typed_set_scalar("nwpw:atom_efield",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true");
  assert_typed_set_scalar("nwpw:atom_efield_grad",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true");
  assert_typed_set_scalar("cgsd:mulliken", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_typed_set_scalar("band:mulliken", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_typed_set_scalar("nwpw:mulliken", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_typed_set_scalar("cpsd:mulliken", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_typed_set_scalar("cpmd:mulliken", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_typed_set_scalar("nwpw:mulliken_kawai",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true");
  assert_typed_set_scalar("nwpw:periodic_dipole",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true");
  assert_typed_set_scalar("nwpw:efield", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  const char *efield_vector_values[3] = {"0.1", "0.2", "0.3"};
  assert_typed_set_values("nwpw:efield_vector",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, 3,
                          efield_vector_values);
  const char *efield_center_values[3] = {"1", "2", "3"};
  assert_typed_set_values("nwpw:efield_center",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, 3,
                          efield_center_values);
  assert_typed_set_scalar("nwpw:efield_type",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "1");
  const char *smooth_cutoff_values[2] = {"1.5", "3.5"};
  assert_typed_set_values("nwpw:smooth_cutoff",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, 2,
                          smooth_cutoff_values);
  assert_typed_set_scalar("nwpw:cutoff_boot_psi",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "false");
  assert_typed_set_scalar("nwpw:fast_erf", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_typed_set_scalar("nwpw:dipole_motion",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true");
  assert_typed_set_scalar("nwpw:dipole_motion_filename",
                          NWCHEMC_DIRECT_SET_VALUE_TEXT, "dipole.mov");
  assert_typed_set_scalar("nwpw:rho_use_symmetry",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "false");
  assert_typed_set_scalar("nwpw:H1_it_in", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "25");
  assert_typed_set_scalar("nwpw:H1_it_out", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "3");
  assert_typed_set_scalar("nwpw:H1_it_ortho",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "2");
  assert_typed_set_scalar("nwpw:fmm", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_typed_set_scalar("nwpw:fmm_lmax", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "12");
  assert_typed_set_scalar("nwpw:fmm_lr", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "2");
  assert_typed_set_scalar("nwpw:born", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_typed_set_scalar("nwpw:born_dielec", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "78.4");
  assert_typed_set_scalar("nwpw:born_relax", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  const char *born_vradii_values[2] = {"1", "2"};
  assert_typed_set_values("nwpw:born_vradii", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          2, born_vradii_values);
  assert_typed_set_scalar("pspw:HFX_single_precision",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true");
  assert_typed_set_scalar("cgsd:geometry_optimize",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true");
  assert_typed_set_scalar("cpsd:geometry_optimize",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true");
  assert_typed_set_scalar("band:geometry_optimize",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true");
  assert_typed_set_scalar("pspw_qmmm_auxon",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true");
  assert_typed_set_scalar("cgsd:ispin", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "2");
  assert_typed_set_scalar("band:ispin", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "2");
  assert_typed_set_scalar("cpsd:ispin", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "2");
  assert_typed_set_scalar("cgsd:mult", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "3");
  assert_typed_set_scalar("band:mult", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "3");
  assert_typed_set_scalar("cpsd:mult", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "3");
  assert_typed_set_scalar("dos:alpha", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "0.0025");
  assert_typed_set_scalar("dos:npoints", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "120");
  assert_typed_set_scalar("dos:emin", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "-0.5");
  assert_typed_set_scalar("dos:emax", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "1.5");
  assert_typed_set_scalar("nwpw:cpmd_properties",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true");
  assert_typed_set_scalar("nwpw:use_grid_cmp",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "false");
  assert_typed_set_scalar("nwpw:use_director",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true");
  assert_typed_set_scalar("nwpw:director_filename",
                          NWCHEMC_DIRECT_SET_VALUE_TEXT, "director.dat");
  assert_typed_set_triple("nwpw:cell_expand",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "2", "3", "4");
  assert_typed_set_scalar("nwpw:mapping", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "3");
  assert_typed_set_scalar("nwpw:rotation", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "false");
  assert_typed_set_scalar("nwpw:lmax_multipole",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "4");
  assert_typed_set_scalar("cpmd:fei_filename", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                          "fei.dat");
  assert_typed_set_scalar("nwpw:fei_filename", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                          "fei.dat");
  assert_typed_set_scalar("cpmd:fei", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_typed_set_scalar("nwpw:fei", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_typed_set_scalar("nwpw:init_velocities_temperature",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "298.15");
  assert_typed_set_scalar("nwpw:init_velocities_seed",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "12345");
  assert_typed_set_scalar("nwpw:init_velocities",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true");
  assert_typed_set_scalar("nwpw:makehmass2",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "false");
  assert_typed_set_triple("nwpw:translate_vector",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "0.1", "0.2",
                          "0.3");
  assert_typed_set_scalar("nwpw:translate_geom_name",
                          NWCHEMC_DIRECT_SET_VALUE_TEXT, "geomA");
  assert_typed_set_scalar("nwpw:translate_reorder",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true");
  assert_typed_set_scalar("nwpw:socket_type", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                          "ipi_client");
  assert_typed_set_scalar("nwpw:socket_ip", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                          "127.0.0.1:31415");
  assert_typed_set_scalar("nwpw_APC:Gc", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "1.25");
  assert_typed_set_scalar("nwpw_APC:nga", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "3");
  const char *apc_gamma_values[3] = {"0.5", "0.25", "0.125"};
  assert_typed_set_values("nwpw_APC:gamma", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          3, apc_gamma_values);
  assert_typed_set_scalar("cgsd:allow_translation",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "false");
  assert_typed_set_scalar("band:allow_translation",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "false");
  assert_typed_set_scalar("nwpw:minimizer", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "8");
  assert_typed_set_scalar("nwpw:ks_algorithm",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "1");
  assert_typed_set_scalar("nwpw:scf_algorithm",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "2");
  assert_typed_set_scalar("nwpw:precondition",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true");
  assert_typed_set_scalar("nwpw:kerker_g0",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "0.375");
  assert_typed_set_scalar("nwpw:ks_alpha",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "0.125");
  assert_typed_set_scalar("nwpw:ks_maxit_orb",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "12");
  assert_typed_set_scalar("nwpw:ks_maxit_orbs",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "4");
  assert_typed_set_scalar("nwpw:diis_histories",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "6");
  assert_typed_set_scalar("cellA:boundry", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                          "periodic");
  const char *unita_values[9] = {"1", "0", "0", "0", "2",
                                 "0", "0", "0", "3"};
  assert_typed_set_values("cellA:unita", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, 9,
                          unita_values);
  assert_typed_set_triple("cellA:ngrid", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "20", "22", "24");
  assert_typed_set_triple("cellA:ngrid_small",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "10", "11",
                          "12");
  assert_typed_set_scalar("cellA:box_delta",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "1");
  assert_typed_set_scalar("cellA:box_orient",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true");
  assert_typed_set_scalar("cellA:box_type", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "1");
  assert_typed_set_scalar("ccsd:maxiter", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "20");
  assert_typed_set_scalar("ccsd:thresh", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "1e-07");
  assert_typed_set_scalar("ccsd:tol2e", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "1e-09");
  assert_typed_set_scalar("ccsd:iprt", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "2");
  assert_typed_set_scalar("ccsd:maxdiis", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "6");
  assert_typed_set_scalar("ccsd:frozen core",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "1");
  assert_typed_set_scalar("ccsd:frozen virtual",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "2");
  assert_typed_set_scalar("ccsd:usedisk", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "false");
  assert_typed_set_scalar("ccsd:fss", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "1.2");
  assert_typed_set_scalar("ccsd:fos", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "0.8");
  assert_typed_set_scalar("ccsd:use_trpdrv_nb",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true");
  assert_typed_set_scalar("ccsd:use_ccsd_omp",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true");
  assert_typed_set_scalar("ccsd:use_trpdrv_omp",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "false");
  assert_typed_set_scalar("ccsd:use_trpdrv_offload",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true");
  assert_typed_set_scalar("tce:model", NWCHEMC_DIRECT_SET_VALUE_TEXT, "ccsd");
  assert_typed_set_scalar("tce:model2e", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                          "2eorb");
  assert_typed_set_scalar("tce:perturbative", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                          "cr_(t)");
  assert_typed_set_scalar("tce:reference", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "0");
  assert_typed_set_scalar("tce:frozen core",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "1");
  assert_typed_set_scalar("tce:frozen virtual",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "2");
  assert_typed_set_scalar("tce:thresh", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "1e-08");
  assert_typed_set_scalar("tce:zlshift", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "0.01");
  assert_typed_set_scalar("tce:zlshiftl", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "0.02");
  assert_typed_set_pair("tce:zlshift2", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                        "0.03", "0.04");
  assert_typed_set_pair("tce:zlshift3", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                        "0.05", "0.06");
  assert_typed_set_scalar("tce:maxiter", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "30");
  assert_typed_set_scalar("tce:ioalg", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "6");
  assert_typed_set_scalar("tce:diis", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "7");
  assert_typed_set_scalar("tce:diis2", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "8");
  assert_typed_set_scalar("tce:diis3", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "9");
  assert_typed_set_scalar("tce:eoms", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "2");
  assert_typed_set_scalar("tce:hbard", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "600");
  assert_typed_set_scalar("tce:nroots", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "4");
  assert_typed_set_scalar("tce:target", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "3");
  assert_typed_set_scalar("tce:targetsym", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                          "b2");
  assert_typed_set_scalar("tce:symmetry", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_typed_set_scalar("tce:densmat", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_typed_set_scalar("tce:file_densmat", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                          "dens.dat");
  assert_typed_set_scalar("tce:left", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_typed_set_scalar("tce:multipole", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "3");
  assert_typed_set_scalar_entry("tce:left", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                                "true");
  assert_typed_set_scalar_entry("tce:multipole",
                                NWCHEMC_DIRECT_SET_VALUE_INTEGER, "1");
  assert_typed_set_scalar("tce:fragment", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "2");
  assert_typed_set_scalar("tce:recompf", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "false");
  assert_typed_set_scalar("tce:active_oa",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "5");
  assert_typed_set_scalar("tce:active_ob",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "4");
  assert_typed_set_scalar("tce:active_va",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "6");
  assert_typed_set_scalar("tce:active_vb",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "7");
  assert_typed_set_scalar("tce:oact", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "8");
  assert_typed_set_scalar("tce:uact", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "9");
  assert_typed_set_scalar("tce:eactmin", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "0.1");
  assert_typed_set_scalar("tce:eactmax", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "0.9");
  assert_typed_set_scalar("tce:act_excit_lvl",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "2");
  assert_typed_set_scalar("tce:maxdiff", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "0.25");
  assert_typed_set_scalar("tce:maxs", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "44");
  assert_typed_set_scalar("tce:ichopx", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "2");
  assert_typed_set_scalar("tce:i4im", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "3");
  assert_typed_set_scalar("tce:idiskx", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "1");
  assert_typed_set_scalar("tce:tilesize", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "40");
  assert_typed_set_scalar("tce:cuda", NWCHEMC_DIRECT_SET_VALUE_INTEGER, "1");
  assert_typed_set_scalar("tce:ltcc", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_int_equal(g_set_nwpw_direct_calls, 1);
  assert_int_equal(g_nwpw_has_options, 1);
  assert_close(g_nwpw_energy_cutoff, 12.5, 1e-12);
  assert_close(g_nwpw_wavefunction_cutoff, 6.25, 1e-12);
  assert_close(g_nwpw_ewald_rcut, 3.5, 1e-12);
  assert_int_equal(g_nwpw_ewald_ncut, 9);
  assert_int_equal(g_set_brillouin_zone_calls, 1);
  assert_int_equal(g_brillouin_has_options, 1);
  assert_string_equal(g_brillouin_zone_name, "zoneA");
  assert_int_equal(g_brillouin_monkhorst_pack[0], 3);
  assert_int_equal(g_brillouin_monkhorst_pack[1], 4);
  assert_int_equal(g_brillouin_monkhorst_pack[2], -5);
  assert_int_equal(g_brillouin_max_kpoints_print, 12);
  assert_int_equal(g_brillouin_kvector_count, 2);
  assert_close(g_brillouin_kvectors[0], 0.0, 1e-12);
  assert_close(g_brillouin_kvectors[1], 0.0, 1e-12);
  assert_close(g_brillouin_kvectors[2], 0.0, 1e-12);
  assert_close(g_brillouin_kvectors[3], 0.5, 1e-12);
  assert_close(g_brillouin_kvectors[4], 0.5, 1e-12);
  assert_close(g_brillouin_kvectors[5], 0.0, 1e-12);
  assert_close(g_brillouin_kvectors[6], 0.0, 1e-12);
  assert_close(g_brillouin_kvectors[7], 0.5, 1e-12);
  assert_int_equal(g_set_pseudopotential_calls, 1);
  assert_int_equal(g_psp_count, 6);
  assert_string_equal(g_psp_elements[0], "Si");
  assert_int_equal(g_psp_types[0],
                   NWChemPseudopotentialEntry_LibraryType_library);
  assert_string_equal(g_psp_names[0], "sg15");
  assert_string_equal(g_psp_elements[1], "H");
  assert_int_equal(g_psp_types[1],
                   NWChemPseudopotentialEntry_LibraryType_pspwLibrary);
  assert_string_equal(g_psp_names[1], "hgh_lda");
  assert_string_equal(g_psp_elements[2], "O");
  assert_int_equal(g_psp_types[2],
                   NWChemPseudopotentialEntry_LibraryType_pawLibrary);
  assert_string_equal(g_psp_names[2], "paw_default");
  assert_string_equal(g_psp_elements[3], "C");
  assert_int_equal(g_psp_types[3],
                   NWChemPseudopotentialEntry_LibraryType_cpi);
  assert_string_equal(g_psp_names[3], "C.cpi");
  assert_string_equal(g_psp_elements[4], "N");
  assert_int_equal(g_psp_types[4],
                   NWChemPseudopotentialEntry_LibraryType_teter);
  assert_string_equal(g_psp_names[4], "N.teter");
  assert_string_equal(g_psp_elements[5], "*");
  assert_int_equal(g_psp_types[5],
                   NWChemPseudopotentialEntry_LibraryType_pspwLibrary);
  assert_string_equal(g_psp_names[5], "pspw_default");

  free(message);
}

static void test_embed_config_promotes_compact_simulation_cells(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message = read_file(g_compact_cells_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_null(strstr(g_input_blocks, "simulation_cell"));
  assert_int_equal(g_set_rtdb_values_calls, 1);
  const char *sc_values[9] = {"6", "0", "0", "0", "6", "0", "0", "0", "6"};
  const char *fcc_values[9] = {"4", "4", "0", "4", "0",
                               "4", "0", "4", "4"};
  const char *bcc_values[9] = {"-5", "5", "5", "5", "-5",
                               "5",  "5", "5", "-5"};
  const char *default_values[9] = {"12", "0",  "0",  "0", "12",
                                   "0",  "0",  "0",  "12"};
  assert_typed_set_values("scCell:unita", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, 9,
                          sc_values);
  assert_typed_set_values("fccCell:unita", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, 9,
                          fcc_values);
  assert_typed_set_values("bccCell:unita", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, 9,
                          bcc_values);
  assert_typed_set_values("cell_default:unita",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, 9, default_values);
  free(message);
}

static void test_embed_config_promotes_tce_method_tokens(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message = read_file(g_tce_methods_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_int_equal(g_set_rtdb_values_calls, 1);
  assert_int_equal(g_typed_set_count, 16);
  assert_null(strstr(g_input_blocks, "dipole"));
  assert_null(strstr(g_input_blocks, "quadrupole"));
  assert_null(strstr(g_input_blocks, "octupole"));
  assert_typed_set_scalar_entry("tce:model", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                                "multi");
  assert_typed_set_scalar_entry("tce:model", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                                "eionly");
  assert_typed_set_scalar_entry("tce:model", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                                "ccsd");
  assert_typed_set_scalar_entry("tce:ccsdvar", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                                "cc2");
  assert_typed_set_scalar_entry("tce:nts", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                                "false");
  assert_typed_set_scalar_entry("tce:perturbative",
                                NWCHEMC_DIRECT_SET_VALUE_TEXT, "lambda(t)");
  assert_typed_set_scalar_entry("tce:left", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                                "true");
  assert_typed_set_scalar_entry("tce:multipole",
                                NWCHEMC_DIRECT_SET_VALUE_INTEGER, "1");
  assert_typed_set_scalar_entry("tce:multipole",
                                NWCHEMC_DIRECT_SET_VALUE_INTEGER, "2");
  assert_typed_set_scalar_entry("tce:multipole",
                                NWCHEMC_DIRECT_SET_VALUE_INTEGER, "3");
  assert_typed_set_scalar_entry("tce:model", NWCHEMC_DIRECT_SET_VALUE_TEXT,
                                "bwccsd");
  assert_typed_set_scalar_entry("tce:mrcc", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                                "1");
  free(message);
}

static void test_embed_config_uses_direct_scf_values(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message = read_file(g_config_options_path, &message_size);
  assert_non_null(message);

  assert_int_equal(nwchemc_set_params(message, message_size), 0);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_set_scf_direct_calls, 1);
  assert_int_equal(g_scf_has_options, 1);
  assert_int_equal(g_scf_maxiter, 50);
  assert_close(g_scf_thresh, 1.0e-6, 1.0e-12);
  assert_close(g_scf_tol2e, 1.0e-9, 1.0e-15);
  assert_int_equal(g_set_driver_direct_calls, 1);
  assert_int_equal(g_driver_has_options, 1);
  assert_int_equal(g_driver_maxiter, 40);
  assert_int_equal(g_driver_tolerance_mode, NWCHEMC_DRIVER_TOLERANCE_TIGHT);
  assert_close(g_driver_gmax_tol, 2.5e-5, 1.0e-12);
  assert_close(g_driver_grms_tol, 1.5e-5, 1.0e-12);
  assert_close(g_driver_xmax_tol, 7.5e-5, 1.0e-12);
  assert_close(g_driver_xrms_tol, 5.5e-5, 1.0e-12);
  /* set strings may include typed-stanza RTDB promo plus explicit set stanzas. */
  assert_true(g_set_rtdb_strings_calls >= 1);
  int found_grid = 0;
  for (int i = 0; i < g_set_string_count; ++i) {
    if (strcmp(g_set_keys[i], "dft:grid") == 0) {
      assert_string_equal(g_set_values[i], "xfine");
      found_grid = 1;
    }
  }
  assert_int_equal(found_grid, 1);
  assert_true(g_set_rtdb_values_calls >= 1);
  assert_true(g_typed_set_count >= 3);
  int found_nopen = 0, found_smear = 0, found_spinset = 0;
  for (int i = 0; i < g_typed_set_count; ++i) {
    if (strcmp(g_typed_set_keys[i], "dft:nopen") == 0) {
      assert_int_equal(g_typed_set_types[i], NWCHEMC_DIRECT_SET_VALUE_INTEGER);
      assert_int_equal(g_typed_set_value_counts[i], 1);
      assert_string_equal(g_typed_set_values[i][0], "2");
      found_nopen = 1;
    } else if (strcmp(g_typed_set_keys[i], "dft:smear_sigma") == 0) {
      assert_int_equal(g_typed_set_types[i], NWCHEMC_DIRECT_SET_VALUE_DOUBLE);
      assert_int_equal(g_typed_set_value_counts[i], 1);
      assert_string_equal(g_typed_set_values[i][0], "0.0015");
      found_smear = 1;
    } else if (strcmp(g_typed_set_keys[i], "dft:spinset") == 0) {
      assert_int_equal(g_typed_set_types[i], NWCHEMC_DIRECT_SET_VALUE_LOGICAL);
      assert_int_equal(g_typed_set_value_counts[i], 1);
      assert_string_equal(g_typed_set_values[i][0], "false");
      found_spinset = 1;
    }
  }
  assert_int_equal(found_nopen, 1);
  assert_int_equal(found_smear, 1);
  assert_int_equal(found_spinset, 1);
  assert_null(strstr(g_input_blocks, "scf\n"));
  assert_null(strstr(g_input_blocks, "maxiter 50"));
  assert_null(strstr(g_input_blocks, "tol2e 1e-09"));
  assert_null(strstr(g_input_blocks, "driver\n"));
  assert_null(strstr(g_input_blocks, "maxiter 40"));
  assert_null(strstr(g_input_blocks, "tight"));
  assert_null(strstr(g_input_blocks, "gmax 2.5e-05"));
  assert_null(strstr(g_input_blocks, "grms 1.5e-05"));
  assert_null(strstr(g_input_blocks, "xmax 7.5e-05"));
  assert_null(strstr(g_input_blocks, "xrms 5.5e-05"));
  assert_null(strstr(g_input_blocks, "task scf energy"));
  assert_null(strstr(g_input_blocks, "set dft:grid xfine"));
  assert_null(strstr(g_input_blocks, "set dft:nopen integer 2"));
  assert_null(strstr(g_input_blocks, "set dft:smear_sigma double 0.0015"));
  assert_null(strstr(g_input_blocks, "set dft:spinset logical false"));

  free(message);
}

/* Typed-only SCF wavefunctionType/nopen + DFT iterations: set_params must RTDB
 * promote (scf:scftype, scf:nopen, dft:iterations) and omit those lines from
 * embed-rendered input_blocks (shipped apply path, not strings(1) theater). */
static void test_embed_promotes_typed_scf_wf_nopen_and_dft_iterations(
    void **state) {
  (void)state;
  reset_embed_captures();

  struct capn arena;
  capn_init_malloc(&arena);
  capn_ptr root = capn_root(&arena);
  assert_int_not_equal(root.type, CAPN_NULL);
  NWChemParams_ptr params = new_NWChemParams(root.seg);
  assert_int_not_equal(params.p.type, CAPN_NULL);

  struct NWChemScfStanza scf;
  memset(&scf, 0, sizeof(scf));
  scf.wavefunctionType = test_text_from_cstr("uhf");
  scf.nopen = 2;

  struct NWChemDftStanza dft;
  memset(&dft, 0, sizeof(dft));
  dft.iterations = 88;

  struct NWChemInputStanza scf_stanza;
  memset(&scf_stanza, 0, sizeof(scf_stanza));
  scf_stanza.kind = NWChemInputStanza_Kind_scf;
  scf_stanza.scf = new_NWChemScfStanza(root.seg);
  write_NWChemScfStanza(&scf, scf_stanza.scf);

  struct NWChemInputStanza dft_stanza;
  memset(&dft_stanza, 0, sizeof(dft_stanza));
  dft_stanza.kind = NWChemInputStanza_Kind_dft;
  dft_stanza.dft = new_NWChemDftStanza(root.seg);
  write_NWChemDftStanza(&dft, dft_stanza.dft);

  struct NWChemParams view;
  memset(&view, 0, sizeof(view));
  view.basis = test_text_from_cstr("sto-3g");
  view.theory = test_text_from_cstr("scf");
  view.scfType = test_text_from_cstr("rhf");
  view.task = test_text_from_cstr("energy");
  view.multiplicity = 1;
  view.inputStanzas = new_NWChemInputStanza_list(root.seg, 2);
  set_NWChemInputStanza(&scf_stanza, view.inputStanzas, 0);
  set_NWChemInputStanza(&dft_stanza, view.inputStanzas, 1);
  write_NWChemParams(&view, params);
  assert_int_equal(capn_setp(root, 0, params.p), 0);

  size_t capacity = 4096u;
  unsigned char *buffer = NULL;
  int written = -1;
  for (int attempt = 0; attempt < 6 && written < 0; ++attempt) {
    unsigned char *next = (unsigned char *)realloc(buffer, capacity);
    assert_non_null(next);
    buffer = next;
    written = capn_write_mem(&arena, buffer, capacity, 0);
    capacity *= 2u;
  }
  assert_true(written > 0);
  capn_free(&arena);

  assert_int_equal(nwchemc_set_params(buffer, (size_t)written), 0);
  assert_int_equal(g_set_config_calls, 1);

  struct capn render_arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(buffer, (size_t)written, &render_arena, &params_root),
      0);
  char full_blocks[NWCHEMC_BLOCKS];
  char embed_blocks[NWCHEMC_BLOCKS];
  assert_int_equal(nwchemc_params_render_input_blocks(
                       params_root, full_blocks, sizeof(full_blocks)),
                   0);
  assert_int_equal(nwchemc_params_render_embed_input_blocks(
                       params_root, embed_blocks, sizeof(embed_blocks)),
                   0);
  assert_non_null(strstr(full_blocks, "uhf"));
  assert_non_null(strstr(full_blocks, "nopen 2"));
  assert_non_null(strstr(full_blocks, "iterations 88"));
  assert_null(strstr(embed_blocks, "uhf"));
  assert_null(strstr(embed_blocks, "nopen 2"));
  assert_null(strstr(embed_blocks, "iterations 88"));
  nwchemc_params_release(&render_arena);

  int found_scftype = 0;
  for (int i = 0; i < g_set_string_count; ++i) {
    if (strcmp(g_set_keys[i], "scf:scftype") == 0) {
      assert_string_equal(g_set_values[i], "uhf");
      found_scftype = 1;
    }
  }
  assert_int_equal(found_scftype, 1);

  int found_nopen = 0;
  int found_iterations = 0;
  for (int i = 0; i < g_typed_set_count; ++i) {
    if (strcmp(g_typed_set_keys[i], "scf:nopen") == 0) {
      assert_int_equal(g_typed_set_types[i], NWCHEMC_DIRECT_SET_VALUE_INTEGER);
      assert_int_equal(g_typed_set_value_counts[i], 1);
      assert_string_equal(g_typed_set_values[i][0], "2");
      found_nopen = 1;
    } else if (strcmp(g_typed_set_keys[i], "dft:iterations") == 0) {
      assert_int_equal(g_typed_set_types[i], NWCHEMC_DIRECT_SET_VALUE_INTEGER);
      assert_int_equal(g_typed_set_value_counts[i], 1);
      assert_string_equal(g_typed_set_values[i][0], "88");
      found_iterations = 1;
    }
  }
  assert_int_equal(found_nopen, 1);
  assert_int_equal(found_iterations, 1);

  assert_null(strstr(g_input_blocks, "uhf"));
  assert_null(strstr(g_input_blocks, "nopen 2"));
  assert_null(strstr(g_input_blocks, "iterations 88"));

  free(buffer);
}

/* Logical semidirect sizes promote int2e:* without directives list entries. */
static void test_embed_promotes_typed_scf_semidirect_int2e(void **state) {
  (void)state;
  reset_embed_captures();

  struct capn arena;
  capn_init_malloc(&arena);
  capn_ptr root = capn_root(&arena);
  assert_int_not_equal(root.type, CAPN_NULL);
  NWChemParams_ptr params = new_NWChemParams(root.seg);

  struct NWChemScfSemidirect sd;
  memset(&sd, 0, sizeof(sd));
  sd.enabled = NWChemToggle_enabled;
  sd.filesize = 200;
  sd.memsize = 80;
  NWChemScfSemidirect_ptr sd_ptr = new_NWChemScfSemidirect(root.seg);
  write_NWChemScfSemidirect(&sd, sd_ptr);

  struct NWChemScfStanza scf;
  memset(&scf, 0, sizeof(scf));
  scf.semidirect = sd_ptr;

  struct NWChemInputStanza scf_stanza;
  memset(&scf_stanza, 0, sizeof(scf_stanza));
  scf_stanza.kind = NWChemInputStanza_Kind_scf;
  scf_stanza.scf = new_NWChemScfStanza(root.seg);
  write_NWChemScfStanza(&scf, scf_stanza.scf);

  struct NWChemParams view;
  memset(&view, 0, sizeof(view));
  view.basis = test_text_from_cstr("sto-3g");
  view.theory = test_text_from_cstr("scf");
  view.scfType = test_text_from_cstr("rhf");
  view.task = test_text_from_cstr("energy");
  view.multiplicity = 1;
  view.inputStanzas = new_NWChemInputStanza_list(root.seg, 1);
  set_NWChemInputStanza(&scf_stanza, view.inputStanzas, 0);
  write_NWChemParams(&view, params);
  assert_int_equal(capn_setp(root, 0, params.p), 0);

  size_t capacity = 4096u;
  unsigned char *buffer = NULL;
  int written = -1;
  for (int attempt = 0; attempt < 6 && written < 0; ++attempt) {
    unsigned char *next = (unsigned char *)realloc(buffer, capacity);
    assert_non_null(next);
    buffer = next;
    written = capn_write_mem(&arena, buffer, capacity, 0);
    capacity *= 2u;
  }
  assert_true(written > 0);
  capn_free(&arena);

  assert_int_equal(nwchemc_set_params(buffer, (size_t)written), 0);

  int found_fs = 0, found_ms = 0;
  for (int i = 0; i < g_typed_set_count; ++i) {
    if (strcmp(g_typed_set_keys[i], "int2e:filesize") == 0) {
      assert_string_equal(g_typed_set_values[i][0], "200");
      found_fs = 1;
    } else if (strcmp(g_typed_set_keys[i], "int2e:memsize") == 0) {
      assert_string_equal(g_typed_set_values[i][0], "80");
      found_ms = 1;
    }
  }
  assert_int_equal(found_fs, 1);
  assert_int_equal(found_ms, 1);
  assert_null(strstr(g_input_blocks, "semidirect"));
  assert_null(strstr(g_input_blocks, "filesize 200"));

  free(buffer);
}

static void test_configure_accepts_potential_config_nwchem(void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_configure != NULL);
  size_t message_size = 0;
  size_t config_size = 0;
  unsigned char *message = read_file(g_config_options_path, &message_size);
  assert_non_null(message);
  unsigned char *config = wrap_params_in_config(message, message_size,
                                                &config_size);
  assert_non_null(config);

  assert_int_equal(nwchemc_configure(config, config_size), 0);
  assert_int_equal(g_set_config_calls, 1);
  assert_string_equal(g_basis, "6-31g");
  assert_string_equal(g_theory, "scf");
  assert_string_equal(g_scf_type, "rhf");
  assert_int_equal(g_set_scf_direct_calls, 1);
  assert_int_equal(g_scf_has_options, 1);
  assert_int_equal(g_scf_maxiter, 50);
  assert_int_equal(g_set_driver_direct_calls, 1);
  assert_int_equal(g_driver_has_options, 1);
  assert_int_equal(g_driver_maxiter, 40);
  assert_true(g_set_rtdb_strings_calls >= 1);
  assert_true(g_set_rtdb_values_calls >= 1);
  assert_true(g_typed_set_count >= 3);
  assert_null(strstr(g_input_blocks, "task scf energy"));

  free(config);
  free(message);
}

static void test_engine_path_is_rejected_before_embed_config(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message =
      make_params_with_engine_path("/tmp/libnwchem_engine.so", &message_size);
  assert_non_null(message);

  assert_int_equal(nwchemc_set_params(message, message_size), -1);
  assert_int_equal(g_set_config_calls, 0);

  const double positions_ang[3] = {0.0, 0.0, 0.0};
  const int atomic_numbers[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult status = nwchemc_energy_gradient(
      1, positions_ang, atomic_numbers, message, message_size, grad);
  assert_int_equal(status.ok, 0);
  assert_int_equal(g_set_config_calls, 0);

  size_t config_size = 0;
  unsigned char *config = wrap_params_in_config(message, message_size,
                                                &config_size);
  assert_non_null(config);
  assert_int_equal(nwchemc_configure(config, config_size), -1);
  assert_int_equal(g_set_config_calls, 0);
  assert_null(nwchemc_session_create_from_config(config, config_size));
  assert_int_equal(g_set_config_calls, 0);

  free(config);
  free(message);
}

static void test_session_create_from_config_applies_nwchem(void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_session_create_from_config != NULL);
  size_t message_size = 0;
  size_t config_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  assert_non_null(message);
  unsigned char *config = wrap_params_in_config(message, message_size,
                                                &config_size);
  assert_non_null(config);

  NWChemCSession *session =
      nwchemc_session_create_from_config(config, config_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);
  assert_string_equal(g_basis, "sto-3g");
  assert_string_equal(g_theory, "dft");
  assert_int_equal(g_set_dft_direct_calls, 1);
  assert_int_equal(g_set_pseudopotential_calls, 1);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_session_energy_gradient(session, 1, pos, z, grad);
  assert_int_equal(result.ok, 1);
  assert_int_equal(g_energy_grad_calls, 1);
  assert_int_equal(g_set_config_calls, 1);

  nwchemc_session_destroy(session);
  free(config);
  free(message);
}

static void test_session_configure_replaces_before_topology(void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_session_configure != NULL);
  size_t message_size = 0;
  size_t replacement_size = 0;
  size_t config_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *replacement =
      read_file(g_config_options_path, &replacement_size);
  assert_non_null(message);
  assert_non_null(replacement);
  unsigned char *config = wrap_params_in_config(replacement, replacement_size,
                                                &config_size);
  assert_non_null(config);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);
  assert_string_equal(g_basis, "sto-3g");
  assert_string_equal(g_theory, "dft");
  assert_int_equal(g_set_dft_direct_calls, 1);

  assert_int_equal(nwchemc_session_configure(session, config, config_size), 0);
  assert_int_equal(g_set_config_calls, 2);
  assert_string_equal(g_basis, "6-31g");
  assert_string_equal(g_theory, "scf");
  assert_string_equal(g_scf_type, "rhf");
  assert_int_equal(g_set_scf_direct_calls, 2);
  assert_int_equal(g_scf_has_options, 1);
  assert_int_equal(g_scf_maxiter, 50);
  assert_int_equal(g_set_driver_direct_calls, 2);
  assert_int_equal(g_driver_has_options, 1);
  assert_int_equal(g_driver_maxiter, 40);

  nwchemc_session_destroy(session);
  free(config);
  free(replacement);
  free(message);
}

static void test_session_configure_rejects_after_topology(void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_session_configure != NULL);
  size_t message_size = 0;
  size_t replacement_size = 0;
  size_t config_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *replacement =
      read_file(g_config_options_path, &replacement_size);
  assert_non_null(message);
  assert_non_null(replacement);
  unsigned char *config = wrap_params_in_config(replacement, replacement_size,
                                                &config_size);
  assert_non_null(config);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult first =
      nwchemc_session_energy_gradient(session, 1, pos, z, grad);
  assert_int_equal(first.ok, 1);
  assert_int_equal(g_set_config_calls, 1);

  assert_int_not_equal(nwchemc_session_configure(session, config, config_size),
                       0);
  assert_int_equal(g_set_config_calls, 1);

  nwchemc_session_destroy(session);
  free(config);
  free(replacement);
  free(message);
}

static void test_embed_config_promotes_pspspin_rules(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message = read_file(g_pspspin_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_null(strstr(g_input_blocks, "pspspin"));
  assert_int_equal(g_set_rtdb_values_calls, 1);
  assert_int_equal(g_typed_set_count, 11);
  assert_typed_set_scalar("nwpw:pspspin", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_typed_set_scalar("nwpw:pspspin_count",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "2");
  assert_typed_set_scalar("nwpw:pspspin_iamup:_000001",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true");
  assert_typed_set_scalar("nwpw:pspspin_upscale:_000001",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "1.25");
  assert_typed_set_scalar("nwpw:pspspin_upl:_000001",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "1");
  const char *up_ions[] = {"1", "3"};
  assert_typed_set_values("nwpw:pspspin_upions:_000001",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, 2, up_ions);
  assert_typed_set_scalar("nwpw:pspspin_iamup:_000002",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "false");
  assert_typed_set_scalar("nwpw:pspspin_downscale:_000002",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "0.75");
  assert_typed_set_scalar("nwpw:pspspin_downl:_000002",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "2");
  assert_typed_set_scalar("nwpw:pspspin_downm:_000002",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "0");
  const char *down_ions[] = {"2"};
  assert_typed_set_values("nwpw:pspspin_downions:_000002",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, 1, down_ions);

  free(message);
}

static void test_embed_config_promotes_large_pspspin_ion_list(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message = read_file(g_pspspin_many_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_null(strstr(g_input_blocks, "pspspin"));
  assert_int_equal(g_set_rtdb_values_calls, 1);
  assert_int_equal(g_typed_set_count, 6);
  assert_typed_set_scalar("nwpw:pspspin", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_typed_set_scalar("nwpw:pspspin_count",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "1");
  assert_typed_set_scalar("nwpw:pspspin_iamup:_000001",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true");
  assert_typed_set_scalar("nwpw:pspspin_upscale:_000001",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "2.5");
  assert_typed_set_scalar("nwpw:pspspin_upl:_000001",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "3");
  const char *ions[] = {
      "1",  "2",  "3",  "4",  "5",  "6",  "7",
      "8",  "9",  "10", "11", "12", "13", "14",
      "15", "16", "17", "18", "19", "20"};
  assert_typed_set_values("nwpw:pspspin_upions:_000001",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, 20, ions);

  free(message);
}

static void test_embed_config_promotes_nwpw_spin_mode(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message = read_file(g_nwpw_spin_mode_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_null(strstr(g_input_blocks, "  dft\n"));
  assert_null(strstr(g_input_blocks, "  odft\n"));
  assert_int_equal(g_set_rtdb_values_calls, 1);
  assert_int_equal(g_typed_set_count, 6);
  assert_typed_set_scalar_entry("cgsd:ispin",
                                NWCHEMC_DIRECT_SET_VALUE_INTEGER, "1");
  assert_typed_set_scalar_entry("band:ispin",
                                NWCHEMC_DIRECT_SET_VALUE_INTEGER, "1");
  assert_typed_set_scalar_entry("cpsd:ispin",
                                NWCHEMC_DIRECT_SET_VALUE_INTEGER, "1");
  assert_typed_set_scalar_entry("cgsd:ispin",
                                NWCHEMC_DIRECT_SET_VALUE_INTEGER, "2");
  assert_typed_set_scalar_entry("band:ispin",
                                NWCHEMC_DIRECT_SET_VALUE_INTEGER, "2");
  assert_typed_set_scalar_entry("cpsd:ispin",
                                NWCHEMC_DIRECT_SET_VALUE_INTEGER, "2");

  free(message);
}

static void test_embed_config_promotes_nwpw_allow_translation(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message =
      read_file(g_nwpw_allow_translation_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_null(strstr(g_input_blocks, "  allow_translation\n"));
  assert_int_equal(g_set_rtdb_values_calls, 1);
  assert_int_equal(g_typed_set_count, 2);
  assert_typed_set_scalar("cgsd:allow_translation",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true");
  assert_typed_set_scalar("band:allow_translation",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "true");

  free(message);
}

static void test_embed_config_promotes_nwpw_cutoff_alias(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message = read_file(g_nwpw_cutoff_alias_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_null(strstr(g_input_blocks, "  cutoff 7.5\n"));
  assert_int_equal(g_set_nwpw_direct_calls, 1);
  assert_close(g_nwpw_energy_cutoff, 15.0, 1e-12);
  assert_close(g_nwpw_wavefunction_cutoff, 7.5, 1e-12);

  free(message);
}

static void test_embed_config_promotes_nwpw_mc_steps(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message = read_file(g_nwpw_mc_steps_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_null(strstr(g_input_blocks, "  mc_steps 13 17\n"));
  assert_int_equal(g_set_rtdb_values_calls, 1);
  assert_int_equal(g_typed_set_count, 1);
  assert_typed_set_pair("nwpw:bo_steps", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                        "13", "17");

  free(message);
}

static void test_embed_config_promotes_nwpw_bo_steps_default(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message =
      read_file(g_nwpw_bo_steps_default_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_null(strstr(g_input_blocks, "  bo_steps 12 100\n"));
  assert_int_equal(g_set_rtdb_values_calls, 1);
  assert_int_equal(g_typed_set_count, 1);
  assert_typed_set_pair("nwpw:bo_steps", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                        "12", "100");

  free(message);
}

static void
test_embed_config_promotes_nwpw_bo_time_step_default(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message =
      read_file(g_nwpw_bo_time_step_default_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_null(strstr(g_input_blocks, "  bo_time_step 15\n"));
  assert_int_equal(g_set_rtdb_values_calls, 1);
  assert_int_equal(g_typed_set_count, 1);
  assert_typed_set_scalar("nwpw:bo_time_step",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "15");

  free(message);
}

static void
test_embed_config_promotes_nwpw_bo_fake_mass_default(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message =
      read_file(g_nwpw_bo_fake_mass_default_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_null(strstr(g_input_blocks, "  bo_fake_mass 500\n"));
  assert_int_equal(g_set_rtdb_values_calls, 1);
  assert_int_equal(g_typed_set_count, 1);
  assert_typed_set_scalar("nwpw:bo_fake_mass",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "500");

  free(message);
}

static void test_embed_config_promotes_nwpw_scaling_default(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message =
      read_file(g_nwpw_scaling_default_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_null(strstr(g_input_blocks, "  scaling 1 1\n"));
  assert_int_equal(g_set_rtdb_values_calls, 1);
  assert_int_equal(g_typed_set_count, 2);
  assert_typed_set_pair("nwpw:scaling", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "1",
                        "1");
  assert_typed_set_pair("cpmd:scaling", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "1",
                        "1");

  free(message);
}

static void
test_embed_config_promotes_nwpw_np_dimensions_default(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message =
      read_file(g_nwpw_np_dimensions_default_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_null(strstr(g_input_blocks, "  np_dimensions -1 -1 -1\n"));
  assert_int_equal(g_set_rtdb_values_calls, 1);
  assert_int_equal(g_typed_set_count, 1);
  assert_typed_set_triple("nwpw:np_dimensions",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "-1", "-1",
                          "-1");

  free(message);
}

static void test_embed_config_promotes_nwpw_tolerances_default(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message =
      read_file(g_nwpw_tolerances_default_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_null(strstr(g_input_blocks, "  tolerances 1e-07 1e-07 0.0001\n"));
  assert_int_equal(g_set_rtdb_values_calls, 1);
  assert_int_equal(g_typed_set_count, 2);
  assert_typed_set_triple("cgsd:tolerances",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "9.9999999999999995e-08",
                          "9.9999999999999995e-08", "0.0001");
  assert_typed_set_triple("band:tolerances",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "9.9999999999999995e-08",
                          "9.9999999999999995e-08", "0.0001");

  free(message);
}

static void test_embed_config_promotes_nwpw_mc_steps_default(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message =
      read_file(g_nwpw_mc_steps_default_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_null(strstr(g_input_blocks, "  mc_steps 14 100\n"));
  assert_int_equal(g_set_rtdb_values_calls, 1);
  assert_int_equal(g_typed_set_count, 1);
  assert_typed_set_pair("nwpw:bo_steps", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                        "14", "100");

  free(message);
}

static void test_embed_config_promotes_brillouin_tetrahedron(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message =
      read_file(g_brillouin_tetrahedron_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_null(strstr(g_input_blocks, "  tetrahedron 4 5 6 tetraA\n"));
  assert_int_equal(g_set_rtdb_values_calls, 1);
  assert_int_equal(g_typed_set_count, 1);
  assert_typed_set_triple("band:tetrahedron-grid",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "4", "5", "6");
  assert_int_equal(g_set_brillouin_dos_zones_calls, 1);
  assert_int_equal(g_brillouin_dos_zone_count, 1);
  assert_brillouin_dos_zone(0, "tetraA", 4, 5, 6);

  free(message);
}

static void test_embed_config_promotes_brillouin_dos_grid(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message =
      read_file(g_brillouin_dos_grid_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_null(strstr(g_input_blocks, "  dos-grid 7 8 9 dosA\n"));
  assert_null(strstr(g_input_blocks, "  dos-fft-grid 10 11 12 fftA\n"));
  assert_int_equal(g_set_rtdb_values_calls, 1);
  assert_int_equal(g_typed_set_count, 1);
  assert_typed_set_triple("band:dos-grid", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "7", "8", "9");
  assert_int_equal(g_set_brillouin_dos_zones_calls, 1);
  assert_int_equal(g_brillouin_dos_zone_count, 2);
  assert_brillouin_dos_zone(0, "dosA", 7, 8, 9);
  assert_brillouin_dos_zone(1, "fftA", 10, 11, 12);

  free(message);
}

static void
test_embed_config_promotes_brillouin_dos_grid_default(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message =
      read_file(g_brillouin_dos_grid_default_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_null(strstr(g_input_blocks, "  dos-grid 7 2 2 structure_default\n"));
  assert_int_equal(g_set_rtdb_values_calls, 1);
  assert_int_equal(g_typed_set_count, 1);
  assert_typed_set_triple("band:dos-grid", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "7", "2", "2");
  assert_int_equal(g_set_brillouin_dos_zones_calls, 1);
  assert_int_equal(g_brillouin_dos_zone_count, 1);
  assert_brillouin_dos_zone(0, "structure_default", 7, 2, 2);

  free(message);
}

static void test_embed_config_promotes_nwpw_et(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message = read_file(g_nwpw_et_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_null(strstr(g_input_blocks, "  et movecs_a.dat"));
  assert_int_equal(g_set_rtdb_strings_calls, 1);
  assert_int_equal(g_set_string_count, 4);
  assert_set_string("pspw:et:movecs_a", "movecs_a.dat");
  assert_set_string("pspw:et:movecs_b", "movecs_b.dat");
  assert_set_string("pspw:et:ion_a", "ion_a.xyz");
  assert_set_string("pspw:et:ion_b", "ion_b.xyz");

  free(message);
}

static void test_embed_config_promotes_nwpw_temperature(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message =
      read_file(g_nwpw_temperature_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_null(strstr(g_input_blocks, "  temperature 400"));
  assert_int_equal(g_set_rtdb_values_calls, 1);
  assert_int_equal(g_typed_set_count, 16);
  assert_typed_set_scalar("cpmd:nose", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_typed_set_scalar("nwpw:nose", NWCHEMC_DIRECT_SET_VALUE_LOGICAL,
                          "true");
  assert_typed_set_scalar("cpmd:nose_restart",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "false");
  assert_typed_set_scalar("nwpw:nose_restart",
                          NWCHEMC_DIRECT_SET_VALUE_LOGICAL, "false");
  assert_typed_set_scalar("cpmd:Pe", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "900");
  assert_typed_set_scalar("nwpw:Pe", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "900");
  assert_typed_set_scalar("cpmd:Te", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "350");
  assert_typed_set_scalar("nwpw:Te", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "350");
  assert_typed_set_scalar("cpmd:Pr", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "1200");
  assert_typed_set_scalar("nwpw:Pr", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "1200");
  assert_typed_set_scalar("cpmd:Tr", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "400");
  assert_typed_set_scalar("nwpw:Tr", NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "400");
  assert_typed_set_scalar("cpmd:Mchain", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "5");
  assert_typed_set_scalar("nwpw:Mchain", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "5");
  assert_typed_set_scalar("cpmd:Nchain", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "4");
  assert_typed_set_scalar("nwpw:Nchain", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "4");

  free(message);
}

static void test_embed_config_promotes_nwpw_mapping_alias(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message =
      read_file(g_nwpw_mapping_alias_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_null(strstr(g_input_blocks, "  2d-hcurve\n"));
  assert_int_equal(g_set_rtdb_values_calls, 1);
  assert_int_equal(g_typed_set_count, 1);
  assert_typed_set_scalar("nwpw:mapping", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "3");

  free(message);
}

static void test_embed_config_promotes_nwpw_dos_default(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message = read_file(g_nwpw_dos_default_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_null(strstr(g_input_blocks, "  dos 0.00183745167502095\n"));
  assert_int_equal(g_set_rtdb_values_calls, 1);
  assert_int_equal(g_typed_set_count, 1);
  assert_typed_set_scalar("dos:alpha", NWCHEMC_DIRECT_SET_VALUE_DOUBLE,
                          "0.00183745167502095");

  free(message);
}

static void test_embed_config_promotes_nwpw_mapping_default(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message =
      read_file(g_nwpw_mapping_default_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_null(strstr(g_input_blocks, "  mapping 1\n"));
  assert_int_equal(g_set_rtdb_values_calls, 1);
  assert_int_equal(g_typed_set_count, 1);
  assert_typed_set_scalar("nwpw:mapping", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "1");

  free(message);
}

static void test_embed_config_promotes_nwpw_virtual_alias(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message =
      read_file(g_nwpw_virtual_alias_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_null(strstr(g_input_blocks, "  virtual 5 5\n"));
  assert_int_equal(g_set_rtdb_values_calls, 1);
  assert_int_equal(g_typed_set_count, 1);
  assert_typed_set_pair("nwpw:excited_ne",
                        NWCHEMC_DIRECT_SET_VALUE_INTEGER, "5", "5");

  free(message);
}

static void
test_embed_config_promotes_nwpw_one_electron_guess_defaults(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message =
      read_file(g_nwpw_one_electron_guess_defaults_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_null(strstr(g_input_blocks, "  one_electron_guess"));
  assert_int_equal(g_set_rtdb_values_calls, 1);
  assert_int_equal(g_typed_set_count, 3);
  assert_typed_set_scalar("nwpw:H1_it_in", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "50");
  assert_typed_set_scalar("nwpw:H1_it_out", NWCHEMC_DIRECT_SET_VALUE_INTEGER,
                          "1");
  assert_typed_set_scalar("nwpw:H1_it_ortho",
                          NWCHEMC_DIRECT_SET_VALUE_INTEGER, "1");

  free(message);
}

static void
test_embed_config_promotes_nwpw_fractional_orbitals_default(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message =
      read_file(g_nwpw_fractional_orbitals_default_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_null(strstr(g_input_blocks, "  fractional_orbitals"));
  assert_int_equal(g_set_rtdb_values_calls, 1);
  assert_int_equal(g_typed_set_count, 1);
  assert_typed_set_pair("nwpw:fractional_orbitals",
                        NWCHEMC_DIRECT_SET_VALUE_INTEGER, "6", "6");

  free(message);
}

static void
test_embed_config_promotes_nwpw_smear_orbitals_default(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message =
      read_file(g_nwpw_smear_orbitals_default_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_null(strstr(g_input_blocks, "  fractional_orbitals"));
  assert_null(strstr(g_input_blocks, "  smear"));
  assert_int_equal(g_set_rtdb_values_calls, 1);
  assert_int_equal(g_typed_set_count, 2);
  assert_typed_set_pair("nwpw:fractional_orbitals",
                        NWCHEMC_DIRECT_SET_VALUE_INTEGER, "9", "9");
  assert_typed_set_scalar("nwpw:fractional_temperature",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "0.03");

  free(message);
}

static void
test_embed_config_promotes_nwpw_virtual_orbitals_default(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message =
      read_file(g_nwpw_virtual_orbitals_default_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_null(strstr(g_input_blocks, "  virtual 8 8\n"));
  assert_int_equal(g_set_rtdb_values_calls, 1);
  assert_int_equal(g_typed_set_count, 1);
  assert_typed_set_pair("nwpw:excited_ne",
                        NWCHEMC_DIRECT_SET_VALUE_INTEGER, "8", "8");

  free(message);
}

static void
test_embed_config_promotes_nwpw_translate_vector_default(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message =
      read_file(g_nwpw_translate_vector_default_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_null(strstr(g_input_blocks, "  translate_vector 0.25"));
  assert_int_equal(g_set_rtdb_values_calls, 1);
  assert_int_equal(g_typed_set_count, 1);
  assert_typed_set_triple("nwpw:translate_vector",
                          NWCHEMC_DIRECT_SET_VALUE_DOUBLE, "0.25", "0.25",
                          "0.25");

  free(message);
}

static void
test_embed_config_promotes_brillouin_monkhorst_default(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message =
      read_file(g_brillouin_monkhorst_default_path, &message_size);
  assert_non_null(message);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);

  assert_int_equal(result.ok, 1);
  assert_null(strstr(g_input_blocks, "  monkhorst-pack 3 1 1 zone_default\n"));
  assert_int_equal(g_set_brillouin_zone_calls, 1);
  assert_int_equal(g_brillouin_has_options, 1);
  assert_string_equal(g_brillouin_zone_name, "zone_default");
  assert_int_equal(g_brillouin_monkhorst_pack[0], 3);
  assert_int_equal(g_brillouin_monkhorst_pack[1], 1);
  assert_int_equal(g_brillouin_monkhorst_pack[2], 1);
  assert_int_equal(g_brillouin_max_kpoints_print, 0);
  assert_int_equal(g_brillouin_kvector_count, 0);

  free(message);
}

static void test_session_reuses_config_across_geometry_steps(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  assert_non_null(message);

  NWChemCSession *session =
      nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_set_dft_direct_calls, 1);
  assert_int_equal(g_set_pseudopotential_calls, 1);

  double pos_a[3] = {0.0, 0.0, 0.0};
  double pos_b[3] = {0.0, 0.0, 0.1};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};

  NWChemCResult first =
      nwchemc_session_energy_gradient(session, 1, pos_a, z, grad);
  NWChemCResult second =
      nwchemc_session_energy_gradient(session, 1, pos_b, z, grad);

  assert_int_equal(first.ok, 1);
  assert_int_equal(second.ok, 1);
  assert_int_equal(g_energy_grad_calls, 2);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_set_dft_direct_calls, 1);

  int changed_z[1] = {8};
  NWChemCResult changed_species =
      nwchemc_session_energy_gradient(session, 1, pos_b, changed_z, grad);
  assert_int_equal(changed_species.ok, 0);
  assert_non_null(strstr(changed_species.message, "topology"));
  assert_int_equal(g_energy_grad_calls, 2);
  assert_int_equal(g_set_config_calls, 1);

  nwchemc_session_destroy(session);
  free(message);
}

static void test_session_reapplies_after_one_shot_config(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  assert_non_null(message);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};

  NWChemCResult first =
      nwchemc_session_energy_gradient(session, 1, pos, z, grad);
  assert_int_equal(first.ok, 1);
  assert_int_equal(g_set_config_calls, 1);

  NWChemCResult one_shot =
      nwchemc_energy_gradient(1, pos, z, message, message_size, grad);
  assert_int_equal(one_shot.ok, 1);
  assert_int_equal(g_set_config_calls, 2);

  NWChemCResult second =
      nwchemc_session_energy_gradient(session, 1, pos, z, grad);
  assert_int_equal(second.ok, 1);
  assert_int_equal(g_set_config_calls, 3);

  NWChemCResult third =
      nwchemc_session_energy_gradient(session, 1, pos, z, grad);
  assert_int_equal(third.ok, 1);
  assert_int_equal(g_set_config_calls, 3);

  nwchemc_session_destroy(session);
  free(message);
}

static void test_session_set_params_replaces_before_topology(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  size_t replacement_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *replacement =
      read_file(g_config_options_path, &replacement_size);
  assert_non_null(message);
  assert_non_null(replacement);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);
  assert_string_equal(g_basis, "sto-3g");
  assert_string_equal(g_theory, "dft");
  assert_int_equal(g_set_dft_direct_calls, 1);

  assert_int_equal(
      nwchemc_session_set_params(session, replacement, replacement_size), 0);
  assert_int_equal(g_set_config_calls, 2);
  assert_string_equal(g_basis, "6-31g");
  assert_string_equal(g_theory, "scf");
  assert_string_equal(g_scf_type, "rhf");
  assert_int_equal(g_set_scf_direct_calls, 2);
  assert_int_equal(g_scf_has_options, 1);
  assert_int_equal(g_scf_maxiter, 50);
  assert_int_equal(g_set_driver_direct_calls, 2);
  assert_int_equal(g_driver_has_options, 1);
  assert_int_equal(g_driver_maxiter, 40);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult result =
      nwchemc_session_energy_gradient(session, 1, pos, z, grad);
  assert_int_equal(result.ok, 1);
  assert_int_equal(g_set_config_calls, 2);
  assert_int_equal(g_energy_grad_calls, 1);
  assert_int_equal(g_call_charge[0], 0);
  assert_int_equal(g_call_multiplicity[0], 1);

  nwchemc_session_destroy(session);
  free(replacement);
  free(message);
}

static void test_session_rejects_param_replacement_after_topology(
    void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  size_t replacement_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *replacement =
      read_file(g_config_options_path, &replacement_size);
  assert_non_null(message);
  assert_non_null(replacement);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);

  double pos[3] = {0.0, 0.0, 0.0};
  int z[1] = {1};
  double grad[3] = {0.0, 0.0, 0.0};
  NWChemCResult first =
      nwchemc_session_energy_gradient(session, 1, pos, z, grad);
  assert_int_equal(first.ok, 1);
  assert_int_equal(g_set_config_calls, 1);

  assert_int_not_equal(
      nwchemc_session_set_params(session, replacement, replacement_size), 0);
  assert_int_equal(g_set_config_calls, 1);

  nwchemc_session_destroy(session);
  free(replacement);
  free(message);
}

static void test_session_calculate_forces_accepts_force_input_steps(
    void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  size_t step_a_size = 0;
  size_t step_b_size = 0;
  size_t step_changed_species_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  unsigned char *step_b = read_file(g_force_step_b_path, &step_b_size);
  unsigned char *step_changed_species = read_file(
      g_force_step_changed_species_path, &step_changed_species_size);
  assert_non_null(message);
  assert_non_null(step_a);
  assert_non_null(step_b);
  assert_non_null(step_changed_species);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);

  double forces[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult first = nwchemc_session_calculate_forces(
      session, step_a, step_a_size, forces, 6);
  assert_int_equal(first.ok, 1);
  assert_int_equal(g_energy_grad_calls, 1);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_call_n_atoms[0], 2);
  assert_int_equal(g_call_atomic_numbers[0][0], 1);
  assert_int_equal(g_call_atomic_numbers[0][1], 8);
  assert_close(g_call_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_call_has_cell[0], 1);
  assert_close(g_call_cell_ang[0][0], 10.0, 1.0e-12);
  assert_close(forces[0], -1.0, 1.0e-12);
  assert_close(forces[5], -6.0, 1.0e-12);

  NWChemCResult second = nwchemc_session_calculate_forces(
      session, step_b, step_b_size, forces, 6);
  assert_int_equal(second.ok, 1);
  assert_int_equal(g_energy_grad_calls, 2);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_call_n_atoms[1], 2);
  assert_int_equal(g_call_atomic_numbers[1][0], 1);
  assert_int_equal(g_call_atomic_numbers[1][1], 8);
  assert_close(g_call_positions_ang[1][5], 1.058354421806, 1.0e-12);
  assert_int_equal(g_call_has_cell[1], 1);
  assert_close(g_call_cell_ang[1][0], 10.58354421806, 1.0e-11);

  NWChemCResult changed_species = nwchemc_session_calculate_forces(
      session, step_changed_species, step_changed_species_size, forces, 6);
  assert_int_equal(changed_species.ok, 0);
  assert_non_null(strstr(changed_species.message, "topology"));
  assert_int_equal(g_energy_grad_calls, 2);

  NWChemCResult short_output = nwchemc_session_calculate_forces(
      session, step_b, step_b_size, forces, 5);
  assert_int_equal(short_output.ok, 0);
  assert_int_equal(g_energy_grad_calls, 2);

  nwchemc_session_destroy(session);
  free(step_changed_species);
  free(step_b);
  free(step_a);
  free(message);
}

static void test_session_reset_topology_allows_changed_species(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  size_t step_a_size = 0;
  size_t step_changed_species_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  unsigned char *step_changed_species = read_file(
      g_force_step_changed_species_path, &step_changed_species_size);
  assert_non_null(message);
  assert_non_null(step_a);
  assert_non_null(step_changed_species);
  assert_true(nwchemc_session_reset_topology != NULL);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);

  double forces[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult first = nwchemc_session_calculate_forces(
      session, step_a, step_a_size, forces, 6);
  assert_int_equal(first.ok, 1);
  assert_int_equal(g_energy_grad_calls, 1);

  NWChemCResult rejected = nwchemc_session_calculate_forces(
      session, step_changed_species, step_changed_species_size, forces, 6);
  assert_int_equal(rejected.ok, 0);
  assert_non_null(strstr(rejected.message, "topology"));
  assert_int_equal(g_energy_grad_calls, 1);

  assert_int_equal(nwchemc_session_reset_topology(session), 0);
  NWChemCResult accepted = nwchemc_session_calculate_forces(
      session, step_changed_species, step_changed_species_size, forces, 6);
  assert_int_equal(accepted.ok, 1);
  assert_int_equal(g_energy_grad_calls, 2);
  assert_int_equal(g_call_n_atoms[1], 2);
  assert_int_equal(g_call_atomic_numbers[1][0], 1);
  assert_int_equal(g_call_atomic_numbers[1][1], 1);

  nwchemc_session_destroy(session);
  free(step_changed_species);
  free(step_a);
  free(message);
}

static void test_session_calculate_hessian_accepts_force_input_step(
    void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  size_t step_a_size = 0;
  size_t step_changed_species_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  unsigned char *step_changed_species = read_file(
      g_force_step_changed_species_path, &step_changed_species_size);
  assert_non_null(message);
  assert_non_null(step_a);
  assert_non_null(step_changed_species);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);

  double hessian[36] = {0.0};
  NWChemCResult first = nwchemc_session_calculate_hessian(
      session, step_a, step_a_size, hessian, 36);
  assert_int_equal(first.ok, 1);
  assert_close(first.energy_h, -1.125, 1.0e-12);
  assert_int_equal(g_hessian_calls, 1);
  assert_int_equal(g_hessian_cell_calls, 1);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_hessian_n_atoms[0], 2);
  assert_int_equal(g_hessian_atomic_numbers[0][0], 1);
  assert_int_equal(g_hessian_atomic_numbers[0][1], 8);
  assert_close(g_hessian_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_hessian_has_cell[0], 1);
  assert_close(g_hessian_cell_ang[0][0], 10.0, 1.0e-12);
  assert_close(hessian[0], 10.0, 1.0e-12);
  assert_close(hessian[35], 45.0, 1.0e-12);

  NWChemCResult changed_species = nwchemc_session_calculate_hessian(
      session, step_changed_species, step_changed_species_size, hessian, 36);
  assert_int_equal(changed_species.ok, 0);
  assert_non_null(strstr(changed_species.message, "topology"));
  assert_int_equal(g_hessian_calls, 1);

  NWChemCResult short_output = nwchemc_session_calculate_hessian(
      session, step_a, step_a_size, hessian, 35);
  assert_int_equal(short_output.ok, 0);
  assert_int_equal(g_hessian_calls, 1);

  nwchemc_session_destroy(session);
  free(step_changed_species);
  free(step_a);
  free(message);
}

static void test_session_calculate_dipole_accepts_force_input_step(
    void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_session_calculate_dipole != NULL);
  size_t message_size = 0;
  size_t step_a_size = 0;
  size_t step_changed_species_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  unsigned char *step_changed_species = read_file(
      g_force_step_changed_species_path, &step_changed_species_size);
  assert_non_null(message);
  assert_non_null(step_a);
  assert_non_null(step_changed_species);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);

  double dipole[3] = {0.0, 0.0, 0.0};
  NWChemCResult first = nwchemc_session_calculate_dipole(
      session, step_a, step_a_size, dipole, 3);
  assert_int_equal(first.ok, 1);
  assert_close(first.energy_h, -1.25, 1.0e-12);
  assert_int_equal(g_dipole_calls, 1);
  assert_int_equal(g_dipole_cell_calls, 1);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_dipole_n_atoms[0], 2);
  assert_int_equal(g_dipole_atomic_numbers[0][0], 1);
  assert_int_equal(g_dipole_atomic_numbers[0][1], 8);
  assert_close(g_dipole_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_dipole_has_cell[0], 1);
  assert_close(g_dipole_cell_ang[0][0], 10.0, 1.0e-12);
  assert_close(dipole[0], 0.25, 1.0e-12);
  assert_close(dipole[1], 0.5, 1.0e-12);
  assert_close(dipole[2], 0.75, 1.0e-12);

  NWChemCResult changed_species = nwchemc_session_calculate_dipole(
      session, step_changed_species, step_changed_species_size, dipole, 3);
  assert_int_equal(changed_species.ok, 0);
  assert_non_null(strstr(changed_species.message, "topology"));
  assert_int_equal(g_dipole_calls, 1);

  NWChemCResult short_output = nwchemc_session_calculate_dipole(
      session, step_a, step_a_size, dipole, 2);
  assert_int_equal(short_output.ok, 0);
  assert_int_equal(g_dipole_calls, 1);

  nwchemc_session_destroy(session);
  free(step_changed_species);
  free(step_a);
  free(message);
}

static void test_session_calculate_quadrupole_accepts_force_input_step(
    void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_session_calculate_quadrupole != NULL);
  size_t message_size = 0;
  size_t step_a_size = 0;
  size_t step_changed_species_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  unsigned char *step_changed_species = read_file(
      g_force_step_changed_species_path, &step_changed_species_size);
  assert_non_null(message);
  assert_non_null(step_a);
  assert_non_null(step_changed_species);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);

  double quadrupole[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult first = nwchemc_session_calculate_quadrupole(
      session, step_a, step_a_size, quadrupole, 6);
  assert_int_equal(first.ok, 1);
  assert_close(first.energy_h, -1.5, 1.0e-12);
  assert_int_equal(g_quadrupole_calls, 1);
  assert_int_equal(g_quadrupole_cell_calls, 1);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_quadrupole_n_atoms[0], 2);
  assert_int_equal(g_quadrupole_atomic_numbers[0][0], 1);
  assert_int_equal(g_quadrupole_atomic_numbers[0][1], 8);
  assert_close(g_quadrupole_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_quadrupole_has_cell[0], 1);
  assert_close(g_quadrupole_cell_ang[0][0], 10.0, 1.0e-12);
  assert_close(quadrupole[0], 0.125, 1.0e-12);
  assert_close(quadrupole[5], 0.75, 1.0e-12);

  NWChemCResult changed_species = nwchemc_session_calculate_quadrupole(
      session, step_changed_species, step_changed_species_size, quadrupole, 6);
  assert_int_equal(changed_species.ok, 0);
  assert_non_null(strstr(changed_species.message, "topology"));
  assert_int_equal(g_quadrupole_calls, 1);

  NWChemCResult short_output = nwchemc_session_calculate_quadrupole(
      session, step_a, step_a_size, quadrupole, 5);
  assert_int_equal(short_output.ok, 0);
  assert_int_equal(g_quadrupole_calls, 1);

  nwchemc_session_destroy(session);
  free(step_changed_species);
  free(step_a);
  free(message);
}

static void test_session_calculate_stress_accepts_force_input_step(
    void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_session_calculate_stress != NULL);
  size_t message_size = 0;
  size_t step_a_size = 0;
  size_t step_changed_species_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  unsigned char *step_changed_species = read_file(
      g_force_step_changed_species_path, &step_changed_species_size);
  assert_non_null(message);
  assert_non_null(step_a);
  assert_non_null(step_changed_species);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);

  double stress[9] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult first = nwchemc_session_calculate_stress(
      session, step_a, step_a_size, stress, 9);
  assert_int_equal(first.ok, 1);
  assert_close(first.energy_h, -2.0, 1.0e-12);
  assert_int_equal(g_stress_calls, 1);
  assert_int_equal(g_stress_cell_calls, 1);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_stress_n_atoms[0], 2);
  assert_int_equal(g_stress_atomic_numbers[0][0], 1);
  assert_int_equal(g_stress_atomic_numbers[0][1], 8);
  assert_close(g_stress_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_stress_has_cell[0], 1);
  assert_close(g_stress_cell_ang[0][0], 10.0, 1.0e-12);
  assert_close(stress[0], 0.03125, 1.0e-12);
  assert_close(stress[8], 0.28125, 1.0e-12);

  NWChemCResult changed_species = nwchemc_session_calculate_stress(
      session, step_changed_species, step_changed_species_size, stress, 9);
  assert_int_equal(changed_species.ok, 0);
  assert_non_null(strstr(changed_species.message, "topology"));
  assert_int_equal(g_stress_calls, 1);

  NWChemCResult short_output = nwchemc_session_calculate_stress(
      session, step_a, step_a_size, stress, 8);
  assert_int_equal(short_output.ok, 0);
  assert_int_equal(g_stress_calls, 1);

  nwchemc_session_destroy(session);
  free(step_changed_species);
  free(step_a);
  free(message);
}

static void test_session_calculate_optimize_accepts_force_input_step(
    void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_session_calculate_optimize != NULL);
  size_t message_size = 0;
  size_t step_a_size = 0;
  size_t step_changed_species_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  unsigned char *step_changed_species = read_file(
      g_force_step_changed_species_path, &step_changed_species_size);
  assert_non_null(message);
  assert_non_null(step_a);
  assert_non_null(step_changed_species);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);

  double optimized_positions[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult first = nwchemc_session_calculate_optimize(
      session, step_a, step_a_size, optimized_positions, 6);
  assert_int_equal(first.ok, 1);
  assert_close(first.energy_h, -1.75, 1.0e-12);
  assert_int_equal(g_optimize_calls, 1);
  assert_int_equal(g_optimize_cell_calls, 1);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_optimize_n_atoms[0], 2);
  assert_int_equal(g_optimize_atomic_numbers[0][0], 1);
  assert_int_equal(g_optimize_atomic_numbers[0][1], 8);
  assert_close(g_optimize_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_optimize_has_cell[0], 1);
  assert_close(g_optimize_cell_ang[0][0], 10.0, 1.0e-12);
  assert_close(optimized_positions[0], 0.01, 1.0e-12);
  assert_close(optimized_positions[5], 0.8014, 1.0e-12);

  NWChemCResult changed_species = nwchemc_session_calculate_optimize(
      session, step_changed_species, step_changed_species_size,
      optimized_positions, 6);
  assert_int_equal(changed_species.ok, 0);
  assert_non_null(strstr(changed_species.message, "topology"));
  assert_int_equal(g_optimize_calls, 1);

  NWChemCResult short_output = nwchemc_session_calculate_optimize(
      session, step_a, step_a_size, optimized_positions, 5);
  assert_int_equal(short_output.ok, 0);
  assert_int_equal(g_optimize_calls, 1);

  nwchemc_session_destroy(session);
  free(step_changed_species);
  free(step_a);
  free(message);
}

static void test_session_calculate_frequencies_accepts_force_input_step(
    void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_session_calculate_frequencies != NULL);
  size_t message_size = 0;
  size_t step_a_size = 0;
  size_t step_changed_species_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  unsigned char *step_changed_species = read_file(
      g_force_step_changed_species_path, &step_changed_species_size);
  assert_non_null(message);
  assert_non_null(step_a);
  assert_non_null(step_changed_species);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);

  double frequencies[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  double intensities[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult first = nwchemc_session_calculate_frequencies(
      session, step_a, step_a_size, frequencies, 6, intensities, 6);
  assert_int_equal(first.ok, 1);
  assert_close(first.energy_h, -1.625, 1.0e-12);
  assert_int_equal(g_frequency_calls, 1);
  assert_int_equal(g_frequency_cell_calls, 1);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_frequency_n_atoms[0], 2);
  assert_int_equal(g_frequency_atomic_numbers[0][0], 1);
  assert_int_equal(g_frequency_atomic_numbers[0][1], 8);
  assert_close(g_frequency_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_frequency_has_cell[0], 1);
  assert_close(g_frequency_cell_ang[0][0], 10.0, 1.0e-12);
  assert_close(frequencies[0], 100.0, 1.0e-12);
  assert_close(frequencies[5], 105.0, 1.0e-12);
  assert_close(intensities[0], 0.01, 1.0e-12);
  assert_close(intensities[5], 0.06, 1.0e-12);

  NWChemCResult changed_species = nwchemc_session_calculate_frequencies(
      session, step_changed_species, step_changed_species_size, frequencies, 6,
      intensities, 6);
  assert_int_equal(changed_species.ok, 0);
  assert_non_null(strstr(changed_species.message, "topology"));
  assert_int_equal(g_frequency_calls, 1);

  NWChemCResult short_frequency_output = nwchemc_session_calculate_frequencies(
      session, step_a, step_a_size, frequencies, 5, intensities, 6);
  assert_int_equal(short_frequency_output.ok, 0);
  assert_int_equal(g_frequency_calls, 1);

  NWChemCResult short_intensity_output = nwchemc_session_calculate_frequencies(
      session, step_a, step_a_size, frequencies, 6, intensities, 5);
  assert_int_equal(short_intensity_output.ok, 0);
  assert_int_equal(g_frequency_calls, 1);

  nwchemc_session_destroy(session);
  free(step_changed_species);
  free(step_a);
  free(message);
}

static void test_session_calculate_frequency_detail_accepts_force_input_step(
    void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_session_calculate_frequencies_detail != NULL);
  size_t message_size = 0;
  size_t step_a_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  assert_non_null(message);
  assert_non_null(step_a);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);

  double frequencies[6] = {0.0};
  double intensities[6] = {0.0};
  double normal_modes[36] = {0.0};
  double projected_frequencies[6] = {0.0};
  double projected_intensities[6] = {0.0};
  double thermochemistry[5] = {0.0};
  NWChemCResult result = nwchemc_session_calculate_frequencies_detail(
      session, step_a, step_a_size, frequencies, 6, intensities, 6,
      normal_modes, 36, projected_frequencies, 6, projected_intensities, 6,
      thermochemistry, 5);
  assert_int_equal(result.ok, 1);
  assert_close(result.energy_h, -1.625, 1.0e-12);
  assert_int_equal(g_frequency_calls, 1);
  assert_int_equal(g_frequency_modes_calls, 1);
  assert_int_equal(g_frequency_detail_calls, 1);
  assert_close(frequencies[0], 100.0, 1.0e-12);
  assert_close(intensities[5], 0.06, 1.0e-12);
  assert_close(normal_modes[35], 0.036, 1.0e-12);
  assert_close(projected_frequencies[0], 90.0, 1.0e-12);
  assert_close(projected_frequencies[5], 95.0, 1.0e-12);
  assert_close(projected_intensities[5], 0.12, 1.0e-12);
  assert_close(thermochemistry[0], 0.001, 1.0e-12);
  assert_close(thermochemistry[4], 5.0, 1.0e-12);

  NWChemCResult short_modes = nwchemc_session_calculate_frequencies_detail(
      session, step_a, step_a_size, frequencies, 6, intensities, 6,
      normal_modes, 35, projected_frequencies, 6, projected_intensities, 6,
      thermochemistry, 5);
  assert_int_equal(short_modes.ok, 0);
  assert_int_equal(g_frequency_detail_calls, 1);

  NWChemCResult short_thermo = nwchemc_session_calculate_frequencies_detail(
      session, step_a, step_a_size, frequencies, 6, intensities, 6,
      normal_modes, 36, projected_frequencies, 6, projected_intensities, 6,
      thermochemistry, 4);
  assert_int_equal(short_thermo.ok, 0);
  assert_int_equal(g_frequency_detail_calls, 1);

  nwchemc_session_destroy(session);
  free(step_a);
  free(message);
}

static void test_direct_coordinate_energy_abi_calls_embed_wrappers(
    void **state) {
  (void)state;
  size_t message_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  assert_non_null(message);

  const double positions[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.7414};
  const int atomic_numbers[2] = {1, 8};

  reset_embed_captures();
  NWChemCResult energy_result =
      nwchemc_energy(2, positions, atomic_numbers, message, message_size);
  assert_int_equal(energy_result.ok, 1);
  assert_close(energy_result.energy_h, -1.0, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_energy_only_calls, 1);
  assert_int_equal(g_energy_only_cell_calls, 0);
  assert_int_equal(g_call_n_atoms[0], 2);
  assert_int_equal(g_call_charge[0], 0);
  assert_int_equal(g_call_multiplicity[0], 1);
  assert_int_equal(g_call_atomic_numbers[0][0], 1);
  assert_int_equal(g_call_atomic_numbers[0][1], 8);
  assert_close(g_call_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_call_has_cell[0], 0);

  reset_embed_captures();
  double grad[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult gradient_result = nwchemc_energy_gradient(
      2, positions, atomic_numbers, message, message_size, grad);
  assert_int_equal(gradient_result.ok, 1);
  assert_close(gradient_result.energy_h, -1.0, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_energy_grad_calls, 1);
  assert_int_equal(g_call_n_atoms[0], 2);
  assert_int_equal(g_call_charge[0], 0);
  assert_int_equal(g_call_multiplicity[0], 1);
  assert_int_equal(g_call_atomic_numbers[0][0], 1);
  assert_int_equal(g_call_atomic_numbers[0][1], 8);
  assert_close(g_call_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_call_has_cell[0], 0);
  assert_close(grad[0], 1.0, 1.0e-12);
  assert_close(grad[5], 6.0, 1.0e-12);

  reset_embed_captures();
  double forces[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult force_result = nwchemc_energy_forces(
      2, positions, atomic_numbers, message, message_size, forces);
  assert_int_equal(force_result.ok, 1);
  assert_close(force_result.energy_h, -1.0, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_energy_grad_calls, 1);
  assert_int_equal(g_call_n_atoms[0], 2);
  assert_int_equal(g_call_charge[0], 0);
  assert_int_equal(g_call_multiplicity[0], 1);
  assert_int_equal(g_call_atomic_numbers[0][0], 1);
  assert_int_equal(g_call_atomic_numbers[0][1], 8);
  assert_close(g_call_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_call_has_cell[0], 0);
  assert_close(forces[0], -1.0, 1.0e-12);
  assert_close(forces[5], -6.0, 1.0e-12);

  free(message);
}

static void test_direct_coordinate_config_energy_abi_calls_embed_wrappers(
    void **state) {
  (void)state;
  assert_true(nwchemc_energy_from_config != NULL);
  assert_true(nwchemc_energy_gradient_from_config != NULL);
  assert_true(nwchemc_energy_forces_from_config != NULL);
  size_t message_size = 0;
  size_t config_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  assert_non_null(message);
  unsigned char *config = wrap_params_in_config(message, message_size,
                                                &config_size);
  assert_non_null(config);

  const double positions[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.7414};
  const int atomic_numbers[2] = {1, 8};

  reset_embed_captures();
  NWChemCResult energy_result =
      nwchemc_energy_from_config(2, positions, atomic_numbers, config,
                                 config_size);
  assert_int_equal(energy_result.ok, 1);
  assert_close(energy_result.energy_h, -1.0, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_energy_only_calls, 1);
  assert_int_equal(g_energy_only_cell_calls, 1);
  assert_int_equal(g_call_n_atoms[0], 2);
  assert_int_equal(g_call_charge[0], 0);
  assert_int_equal(g_call_multiplicity[0], 1);
  assert_int_equal(g_call_atomic_numbers[0][0], 1);
  assert_int_equal(g_call_atomic_numbers[0][1], 8);
  assert_close(g_call_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_call_has_cell[0], 0);

  reset_embed_captures();
  double grad[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult gradient_result = nwchemc_energy_gradient_from_config(
      2, positions, atomic_numbers, config, config_size, grad);
  assert_int_equal(gradient_result.ok, 1);
  assert_close(gradient_result.energy_h, -1.0, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_energy_grad_calls, 1);
  assert_int_equal(g_call_n_atoms[0], 2);
  assert_int_equal(g_call_charge[0], 0);
  assert_int_equal(g_call_multiplicity[0], 1);
  assert_int_equal(g_call_atomic_numbers[0][0], 1);
  assert_int_equal(g_call_atomic_numbers[0][1], 8);
  assert_close(g_call_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_call_has_cell[0], 0);
  assert_close(grad[0], 1.0, 1.0e-12);
  assert_close(grad[5], 6.0, 1.0e-12);

  reset_embed_captures();
  double forces[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult force_result = nwchemc_energy_forces_from_config(
      2, positions, atomic_numbers, config, config_size, forces);
  assert_int_equal(force_result.ok, 1);
  assert_close(force_result.energy_h, -1.0, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_energy_grad_calls, 1);
  assert_int_equal(g_call_n_atoms[0], 2);
  assert_int_equal(g_call_charge[0], 0);
  assert_int_equal(g_call_multiplicity[0], 1);
  assert_int_equal(g_call_atomic_numbers[0][0], 1);
  assert_int_equal(g_call_atomic_numbers[0][1], 8);
  assert_close(g_call_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_call_has_cell[0], 0);
  assert_close(forces[0], -1.0, 1.0e-12);
  assert_close(forces[5], -6.0, 1.0e-12);

  free(config);
  free(message);
}

static void test_session_coordinate_energy_abi_calls_embed_wrappers(
    void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  assert_non_null(message);

  const double positions[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.7414};
  const int atomic_numbers[2] = {1, 8};
  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);

  NWChemCResult energy_result =
      nwchemc_session_energy(session, 2, positions, atomic_numbers);
  assert_int_equal(energy_result.ok, 1);
  assert_close(energy_result.energy_h, -1.0, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_energy_only_calls, 1);
  assert_int_equal(g_energy_only_cell_calls, 1);
  assert_int_equal(g_call_n_atoms[0], 2);
  assert_int_equal(g_call_charge[0], 0);
  assert_int_equal(g_call_multiplicity[0], 1);
  assert_int_equal(g_call_atomic_numbers[0][0], 1);
  assert_int_equal(g_call_atomic_numbers[0][1], 8);
  assert_close(g_call_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_call_has_cell[0], 0);

  double grad[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult gradient_result = nwchemc_session_energy_gradient(
      session, 2, positions, atomic_numbers, grad);
  assert_int_equal(gradient_result.ok, 1);
  assert_close(gradient_result.energy_h, -1.0, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_energy_grad_calls, 1);
  assert_int_equal(g_call_n_atoms[0], 2);
  assert_int_equal(g_call_charge[0], 0);
  assert_int_equal(g_call_multiplicity[0], 1);
  assert_int_equal(g_call_atomic_numbers[0][0], 1);
  assert_int_equal(g_call_atomic_numbers[0][1], 8);
  assert_close(g_call_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_call_has_cell[0], 0);
  assert_close(grad[0], 1.0, 1.0e-12);
  assert_close(grad[5], 6.0, 1.0e-12);

  double forces[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult force_result = nwchemc_session_energy_forces(
      session, 2, positions, atomic_numbers, forces);
  assert_int_equal(force_result.ok, 1);
  assert_close(force_result.energy_h, -1.0, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_energy_grad_calls, 2);
  assert_int_equal(g_call_n_atoms[1], 2);
  assert_int_equal(g_call_charge[1], 0);
  assert_int_equal(g_call_multiplicity[1], 1);
  assert_int_equal(g_call_atomic_numbers[1][0], 1);
  assert_int_equal(g_call_atomic_numbers[1][1], 8);
  assert_close(g_call_positions_ang[1][5], 0.7414, 1.0e-12);
  assert_int_equal(g_call_has_cell[1], 0);
  assert_close(forces[0], -1.0, 1.0e-12);
  assert_close(forces[5], -6.0, 1.0e-12);

  int changed_atomic_numbers[2] = {1, 1};
  NWChemCResult changed_topology =
      nwchemc_session_energy(session, 2, positions, changed_atomic_numbers);
  assert_int_equal(changed_topology.ok, 0);
  assert_non_null(strstr(changed_topology.message, "topology"));
  assert_int_equal(g_energy_only_calls, 1);

  nwchemc_session_destroy(session);
  free(message);
}

static void test_direct_coordinate_abi_calls_embed_wrappers(void **state) {
  (void)state;
  size_t message_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  assert_non_null(message);

  const double positions[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.7414};
  const int atomic_numbers[2] = {1, 8};

  reset_embed_captures();
  double hessian[36] = {0.0};
  NWChemCResult hessian_result =
      nwchemc_hessian(2, positions, atomic_numbers, message, message_size,
                      hessian);
  assert_int_equal(hessian_result.ok, 1);
  assert_close(hessian_result.energy_h, -1.125, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_hessian_calls, 1);
  assert_int_equal(g_hessian_cell_calls, 0);
  assert_int_equal(g_hessian_n_atoms[0], 2);
  assert_int_equal(g_hessian_charge[0], 0);
  assert_int_equal(g_hessian_multiplicity[0], 1);
  assert_int_equal(g_hessian_atomic_numbers[0][0], 1);
  assert_int_equal(g_hessian_atomic_numbers[0][1], 8);
  assert_close(g_hessian_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_hessian_has_cell[0], 0);
  assert_close(hessian[0], 10.0, 1.0e-12);
  assert_close(hessian[35], 45.0, 1.0e-12);

  reset_embed_captures();
  double optimized_positions[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult optimize_result =
      nwchemc_optimize(2, positions, atomic_numbers, message, message_size,
                       optimized_positions);
  assert_int_equal(optimize_result.ok, 1);
  assert_close(optimize_result.energy_h, -1.75, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_optimize_calls, 1);
  assert_int_equal(g_optimize_cell_calls, 0);
  assert_int_equal(g_optimize_n_atoms[0], 2);
  assert_int_equal(g_optimize_charge[0], 0);
  assert_int_equal(g_optimize_multiplicity[0], 1);
  assert_int_equal(g_optimize_atomic_numbers[0][0], 1);
  assert_int_equal(g_optimize_atomic_numbers[0][1], 8);
  assert_close(g_optimize_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_optimize_has_cell[0], 0);
  assert_close(optimized_positions[0], 0.01, 1.0e-12);
  assert_close(optimized_positions[5], 0.8014, 1.0e-12);

  reset_embed_captures();
  double frequencies[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  double intensities[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult frequencies_result =
      nwchemc_frequencies(2, positions, atomic_numbers, message, message_size,
                          frequencies, intensities);
  assert_int_equal(frequencies_result.ok, 1);
  assert_close(frequencies_result.energy_h, -1.625, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_frequency_calls, 1);
  assert_int_equal(g_frequency_cell_calls, 0);
  assert_int_equal(g_frequency_n_atoms[0], 2);
  assert_int_equal(g_frequency_charge[0], 0);
  assert_int_equal(g_frequency_multiplicity[0], 1);
  assert_int_equal(g_frequency_atomic_numbers[0][0], 1);
  assert_int_equal(g_frequency_atomic_numbers[0][1], 8);
  assert_close(g_frequency_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_frequency_has_cell[0], 0);
  assert_close(frequencies[0], 100.0, 1.0e-12);
  assert_close(frequencies[5], 105.0, 1.0e-12);
  assert_close(intensities[0], 0.01, 1.0e-12);
  assert_close(intensities[5], 0.06, 1.0e-12);

  reset_embed_captures();
  memset(frequencies, 0, sizeof(frequencies));
  NWChemCResult frequencies_without_intensities =
      nwchemc_frequencies(2, positions, atomic_numbers, message, message_size,
                          frequencies, NULL);
  assert_int_equal(frequencies_without_intensities.ok, 1);
  assert_close(frequencies_without_intensities.energy_h, -1.625, 1.0e-12);
  assert_int_equal(g_frequency_calls, 1);
  assert_int_equal(g_frequency_cell_calls, 0);
  assert_close(frequencies[0], 100.0, 1.0e-12);
  assert_close(frequencies[5], 105.0, 1.0e-12);

  free(message);
}

static void test_direct_coordinate_config_abi_calls_embed_wrappers(
    void **state) {
  (void)state;
  assert_true(nwchemc_hessian_from_config != NULL);
  assert_true(nwchemc_optimize_from_config != NULL);
  assert_true(nwchemc_frequencies_from_config != NULL);
  size_t message_size = 0;
  size_t config_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  assert_non_null(message);
  unsigned char *config = wrap_params_in_config(message, message_size,
                                                &config_size);
  assert_non_null(config);

  const double positions[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.7414};
  const int atomic_numbers[2] = {1, 8};

  reset_embed_captures();
  double hessian[36] = {0.0};
  NWChemCResult hessian_result = nwchemc_hessian_from_config(
      2, positions, atomic_numbers, config, config_size, hessian);
  assert_int_equal(hessian_result.ok, 1);
  assert_close(hessian_result.energy_h, -1.125, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_hessian_calls, 1);
  assert_int_equal(g_hessian_cell_calls, 1);
  assert_int_equal(g_hessian_n_atoms[0], 2);
  assert_int_equal(g_hessian_charge[0], 0);
  assert_int_equal(g_hessian_multiplicity[0], 1);
  assert_int_equal(g_hessian_atomic_numbers[0][0], 1);
  assert_int_equal(g_hessian_atomic_numbers[0][1], 8);
  assert_close(g_hessian_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_hessian_has_cell[0], 0);
  assert_close(hessian[0], 10.0, 1.0e-12);
  assert_close(hessian[35], 45.0, 1.0e-12);

  reset_embed_captures();
  double optimized_positions[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult optimize_result = nwchemc_optimize_from_config(
      2, positions, atomic_numbers, config, config_size, optimized_positions);
  assert_int_equal(optimize_result.ok, 1);
  assert_close(optimize_result.energy_h, -1.75, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_optimize_calls, 1);
  assert_int_equal(g_optimize_cell_calls, 1);
  assert_int_equal(g_optimize_n_atoms[0], 2);
  assert_int_equal(g_optimize_charge[0], 0);
  assert_int_equal(g_optimize_multiplicity[0], 1);
  assert_int_equal(g_optimize_atomic_numbers[0][0], 1);
  assert_int_equal(g_optimize_atomic_numbers[0][1], 8);
  assert_close(g_optimize_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_optimize_has_cell[0], 0);
  assert_close(optimized_positions[0], 0.01, 1.0e-12);
  assert_close(optimized_positions[5], 0.8014, 1.0e-12);

  reset_embed_captures();
  double frequencies[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  double intensities[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult frequencies_result = nwchemc_frequencies_from_config(
      2, positions, atomic_numbers, config, config_size, frequencies,
      intensities);
  assert_int_equal(frequencies_result.ok, 1);
  assert_close(frequencies_result.energy_h, -1.625, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_frequency_calls, 1);
  assert_int_equal(g_frequency_cell_calls, 1);
  assert_int_equal(g_frequency_n_atoms[0], 2);
  assert_int_equal(g_frequency_charge[0], 0);
  assert_int_equal(g_frequency_multiplicity[0], 1);
  assert_int_equal(g_frequency_atomic_numbers[0][0], 1);
  assert_int_equal(g_frequency_atomic_numbers[0][1], 8);
  assert_close(g_frequency_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_frequency_has_cell[0], 0);
  assert_close(frequencies[0], 100.0, 1.0e-12);
  assert_close(frequencies[5], 105.0, 1.0e-12);
  assert_close(intensities[0], 0.01, 1.0e-12);
  assert_close(intensities[5], 0.06, 1.0e-12);

  reset_embed_captures();
  memset(frequencies, 0, sizeof(frequencies));
  NWChemCResult frequencies_without_intensities =
      nwchemc_frequencies_from_config(2, positions, atomic_numbers, config,
                                      config_size, frequencies, NULL);
  assert_int_equal(frequencies_without_intensities.ok, 1);
  assert_close(frequencies_without_intensities.energy_h, -1.625, 1.0e-12);
  assert_int_equal(g_frequency_calls, 1);
  assert_int_equal(g_frequency_cell_calls, 1);
  assert_close(frequencies[0], 100.0, 1.0e-12);
  assert_close(frequencies[5], 105.0, 1.0e-12);

  free(config);
  free(message);
}

static void test_session_coordinate_abi_calls_embed_wrappers(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  assert_non_null(message);

  const double positions[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.7414};
  const int atomic_numbers[2] = {1, 8};
  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);

  double hessian[36] = {0.0};
  NWChemCResult hessian_result =
      nwchemc_session_hessian(session, 2, positions, atomic_numbers, hessian);
  assert_int_equal(hessian_result.ok, 1);
  assert_close(hessian_result.energy_h, -1.125, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_hessian_calls, 1);
  assert_int_equal(g_hessian_cell_calls, 1);
  assert_int_equal(g_hessian_n_atoms[0], 2);
  assert_int_equal(g_hessian_charge[0], 0);
  assert_int_equal(g_hessian_multiplicity[0], 1);
  assert_int_equal(g_hessian_atomic_numbers[0][0], 1);
  assert_int_equal(g_hessian_atomic_numbers[0][1], 8);
  assert_close(g_hessian_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_hessian_has_cell[0], 0);
  assert_close(hessian[0], 10.0, 1.0e-12);
  assert_close(hessian[35], 45.0, 1.0e-12);

  double optimized_positions[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult optimize_result = nwchemc_session_optimize(
      session, 2, positions, atomic_numbers, optimized_positions);
  assert_int_equal(optimize_result.ok, 1);
  assert_close(optimize_result.energy_h, -1.75, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_optimize_calls, 1);
  assert_int_equal(g_optimize_cell_calls, 1);
  assert_int_equal(g_optimize_n_atoms[0], 2);
  assert_int_equal(g_optimize_charge[0], 0);
  assert_int_equal(g_optimize_multiplicity[0], 1);
  assert_int_equal(g_optimize_atomic_numbers[0][0], 1);
  assert_int_equal(g_optimize_atomic_numbers[0][1], 8);
  assert_close(g_optimize_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_optimize_has_cell[0], 0);
  assert_close(optimized_positions[0], 0.01, 1.0e-12);
  assert_close(optimized_positions[5], 0.8014, 1.0e-12);

  double frequencies[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  double intensities[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult frequencies_result = nwchemc_session_frequencies(
      session, 2, positions, atomic_numbers, frequencies, intensities);
  assert_int_equal(frequencies_result.ok, 1);
  assert_close(frequencies_result.energy_h, -1.625, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_frequency_calls, 1);
  assert_int_equal(g_frequency_cell_calls, 1);
  assert_int_equal(g_frequency_n_atoms[0], 2);
  assert_int_equal(g_frequency_charge[0], 0);
  assert_int_equal(g_frequency_multiplicity[0], 1);
  assert_int_equal(g_frequency_atomic_numbers[0][0], 1);
  assert_int_equal(g_frequency_atomic_numbers[0][1], 8);
  assert_close(g_frequency_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_frequency_has_cell[0], 0);
  assert_close(frequencies[0], 100.0, 1.0e-12);
  assert_close(frequencies[5], 105.0, 1.0e-12);
  assert_close(intensities[0], 0.01, 1.0e-12);
  assert_close(intensities[5], 0.06, 1.0e-12);

  int changed_atomic_numbers[2] = {1, 1};
  NWChemCResult changed_topology = nwchemc_session_hessian(
      session, 2, positions, changed_atomic_numbers, hessian);
  assert_int_equal(changed_topology.ok, 0);
  assert_non_null(strstr(changed_topology.message, "topology"));
  assert_int_equal(g_hessian_calls, 1);

  nwchemc_session_destroy(session);
  free(message);
}

static void test_direct_coordinate_property_stress_abi_calls_embed_wrappers(
    void **state) {
  (void)state;
  size_t message_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  assert_non_null(message);

  const double positions[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.7414};
  const int atomic_numbers[2] = {1, 8};

  reset_embed_captures();
  double dipole[3] = {0.0, 0.0, 0.0};
  NWChemCResult dipole_result =
      nwchemc_dipole(2, positions, atomic_numbers, message, message_size,
                     dipole);
  assert_int_equal(dipole_result.ok, 1);
  assert_close(dipole_result.energy_h, -1.25, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_dipole_calls, 1);
  assert_int_equal(g_dipole_cell_calls, 0);
  assert_int_equal(g_dipole_n_atoms[0], 2);
  assert_int_equal(g_dipole_charge[0], 0);
  assert_int_equal(g_dipole_multiplicity[0], 1);
  assert_int_equal(g_dipole_atomic_numbers[0][0], 1);
  assert_int_equal(g_dipole_atomic_numbers[0][1], 8);
  assert_close(g_dipole_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_dipole_has_cell[0], 0);
  assert_close(dipole[0], 0.25, 1.0e-12);
  assert_close(dipole[2], 0.75, 1.0e-12);

  reset_embed_captures();
  double quadrupole[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult quadrupole_result =
      nwchemc_quadrupole(2, positions, atomic_numbers, message, message_size,
                         quadrupole);
  assert_int_equal(quadrupole_result.ok, 1);
  assert_close(quadrupole_result.energy_h, -1.5, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_quadrupole_calls, 1);
  assert_int_equal(g_quadrupole_cell_calls, 0);
  assert_int_equal(g_quadrupole_n_atoms[0], 2);
  assert_int_equal(g_quadrupole_charge[0], 0);
  assert_int_equal(g_quadrupole_multiplicity[0], 1);
  assert_int_equal(g_quadrupole_atomic_numbers[0][0], 1);
  assert_int_equal(g_quadrupole_atomic_numbers[0][1], 8);
  assert_close(g_quadrupole_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_quadrupole_has_cell[0], 0);
  assert_close(quadrupole[0], 0.125, 1.0e-12);
  assert_close(quadrupole[5], 0.75, 1.0e-12);

  reset_embed_captures();
  double stress[9] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult stress_result =
      nwchemc_stress(2, positions, atomic_numbers, message, message_size,
                     stress);
  assert_int_equal(stress_result.ok, 1);
  assert_close(stress_result.energy_h, -2.0, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_stress_calls, 1);
  assert_int_equal(g_stress_cell_calls, 1);
  assert_int_equal(g_stress_n_atoms[0], 2);
  assert_int_equal(g_stress_charge[0], 0);
  assert_int_equal(g_stress_multiplicity[0], 1);
  assert_int_equal(g_stress_atomic_numbers[0][0], 1);
  assert_int_equal(g_stress_atomic_numbers[0][1], 8);
  assert_close(g_stress_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_stress_has_cell[0], 0);
  assert_close(stress[0], 0.03125, 1.0e-12);
  assert_close(stress[8], 0.28125, 1.0e-12);

  free(message);
}

static void
test_direct_coordinate_config_property_stress_abi_calls_embed_wrappers(
    void **state) {
  (void)state;
  assert_true(nwchemc_dipole_from_config != NULL);
  assert_true(nwchemc_quadrupole_from_config != NULL);
  assert_true(nwchemc_stress_from_config != NULL);
  size_t message_size = 0;
  size_t config_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  assert_non_null(message);
  unsigned char *config = wrap_params_in_config(message, message_size,
                                                &config_size);
  assert_non_null(config);

  const double positions[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.7414};
  const int atomic_numbers[2] = {1, 8};

  reset_embed_captures();
  double dipole[3] = {0.0, 0.0, 0.0};
  NWChemCResult dipole_result = nwchemc_dipole_from_config(
      2, positions, atomic_numbers, config, config_size, dipole);
  assert_int_equal(dipole_result.ok, 1);
  assert_close(dipole_result.energy_h, -1.25, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_dipole_calls, 1);
  assert_int_equal(g_dipole_cell_calls, 1);
  assert_int_equal(g_dipole_n_atoms[0], 2);
  assert_int_equal(g_dipole_charge[0], 0);
  assert_int_equal(g_dipole_multiplicity[0], 1);
  assert_int_equal(g_dipole_atomic_numbers[0][0], 1);
  assert_int_equal(g_dipole_atomic_numbers[0][1], 8);
  assert_close(g_dipole_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_dipole_has_cell[0], 0);
  assert_close(dipole[0], 0.25, 1.0e-12);
  assert_close(dipole[2], 0.75, 1.0e-12);

  reset_embed_captures();
  double quadrupole[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult quadrupole_result = nwchemc_quadrupole_from_config(
      2, positions, atomic_numbers, config, config_size, quadrupole);
  assert_int_equal(quadrupole_result.ok, 1);
  assert_close(quadrupole_result.energy_h, -1.5, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_quadrupole_calls, 1);
  assert_int_equal(g_quadrupole_cell_calls, 1);
  assert_int_equal(g_quadrupole_n_atoms[0], 2);
  assert_int_equal(g_quadrupole_charge[0], 0);
  assert_int_equal(g_quadrupole_multiplicity[0], 1);
  assert_int_equal(g_quadrupole_atomic_numbers[0][0], 1);
  assert_int_equal(g_quadrupole_atomic_numbers[0][1], 8);
  assert_close(g_quadrupole_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_quadrupole_has_cell[0], 0);
  assert_close(quadrupole[0], 0.125, 1.0e-12);
  assert_close(quadrupole[5], 0.75, 1.0e-12);

  reset_embed_captures();
  double stress[9] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult stress_result = nwchemc_stress_from_config(
      2, positions, atomic_numbers, config, config_size, stress);
  assert_int_equal(stress_result.ok, 1);
  assert_close(stress_result.energy_h, -2.0, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_stress_calls, 1);
  assert_int_equal(g_stress_cell_calls, 1);
  assert_int_equal(g_stress_n_atoms[0], 2);
  assert_int_equal(g_stress_charge[0], 0);
  assert_int_equal(g_stress_multiplicity[0], 1);
  assert_int_equal(g_stress_atomic_numbers[0][0], 1);
  assert_int_equal(g_stress_atomic_numbers[0][1], 8);
  assert_close(g_stress_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_stress_has_cell[0], 0);
  assert_close(stress[0], 0.03125, 1.0e-12);
  assert_close(stress[8], 0.28125, 1.0e-12);

  free(config);
  free(message);
}

static void test_session_coordinate_property_stress_abi_calls_embed_wrappers(
    void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  assert_non_null(message);

  const double positions[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.7414};
  const int atomic_numbers[2] = {1, 8};
  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);

  double dipole[3] = {0.0, 0.0, 0.0};
  NWChemCResult dipole_result =
      nwchemc_session_dipole(session, 2, positions, atomic_numbers, dipole);
  assert_int_equal(dipole_result.ok, 1);
  assert_close(dipole_result.energy_h, -1.25, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_dipole_calls, 1);
  assert_int_equal(g_dipole_cell_calls, 1);
  assert_int_equal(g_dipole_n_atoms[0], 2);
  assert_int_equal(g_dipole_charge[0], 0);
  assert_int_equal(g_dipole_multiplicity[0], 1);
  assert_int_equal(g_dipole_atomic_numbers[0][0], 1);
  assert_int_equal(g_dipole_atomic_numbers[0][1], 8);
  assert_close(g_dipole_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_dipole_has_cell[0], 0);
  assert_close(dipole[0], 0.25, 1.0e-12);
  assert_close(dipole[2], 0.75, 1.0e-12);

  double quadrupole[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult quadrupole_result = nwchemc_session_quadrupole(
      session, 2, positions, atomic_numbers, quadrupole);
  assert_int_equal(quadrupole_result.ok, 1);
  assert_close(quadrupole_result.energy_h, -1.5, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_quadrupole_calls, 1);
  assert_int_equal(g_quadrupole_cell_calls, 1);
  assert_int_equal(g_quadrupole_n_atoms[0], 2);
  assert_int_equal(g_quadrupole_charge[0], 0);
  assert_int_equal(g_quadrupole_multiplicity[0], 1);
  assert_int_equal(g_quadrupole_atomic_numbers[0][0], 1);
  assert_int_equal(g_quadrupole_atomic_numbers[0][1], 8);
  assert_close(g_quadrupole_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_quadrupole_has_cell[0], 0);
  assert_close(quadrupole[0], 0.125, 1.0e-12);
  assert_close(quadrupole[5], 0.75, 1.0e-12);

  double stress[9] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult stress_result =
      nwchemc_session_stress(session, 2, positions, atomic_numbers, stress);
  assert_int_equal(stress_result.ok, 1);
  assert_close(stress_result.energy_h, -2.0, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_stress_calls, 1);
  assert_int_equal(g_stress_cell_calls, 1);
  assert_int_equal(g_stress_n_atoms[0], 2);
  assert_int_equal(g_stress_charge[0], 0);
  assert_int_equal(g_stress_multiplicity[0], 1);
  assert_int_equal(g_stress_atomic_numbers[0][0], 1);
  assert_int_equal(g_stress_atomic_numbers[0][1], 8);
  assert_close(g_stress_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_stress_has_cell[0], 0);
  assert_close(stress[0], 0.03125, 1.0e-12);
  assert_close(stress[8], 0.28125, 1.0e-12);

  nwchemc_session_destroy(session);
  free(message);
}

static void test_session_force_input_state_overrides_params(void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  size_t step_a_size = 0;
  size_t step_state_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  unsigned char *step_state = read_file(g_force_step_state_path,
                                        &step_state_size);
  assert_non_null(message);
  assert_non_null(step_a);
  assert_non_null(step_state);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);

  assert_true(nwchemc_session_calculate_energy != NULL);
  NWChemCResult default_energy =
      nwchemc_session_calculate_energy(session, step_a, step_a_size);
  assert_int_equal(default_energy.ok, 1);
  assert_close(default_energy.energy_h, -1.0, 1.0e-12);
  assert_int_equal(g_energy_only_calls, 1);
  assert_int_equal(g_energy_only_cell_calls, 1);
  assert_int_equal(g_energy_grad_calls, 0);
  assert_int_equal(g_call_has_cell[0], 1);
  assert_int_equal(g_call_charge[0], 0);
  assert_int_equal(g_call_multiplicity[0], 1);

  NWChemCResult override_energy =
      nwchemc_session_calculate_energy(session, step_state, step_state_size);
  assert_int_equal(override_energy.ok, 1);
  assert_close(override_energy.energy_h, -1.0, 1.0e-12);
  assert_int_equal(g_energy_only_calls, 2);
  assert_int_equal(g_energy_only_cell_calls, 2);
  assert_int_equal(g_energy_grad_calls, 0);
  assert_int_equal(g_call_has_cell[1], 1);
  assert_int_equal(g_call_charge[1], -2);
  assert_int_equal(g_call_multiplicity[1], 5);

  reset_embed_captures();
  double forces[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult default_forces =
      nwchemc_session_calculate_forces(session, step_a, step_a_size, forces, 6);
  assert_int_equal(default_forces.ok, 1);
  assert_int_equal(g_energy_grad_calls, 1);
  assert_int_equal(g_call_charge[0], 0);
  assert_int_equal(g_call_multiplicity[0], 1);

  NWChemCResult override_forces = nwchemc_session_calculate_forces(
      session, step_state, step_state_size, forces, 6);
  assert_int_equal(override_forces.ok, 1);
  assert_int_equal(g_energy_grad_calls, 2);
  assert_int_equal(g_call_charge[1], -2);
  assert_int_equal(g_call_multiplicity[1], 5);

  unsigned char result_bytes[2048];
  size_t result_size = 0;
  double gradient[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult override_gradient = nwchemc_session_calculate_gradient(
      session, step_state, step_state_size, gradient, 6);
  assert_int_equal(override_gradient.ok, 1);
  assert_int_equal(g_energy_grad_calls, 3);
  assert_int_equal(g_call_charge[2], -2);
  assert_int_equal(g_call_multiplicity[2], 5);
  for (int i = 0; i < 6; ++i)
    assert_close(gradient[i], (double)(i + 1), 1.0e-12);

  const double bohr_to_angstrom = 0.529177210903;
  double hartree_angstrom_gradient[6];
  for (int i = 0; i < 6; ++i)
    hartree_angstrom_gradient[i] = gradient[i] / bohr_to_angstrom;

  result_size = 0;
  NWChemCResult gradient_carrier =
      nwchemc_session_calculate_gradient_result(
          session, step_state, step_state_size, result_bytes,
          sizeof(result_bytes), &result_size);
  assert_int_equal(gradient_carrier.ok, 1);
  assert_int_equal(g_energy_grad_calls, 4);
  assert_int_equal(g_call_charge[3], -2);
  assert_int_equal(g_call_multiplicity[3], 5);
  assert_potential_result_gradient(result_bytes, result_size, -1.0,
                                   hartree_angstrom_gradient, 6, 1.0e-12);

  result_size = 0;
  NWChemCResult force_result = nwchemc_session_calculate_result(
      session, step_state, step_state_size, result_bytes, sizeof(result_bytes),
      &result_size);
  assert_int_equal(force_result.ok, 1);
  assert_int_equal(g_call_charge[4], -2);
  assert_int_equal(g_call_multiplicity[4], 5);

  double hessian[36] = {0.0};
  NWChemCResult hessian_result = nwchemc_session_calculate_hessian(
      session, step_state, step_state_size, hessian, 36);
  assert_int_equal(hessian_result.ok, 1);
  assert_close(hessian_result.energy_h, -1.125, 1.0e-12);
  assert_int_equal(g_hessian_charge[0], -2);
  assert_int_equal(g_hessian_multiplicity[0], 5);

  result_size = 0;
  NWChemCResult hessian_carrier = nwchemc_session_calculate_hessian_result(
      session, step_state, step_state_size, result_bytes, sizeof(result_bytes),
      &result_size);
  assert_int_equal(hessian_carrier.ok, 1);
  assert_close(hessian_carrier.energy_h, -1.125, 1.0e-12);
  assert_int_equal(g_hessian_charge[1], -2);
  assert_int_equal(g_hessian_multiplicity[1], 5);

  double dipole[3] = {0.0, 0.0, 0.0};
  NWChemCResult dipole_raw = nwchemc_session_calculate_dipole(
      session, step_state, step_state_size, dipole, 3);
  assert_int_equal(dipole_raw.ok, 1);
  assert_int_equal(g_dipole_charge[0], -2);
  assert_int_equal(g_dipole_multiplicity[0], 5);

  result_size = 0;
  NWChemCResult dipole_carrier = nwchemc_session_calculate_dipole_result(
      session, step_state, step_state_size, result_bytes, sizeof(result_bytes),
      &result_size);
  assert_int_equal(dipole_carrier.ok, 1);
  assert_int_equal(g_dipole_charge[1], -2);
  assert_int_equal(g_dipole_multiplicity[1], 5);

  double quadrupole[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult quadrupole_raw = nwchemc_session_calculate_quadrupole(
      session, step_state, step_state_size, quadrupole, 6);
  assert_int_equal(quadrupole_raw.ok, 1);
  assert_int_equal(g_quadrupole_charge[0], -2);
  assert_int_equal(g_quadrupole_multiplicity[0], 5);

  result_size = 0;
  NWChemCResult quadrupole_carrier =
      nwchemc_session_calculate_quadrupole_result(
          session, step_state, step_state_size, result_bytes,
          sizeof(result_bytes), &result_size);
  assert_int_equal(quadrupole_carrier.ok, 1);
  assert_int_equal(g_quadrupole_charge[1], -2);
  assert_int_equal(g_quadrupole_multiplicity[1], 5);

  double stress[9] = {0.0};
  NWChemCResult stress_raw = nwchemc_session_calculate_stress(
      session, step_state, step_state_size, stress, 9);
  assert_int_equal(stress_raw.ok, 1);
  assert_int_equal(g_stress_charge[0], -2);
  assert_int_equal(g_stress_multiplicity[0], 5);

  result_size = 0;
  NWChemCResult stress_carrier = nwchemc_session_calculate_stress_result(
      session, step_state, step_state_size, result_bytes, sizeof(result_bytes),
      &result_size);
  assert_int_equal(stress_carrier.ok, 1);
  assert_int_equal(g_stress_charge[1], -2);
  assert_int_equal(g_stress_multiplicity[1], 5);

  double optimized_positions[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult optimize_raw = nwchemc_session_calculate_optimize(
      session, step_state, step_state_size, optimized_positions, 6);
  assert_int_equal(optimize_raw.ok, 1);
  assert_int_equal(g_optimize_charge[0], -2);
  assert_int_equal(g_optimize_multiplicity[0], 5);

  result_size = 0;
  NWChemCResult optimize_carrier = nwchemc_session_calculate_optimize_result(
      session, step_state, step_state_size, result_bytes, sizeof(result_bytes),
      &result_size);
  assert_int_equal(optimize_carrier.ok, 1);
  assert_int_equal(g_optimize_charge[1], -2);
  assert_int_equal(g_optimize_multiplicity[1], 5);

  double frequencies[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  double intensities[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult frequencies_raw = nwchemc_session_calculate_frequencies(
      session, step_state, step_state_size, frequencies, 6, intensities, 6);
  assert_int_equal(frequencies_raw.ok, 1);
  assert_close(frequencies_raw.energy_h, -1.625, 1.0e-12);
  assert_int_equal(g_frequency_charge[0], -2);
  assert_int_equal(g_frequency_multiplicity[0], 5);

  result_size = 0;
  NWChemCResult frequency_carrier =
      nwchemc_session_calculate_frequencies_result(
          session, step_state, step_state_size, result_bytes,
          sizeof(result_bytes), &result_size);
  assert_int_equal(frequency_carrier.ok, 1);
  assert_close(frequency_carrier.energy_h, -1.625, 1.0e-12);
  assert_int_equal(g_frequency_charge[1], -2);
  assert_int_equal(g_frequency_multiplicity[1], 5);

  nwchemc_session_destroy(session);
  free(step_state);
  free(step_a);
  free(message);
}

static void test_session_calculate_result_writes_potential_result(
    void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  size_t step_a_size = 0;
  size_t step_b_size = 0;
  size_t step_ev_size = 0;
  size_t step_changed_species_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  unsigned char *step_b = read_file(g_force_step_b_path, &step_b_size);
  unsigned char *step_ev = read_file(g_force_step_ev_path, &step_ev_size);
  unsigned char *step_changed_species = read_file(
      g_force_step_changed_species_path, &step_changed_species_size);
  assert_non_null(message);
  assert_non_null(step_a);
  assert_non_null(step_b);
  assert_non_null(step_ev);
  assert_non_null(step_changed_species);
  assert_true(nwchemc_forces_result_size_for_force_input != NULL);
  assert_true(nwchemc_session_calculate_forces_result != NULL);
  assert_true(nwchemc_calculate_forces_result != NULL);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);

  unsigned char result_bytes[512];
  size_t result_size = 0;
  size_t expected_step_a_size =
      nwchemc_potential_result_size_for_force_input(step_a, step_a_size);
  assert_true(expected_step_a_size > 0);
  assert_int_equal(nwchemc_forces_result_size_for_force_input(step_a, step_a_size),
                   expected_step_a_size);
  NWChemCResult native = nwchemc_session_calculate_result(
      session, step_a, step_a_size, result_bytes, sizeof(result_bytes),
      &result_size);
  assert_int_equal(native.ok, 1);
  assert_close(native.energy_h, -1.0, 1.0e-12);
  assert_int_equal(result_size, expected_step_a_size);
  assert_true(result_size < sizeof(result_bytes));
  const double native_forces[6] = {-1.0, -2.0, -3.0, -4.0, -5.0, -6.0};
  const double bohr_to_angstrom = 0.529177210903;
  double hartree_angstrom_forces[6];
  for (int i = 0; i < 6; ++i)
    hartree_angstrom_forces[i] = native_forces[i] / bohr_to_angstrom;
  assert_potential_result(result_bytes, result_size, -1.0,
                          hartree_angstrom_forces, 6,
                          1.0e-12);
  assert_int_equal(g_energy_grad_calls, 1);
  assert_int_equal(g_set_config_calls, 1);

  result_size = 0;
  NWChemCResult force_named =
      nwchemc_session_calculate_forces_result(
          session, step_a, step_a_size, result_bytes, sizeof(result_bytes),
          &result_size);
  assert_int_equal(force_named.ok, 1);
  assert_close(force_named.energy_h, -1.0, 1.0e-12);
  assert_int_equal(result_size, expected_step_a_size);
  assert_potential_result(result_bytes, result_size, -1.0,
                          hartree_angstrom_forces, 6, 1.0e-12);
  assert_int_equal(g_energy_grad_calls, 2);
  assert_int_equal(g_set_config_calls, 1);

  size_t bohr_result_size = 0;
  size_t expected_step_b_size =
      nwchemc_potential_result_size_for_force_input(step_b, step_b_size);
  assert_int_equal(expected_step_b_size, expected_step_a_size);
  NWChemCResult bohr = nwchemc_session_calculate_result(
      session, step_b, step_b_size, result_bytes, sizeof(result_bytes),
      &bohr_result_size);
  assert_int_equal(bohr.ok, 1);
  assert_close(bohr.energy_h, -1.0, 1.0e-12);
  assert_int_equal(bohr_result_size, result_size);
  assert_potential_result(result_bytes, bohr_result_size, -1.0,
                          native_forces, 6, 1.0e-12);
  assert_int_equal(g_energy_grad_calls, 3);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_call_n_atoms[2], 2);
  assert_close(g_call_positions_ang[2][5], 1.058354421806, 1.0e-12);
  assert_int_equal(g_call_has_cell[2], 1);
  assert_close(g_call_cell_ang[2][0], 10.58354421806, 1.0e-11);

  size_t changed_result_size = 0;
  NWChemCResult changed_species = nwchemc_session_calculate_result(
      session, step_changed_species, step_changed_species_size, result_bytes,
      sizeof(result_bytes), &changed_result_size);
  assert_int_equal(changed_species.ok, 0);
  assert_non_null(strstr(changed_species.message, "topology"));
  assert_int_equal(changed_result_size, result_size);
  assert_int_equal(g_energy_grad_calls, 3);

  unsigned char short_result[79];
  size_t required_size = 0;
  NWChemCResult short_output = nwchemc_session_calculate_result(
      session, step_a, step_a_size, short_result, sizeof(short_result),
      &required_size);
  assert_int_equal(short_output.ok, 0);
  assert_int_equal(required_size, result_size);
  assert_int_equal(g_energy_grad_calls, 3);

  result_size = 0;
  NWChemCResult ev = nwchemc_session_calculate_result(
      session, step_ev, step_ev_size, result_bytes, sizeof(result_bytes),
      &result_size);
  assert_int_equal(ev.ok, 1);
  assert_close(ev.energy_h, -1.0, 1.0e-12);
  const double hartree_to_ev = 27.211386245988;
  double ev_forces[6];
  for (int i = 0; i < 6; ++i)
    ev_forces[i] = native_forces[i] * hartree_to_ev / bohr_to_angstrom;
  assert_potential_result(result_bytes, result_size, -hartree_to_ev, ev_forces,
                          6, 1.0e-10);
  assert_int_equal(g_energy_grad_calls, 4);
  assert_int_equal(g_set_config_calls, 1);

  nwchemc_session_destroy(session);
  free(step_changed_species);
  free(step_ev);
  free(step_b);
  free(step_a);
  free(message);
}

static void test_session_calculate_energy_result_writes_potential_result(
    void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_energy_result_size_for_force_input != NULL);
  assert_true(nwchemc_session_calculate_energy_result != NULL);
  assert_true(nwchemc_calculate_energy_result != NULL);
  size_t message_size = 0;
  size_t step_a_size = 0;
  size_t step_ev_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  unsigned char *step_ev = read_file(g_force_step_ev_path, &step_ev_size);
  assert_non_null(message);
  assert_non_null(step_a);
  assert_non_null(step_ev);

  size_t expected_size =
      nwchemc_energy_result_size_for_force_input(step_a, step_a_size);
  assert_true(expected_size > 0);
  unsigned char result_bytes[512];
  size_t result_size = 0;
  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);

  NWChemCResult native = nwchemc_session_calculate_energy_result(
      session, step_a, step_a_size, result_bytes, sizeof(result_bytes),
      &result_size);
  assert_int_equal(native.ok, 1);
  assert_close(native.energy_h, -1.0, 1.0e-12);
  assert_int_equal(result_size, expected_size);
  assert_potential_result_energy_only(result_bytes, result_size, -1.0,
                                      1.0e-12);
  assert_int_equal(g_energy_only_calls, 1);
  assert_int_equal(g_energy_grad_calls, 0);

  const double hartree_to_ev = 27.211386245988;
  result_size = 0;
  NWChemCResult ev = nwchemc_session_calculate_energy_result(
      session, step_ev, step_ev_size, result_bytes, sizeof(result_bytes),
      &result_size);
  assert_int_equal(ev.ok, 1);
  assert_close(ev.energy_h, -1.0, 1.0e-12);
  assert_int_equal(result_size, expected_size);
  assert_potential_result_energy_only(result_bytes, result_size,
                                      -hartree_to_ev, 1.0e-10);
  assert_int_equal(g_energy_only_calls, 2);
  assert_int_equal(g_energy_grad_calls, 0);
  nwchemc_session_destroy(session);

  result_size = 0;
  NWChemCResult one_shot = nwchemc_calculate_energy_result(
      message, message_size, step_a, step_a_size, result_bytes,
      sizeof(result_bytes), &result_size);
  assert_int_equal(one_shot.ok, 1);
  assert_close(one_shot.energy_h, -1.0, 1.0e-12);
  assert_int_equal(result_size, expected_size);
  assert_potential_result_energy_only(result_bytes, result_size, -1.0,
                                      1.0e-12);

  free(step_ev);
  free(step_a);
  free(message);
}

static void test_session_calculate_hessian_result_writes_potential_result(
    void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_hessian_result_size_for_force_input != NULL);
  assert_true(nwchemc_session_calculate_hessian_result != NULL);
  size_t message_size = 0;
  size_t step_a_size = 0;
  size_t step_changed_species_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  unsigned char *step_changed_species = read_file(
      g_force_step_changed_species_path, &step_changed_species_size);
  assert_non_null(message);
  assert_non_null(step_a);
  assert_non_null(step_changed_species);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);

  unsigned char result_bytes[2048];
  size_t result_size = 0;
  size_t expected_size =
      nwchemc_hessian_result_size_for_force_input(step_a, step_a_size);
  assert_true(expected_size > 0);
  NWChemCResult result = nwchemc_session_calculate_hessian_result(
      session, step_a, step_a_size, result_bytes, sizeof(result_bytes),
      &result_size);
  assert_int_equal(result.ok, 1);
  assert_close(result.energy_h, -1.125, 1.0e-12);
  assert_int_equal(result_size, expected_size);
  assert_int_equal(g_hessian_calls, 1);
  assert_int_equal(g_hessian_cell_calls, 1);
  double expected_hessian[36];
  const double bohr_to_angstrom = 0.529177210903;
  for (int i = 0; i < 36; ++i)
    expected_hessian[i] =
        (double)(i + 10) / (bohr_to_angstrom * bohr_to_angstrom);
  assert_potential_result_hessian(result_bytes, result_size, -1.125,
                                  expected_hessian, 36, 1.0e-10);

  size_t changed_result_size = 0;
  NWChemCResult changed_species = nwchemc_session_calculate_hessian_result(
      session, step_changed_species, step_changed_species_size, result_bytes,
      sizeof(result_bytes), &changed_result_size);
  assert_int_equal(changed_species.ok, 0);
  assert_non_null(strstr(changed_species.message, "topology"));
  assert_int_equal(changed_result_size, expected_size);
  assert_int_equal(g_hessian_calls, 1);

  unsigned char short_result[127];
  size_t required_size = 0;
  NWChemCResult short_output = nwchemc_session_calculate_hessian_result(
      session, step_a, step_a_size, short_result, sizeof(short_result),
      &required_size);
  assert_int_equal(short_output.ok, 0);
  assert_int_equal(required_size, expected_size);
  assert_int_equal(g_hessian_calls, 1);

  nwchemc_session_destroy(session);
  free(step_changed_species);
  free(step_a);
  free(message);
}

static void test_session_calculate_property_results_write_potential_result(
    void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_dipole_result_size_for_force_input != NULL);
  assert_true(nwchemc_session_calculate_dipole_result != NULL);
  assert_true(nwchemc_quadrupole_result_size_for_force_input != NULL);
  assert_true(nwchemc_session_calculate_quadrupole_result != NULL);
  size_t message_size = 0;
  size_t step_a_size = 0;
  size_t step_ev_size = 0;
  size_t step_changed_species_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  unsigned char *step_ev = read_file(g_force_step_ev_path, &step_ev_size);
  unsigned char *step_changed_species = read_file(
      g_force_step_changed_species_path, &step_changed_species_size);
  assert_non_null(message);
  assert_non_null(step_a);
  assert_non_null(step_ev);
  assert_non_null(step_changed_species);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);

  unsigned char result_bytes[512];
  size_t dipole_result_size = 0;
  size_t expected_dipole_size =
      nwchemc_dipole_result_size_for_force_input(step_a, step_a_size);
  assert_true(expected_dipole_size > 0);
  NWChemCResult dipole_result = nwchemc_session_calculate_dipole_result(
      session, step_a, step_a_size, result_bytes, sizeof(result_bytes),
      &dipole_result_size);
  assert_int_equal(dipole_result.ok, 1);
  assert_close(dipole_result.energy_h, -1.25, 1.0e-12);
  assert_int_equal(dipole_result_size, expected_dipole_size);
  assert_int_equal(g_dipole_calls, 1);
  assert_int_equal(g_dipole_cell_calls, 1);
  const double expected_dipole[3] = {0.25, 0.5, 0.75};
  assert_potential_result_dipole(result_bytes, dipole_result_size, -1.25,
                                 expected_dipole, 1.0e-12);

  const double hartree_to_ev = 27.211386245988;
  dipole_result_size = 0;
  NWChemCResult dipole_ev = nwchemc_session_calculate_dipole_result(
      session, step_ev, step_ev_size, result_bytes, sizeof(result_bytes),
      &dipole_result_size);
  assert_int_equal(dipole_ev.ok, 1);
  assert_close(dipole_ev.energy_h, -1.25, 1.0e-12);
  assert_int_equal(dipole_result_size, expected_dipole_size);
  assert_potential_result_dipole(result_bytes, dipole_result_size,
                                 -1.25 * hartree_to_ev, expected_dipole,
                                 1.0e-10);

  size_t changed_dipole_size = 0;
  NWChemCResult changed_dipole =
      nwchemc_session_calculate_dipole_result(
          session, step_changed_species, step_changed_species_size,
          result_bytes, sizeof(result_bytes), &changed_dipole_size);
  assert_int_equal(changed_dipole.ok, 0);
  assert_non_null(strstr(changed_dipole.message, "topology"));
  assert_int_equal(changed_dipole_size, expected_dipole_size);
  assert_int_equal(g_dipole_calls, 2);

  unsigned char short_result[63];
  size_t required_size = 0;
  NWChemCResult short_dipole = nwchemc_session_calculate_dipole_result(
      session, step_a, step_a_size, short_result, sizeof(short_result),
      &required_size);
  assert_int_equal(short_dipole.ok, 0);
  assert_int_equal(required_size, expected_dipole_size);
  assert_int_equal(g_dipole_calls, 2);

  size_t quadrupole_result_size = 0;
  size_t expected_quadrupole_size =
      nwchemc_quadrupole_result_size_for_force_input(step_a, step_a_size);
  assert_true(expected_quadrupole_size > 0);
  NWChemCResult quadrupole_result =
      nwchemc_session_calculate_quadrupole_result(
          session, step_a, step_a_size, result_bytes, sizeof(result_bytes),
          &quadrupole_result_size);
  assert_int_equal(quadrupole_result.ok, 1);
  assert_close(quadrupole_result.energy_h, -1.5, 1.0e-12);
  assert_int_equal(quadrupole_result_size, expected_quadrupole_size);
  assert_int_equal(g_quadrupole_calls, 1);
  assert_int_equal(g_quadrupole_cell_calls, 1);
  const double expected_quadrupole[6] = {0.125, 0.25, 0.375,
                                         0.5,   0.625, 0.75};
  assert_potential_result_quadrupole(result_bytes, quadrupole_result_size,
                                     -1.5, expected_quadrupole, 1.0e-12);

  quadrupole_result_size = 0;
  NWChemCResult quadrupole_ev =
      nwchemc_session_calculate_quadrupole_result(
          session, step_ev, step_ev_size, result_bytes, sizeof(result_bytes),
          &quadrupole_result_size);
  assert_int_equal(quadrupole_ev.ok, 1);
  assert_close(quadrupole_ev.energy_h, -1.5, 1.0e-12);
  assert_int_equal(quadrupole_result_size, expected_quadrupole_size);
  assert_potential_result_quadrupole(result_bytes, quadrupole_result_size,
                                     -1.5 * hartree_to_ev,
                                     expected_quadrupole, 1.0e-10);

  size_t changed_quadrupole_size = 0;
  NWChemCResult changed_quadrupole =
      nwchemc_session_calculate_quadrupole_result(
          session, step_changed_species, step_changed_species_size,
          result_bytes, sizeof(result_bytes), &changed_quadrupole_size);
  assert_int_equal(changed_quadrupole.ok, 0);
  assert_non_null(strstr(changed_quadrupole.message, "topology"));
  assert_int_equal(changed_quadrupole_size, expected_quadrupole_size);
  assert_int_equal(g_quadrupole_calls, 2);

  required_size = 0;
  NWChemCResult short_quadrupole =
      nwchemc_session_calculate_quadrupole_result(
          session, step_a, step_a_size, short_result, sizeof(short_result),
          &required_size);
  assert_int_equal(short_quadrupole.ok, 0);
  assert_int_equal(required_size, expected_quadrupole_size);
  assert_int_equal(g_quadrupole_calls, 2);

  nwchemc_session_destroy(session);
  free(step_changed_species);
  free(step_ev);
  free(step_a);
  free(message);
}

static void test_session_calculate_stress_result_writes_potential_result(
    void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_stress_result_size_for_force_input != NULL);
  assert_true(nwchemc_session_calculate_stress_result != NULL);
  size_t message_size = 0;
  size_t step_a_size = 0;
  size_t step_ev_size = 0;
  size_t step_changed_species_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  unsigned char *step_ev = read_file(g_force_step_ev_path, &step_ev_size);
  unsigned char *step_changed_species = read_file(
      g_force_step_changed_species_path, &step_changed_species_size);
  assert_non_null(message);
  assert_non_null(step_a);
  assert_non_null(step_ev);
  assert_non_null(step_changed_species);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);

  const double bohr_to_angstrom = 0.529177210903;
  const double hartree_to_ev = 27.211386245988;
  double expected_stress[9];
  double expected_stress_ev[9];
  for (int i = 0; i < 9; ++i) {
    double native_stress = 0.03125 * (double)(i + 1);
    expected_stress[i] =
        native_stress / (bohr_to_angstrom * bohr_to_angstrom *
                         bohr_to_angstrom);
    expected_stress_ev[i] = expected_stress[i] * hartree_to_ev;
  }

  unsigned char result_bytes[512];
  size_t stress_result_size = 0;
  size_t expected_stress_size =
      nwchemc_stress_result_size_for_force_input(step_a, step_a_size);
  assert_true(expected_stress_size > 0);
  NWChemCResult stress_result = nwchemc_session_calculate_stress_result(
      session, step_a, step_a_size, result_bytes, sizeof(result_bytes),
      &stress_result_size);
  assert_int_equal(stress_result.ok, 1);
  assert_close(stress_result.energy_h, -2.0, 1.0e-12);
  assert_int_equal(stress_result_size, expected_stress_size);
  assert_int_equal(g_stress_calls, 1);
  assert_int_equal(g_stress_cell_calls, 1);
  assert_potential_result_stress(result_bytes, stress_result_size, -2.0,
                                 expected_stress, 1.0e-12);

  stress_result_size = 0;
  NWChemCResult stress_ev = nwchemc_session_calculate_stress_result(
      session, step_ev, step_ev_size, result_bytes, sizeof(result_bytes),
      &stress_result_size);
  assert_int_equal(stress_ev.ok, 1);
  assert_close(stress_ev.energy_h, -2.0, 1.0e-12);
  assert_int_equal(stress_result_size, expected_stress_size);
  assert_int_equal(g_stress_calls, 2);
  assert_potential_result_stress(result_bytes, stress_result_size,
                                 -2.0 * hartree_to_ev, expected_stress_ev,
                                 1.0e-10);

  size_t changed_stress_size = 0;
  NWChemCResult changed_stress =
      nwchemc_session_calculate_stress_result(
          session, step_changed_species, step_changed_species_size,
          result_bytes, sizeof(result_bytes), &changed_stress_size);
  assert_int_equal(changed_stress.ok, 0);
  assert_non_null(strstr(changed_stress.message, "topology"));
  assert_int_equal(changed_stress_size, expected_stress_size);
  assert_int_equal(g_stress_calls, 2);

  unsigned char short_result[63];
  size_t required_size = 0;
  NWChemCResult short_stress = nwchemc_session_calculate_stress_result(
      session, step_a, step_a_size, short_result, sizeof(short_result),
      &required_size);
  assert_int_equal(short_stress.ok, 0);
  assert_int_equal(required_size, expected_stress_size);
  assert_int_equal(g_stress_calls, 2);

  nwchemc_session_destroy(session);
  free(step_changed_species);
  free(step_ev);
  free(step_a);
  free(message);
}

static void test_session_calculate_structural_results_write_potential_result(
    void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_optimize_result_size_for_force_input != NULL);
  assert_true(nwchemc_session_calculate_optimize_result != NULL);
  assert_true(nwchemc_frequencies_result_size_for_force_input != NULL);
  assert_true(nwchemc_session_calculate_frequencies_result != NULL);
  size_t message_size = 0;
  size_t step_a_size = 0;
  size_t step_b_size = 0;
  size_t step_changed_species_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  unsigned char *step_b = read_file(g_force_step_b_path, &step_b_size);
  unsigned char *step_changed_species = read_file(
      g_force_step_changed_species_path, &step_changed_species_size);
  assert_non_null(message);
  assert_non_null(step_a);
  assert_non_null(step_b);
  assert_non_null(step_changed_species);

  NWChemCSession *session = nwchemc_session_create(message, message_size);
  assert_non_null(session);
  assert_int_equal(g_set_config_calls, 1);

  unsigned char result_bytes[2048];
  unsigned char short_result[63];
  size_t result_size = 0;
  size_t required_size = 0;
  const double expected_optimized_ang[6] = {0.01, 0.02, 0.03,
                                            0.04, 0.05, 0.8014};
  size_t expected_optimize_size =
      nwchemc_optimize_result_size_for_force_input(step_a, step_a_size);
  assert_true(expected_optimize_size > 0);

  NWChemCResult short_optimize =
      nwchemc_session_calculate_optimize_result(
          session, step_a, step_a_size, short_result, sizeof(short_result),
          &required_size);
  assert_int_equal(short_optimize.ok, 0);
  assert_int_equal(required_size, expected_optimize_size);
  assert_int_equal(g_optimize_calls, 0);

  NWChemCResult optimize_result = nwchemc_session_calculate_optimize_result(
      session, step_a, step_a_size, result_bytes, sizeof(result_bytes),
      &result_size);
  assert_int_equal(optimize_result.ok, 1);
  assert_close(optimize_result.energy_h, -1.75, 1.0e-12);
  assert_int_equal(result_size, expected_optimize_size);
  assert_int_equal(g_optimize_calls, 1);
  assert_int_equal(g_optimize_cell_calls, 1);
  assert_potential_result_optimized(result_bytes, result_size, -1.75,
                                    expected_optimized_ang, 6, 1.0e-12);

  const double bohr_to_angstrom = 0.529177210903;
  double expected_optimized_bohr[6];
  for (int i = 0; i < 6; ++i) {
    double input_ang = i == 5 ? 2.0 * bohr_to_angstrom : 0.0;
    expected_optimized_bohr[i] =
        (input_ang + 0.01 * (double)(i + 1)) / bohr_to_angstrom;
  }
  size_t expected_bohr_optimize_size =
      nwchemc_optimize_result_size_for_force_input(step_b, step_b_size);
  assert_int_equal(expected_bohr_optimize_size, expected_optimize_size);
  result_size = 0;
  NWChemCResult optimize_bohr = nwchemc_session_calculate_optimize_result(
      session, step_b, step_b_size, result_bytes, sizeof(result_bytes),
      &result_size);
  assert_int_equal(optimize_bohr.ok, 1);
  assert_int_equal(result_size, expected_optimize_size);
  assert_int_equal(g_optimize_calls, 2);
  assert_close(g_optimize_positions_ang[1][5], 1.058354421806, 1.0e-12);
  assert_potential_result_optimized(result_bytes, result_size, -1.75,
                                    expected_optimized_bohr, 6, 1.0e-12);

  size_t changed_optimize_size = 0;
  NWChemCResult changed_optimize =
      nwchemc_session_calculate_optimize_result(
          session, step_changed_species, step_changed_species_size,
          result_bytes, sizeof(result_bytes), &changed_optimize_size);
  assert_int_equal(changed_optimize.ok, 0);
  assert_non_null(strstr(changed_optimize.message, "topology"));
  assert_int_equal(changed_optimize_size, expected_optimize_size);
  assert_int_equal(g_optimize_calls, 2);

  size_t expected_frequencies_size =
      nwchemc_frequencies_result_size_for_force_input(step_a, step_a_size);
  assert_true(expected_frequencies_size > 0);
  required_size = 0;
  NWChemCResult short_frequencies =
      nwchemc_session_calculate_frequencies_result(
          session, step_a, step_a_size, short_result, sizeof(short_result),
          &required_size);
  assert_int_equal(short_frequencies.ok, 0);
  assert_int_equal(required_size, expected_frequencies_size);
  assert_int_equal(g_frequency_calls, 0);
  assert_int_equal(g_frequency_modes_calls, 0);
  assert_int_equal(g_frequency_detail_calls, 0);

  result_size = 0;
  NWChemCResult frequencies_result =
      nwchemc_session_calculate_frequencies_result(
          session, step_a, step_a_size, result_bytes, sizeof(result_bytes),
          &result_size);
  assert_int_equal(frequencies_result.ok, 1);
  assert_close(frequencies_result.energy_h, -1.625, 1.0e-12);
  assert_int_equal(result_size, expected_frequencies_size);
  assert_int_equal(g_frequency_calls, 1);
  assert_int_equal(g_frequency_cell_calls, 0);
  assert_int_equal(g_frequency_modes_calls, 1);
  assert_int_equal(g_frequency_modes_cell_calls, 0);
  assert_int_equal(g_frequency_detail_calls, 1);
  assert_int_equal(g_frequency_detail_cell_calls, 1);
  const double expected_frequencies[6] = {100.0, 101.0, 102.0,
                                          103.0, 104.0, 105.0};
  const double expected_intensities[6] = {0.01, 0.02, 0.03,
                                          0.04, 0.05, 0.06};
  const double expected_projected_frequencies[6] = {90.0, 91.0, 92.0,
                                                    93.0, 94.0, 95.0};
  const double expected_projected_intensities[6] = {0.02, 0.04, 0.06,
                                                    0.08, 0.10, 0.12};
  assert_potential_result_frequencies(result_bytes, result_size, -1.625,
                                      expected_frequencies,
                                      expected_intensities,
                                      expected_projected_frequencies,
                                      expected_projected_intensities, 6,
                                      1.0e-12);

  size_t changed_frequencies_size = 0;
  NWChemCResult changed_frequencies =
      nwchemc_session_calculate_frequencies_result(
          session, step_changed_species, step_changed_species_size,
          result_bytes, sizeof(result_bytes), &changed_frequencies_size);
  assert_int_equal(changed_frequencies.ok, 0);
  assert_non_null(strstr(changed_frequencies.message, "topology"));
  assert_int_equal(changed_frequencies_size, expected_frequencies_size);
  assert_int_equal(g_frequency_calls, 1);
  assert_int_equal(g_frequency_modes_calls, 1);
  assert_int_equal(g_frequency_detail_calls, 1);

  nwchemc_session_destroy(session);
  free(step_changed_species);
  free(step_b);
  free(step_a);
  free(message);
}

static void test_calculate_result_one_shot_writes_potential_result(
    void **state) {
  (void)state;
  reset_embed_captures();
  size_t message_size = 0;
  size_t step_a_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  assert_non_null(message);
  assert_non_null(step_a);
  assert_true(nwchemc_forces_result_size_for_force_input != NULL);
  assert_true(nwchemc_calculate_forces_result != NULL);

  unsigned char result_bytes[512];
  size_t result_size = 0;
  size_t expected_size =
      nwchemc_potential_result_size_for_force_input(step_a, step_a_size);
  assert_true(expected_size > 0);
  assert_int_equal(nwchemc_forces_result_size_for_force_input(step_a, step_a_size),
                   expected_size);

  NWChemCResult one_shot = nwchemc_calculate_result(
      message, message_size, step_a, step_a_size, result_bytes,
      sizeof(result_bytes), &result_size);
  assert_int_equal(one_shot.ok, 1);
  assert_close(one_shot.energy_h, -1.0, 1.0e-12);
  assert_int_equal(result_size, expected_size);
  const double native_forces[6] = {-1.0, -2.0, -3.0, -4.0, -5.0, -6.0};
  const double bohr_to_angstrom = 0.529177210903;
  double hartree_angstrom_forces[6];
  for (int i = 0; i < 6; ++i)
    hartree_angstrom_forces[i] = native_forces[i] / bohr_to_angstrom;
  assert_potential_result(result_bytes, result_size, -1.0,
                          hartree_angstrom_forces, 6, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_energy_grad_calls, 1);

  reset_embed_captures();
  result_size = 0;
  NWChemCResult force_named = nwchemc_calculate_forces_result(
      message, message_size, step_a, step_a_size, result_bytes,
      sizeof(result_bytes), &result_size);
  assert_int_equal(force_named.ok, 1);
  assert_close(force_named.energy_h, -1.0, 1.0e-12);
  assert_int_equal(result_size, expected_size);
  assert_potential_result(result_bytes, result_size, -1.0,
                          hartree_angstrom_forces, 6, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_energy_grad_calls, 1);

  reset_embed_captures();
  unsigned char short_result[79];
  size_t required_size = 0;
  NWChemCResult short_output = nwchemc_calculate_result(
      message, message_size, step_a, step_a_size, short_result,
      sizeof(short_result), &required_size);
  assert_int_equal(short_output.ok, 0);
  assert_int_equal(required_size, expected_size);
  assert_int_equal(g_set_config_calls, 0);
  assert_int_equal(g_energy_grad_calls, 0);

  free(step_a);
  free(message);
}

static void test_calculate_hessian_result_one_shot_writes_potential_result(
    void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_calculate_hessian_result != NULL);
  size_t message_size = 0;
  size_t step_a_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  assert_non_null(message);
  assert_non_null(step_a);

  unsigned char result_bytes[2048];
  size_t result_size = 0;
  size_t expected_size =
      nwchemc_hessian_result_size_for_force_input(step_a, step_a_size);
  assert_true(expected_size > 0);
  NWChemCResult result = nwchemc_calculate_hessian_result(
      message, message_size, step_a, step_a_size, result_bytes,
      sizeof(result_bytes), &result_size);
  assert_int_equal(result.ok, 1);
  assert_close(result.energy_h, -1.125, 1.0e-12);
  assert_int_equal(result_size, expected_size);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_hessian_calls, 1);
  assert_int_equal(g_hessian_cell_calls, 1);
  double expected_hessian[36];
  const double bohr_to_angstrom = 0.529177210903;
  for (int i = 0; i < 36; ++i)
    expected_hessian[i] =
        (double)(i + 10) / (bohr_to_angstrom * bohr_to_angstrom);
  assert_potential_result_hessian(result_bytes, result_size, -1.125,
                                  expected_hessian, 36, 1.0e-10);

  reset_embed_captures();
  unsigned char short_result[127];
  size_t required_size = 0;
  NWChemCResult short_output = nwchemc_calculate_hessian_result(
      message, message_size, step_a, step_a_size, short_result,
      sizeof(short_result), &required_size);
  assert_int_equal(short_output.ok, 0);
  assert_int_equal(required_size, expected_size);
  assert_int_equal(g_set_config_calls, 0);
  assert_int_equal(g_hessian_calls, 0);

  free(step_a);
  free(message);
}

static void test_calculate_property_results_one_shot_write_potential_result(
    void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_calculate_dipole_result != NULL);
  assert_true(nwchemc_calculate_quadrupole_result != NULL);
  size_t message_size = 0;
  size_t step_a_size = 0;
  size_t step_ev_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  unsigned char *step_ev = read_file(g_force_step_ev_path, &step_ev_size);
  assert_non_null(message);
  assert_non_null(step_a);
  assert_non_null(step_ev);

  unsigned char result_bytes[512];
  size_t result_size = 0;
  size_t expected_dipole_size =
      nwchemc_dipole_result_size_for_force_input(step_a, step_a_size);
  NWChemCResult dipole_result = nwchemc_calculate_dipole_result(
      message, message_size, step_a, step_a_size, result_bytes,
      sizeof(result_bytes), &result_size);
  assert_int_equal(dipole_result.ok, 1);
  assert_int_equal(result_size, expected_dipole_size);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_dipole_calls, 1);
  const double expected_dipole[3] = {0.25, 0.5, 0.75};
  assert_potential_result_dipole(result_bytes, result_size, -1.25,
                                 expected_dipole, 1.0e-12);

  reset_embed_captures();
  const double hartree_to_ev = 27.211386245988;
  result_size = 0;
  NWChemCResult dipole_ev = nwchemc_calculate_dipole_result(
      message, message_size, step_ev, step_ev_size, result_bytes,
      sizeof(result_bytes), &result_size);
  assert_int_equal(dipole_ev.ok, 1);
  assert_int_equal(result_size, expected_dipole_size);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_dipole_calls, 1);
  assert_potential_result_dipole(result_bytes, result_size,
                                 -1.25 * hartree_to_ev, expected_dipole,
                                 1.0e-10);

  reset_embed_captures();
  unsigned char short_result[63];
  size_t required_size = 0;
  NWChemCResult short_dipole = nwchemc_calculate_dipole_result(
      message, message_size, step_a, step_a_size, short_result,
      sizeof(short_result), &required_size);
  assert_int_equal(short_dipole.ok, 0);
  assert_int_equal(required_size, expected_dipole_size);
  assert_int_equal(g_set_config_calls, 0);
  assert_int_equal(g_dipole_calls, 0);

  reset_embed_captures();
  size_t expected_quadrupole_size =
      nwchemc_quadrupole_result_size_for_force_input(step_a, step_a_size);
  NWChemCResult quadrupole_result = nwchemc_calculate_quadrupole_result(
      message, message_size, step_a, step_a_size, result_bytes,
      sizeof(result_bytes), &result_size);
  assert_int_equal(quadrupole_result.ok, 1);
  assert_int_equal(result_size, expected_quadrupole_size);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_quadrupole_calls, 1);
  const double expected_quadrupole[6] = {0.125, 0.25, 0.375,
                                         0.5,   0.625, 0.75};
  assert_potential_result_quadrupole(result_bytes, result_size,
                                     -1.5, expected_quadrupole, 1.0e-12);

  reset_embed_captures();
  result_size = 0;
  NWChemCResult quadrupole_ev = nwchemc_calculate_quadrupole_result(
      message, message_size, step_ev, step_ev_size, result_bytes,
      sizeof(result_bytes), &result_size);
  assert_int_equal(quadrupole_ev.ok, 1);
  assert_int_equal(result_size, expected_quadrupole_size);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_quadrupole_calls, 1);
  assert_potential_result_quadrupole(result_bytes, result_size,
                                     -1.5 * hartree_to_ev,
                                     expected_quadrupole, 1.0e-10);

  reset_embed_captures();
  required_size = 0;
  NWChemCResult short_quadrupole = nwchemc_calculate_quadrupole_result(
      message, message_size, step_a, step_a_size, short_result,
      sizeof(short_result), &required_size);
  assert_int_equal(short_quadrupole.ok, 0);
  assert_int_equal(required_size, expected_quadrupole_size);
  assert_int_equal(g_set_config_calls, 0);
  assert_int_equal(g_quadrupole_calls, 0);

  free(step_a);
  free(step_ev);
  free(message);
}

static void test_calculate_stress_result_one_shot_writes_potential_result(
    void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_calculate_stress_result != NULL);
  size_t message_size = 0;
  size_t step_a_size = 0;
  size_t step_ev_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  unsigned char *step_ev = read_file(g_force_step_ev_path, &step_ev_size);
  assert_non_null(message);
  assert_non_null(step_a);
  assert_non_null(step_ev);

  const double bohr_to_angstrom = 0.529177210903;
  const double hartree_to_ev = 27.211386245988;
  double expected_stress[9];
  double expected_stress_ev[9];
  for (int i = 0; i < 9; ++i) {
    double native_stress = 0.03125 * (double)(i + 1);
    expected_stress[i] =
        native_stress / (bohr_to_angstrom * bohr_to_angstrom *
                         bohr_to_angstrom);
    expected_stress_ev[i] = expected_stress[i] * hartree_to_ev;
  }

  unsigned char result_bytes[512];
  size_t result_size = 0;
  size_t expected_stress_size =
      nwchemc_stress_result_size_for_force_input(step_a, step_a_size);
  assert_true(expected_stress_size > 0);
  NWChemCResult stress_result = nwchemc_calculate_stress_result(
      message, message_size, step_a, step_a_size, result_bytes,
      sizeof(result_bytes), &result_size);
  assert_int_equal(stress_result.ok, 1);
  assert_close(stress_result.energy_h, -2.0, 1.0e-12);
  assert_int_equal(result_size, expected_stress_size);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_stress_calls, 1);
  assert_potential_result_stress(result_bytes, result_size, -2.0,
                                 expected_stress, 1.0e-12);

  reset_embed_captures();
  result_size = 0;
  NWChemCResult stress_ev = nwchemc_calculate_stress_result(
      message, message_size, step_ev, step_ev_size, result_bytes,
      sizeof(result_bytes), &result_size);
  assert_int_equal(stress_ev.ok, 1);
  assert_int_equal(result_size, expected_stress_size);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_stress_calls, 1);
  assert_potential_result_stress(result_bytes, result_size,
                                 -2.0 * hartree_to_ev, expected_stress_ev,
                                 1.0e-10);

  reset_embed_captures();
  unsigned char short_result[63];
  size_t required_size = 0;
  NWChemCResult short_stress = nwchemc_calculate_stress_result(
      message, message_size, step_a, step_a_size, short_result,
      sizeof(short_result), &required_size);
  assert_int_equal(short_stress.ok, 0);
  assert_int_equal(required_size, expected_stress_size);
  assert_int_equal(g_set_config_calls, 0);
  assert_int_equal(g_stress_calls, 0);

  free(step_a);
  free(step_ev);
  free(message);
}

static void test_calculate_structural_results_one_shot_write_potential_result(
    void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_calculate_optimize_result != NULL);
  assert_true(nwchemc_calculate_frequencies_result != NULL);
  size_t message_size = 0;
  size_t step_a_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  assert_non_null(message);
  assert_non_null(step_a);

  unsigned char result_bytes[2048];
  unsigned char short_result[63];
  size_t result_size = 0;
  size_t required_size = 0;
  size_t expected_optimize_size =
      nwchemc_optimize_result_size_for_force_input(step_a, step_a_size);
  assert_true(expected_optimize_size > 0);

  NWChemCResult short_optimize = nwchemc_calculate_optimize_result(
      message, message_size, step_a, step_a_size, short_result,
      sizeof(short_result), &required_size);
  assert_int_equal(short_optimize.ok, 0);
  assert_int_equal(required_size, expected_optimize_size);
  assert_int_equal(g_set_config_calls, 0);
  assert_int_equal(g_optimize_calls, 0);

  NWChemCResult optimize_result = nwchemc_calculate_optimize_result(
      message, message_size, step_a, step_a_size, result_bytes,
      sizeof(result_bytes), &result_size);
  assert_int_equal(optimize_result.ok, 1);
  assert_close(optimize_result.energy_h, -1.75, 1.0e-12);
  assert_int_equal(result_size, expected_optimize_size);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_optimize_calls, 1);
  const double expected_optimized_ang[6] = {0.01, 0.02, 0.03,
                                            0.04, 0.05, 0.8014};
  assert_potential_result_optimized(result_bytes, result_size, -1.75,
                                    expected_optimized_ang, 6, 1.0e-12);

  reset_embed_captures();
  size_t expected_frequencies_size =
      nwchemc_frequencies_result_size_for_force_input(step_a, step_a_size);
  assert_true(expected_frequencies_size > 0);
  required_size = 0;
  NWChemCResult short_frequencies = nwchemc_calculate_frequencies_result(
      message, message_size, step_a, step_a_size, short_result,
      sizeof(short_result), &required_size);
  assert_int_equal(short_frequencies.ok, 0);
  assert_int_equal(required_size, expected_frequencies_size);
  assert_int_equal(g_set_config_calls, 0);
  assert_int_equal(g_frequency_calls, 0);
  assert_int_equal(g_frequency_modes_calls, 0);
  assert_int_equal(g_frequency_detail_calls, 0);

  result_size = 0;
  NWChemCResult frequencies_result = nwchemc_calculate_frequencies_result(
      message, message_size, step_a, step_a_size, result_bytes,
      sizeof(result_bytes), &result_size);
  assert_int_equal(frequencies_result.ok, 1);
  assert_close(frequencies_result.energy_h, -1.625, 1.0e-12);
  assert_int_equal(result_size, expected_frequencies_size);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_frequency_calls, 1);
  assert_int_equal(g_frequency_modes_calls, 1);
  assert_int_equal(g_frequency_detail_calls, 1);
  const double expected_frequencies[6] = {100.0, 101.0, 102.0,
                                          103.0, 104.0, 105.0};
  const double expected_intensities[6] = {0.01, 0.02, 0.03,
                                          0.04, 0.05, 0.06};
  const double expected_projected_frequencies[6] = {90.0, 91.0, 92.0,
                                                    93.0, 94.0, 95.0};
  const double expected_projected_intensities[6] = {0.02, 0.04, 0.06,
                                                    0.08, 0.10, 0.12};
  assert_potential_result_frequencies(result_bytes, result_size, -1.625,
                                      expected_frequencies,
                                      expected_intensities,
                                      expected_projected_frequencies,
                                      expected_projected_intensities, 6,
                                      1.0e-12);

  free(step_a);
  free(message);
}

static void test_calculate_config_results_one_shot_write_potential_results(
    void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_calculate_result_from_config != NULL);
  assert_true(nwchemc_calculate_energy_result_from_config != NULL);
  assert_true(nwchemc_calculate_forces_result_from_config != NULL);
  assert_true(nwchemc_calculate_hessian_result_from_config != NULL);
  assert_true(nwchemc_calculate_dipole_result_from_config != NULL);
  assert_true(nwchemc_calculate_quadrupole_result_from_config != NULL);
  assert_true(nwchemc_calculate_stress_result_from_config != NULL);
  assert_true(nwchemc_calculate_optimize_result_from_config != NULL);
  assert_true(nwchemc_calculate_frequencies_result_from_config != NULL);

  size_t message_size = 0;
  size_t step_a_size = 0;
  size_t config_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  assert_non_null(message);
  assert_non_null(step_a);
  unsigned char *config = wrap_params_in_config(message, message_size,
                                                &config_size);
  assert_non_null(config);

  unsigned char result_bytes[2048];
  size_t result_size = 0;
  const double bohr_to_angstrom = 0.529177210903;
  const double native_forces[6] = {-1.0, -2.0, -3.0, -4.0, -5.0, -6.0};
  double hartree_angstrom_forces[6];
  for (int i = 0; i < 6; ++i)
    hartree_angstrom_forces[i] = native_forces[i] / bohr_to_angstrom;

  size_t expected_force_size =
      nwchemc_potential_result_size_for_force_input(step_a, step_a_size);
  NWChemCResult result = nwchemc_calculate_result_from_config(
      config, config_size, step_a, step_a_size, result_bytes,
      sizeof(result_bytes), &result_size);
  assert_int_equal(result.ok, 1);
  assert_close(result.energy_h, -1.0, 1.0e-12);
  assert_int_equal(result_size, expected_force_size);
  assert_potential_result(result_bytes, result_size, -1.0,
                          hartree_angstrom_forces, 6, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_energy_grad_calls, 1);

  reset_embed_captures();
  unsigned char short_result[79];
  size_t required_size = 0;
  NWChemCResult short_output = nwchemc_calculate_result_from_config(
      config, config_size, step_a, step_a_size, short_result,
      sizeof(short_result), &required_size);
  assert_int_equal(short_output.ok, 0);
  assert_int_equal(required_size, expected_force_size);
  assert_int_equal(g_set_config_calls, 0);
  assert_int_equal(g_energy_grad_calls, 0);

  reset_embed_captures();
  result_size = 0;
  size_t expected_energy_size =
      nwchemc_energy_result_size_for_force_input(step_a, step_a_size);
  NWChemCResult energy_result = nwchemc_calculate_energy_result_from_config(
      config, config_size, step_a, step_a_size, result_bytes,
      sizeof(result_bytes), &result_size);
  assert_int_equal(energy_result.ok, 1);
  assert_close(energy_result.energy_h, -1.0, 1.0e-12);
  assert_int_equal(result_size, expected_energy_size);
  assert_potential_result_energy_only(result_bytes, result_size, -1.0,
                                      1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_energy_only_calls, 1);

  reset_embed_captures();
  result_size = 0;
  NWChemCResult force_named =
      nwchemc_calculate_forces_result_from_config(
          config, config_size, step_a, step_a_size, result_bytes,
          sizeof(result_bytes), &result_size);
  assert_int_equal(force_named.ok, 1);
  assert_int_equal(result_size, expected_force_size);
  assert_potential_result(result_bytes, result_size, -1.0,
                          hartree_angstrom_forces, 6, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_energy_grad_calls, 1);

  reset_embed_captures();
  result_size = 0;
  size_t expected_hessian_size =
      nwchemc_hessian_result_size_for_force_input(step_a, step_a_size);
  NWChemCResult hessian_result =
      nwchemc_calculate_hessian_result_from_config(
          config, config_size, step_a, step_a_size, result_bytes,
          sizeof(result_bytes), &result_size);
  assert_int_equal(hessian_result.ok, 1);
  assert_close(hessian_result.energy_h, -1.125, 1.0e-12);
  assert_int_equal(result_size, expected_hessian_size);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_hessian_calls, 1);
  double expected_hessian[36];
  for (int i = 0; i < 36; ++i)
    expected_hessian[i] =
        (double)(i + 10) / (bohr_to_angstrom * bohr_to_angstrom);
  assert_potential_result_hessian(result_bytes, result_size, -1.125,
                                  expected_hessian, 36, 1.0e-10);

  reset_embed_captures();
  result_size = 0;
  size_t expected_dipole_size =
      nwchemc_dipole_result_size_for_force_input(step_a, step_a_size);
  NWChemCResult dipole_result =
      nwchemc_calculate_dipole_result_from_config(
          config, config_size, step_a, step_a_size, result_bytes,
          sizeof(result_bytes), &result_size);
  assert_int_equal(dipole_result.ok, 1);
  assert_int_equal(result_size, expected_dipole_size);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_dipole_calls, 1);
  const double expected_dipole[3] = {0.25, 0.5, 0.75};
  assert_potential_result_dipole(result_bytes, result_size, -1.25,
                                 expected_dipole, 1.0e-12);

  reset_embed_captures();
  result_size = 0;
  size_t expected_quadrupole_size =
      nwchemc_quadrupole_result_size_for_force_input(step_a, step_a_size);
  NWChemCResult quadrupole_result =
      nwchemc_calculate_quadrupole_result_from_config(
          config, config_size, step_a, step_a_size, result_bytes,
          sizeof(result_bytes), &result_size);
  assert_int_equal(quadrupole_result.ok, 1);
  assert_int_equal(result_size, expected_quadrupole_size);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_quadrupole_calls, 1);
  const double expected_quadrupole[6] = {0.125, 0.25, 0.375,
                                         0.5,   0.625, 0.75};
  assert_potential_result_quadrupole(result_bytes, result_size, -1.5,
                                     expected_quadrupole, 1.0e-12);

  reset_embed_captures();
  result_size = 0;
  size_t expected_stress_size =
      nwchemc_stress_result_size_for_force_input(step_a, step_a_size);
  NWChemCResult stress_result =
      nwchemc_calculate_stress_result_from_config(
          config, config_size, step_a, step_a_size, result_bytes,
          sizeof(result_bytes), &result_size);
  assert_int_equal(stress_result.ok, 1);
  assert_int_equal(result_size, expected_stress_size);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_stress_calls, 1);
  double expected_stress[9];
  for (int i = 0; i < 9; ++i) {
    double native_stress = 0.03125 * (double)(i + 1);
    expected_stress[i] =
        native_stress / (bohr_to_angstrom * bohr_to_angstrom *
                         bohr_to_angstrom);
  }
  assert_potential_result_stress(result_bytes, result_size, -2.0,
                                 expected_stress, 1.0e-12);

  reset_embed_captures();
  result_size = 0;
  size_t expected_optimize_size =
      nwchemc_optimize_result_size_for_force_input(step_a, step_a_size);
  NWChemCResult optimize_result =
      nwchemc_calculate_optimize_result_from_config(
          config, config_size, step_a, step_a_size, result_bytes,
          sizeof(result_bytes), &result_size);
  assert_int_equal(optimize_result.ok, 1);
  assert_int_equal(result_size, expected_optimize_size);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_optimize_calls, 1);
  const double expected_optimized_ang[6] = {0.01, 0.02, 0.03,
                                            0.04, 0.05, 0.8014};
  assert_potential_result_optimized(result_bytes, result_size, -1.75,
                                    expected_optimized_ang, 6, 1.0e-12);

  reset_embed_captures();
  result_size = 0;
  size_t expected_frequencies_size =
      nwchemc_frequencies_result_size_for_force_input(step_a, step_a_size);
  NWChemCResult frequencies_result =
      nwchemc_calculate_frequencies_result_from_config(
          config, config_size, step_a, step_a_size, result_bytes,
          sizeof(result_bytes), &result_size);
  assert_int_equal(frequencies_result.ok, 1);
  assert_close(frequencies_result.energy_h, -1.625, 1.0e-12);
  assert_int_equal(result_size, expected_frequencies_size);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_frequency_calls, 1);
  assert_int_equal(g_frequency_modes_calls, 1);
  assert_int_equal(g_frequency_detail_calls, 1);
  const double expected_frequencies[6] = {100.0, 101.0, 102.0,
                                          103.0, 104.0, 105.0};
  const double expected_intensities[6] = {0.01, 0.02, 0.03,
                                          0.04, 0.05, 0.06};
  const double expected_projected_frequencies[6] = {90.0, 91.0, 92.0,
                                                    93.0, 94.0, 95.0};
  const double expected_projected_intensities[6] = {0.02, 0.04, 0.06,
                                                    0.08, 0.10, 0.12};
  assert_potential_result_frequencies(result_bytes, result_size, -1.625,
                                      expected_frequencies,
                                      expected_intensities,
                                      expected_projected_frequencies,
                                      expected_projected_intensities, 6,
                                      1.0e-12);

  free(config);
  free(step_a);
  free(message);
}

static void test_calculate_hessian_and_dipole_one_shot_accept_force_input(
    void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_calculate_forces != NULL);
  assert_true(nwchemc_calculate_energy != NULL);
  assert_true(nwchemc_calculate_hessian != NULL);
  assert_true(nwchemc_calculate_dipole != NULL);
  assert_true(nwchemc_calculate_quadrupole != NULL);
  assert_true(nwchemc_calculate_optimize != NULL);
  size_t message_size = 0;
  size_t step_a_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  assert_non_null(message);
  assert_non_null(step_a);

  NWChemCResult energy_result =
      nwchemc_calculate_energy(message, message_size, step_a, step_a_size);
  assert_int_equal(energy_result.ok, 1);
  assert_close(energy_result.energy_h, -1.0, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_energy_only_calls, 1);
  assert_int_equal(g_energy_only_cell_calls, 1);
  assert_int_equal(g_energy_grad_calls, 0);
  assert_int_equal(g_call_n_atoms[0], 2);
  assert_int_equal(g_call_atomic_numbers[0][0], 1);
  assert_int_equal(g_call_atomic_numbers[0][1], 8);
  assert_close(g_call_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_call_has_cell[0], 1);

  reset_embed_captures();
  double forces[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult force_result = nwchemc_calculate_forces(
      message, message_size, step_a, step_a_size, forces, 6);
  assert_int_equal(force_result.ok, 1);
  assert_close(force_result.energy_h, -1.0, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_energy_grad_calls, 1);
  assert_int_equal(g_call_n_atoms[0], 2);
  assert_int_equal(g_call_atomic_numbers[0][0], 1);
  assert_int_equal(g_call_atomic_numbers[0][1], 8);
  assert_close(g_call_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_call_has_cell[0], 1);
  assert_close(forces[0], -1.0, 1.0e-12);
  assert_close(forces[5], -6.0, 1.0e-12);

  reset_embed_captures();
  NWChemCResult short_forces = nwchemc_calculate_forces(
      message, message_size, step_a, step_a_size, forces, 5);
  assert_int_equal(short_forces.ok, 0);
  assert_int_equal(g_set_config_calls, 0);
  assert_int_equal(g_energy_grad_calls, 0);

  reset_embed_captures();
  double hessian[36] = {0.0};
  NWChemCResult hessian_result = nwchemc_calculate_hessian(
      message, message_size, step_a, step_a_size, hessian, 36);
  assert_int_equal(hessian_result.ok, 1);
  assert_close(hessian_result.energy_h, -1.125, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_hessian_calls, 1);
  assert_int_equal(g_hessian_cell_calls, 1);
  assert_int_equal(g_hessian_n_atoms[0], 2);
  assert_int_equal(g_hessian_atomic_numbers[0][0], 1);
  assert_int_equal(g_hessian_atomic_numbers[0][1], 8);
  assert_close(g_hessian_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_hessian_has_cell[0], 1);
  assert_close(hessian[0], 10.0, 1.0e-12);
  assert_close(hessian[35], 45.0, 1.0e-12);

  reset_embed_captures();
  NWChemCResult short_hessian = nwchemc_calculate_hessian(
      message, message_size, step_a, step_a_size, hessian, 35);
  assert_int_equal(short_hessian.ok, 0);
  assert_int_equal(g_set_config_calls, 0);
  assert_int_equal(g_hessian_calls, 0);

  reset_embed_captures();
  double dipole[3] = {0.0, 0.0, 0.0};
  NWChemCResult dipole_result = nwchemc_calculate_dipole(
      message, message_size, step_a, step_a_size, dipole, 3);
  assert_int_equal(dipole_result.ok, 1);
  assert_close(dipole_result.energy_h, -1.25, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_dipole_calls, 1);
  assert_int_equal(g_dipole_cell_calls, 1);
  assert_int_equal(g_dipole_n_atoms[0], 2);
  assert_int_equal(g_dipole_atomic_numbers[0][0], 1);
  assert_int_equal(g_dipole_atomic_numbers[0][1], 8);
  assert_close(g_dipole_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_dipole_has_cell[0], 1);
  assert_close(dipole[0], 0.25, 1.0e-12);
  assert_close(dipole[1], 0.5, 1.0e-12);
  assert_close(dipole[2], 0.75, 1.0e-12);

  reset_embed_captures();
  NWChemCResult short_dipole = nwchemc_calculate_dipole(
      message, message_size, step_a, step_a_size, dipole, 2);
  assert_int_equal(short_dipole.ok, 0);
  assert_int_equal(g_set_config_calls, 0);
  assert_int_equal(g_dipole_calls, 0);

  reset_embed_captures();
  double quadrupole[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult quadrupole_result = nwchemc_calculate_quadrupole(
      message, message_size, step_a, step_a_size, quadrupole, 6);
  assert_int_equal(quadrupole_result.ok, 1);
  assert_close(quadrupole_result.energy_h, -1.5, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_quadrupole_calls, 1);
  assert_int_equal(g_quadrupole_cell_calls, 1);
  assert_int_equal(g_quadrupole_n_atoms[0], 2);
  assert_int_equal(g_quadrupole_atomic_numbers[0][0], 1);
  assert_int_equal(g_quadrupole_atomic_numbers[0][1], 8);
  assert_close(g_quadrupole_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_quadrupole_has_cell[0], 1);
  assert_close(quadrupole[0], 0.125, 1.0e-12);
  assert_close(quadrupole[5], 0.75, 1.0e-12);

  reset_embed_captures();
  NWChemCResult short_quadrupole = nwchemc_calculate_quadrupole(
      message, message_size, step_a, step_a_size, quadrupole, 5);
  assert_int_equal(short_quadrupole.ok, 0);
  assert_int_equal(g_set_config_calls, 0);
  assert_int_equal(g_quadrupole_calls, 0);

  reset_embed_captures();
  assert_true(nwchemc_calculate_stress != NULL);
  double stress[9] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult stress_result = nwchemc_calculate_stress(
      message, message_size, step_a, step_a_size, stress, 9);
  assert_int_equal(stress_result.ok, 1);
  assert_close(stress_result.energy_h, -2.0, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_stress_calls, 1);
  assert_int_equal(g_stress_cell_calls, 1);
  assert_int_equal(g_stress_n_atoms[0], 2);
  assert_int_equal(g_stress_atomic_numbers[0][0], 1);
  assert_int_equal(g_stress_atomic_numbers[0][1], 8);
  assert_close(g_stress_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_stress_has_cell[0], 1);
  assert_close(stress[0], 0.03125, 1.0e-12);
  assert_close(stress[8], 0.28125, 1.0e-12);

  reset_embed_captures();
  NWChemCResult short_stress = nwchemc_calculate_stress(
      message, message_size, step_a, step_a_size, stress, 8);
  assert_int_equal(short_stress.ok, 0);
  assert_int_equal(g_set_config_calls, 0);
  assert_int_equal(g_stress_calls, 0);

  reset_embed_captures();
  double optimized_positions[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult optimize_result = nwchemc_calculate_optimize(
      message, message_size, step_a, step_a_size, optimized_positions, 6);
  assert_int_equal(optimize_result.ok, 1);
  assert_close(optimize_result.energy_h, -1.75, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_optimize_calls, 1);
  assert_int_equal(g_optimize_cell_calls, 1);
  assert_int_equal(g_optimize_n_atoms[0], 2);
  assert_int_equal(g_optimize_atomic_numbers[0][0], 1);
  assert_int_equal(g_optimize_atomic_numbers[0][1], 8);
  assert_close(g_optimize_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_optimize_has_cell[0], 1);
  assert_close(optimized_positions[0], 0.01, 1.0e-12);
  assert_close(optimized_positions[5], 0.8014, 1.0e-12);

  reset_embed_captures();
  NWChemCResult short_optimize = nwchemc_calculate_optimize(
      message, message_size, step_a, step_a_size, optimized_positions, 5);
  assert_int_equal(short_optimize.ok, 0);
  assert_int_equal(g_set_config_calls, 0);
  assert_int_equal(g_optimize_calls, 0);

  free(step_a);
  free(message);
}

static void test_calculate_frequencies_one_shot_accepts_force_input(
    void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_calculate_frequencies != NULL);
  size_t message_size = 0;
  size_t step_a_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  assert_non_null(message);
  assert_non_null(step_a);

  double frequencies[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  double intensities[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult frequency_result = nwchemc_calculate_frequencies(
      message, message_size, step_a, step_a_size, frequencies, 6, intensities,
      6);
  assert_int_equal(frequency_result.ok, 1);
  assert_close(frequency_result.energy_h, -1.625, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_frequency_calls, 1);
  assert_int_equal(g_frequency_cell_calls, 1);
  assert_int_equal(g_frequency_n_atoms[0], 2);
  assert_int_equal(g_frequency_atomic_numbers[0][0], 1);
  assert_int_equal(g_frequency_atomic_numbers[0][1], 8);
  assert_close(g_frequency_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_frequency_has_cell[0], 1);
  assert_close(frequencies[0], 100.0, 1.0e-12);
  assert_close(frequencies[5], 105.0, 1.0e-12);
  assert_close(intensities[0], 0.01, 1.0e-12);
  assert_close(intensities[5], 0.06, 1.0e-12);

  reset_embed_captures();
  NWChemCResult short_frequency = nwchemc_calculate_frequencies(
      message, message_size, step_a, step_a_size, frequencies, 5, intensities,
      6);
  assert_int_equal(short_frequency.ok, 0);
  assert_int_equal(g_set_config_calls, 0);
  assert_int_equal(g_frequency_calls, 0);

  reset_embed_captures();
  NWChemCResult short_intensity = nwchemc_calculate_frequencies(
      message, message_size, step_a, step_a_size, frequencies, 6, intensities,
      5);
  assert_int_equal(short_intensity.ok, 0);
  assert_int_equal(g_set_config_calls, 0);
  assert_int_equal(g_frequency_calls, 0);

  free(step_a);
  free(message);
}

static void test_calculate_config_raw_one_shot_accepts_force_input(
    void **state) {
  (void)state;
  reset_embed_captures();
  assert_true(nwchemc_calculate_energy_from_config != NULL);
  assert_true(nwchemc_calculate_forces_from_config != NULL);
  assert_true(nwchemc_calculate_hessian_from_config != NULL);
  assert_true(nwchemc_calculate_dipole_from_config != NULL);
  assert_true(nwchemc_calculate_quadrupole_from_config != NULL);
  assert_true(nwchemc_calculate_stress_from_config != NULL);
  assert_true(nwchemc_calculate_optimize_from_config != NULL);
  assert_true(nwchemc_calculate_frequencies_from_config != NULL);

  size_t message_size = 0;
  size_t step_a_size = 0;
  size_t config_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  unsigned char *step_a = read_file(g_force_step_a_path, &step_a_size);
  assert_non_null(message);
  assert_non_null(step_a);
  unsigned char *config = wrap_params_in_config(message, message_size,
                                                &config_size);
  assert_non_null(config);

  NWChemCResult energy_result =
      nwchemc_calculate_energy_from_config(config, config_size, step_a,
                                           step_a_size);
  assert_int_equal(energy_result.ok, 1);
  assert_close(energy_result.energy_h, -1.0, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_energy_only_calls, 1);
  assert_int_equal(g_energy_only_cell_calls, 1);
  assert_int_equal(g_call_n_atoms[0], 2);
  assert_int_equal(g_call_atomic_numbers[0][0], 1);
  assert_int_equal(g_call_atomic_numbers[0][1], 8);
  assert_close(g_call_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_call_has_cell[0], 1);

  reset_embed_captures();
  double forces[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult force_result = nwchemc_calculate_forces_from_config(
      config, config_size, step_a, step_a_size, forces, 6);
  assert_int_equal(force_result.ok, 1);
  assert_close(force_result.energy_h, -1.0, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_energy_grad_calls, 1);
  assert_int_equal(g_call_n_atoms[0], 2);
  assert_int_equal(g_call_atomic_numbers[0][0], 1);
  assert_int_equal(g_call_atomic_numbers[0][1], 8);
  assert_close(g_call_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_call_has_cell[0], 1);
  assert_close(forces[0], -1.0, 1.0e-12);
  assert_close(forces[5], -6.0, 1.0e-12);

  reset_embed_captures();
  NWChemCResult short_forces = nwchemc_calculate_forces_from_config(
      config, config_size, step_a, step_a_size, forces, 5);
  assert_int_equal(short_forces.ok, 0);
  assert_int_equal(g_set_config_calls, 0);
  assert_int_equal(g_energy_grad_calls, 0);

  reset_embed_captures();
  double hessian[36] = {0.0};
  NWChemCResult hessian_result = nwchemc_calculate_hessian_from_config(
      config, config_size, step_a, step_a_size, hessian, 36);
  assert_int_equal(hessian_result.ok, 1);
  assert_close(hessian_result.energy_h, -1.125, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_hessian_calls, 1);
  assert_int_equal(g_hessian_cell_calls, 1);
  assert_int_equal(g_hessian_n_atoms[0], 2);
  assert_int_equal(g_hessian_atomic_numbers[0][0], 1);
  assert_int_equal(g_hessian_atomic_numbers[0][1], 8);
  assert_close(g_hessian_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_hessian_has_cell[0], 1);
  assert_close(hessian[0], 10.0, 1.0e-12);
  assert_close(hessian[35], 45.0, 1.0e-12);

  reset_embed_captures();
  NWChemCResult short_hessian = nwchemc_calculate_hessian_from_config(
      config, config_size, step_a, step_a_size, hessian, 35);
  assert_int_equal(short_hessian.ok, 0);
  assert_int_equal(g_set_config_calls, 0);
  assert_int_equal(g_hessian_calls, 0);

  reset_embed_captures();
  double dipole[3] = {0.0, 0.0, 0.0};
  NWChemCResult dipole_result = nwchemc_calculate_dipole_from_config(
      config, config_size, step_a, step_a_size, dipole, 3);
  assert_int_equal(dipole_result.ok, 1);
  assert_close(dipole_result.energy_h, -1.25, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_dipole_calls, 1);
  assert_int_equal(g_dipole_cell_calls, 1);
  assert_int_equal(g_dipole_n_atoms[0], 2);
  assert_int_equal(g_dipole_atomic_numbers[0][0], 1);
  assert_int_equal(g_dipole_atomic_numbers[0][1], 8);
  assert_close(g_dipole_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_dipole_has_cell[0], 1);
  assert_close(dipole[0], 0.25, 1.0e-12);
  assert_close(dipole[1], 0.5, 1.0e-12);
  assert_close(dipole[2], 0.75, 1.0e-12);

  reset_embed_captures();
  double quadrupole[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult quadrupole_result =
      nwchemc_calculate_quadrupole_from_config(
          config, config_size, step_a, step_a_size, quadrupole, 6);
  assert_int_equal(quadrupole_result.ok, 1);
  assert_close(quadrupole_result.energy_h, -1.5, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_quadrupole_calls, 1);
  assert_int_equal(g_quadrupole_cell_calls, 1);
  assert_int_equal(g_quadrupole_n_atoms[0], 2);
  assert_int_equal(g_quadrupole_atomic_numbers[0][0], 1);
  assert_int_equal(g_quadrupole_atomic_numbers[0][1], 8);
  assert_close(g_quadrupole_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_quadrupole_has_cell[0], 1);
  assert_close(quadrupole[0], 0.125, 1.0e-12);
  assert_close(quadrupole[5], 0.75, 1.0e-12);

  reset_embed_captures();
  double stress[9] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult stress_result = nwchemc_calculate_stress_from_config(
      config, config_size, step_a, step_a_size, stress, 9);
  assert_int_equal(stress_result.ok, 1);
  assert_close(stress_result.energy_h, -2.0, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_stress_calls, 1);
  assert_int_equal(g_stress_cell_calls, 1);
  assert_int_equal(g_stress_n_atoms[0], 2);
  assert_int_equal(g_stress_atomic_numbers[0][0], 1);
  assert_int_equal(g_stress_atomic_numbers[0][1], 8);
  assert_close(g_stress_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_stress_has_cell[0], 1);
  assert_close(stress[0], 0.03125, 1.0e-12);
  assert_close(stress[8], 0.28125, 1.0e-12);

  reset_embed_captures();
  double optimized_positions[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult optimize_result = nwchemc_calculate_optimize_from_config(
      config, config_size, step_a, step_a_size, optimized_positions, 6);
  assert_int_equal(optimize_result.ok, 1);
  assert_close(optimize_result.energy_h, -1.75, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_optimize_calls, 1);
  assert_int_equal(g_optimize_cell_calls, 1);
  assert_int_equal(g_optimize_n_atoms[0], 2);
  assert_int_equal(g_optimize_atomic_numbers[0][0], 1);
  assert_int_equal(g_optimize_atomic_numbers[0][1], 8);
  assert_close(g_optimize_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_optimize_has_cell[0], 1);
  assert_close(optimized_positions[0], 0.01, 1.0e-12);
  assert_close(optimized_positions[5], 0.8014, 1.0e-12);

  reset_embed_captures();
  double frequencies[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  double intensities[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  NWChemCResult frequency_result = nwchemc_calculate_frequencies_from_config(
      config, config_size, step_a, step_a_size, frequencies, 6, intensities,
      6);
  assert_int_equal(frequency_result.ok, 1);
  assert_close(frequency_result.energy_h, -1.625, 1.0e-12);
  assert_int_equal(g_set_config_calls, 1);
  assert_int_equal(g_frequency_calls, 1);
  assert_int_equal(g_frequency_cell_calls, 1);
  assert_int_equal(g_frequency_n_atoms[0], 2);
  assert_int_equal(g_frequency_atomic_numbers[0][0], 1);
  assert_int_equal(g_frequency_atomic_numbers[0][1], 8);
  assert_close(g_frequency_positions_ang[0][5], 0.7414, 1.0e-12);
  assert_int_equal(g_frequency_has_cell[0], 1);
  assert_close(frequencies[0], 100.0, 1.0e-12);
  assert_close(frequencies[5], 105.0, 1.0e-12);
  assert_close(intensities[0], 0.01, 1.0e-12);
  assert_close(intensities[5], 0.06, 1.0e-12);

  reset_embed_captures();
  NWChemCResult short_frequency = nwchemc_calculate_frequencies_from_config(
      config, config_size, step_a, step_a_size, frequencies, 5, intensities,
      6);
  assert_int_equal(short_frequency.ok, 0);
  assert_int_equal(g_set_config_calls, 0);
  assert_int_equal(g_frequency_calls, 0);

  free(config);
  free(step_a);
  free(message);
}

int main(int argc, char **argv) {
  if (argc != 38) {
    fprintf(stderr,
            "usage: %s PARAMS_BIN CONFIG_OPTIONS_BIN PSPSPIN_PARAMS_BIN "
            "PSPSPIN_MANY_PARAMS_BIN NWPW_SPIN_MODE_PARAMS_BIN "
            "NWPW_ALLOW_TRANSLATION_PARAMS_BIN NWPW_CUTOFF_ALIAS_PARAMS_BIN "
            "NWPW_MC_STEPS_PARAMS_BIN NWPW_BO_STEPS_DEFAULT_PARAMS_BIN "
            "NWPW_BO_TIME_STEP_DEFAULT_PARAMS_BIN "
            "NWPW_BO_FAKE_MASS_DEFAULT_PARAMS_BIN "
            "NWPW_SCALING_DEFAULT_PARAMS_BIN "
            "NWPW_NP_DIMENSIONS_DEFAULT_PARAMS_BIN "
            "NWPW_TOLERANCES_DEFAULT_PARAMS_BIN "
            "NWPW_MC_STEPS_DEFAULT_PARAMS_BIN "
            "BRILLOUIN_TETRAHEDRON_PARAMS_BIN "
            "BRILLOUIN_DOS_GRID_PARAMS_BIN NWPW_ET_PARAMS_BIN "
            "NWPW_TEMPERATURE_PARAMS_BIN NWPW_DOS_DEFAULT_PARAMS_BIN "
            "NWPW_MAPPING_ALIAS_PARAMS_BIN "
            "NWPW_MAPPING_DEFAULT_PARAMS_BIN "
            "NWPW_VIRTUAL_ALIAS_PARAMS_BIN "
            "NWPW_ONE_ELECTRON_GUESS_DEFAULTS_PARAMS_BIN "
            "NWPW_FRACTIONAL_ORBITALS_DEFAULT_PARAMS_BIN "
            "NWPW_SMEAR_ORBITALS_DEFAULT_PARAMS_BIN "
            "NWPW_VIRTUAL_ORBITALS_DEFAULT_PARAMS_BIN "
            "FORCE_STEP_A_BIN FORCE_STEP_B_BIN FORCE_STEP_EV_BIN "
            "FORCE_STEP_CHANGED_SPECIES_BIN "
            "FORCE_STEP_STATE_BIN TCE_METHODS_BIN COMPACT_CELLS_BIN "
            "NWPW_TRANSLATE_VECTOR_DEFAULT_PARAMS_BIN "
            "BRILLOUIN_MONKHORST_DEFAULT_PARAMS_BIN "
            "BRILLOUIN_DOS_GRID_DEFAULT_PARAMS_BIN\n",
            argv[0]);
    return 2;
  }
  g_params_path = argv[1];
  g_config_options_path = argv[2];
  g_pspspin_path = argv[3];
  g_pspspin_many_path = argv[4];
  g_nwpw_spin_mode_path = argv[5];
  g_nwpw_allow_translation_path = argv[6];
  g_nwpw_cutoff_alias_path = argv[7];
  g_nwpw_mc_steps_path = argv[8];
  g_nwpw_bo_steps_default_path = argv[9];
  g_nwpw_bo_time_step_default_path = argv[10];
  g_nwpw_bo_fake_mass_default_path = argv[11];
  g_nwpw_scaling_default_path = argv[12];
  g_nwpw_np_dimensions_default_path = argv[13];
  g_nwpw_tolerances_default_path = argv[14];
  g_nwpw_mc_steps_default_path = argv[15];
  g_brillouin_tetrahedron_path = argv[16];
  g_brillouin_dos_grid_path = argv[17];
  g_nwpw_et_path = argv[18];
  g_nwpw_temperature_path = argv[19];
  g_nwpw_dos_default_path = argv[20];
  g_nwpw_mapping_alias_path = argv[21];
  g_nwpw_mapping_default_path = argv[22];
  g_nwpw_virtual_alias_path = argv[23];
  g_nwpw_one_electron_guess_defaults_path = argv[24];
  g_nwpw_fractional_orbitals_default_path = argv[25];
  g_nwpw_smear_orbitals_default_path = argv[26];
  g_nwpw_virtual_orbitals_default_path = argv[27];
  g_force_step_a_path = argv[28];
  g_force_step_b_path = argv[29];
  g_force_step_ev_path = argv[30];
  g_force_step_changed_species_path = argv[31];
  g_force_step_state_path = argv[32];
  g_tce_methods_path = argv[33];
  g_compact_cells_path = argv[34];
  g_nwpw_translate_vector_default_path = argv[35];
  g_brillouin_monkhorst_default_path = argv[36];
  g_brillouin_dos_grid_default_path = argv[37];
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(
          test_embed_promotes_typed_scf_wf_nopen_and_dft_iterations),
      cmocka_unit_test(test_embed_promotes_typed_scf_semidirect_int2e),
      cmocka_unit_test(test_embed_config_uses_direct_dft_values),
      cmocka_unit_test(test_embed_config_promotes_compact_simulation_cells),
      cmocka_unit_test(test_embed_config_promotes_tce_method_tokens),
      cmocka_unit_test(test_embed_config_uses_direct_scf_values),
      cmocka_unit_test(test_configure_accepts_potential_config_nwchem),
      cmocka_unit_test(test_engine_path_is_rejected_before_embed_config),
      cmocka_unit_test(test_session_create_from_config_applies_nwchem),
      cmocka_unit_test(test_session_configure_replaces_before_topology),
      cmocka_unit_test(test_session_configure_rejects_after_topology),
      cmocka_unit_test(test_embed_config_promotes_pspspin_rules),
      cmocka_unit_test(test_embed_config_promotes_large_pspspin_ion_list),
      cmocka_unit_test(test_embed_config_promotes_nwpw_spin_mode),
      cmocka_unit_test(test_embed_config_promotes_nwpw_allow_translation),
      cmocka_unit_test(test_embed_config_promotes_nwpw_cutoff_alias),
      cmocka_unit_test(test_embed_config_promotes_nwpw_mc_steps),
      cmocka_unit_test(test_embed_config_promotes_nwpw_bo_steps_default),
      cmocka_unit_test(
          test_embed_config_promotes_nwpw_bo_time_step_default),
      cmocka_unit_test(
          test_embed_config_promotes_nwpw_bo_fake_mass_default),
      cmocka_unit_test(test_embed_config_promotes_nwpw_scaling_default),
      cmocka_unit_test(
          test_embed_config_promotes_nwpw_np_dimensions_default),
      cmocka_unit_test(test_embed_config_promotes_nwpw_tolerances_default),
      cmocka_unit_test(test_embed_config_promotes_nwpw_mc_steps_default),
      cmocka_unit_test(test_embed_config_promotes_brillouin_tetrahedron),
      cmocka_unit_test(test_embed_config_promotes_brillouin_dos_grid),
      cmocka_unit_test(test_embed_config_promotes_brillouin_dos_grid_default),
      cmocka_unit_test(test_embed_config_promotes_nwpw_et),
      cmocka_unit_test(test_embed_config_promotes_nwpw_temperature),
      cmocka_unit_test(test_embed_config_promotes_nwpw_dos_default),
      cmocka_unit_test(test_embed_config_promotes_nwpw_mapping_alias),
      cmocka_unit_test(test_embed_config_promotes_nwpw_mapping_default),
      cmocka_unit_test(test_embed_config_promotes_nwpw_virtual_alias),
      cmocka_unit_test(
          test_embed_config_promotes_nwpw_one_electron_guess_defaults),
      cmocka_unit_test(
          test_embed_config_promotes_nwpw_fractional_orbitals_default),
      cmocka_unit_test(
          test_embed_config_promotes_nwpw_smear_orbitals_default),
      cmocka_unit_test(
          test_embed_config_promotes_nwpw_virtual_orbitals_default),
      cmocka_unit_test(
          test_embed_config_promotes_nwpw_translate_vector_default),
      cmocka_unit_test(
          test_embed_config_promotes_brillouin_monkhorst_default),
      cmocka_unit_test(test_session_reuses_config_across_geometry_steps),
      cmocka_unit_test(test_session_reapplies_after_one_shot_config),
      cmocka_unit_test(test_session_set_params_replaces_before_topology),
      cmocka_unit_test(test_session_rejects_param_replacement_after_topology),
      cmocka_unit_test(test_session_calculate_forces_accepts_force_input_steps),
      cmocka_unit_test(test_session_reset_topology_allows_changed_species),
      cmocka_unit_test(test_session_calculate_hessian_accepts_force_input_step),
      cmocka_unit_test(test_session_calculate_dipole_accepts_force_input_step),
      cmocka_unit_test(
          test_session_calculate_quadrupole_accepts_force_input_step),
      cmocka_unit_test(test_session_calculate_stress_accepts_force_input_step),
      cmocka_unit_test(test_session_calculate_optimize_accepts_force_input_step),
      cmocka_unit_test(
          test_session_calculate_frequencies_accepts_force_input_step),
      cmocka_unit_test(
          test_session_calculate_frequency_detail_accepts_force_input_step),
      cmocka_unit_test(test_direct_coordinate_energy_abi_calls_embed_wrappers),
      cmocka_unit_test(
          test_direct_coordinate_config_energy_abi_calls_embed_wrappers),
      cmocka_unit_test(test_session_coordinate_energy_abi_calls_embed_wrappers),
      cmocka_unit_test(test_direct_coordinate_abi_calls_embed_wrappers),
      cmocka_unit_test(test_direct_coordinate_config_abi_calls_embed_wrappers),
      cmocka_unit_test(test_session_coordinate_abi_calls_embed_wrappers),
      cmocka_unit_test(
          test_direct_coordinate_property_stress_abi_calls_embed_wrappers),
      cmocka_unit_test(
          test_direct_coordinate_config_property_stress_abi_calls_embed_wrappers),
      cmocka_unit_test(
          test_session_coordinate_property_stress_abi_calls_embed_wrappers),
      cmocka_unit_test(test_session_force_input_state_overrides_params),
      cmocka_unit_test(test_session_calculate_result_writes_potential_result),
      cmocka_unit_test(
          test_session_calculate_energy_result_writes_potential_result),
      cmocka_unit_test(
          test_session_calculate_hessian_result_writes_potential_result),
      cmocka_unit_test(
          test_session_calculate_property_results_write_potential_result),
      cmocka_unit_test(
          test_session_calculate_stress_result_writes_potential_result),
      cmocka_unit_test(
          test_session_calculate_structural_results_write_potential_result),
      cmocka_unit_test(test_calculate_result_one_shot_writes_potential_result),
      cmocka_unit_test(
          test_calculate_hessian_result_one_shot_writes_potential_result),
      cmocka_unit_test(
          test_calculate_property_results_one_shot_write_potential_result),
      cmocka_unit_test(
          test_calculate_stress_result_one_shot_writes_potential_result),
      cmocka_unit_test(
          test_calculate_structural_results_one_shot_write_potential_result),
      cmocka_unit_test(
          test_calculate_config_results_one_shot_write_potential_results),
      cmocka_unit_test(
          test_calculate_hessian_and_dipole_one_shot_accept_force_input),
      cmocka_unit_test(test_calculate_frequencies_one_shot_accepts_force_input),
      cmocka_unit_test(
          test_calculate_config_raw_one_shot_accepts_force_input),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
