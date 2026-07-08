! SPDX-License-Identifier: MIT
!
! Decode NWChemParams with public capnp-fortran accessors and apply migrated
! config families into embed module state (SCF first; more stanzas later).

module nwchemc_embed_apply_params_mod
  use, intrinsic :: iso_c_binding, only: &
      c_double, c_int, c_ptr, c_size_t, c_associated, c_f_pointer
  use, intrinsic :: iso_fortran_env, only: int8, int64, real64
  use capnp
  use potentials_capnp
  implicit none
  private

  public :: nwchemc_embed_apply_params
  public :: nwchemc_embed_set_scf_direct
  public :: nwchemc_embed_get_scf_direct
  public :: applied_scf_has_options
  public :: applied_scf_maxiter
  public :: applied_scf_thresh
  public :: applied_scf_tol2e

  integer, save :: applied_scf_has_options = 0
  integer, save :: applied_scf_maxiter = 0
  real(real64), save :: applied_scf_thresh = 0.0_real64
  real(real64), save :: applied_scf_tol2e = 0.0_real64

contains

  !> Apply migrated families from framed NWChemParams bytes.
  !> Currently: SCF maxiter/thresh/tol2e (same outcome as set_scf_direct).
  function nwchemc_embed_apply_params(params_capnp, params_capnp_size) &
      result(rc) bind(C, name='nwchemc_embed_apply_params')
    type(c_ptr), intent(in), value :: params_capnp
    integer(c_size_t), intent(in), value :: params_capnp_size
    integer(c_int) :: rc
    integer(int8), pointer :: raw(:)
    integer(int8), allocatable :: bytes(:)
    type(capnp_message_t), target :: msg
    type(n_w_chem_params_t) :: params
    type(n_w_chem_input_stanza_t) :: stanza
    type(n_w_chem_scf_stanza_t) :: scf
    type(capnp_ptr_t) :: stanzas
    integer :: err, i, n, kind, has_options, maxiter
    real(real64) :: thresh, tol2e
    integer(int64) :: n64

    rc = -1_c_int
    applied_scf_has_options = 0
    applied_scf_maxiter = 0
    applied_scf_thresh = 0.0_real64
    applied_scf_tol2e = 0.0_real64

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

    has_options = 0
    maxiter = 0
    thresh = 0.0_real64
    tol2e = 0.0_real64

    stanzas = n_w_chem_params_input_stanzas_get(params, err)
    n64 = 0_int64
    if (err == CAPNP_OK) n64 = capnp_list_len(stanzas)

    do i = 0, int(n64) - 1
      stanza = n_w_chem_params_input_stanzas_get_elem(params, i, err)
      if (err /= CAPNP_OK) cycle
      kind = n_w_chem_input_stanza_kind_get(stanza)
      if (kind /= N_W_CHEM_INPUT_STANZA_KIND_SCF_E) cycle
      scf = n_w_chem_input_stanza_scf_get(stanza, err)
      if (err /= CAPNP_OK) cycle
      if (scf%p%kind == CAPNP_PK_NULL) cycle

      maxiter = int(n_w_chem_scf_stanza_maxiter_get(scf))
      thresh = n_w_chem_scf_stanza_thresh_get(scf)
      tol2e = n_w_chem_scf_stanza_tol2e_get(scf)
      if (maxiter > 0) has_options = 1
      if (thresh > 0.0_real64) has_options = 1
      if (tol2e > 0.0_real64) has_options = 1
    end do

    applied_scf_has_options = has_options
    applied_scf_maxiter = maxiter
    applied_scf_thresh = thresh
    applied_scf_tol2e = tol2e

    call capnp_message_free(msg)
    deallocate (bytes)
    rc = 0_c_int
  end function nwchemc_embed_apply_params

  !> CommonMethodSpec / PotentialConfig overlay path (not NWChemParams bytes).
  function nwchemc_embed_set_scf_direct(has_options, maxiter, thresh, tol2e) &
      result(rc) bind(C, name='nwchemc_embed_set_scf_direct')
    integer(c_int), intent(in), value :: has_options
    integer(c_int), intent(in), value :: maxiter
    real(c_double), intent(in), value :: thresh
    real(c_double), intent(in), value :: tol2e
    integer(c_int) :: rc

    applied_scf_has_options = int(has_options)
    applied_scf_maxiter = int(maxiter)
    applied_scf_thresh = real(thresh, real64)
    applied_scf_tol2e = real(tol2e, real64)
    rc = 0_c_int
  end function nwchemc_embed_set_scf_direct

  !> Observability for tests and any C reader of applied SCF knobs.
  function nwchemc_embed_get_scf_direct(has_options, maxiter, thresh, tol2e) &
      result(rc) bind(C, name='nwchemc_embed_get_scf_direct')
    integer(c_int), intent(out) :: has_options
    integer(c_int), intent(out) :: maxiter
    real(c_double), intent(out) :: thresh
    real(c_double), intent(out) :: tol2e
    integer(c_int) :: rc

    has_options = int(applied_scf_has_options, c_int)
    maxiter = int(applied_scf_maxiter, c_int)
    thresh = real(applied_scf_thresh, c_double)
    tol2e = real(applied_scf_tol2e, c_double)
    rc = 0_c_int
  end function nwchemc_embed_get_scf_direct

end module nwchemc_embed_apply_params_mod
