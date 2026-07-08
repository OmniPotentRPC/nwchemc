! SPDX-License-Identifier: MIT
!
! Decode NWChemParams with public capnp-fortran and apply all exploded-setter
! families into embed config state (no C multi-arg marshaling on that path).

module nwchemc_embed_apply_params_mod
  use, intrinsic :: iso_c_binding, only: &
      c_char, c_double, c_int, c_ptr, c_size_t, c_null_char, &
      c_associated, c_f_pointer
  use, intrinsic :: iso_fortran_env, only: int8, int64, real64
  use capnp
  use potentials_capnp
  implicit none
  private

  public :: nwchemc_embed_apply_params
  public :: nwchemc_embed_set_scf_direct
  public :: nwchemc_embed_get_scf_direct
  public :: nwchemc_embed_get_config
  public :: nwchemc_embed_get_dft_direct
  public :: nwchemc_embed_get_driver_direct
  public :: nwchemc_embed_get_nwpw_direct
  public :: nwchemc_embed_get_input_blocks
  public :: applied_scf_has_options, applied_scf_maxiter
  public :: applied_scf_thresh, applied_scf_tol2e
  public :: applied_basis, applied_theory, applied_scf_type
  public :: applied_input_blocks, applied_charge, applied_mult
  public :: applied_dft_xc, applied_dft_direct, applied_dft_smear_on
  public :: applied_dft_smear_sigma, applied_dft_smear_spinset
  public :: applied_driver_has_options, applied_driver_maxiter
  public :: applied_driver_tolerance_mode
  public :: applied_driver_gmax_tol, applied_driver_grms_tol
  public :: applied_driver_xmax_tol, applied_driver_xrms_tol
  public :: applied_nwpw_has_options, applied_nwpw_energy_cutoff
  public :: applied_nwpw_wavefunction_cutoff, applied_nwpw_ewald_rcut
  public :: applied_nwpw_ewald_ncut
  public :: applied_basis_library_root, applied_basis_angular_kind
  public :: applied_basis_segment_mode, applied_basis_legacy_spherical
  public :: applied_basis_ecp
  public :: applied_brillouin_has_options, applied_brillouin_zone_name
  public :: applied_brillouin_monkhorst_pack, applied_brillouin_max_kpoints_print
  public :: applied_brillouin_kvector_count, applied_brillouin_kvectors
  public :: applied_brillouin_dos_zone_count
  public :: applied_brillouin_dos_zone_names, applied_brillouin_dos_zone_grids
  public :: applied_psp_count, applied_psp_elements, applied_psp_names
  public :: applied_psp_types
  public :: applied_set_string_count, applied_set_keys, applied_set_values
  public :: applied_typed_set_count, applied_typed_set_keys
  public :: applied_typed_set_types, applied_typed_set_value_counts
  public :: applied_typed_set_values

  character(len=64), save :: applied_basis = 'sto-3g'
  character(len=64), save :: applied_theory = 'scf'
  character(len=64), save :: applied_scf_type = 'rhf'
  character(len=4096), save :: applied_input_blocks = ' '
  integer, save :: applied_charge = 0
  integer, save :: applied_mult = 1
  character(len=64), save :: applied_dft_xc = ' '
  integer, save :: applied_dft_direct = 0
  integer, save :: applied_dft_smear_on = 0
  real(real64), save :: applied_dft_smear_sigma = 0.0_real64
  integer, save :: applied_dft_smear_spinset = 1
  integer, save :: applied_scf_has_options = 0
  integer, save :: applied_scf_maxiter = 0
  real(real64), save :: applied_scf_thresh = 0.0_real64
  real(real64), save :: applied_scf_tol2e = 0.0_real64
  integer, save :: applied_driver_has_options = 0
  integer, save :: applied_driver_maxiter = 0
  integer, save :: applied_driver_tolerance_mode = 0
  real(real64), save :: applied_driver_gmax_tol = 0.0_real64
  real(real64), save :: applied_driver_grms_tol = 0.0_real64
  real(real64), save :: applied_driver_xmax_tol = 0.0_real64
  real(real64), save :: applied_driver_xrms_tol = 0.0_real64
  integer, save :: applied_nwpw_has_options = 0
  real(real64), save :: applied_nwpw_energy_cutoff = 0.0_real64
  real(real64), save :: applied_nwpw_wavefunction_cutoff = 0.0_real64
  real(real64), save :: applied_nwpw_ewald_rcut = 0.0_real64
  integer, save :: applied_nwpw_ewald_ncut = 0
  integer, save :: applied_basis_library_root = 0
  integer, save :: applied_basis_angular_kind = 0
  integer, save :: applied_basis_segment_mode = 0
  integer, save :: applied_basis_legacy_spherical = 0
  character(len=64), save :: applied_basis_ecp = ' '
  integer, save :: applied_brillouin_has_options = 0
  character(len=64), save :: applied_brillouin_zone_name = 'zone_default'
  integer, save :: applied_brillouin_monkhorst_pack(3) = 0
  integer, save :: applied_brillouin_max_kpoints_print = 0
  integer, save :: applied_brillouin_kvector_count = 0
  real(real64), allocatable, save :: applied_brillouin_kvectors(:)
  integer, save :: applied_brillouin_dos_zone_count = 0
  character(len=64), save :: applied_brillouin_dos_zone_names(512) = ' '
  integer, save :: applied_brillouin_dos_zone_grids(3, 512) = 0
  integer, parameter :: max_psp = 64
  integer, parameter :: psp_element_len = 16
  integer, parameter :: psp_name_len = 256
  integer, save :: applied_psp_count = 0
  character(len=psp_element_len), save :: applied_psp_elements(max_psp) = ' '
  character(len=psp_name_len), save :: applied_psp_names(max_psp) = ' '
  integer, save :: applied_psp_types(max_psp) = 0
  integer, parameter :: max_sets = 512
  integer, parameter :: max_set_vals = 64
  integer, parameter :: set_key_len = 128
  integer, parameter :: set_value_len = 256
  integer, save :: applied_set_string_count = 0
  character(len=set_key_len), save :: applied_set_keys(max_sets) = ' '
  character(len=set_value_len), save :: applied_set_values(max_sets) = ' '
  integer, save :: applied_typed_set_count = 0
  character(len=set_key_len), save :: applied_typed_set_keys(max_sets) = ' '
  integer, save :: applied_typed_set_types(max_sets) = 0
  integer, save :: applied_typed_set_value_counts(max_sets) = 0
  character(len=set_value_len), save :: &
      applied_typed_set_values(max_set_vals, max_sets) = ' '
  integer, parameter :: DRIVER_TOL_NONE = 0
  integer, parameter :: DRIVER_TOL_TIGHT = 1
  integer, parameter :: DRIVER_TOL_LOOSE = 2

