      subroutine nwchemc_test_forceinput_cell_rtdb(n_atoms_a,
     $     pos_ang_a, atmnrs_a, cell_ang_a, has_cell_a, expected_a,
     $     n_atoms_b, pos_ang_b, atmnrs_b, cell_ang_b, has_cell_b,
     $     expected_b, state_charge_c, state_mult_c, result) bind(C,
     $     name='nwchemc_test_forceinput_cell_rtdb')
      use, intrinsic :: iso_c_binding, only: c_double, c_int
      implicit none
      integer(c_int), intent(in) :: n_atoms_a, n_atoms_b
      integer(c_int), intent(in) :: atmnrs_a(*), atmnrs_b(*)
      integer(c_int), intent(in) :: has_cell_a, has_cell_b
      integer(c_int), intent(in) :: state_charge_c, state_mult_c
      real(c_double), intent(in) :: pos_ang_a(*), pos_ang_b(*)
      real(c_double), intent(in) :: cell_ang_a(9), cell_ang_b(9)
      real(c_double), intent(in) :: expected_a(9), expected_b(9)
      integer(c_int), intent(out) :: result
      integer rtdb, init_ok, reset_ok, finish_ok, owns_mpi
      integer(c_int) case_result

      result = 0
      rtdb = -1
      owns_mpi = 0
      call nwchem_legacy_init(rtdb, init_ok, owns_mpi)
      if (init_ok .ne. 0) then
         result = 3
         return
      endif

      case_result = 0
      call nwchemc_check_forceinput_cell_rtdb(rtdb, n_atoms_a,
     $     pos_ang_a, atmnrs_a, cell_ang_a, has_cell_a, expected_a,
     $     case_result)
      if (case_result .ne. 0) then
         result = case_result
         goto 900
      endif

      reset_ok = 0
      call nwchem_legacy_reset_rtdb(rtdb, reset_ok)
      if (reset_ok .ne. 0) then
         result = 5
         goto 900
      endif

      case_result = 0
      call nwchemc_check_forceinput_cell_rtdb(rtdb, n_atoms_b,
     $     pos_ang_b, atmnrs_b, cell_ang_b, has_cell_b, expected_b,
     $     case_result)
      if (case_result .ne. 0) result = int(1000 + case_result, c_int)
      if (result .ne. 0) goto 900

      reset_ok = 0
      call nwchem_legacy_reset_rtdb(rtdb, reset_ok)
      if (reset_ok .ne. 0) then
         result = 6
         goto 900
      endif

      case_result = 0
      call nwchemc_check_forceinput_state_rtdb(rtdb, state_charge_c,
     $     state_mult_c, case_result)
      if (case_result .ne. 0) result = int(2000 + case_result, c_int)

 900  continue
      finish_ok = 0
      call nwchem_legacy_finalize(rtdb, owns_mpi, finish_ok)
      if ((finish_ok .ne. 0) .and. (result .eq. 0)) result = 91
      return
      end

      subroutine nwchemc_check_forceinput_state_rtdb(rtdb, charge_c,
     $     multiplicity_c, result)
      use, intrinsic :: iso_c_binding, only: c_double, c_int
      implicit none
