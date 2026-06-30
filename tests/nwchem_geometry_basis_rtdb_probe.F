      subroutine nwchemc_test_geometry_basis_rtdb(result) bind(C,
     $     name='nwchemc_test_geometry_basis_rtdb')
      use, intrinsic :: iso_c_binding, only: c_int
      implicit none
#include "bas.fh"
#include "geom.fh"
#include "rtdb.fh"
      integer(c_int), intent(out) :: result
      integer nat
      parameter (nat = 65)
      integer rtdb, init_ok, finish_ok, owns_mpi
      integer atmnrs(nat)
      integer i, geom, basis
      integer atmnrs_cl(2)
      double precision pos(3 * nat)
      double precision pos_cl(6)
      double precision cell(9)
      character*64 basis_name, ecp_lib, ecp_on_ao
      character*16 tags(1)
      character*64 libs(1)
      character*256 errmsg
      logical status
      logical bas_name_exist_rtdb
      external nwchemc_set_basis_options, bas_name_exist_rtdb

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
c     Clear optional ECP for the large-H AO-only store.
      tags(1) = ' '
      libs(1) = ' '
      call nwchemc_set_basis_options(0, 0, 0, 0, ' ', 0, 0, tags, libs)

      call nwchemc_store_geometry_cell(rtdb, nat, pos, atmnrs, cell, 0,
     $     errmsg, status)
      if (.not. status) then
         result = 2
         goto 900
      endif
      call nwchemc_store_library_basis(rtdb, nat, atmnrs, basis_name,
     $     errmsg, status)
      if (.not. status) then
         result = 3
         goto 900
      endif

c     Same GA/RTDB session: Cl2 + lanl2dz_ecp AO with ECP association.
      atmnrs_cl(1) = 17
      atmnrs_cl(2) = 17
      pos_cl(1) = 0.0d0
      pos_cl(2) = 0.0d0
      pos_cl(3) = -1.0d0
      pos_cl(4) = 0.0d0
      pos_cl(5) = 0.0d0
      pos_cl(6) = 1.0d0
      basis_name = 'lanl2dz_ecp'
      ecp_lib = 'lanl2dz_ecp'
      call nwchemc_set_basis_options(1, 0, 0, 0, ecp_lib,
     $     len_trim(ecp_lib), 0, tags, libs)
      call nwchemc_store_geometry_cell(rtdb, 2, pos_cl, atmnrs_cl, cell,
     $     0, errmsg, status)
      if (.not. status) then
         result = 12
         goto 900
      endif
      call nwchemc_store_library_basis(rtdb, 2, atmnrs_cl, basis_name,
     $     errmsg, status)
      if (.not. status) then
         result = 13
         goto 900
      endif
      if (.not. bas_name_exist_rtdb(rtdb, 'ecp basis')) then
         result = 14
         goto 900
      endif
      if (.not. geom_create(geom, 'geometry')) then
         result = 15
         goto 900
      endif
      if (.not. geom_rtdb_load(rtdb, geom, 'geometry')) then
         result = 16
         status = geom_destroy(geom)
         goto 900
      endif
      if (.not. bas_create(basis, 'ao basis')) then
         result = 17
         status = geom_destroy(geom)
         goto 900
      endif
      if (.not. bas_rtdb_load(rtdb, geom, basis, 'ao basis')) then
         result = 18
         status = bas_destroy(basis)
         status = geom_destroy(geom)
         goto 900
      endif
      ecp_on_ao = ' '
      if (.not. bas_get_ecp_name(basis, ecp_on_ao)) then
         result = 19
         status = bas_destroy(basis)
         status = geom_destroy(geom)
         goto 900
      endif
      if (ecp_on_ao(1:9) .ne. 'ecp basis') then
         result = 20
         status = bas_destroy(basis)
         status = geom_destroy(geom)
         goto 900
      endif
      status = bas_destroy(basis)
      status = geom_destroy(geom)

 900  continue
      finish_ok = 0
      call nwchem_legacy_finalize(rtdb, owns_mpi, finish_ok)
      if ((finish_ok .ne. 0) .and. (result .eq. 0)) result = 90
      return
      end
