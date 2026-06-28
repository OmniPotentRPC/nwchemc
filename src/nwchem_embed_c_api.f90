! SPDX-License-Identifier: MIT
!
! nwchem_embed_c_api.f90 - compiler-independent C ABI for NWChem embed.
!
! Public symbols are fixed with bind(C, name=...) (iso_c_binding); no
! compiler-specific underscore mangling. NWChem F77/rtdb calls live in
! nwchem_embed_legacy.F and are invoked via private interfaces.
!
! Style: modern free-form Fortran (iso_fortran_env + iso_c_binding).

module nwchem_embed_c_api
  use, intrinsic :: iso_c_binding, only: &
      c_char, c_double, c_int, c_long_long, c_null_char
  use, intrinsic :: iso_fortran_env, only: real64
  implicit none
  private

  ! C ABI (stable names - match nwchem_c_abi.c extern declarations)
  public :: nwchemc_embed_init
  public :: nwchemc_embed_available
  public :: nwchemc_embed_last_energy
  public :: nwchemc_embed_current_rtdb
  public :: nwchemc_embed_reset_rtdb
  public :: nwchemc_embed_set_config
  public :: nwchemc_embed_set_dft_direct
  public :: nwchemc_embed_set_scf_direct
  public :: nwchemc_embed_set_driver_direct
  public :: nwchemc_embed_set_nwpw_direct
  public :: nwchemc_embed_set_brillouin_zone
  public :: nwchemc_embed_set_brillouin_dos_zones
  public :: nwchemc_embed_set_pseudopotentials
  public :: nwchemc_embed_set_rtdb_strings
  public :: nwchemc_embed_set_rtdb_values
  public :: nwchemc_embed_energy_only
  public :: nwchemc_embed_energy_only_cell
  public :: nwchemc_embed_energy_grad
  public :: nwchemc_embed_energy_grad_cell
  public :: nwchemc_embed_hessian
  public :: nwchemc_embed_hessian_cell
  public :: nwchemc_embed_dipole
  public :: nwchemc_embed_dipole_cell
  public :: nwchemc_embed_polarizability
  public :: nwchemc_embed_polarizability_cell
  public :: nwchemc_embed_quadrupole
  public :: nwchemc_embed_quadrupole_cell
  public :: nwchemc_embed_stress
  public :: nwchemc_embed_stress_cell
  public :: nwchemc_embed_optimize
  public :: nwchemc_embed_optimize_cell
  public :: nwchemc_embed_frequencies
  public :: nwchemc_embed_frequencies_cell
  public :: nwchemc_embed_frequencies_modes
  public :: nwchemc_embed_frequencies_modes_cell
  public :: nwchemc_embed_frequencies_detail_cell
  public :: nwchemc_embed_finalize

  ! Module state (saved across C calls).
  ! Note: NWChem RTDB handles may be 0 on success; never use handle==0 as failure.
  logical, save :: ga_ready = .false.
  logical, save :: rtdb_ready = .false.
  logical, save :: owns_mpi_runtime = .false.
  logical, save :: runtime_finalized = .false.
  integer, save :: rtdb_handle = -1
  real(real64), save :: last_task_energy_h = 0.0_real64
  character(len=64), save :: cfg_basis = 'sto-3g'
  character(len=64), save :: cfg_theory = 'scf'
  character(len=64), save :: cfg_scf = 'rhf'
  character(len=4096), save :: cfg_input_blocks = ' '
  integer, save :: cfg_charge = 0
  integer, save :: cfg_mult = 1
  ! Direct DFT knobs (Cap'n Proto stanza; applied via RTDB in legacy eval path).
  character(len=64), save :: cfg_dft_xc = ' '
  integer, save :: cfg_dft_direct = 0
  integer, save :: cfg_dft_smear_on = 0
  real(real64), save :: cfg_dft_smear_sigma = 0.0_real64
  integer, save :: cfg_dft_smear_spinset = 1
  integer, save :: cfg_scf_has_options = 0
  integer, save :: cfg_scf_maxiter = 0
  real(real64), save :: cfg_scf_thresh = 0.0_real64
  real(real64), save :: cfg_scf_tol2e = 0.0_real64
  integer, save :: cfg_driver_has_options = 0
  integer, save :: cfg_driver_maxiter = 0
  integer, save :: cfg_driver_tolerance_mode = 0
  real(real64), save :: cfg_driver_gmax_tol = 0.0_real64
  real(real64), save :: cfg_driver_grms_tol = 0.0_real64
  real(real64), save :: cfg_driver_xmax_tol = 0.0_real64
  real(real64), save :: cfg_driver_xrms_tol = 0.0_real64
  integer, parameter :: max_embed_pseudopotentials = 64
  integer, parameter :: psp_element_len = 16
  integer, parameter :: psp_name_len = 256
  integer, parameter :: max_embed_set_strings = 512
  integer, parameter :: max_embed_set_values = 64
  integer, parameter :: set_key_len = 128
  integer, parameter :: set_value_len = 256
  integer, save :: cfg_psp_count = 0
  character(len=psp_element_len), save :: cfg_psp_elements(max_embed_pseudopotentials) = ' '
  character(len=psp_name_len), save :: cfg_psp_names(max_embed_pseudopotentials) = ' '
  integer, save :: cfg_psp_types(max_embed_pseudopotentials) = 0
  integer, save :: cfg_set_string_count = 0
  character(len=set_key_len), save :: cfg_set_keys(max_embed_set_strings) = ' '
  character(len=set_value_len), save :: cfg_set_values(max_embed_set_strings) = ' '
  integer, save :: cfg_typed_set_count = 0
  character(len=set_key_len), save :: cfg_typed_set_keys(max_embed_set_strings) = ' '
  integer, save :: cfg_typed_set_types(max_embed_set_strings) = 0
  integer, save :: cfg_typed_set_value_counts(max_embed_set_strings) = 0
  character(len=set_value_len), save :: cfg_typed_set_values(max_embed_set_values, max_embed_set_strings) = ' '
  integer, save :: cfg_brillouin_has_options = 0
  character(len=64), save :: cfg_brillouin_zone_name = 'zone_default'
  integer, save :: cfg_brillouin_monkhorst_pack(3) = 0
  integer, save :: cfg_brillouin_max_kpoints_print = 0
  integer, save :: cfg_brillouin_kvector_count = 0
  real(real64), allocatable, save :: cfg_brillouin_kvectors(:)
  integer, save :: cfg_brillouin_dos_zone_count = 0
  character(len=64), save :: &
      cfg_brillouin_dos_zone_names(max_embed_set_strings) = ' '
  integer, save :: &
      cfg_brillouin_dos_zone_grids(3, max_embed_set_strings) = 0

  ! Legacy NWChem helpers (fixed-form; no bind(C))
  interface
    subroutine nwchem_legacy_init(rtdb, ok, owns_mpi)
      integer, intent(out) :: rtdb
      integer, intent(out) :: ok
      integer, intent(out) :: owns_mpi
    end subroutine nwchem_legacy_init

    subroutine nwchem_legacy_finalize(rtdb, owns_mpi, ok)
      integer, intent(inout) :: rtdb
      integer, intent(in) :: owns_mpi
      integer, intent(out) :: ok
    end subroutine nwchem_legacy_finalize

    subroutine nwchemc_store_pseudopotentials(rtdb, psp_count, &
        psp_elements, psp_types, psp_names, errmsg, ok)
      integer, intent(in) :: rtdb
      integer, intent(in) :: psp_count
      character(len=16), intent(in) :: psp_elements(*)
      integer, intent(in) :: psp_types(*)
      character(len=256), intent(in) :: psp_names(*)
      character(len=*), intent(out) :: errmsg
      logical, intent(out) :: ok
    end subroutine nwchemc_store_pseudopotentials

    subroutine nwchemc_apply_direct_string_sets(rtdb, set_count, &
        set_keys, set_values, errmsg, ok)
      integer, intent(in) :: rtdb
      integer, intent(in) :: set_count
      character(len=128), intent(in) :: set_keys(*)
      character(len=256), intent(in) :: set_values(*)
      character(len=*), intent(out) :: errmsg
      logical, intent(out) :: ok
    end subroutine nwchemc_apply_direct_string_sets

    subroutine nwchemc_apply_direct_typed_sets(rtdb, set_count, &
        set_keys, set_types, set_value_counts, set_values, errmsg, ok)
      integer, intent(in) :: rtdb
      integer, intent(in) :: set_count
      character(len=128), intent(in) :: set_keys(*)
      integer, intent(in) :: set_types(*)
      integer, intent(in) :: set_value_counts(*)
      character(len=256), intent(in) :: set_values(64,*)
      character(len=*), intent(out) :: errmsg
      logical, intent(out) :: ok
    end subroutine nwchemc_apply_direct_typed_sets

    subroutine nwchemc_store_brillouin_dos_zones(rtdb, zone_count, &
        zone_names, zone_grids, errmsg, ok)
      integer, intent(in) :: rtdb
      integer, intent(in) :: zone_count
      character(len=64), intent(in) :: zone_names(*)
      integer, intent(in) :: zone_grids(3,*)
      character(len=*), intent(out) :: errmsg
      logical, intent(out) :: ok
    end subroutine nwchemc_store_brillouin_dos_zones

    subroutine nwchem_legacy_reset_rtdb(rtdb, ok)
      integer, intent(inout) :: rtdb
      integer, intent(out) :: ok
    end subroutine nwchem_legacy_reset_rtdb

    subroutine nwchem_legacy_energy_only(rtdb, n_atoms, pos_ang, atmnrs, &
        cell_ang, has_cell, basis_name, theory_name, scf_type, input_blocks, &
        charge, mult, dft_direct, dft_smear_on, dft_smear_sigma, &
        dft_smear_spinset, scf_has_options, scf_maxiter, scf_thresh, &
        scf_tol2e, driver_has_options, driver_maxiter, &
        driver_tolerance_mode, driver_gmax_tol, driver_grms_tol, &
        driver_xmax_tol, driver_xrms_tol, psp_count, psp_elements, &
        psp_types, psp_names, set_string_count, set_keys, set_values, &
        typed_set_count, typed_set_keys, typed_set_types, &
        typed_set_value_counts, typed_set_values, &
        brillouin_has_options, brillouin_zone_name, &
        brillouin_monkhorst_pack, brillouin_max_kpoints_print, &
        brillouin_kvector_count, brillouin_kvectors, energy_h, errmsg, ok)
      import :: real64
      integer, intent(in) :: rtdb, n_atoms
      real(real64), intent(in) :: pos_ang(*)
      integer, intent(in) :: atmnrs(*)
      real(real64), intent(in) :: cell_ang(*)
      integer, intent(in) :: has_cell
      character(len=64), intent(in) :: basis_name
      character(len=64), intent(in) :: theory_name, scf_type
      character(len=4096), intent(in) :: input_blocks
      integer, intent(in) :: charge, mult
      integer, intent(in) :: dft_direct, dft_smear_on, dft_smear_spinset
      integer, intent(in) :: scf_has_options, scf_maxiter
      integer, intent(in) :: driver_has_options, driver_maxiter
      integer, intent(in) :: driver_tolerance_mode
      real(real64), intent(in) :: driver_gmax_tol, driver_grms_tol
      real(real64), intent(in) :: driver_xmax_tol, driver_xrms_tol
      integer, intent(in) :: psp_count
      character(len=16), intent(in) :: psp_elements(*)
      integer, intent(in) :: psp_types(*)
      character(len=256), intent(in) :: psp_names(*)
      integer, intent(in) :: set_string_count
      character(len=128), intent(in) :: set_keys(*)
      character(len=256), intent(in) :: set_values(*)
      integer, intent(in) :: typed_set_count
      character(len=128), intent(in) :: typed_set_keys(*)
      integer, intent(in) :: typed_set_types(*)
      integer, intent(in) :: typed_set_value_counts(*)
      character(len=256), intent(in) :: typed_set_values(64,*)
      integer, intent(in) :: brillouin_has_options
      character(len=64), intent(in) :: brillouin_zone_name
      integer, intent(in) :: brillouin_monkhorst_pack(3)
      integer, intent(in) :: brillouin_max_kpoints_print
      integer, intent(in) :: brillouin_kvector_count
      real(real64), intent(in) :: brillouin_kvectors(*)
      real(real64), intent(in) :: dft_smear_sigma
      real(real64), intent(in) :: scf_thresh, scf_tol2e
      real(real64), intent(out) :: energy_h
      character(len=*), intent(out) :: errmsg
      integer, intent(out) :: ok
    end subroutine nwchem_legacy_energy_only

    subroutine nwchem_legacy_energy_grad(rtdb, n_atoms, pos_ang, atmnrs, &
        cell_ang, has_cell, basis_name, theory_name, scf_type, input_blocks, &
        charge, mult, dft_direct, dft_smear_on, dft_smear_sigma, &
        dft_smear_spinset, scf_has_options, scf_maxiter, scf_thresh, &
        scf_tol2e, driver_has_options, driver_maxiter, &
        driver_tolerance_mode, driver_gmax_tol, driver_grms_tol, &
        driver_xmax_tol, driver_xrms_tol, psp_count, psp_elements, &
        psp_types, psp_names, set_string_count, set_keys, set_values, &
        typed_set_count, typed_set_keys, typed_set_types, &
        typed_set_value_counts, typed_set_values, &
        brillouin_has_options, brillouin_zone_name, &
        brillouin_monkhorst_pack, brillouin_max_kpoints_print, &
        brillouin_kvector_count, brillouin_kvectors, &
        energy_h, grad_h_bohr, errmsg, ok)
      import :: real64
      integer, intent(in) :: rtdb, n_atoms
      real(real64), intent(in) :: pos_ang(*)
      integer, intent(in) :: atmnrs(*)
      real(real64), intent(in) :: cell_ang(*)
      integer, intent(in) :: has_cell
      character(len=64), intent(in) :: basis_name
      character(len=64), intent(in) :: theory_name, scf_type
      character(len=4096), intent(in) :: input_blocks
      integer, intent(in) :: charge, mult
      integer, intent(in) :: dft_direct, dft_smear_on, dft_smear_spinset
      integer, intent(in) :: scf_has_options, scf_maxiter
      integer, intent(in) :: driver_has_options, driver_maxiter
      integer, intent(in) :: driver_tolerance_mode
      real(real64), intent(in) :: driver_gmax_tol, driver_grms_tol
      real(real64), intent(in) :: driver_xmax_tol, driver_xrms_tol
      integer, intent(in) :: psp_count
      character(len=16), intent(in) :: psp_elements(*)
      integer, intent(in) :: psp_types(*)
      character(len=256), intent(in) :: psp_names(*)
      integer, intent(in) :: set_string_count
      character(len=128), intent(in) :: set_keys(*)
      character(len=256), intent(in) :: set_values(*)
      integer, intent(in) :: typed_set_count
      character(len=128), intent(in) :: typed_set_keys(*)
      integer, intent(in) :: typed_set_types(*)
      integer, intent(in) :: typed_set_value_counts(*)
      character(len=256), intent(in) :: typed_set_values(64,*)
      integer, intent(in) :: brillouin_has_options
      character(len=64), intent(in) :: brillouin_zone_name
      integer, intent(in) :: brillouin_monkhorst_pack(3)
      integer, intent(in) :: brillouin_max_kpoints_print
      integer, intent(in) :: brillouin_kvector_count
      real(real64), intent(in) :: brillouin_kvectors(*)
      real(real64), intent(in) :: dft_smear_sigma
      real(real64), intent(in) :: scf_thresh, scf_tol2e
      real(real64), intent(out) :: energy_h
      real(real64), intent(out) :: grad_h_bohr(*)
      character(len=*), intent(out) :: errmsg
      integer, intent(out) :: ok
    end subroutine nwchem_legacy_energy_grad

    subroutine nwchem_legacy_dipole(rtdb, n_atoms, pos_ang, atmnrs, &
        cell_ang, has_cell, basis_name, theory_name, scf_type, input_blocks, &
        charge, mult, dft_direct, dft_smear_on, dft_smear_sigma, &
        dft_smear_spinset, scf_has_options, scf_maxiter, scf_thresh, &
        scf_tol2e, driver_has_options, driver_maxiter, &
        driver_tolerance_mode, driver_gmax_tol, driver_grms_tol, &
        driver_xmax_tol, driver_xrms_tol, psp_count, psp_elements, &
        psp_types, psp_names, set_string_count, set_keys, set_values, &
        typed_set_count, typed_set_keys, typed_set_types, &
        typed_set_value_counts, typed_set_values, &
        brillouin_has_options, brillouin_zone_name, &
        brillouin_monkhorst_pack, brillouin_max_kpoints_print, &
        brillouin_kvector_count, brillouin_kvectors, &
        energy_h, dipole_au, errmsg, ok)
      import :: real64
      integer, intent(in) :: rtdb, n_atoms
      real(real64), intent(in) :: pos_ang(*)
      integer, intent(in) :: atmnrs(*)
      real(real64), intent(in) :: cell_ang(*)
      integer, intent(in) :: has_cell
      character(len=64), intent(in) :: basis_name
      character(len=64), intent(in) :: theory_name, scf_type
      character(len=4096), intent(in) :: input_blocks
      integer, intent(in) :: charge, mult
      integer, intent(in) :: dft_direct, dft_smear_on, dft_smear_spinset
      integer, intent(in) :: scf_has_options, scf_maxiter
      integer, intent(in) :: driver_has_options, driver_maxiter
      integer, intent(in) :: driver_tolerance_mode
      real(real64), intent(in) :: driver_gmax_tol, driver_grms_tol
      real(real64), intent(in) :: driver_xmax_tol, driver_xrms_tol
      integer, intent(in) :: psp_count
      character(len=16), intent(in) :: psp_elements(*)
      integer, intent(in) :: psp_types(*)
      character(len=256), intent(in) :: psp_names(*)
      integer, intent(in) :: set_string_count
      character(len=128), intent(in) :: set_keys(*)
      character(len=256), intent(in) :: set_values(*)
      integer, intent(in) :: typed_set_count
      character(len=128), intent(in) :: typed_set_keys(*)
      integer, intent(in) :: typed_set_types(*)
      integer, intent(in) :: typed_set_value_counts(*)
      character(len=256), intent(in) :: typed_set_values(64,*)
      integer, intent(in) :: brillouin_has_options
      character(len=64), intent(in) :: brillouin_zone_name
      integer, intent(in) :: brillouin_monkhorst_pack(3)
      integer, intent(in) :: brillouin_max_kpoints_print
      integer, intent(in) :: brillouin_kvector_count
      real(real64), intent(in) :: brillouin_kvectors(*)
      real(real64), intent(in) :: dft_smear_sigma
      real(real64), intent(in) :: scf_thresh, scf_tol2e
      real(real64), intent(out) :: energy_h
      real(real64), intent(out) :: dipole_au(*)
      character(len=*), intent(out) :: errmsg
      integer, intent(out) :: ok
    end subroutine nwchem_legacy_dipole

    subroutine nwchem_legacy_polarizability(rtdb, n_atoms, pos_ang, &
        atmnrs, cell_ang, has_cell, basis_name, theory_name, scf_type, &
        input_blocks, charge, mult, dft_direct, dft_smear_on, &
        dft_smear_sigma, dft_smear_spinset, scf_has_options, scf_maxiter, &
        scf_thresh, scf_tol2e, driver_has_options, driver_maxiter, &
        driver_tolerance_mode, driver_gmax_tol, driver_grms_tol, &
        driver_xmax_tol, driver_xrms_tol, psp_count, psp_elements, &
        psp_types, psp_names, set_string_count, set_keys, set_values, &
        typed_set_count, typed_set_keys, typed_set_types, &
        typed_set_value_counts, typed_set_values, &
        brillouin_has_options, brillouin_zone_name, &
        brillouin_monkhorst_pack, brillouin_max_kpoints_print, &
        brillouin_kvector_count, brillouin_kvectors, &
        energy_h, polarizability_au, errmsg, ok)
      import :: real64
      integer, intent(in) :: rtdb, n_atoms
      real(real64), intent(in) :: pos_ang(*)
      integer, intent(in) :: atmnrs(*)
      real(real64), intent(in) :: cell_ang(*)
      integer, intent(in) :: has_cell
      character(len=64), intent(in) :: basis_name
      character(len=64), intent(in) :: theory_name, scf_type
      character(len=4096), intent(in) :: input_blocks
      integer, intent(in) :: charge, mult
      integer, intent(in) :: dft_direct, dft_smear_on, dft_smear_spinset
      integer, intent(in) :: scf_has_options, scf_maxiter
      integer, intent(in) :: driver_has_options, driver_maxiter
      integer, intent(in) :: driver_tolerance_mode
      real(real64), intent(in) :: driver_gmax_tol, driver_grms_tol
      real(real64), intent(in) :: driver_xmax_tol, driver_xrms_tol
      integer, intent(in) :: psp_count
      character(len=16), intent(in) :: psp_elements(*)
      integer, intent(in) :: psp_types(*)
      character(len=256), intent(in) :: psp_names(*)
      integer, intent(in) :: set_string_count
      character(len=128), intent(in) :: set_keys(*)
      character(len=256), intent(in) :: set_values(*)
      integer, intent(in) :: typed_set_count
      character(len=128), intent(in) :: typed_set_keys(*)
      integer, intent(in) :: typed_set_types(*)
      integer, intent(in) :: typed_set_value_counts(*)
      character(len=256), intent(in) :: typed_set_values(64,*)
      integer, intent(in) :: brillouin_has_options
      character(len=64), intent(in) :: brillouin_zone_name
      integer, intent(in) :: brillouin_monkhorst_pack(3)
      integer, intent(in) :: brillouin_max_kpoints_print
      integer, intent(in) :: brillouin_kvector_count
      real(real64), intent(in) :: brillouin_kvectors(*)
      real(real64), intent(in) :: dft_smear_sigma
      real(real64), intent(in) :: scf_thresh, scf_tol2e
      real(real64), intent(out) :: energy_h
      real(real64), intent(out) :: polarizability_au(*)
      character(len=*), intent(out) :: errmsg
      integer, intent(out) :: ok
    end subroutine nwchem_legacy_polarizability

    subroutine nwchem_legacy_quadrupole(rtdb, n_atoms, pos_ang, atmnrs, &
        cell_ang, has_cell, basis_name, theory_name, scf_type, input_blocks, &
        charge, mult, dft_direct, dft_smear_on, dft_smear_sigma, &
        dft_smear_spinset, scf_has_options, scf_maxiter, scf_thresh, &
        scf_tol2e, driver_has_options, driver_maxiter, &
        driver_tolerance_mode, driver_gmax_tol, driver_grms_tol, &
        driver_xmax_tol, driver_xrms_tol, psp_count, psp_elements, &
        psp_types, psp_names, set_string_count, set_keys, set_values, &
        typed_set_count, typed_set_keys, typed_set_types, &
        typed_set_value_counts, typed_set_values, &
        brillouin_has_options, brillouin_zone_name, &
        brillouin_monkhorst_pack, brillouin_max_kpoints_print, &
        brillouin_kvector_count, brillouin_kvectors, &
        energy_h, quadrupole_au, errmsg, ok)
      import :: real64
      integer, intent(in) :: rtdb, n_atoms
      real(real64), intent(in) :: pos_ang(*)
      integer, intent(in) :: atmnrs(*)
      real(real64), intent(in) :: cell_ang(*)
      integer, intent(in) :: has_cell
      character(len=64), intent(in) :: basis_name
      character(len=64), intent(in) :: theory_name, scf_type
      character(len=4096), intent(in) :: input_blocks
      integer, intent(in) :: charge, mult
      integer, intent(in) :: dft_direct, dft_smear_on, dft_smear_spinset
      integer, intent(in) :: scf_has_options, scf_maxiter
      integer, intent(in) :: driver_has_options, driver_maxiter
      integer, intent(in) :: driver_tolerance_mode
      real(real64), intent(in) :: driver_gmax_tol, driver_grms_tol
      real(real64), intent(in) :: driver_xmax_tol, driver_xrms_tol
      integer, intent(in) :: psp_count
      character(len=16), intent(in) :: psp_elements(*)
      integer, intent(in) :: psp_types(*)
      character(len=256), intent(in) :: psp_names(*)
      integer, intent(in) :: set_string_count
      character(len=128), intent(in) :: set_keys(*)
      character(len=256), intent(in) :: set_values(*)
      integer, intent(in) :: typed_set_count
      character(len=128), intent(in) :: typed_set_keys(*)
      integer, intent(in) :: typed_set_types(*)
      integer, intent(in) :: typed_set_value_counts(*)
      character(len=256), intent(in) :: typed_set_values(64,*)
      integer, intent(in) :: brillouin_has_options
      character(len=64), intent(in) :: brillouin_zone_name
      integer, intent(in) :: brillouin_monkhorst_pack(3)
      integer, intent(in) :: brillouin_max_kpoints_print
      integer, intent(in) :: brillouin_kvector_count
      real(real64), intent(in) :: brillouin_kvectors(*)
      real(real64), intent(in) :: dft_smear_sigma
      real(real64), intent(in) :: scf_thresh, scf_tol2e
      real(real64), intent(out) :: energy_h
      real(real64), intent(out) :: quadrupole_au(*)
      character(len=*), intent(out) :: errmsg
      integer, intent(out) :: ok
    end subroutine nwchem_legacy_quadrupole

    subroutine nwchem_legacy_stress(rtdb, n_atoms, pos_ang, atmnrs, &
        cell_ang, has_cell, basis_name, theory_name, scf_type, input_blocks, &
        charge, mult, dft_direct, dft_smear_on, dft_smear_sigma, &
        dft_smear_spinset, scf_has_options, scf_maxiter, scf_thresh, &
        scf_tol2e, driver_has_options, driver_maxiter, &
        driver_tolerance_mode, driver_gmax_tol, driver_grms_tol, &
        driver_xmax_tol, driver_xrms_tol, psp_count, psp_elements, &
        psp_types, psp_names, set_string_count, set_keys, set_values, &
        typed_set_count, typed_set_keys, typed_set_types, &
        typed_set_value_counts, typed_set_values, &
        brillouin_has_options, brillouin_zone_name, &
        brillouin_monkhorst_pack, brillouin_max_kpoints_print, &
        brillouin_kvector_count, brillouin_kvectors, &
        energy_h, stress_au, errmsg, ok)
      import :: real64
      integer, intent(in) :: rtdb, n_atoms
      real(real64), intent(in) :: pos_ang(*)
      integer, intent(in) :: atmnrs(*)
      real(real64), intent(in) :: cell_ang(*)
      integer, intent(in) :: has_cell
      character(len=64), intent(in) :: basis_name
      character(len=64), intent(in) :: theory_name, scf_type
      character(len=4096), intent(in) :: input_blocks
      integer, intent(in) :: charge, mult
      integer, intent(in) :: dft_direct, dft_smear_on, dft_smear_spinset
      integer, intent(in) :: scf_has_options, scf_maxiter
      integer, intent(in) :: driver_has_options, driver_maxiter
      integer, intent(in) :: driver_tolerance_mode
      real(real64), intent(in) :: driver_gmax_tol, driver_grms_tol
      real(real64), intent(in) :: driver_xmax_tol, driver_xrms_tol
      integer, intent(in) :: psp_count
      character(len=16), intent(in) :: psp_elements(*)
      integer, intent(in) :: psp_types(*)
      character(len=256), intent(in) :: psp_names(*)
      integer, intent(in) :: set_string_count
      character(len=128), intent(in) :: set_keys(*)
      character(len=256), intent(in) :: set_values(*)
      integer, intent(in) :: typed_set_count
      character(len=128), intent(in) :: typed_set_keys(*)
      integer, intent(in) :: typed_set_types(*)
      integer, intent(in) :: typed_set_value_counts(*)
      character(len=256), intent(in) :: typed_set_values(64,*)
      integer, intent(in) :: brillouin_has_options
      character(len=64), intent(in) :: brillouin_zone_name
      integer, intent(in) :: brillouin_monkhorst_pack(3)
      integer, intent(in) :: brillouin_max_kpoints_print
      integer, intent(in) :: brillouin_kvector_count
      real(real64), intent(in) :: brillouin_kvectors(*)
      real(real64), intent(in) :: dft_smear_sigma
      real(real64), intent(in) :: scf_thresh, scf_tol2e
      real(real64), intent(out) :: energy_h
      real(real64), intent(out) :: stress_au(*)
      character(len=*), intent(out) :: errmsg
      integer, intent(out) :: ok
    end subroutine nwchem_legacy_stress

    subroutine nwchem_legacy_hessian(rtdb, n_atoms, pos_ang, atmnrs, &
        cell_ang, has_cell, basis_name, theory_name, scf_type, input_blocks, &
        charge, mult, dft_direct, dft_smear_on, dft_smear_sigma, &
        dft_smear_spinset, scf_has_options, scf_maxiter, scf_thresh, &
        scf_tol2e, driver_has_options, driver_maxiter, &
        driver_tolerance_mode, driver_gmax_tol, driver_grms_tol, &
        driver_xmax_tol, driver_xrms_tol, psp_count, psp_elements, psp_types, &
        psp_names, set_string_count, set_keys, set_values, &
        typed_set_count, typed_set_keys, typed_set_types, &
        typed_set_value_counts, typed_set_values, &
        brillouin_has_options, brillouin_zone_name, &
        brillouin_monkhorst_pack, brillouin_max_kpoints_print, &
        brillouin_kvector_count, brillouin_kvectors, &
        energy_h, hessian_h_bohr2, errmsg, ok)
      import :: real64
      integer, intent(in) :: rtdb, n_atoms
      real(real64), intent(in) :: pos_ang(*)
      integer, intent(in) :: atmnrs(*)
      real(real64), intent(in) :: cell_ang(*)
      integer, intent(in) :: has_cell
      character(len=64), intent(in) :: basis_name
      character(len=64), intent(in) :: theory_name, scf_type
      character(len=4096), intent(in) :: input_blocks
      integer, intent(in) :: charge, mult
      integer, intent(in) :: dft_direct, dft_smear_on, dft_smear_spinset
      integer, intent(in) :: scf_has_options, scf_maxiter
      integer, intent(in) :: driver_has_options, driver_maxiter
      integer, intent(in) :: driver_tolerance_mode
      real(real64), intent(in) :: driver_gmax_tol, driver_grms_tol
      real(real64), intent(in) :: driver_xmax_tol, driver_xrms_tol
      integer, intent(in) :: psp_count
      character(len=16), intent(in) :: psp_elements(*)
      integer, intent(in) :: psp_types(*)
      character(len=256), intent(in) :: psp_names(*)
      integer, intent(in) :: set_string_count
      character(len=128), intent(in) :: set_keys(*)
      character(len=256), intent(in) :: set_values(*)
      integer, intent(in) :: typed_set_count
      character(len=128), intent(in) :: typed_set_keys(*)
      integer, intent(in) :: typed_set_types(*)
      integer, intent(in) :: typed_set_value_counts(*)
      character(len=256), intent(in) :: typed_set_values(64,*)
      integer, intent(in) :: brillouin_has_options
      character(len=64), intent(in) :: brillouin_zone_name
      integer, intent(in) :: brillouin_monkhorst_pack(3)
      integer, intent(in) :: brillouin_max_kpoints_print
      integer, intent(in) :: brillouin_kvector_count
      real(real64), intent(in) :: brillouin_kvectors(*)
      real(real64), intent(in) :: dft_smear_sigma
      real(real64), intent(in) :: scf_thresh, scf_tol2e
      real(real64), intent(out) :: energy_h
      real(real64), intent(out) :: hessian_h_bohr2(*)
      character(len=*), intent(out) :: errmsg
      integer, intent(out) :: ok
    end subroutine nwchem_legacy_hessian

    subroutine nwchem_legacy_frequencies(rtdb, n_atoms, pos_ang, atmnrs, &
        cell_ang, has_cell, basis_name, theory_name, scf_type, input_blocks, &
        charge, mult, dft_direct, dft_smear_on, dft_smear_sigma, &
        dft_smear_spinset, scf_has_options, scf_maxiter, scf_thresh, &
        scf_tol2e, driver_has_options, driver_maxiter, &
        driver_tolerance_mode, driver_gmax_tol, driver_grms_tol, &
        driver_xmax_tol, driver_xrms_tol, psp_count, psp_elements, &
        psp_types, psp_names, set_string_count, set_keys, set_values, &
        typed_set_count, typed_set_keys, typed_set_types, &
        typed_set_value_counts, typed_set_values, &
        brillouin_has_options, brillouin_zone_name, &
        brillouin_monkhorst_pack, brillouin_max_kpoints_print, &
        brillouin_kvector_count, brillouin_kvectors, &
        energy_h, frequencies_cm1, intensities_au, normal_modes, &
        read_modes, projected_frequencies_cm1, projected_intensities_au, &
        read_projected, thermochemistry, read_thermo, errmsg, ok)
      import :: real64
      integer, intent(in) :: rtdb, n_atoms
      real(real64), intent(in) :: pos_ang(*)
      integer, intent(in) :: atmnrs(*)
      real(real64), intent(in) :: cell_ang(*)
      integer, intent(in) :: has_cell
      character(len=64), intent(in) :: basis_name
      character(len=64), intent(in) :: theory_name, scf_type
      character(len=4096), intent(in) :: input_blocks
      integer, intent(in) :: charge, mult
      integer, intent(in) :: dft_direct, dft_smear_on, dft_smear_spinset
      integer, intent(in) :: scf_has_options, scf_maxiter
      integer, intent(in) :: driver_has_options, driver_maxiter
      integer, intent(in) :: driver_tolerance_mode
      real(real64), intent(in) :: driver_gmax_tol, driver_grms_tol
      real(real64), intent(in) :: driver_xmax_tol, driver_xrms_tol
      integer, intent(in) :: psp_count
      character(len=16), intent(in) :: psp_elements(*)
      integer, intent(in) :: psp_types(*)
      character(len=256), intent(in) :: psp_names(*)
      integer, intent(in) :: set_string_count
      character(len=128), intent(in) :: set_keys(*)
      character(len=256), intent(in) :: set_values(*)
      integer, intent(in) :: typed_set_count
      character(len=128), intent(in) :: typed_set_keys(*)
      integer, intent(in) :: typed_set_types(*)
      integer, intent(in) :: typed_set_value_counts(*)
      character(len=256), intent(in) :: typed_set_values(64,*)
      integer, intent(in) :: brillouin_has_options
      character(len=64), intent(in) :: brillouin_zone_name
      integer, intent(in) :: brillouin_monkhorst_pack(3)
      integer, intent(in) :: brillouin_max_kpoints_print
      integer, intent(in) :: brillouin_kvector_count
      real(real64), intent(in) :: brillouin_kvectors(*)
      real(real64), intent(in) :: dft_smear_sigma
      real(real64), intent(in) :: scf_thresh, scf_tol2e
      real(real64), intent(out) :: energy_h
      real(real64), intent(out) :: frequencies_cm1(*)
      real(real64), intent(out) :: intensities_au(*)
      real(real64), intent(out) :: normal_modes(*)
      integer, intent(in) :: read_modes
      real(real64), intent(out) :: projected_frequencies_cm1(*)
      real(real64), intent(out) :: projected_intensities_au(*)
      integer, intent(in) :: read_projected
      real(real64), intent(out) :: thermochemistry(*)
      integer, intent(in) :: read_thermo
      character(len=*), intent(out) :: errmsg
      integer, intent(out) :: ok
    end subroutine nwchem_legacy_frequencies

    subroutine nwchem_legacy_optimize(rtdb, n_atoms, pos_ang, atmnrs, &
        cell_ang, has_cell, basis_name, theory_name, scf_type, input_blocks, &
        charge, mult, dft_direct, dft_smear_on, dft_smear_sigma, &
        dft_smear_spinset, scf_has_options, scf_maxiter, scf_thresh, &
        scf_tol2e, driver_has_options, driver_maxiter, &
        driver_tolerance_mode, driver_gmax_tol, driver_grms_tol, &
        driver_xmax_tol, driver_xrms_tol, psp_count, psp_elements, &
        psp_types, psp_names, set_string_count, set_keys, set_values, &
        typed_set_count, typed_set_keys, typed_set_types, &
        typed_set_value_counts, typed_set_values, &
        brillouin_has_options, brillouin_zone_name, &
        brillouin_monkhorst_pack, brillouin_max_kpoints_print, &
        brillouin_kvector_count, brillouin_kvectors, &
        energy_h, optimized_pos_ang, errmsg, ok)
      import :: real64
      integer, intent(in) :: rtdb, n_atoms
      real(real64), intent(in) :: pos_ang(*)
      integer, intent(in) :: atmnrs(*)
      real(real64), intent(in) :: cell_ang(*)
      integer, intent(in) :: has_cell
      character(len=64), intent(in) :: basis_name
      character(len=64), intent(in) :: theory_name, scf_type
      character(len=4096), intent(in) :: input_blocks
      integer, intent(in) :: charge, mult
      integer, intent(in) :: dft_direct, dft_smear_on, dft_smear_spinset
      integer, intent(in) :: scf_has_options, scf_maxiter
      integer, intent(in) :: driver_has_options, driver_maxiter
      integer, intent(in) :: driver_tolerance_mode
      real(real64), intent(in) :: driver_gmax_tol, driver_grms_tol
      real(real64), intent(in) :: driver_xmax_tol, driver_xrms_tol
      integer, intent(in) :: psp_count
      character(len=16), intent(in) :: psp_elements(*)
      integer, intent(in) :: psp_types(*)
      character(len=256), intent(in) :: psp_names(*)
      integer, intent(in) :: set_string_count
      character(len=128), intent(in) :: set_keys(*)
      character(len=256), intent(in) :: set_values(*)
      integer, intent(in) :: typed_set_count
      character(len=128), intent(in) :: typed_set_keys(*)
      integer, intent(in) :: typed_set_types(*)
      integer, intent(in) :: typed_set_value_counts(*)
      character(len=256), intent(in) :: typed_set_values(64,*)
      integer, intent(in) :: brillouin_has_options
      character(len=64), intent(in) :: brillouin_zone_name
      integer, intent(in) :: brillouin_monkhorst_pack(3)
      integer, intent(in) :: brillouin_max_kpoints_print
      integer, intent(in) :: brillouin_kvector_count
      real(real64), intent(in) :: brillouin_kvectors(*)
      real(real64), intent(in) :: dft_smear_sigma
      real(real64), intent(in) :: scf_thresh, scf_tol2e
      real(real64), intent(out) :: energy_h
      real(real64), intent(out) :: optimized_pos_ang(*)
      character(len=*), intent(out) :: errmsg
      integer, intent(out) :: ok
    end subroutine nwchem_legacy_optimize
  end interface

contains

  !> Ensure GA/MA/RTDB once; safe to call repeatedly from C.
  subroutine nwchemc_embed_init() bind(C, name='nwchemc_embed_init')
    integer :: ok, owns_mpi
    if (rtdb_ready) return
    if (runtime_finalized) return
    if (.not. ga_ready) then
      call nwchem_legacy_init(rtdb_handle, ok, owns_mpi)
      if (ok /= 0) then
        rtdb_handle = -1
        rtdb_ready = .false.
        return
      end if
      owns_mpi_runtime = owns_mpi /= 0
      ga_ready = .true.
      rtdb_ready = .true.
    end if
  end subroutine nwchemc_embed_init

  function nwchemc_embed_reset_rtdb() result(rc) &
      bind(C, name='nwchemc_embed_reset_rtdb')
    integer(c_int) :: rc
    integer :: ok

    rc = -1_c_int
    if (runtime_finalized) return
    call nwchemc_embed_init()
    if (.not. rtdb_ready) return
    call nwchem_legacy_reset_rtdb(rtdb_handle, ok)
    if (ok == 0) then
      rtdb_ready = .true.
      rc = 0_c_int
    else
      rtdb_handle = -1
      rtdb_ready = .false.
    end if
  end function nwchemc_embed_reset_rtdb

  !> Tear down the owned in-process NWChem runtime.
  subroutine nwchemc_embed_finalize() bind(C, name='nwchemc_embed_finalize')
    integer :: ok, owns_mpi
    if (.not. ga_ready .and. .not. rtdb_ready) return
    owns_mpi = 0
    if (owns_mpi_runtime) owns_mpi = 1
    call nwchem_legacy_finalize(rtdb_handle, owns_mpi, ok)
    ga_ready = .false.
    rtdb_ready = .false.
    owns_mpi_runtime = .false.
    runtime_finalized = .true.
    rtdb_handle = -1
  end subroutine nwchemc_embed_finalize

  !> 1 if embed RTDB is usable, else 0.
  function nwchemc_embed_available() result(avail) &
      bind(C, name='nwchemc_embed_available')
    integer(c_int) :: avail
    call nwchemc_embed_init()
    if (rtdb_ready) then
      avail = 1_c_int
    else
      avail = 0_c_int
    end if
  end function nwchemc_embed_available

  !> Energy from the most recent embedded NWChem task.
  function nwchemc_embed_last_energy(energy_h) result(rc) &
      bind(C, name='nwchemc_embed_last_energy')
    real(c_double), intent(out) :: energy_h
    integer(c_int) :: rc

    energy_h = real(last_task_energy_h, kind=c_double)
    if (rtdb_ready) then
      rc = 0_c_int
    else
      rc = -1_c_int
    end if
  end function nwchemc_embed_last_energy

  !> Active RTDB handle for integration probes that verify embedded state.
  function nwchemc_embed_current_rtdb() result(handle) &
      bind(C, name='nwchemc_embed_current_rtdb')
    integer(c_long_long) :: handle

    handle = -1_c_long_long
    if (runtime_finalized) return
    call nwchemc_embed_init()
    if (rtdb_ready) handle = int(rtdb_handle, c_long_long)
  end function nwchemc_embed_current_rtdb

  !> Apply method options from C strings (lengths explicit for C interop).
  function nwchemc_embed_set_config(basis, basis_len, theory, theory_len, &
      scf_type, scf_len, charge, mult, input_blocks, input_len) result(rc) &
      bind(C, name='nwchemc_embed_set_config')
    character(kind=c_char), intent(in) :: basis(*)
    integer(c_int), intent(in), value :: basis_len
    character(kind=c_char), intent(in) :: theory(*)
    integer(c_int), intent(in), value :: theory_len
    character(kind=c_char), intent(in) :: scf_type(*)
    integer(c_int), intent(in), value :: scf_len
    integer(c_int), intent(in) :: charge
    integer(c_int), intent(in) :: mult
    character(kind=c_char), intent(in) :: input_blocks(*)
    integer(c_int), intent(in), value :: input_len
    integer(c_int) :: rc
    character(len=64) :: bstr, tstr, sstr
    character(len=4096) :: iblocks

    rc = -1_c_int
    call nwchemc_embed_init()
    if (.not. rtdb_ready) return

    call c_chars_to_f(basis, basis_len, bstr)
    call c_chars_to_f(theory, theory_len, tstr)
    call c_chars_to_f(scf_type, scf_len, sstr)
    call c_chars_to_f(input_blocks, input_len, iblocks)
    if (len_trim(bstr) == 0) bstr = 'sto-3g'
    if (len_trim(tstr) == 0) tstr = 'scf'
    if (len_trim(sstr) == 0) sstr = 'rhf'

    ! theory aliases: blyp / b3lyp / pbe -> dft + xc
    if (tstr(1:4) == 'blyp' .or. tstr(1:5) == 'b3lyp' .or. &
        tstr(1:3) == 'pbe' .or. tstr(1:3) == 'dft') then
      if (tstr(1:3) /= 'dft') sstr = tstr
      tstr = 'dft'
    end if

    cfg_basis = bstr
    cfg_theory = tstr
    cfg_scf = sstr
    cfg_input_blocks = iblocks
    cfg_charge = int(charge)
    cfg_mult = max(1, int(mult))

    ! Evaluation calls write method state after numeric geometry setup.
    rc = 0_c_int
  end function nwchemc_embed_set_config

  !> Store direct DFT options extracted from Cap'n Proto (xc/direct/smear).
  !> Promoted xc updates cfg_scf; numeric DFT knobs are applied through RTDB.
  function nwchemc_embed_set_dft_direct(xc, xc_len, direct_enabled, &
      smearing_enabled, smear_sigma_hartree, smearing_spinset) result(rc) &
      bind(C, name='nwchemc_embed_set_dft_direct')
    character(kind=c_char), intent(in) :: xc(*)
    integer(c_int), intent(in), value :: xc_len
    integer(c_int), intent(in), value :: direct_enabled
    integer(c_int), intent(in), value :: smearing_enabled
    real(c_double), intent(in), value :: smear_sigma_hartree
    integer(c_int), intent(in), value :: smearing_spinset
    integer(c_int) :: rc
    character(len=64) :: xcstr

    rc = 0_c_int
    call c_chars_to_f(xc, xc_len, xcstr)
    if (len_trim(xcstr) > 0) then
      cfg_dft_xc = xcstr
      cfg_scf = xcstr
      ! DFT XC is the reference functional; do not clobber tddft/sodft theory.
      if (cfg_theory(1:3) /= 'dft' .and. cfg_theory(1:5) /= 'tddft' .and. &
          cfg_theory(1:5) /= 'sodft' .and. cfg_theory(1:8) /= 'rt_tddft') then
        cfg_theory = 'dft'
      end if
    end if
    cfg_dft_direct = int(direct_enabled)
    cfg_dft_smear_on = int(smearing_enabled)
    cfg_dft_smear_sigma = real(smear_sigma_hartree, real64)
    cfg_dft_smear_spinset = int(smearing_spinset)
  end function nwchemc_embed_set_dft_direct

  !> Store structured SCF scalar controls extracted from Cap'n Proto.
  function nwchemc_embed_set_scf_direct(has_options, maxiter, thresh, &
      tol2e) result(rc) bind(C, name='nwchemc_embed_set_scf_direct')
    integer(c_int), intent(in), value :: has_options
    integer(c_int), intent(in), value :: maxiter
    real(c_double), intent(in), value :: thresh
    real(c_double), intent(in), value :: tol2e
    integer(c_int) :: rc

    cfg_scf_has_options = int(has_options)
    cfg_scf_maxiter = int(maxiter)
    cfg_scf_thresh = real(thresh, real64)
    cfg_scf_tol2e = real(tol2e, real64)
    rc = 0_c_int
  end function nwchemc_embed_set_scf_direct

  !> Store structured driver scalar controls extracted from Cap'n Proto.
  function nwchemc_embed_set_driver_direct(has_options, maxiter, &
      tolerance_mode, gmax_tol, grms_tol, xmax_tol, xrms_tol) result(rc) &
      bind(C, name='nwchemc_embed_set_driver_direct')
    integer(c_int), intent(in), value :: has_options
    integer(c_int), intent(in), value :: maxiter
    integer(c_int), intent(in), value :: tolerance_mode
    real(c_double), intent(in), value :: gmax_tol
    real(c_double), intent(in), value :: grms_tol
    real(c_double), intent(in), value :: xmax_tol
    real(c_double), intent(in), value :: xrms_tol
    integer(c_int) :: rc

    cfg_driver_has_options = int(has_options)
    cfg_driver_maxiter = int(maxiter)
    cfg_driver_tolerance_mode = int(tolerance_mode)
    cfg_driver_gmax_tol = real(gmax_tol, real64)
    cfg_driver_grms_tol = real(grms_tol, real64)
    cfg_driver_xmax_tol = real(xmax_tol, real64)
    cfg_driver_xrms_tol = real(xrms_tol, real64)
    rc = 0_c_int
  end function nwchemc_embed_set_driver_direct

  !> Confirm structured NWPW cutoff controls are present on the direct path.
  !> The C layer expands these controls into typed RTDB set entries.
  function nwchemc_embed_set_nwpw_direct(has_options, energy_cutoff, &
      wavefunction_cutoff, ewald_rcut, ewald_ncut) result(rc) &
      bind(C, name='nwchemc_embed_set_nwpw_direct')
    integer(c_int), intent(in), value :: has_options
    real(c_double), intent(in), value :: energy_cutoff
    real(c_double), intent(in), value :: wavefunction_cutoff
    real(c_double), intent(in), value :: ewald_rcut
    integer(c_int), intent(in), value :: ewald_ncut
    integer(c_int) :: rc

    if (has_options < 0 .or. energy_cutoff < 0.0_c_double .or. &
        wavefunction_cutoff < 0.0_c_double .or. &
        ewald_rcut < 0.0_c_double .or. ewald_ncut < 0) then
      rc = -1_c_int
    else
      rc = 0_c_int
    end if
  end function nwchemc_embed_set_nwpw_direct

  !> Store structured Brillouin-zone controls for direct RTDB setup.
  function nwchemc_embed_set_brillouin_zone(has_options, zone_name, &
      zone_name_len, monkhorst_pack_x, monkhorst_pack_y, monkhorst_pack_z, &
      max_kpoints_print, kvector_values, kvector_count) result(rc) &
      bind(C, name='nwchemc_embed_set_brillouin_zone')
    integer(c_int), intent(in), value :: has_options
    character(kind=c_char), intent(in) :: zone_name(*)
    integer(c_int), intent(in), value :: zone_name_len
    integer(c_int), intent(in), value :: monkhorst_pack_x
    integer(c_int), intent(in), value :: monkhorst_pack_y
    integer(c_int), intent(in), value :: monkhorst_pack_z
    integer(c_int), intent(in), value :: max_kpoints_print
    real(c_double), intent(in) :: kvector_values(*)
    integer(c_int), intent(in), value :: kvector_count
    integer(c_int) :: rc
    character(len=64) :: zname
    integer :: alloc_status, i, nvalues

    rc = -1_c_int
    if (has_options < 0 .or. max_kpoints_print < 0 .or. &
        kvector_count < 0) return
    call c_chars_to_f(zone_name, zone_name_len, zname)
    if (len_trim(zname) == 0) zname = 'zone_default'
    if (allocated(cfg_brillouin_kvectors)) deallocate (cfg_brillouin_kvectors)
    nvalues = 4 * int(kvector_count)
    allocate (cfg_brillouin_kvectors(max(1, nvalues)), stat=alloc_status)
    if (alloc_status /= 0) return
    cfg_brillouin_kvectors = 0.0_real64
    do i = 1, nvalues
      cfg_brillouin_kvectors(i) = real(kvector_values(i), real64)
    end do
    cfg_brillouin_has_options = int(has_options)
    cfg_brillouin_zone_name = zname
    cfg_brillouin_monkhorst_pack(1) = int(monkhorst_pack_x)
    cfg_brillouin_monkhorst_pack(2) = int(monkhorst_pack_y)
    cfg_brillouin_monkhorst_pack(3) = int(monkhorst_pack_z)
    cfg_brillouin_max_kpoints_print = int(max_kpoints_print)
    cfg_brillouin_kvector_count = int(kvector_count)
    rc = 0_c_int
  end function nwchemc_embed_set_brillouin_zone

  !> Store generated DOS-style Brillouin zones extracted from Cap'n Proto.
  function nwchemc_embed_set_brillouin_dos_zones(zone_names, zone_grids, &
      count) result(rc) bind(C, name='nwchemc_embed_set_brillouin_dos_zones')
    character(kind=c_char), intent(in) :: zone_names(*)
    integer(c_int), intent(in) :: zone_grids(*)
    integer(c_int), intent(in), value :: count
    integer(c_int) :: rc
    integer :: i, n
    character(len=256) :: errmsg
    logical :: ok

    rc = -1_c_int
    n = int(count)
    if (n < 0 .or. n > max_embed_set_strings) return

    cfg_brillouin_dos_zone_count = n
    cfg_brillouin_dos_zone_names = ' '
    cfg_brillouin_dos_zone_grids = 0
    do i = 1, n
      call c_chars_to_f(zone_names((i - 1) * 64 + 1), 64_c_int, &
          cfg_brillouin_dos_zone_names(i))
      cfg_brillouin_dos_zone_grids(1, i) = int(zone_grids(3 * (i - 1) + 1))
      cfg_brillouin_dos_zone_grids(2, i) = int(zone_grids(3 * (i - 1) + 2))
      cfg_brillouin_dos_zone_grids(3, i) = int(zone_grids(3 * (i - 1) + 3))
    end do
    if (rtdb_ready .and. n > 0) then
      call nwchemc_store_brillouin_dos_zones(rtdb_handle, n, &
          cfg_brillouin_dos_zone_names, cfg_brillouin_dos_zone_grids, &
          errmsg, ok)
      if (.not. ok) return
    end if
    rc = 0_c_int
  end function nwchemc_embed_set_brillouin_dos_zones

  !> Store direct NWPW pseudopotential library entries extracted from Cap'n Proto.
  function nwchemc_embed_set_pseudopotentials(elements, library_types, &
      library_names, count) result(rc) &
      bind(C, name='nwchemc_embed_set_pseudopotentials')
    character(kind=c_char), intent(in) :: elements(*)
    integer(c_int), intent(in) :: library_types(*)
    character(kind=c_char), intent(in) :: library_names(*)
    integer(c_int), intent(in), value :: count
    integer(c_int) :: rc
    integer :: i, n
    character(len=256) :: errmsg
    logical :: ok

    rc = -1_c_int
    n = int(count)
    if (n < 0 .or. n > max_embed_pseudopotentials) return

    cfg_psp_count = n
    cfg_psp_elements = ' '
    cfg_psp_names = ' '
    cfg_psp_types = 0
    do i = 1, n
      call c_chars_to_f(elements((i - 1) * psp_element_len + 1), &
          int(psp_element_len, c_int), cfg_psp_elements(i))
      call c_chars_to_f(library_names((i - 1) * psp_name_len + 1), &
          int(psp_name_len, c_int), cfg_psp_names(i))
      cfg_psp_types(i) = int(library_types(i))
    end do
    if (rtdb_ready .and. n > 0) then
      call nwchemc_store_pseudopotentials(rtdb_handle, n, &
          cfg_psp_elements, cfg_psp_types, cfg_psp_names, errmsg, ok)
      if (.not. ok) return
    end if
    rc = 0_c_int
  end function nwchemc_embed_set_pseudopotentials

  !> Store text-valued RTDB set directives extracted from Cap'n Proto.
  function nwchemc_embed_set_rtdb_strings(keys, values, count) result(rc) &
      bind(C, name='nwchemc_embed_set_rtdb_strings')
    character(kind=c_char), intent(in) :: keys(*)
    character(kind=c_char), intent(in) :: values(*)
    integer(c_int), intent(in), value :: count
    integer(c_int) :: rc
    integer :: i, n
    character(len=256) :: errmsg
    logical :: ok

    rc = -1_c_int
    n = int(count)
    if (n < 0 .or. n > max_embed_set_strings) return

    cfg_set_string_count = n
    cfg_set_keys = ' '
    cfg_set_values = ' '
    do i = 1, n
      call c_chars_to_f(keys((i - 1) * set_key_len + 1), &
          int(set_key_len, c_int), cfg_set_keys(i))
      call c_chars_to_f(values((i - 1) * set_value_len + 1), &
          int(set_value_len, c_int), cfg_set_values(i))
    end do
    if (rtdb_ready .and. n > 0) then
      call nwchemc_apply_direct_string_sets(rtdb_handle, n, cfg_set_keys, &
          cfg_set_values, errmsg, ok)
      if (.not. ok) return
    end if
    rc = 0_c_int
  end function nwchemc_embed_set_rtdb_strings

  !> Store typed RTDB set directives extracted from Cap'n Proto.
  function nwchemc_embed_set_rtdb_values(keys, value_types, value_counts, &
      values, count) result(rc) bind(C, name='nwchemc_embed_set_rtdb_values')
    character(kind=c_char), intent(in) :: keys(*)
    integer(c_int), intent(in) :: value_types(*)
    integer(c_int), intent(in) :: value_counts(*)
    character(kind=c_char), intent(in) :: values(*)
    integer(c_int), intent(in), value :: count
    integer(c_int) :: rc
    integer :: i, j, n, nvalues, offset
    character(len=256) :: errmsg
    logical :: ok

    rc = -1_c_int
    n = int(count)
    if (n < 0 .or. n > max_embed_set_strings) return

    cfg_typed_set_count = n
    cfg_typed_set_keys = ' '
    cfg_typed_set_types = 0
    cfg_typed_set_value_counts = 0
    cfg_typed_set_values = ' '
    do i = 1, n
      nvalues = int(value_counts(i))
      if (nvalues <= 0 .or. nvalues > max_embed_set_values) return
      call c_chars_to_f(keys((i - 1) * set_key_len + 1), &
          int(set_key_len, c_int), cfg_typed_set_keys(i))
      cfg_typed_set_types(i) = int(value_types(i))
      cfg_typed_set_value_counts(i) = nvalues
      do j = 1, nvalues
        offset = ((i - 1) * max_embed_set_values + (j - 1)) * &
            set_value_len + 1
        call c_chars_to_f(values(offset), int(set_value_len, c_int), &
            cfg_typed_set_values(j, i))
      end do
    end do
    if (rtdb_ready .and. n > 0) then
      call nwchemc_apply_direct_typed_sets(rtdb_handle, n, &
          cfg_typed_set_keys, cfg_typed_set_types, &
          cfg_typed_set_value_counts, cfg_typed_set_values, errmsg, ok)
      if (.not. ok) return
    end if
    rc = 0_c_int
  end function nwchemc_embed_set_rtdb_values

  !> Energy (Hartree) for current config.
  function nwchemc_embed_energy_only(n_atoms, positions_ang, &
      atomic_numbers, charge, mult, energy_h, errmsg, errmsg_len) result(rc) &
      bind(C, name='nwchemc_embed_energy_only')
    integer(c_int), intent(in) :: n_atoms
    real(c_double), intent(in) :: positions_ang(*)
    integer(c_int), intent(in) :: atomic_numbers(*)
    integer(c_int), intent(in) :: charge
    integer(c_int), intent(in) :: mult
    real(c_double), intent(out) :: energy_h
    character(kind=c_char), intent(out) :: errmsg(*)
    integer(c_int), intent(in), value :: errmsg_len
    integer(c_int) :: rc
    real(c_double) :: empty_cell(9)
    integer(c_int) :: no_cell

    empty_cell = 0.0_c_double
    no_cell = 0_c_int
    rc = nwchemc_embed_energy_only_impl(n_atoms, positions_ang, &
        atomic_numbers, empty_cell, no_cell, charge, mult, energy_h, &
        errmsg, errmsg_len)
  end function nwchemc_embed_energy_only

  !> Energy (Hartree) with an optional 3x3 cell.
  function nwchemc_embed_energy_only_cell(n_atoms, positions_ang, &
      atomic_numbers, cell_ang, has_cell, charge, mult, energy_h, errmsg, &
      errmsg_len) result(rc) bind(C, name='nwchemc_embed_energy_only_cell')
    integer(c_int), intent(in) :: n_atoms
    real(c_double), intent(in) :: positions_ang(*)
    integer(c_int), intent(in) :: atomic_numbers(*)
    real(c_double), intent(in) :: cell_ang(*)
    integer(c_int), intent(in) :: has_cell
    integer(c_int), intent(in) :: charge
    integer(c_int), intent(in) :: mult
    real(c_double), intent(out) :: energy_h
    character(kind=c_char), intent(out) :: errmsg(*)
    integer(c_int), intent(in), value :: errmsg_len
    integer(c_int) :: rc

    rc = nwchemc_embed_energy_only_impl(n_atoms, positions_ang, &
        atomic_numbers, cell_ang, has_cell, charge, mult, energy_h, errmsg, &
        errmsg_len)
  end function nwchemc_embed_energy_only_cell

  function nwchemc_embed_energy_only_impl(n_atoms, positions_ang, &
      atomic_numbers, cell_ang, has_cell, charge, mult, energy_h, errmsg, &
      errmsg_len) result(rc)
    integer(c_int), intent(in) :: n_atoms
    real(c_double), intent(in) :: positions_ang(*)
    integer(c_int), intent(in) :: atomic_numbers(*)
    real(c_double), intent(in) :: cell_ang(*)
    integer(c_int), intent(in) :: has_cell
    integer(c_int), intent(in) :: charge
    integer(c_int), intent(in) :: mult
    real(c_double), intent(out) :: energy_h
    character(kind=c_char), intent(out) :: errmsg(*)
    integer(c_int), intent(in), value :: errmsg_len
    integer(c_int) :: rc
    integer :: ok, n, i
    character(len=512) :: msg
    real(real64), allocatable :: pos(:), cell(:)
    integer, allocatable :: z(:)

    rc = -1_c_int
    energy_h = 0.0_c_double
    call clear_c_errmsg(errmsg, errmsg_len)

    call nwchemc_embed_init()
    if (.not. rtdb_ready) then
      call set_c_errmsg(errmsg, errmsg_len, 'embed not initialized')
      return
    end if

    n = int(n_atoms)
    if (n <= 0) then
      call set_c_errmsg(errmsg, errmsg_len, 'n_atoms must be positive')
      return
    end if
    allocate (pos(3 * n), cell(9), z(n))
    do i = 1, 3 * n
      pos(i) = real(positions_ang(i), kind=real64)
    end do
    do i = 1, 9
      cell(i) = real(cell_ang(i), kind=real64)
    end do
    do i = 1, n
      z(i) = int(atomic_numbers(i))
    end do

    if (.not. ensure_brillouin_kvectors()) then
      call set_c_errmsg(errmsg, errmsg_len, 'brillouin kvector allocation failed')
      deallocate (pos, cell, z)
      return
    end if
    call nwchem_legacy_energy_only(rtdb_handle, n, pos, z, cell, &
        int(has_cell), cfg_basis, cfg_theory, cfg_scf, cfg_input_blocks, &
        int(charge), max(1, int(mult)), cfg_dft_direct, cfg_dft_smear_on, &
        cfg_dft_smear_sigma, cfg_dft_smear_spinset, cfg_scf_has_options, &
        cfg_scf_maxiter, cfg_scf_thresh, cfg_scf_tol2e, &
        cfg_driver_has_options, cfg_driver_maxiter, &
        cfg_driver_tolerance_mode, cfg_driver_gmax_tol, &
        cfg_driver_grms_tol, cfg_driver_xmax_tol, cfg_driver_xrms_tol, &
        cfg_psp_count, cfg_psp_elements, cfg_psp_types, cfg_psp_names, &
        cfg_set_string_count, cfg_set_keys, cfg_set_values, &
        cfg_typed_set_count, cfg_typed_set_keys, cfg_typed_set_types, &
        cfg_typed_set_value_counts, cfg_typed_set_values, &
        cfg_brillouin_has_options, cfg_brillouin_zone_name, &
        cfg_brillouin_monkhorst_pack, cfg_brillouin_max_kpoints_print, &
        cfg_brillouin_kvector_count, cfg_brillouin_kvectors, &
        energy_h, msg, ok)

    call set_c_errmsg(errmsg, errmsg_len, trim(msg))
    if (ok == 0) rc = 0_c_int
    deallocate (pos, cell, z)
  end function nwchemc_embed_energy_only_impl

  !> Energy (Hartree) + nuclear gradient (Hartree/Bohr) for current config.
  function nwchemc_embed_energy_grad(n_atoms, positions_ang, &
      atomic_numbers, charge, mult, energy_h, grad_h_bohr, errmsg, &
      errmsg_len) result(rc) bind(C, name='nwchemc_embed_energy_grad')
    integer(c_int), intent(in) :: n_atoms
    real(c_double), intent(in) :: positions_ang(*)
    integer(c_int), intent(in) :: atomic_numbers(*)
    integer(c_int), intent(in) :: charge
    integer(c_int), intent(in) :: mult
    real(c_double), intent(out) :: energy_h
    real(c_double), intent(out) :: grad_h_bohr(*)
    character(kind=c_char), intent(out) :: errmsg(*)
    integer(c_int), intent(in), value :: errmsg_len
    integer(c_int) :: rc
    real(c_double) :: empty_cell(9)
    integer(c_int) :: no_cell

    empty_cell = 0.0_c_double
    no_cell = 0_c_int
    rc = nwchemc_embed_energy_grad_impl(n_atoms, positions_ang, &
        atomic_numbers, empty_cell, no_cell, charge, mult, energy_h, &
        grad_h_bohr, errmsg, errmsg_len)
  end function nwchemc_embed_energy_grad

  !> Energy (Hartree) + nuclear gradient with an optional 3x3 cell.
  function nwchemc_embed_energy_grad_cell(n_atoms, positions_ang, &
      atomic_numbers, cell_ang, has_cell, charge, mult, energy_h, &
      grad_h_bohr, errmsg, errmsg_len) result(rc) &
      bind(C, name='nwchemc_embed_energy_grad_cell')
    integer(c_int), intent(in) :: n_atoms
    real(c_double), intent(in) :: positions_ang(*)
    integer(c_int), intent(in) :: atomic_numbers(*)
    real(c_double), intent(in) :: cell_ang(*)
    integer(c_int), intent(in) :: has_cell
    integer(c_int), intent(in) :: charge
    integer(c_int), intent(in) :: mult
    real(c_double), intent(out) :: energy_h
    real(c_double), intent(out) :: grad_h_bohr(*)
    character(kind=c_char), intent(out) :: errmsg(*)
    integer(c_int), intent(in), value :: errmsg_len
    integer(c_int) :: rc

    rc = nwchemc_embed_energy_grad_impl(n_atoms, positions_ang, &
        atomic_numbers, cell_ang, has_cell, charge, mult, energy_h, &
        grad_h_bohr, errmsg, errmsg_len)
  end function nwchemc_embed_energy_grad_cell

  function nwchemc_embed_energy_grad_impl(n_atoms, positions_ang, &
      atomic_numbers, cell_ang, has_cell, charge, mult, energy_h, &
      grad_h_bohr, errmsg, errmsg_len) result(rc)
    integer(c_int), intent(in) :: n_atoms
    real(c_double), intent(in) :: positions_ang(*)
    integer(c_int), intent(in) :: atomic_numbers(*)
    real(c_double), intent(in) :: cell_ang(*)
    integer(c_int), intent(in) :: has_cell
    integer(c_int), intent(in) :: charge
    integer(c_int), intent(in) :: mult
    real(c_double), intent(out) :: energy_h
    real(c_double), intent(out) :: grad_h_bohr(*)
    character(kind=c_char), intent(out) :: errmsg(*)
    integer(c_int), intent(in), value :: errmsg_len
    integer(c_int) :: rc
    integer :: ok, n, i
    character(len=512) :: msg
    real(real64), allocatable :: pos(:), grad(:), cell(:)
    integer, allocatable :: z(:)

    rc = -1_c_int
    energy_h = 0.0_c_double
    call clear_c_errmsg(errmsg, errmsg_len)

    call nwchemc_embed_init()
    if (.not. rtdb_ready) then
      call set_c_errmsg(errmsg, errmsg_len, 'embed not initialized')
      return
    end if

    n = int(n_atoms)
    if (n <= 0) then
      call set_c_errmsg(errmsg, errmsg_len, 'n_atoms must be positive')
      return
    end if
    allocate (pos(3 * n), grad(3 * n), cell(9), z(n))
    do i = 1, 3 * n
      pos(i) = real(positions_ang(i), kind=real64)
      grad(i) = 0.0_real64
    end do
    do i = 1, 9
      cell(i) = real(cell_ang(i), kind=real64)
    end do
    do i = 1, n
      z(i) = int(atomic_numbers(i))
    end do

    if (.not. ensure_brillouin_kvectors()) then
      call set_c_errmsg(errmsg, errmsg_len, 'brillouin kvector allocation failed')
      return
    end if
    call nwchem_legacy_energy_grad(rtdb_handle, n, pos, z, cell, &
        int(has_cell), cfg_basis, cfg_theory, cfg_scf, cfg_input_blocks, &
        int(charge), max(1, int(mult)), cfg_dft_direct, cfg_dft_smear_on, &
        cfg_dft_smear_sigma, cfg_dft_smear_spinset, cfg_scf_has_options, &
        cfg_scf_maxiter, cfg_scf_thresh, cfg_scf_tol2e, &
        cfg_driver_has_options, cfg_driver_maxiter, &
        cfg_driver_tolerance_mode, cfg_driver_gmax_tol, &
        cfg_driver_grms_tol, cfg_driver_xmax_tol, cfg_driver_xrms_tol, &
        cfg_psp_count, cfg_psp_elements, cfg_psp_types, cfg_psp_names, &
        cfg_set_string_count, cfg_set_keys, cfg_set_values, &
        cfg_typed_set_count, cfg_typed_set_keys, cfg_typed_set_types, &
        cfg_typed_set_value_counts, cfg_typed_set_values, &
        cfg_brillouin_has_options, cfg_brillouin_zone_name, &
        cfg_brillouin_monkhorst_pack, cfg_brillouin_max_kpoints_print, &
        cfg_brillouin_kvector_count, cfg_brillouin_kvectors, &
        energy_h, grad, msg, ok)

    do i = 1, 3 * n
      grad_h_bohr(i) = real(grad(i), kind=c_double)
    end do
    call set_c_errmsg(errmsg, errmsg_len, trim(msg))
    if (ok == 0) rc = 0_c_int
    deallocate (pos, grad, cell, z)
  end function nwchemc_embed_energy_grad_impl

  !> Total electric dipole (atomic units) for current config.
  function nwchemc_embed_dipole(n_atoms, positions_ang, atomic_numbers, &
      charge, mult, energy_h, dipole_au, errmsg, errmsg_len) result(rc) &
      bind(C, name='nwchemc_embed_dipole')
    integer(c_int), intent(in) :: n_atoms
    real(c_double), intent(in) :: positions_ang(*)
    integer(c_int), intent(in) :: atomic_numbers(*)
    integer(c_int), intent(in) :: charge
    integer(c_int), intent(in) :: mult
    real(c_double), intent(out) :: energy_h
    real(c_double), intent(out) :: dipole_au(*)
    character(kind=c_char), intent(out) :: errmsg(*)
    integer(c_int), intent(in), value :: errmsg_len
    integer(c_int) :: rc
    real(c_double) :: empty_cell(9)
    integer(c_int) :: no_cell

    empty_cell = 0.0_c_double
    no_cell = 0_c_int
    rc = nwchemc_embed_dipole_impl(n_atoms, positions_ang, atomic_numbers, &
        empty_cell, no_cell, charge, mult, energy_h, dipole_au, errmsg, &
        errmsg_len)
  end function nwchemc_embed_dipole

  !> Total electric dipole with an optional 3x3 cell.
  function nwchemc_embed_dipole_cell(n_atoms, positions_ang, atomic_numbers, &
      cell_ang, has_cell, charge, mult, energy_h, dipole_au, errmsg, &
      errmsg_len) result(rc) bind(C, name='nwchemc_embed_dipole_cell')
    integer(c_int), intent(in) :: n_atoms
    real(c_double), intent(in) :: positions_ang(*)
    integer(c_int), intent(in) :: atomic_numbers(*)
    real(c_double), intent(in) :: cell_ang(*)
    integer(c_int), intent(in) :: has_cell
    integer(c_int), intent(in) :: charge
    integer(c_int), intent(in) :: mult
    real(c_double), intent(out) :: energy_h
    real(c_double), intent(out) :: dipole_au(*)
    character(kind=c_char), intent(out) :: errmsg(*)
    integer(c_int), intent(in), value :: errmsg_len
    integer(c_int) :: rc

    rc = nwchemc_embed_dipole_impl(n_atoms, positions_ang, atomic_numbers, &
        cell_ang, has_cell, charge, mult, energy_h, dipole_au, errmsg, &
        errmsg_len)
  end function nwchemc_embed_dipole_cell

  function nwchemc_embed_dipole_impl(n_atoms, positions_ang, atomic_numbers, &
      cell_ang, has_cell, charge, mult, energy_h, dipole_au, errmsg, &
      errmsg_len) result(rc)
    integer(c_int), intent(in) :: n_atoms
    real(c_double), intent(in) :: positions_ang(*)
    integer(c_int), intent(in) :: atomic_numbers(*)
    real(c_double), intent(in) :: cell_ang(*)
    integer(c_int), intent(in) :: has_cell
    integer(c_int), intent(in) :: charge
    integer(c_int), intent(in) :: mult
    real(c_double), intent(out) :: energy_h
    real(c_double), intent(out) :: dipole_au(*)
    character(kind=c_char), intent(out) :: errmsg(*)
    integer(c_int), intent(in), value :: errmsg_len
    integer(c_int) :: rc
    integer :: ok, n, i
    character(len=512) :: msg
    real(real64), allocatable :: pos(:), dipole(:), cell(:)
    integer, allocatable :: z(:)

    rc = -1_c_int
    energy_h = 0.0_c_double
    do i = 1, 3
      dipole_au(i) = 0.0_c_double
    end do
    call clear_c_errmsg(errmsg, errmsg_len)

    call nwchemc_embed_init()
    if (.not. rtdb_ready) then
      call set_c_errmsg(errmsg, errmsg_len, 'embed not initialized')
      return
    end if

    n = int(n_atoms)
    if (n <= 0) then
      call set_c_errmsg(errmsg, errmsg_len, 'n_atoms must be positive')
      return
    end if
    allocate (pos(3 * n), dipole(3), cell(9), z(n))
    do i = 1, 3 * n
      pos(i) = real(positions_ang(i), kind=real64)
    end do
    do i = 1, 9
      cell(i) = real(cell_ang(i), kind=real64)
    end do
    do i = 1, 3
      dipole(i) = 0.0_real64
    end do
    do i = 1, n
      z(i) = int(atomic_numbers(i))
    end do

    if (.not. ensure_brillouin_kvectors()) then
      call set_c_errmsg(errmsg, errmsg_len, 'brillouin kvector allocation failed')
      return
    end if
    call nwchem_legacy_dipole(rtdb_handle, n, pos, z, cell, &
        int(has_cell), cfg_basis, cfg_theory, cfg_scf, cfg_input_blocks, &
        int(charge), max(1, int(mult)), cfg_dft_direct, cfg_dft_smear_on, &
        cfg_dft_smear_sigma, cfg_dft_smear_spinset, cfg_scf_has_options, &
        cfg_scf_maxiter, cfg_scf_thresh, cfg_scf_tol2e, &
        cfg_driver_has_options, cfg_driver_maxiter, &
        cfg_driver_tolerance_mode, cfg_driver_gmax_tol, &
        cfg_driver_grms_tol, cfg_driver_xmax_tol, cfg_driver_xrms_tol, &
        cfg_psp_count, cfg_psp_elements, cfg_psp_types, cfg_psp_names, &
        cfg_set_string_count, cfg_set_keys, cfg_set_values, &
        cfg_typed_set_count, cfg_typed_set_keys, cfg_typed_set_types, &
        cfg_typed_set_value_counts, cfg_typed_set_values, &
        cfg_brillouin_has_options, cfg_brillouin_zone_name, &
        cfg_brillouin_monkhorst_pack, cfg_brillouin_max_kpoints_print, &
        cfg_brillouin_kvector_count, cfg_brillouin_kvectors, &
        energy_h, dipole, msg, ok)

    do i = 1, 3
      dipole_au(i) = real(dipole(i), kind=c_double)
    end do
    call set_c_errmsg(errmsg, errmsg_len, trim(msg))
    if (ok == 0) rc = 0_c_int
    deallocate (pos, dipole, cell, z)
  end function nwchemc_embed_dipole_impl

  !> Electric polarizability response vector (atomic units) for current config.
  function nwchemc_embed_polarizability(n_atoms, positions_ang, &
      atomic_numbers, charge, mult, energy_h, polarizability_au, errmsg, &
      errmsg_len) result(rc) bind(C, name='nwchemc_embed_polarizability')
    integer(c_int), intent(in) :: n_atoms
    real(c_double), intent(in) :: positions_ang(*)
    integer(c_int), intent(in) :: atomic_numbers(*)
    integer(c_int), intent(in) :: charge
    integer(c_int), intent(in) :: mult
    real(c_double), intent(out) :: energy_h
    real(c_double), intent(out) :: polarizability_au(*)
    character(kind=c_char), intent(out) :: errmsg(*)
    integer(c_int), intent(in), value :: errmsg_len
    integer(c_int) :: rc
    real(c_double) :: empty_cell(9)
    integer(c_int) :: no_cell

    empty_cell = 0.0_c_double
    no_cell = 0_c_int
    rc = nwchemc_embed_polarizability_impl(n_atoms, positions_ang, &
        atomic_numbers, empty_cell, no_cell, charge, mult, energy_h, &
        polarizability_au, errmsg, errmsg_len)
  end function nwchemc_embed_polarizability

  !> Electric polarizability response vector with an optional 3x3 cell.
  function nwchemc_embed_polarizability_cell(n_atoms, positions_ang, &
      atomic_numbers, cell_ang, has_cell, charge, mult, energy_h, &
      polarizability_au, errmsg, errmsg_len) result(rc) &
      bind(C, name='nwchemc_embed_polarizability_cell')
    integer(c_int), intent(in) :: n_atoms
    real(c_double), intent(in) :: positions_ang(*)
    integer(c_int), intent(in) :: atomic_numbers(*)
    real(c_double), intent(in) :: cell_ang(*)
    integer(c_int), intent(in) :: has_cell
    integer(c_int), intent(in) :: charge
    integer(c_int), intent(in) :: mult
    real(c_double), intent(out) :: energy_h
    real(c_double), intent(out) :: polarizability_au(*)
    character(kind=c_char), intent(out) :: errmsg(*)
    integer(c_int), intent(in), value :: errmsg_len
    integer(c_int) :: rc

    rc = nwchemc_embed_polarizability_impl(n_atoms, positions_ang, &
        atomic_numbers, cell_ang, has_cell, charge, mult, energy_h, &
        polarizability_au, errmsg, errmsg_len)
  end function nwchemc_embed_polarizability_cell

  function nwchemc_embed_polarizability_impl(n_atoms, positions_ang, &
      atomic_numbers, cell_ang, has_cell, charge, mult, energy_h, &
      polarizability_au, errmsg, errmsg_len) result(rc)
    integer(c_int), intent(in) :: n_atoms
    real(c_double), intent(in) :: positions_ang(*)
    integer(c_int), intent(in) :: atomic_numbers(*)
    real(c_double), intent(in) :: cell_ang(*)
    integer(c_int), intent(in) :: has_cell
    integer(c_int), intent(in) :: charge
    integer(c_int), intent(in) :: mult
    real(c_double), intent(out) :: energy_h
    real(c_double), intent(out) :: polarizability_au(*)
    character(kind=c_char), intent(out) :: errmsg(*)
    integer(c_int), intent(in), value :: errmsg_len
    integer(c_int) :: rc
    integer :: ok, n, i
    character(len=512) :: msg
    real(real64), allocatable :: pos(:), polarizability(:), cell(:)
    integer, allocatable :: z(:)

    rc = -1_c_int
    energy_h = 0.0_c_double
    do i = 1, 12
      polarizability_au(i) = 0.0_c_double
    end do
    call clear_c_errmsg(errmsg, errmsg_len)

    call nwchemc_embed_init()
    if (.not. rtdb_ready) then
      call set_c_errmsg(errmsg, errmsg_len, 'embed not initialized')
      return
    end if

    n = int(n_atoms)
    if (n <= 0) then
      call set_c_errmsg(errmsg, errmsg_len, 'n_atoms must be positive')
      return
    end if
    allocate (pos(3 * n), polarizability(12), cell(9), z(n))
    do i = 1, 3 * n
      pos(i) = real(positions_ang(i), kind=real64)
    end do
    do i = 1, 9
      cell(i) = real(cell_ang(i), kind=real64)
    end do
    do i = 1, 12
      polarizability(i) = 0.0_real64
    end do
    do i = 1, n
      z(i) = int(atomic_numbers(i))
    end do

    if (.not. ensure_brillouin_kvectors()) then
      call set_c_errmsg(errmsg, errmsg_len, 'brillouin kvector allocation failed')
      deallocate (pos, polarizability, cell, z)
      return
    end if
    call nwchem_legacy_polarizability(rtdb_handle, n, pos, z, cell, &
        int(has_cell), cfg_basis, cfg_theory, cfg_scf, cfg_input_blocks, &
        int(charge), max(1, int(mult)), cfg_dft_direct, cfg_dft_smear_on, &
        cfg_dft_smear_sigma, cfg_dft_smear_spinset, cfg_scf_has_options, &
        cfg_scf_maxiter, cfg_scf_thresh, cfg_scf_tol2e, &
        cfg_driver_has_options, cfg_driver_maxiter, &
        cfg_driver_tolerance_mode, cfg_driver_gmax_tol, &
        cfg_driver_grms_tol, cfg_driver_xmax_tol, cfg_driver_xrms_tol, &
        cfg_psp_count, cfg_psp_elements, cfg_psp_types, cfg_psp_names, &
        cfg_set_string_count, cfg_set_keys, cfg_set_values, &
        cfg_typed_set_count, cfg_typed_set_keys, cfg_typed_set_types, &
        cfg_typed_set_value_counts, cfg_typed_set_values, &
        cfg_brillouin_has_options, cfg_brillouin_zone_name, &
        cfg_brillouin_monkhorst_pack, cfg_brillouin_max_kpoints_print, &
        cfg_brillouin_kvector_count, cfg_brillouin_kvectors, &
        energy_h, polarizability, msg, ok)

    do i = 1, 12
      polarizability_au(i) = real(polarizability(i), kind=c_double)
    end do
    call set_c_errmsg(errmsg, errmsg_len, trim(msg))
    if (ok == 0) rc = 0_c_int
    deallocate (pos, polarizability, cell, z)
  end function nwchemc_embed_polarizability_impl

  !> Total traceless electric quadrupole (atomic units) for current config.
  function nwchemc_embed_quadrupole(n_atoms, positions_ang, atomic_numbers, &
      charge, mult, energy_h, quadrupole_au, errmsg, errmsg_len) result(rc) &
      bind(C, name='nwchemc_embed_quadrupole')
    integer(c_int), intent(in) :: n_atoms
    real(c_double), intent(in) :: positions_ang(*)
    integer(c_int), intent(in) :: atomic_numbers(*)
    integer(c_int), intent(in) :: charge
    integer(c_int), intent(in) :: mult
    real(c_double), intent(out) :: energy_h
    real(c_double), intent(out) :: quadrupole_au(*)
    character(kind=c_char), intent(out) :: errmsg(*)
    integer(c_int), intent(in), value :: errmsg_len
    integer(c_int) :: rc
    real(c_double) :: empty_cell(9)
    integer(c_int) :: no_cell

    empty_cell = 0.0_c_double
    no_cell = 0_c_int
    rc = nwchemc_embed_quadrupole_impl(n_atoms, positions_ang, &
        atomic_numbers, empty_cell, no_cell, charge, mult, energy_h, &
        quadrupole_au, errmsg, errmsg_len)
  end function nwchemc_embed_quadrupole

  !> Total traceless electric quadrupole with an optional 3x3 cell.
  function nwchemc_embed_quadrupole_cell(n_atoms, positions_ang, &
      atomic_numbers, cell_ang, has_cell, charge, mult, energy_h, &
      quadrupole_au, errmsg, errmsg_len) result(rc) &
      bind(C, name='nwchemc_embed_quadrupole_cell')
    integer(c_int), intent(in) :: n_atoms
    real(c_double), intent(in) :: positions_ang(*)
    integer(c_int), intent(in) :: atomic_numbers(*)
    real(c_double), intent(in) :: cell_ang(*)
    integer(c_int), intent(in) :: has_cell
    integer(c_int), intent(in) :: charge
    integer(c_int), intent(in) :: mult
    real(c_double), intent(out) :: energy_h
    real(c_double), intent(out) :: quadrupole_au(*)
    character(kind=c_char), intent(out) :: errmsg(*)
    integer(c_int), intent(in), value :: errmsg_len
    integer(c_int) :: rc

    rc = nwchemc_embed_quadrupole_impl(n_atoms, positions_ang, &
        atomic_numbers, cell_ang, has_cell, charge, mult, energy_h, &
        quadrupole_au, errmsg, errmsg_len)
  end function nwchemc_embed_quadrupole_cell

  function nwchemc_embed_quadrupole_impl(n_atoms, positions_ang, &
      atomic_numbers, cell_ang, has_cell, charge, mult, energy_h, &
      quadrupole_au, errmsg, errmsg_len) result(rc)
    integer(c_int), intent(in) :: n_atoms
    real(c_double), intent(in) :: positions_ang(*)
    integer(c_int), intent(in) :: atomic_numbers(*)
    real(c_double), intent(in) :: cell_ang(*)
    integer(c_int), intent(in) :: has_cell
    integer(c_int), intent(in) :: charge
    integer(c_int), intent(in) :: mult
    real(c_double), intent(out) :: energy_h
    real(c_double), intent(out) :: quadrupole_au(*)
    character(kind=c_char), intent(out) :: errmsg(*)
    integer(c_int), intent(in), value :: errmsg_len
    integer(c_int) :: rc
    integer :: ok, n, i
    character(len=512) :: msg
    real(real64), allocatable :: pos(:), quadrupole(:), cell(:)
    integer, allocatable :: z(:)

    rc = -1_c_int
    energy_h = 0.0_c_double
    do i = 1, 6
      quadrupole_au(i) = 0.0_c_double
    end do
    call clear_c_errmsg(errmsg, errmsg_len)

    call nwchemc_embed_init()
    if (.not. rtdb_ready) then
      call set_c_errmsg(errmsg, errmsg_len, 'embed not initialized')
      return
    end if

    n = int(n_atoms)
    if (n <= 0) then
      call set_c_errmsg(errmsg, errmsg_len, 'n_atoms must be positive')
      return
    end if
    allocate (pos(3 * n), quadrupole(6), cell(9), z(n))
    do i = 1, 3 * n
      pos(i) = real(positions_ang(i), kind=real64)
    end do
    do i = 1, 9
      cell(i) = real(cell_ang(i), kind=real64)
    end do
    do i = 1, 6
      quadrupole(i) = 0.0_real64
    end do
    do i = 1, n
      z(i) = int(atomic_numbers(i))
    end do

    if (.not. ensure_brillouin_kvectors()) then
      call set_c_errmsg(errmsg, errmsg_len, 'brillouin kvector allocation failed')
      deallocate (pos, quadrupole, cell, z)
      return
    end if
    call nwchem_legacy_quadrupole(rtdb_handle, n, pos, z, cell, &
        int(has_cell), cfg_basis, cfg_theory, cfg_scf, cfg_input_blocks, &
        int(charge), max(1, int(mult)), cfg_dft_direct, cfg_dft_smear_on, &
        cfg_dft_smear_sigma, cfg_dft_smear_spinset, cfg_scf_has_options, &
        cfg_scf_maxiter, cfg_scf_thresh, cfg_scf_tol2e, &
        cfg_driver_has_options, cfg_driver_maxiter, &
        cfg_driver_tolerance_mode, cfg_driver_gmax_tol, &
        cfg_driver_grms_tol, cfg_driver_xmax_tol, cfg_driver_xrms_tol, &
        cfg_psp_count, cfg_psp_elements, cfg_psp_types, cfg_psp_names, &
        cfg_set_string_count, cfg_set_keys, cfg_set_values, &
        cfg_typed_set_count, cfg_typed_set_keys, cfg_typed_set_types, &
        cfg_typed_set_value_counts, cfg_typed_set_values, &
        cfg_brillouin_has_options, cfg_brillouin_zone_name, &
        cfg_brillouin_monkhorst_pack, cfg_brillouin_max_kpoints_print, &
        cfg_brillouin_kvector_count, cfg_brillouin_kvectors, &
        energy_h, quadrupole, msg, ok)

    do i = 1, 6
      quadrupole_au(i) = real(quadrupole(i), kind=c_double)
    end do
    call set_c_errmsg(errmsg, errmsg_len, trim(msg))
    if (ok == 0) rc = 0_c_int
    deallocate (pos, quadrupole, cell, z)
  end function nwchemc_embed_quadrupole_impl

  !> Periodic stress tensor (atomic units) for current config.
  function nwchemc_embed_stress(n_atoms, positions_ang, atomic_numbers, &
      charge, mult, energy_h, stress_au, errmsg, errmsg_len) result(rc) &
      bind(C, name='nwchemc_embed_stress')
    integer(c_int), intent(in) :: n_atoms
    real(c_double), intent(in) :: positions_ang(*)
    integer(c_int), intent(in) :: atomic_numbers(*)
    integer(c_int), intent(in) :: charge
    integer(c_int), intent(in) :: mult
    real(c_double), intent(out) :: energy_h
    real(c_double), intent(out) :: stress_au(*)
    character(kind=c_char), intent(out) :: errmsg(*)
    integer(c_int), intent(in), value :: errmsg_len
    integer(c_int) :: rc
    real(c_double) :: empty_cell(9)
    integer(c_int) :: no_cell

    empty_cell = 0.0_c_double
    no_cell = 0_c_int
    rc = nwchemc_embed_stress_impl(n_atoms, positions_ang, atomic_numbers, &
        empty_cell, no_cell, charge, mult, energy_h, stress_au, errmsg, &
        errmsg_len)
  end function nwchemc_embed_stress

  !> Periodic stress tensor with an optional 3x3 cell.
  function nwchemc_embed_stress_cell(n_atoms, positions_ang, atomic_numbers, &
      cell_ang, has_cell, charge, mult, energy_h, stress_au, errmsg, &
      errmsg_len) result(rc) bind(C, name='nwchemc_embed_stress_cell')
    integer(c_int), intent(in) :: n_atoms
    real(c_double), intent(in) :: positions_ang(*)
    integer(c_int), intent(in) :: atomic_numbers(*)
    real(c_double), intent(in) :: cell_ang(*)
    integer(c_int), intent(in) :: has_cell
    integer(c_int), intent(in) :: charge
    integer(c_int), intent(in) :: mult
    real(c_double), intent(out) :: energy_h
    real(c_double), intent(out) :: stress_au(*)
    character(kind=c_char), intent(out) :: errmsg(*)
    integer(c_int), intent(in), value :: errmsg_len
    integer(c_int) :: rc

    rc = nwchemc_embed_stress_impl(n_atoms, positions_ang, atomic_numbers, &
        cell_ang, has_cell, charge, mult, energy_h, stress_au, errmsg, &
        errmsg_len)
  end function nwchemc_embed_stress_cell

  function nwchemc_embed_stress_impl(n_atoms, positions_ang, atomic_numbers, &
      cell_ang, has_cell, charge, mult, energy_h, stress_au, errmsg, &
      errmsg_len) result(rc)
    integer(c_int), intent(in) :: n_atoms
    real(c_double), intent(in) :: positions_ang(*)
    integer(c_int), intent(in) :: atomic_numbers(*)
    real(c_double), intent(in) :: cell_ang(*)
    integer(c_int), intent(in) :: has_cell
    integer(c_int), intent(in) :: charge
    integer(c_int), intent(in) :: mult
    real(c_double), intent(out) :: energy_h
    real(c_double), intent(out) :: stress_au(*)
    character(kind=c_char), intent(out) :: errmsg(*)
    integer(c_int), intent(in), value :: errmsg_len
    integer(c_int) :: rc
    integer :: ok, n, i
    character(len=512) :: msg
    real(real64), allocatable :: pos(:), stress(:), cell(:)
    integer, allocatable :: z(:)

    rc = -1_c_int
    energy_h = 0.0_c_double
    do i = 1, 9
      stress_au(i) = 0.0_c_double
    end do
    call clear_c_errmsg(errmsg, errmsg_len)

    call nwchemc_embed_init()
    if (.not. rtdb_ready) then
      call set_c_errmsg(errmsg, errmsg_len, 'embed not initialized')
      return
    end if

    n = int(n_atoms)
    if (n <= 0) then
      call set_c_errmsg(errmsg, errmsg_len, 'n_atoms must be positive')
      return
    end if
    allocate (pos(3 * n), stress(9), cell(9), z(n))
    do i = 1, 3 * n
      pos(i) = real(positions_ang(i), kind=real64)
    end do
    do i = 1, 9
      cell(i) = real(cell_ang(i), kind=real64)
      stress(i) = 0.0_real64
    end do
    do i = 1, n
      z(i) = int(atomic_numbers(i))
    end do

    if (.not. ensure_brillouin_kvectors()) then
      call set_c_errmsg(errmsg, errmsg_len, 'brillouin kvector allocation failed')
      deallocate (pos, stress, cell, z)
      return
    end if
    call nwchem_legacy_stress(rtdb_handle, n, pos, z, cell, &
        int(has_cell), cfg_basis, cfg_theory, cfg_scf, cfg_input_blocks, &
        int(charge), max(1, int(mult)), cfg_dft_direct, cfg_dft_smear_on, &
        cfg_dft_smear_sigma, cfg_dft_smear_spinset, cfg_scf_has_options, &
        cfg_scf_maxiter, cfg_scf_thresh, cfg_scf_tol2e, &
        cfg_driver_has_options, cfg_driver_maxiter, &
        cfg_driver_tolerance_mode, cfg_driver_gmax_tol, &
        cfg_driver_grms_tol, cfg_driver_xmax_tol, cfg_driver_xrms_tol, &
        cfg_psp_count, cfg_psp_elements, cfg_psp_types, cfg_psp_names, &
        cfg_set_string_count, cfg_set_keys, cfg_set_values, &
        cfg_typed_set_count, cfg_typed_set_keys, cfg_typed_set_types, &
        cfg_typed_set_value_counts, cfg_typed_set_values, &
        cfg_brillouin_has_options, cfg_brillouin_zone_name, &
        cfg_brillouin_monkhorst_pack, cfg_brillouin_max_kpoints_print, &
        cfg_brillouin_kvector_count, cfg_brillouin_kvectors, &
        energy_h, stress, msg, ok)

    do i = 1, 9
      stress_au(i) = real(stress(i), kind=c_double)
    end do
    call set_c_errmsg(errmsg, errmsg_len, trim(msg))
    if (ok == 0) rc = 0_c_int
    deallocate (pos, stress, cell, z)
  end function nwchemc_embed_stress_impl

  !> Geometry optimization; final Cartesian coordinates are Angstrom.
  function nwchemc_embed_optimize(n_atoms, positions_ang, atomic_numbers, &
      charge, mult, energy_h, optimized_positions_ang, errmsg, errmsg_len) &
      result(rc) bind(C, name='nwchemc_embed_optimize')
    integer(c_int), intent(in) :: n_atoms
    real(c_double), intent(in) :: positions_ang(*)
    integer(c_int), intent(in) :: atomic_numbers(*)
    integer(c_int), intent(in) :: charge
    integer(c_int), intent(in) :: mult
    real(c_double), intent(out) :: energy_h
    real(c_double), intent(out) :: optimized_positions_ang(*)
    character(kind=c_char), intent(out) :: errmsg(*)
    integer(c_int), intent(in), value :: errmsg_len
    integer(c_int) :: rc
    real(c_double) :: empty_cell(9)
    integer(c_int) :: no_cell

    empty_cell = 0.0_c_double
    no_cell = 0_c_int
    rc = nwchemc_embed_optimize_impl(n_atoms, positions_ang, &
        atomic_numbers, empty_cell, no_cell, charge, mult, energy_h, &
        optimized_positions_ang, errmsg, errmsg_len)
  end function nwchemc_embed_optimize

  !> Geometry optimization with an optional 3x3 cell.
  function nwchemc_embed_optimize_cell(n_atoms, positions_ang, &
      atomic_numbers, cell_ang, has_cell, charge, mult, energy_h, &
      optimized_positions_ang, errmsg, errmsg_len) result(rc) &
      bind(C, name='nwchemc_embed_optimize_cell')
    integer(c_int), intent(in) :: n_atoms
    real(c_double), intent(in) :: positions_ang(*)
    integer(c_int), intent(in) :: atomic_numbers(*)
    real(c_double), intent(in) :: cell_ang(*)
    integer(c_int), intent(in) :: has_cell
    integer(c_int), intent(in) :: charge
    integer(c_int), intent(in) :: mult
    real(c_double), intent(out) :: energy_h
    real(c_double), intent(out) :: optimized_positions_ang(*)
    character(kind=c_char), intent(out) :: errmsg(*)
    integer(c_int), intent(in), value :: errmsg_len
    integer(c_int) :: rc

    rc = nwchemc_embed_optimize_impl(n_atoms, positions_ang, &
        atomic_numbers, cell_ang, has_cell, charge, mult, energy_h, &
        optimized_positions_ang, errmsg, errmsg_len)
  end function nwchemc_embed_optimize_cell

  function nwchemc_embed_optimize_impl(n_atoms, positions_ang, &
      atomic_numbers, cell_ang, has_cell, charge, mult, energy_h, &
      optimized_positions_ang, errmsg, errmsg_len) result(rc)
    integer(c_int), intent(in) :: n_atoms
    real(c_double), intent(in) :: positions_ang(*)
    integer(c_int), intent(in) :: atomic_numbers(*)
    real(c_double), intent(in) :: cell_ang(*)
    integer(c_int), intent(in) :: has_cell
    integer(c_int), intent(in) :: charge
    integer(c_int), intent(in) :: mult
    real(c_double), intent(out) :: energy_h
    real(c_double), intent(out) :: optimized_positions_ang(*)
    character(kind=c_char), intent(out) :: errmsg(*)
    integer(c_int), intent(in), value :: errmsg_len
    integer(c_int) :: rc
    integer :: ok, n, i
    character(len=512) :: msg
    real(real64), allocatable :: pos(:), optpos(:), cell(:)
    integer, allocatable :: z(:)

    rc = -1_c_int
    energy_h = 0.0_c_double
    call clear_c_errmsg(errmsg, errmsg_len)

    call nwchemc_embed_init()
    if (.not. rtdb_ready) then
      call set_c_errmsg(errmsg, errmsg_len, 'embed not initialized')
      return
    end if

    n = int(n_atoms)
    if (n <= 0) then
      call set_c_errmsg(errmsg, errmsg_len, 'n_atoms must be positive')
      return
    end if
    allocate (pos(3 * n), optpos(3 * n), cell(9), z(n))
    do i = 1, 3 * n
      pos(i) = real(positions_ang(i), kind=real64)
      optpos(i) = pos(i)
      optimized_positions_ang(i) = 0.0_c_double
    end do
    do i = 1, 9
      cell(i) = real(cell_ang(i), kind=real64)
    end do
    do i = 1, n
      z(i) = int(atomic_numbers(i))
    end do

    if (.not. ensure_brillouin_kvectors()) then
      call set_c_errmsg(errmsg, errmsg_len, 'brillouin kvector allocation failed')
      deallocate (pos, optpos, cell, z)
      return
    end if
    call nwchem_legacy_optimize(rtdb_handle, n, pos, z, cell, &
        int(has_cell), cfg_basis, cfg_theory, cfg_scf, cfg_input_blocks, &
        int(charge), max(1, int(mult)), cfg_dft_direct, cfg_dft_smear_on, &
        cfg_dft_smear_sigma, cfg_dft_smear_spinset, cfg_scf_has_options, &
        cfg_scf_maxiter, cfg_scf_thresh, cfg_scf_tol2e, &
        cfg_driver_has_options, cfg_driver_maxiter, &
        cfg_driver_tolerance_mode, cfg_driver_gmax_tol, &
        cfg_driver_grms_tol, cfg_driver_xmax_tol, cfg_driver_xrms_tol, &
        cfg_psp_count, cfg_psp_elements, cfg_psp_types, cfg_psp_names, &
        cfg_set_string_count, cfg_set_keys, cfg_set_values, &
        cfg_typed_set_count, cfg_typed_set_keys, cfg_typed_set_types, &
        cfg_typed_set_value_counts, cfg_typed_set_values, &
        cfg_brillouin_has_options, cfg_brillouin_zone_name, &
        cfg_brillouin_monkhorst_pack, cfg_brillouin_max_kpoints_print, &
        cfg_brillouin_kvector_count, cfg_brillouin_kvectors, &
        energy_h, optpos, msg, ok)

    do i = 1, 3 * n
      optimized_positions_ang(i) = real(optpos(i), kind=c_double)
    end do
    call set_c_errmsg(errmsg, errmsg_len, trim(msg))
    if (ok == 0) rc = 0_c_int
    deallocate (pos, optpos, cell, z)
  end function nwchemc_embed_optimize_impl

  !> Harmonic vibrational frequencies in cm^-1 for current config.
  function nwchemc_embed_frequencies(n_atoms, positions_ang, atomic_numbers, &
      charge, mult, frequencies_cm1, intensities_au, errmsg, errmsg_len) &
      result(rc) bind(C, name='nwchemc_embed_frequencies')
    integer(c_int), intent(in) :: n_atoms
    real(c_double), intent(in) :: positions_ang(*)
    integer(c_int), intent(in) :: atomic_numbers(*)
    integer(c_int), intent(in) :: charge
    integer(c_int), intent(in) :: mult
    real(c_double), intent(out) :: frequencies_cm1(*)
    real(c_double), intent(out) :: intensities_au(*)
    character(kind=c_char), intent(out) :: errmsg(*)
    integer(c_int), intent(in), value :: errmsg_len
    integer(c_int) :: rc
    real(c_double) :: empty_cell(9)
    real(c_double) :: no_modes(1)
    real(c_double) :: no_projected_freq(1)
    real(c_double) :: no_projected_intensity(1)
    real(c_double) :: no_thermo(5)
    integer(c_int) :: no_cell

    empty_cell = 0.0_c_double
    no_modes = 0.0_c_double
    no_projected_freq = 0.0_c_double
    no_projected_intensity = 0.0_c_double
    no_thermo = 0.0_c_double
    no_cell = 0_c_int
    rc = nwchemc_embed_frequencies_impl(n_atoms, positions_ang, &
        atomic_numbers, empty_cell, no_cell, charge, mult, frequencies_cm1, &
        intensities_au, no_modes, 0_c_int, no_projected_freq, &
        no_projected_intensity, 0_c_int, no_thermo, 0_c_int, errmsg, &
        errmsg_len)
  end function nwchemc_embed_frequencies

  !> Harmonic vibrational frequencies and dense Cartesian normal modes.
  function nwchemc_embed_frequencies_modes(n_atoms, positions_ang, &
      atomic_numbers, charge, mult, frequencies_cm1, intensities_au, &
      normal_modes, errmsg, errmsg_len) result(rc) &
      bind(C, name='nwchemc_embed_frequencies_modes')
    integer(c_int), intent(in) :: n_atoms
    real(c_double), intent(in) :: positions_ang(*)
    integer(c_int), intent(in) :: atomic_numbers(*)
    integer(c_int), intent(in) :: charge
    integer(c_int), intent(in) :: mult
    real(c_double), intent(out) :: frequencies_cm1(*)
    real(c_double), intent(out) :: intensities_au(*)
    real(c_double), intent(out) :: normal_modes(*)
    character(kind=c_char), intent(out) :: errmsg(*)
    integer(c_int), intent(in), value :: errmsg_len
    integer(c_int) :: rc
    real(c_double) :: empty_cell(9)
    real(c_double) :: no_projected_freq(1)
    real(c_double) :: no_projected_intensity(1)
    real(c_double) :: no_thermo(5)
    integer(c_int) :: no_cell

    empty_cell = 0.0_c_double
    no_projected_freq = 0.0_c_double
    no_projected_intensity = 0.0_c_double
    no_thermo = 0.0_c_double
    no_cell = 0_c_int
    rc = nwchemc_embed_frequencies_impl(n_atoms, positions_ang, &
        atomic_numbers, empty_cell, no_cell, charge, mult, frequencies_cm1, &
        intensities_au, normal_modes, 1_c_int, no_projected_freq, &
        no_projected_intensity, 0_c_int, no_thermo, 0_c_int, errmsg, &
        errmsg_len)
  end function nwchemc_embed_frequencies_modes

  !> Harmonic vibrational frequencies with an optional 3x3 cell.
  function nwchemc_embed_frequencies_cell(n_atoms, positions_ang, &
      atomic_numbers, cell_ang, has_cell, charge, mult, frequencies_cm1, &
      intensities_au, errmsg, errmsg_len) result(rc) &
      bind(C, name='nwchemc_embed_frequencies_cell')
    integer(c_int), intent(in) :: n_atoms
    real(c_double), intent(in) :: positions_ang(*)
    integer(c_int), intent(in) :: atomic_numbers(*)
    real(c_double), intent(in) :: cell_ang(*)
    integer(c_int), intent(in) :: has_cell
    integer(c_int), intent(in) :: charge
    integer(c_int), intent(in) :: mult
    real(c_double), intent(out) :: frequencies_cm1(*)
    real(c_double), intent(out) :: intensities_au(*)
    character(kind=c_char), intent(out) :: errmsg(*)
    integer(c_int), intent(in), value :: errmsg_len
    integer(c_int) :: rc
    real(c_double) :: no_modes(1)
    real(c_double) :: no_projected_freq(1)
    real(c_double) :: no_projected_intensity(1)
    real(c_double) :: no_thermo(5)

    no_modes = 0.0_c_double
    no_projected_freq = 0.0_c_double
    no_projected_intensity = 0.0_c_double
    no_thermo = 0.0_c_double
    rc = nwchemc_embed_frequencies_impl(n_atoms, positions_ang, &
        atomic_numbers, cell_ang, has_cell, charge, mult, frequencies_cm1, &
        intensities_au, no_modes, 0_c_int, no_projected_freq, &
        no_projected_intensity, 0_c_int, no_thermo, 0_c_int, errmsg, &
        errmsg_len)
  end function nwchemc_embed_frequencies_cell

  !> Harmonic vibrational frequencies and normal modes with an optional 3x3 cell.
  function nwchemc_embed_frequencies_modes_cell(n_atoms, positions_ang, &
      atomic_numbers, cell_ang, has_cell, charge, mult, frequencies_cm1, &
      intensities_au, normal_modes, errmsg, errmsg_len) result(rc) &
      bind(C, name='nwchemc_embed_frequencies_modes_cell')
    integer(c_int), intent(in) :: n_atoms
    real(c_double), intent(in) :: positions_ang(*)
    integer(c_int), intent(in) :: atomic_numbers(*)
    real(c_double), intent(in) :: cell_ang(*)
    integer(c_int), intent(in) :: has_cell
    integer(c_int), intent(in) :: charge
    integer(c_int), intent(in) :: mult
    real(c_double), intent(out) :: frequencies_cm1(*)
    real(c_double), intent(out) :: intensities_au(*)
    real(c_double), intent(out) :: normal_modes(*)
    character(kind=c_char), intent(out) :: errmsg(*)
    integer(c_int), intent(in), value :: errmsg_len
    integer(c_int) :: rc
    real(c_double) :: no_projected_freq(1)
    real(c_double) :: no_projected_intensity(1)
    real(c_double) :: no_thermo(5)

    no_projected_freq = 0.0_c_double
    no_projected_intensity = 0.0_c_double
    no_thermo = 0.0_c_double
    rc = nwchemc_embed_frequencies_impl(n_atoms, positions_ang, &
        atomic_numbers, cell_ang, has_cell, charge, mult, frequencies_cm1, &
        intensities_au, normal_modes, 1_c_int, no_projected_freq, &
        no_projected_intensity, 0_c_int, no_thermo, 0_c_int, errmsg, &
        errmsg_len)
  end function nwchemc_embed_frequencies_modes_cell

  !> Harmonic frequencies, normal modes, and thermochemistry with an optional cell.
  function nwchemc_embed_frequencies_detail_cell(n_atoms, positions_ang, &
      atomic_numbers, cell_ang, has_cell, charge, mult, frequencies_cm1, &
      intensities_au, normal_modes, projected_frequencies_cm1, &
      projected_intensities_au, thermochemistry, errmsg, errmsg_len) &
      result(rc) bind(C, name='nwchemc_embed_frequencies_detail_cell')
    integer(c_int), intent(in) :: n_atoms
    real(c_double), intent(in) :: positions_ang(*)
    integer(c_int), intent(in) :: atomic_numbers(*)
    real(c_double), intent(in) :: cell_ang(*)
    integer(c_int), intent(in) :: has_cell
    integer(c_int), intent(in) :: charge
    integer(c_int), intent(in) :: mult
    real(c_double), intent(out) :: frequencies_cm1(*)
    real(c_double), intent(out) :: intensities_au(*)
    real(c_double), intent(out) :: normal_modes(*)
    real(c_double), intent(out) :: projected_frequencies_cm1(*)
    real(c_double), intent(out) :: projected_intensities_au(*)
    real(c_double), intent(out) :: thermochemistry(*)
    character(kind=c_char), intent(out) :: errmsg(*)
    integer(c_int), intent(in), value :: errmsg_len
    integer(c_int) :: rc

    rc = nwchemc_embed_frequencies_impl(n_atoms, positions_ang, &
        atomic_numbers, cell_ang, has_cell, charge, mult, frequencies_cm1, &
        intensities_au, normal_modes, 1_c_int, projected_frequencies_cm1, &
        projected_intensities_au, 1_c_int, thermochemistry, 1_c_int, errmsg, &
        errmsg_len)
  end function nwchemc_embed_frequencies_detail_cell

  function nwchemc_embed_frequencies_impl(n_atoms, positions_ang, &
      atomic_numbers, cell_ang, has_cell, charge, mult, frequencies_cm1, &
      intensities_au, normal_modes, read_modes, projected_frequencies_cm1, &
      projected_intensities_au, read_projected, thermochemistry, &
      read_thermo, errmsg, errmsg_len) result(rc)
    integer(c_int), intent(in) :: n_atoms
    real(c_double), intent(in) :: positions_ang(*)
    integer(c_int), intent(in) :: atomic_numbers(*)
    real(c_double), intent(in) :: cell_ang(*)
    integer(c_int), intent(in) :: has_cell
    integer(c_int), intent(in) :: charge
    integer(c_int), intent(in) :: mult
    real(c_double), intent(out) :: frequencies_cm1(*)
    real(c_double), intent(out) :: intensities_au(*)
    real(c_double), intent(out) :: normal_modes(*)
    integer(c_int), intent(in) :: read_modes
    real(c_double), intent(out) :: projected_frequencies_cm1(*)
    real(c_double), intent(out) :: projected_intensities_au(*)
    integer(c_int), intent(in) :: read_projected
    real(c_double), intent(out) :: thermochemistry(*)
    integer(c_int), intent(in) :: read_thermo
    character(kind=c_char), intent(out) :: errmsg(*)
    integer(c_int), intent(in), value :: errmsg_len
    integer(c_int) :: rc
    integer :: ok, n, ndof, mode_count, i
    character(len=512) :: msg
    real(real64) :: energy_h
    real(real64), allocatable :: pos(:), freq(:), intensity(:), modes(:), cell(:)
    real(real64), allocatable :: projected_freq(:), projected_intensity(:)
    integer, allocatable :: z(:)

    rc = -1_c_int
    last_task_energy_h = 0.0_real64
    call clear_c_errmsg(errmsg, errmsg_len)

    call nwchemc_embed_init()
    if (.not. rtdb_ready) then
      call set_c_errmsg(errmsg, errmsg_len, 'embed not initialized')
      return
    end if

    n = int(n_atoms)
    if (n <= 0) then
      call set_c_errmsg(errmsg, errmsg_len, 'n_atoms must be positive')
      return
    end if
    ndof = 3 * n
    mode_count = max(1, ndof * ndof)
    allocate (pos(ndof), freq(ndof), intensity(ndof), modes(mode_count), &
        projected_freq(ndof), projected_intensity(ndof), cell(9), z(n))
    do i = 1, ndof
      pos(i) = real(positions_ang(i), kind=real64)
      freq(i) = 0.0_real64
      intensity(i) = 0.0_real64
      frequencies_cm1(i) = 0.0_c_double
      intensities_au(i) = 0.0_c_double
      projected_freq(i) = 0.0_real64
      projected_intensity(i) = 0.0_real64
      if (read_projected /= 0_c_int) then
        projected_frequencies_cm1(i) = 0.0_c_double
        projected_intensities_au(i) = 0.0_c_double
      end if
    end do
    do i = 1, mode_count
      modes(i) = 0.0_real64
      if (read_modes /= 0_c_int) normal_modes(i) = 0.0_c_double
    end do
    do i = 1, 5
      if (read_thermo /= 0_c_int) thermochemistry(i) = 0.0_c_double
    end do
    do i = 1, 9
      cell(i) = real(cell_ang(i), kind=real64)
    end do
    do i = 1, n
      z(i) = int(atomic_numbers(i))
    end do

    if (.not. ensure_brillouin_kvectors()) then
      call set_c_errmsg(errmsg, errmsg_len, 'brillouin kvector allocation failed')
      deallocate (pos, freq, intensity, modes, projected_freq, &
          projected_intensity, cell, z)
      return
    end if
    call nwchem_legacy_frequencies(rtdb_handle, n, pos, z, cell, &
        int(has_cell), cfg_basis, cfg_theory, cfg_scf, cfg_input_blocks, &
        int(charge), max(1, int(mult)), cfg_dft_direct, cfg_dft_smear_on, &
        cfg_dft_smear_sigma, cfg_dft_smear_spinset, cfg_scf_has_options, &
        cfg_scf_maxiter, cfg_scf_thresh, cfg_scf_tol2e, &
        cfg_driver_has_options, cfg_driver_maxiter, &
        cfg_driver_tolerance_mode, cfg_driver_gmax_tol, &
        cfg_driver_grms_tol, cfg_driver_xmax_tol, cfg_driver_xrms_tol, &
        cfg_psp_count, cfg_psp_elements, cfg_psp_types, cfg_psp_names, &
        cfg_set_string_count, cfg_set_keys, cfg_set_values, &
        cfg_typed_set_count, cfg_typed_set_keys, cfg_typed_set_types, &
        cfg_typed_set_value_counts, cfg_typed_set_values, &
        cfg_brillouin_has_options, cfg_brillouin_zone_name, &
        cfg_brillouin_monkhorst_pack, cfg_brillouin_max_kpoints_print, &
        cfg_brillouin_kvector_count, cfg_brillouin_kvectors, &
        energy_h, freq, intensity, modes, int(read_modes), projected_freq, &
        projected_intensity, int(read_projected), thermochemistry, &
        int(read_thermo), msg, ok)

    do i = 1, ndof
      frequencies_cm1(i) = real(freq(i), kind=c_double)
      intensities_au(i) = real(intensity(i), kind=c_double)
      if (read_projected /= 0_c_int) then
        projected_frequencies_cm1(i) = real(projected_freq(i), kind=c_double)
        projected_intensities_au(i) = real(projected_intensity(i), kind=c_double)
      end if
    end do
    if (read_modes /= 0_c_int) then
      do i = 1, mode_count
        normal_modes(i) = real(modes(i), kind=c_double)
      end do
    end if
    call set_c_errmsg(errmsg, errmsg_len, trim(msg))
    if (ok == 0) then
      last_task_energy_h = energy_h
      rc = 0_c_int
    end if
    deallocate (pos, freq, intensity, modes, projected_freq, &
        projected_intensity, cell, z)
  end function nwchemc_embed_frequencies_impl

  !> Dense Cartesian Hessian (Hartree/Bohr**2) for current config.
  function nwchemc_embed_hessian(n_atoms, positions_ang, &
      atomic_numbers, charge, mult, hessian_h_bohr2, errmsg, &
      errmsg_len) result(rc) bind(C, name='nwchemc_embed_hessian')
    integer(c_int), intent(in) :: n_atoms
    real(c_double), intent(in) :: positions_ang(*)
    integer(c_int), intent(in) :: atomic_numbers(*)
    integer(c_int), intent(in) :: charge
    integer(c_int), intent(in) :: mult
    real(c_double), intent(out) :: hessian_h_bohr2(*)
    character(kind=c_char), intent(out) :: errmsg(*)
    integer(c_int), intent(in), value :: errmsg_len
    integer(c_int) :: rc
    real(c_double) :: empty_cell(9)
    integer(c_int) :: no_cell

    empty_cell = 0.0_c_double
    no_cell = 0_c_int
    rc = nwchemc_embed_hessian_impl(n_atoms, positions_ang, atomic_numbers, &
        empty_cell, no_cell, charge, mult, hessian_h_bohr2, errmsg, &
        errmsg_len)
  end function nwchemc_embed_hessian

  !> Dense Cartesian Hessian with an optional 3x3 cell.
  function nwchemc_embed_hessian_cell(n_atoms, positions_ang, &
      atomic_numbers, cell_ang, has_cell, charge, mult, hessian_h_bohr2, &
      errmsg, errmsg_len) result(rc) bind(C, name='nwchemc_embed_hessian_cell')
    integer(c_int), intent(in) :: n_atoms
    real(c_double), intent(in) :: positions_ang(*)
    integer(c_int), intent(in) :: atomic_numbers(*)
    real(c_double), intent(in) :: cell_ang(*)
    integer(c_int), intent(in) :: has_cell
    integer(c_int), intent(in) :: charge
    integer(c_int), intent(in) :: mult
    real(c_double), intent(out) :: hessian_h_bohr2(*)
    character(kind=c_char), intent(out) :: errmsg(*)
    integer(c_int), intent(in), value :: errmsg_len
    integer(c_int) :: rc

    rc = nwchemc_embed_hessian_impl(n_atoms, positions_ang, atomic_numbers, &
        cell_ang, has_cell, charge, mult, hessian_h_bohr2, errmsg, errmsg_len)
  end function nwchemc_embed_hessian_cell

  function nwchemc_embed_hessian_impl(n_atoms, positions_ang, &
      atomic_numbers, cell_ang, has_cell, charge, mult, hessian_h_bohr2, &
      errmsg, errmsg_len) result(rc)
    integer(c_int), intent(in) :: n_atoms
    real(c_double), intent(in) :: positions_ang(*)
    integer(c_int), intent(in) :: atomic_numbers(*)
    real(c_double), intent(in) :: cell_ang(*)
    integer(c_int), intent(in) :: has_cell
    integer(c_int), intent(in) :: charge
    integer(c_int), intent(in) :: mult
    real(c_double), intent(out) :: hessian_h_bohr2(*)
    character(kind=c_char), intent(out) :: errmsg(*)
    integer(c_int), intent(in), value :: errmsg_len
    integer(c_int) :: rc
    integer :: ok, n, n2, i
    character(len=512) :: msg
    real(real64) :: energy_h
    real(real64), allocatable :: pos(:), hess(:), cell(:)
    integer, allocatable :: z(:)

    rc = -1_c_int
    last_task_energy_h = 0.0_real64
    call clear_c_errmsg(errmsg, errmsg_len)

    call nwchemc_embed_init()
    if (.not. rtdb_ready) then
      call set_c_errmsg(errmsg, errmsg_len, 'embed not initialized')
      return
    end if

    n = int(n_atoms)
    if (n <= 0) then
      call set_c_errmsg(errmsg, errmsg_len, 'n_atoms must be positive')
      return
    end if
    n2 = (3 * n) * (3 * n)
    allocate (pos(3 * n), hess(n2), cell(9), z(n))
    do i = 1, 3 * n
      pos(i) = real(positions_ang(i), kind=real64)
    end do
    do i = 1, 9
      cell(i) = real(cell_ang(i), kind=real64)
    end do
    do i = 1, n2
      hess(i) = 0.0_real64
    end do
    do i = 1, n
      z(i) = int(atomic_numbers(i))
    end do

    if (.not. ensure_brillouin_kvectors()) then
      call set_c_errmsg(errmsg, errmsg_len, 'brillouin kvector allocation failed')
      return
    end if
    call nwchem_legacy_hessian(rtdb_handle, n, pos, z, cell, &
        int(has_cell), cfg_basis, cfg_theory, cfg_scf, cfg_input_blocks, &
        int(charge), max(1, int(mult)), cfg_dft_direct, cfg_dft_smear_on, &
        cfg_dft_smear_sigma, cfg_dft_smear_spinset, cfg_scf_has_options, &
        cfg_scf_maxiter, cfg_scf_thresh, cfg_scf_tol2e, &
        cfg_driver_has_options, cfg_driver_maxiter, &
        cfg_driver_tolerance_mode, cfg_driver_gmax_tol, &
        cfg_driver_grms_tol, cfg_driver_xmax_tol, cfg_driver_xrms_tol, &
        cfg_psp_count, cfg_psp_elements, cfg_psp_types, cfg_psp_names, &
        cfg_set_string_count, cfg_set_keys, cfg_set_values, &
        cfg_typed_set_count, cfg_typed_set_keys, cfg_typed_set_types, &
        cfg_typed_set_value_counts, cfg_typed_set_values, &
        cfg_brillouin_has_options, cfg_brillouin_zone_name, &
        cfg_brillouin_monkhorst_pack, cfg_brillouin_max_kpoints_print, &
        cfg_brillouin_kvector_count, cfg_brillouin_kvectors, &
        energy_h, hess, msg, ok)

    do i = 1, n2
      hessian_h_bohr2(i) = real(hess(i), kind=c_double)
    end do
    call set_c_errmsg(errmsg, errmsg_len, trim(msg))
    if (ok == 0) then
      last_task_energy_h = energy_h
      rc = 0_c_int
    end if
    deallocate (pos, hess, cell, z)
  end function nwchemc_embed_hessian_impl

  ! --- helpers (private) ---

  logical function ensure_brillouin_kvectors()
    integer :: alloc_status
    ensure_brillouin_kvectors = .true.
    if (allocated(cfg_brillouin_kvectors)) return
    allocate (cfg_brillouin_kvectors(1), stat=alloc_status)
    if (alloc_status /= 0) then
      ensure_brillouin_kvectors = .false.
      return
    end if
    cfg_brillouin_kvectors = 0.0_real64
  end function ensure_brillouin_kvectors

  subroutine c_chars_to_f(cbuf, clen, fstr)
    character(kind=c_char), intent(in) :: cbuf(*)
    integer(c_int), intent(in) :: clen
    character(len=*), intent(out) :: fstr
    integer :: i, n
    fstr = ' '
    n = min(int(clen), len(fstr))
    if (n <= 0) return
    do i = 1, n
      if (cbuf(i) == c_null_char) exit
      fstr(i:i) = transfer(cbuf(i), 'a')
    end do
  end subroutine c_chars_to_f

  subroutine clear_c_errmsg(errmsg, errmsg_len)
    character(kind=c_char), intent(out) :: errmsg(*)
    integer(c_int), intent(in) :: errmsg_len
    integer :: i, n
    n = max(0, int(errmsg_len))
    do i = 1, n
      errmsg(i) = c_null_char
    end do
  end subroutine clear_c_errmsg

  subroutine set_c_errmsg(errmsg, errmsg_len, msg)
    character(kind=c_char), intent(out) :: errmsg(*)
    integer(c_int), intent(in) :: errmsg_len
    character(len=*), intent(in) :: msg
    integer :: i, n, m
    if (errmsg_len <= 0_c_int) return
    call clear_c_errmsg(errmsg, errmsg_len)
    n = max(0, int(errmsg_len) - 1)
    m = min(len_trim(msg), n)
    do i = 1, m
      errmsg(i) = transfer(msg(i:i), c_null_char)
    end do
    if (n >= 0) errmsg(m + 1) = c_null_char
  end subroutine set_c_errmsg

end module nwchem_embed_c_api