#include "mafdecls.fh"
#include "rtdb.fh"
      integer rtdb
      integer(c_int), intent(in) :: charge_c, multiplicity_c
      integer(c_int), intent(out) :: result
      integer charge, mult, stored_mult
      double precision stored_charge(1)
      logical ok
      logical nwchemc_probe_get_int, nwchemc_probe_get_dbl_array
      external nwchemc_probe_get_int, nwchemc_probe_get_dbl_array
      character*64 theory_name, scf_type

      result = 0
      charge = int(charge_c)
      mult = int(multiplicity_c)
      if (mult .le. 0) then
         result = 2
         return
      endif

      theory_name = 'scf'
      scf_type = 'uhf'
      call nwchemc_apply_method_state(rtdb, theory_name, scf_type,
     $     charge, mult, 0, 0, 0.0d0, 0, 0, 0, 0.0d0, 0.0d0,
     $     0, 0, 0, 0.0d0, 0.0d0, 0.0d0, 0.0d0, ok)
      if (.not. ok) then
         result = 4
         return
      endif

      if (.not. nwchemc_probe_get_dbl_array(rtdb, 'charge', 1,
     $     stored_charge)) then
         result = 10
         return
      endif
      if (dabs(stored_charge(1) - dble(charge)) .gt. 1.0d-12) then
         write(*,*) 'forceinput charge mismatch', stored_charge(1),
     $      dble(charge)
         result = 11
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb, 'spin mult',
     $     stored_mult)) then
         result = 20
         return
      endif
      if (stored_mult .ne. mult) then
         write(*,*) 'forceinput spin mult mismatch', stored_mult, mult
         result = 21
         return
      endif

      return
      end

      subroutine nwchemc_check_forceinput_cell_rtdb(rtdb, n_atoms_c,
     $     pos_ang_c, atmnrs_c, cell_ang_c, has_cell_c,
     $     expected_amatrix, result)
      use, intrinsic :: iso_c_binding, only: c_double, c_int
      implicit none
      integer rtdb
      integer(c_int), intent(in) :: n_atoms_c
      integer(c_int), intent(in) :: atmnrs_c(*)
      integer(c_int), intent(in) :: has_cell_c
      real(c_double), intent(in) :: pos_ang_c(*)
      real(c_double), intent(in) :: cell_ang_c(9)
      real(c_double), intent(in) :: expected_amatrix(9)
      integer(c_int), intent(out) :: result
      integer natmax
      parameter (natmax = 16)
      integer geom, isystype
      integer n_atoms, has_cell
      integer atmnrs(natmax)
      integer i, j, k
      double precision pos_ang(3 * natmax)
      double precision cell_ang(9)
      double precision amat(3,3)
      double precision diff, tol
      character*256 errmsg
      logical status
      logical geom_create, geom_rtdb_load, geom_amatrix_get
      logical geom_systype_get, geom_destroy
      external geom_create, geom_rtdb_load, geom_amatrix_get
      external geom_systype_get, geom_destroy

      result = 0
      geom = -1
      tol = 1.0d-5
      n_atoms = int(n_atoms_c)
      has_cell = int(has_cell_c)
      if (n_atoms .le. 0 .or. n_atoms .gt. natmax) then
         result = 1
         return
      endif
      if (has_cell .eq. 0) then
         result = 2
         return
      endif

      do i = 1, n_atoms
         atmnrs(i) = int(atmnrs_c(i))
      enddo
      do i = 1, 3 * n_atoms
         pos_ang(i) = dble(pos_ang_c(i))
      enddo
      do i = 1, 9
         cell_ang(i) = dble(cell_ang_c(i))
      enddo

      call nwchemc_store_geometry_cell(rtdb, n_atoms, pos_ang, atmnrs,
     $     cell_ang, has_cell, errmsg, status)
      if (.not. status) then
         result = 4
         goto 900
      endif

      if (.not. geom_create(geom, 'geometry')) then
         result = 9
         goto 900
      endif
      if (.not. geom_rtdb_load(rtdb, geom, 'geometry')) then
         result = 10
         goto 900
      endif
      if (.not. geom_systype_get(geom, isystype)) then
         result = 20
         goto 900
      endif
      if (isystype .ne. 3) then
         result = 21
         goto 900
      endif
      if (.not. geom_amatrix_get(geom, amat)) then
         result = 30
         goto 900
      endif

      do j = 1, 3
         do i = 1, 3
            k = 3 * (j - 1) + i
            diff = dabs(amat(i,j) - expected_amatrix(k))
            if (diff .gt. tol) then
               write(*,*) 'forceinput cell mismatch', k, amat(i,j),
     $            expected_amatrix(k)
               result = int(100 + k, c_int)
               goto 900
            endif
         enddo
      enddo

 900  continue
      if (geom .ge. 0) then
         status = geom_destroy(geom)
         if ((.not. status) .and. (result .eq. 0)) result = 90
      endif
      return
      end