contains

  subroutine reset_applied_state()
    applied_basis = 'sto-3g'
    applied_theory = 'scf'
    applied_scf_type = 'rhf'
    applied_input_blocks = ' '
    applied_charge = 0
    applied_mult = 1
    applied_dft_xc = ' '
    applied_dft_direct = 0
    applied_dft_smear_on = 0
    applied_dft_smear_sigma = 0.0_real64
    applied_dft_smear_spinset = 1
    applied_scf_has_options = 0
    applied_scf_maxiter = 0
    applied_scf_thresh = 0.0_real64
    applied_scf_tol2e = 0.0_real64
    applied_driver_has_options = 0
    applied_driver_maxiter = 0
    applied_driver_tolerance_mode = DRIVER_TOL_NONE
    applied_driver_gmax_tol = 0.0_real64
    applied_driver_grms_tol = 0.0_real64
    applied_driver_xmax_tol = 0.0_real64
    applied_driver_xrms_tol = 0.0_real64
    applied_nwpw_has_options = 0
    applied_nwpw_energy_cutoff = 0.0_real64
    applied_nwpw_wavefunction_cutoff = 0.0_real64
    applied_nwpw_ewald_rcut = 0.0_real64
    applied_nwpw_ewald_ncut = 0
    applied_basis_library_root = 0
    applied_basis_angular_kind = 0
    applied_basis_segment_mode = 0
    applied_basis_legacy_spherical = 0
    applied_basis_ecp = ' '
    applied_brillouin_has_options = 0
    applied_brillouin_zone_name = 'zone_default'
    applied_brillouin_monkhorst_pack = 0
    applied_brillouin_max_kpoints_print = 0
    applied_brillouin_kvector_count = 0
    if (allocated(applied_brillouin_kvectors)) deallocate (applied_brillouin_kvectors)
    applied_brillouin_dos_zone_count = 0
    applied_brillouin_dos_zone_names = ' '
    applied_brillouin_dos_zone_grids = 0
    applied_psp_count = 0
    applied_psp_elements = ' '
    applied_psp_names = ' '
    applied_psp_types = 0
    applied_set_string_count = 0
    applied_set_keys = ' '
    applied_set_values = ' '
    applied_typed_set_count = 0
    applied_typed_set_keys = ' '
    applied_typed_set_types = 0
    applied_typed_set_value_counts = 0
    applied_typed_set_values = ' '
  end subroutine reset_applied_state

  subroutine copy_alloc_text(src, dst, maxlen)
    character(len=:), allocatable, intent(in) :: src
    character(len=*), intent(out) :: dst
    integer, intent(in) :: maxlen
    integer :: n
    dst = ' '
    if (.not. allocated(src)) return
    n = min(len_trim(src), maxlen)
    if (n > 0) dst(1:n) = src(1:n)
  end subroutine copy_alloc_text

  subroutine f_to_c_chars(fstr, cbuf, clen)
    character(len=*), intent(in) :: fstr
    character(kind=c_char), intent(out) :: cbuf(*)
    integer(c_int), intent(in) :: clen
    integer :: i, n
    n = min(len_trim(fstr), max(0, int(clen) - 1))
    do i = 1, n
      cbuf(i) = fstr(i:i)
    end do
    if (clen > 0) cbuf(n + 1) = c_null_char
  end subroutine f_to_c_chars

  !> Apply all exploded-setter families from framed NWChemParams.
  !> input_blocks is C embed-rendered text (promoted stanzas stripped).
  function nwchemc_embed_apply_params(params_capnp, params_capnp_size, &
      input_blocks, input_blocks_len) result(rc) &
      bind(C, name='nwchemc_embed_apply_params')
    type(c_ptr), intent(in), value :: params_capnp
    integer(c_size_t), intent(in), value :: params_capnp_size
    character(kind=c_char), intent(in) :: input_blocks(*)
    integer(c_int), intent(in), value :: input_blocks_len
    integer(c_int) :: rc
    integer(int8), pointer :: raw(:)
    integer(int8), allocatable :: bytes(:)
    type(capnp_message_t), target :: msg
    type(n_w_chem_params_t) :: params
    type(n_w_chem_input_stanza_t) :: stanza
    type(capnp_ptr_t) :: stanzas
    character(len=:), allocatable :: s
    integer :: err, i, n, kind, ib
    integer(int64) :: n64

    rc = -1_c_int
    call reset_applied_state()
    if (input_blocks_len > 0) then
      n = min(int(input_blocks_len), len(applied_input_blocks))
      do ib = 1, n
        if (input_blocks(ib) == c_null_char) exit
        applied_input_blocks(ib:ib) = input_blocks(ib)
      end do
    end if
    if (.not. c_associated(params_capnp) .or. params_capnp_size <= 0) return
    n = int(params_capnp_size)
    call c_f_pointer(params_capnp, raw, [n])
    allocate (bytes(0:n - 1))
    bytes(0:n - 1) = raw(1:n)
    call capnp_deserialize_bytes(bytes, msg, err)
    if (err /= CAPNP_OK) then
      deallocate (bytes)
      return
    end if
    params = n_w_chem_params_read_root(msg, err)
    if (err /= CAPNP_OK) then
      call capnp_message_free(msg)
      deallocate (bytes)
      return
    end if
    call n_w_chem_params_basis_get(params, s, err)
    if (err == CAPNP_OK) call copy_alloc_text(s, applied_basis, 64)
    if (len_trim(applied_basis) == 0) applied_basis = 'sto-3g'
    call n_w_chem_params_theory_get(params, s, err)
    if (err == CAPNP_OK) call copy_alloc_text(s, applied_theory, 64)
    if (len_trim(applied_theory) == 0) applied_theory = 'scf'
    call n_w_chem_params_scf_type_get(params, s, err)
    if (err == CAPNP_OK) call copy_alloc_text(s, applied_scf_type, 64)
    if (len_trim(applied_scf_type) == 0) applied_scf_type = 'rhf'
    applied_charge = int(n_w_chem_params_charge_get(params))
    applied_mult = max(1, int(n_w_chem_params_multiplicity_get(params)))
    stanzas = n_w_chem_params_input_stanzas_get(params, err)
    n64 = 0_int64
    if (err == CAPNP_OK) n64 = capnp_list_len(stanzas)
    do i = 0, int(n64) - 1
      stanza = n_w_chem_params_input_stanzas_get_elem(params, i, err)
      if (err /= CAPNP_OK) cycle
      kind = n_w_chem_input_stanza_kind_get(stanza)
      select case (kind)
      case (N_W_CHEM_INPUT_STANZA_KIND_SCF_E)
        call apply_scf_stanza(stanza)
      case (N_W_CHEM_INPUT_STANZA_KIND_DFT_E)
        call apply_dft_stanza(stanza)
      case (N_W_CHEM_INPUT_STANZA_KIND_DRIVER_E)
        call apply_driver_stanza(stanza)
      case (N_W_CHEM_INPUT_STANZA_KIND_NWPW_E)
        call apply_nwpw_stanza(stanza)
      case (N_W_CHEM_INPUT_STANZA_KIND_BASIS_E)
        call apply_basis_stanza(stanza)
      case (N_W_CHEM_INPUT_STANZA_KIND_BRILLOUIN_ZONE_E)
        call apply_brillouin_stanza(stanza)
      case (N_W_CHEM_INPUT_STANZA_KIND_PSEUDOPOTENTIAL_E)
        call apply_psp_stanza(stanza)
      case (N_W_CHEM_INPUT_STANZA_KIND_SET_E)
        call apply_set_stanza(stanza)
      end select
    end do
    if (len_trim(applied_dft_xc) > 0) then
      if (index(applied_theory, 'dft') == 1 .or. &
          index(applied_theory, 'tddft') == 1 .or. &
          index(applied_theory, 'sodft') == 1) then
        applied_scf_type = applied_dft_xc
      end if
    end if
    call capnp_message_free(msg)
    deallocate (bytes)
    rc = 0_c_int
  end function nwchemc_embed_apply_params

  subroutine apply_scf_stanza(stanza)
    type(n_w_chem_input_stanza_t), intent(in) :: stanza
    type(n_w_chem_scf_stanza_t) :: scf
    integer :: err, maxiter
    real(real64) :: thresh, tol2e
    scf = n_w_chem_input_stanza_scf_get(stanza, err)
    if (err /= CAPNP_OK .or. scf%p%kind == CAPNP_PK_NULL) return
    maxiter = int(n_w_chem_scf_stanza_maxiter_get(scf))
    thresh = n_w_chem_scf_stanza_thresh_get(scf)
    tol2e = n_w_chem_scf_stanza_tol2e_get(scf)
    if (maxiter > 0) then
      applied_scf_has_options = 1
      applied_scf_maxiter = maxiter
    end if
    if (thresh > 0.0_real64) then
      applied_scf_has_options = 1
      applied_scf_thresh = thresh
    end if
    if (tol2e > 0.0_real64) then
      applied_scf_has_options = 1
      applied_scf_tol2e = tol2e
    end if
  end subroutine apply_scf_stanza

  subroutine apply_dft_stanza(stanza)
    type(n_w_chem_input_stanza_t), intent(in) :: stanza
    type(n_w_chem_dft_stanza_t) :: dft
    type(n_w_chem_dft_smearing_t) :: smear
    character(len=:), allocatable :: xc
    integer :: err, mode
    dft = n_w_chem_input_stanza_dft_get(stanza, err)
    if (err /= CAPNP_OK .or. dft%p%kind == CAPNP_PK_NULL) return
    call n_w_chem_dft_stanza_xc_get(dft, xc, err)
    if (err == CAPNP_OK) call copy_alloc_text(xc, applied_dft_xc, 64)
    if (n_w_chem_dft_stanza_direct_get(dft)) applied_dft_direct = 1
    smear = n_w_chem_dft_stanza_smearing_get(dft, err)
    if (err == CAPNP_OK .and. smear%p%kind /= CAPNP_PK_NULL) then
      if (n_w_chem_dft_smearing_sigma_hartree_get(smear) /= 0.0_real64) then
        applied_dft_smear_on = 1
        applied_dft_smear_sigma = n_w_chem_dft_smearing_sigma_hartree_get(smear)
        mode = n_w_chem_dft_smearing_mode_get(smear)
        if (mode == N_W_CHEM_DFT_SMEARING_MODE_NOFIXSZ_E) then
          applied_dft_smear_spinset = 0
        else
          applied_dft_smear_spinset = 1
        end if
      end if
    end if
  end subroutine apply_dft_stanza

  subroutine apply_driver_stanza(stanza)
    type(n_w_chem_input_stanza_t), intent(in) :: stanza
    type(n_w_chem_driver_stanza_t) :: drv
    integer :: err, maxiter
    real(real64) :: gmax, grms, xmax, xrms
    drv = n_w_chem_input_stanza_driver_get(stanza, err)
    if (err /= CAPNP_OK .or. drv%p%kind == CAPNP_PK_NULL) return
    maxiter = int(n_w_chem_driver_stanza_maxiter_get(drv))
    if (maxiter > 0) then
      applied_driver_has_options = 1
      applied_driver_maxiter = maxiter
    end if
    if (n_w_chem_driver_stanza_tight_get(drv)) then
      applied_driver_has_options = 1
      applied_driver_tolerance_mode = DRIVER_TOL_TIGHT
    end if
    if (n_w_chem_driver_stanza_loose_get(drv)) then
      applied_driver_has_options = 1
      applied_driver_tolerance_mode = DRIVER_TOL_LOOSE
    end if
    gmax = n_w_chem_driver_stanza_gmax_tol_get(drv)
    grms = n_w_chem_driver_stanza_grms_tol_get(drv)
    xmax = n_w_chem_driver_stanza_xmax_tol_get(drv)
    xrms = n_w_chem_driver_stanza_xrms_tol_get(drv)
    if (gmax > 0.0_real64) then
      applied_driver_has_options = 1
      applied_driver_gmax_tol = gmax
    end if
    if (grms > 0.0_real64) then
      applied_driver_has_options = 1
      applied_driver_grms_tol = grms
    end if
    if (xmax > 0.0_real64) then
      applied_driver_has_options = 1
      applied_driver_xmax_tol = xmax
    end if
    if (xrms > 0.0_real64) then
      applied_driver_has_options = 1
      applied_driver_xrms_tol = xrms
    end if
  end subroutine apply_driver_stanza

  subroutine apply_nwpw_stanza(stanza)
    type(n_w_chem_input_stanza_t), intent(in) :: stanza
    type(n_w_chem_nwpw_stanza_t) :: nwpw
    integer :: err, ncut
    real(real64) :: ecut, wcut, rcut
    real(real64) :: alias_ecut, alias_wcut
    nwpw = n_w_chem_input_stanza_nwpw_get(stanza, err)
    if (err /= CAPNP_OK .or. nwpw%p%kind == CAPNP_PK_NULL) return
    ! Primary fields (energyCutoff / wavefunctionCutoff @0/@1).
    ecut = n_w_chem_nwpw_stanza_energy_cutoff_get(nwpw)
    wcut = n_w_chem_nwpw_stanza_wavefunction_cutoff_get(nwpw)
    rcut = n_w_chem_nwpw_stanza_ewald_rcut_get(nwpw)
    ncut = int(n_w_chem_nwpw_stanza_ewald_ncut_get(nwpw))
    ! Alias fields (cutoffWavefunction / cutoffEnergy @144/@145) match C
    ! extract_direct_nwpw: alias overrides primary when present.
    alias_wcut = n_w_chem_nwpw_stanza_cutoff_wavefunction_get(nwpw)
    alias_ecut = n_w_chem_nwpw_stanza_cutoff_energy_get(nwpw)
    if (alias_wcut > 0.0_real64) then
      wcut = alias_wcut
      if (alias_ecut > 0.0_real64) then
        ecut = alias_ecut
      else
        ecut = 2.0_real64 * alias_wcut
      end if
    end if
    if (ecut > 0.0_real64 .or. wcut > 0.0_real64 .or. rcut > 0.0_real64 .or. &
        ncut > 0) then
      applied_nwpw_has_options = 1
      applied_nwpw_energy_cutoff = ecut
      applied_nwpw_wavefunction_cutoff = wcut
      applied_nwpw_ewald_rcut = rcut
      applied_nwpw_ewald_ncut = ncut
    end if
  end subroutine apply_nwpw_stanza

  subroutine apply_basis_stanza(stanza)
    type(n_w_chem_input_stanza_t), intent(in) :: stanza
    type(n_w_chem_basis_stanza_t) :: basis
    character(len=:), allocatable :: ecp
    integer :: err
    basis = n_w_chem_input_stanza_basis_stanza_get(stanza, err)
    if (err /= CAPNP_OK .or. basis%p%kind == CAPNP_PK_NULL) return
    applied_basis_library_root = n_w_chem_basis_stanza_library_root_get(basis)
    applied_basis_angular_kind = n_w_chem_basis_stanza_angular_kind_get(basis)
    applied_basis_segment_mode = n_w_chem_basis_stanza_segment_mode_get(basis)
    if (n_w_chem_basis_stanza_spherical_get(basis)) then
      applied_basis_legacy_spherical = 1
    else
      applied_basis_legacy_spherical = 0
    end if
    call n_w_chem_basis_stanza_ecp_get(basis, ecp, err)
    if (err == CAPNP_OK) call copy_alloc_text(ecp, applied_basis_ecp, 64)
  end subroutine apply_basis_stanza

  subroutine apply_brillouin_stanza(stanza)
    type(n_w_chem_input_stanza_t), intent(in) :: stanza
    type(n_w_chem_brillouin_zone_stanza_t) :: bz
    type(n_w_chem_k_vector_t) :: kv
    type(capnp_ptr_t) :: klist
    character(len=:), allocatable :: zname, dos_name
    integer :: err, i, nkv, nvals, mx, my, mz, alloc_status
    real(real64) :: x, y, z, ww
    bz = n_w_chem_input_stanza_brillouin_zone_get(stanza, err)
    if (err /= CAPNP_OK .or. bz%p%kind == CAPNP_PK_NULL) return
    call n_w_chem_brillouin_zone_stanza_zone_name_get(bz, zname, err)
    if (err == CAPNP_OK) call copy_alloc_text(zname, applied_brillouin_zone_name, 64)
    if (len_trim(applied_brillouin_zone_name) == 0) &
        applied_brillouin_zone_name = 'zone_default'
    mx = int(n_w_chem_brillouin_zone_stanza_monkhorst_pack_x_get(bz))
    my = int(n_w_chem_brillouin_zone_stanza_monkhorst_pack_y_get(bz))
    mz = int(n_w_chem_brillouin_zone_stanza_monkhorst_pack_z_get(bz))
    applied_brillouin_monkhorst_pack = [mx, my, mz]
    applied_brillouin_max_kpoints_print = &
        int(n_w_chem_brillouin_zone_stanza_max_kpoints_print_get(bz))
    klist = n_w_chem_brillouin_zone_stanza_k_vectors_get(bz, err)
    nkv = 0
    if (err == CAPNP_OK) nkv = int(capnp_list_len(klist))
    applied_brillouin_kvector_count = nkv
    nvals = 4 * nkv
    if (allocated(applied_brillouin_kvectors)) deallocate (applied_brillouin_kvectors)
    allocate (applied_brillouin_kvectors(max(1, nvals)), stat=alloc_status)
    if (alloc_status /= 0) return
    applied_brillouin_kvectors = 0.0_real64
    do i = 0, nkv - 1
      kv = n_w_chem_brillouin_zone_stanza_k_vectors_get_elem(bz, i, err)
      if (err /= CAPNP_OK) cycle
      x = n_w_chem_k_vector_x_get(kv)
      y = n_w_chem_k_vector_y_get(kv)
      z = n_w_chem_k_vector_z_get(kv)
      ww = n_w_chem_k_vector_weight_get(kv)
      applied_brillouin_kvectors(4 * i + 1) = x
      applied_brillouin_kvectors(4 * i + 2) = y
      applied_brillouin_kvectors(4 * i + 3) = z
      applied_brillouin_kvectors(4 * i + 4) = ww
    end do
    if (mx > 0 .or. my > 0 .or. mz > 0 .or. nkv > 0 .or. &
        applied_brillouin_max_kpoints_print > 0) &
        applied_brillouin_has_options = 1
    call n_w_chem_brillouin_zone_stanza_dos_grid_zone_name_get(bz, dos_name, err)
    mx = int(n_w_chem_brillouin_zone_stanza_dos_grid_x_get(bz))
    my = int(n_w_chem_brillouin_zone_stanza_dos_grid_y_get(bz))
    mz = int(n_w_chem_brillouin_zone_stanza_dos_grid_z_get(bz))
    if (err == CAPNP_OK .and. allocated(dos_name)) then
      if (len_trim(dos_name) > 0 .and. (mx > 0 .or. my > 0 .or. mz > 0)) then
        applied_brillouin_dos_zone_count = 1
        call copy_alloc_text(dos_name, applied_brillouin_dos_zone_names(1), 64)
        applied_brillouin_dos_zone_grids(:, 1) = [mx, my, mz]
      end if
    end if
  end subroutine apply_brillouin_stanza

  subroutine apply_psp_stanza(stanza)
    type(n_w_chem_input_stanza_t), intent(in) :: stanza
    type(n_w_chem_pseudopotential_stanza_t) :: psp
    type(n_w_chem_pseudopotential_entry_t) :: ent
    type(capnp_ptr_t) :: ents
    character(len=:), allocatable :: elem, lname
    integer :: err, i, n
    psp = n_w_chem_input_stanza_pseudopotential_get(stanza, err)
    if (err /= CAPNP_OK .or. psp%p%kind == CAPNP_PK_NULL) return
    ents = n_w_chem_pseudopotential_stanza_entries_get(psp, err)
    if (err /= CAPNP_OK) return
    n = min(int(capnp_list_len(ents)), max_psp)
    applied_psp_count = n
    do i = 0, n - 1
      ent = n_w_chem_pseudopotential_stanza_entries_get_elem(psp, i, err)
      if (err /= CAPNP_OK) cycle
      call n_w_chem_pseudopotential_entry_element_get(ent, elem, err)
      if (err == CAPNP_OK) &
          call copy_alloc_text(elem, applied_psp_elements(i + 1), psp_element_len)
      call n_w_chem_pseudopotential_entry_library_name_get(ent, lname, err)
      if (err == CAPNP_OK) &
          call copy_alloc_text(lname, applied_psp_names(i + 1), psp_name_len)
      applied_psp_types(i + 1) = &
          n_w_chem_pseudopotential_entry_library_type_get(ent)
    end do
  end subroutine apply_psp_stanza

  subroutine apply_set_stanza(stanza)
    type(n_w_chem_input_stanza_t), intent(in) :: stanza
    type(n_w_chem_set_directive_t) :: sd
    character(len=:), allocatable :: key, val, elem
    type(capnp_ptr_t) :: vals
    integer :: err, vtype, nvals, j, idx
    sd = n_w_chem_input_stanza_set_get(stanza, err)
    if (err /= CAPNP_OK .or. sd%p%kind == CAPNP_PK_NULL) return
    call n_w_chem_set_directive_key_get(sd, key, err)
    if (err /= CAPNP_OK .or. .not. allocated(key)) return
    if (len_trim(key) == 0) return
    vtype = n_w_chem_set_directive_value_type_get(sd)
    vals = n_w_chem_set_directive_values_get(sd, err)
    nvals = 0
    if (err == CAPNP_OK) nvals = int(capnp_list_len(vals))
    if (nvals <= 0) then
      call n_w_chem_set_directive_value_get(sd, val, err)
      if (err /= CAPNP_OK) return
      if (applied_set_string_count >= max_sets) return
      applied_set_string_count = applied_set_string_count + 1
      idx = applied_set_string_count
      call copy_alloc_text(key, applied_set_keys(idx), set_key_len)
      call copy_alloc_text(val, applied_set_values(idx), set_value_len)
      return
    end if
    if (applied_typed_set_count >= max_sets) return
    applied_typed_set_count = applied_typed_set_count + 1
    idx = applied_typed_set_count
    call copy_alloc_text(key, applied_typed_set_keys(idx), set_key_len)
    applied_typed_set_types(idx) = vtype
    nvals = min(nvals, max_set_vals)
    applied_typed_set_value_counts(idx) = nvals
    do j = 0, nvals - 1
      call capnp_list_get_text(vals, j, elem, err)
      if (err /= CAPNP_OK) cycle
      call copy_alloc_text(elem, applied_typed_set_values(j + 1, idx), set_value_len)
    end do
  end subroutine apply_set_stanza

  function nwchemc_embed_set_scf_direct(has_options, maxiter, thresh, tol2e) &
      result(rc) bind(C, name='nwchemc_embed_set_scf_direct')
    integer(c_int), intent(in), value :: has_options, maxiter
    real(c_double), intent(in), value :: thresh, tol2e
    integer(c_int) :: rc
    applied_scf_has_options = int(has_options)
    applied_scf_maxiter = int(maxiter)
    applied_scf_thresh = real(thresh, real64)
    applied_scf_tol2e = real(tol2e, real64)
    rc = 0_c_int
  end function nwchemc_embed_set_scf_direct

  function nwchemc_embed_get_scf_direct(has_options, maxiter, thresh, tol2e) &
      result(rc) bind(C, name='nwchemc_embed_get_scf_direct')
    integer(c_int), intent(out) :: has_options, maxiter
    real(c_double), intent(out) :: thresh, tol2e
    integer(c_int) :: rc
    has_options = int(applied_scf_has_options, c_int)
    maxiter = int(applied_scf_maxiter, c_int)
    thresh = real(applied_scf_thresh, c_double)
    tol2e = real(applied_scf_tol2e, c_double)
    rc = 0_c_int
  end function nwchemc_embed_get_scf_direct

  function nwchemc_embed_get_config(basis, basis_len, theory, theory_len, &
      scf_type, scf_len, charge, mult) result(rc) &
      bind(C, name='nwchemc_embed_get_config')
    character(kind=c_char), intent(out) :: basis(*)
    integer(c_int), intent(in), value :: basis_len
    character(kind=c_char), intent(out) :: theory(*)
    integer(c_int), intent(in), value :: theory_len
    character(kind=c_char), intent(out) :: scf_type(*)
    integer(c_int), intent(in), value :: scf_len
    integer(c_int), intent(out) :: charge, mult
    integer(c_int) :: rc
    call f_to_c_chars(applied_basis, basis, basis_len)
    call f_to_c_chars(applied_theory, theory, theory_len)
    call f_to_c_chars(applied_scf_type, scf_type, scf_len)
    charge = int(applied_charge, c_int)
    mult = int(applied_mult, c_int)
    rc = 0_c_int
  end function nwchemc_embed_get_config

  function nwchemc_embed_get_dft_direct(xc, xc_len, direct_enabled, &
      smearing_enabled, smear_sigma_hartree, smearing_spinset) result(rc) &
      bind(C, name='nwchemc_embed_get_dft_direct')
    character(kind=c_char), intent(out) :: xc(*)
    integer(c_int), intent(in), value :: xc_len
    integer(c_int), intent(out) :: direct_enabled, smearing_enabled
    real(c_double), intent(out) :: smear_sigma_hartree
    integer(c_int), intent(out) :: smearing_spinset
    integer(c_int) :: rc
    call f_to_c_chars(applied_dft_xc, xc, xc_len)
    direct_enabled = int(applied_dft_direct, c_int)
    smearing_enabled = int(applied_dft_smear_on, c_int)
    smear_sigma_hartree = real(applied_dft_smear_sigma, c_double)
    smearing_spinset = int(applied_dft_smear_spinset, c_int)
    rc = 0_c_int
  end function nwchemc_embed_get_dft_direct

  function nwchemc_embed_get_driver_direct(has_options, maxiter, &
      tolerance_mode, gmax_tol, grms_tol, xmax_tol, xrms_tol) result(rc) &
      bind(C, name='nwchemc_embed_get_driver_direct')
    integer(c_int), intent(out) :: has_options, maxiter, tolerance_mode
    real(c_double), intent(out) :: gmax_tol, grms_tol, xmax_tol, xrms_tol
    integer(c_int) :: rc
    has_options = int(applied_driver_has_options, c_int)
    maxiter = int(applied_driver_maxiter, c_int)
    tolerance_mode = int(applied_driver_tolerance_mode, c_int)
    gmax_tol = real(applied_driver_gmax_tol, c_double)
    grms_tol = real(applied_driver_grms_tol, c_double)
    xmax_tol = real(applied_driver_xmax_tol, c_double)
    xrms_tol = real(applied_driver_xrms_tol, c_double)
    rc = 0_c_int
  end function nwchemc_embed_get_driver_direct

  function nwchemc_embed_get_nwpw_direct(has_options, energy_cutoff, &
      wavefunction_cutoff, ewald_rcut, ewald_ncut) result(rc) &
      bind(C, name='nwchemc_embed_get_nwpw_direct')
    integer(c_int), intent(out) :: has_options, ewald_ncut
    real(c_double), intent(out) :: energy_cutoff, wavefunction_cutoff, ewald_rcut
    integer(c_int) :: rc
    has_options = int(applied_nwpw_has_options, c_int)
    energy_cutoff = real(applied_nwpw_energy_cutoff, c_double)
    wavefunction_cutoff = real(applied_nwpw_wavefunction_cutoff, c_double)
    ewald_rcut = real(applied_nwpw_ewald_rcut, c_double)
    ewald_ncut = int(applied_nwpw_ewald_ncut, c_int)
    rc = 0_c_int
  end function nwchemc_embed_get_nwpw_direct


  function nwchemc_embed_get_input_blocks(blocks, blocks_len) result(rc) &
      bind(C, name='nwchemc_embed_get_input_blocks')
    character(kind=c_char), intent(out) :: blocks(*)
    integer(c_int), intent(in), value :: blocks_len
    integer(c_int) :: rc
    call f_to_c_chars(applied_input_blocks, blocks, blocks_len)
    rc = 0_c_int
  end function nwchemc_embed_get_input_blocks

end module nwchemc_embed_apply_params_mod
