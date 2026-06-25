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
      c_char, c_double, c_int, c_null_char
  use, intrinsic :: iso_fortran_env, only: real64
  implicit none
  private

  ! C ABI (stable names - match nwchem_c_abi.c extern declarations)
  public :: nwchemc_embed_init
  public :: nwchemc_embed_available
  public :: nwchemc_embed_set_config
  public :: nwchemc_embed_set_dft_direct
  public :: nwchemc_embed_set_pseudopotentials
  public :: nwchemc_embed_energy_grad
  public :: nwchemc_embed_hessian
  public :: nwchemc_embed_finalize

  ! Module state (saved across C calls).
  ! Note: NWChem RTDB handles may be 0 on success; never use handle==0 as failure.
  logical, save :: ga_ready = .false.
  logical, save :: rtdb_ready = .false.
  logical, save :: owns_mpi_runtime = .false.
  logical, save :: runtime_finalized = .false.
  integer, save :: rtdb_handle = -1
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
  integer, parameter :: max_embed_atoms = 64
  integer, parameter :: max_embed_pseudopotentials = 64
  integer, parameter :: psp_element_len = 16
  integer, parameter :: psp_name_len = 256
  integer, save :: cfg_psp_count = 0
  character(len=psp_element_len), save :: cfg_psp_elements(max_embed_pseudopotentials) = ' '
  character(len=psp_name_len), save :: cfg_psp_names(max_embed_pseudopotentials) = ' '
  integer, save :: cfg_psp_types(max_embed_pseudopotentials) = 0

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

    subroutine nwchem_legacy_energy_grad(rtdb, n_atoms, pos_ang, atmnrs, &
        basis_name, theory_name, scf_type, input_blocks, charge, mult, &
        dft_direct, dft_smear_on, dft_smear_sigma, dft_smear_spinset, &
        psp_count, psp_elements, psp_types, psp_names, &
        energy_h, grad_h_bohr, errmsg, ok)
      import :: real64
      integer, intent(in) :: rtdb, n_atoms
      real(real64), intent(in) :: pos_ang(*)
      integer, intent(in) :: atmnrs(*)
      character(len=64), intent(in) :: basis_name
      character(len=64), intent(in) :: theory_name, scf_type
      character(len=4096), intent(in) :: input_blocks
      integer, intent(in) :: charge, mult
      integer, intent(in) :: dft_direct, dft_smear_on, dft_smear_spinset
      integer, intent(in) :: psp_count
      character(len=16), intent(in) :: psp_elements(*)
      integer, intent(in) :: psp_types(*)
      character(len=256), intent(in) :: psp_names(*)
      real(real64), intent(in) :: dft_smear_sigma
      real(real64), intent(out) :: energy_h
      real(real64), intent(out) :: grad_h_bohr(*)
      character(len=*), intent(out) :: errmsg
      integer, intent(out) :: ok
    end subroutine nwchem_legacy_energy_grad

    subroutine nwchem_legacy_hessian(rtdb, n_atoms, pos_ang, atmnrs, &
        basis_name, theory_name, scf_type, input_blocks, charge, mult, &
        dft_direct, dft_smear_on, dft_smear_sigma, dft_smear_spinset, &
        psp_count, psp_elements, psp_types, psp_names, &
        hessian_h_bohr2, errmsg, ok)
      import :: real64
      integer, intent(in) :: rtdb, n_atoms
      real(real64), intent(in) :: pos_ang(*)
      integer, intent(in) :: atmnrs(*)
      character(len=64), intent(in) :: basis_name
      character(len=64), intent(in) :: theory_name, scf_type
      character(len=4096), intent(in) :: input_blocks
      integer, intent(in) :: charge, mult
      integer, intent(in) :: dft_direct, dft_smear_on, dft_smear_spinset
      integer, intent(in) :: psp_count
      character(len=16), intent(in) :: psp_elements(*)
      integer, intent(in) :: psp_types(*)
      character(len=256), intent(in) :: psp_names(*)
      real(real64), intent(in) :: dft_smear_sigma
      real(real64), intent(out) :: hessian_h_bohr2(*)
      character(len=*), intent(out) :: errmsg
      integer, intent(out) :: ok
    end subroutine nwchem_legacy_hessian
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
      if (cfg_theory(1:3) /= 'dft') cfg_theory = 'dft'
    end if
    cfg_dft_direct = int(direct_enabled)
    cfg_dft_smear_on = int(smearing_enabled)
    cfg_dft_smear_sigma = real(smear_sigma_hartree, real64)
    cfg_dft_smear_spinset = int(smearing_spinset)
  end function nwchemc_embed_set_dft_direct

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
    rc = 0_c_int
  end function nwchemc_embed_set_pseudopotentials

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
    integer :: ok, n, i
    character(len=512) :: msg
    real(real64), allocatable :: pos(:), grad(:)
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
    if (n > max_embed_atoms) then
      call set_c_errmsg(errmsg, errmsg_len, 'n_atoms exceeds embed max')
      return
    end if

    allocate (pos(3 * n), grad(3 * n), z(n))
    do i = 1, 3 * n
      pos(i) = real(positions_ang(i), kind=real64)
      grad(i) = 0.0_real64
    end do
    do i = 1, n
      z(i) = int(atomic_numbers(i))
    end do

    call nwchem_legacy_energy_grad(rtdb_handle, n, pos, z, cfg_basis, &
        cfg_theory, cfg_scf, cfg_input_blocks, int(charge), &
        max(1, int(mult)), cfg_dft_direct, cfg_dft_smear_on, &
        cfg_dft_smear_sigma, cfg_dft_smear_spinset, cfg_psp_count, &
        cfg_psp_elements, cfg_psp_types, cfg_psp_names, energy_h, grad, msg, ok)

    do i = 1, 3 * n
      grad_h_bohr(i) = real(grad(i), kind=c_double)
    end do
    call set_c_errmsg(errmsg, errmsg_len, trim(msg))
    if (ok == 0) rc = 0_c_int
    deallocate (pos, grad, z)
  end function nwchemc_embed_energy_grad

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
    integer :: ok, n, n2, i
    character(len=512) :: msg
    real(real64), allocatable :: pos(:), hess(:)
    integer, allocatable :: z(:)

    rc = -1_c_int
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
    if (n > max_embed_atoms) then
      call set_c_errmsg(errmsg, errmsg_len, 'n_atoms exceeds embed max')
      return
    end if

    n2 = (3 * n) * (3 * n)
    allocate (pos(3 * n), hess(n2), z(n))
    do i = 1, 3 * n
      pos(i) = real(positions_ang(i), kind=real64)
    end do
    do i = 1, n2
      hess(i) = 0.0_real64
    end do
    do i = 1, n
      z(i) = int(atomic_numbers(i))
    end do

    call nwchem_legacy_hessian(rtdb_handle, n, pos, z, cfg_basis, &
        cfg_theory, cfg_scf, cfg_input_blocks, int(charge), &
        max(1, int(mult)), cfg_dft_direct, cfg_dft_smear_on, &
        cfg_dft_smear_sigma, cfg_dft_smear_spinset, cfg_psp_count, &
        cfg_psp_elements, cfg_psp_types, cfg_psp_names, hess, msg, ok)

    do i = 1, n2
      hessian_h_bohr2(i) = real(hess(i), kind=c_double)
    end do
    call set_c_errmsg(errmsg, errmsg_len, trim(msg))
    if (ok == 0) rc = 0_c_int
    deallocate (pos, hess, z)
  end function nwchemc_embed_hessian

  ! --- helpers (private) ---

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
