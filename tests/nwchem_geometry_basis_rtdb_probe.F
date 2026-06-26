      subroutine nwchemc_test_geometry_basis_rtdb(result) bind(C,
     $     name='nwchemc_test_geometry_basis_rtdb')
      use, intrinsic :: iso_c_binding, only: c_int
      implicit none
      integer(c_int), intent(out) :: result
      integer nat
      parameter (nat = 65)
      integer rtdb, init_ok, finish_ok, owns_mpi
      integer atmnrs(nat)
      integer i
      double precision pos(3 * nat)
      double precision cell(9)
      character*64 basis_name
      character*256 errmsg
      logical status

      result = 0
      rtdb = -1
      init_ok = -1
      owns_mpi = 0
      call nwchem_legacy_init(rtdb, init_ok, owns_mpi)
      if (init_ok .ne. 0) then
         result = 1
         return
      endif

      do i = 1, nat
         atmnrs(i) = 1
         pos(3 * (i - 1) + 1) = 0.7d0 * dble(i - 1)
         pos(3 * (i - 1) + 2) = 0.0d0
         pos(3 * (i - 1) + 3) = 0.0d0
      enddo
      do i = 1, 9
         cell(i) = 0.0d0
      enddo
      basis_name = 'sto-3g'

      call nwchemc_store_geometry_cell(rtdb, nat, pos, atmnrs, cell, 0,
     $     errmsg, status)
      if (.not. status) then
         result = 2
         goto 900
      endif
      call nwchemc_store_library_basis(rtdb, nat, atmnrs, basis_name,
     $     errmsg, status)
      if (.not. status) result = 3

 900  continue
      finish_ok = 0
      call nwchem_legacy_finalize(rtdb, owns_mpi, finish_ok)
      if ((finish_ok .ne. 0) .and. (result .eq. 0)) result = 90
      return
      end
