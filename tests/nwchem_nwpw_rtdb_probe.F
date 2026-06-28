      subroutine nwchemc_test_nwpw_rtdb(result) bind(C,
     $     name='nwchemc_test_nwpw_rtdb')
      use, intrinsic :: iso_c_binding, only: c_int
      implicit none
#include "bafdecls.fh"
#include "btdb.fh"
      integer(c_int), intent(out) :: result
      integer rtdb, init_ok, finish_ok, owns_mpi
      integer i, j
      integer typed_count
      integer typed_types(41)
      integer typed_value_counts(41)
      integer ivals(3)
      integer ival
      character*128 typed_keys(41)
      character*256 typed_values(64,41)
      save typed_values
      character*256 text_value
      character*256 errmsg
      double precision dvals(3)
      logical status, lval
      logical nwchemc_probe_get_int, nwchemc_probe_get_log,
     $     nwchemc_probe_get_int_array, nwchemc_probe_get_dbl_array
      external nwchemc_probe_get_int, nwchemc_probe_get_log,
     $     nwchemc_probe_get_int_array, nwchemc_probe_get_dbl_array

      result = 0
      rtdb = -1
      init_ok = -1
      owns_mpi = 0
      call nwchem_legacy_init(rtdb, init_ok, owns_mpi)
      if (init_ok .ne. 0) then
         result = 1
         return
      endif

      do i = 1, 41
         typed_keys(i) = ' '
         typed_types(i) = 0
         typed_value_counts(i) = 0
         do j = 1, 64
            typed_values(j,i) = ' '
         enddo
      enddo
      typed_count = 41
      typed_keys(1) = 'nwpw:balance'
      typed_types(1) = 4
      typed_value_counts(1) = 1
      typed_values(1,1) = 'false'
      typed_keys(2) = 'nwpw:bo_steps'
      typed_types(2) = 3
      typed_value_counts(2) = 2
      typed_values(1,2) = '13'
      typed_values(2,2) = '17'
      typed_keys(3) = 'nwpw:bo_time_step'
      typed_types(3) = 2
      typed_value_counts(3) = 1
      typed_values(1,3) = '15.5'
      typed_keys(4) = 'nwpw:bo_fake_mass'
      typed_types(4) = 2
      typed_value_counts(4) = 1
      typed_values(1,4) = '502.5'
      typed_keys(5) = 'nwpw:np_dimensions'
      typed_types(5) = 3
      typed_value_counts(5) = 3
      typed_values(1,5) = '2'
      typed_values(2,5) = '3'
      typed_values(3,5) = '4'
      typed_keys(6) = 'nwpw:ewald_ngrid'
      typed_types(6) = 3
      typed_value_counts(6) = 3
      typed_values(1,6) = '16'
      typed_values(2,6) = '18'
      typed_values(3,6) = '20'
      typed_keys(7) = 'nwpw:fractional_orbitals'
      typed_types(7) = 3
      typed_value_counts(7) = 2
      typed_values(1,7) = '2'
      typed_values(2,7) = '5'
      typed_keys(8) = 'nwpw:fractional_temperature'
      typed_types(8) = 2
      typed_value_counts(8) = 1
      typed_values(1,8) = '0.0025'
      typed_keys(9) = 'nwpw:fractional_smeartype'
      typed_types(9) = 3
      typed_value_counts(9) = 1
      typed_values(1,9) = '4'
      typed_keys(10) = 'nwpw:born'
      typed_types(10) = 4
      typed_value_counts(10) = 1
      typed_values(1,10) = 'true'
      typed_keys(11) = 'nwpw:born_dielec'
      typed_types(11) = 2
      typed_value_counts(11) = 1
      typed_values(1,11) = '78.4'
      typed_keys(12) = 'nwpw:born_vradii'
      typed_types(12) = 2
      typed_value_counts(12) = 2
      typed_values(1,12) = '1.25'
      typed_values(2,12) = '1.5'
      typed_keys(13) = 'nwpw:cpmd_properties'
      typed_types(13) = 4
      typed_value_counts(13) = 1
      typed_values(1,13) = 'true'
      typed_keys(14) = 'nwpw:cell_expand'
      typed_types(14) = 3
      typed_value_counts(14) = 3
      typed_values(1,14) = '1'
      typed_values(2,14) = '2'
      typed_values(3,14) = '3'
      typed_keys(15) = 'nwpw:mapping'
      typed_types(15) = 3
      typed_value_counts(15) = 1
      typed_values(1,15) = '4'
      typed_keys(16) = 'nwpw:rotation'
      typed_types(16) = 4
      typed_value_counts(16) = 1
      typed_values(1,16) = 'true'
      typed_keys(17) = 'nwpw:lmax_multipole'
      typed_types(17) = 3
      typed_value_counts(17) = 1
      typed_values(1,17) = '6'
      typed_keys(18) = 'pspw:HFX_single_precision'
      typed_types(18) = 4
      typed_value_counts(18) = 1
      typed_values(1,18) = 'true'
      typed_keys(19) = 'cgsd:geometry_optimize'
      typed_types(19) = 4
      typed_value_counts(19) = 1
      typed_values(1,19) = 'true'
      typed_keys(20) = 'cgsd:allow_translation'
      typed_types(20) = 4
      typed_value_counts(20) = 1
      typed_values(1,20) = 'false'
      typed_keys(21) = 'nwpw:init_velocities_temperature'
      typed_types(21) = 2
      typed_value_counts(21) = 1
      typed_values(1,21) = '325.0'
      typed_keys(22) = 'nwpw:init_velocities_seed'
      typed_types(22) = 3
      typed_value_counts(22) = 1
      typed_values(1,22) = '777'
      typed_keys(23) = 'nwpw:init_velocities'
      typed_types(23) = 4
      typed_value_counts(23) = 1
      typed_values(1,23) = 'true'
      typed_keys(24) = 'nwpw:makehmass2'
      typed_types(24) = 4
      typed_value_counts(24) = 1
      typed_values(1,24) = 'false'
      typed_keys(25) = 'nwpw:translate_vector'
      typed_types(25) = 2
      typed_value_counts(25) = 3
      typed_values(1,25) = '0.1'
      typed_values(2,25) = '0.2'
      typed_values(3,25) = '0.3'
      typed_keys(26) = 'nwpw:translate_geom_name'
      typed_types(26) = 1
      typed_value_counts(26) = 1
      typed_values(1,26) = 'mobile'
      typed_keys(27) = 'nwpw:translate_reorder'
      typed_types(27) = 4
      typed_value_counts(27) = 1
      typed_values(1,27) = 'false'
      typed_keys(28) = 'nwpw:socket_type'
      typed_types(28) = 1
      typed_value_counts(28) = 1
      typed_values(1,28) = 'inet'
      typed_keys(29) = 'nwpw:socket_ip'
      typed_types(29) = 1
      typed_value_counts(29) = 1
      typed_values(1,29) = '127.0.0.1'
      typed_keys(30) = 'nwpw_APC:Gc'
      typed_types(30) = 2
      typed_value_counts(30) = 1
      typed_values(1,30) = '1.25'
      typed_keys(31) = 'nwpw_APC:nga'
      typed_types(31) = 3
      typed_value_counts(31) = 1
      typed_values(1,31) = '3'
      typed_keys(32) = 'nwpw_APC:gamma'
      typed_types(32) = 2
      typed_value_counts(32) = 3
      typed_values(1,32) = '0.5'
      typed_values(2,32) = '0.25'
      typed_values(3,32) = '0.125'
      typed_keys(33) = 'nwpw:minimizer'
      typed_types(33) = 3
      typed_value_counts(33) = 1
      typed_values(1,33) = '8'
      typed_keys(34) = 'nwpw:ks_algorithm'
      typed_types(34) = 3
      typed_value_counts(34) = 1
      typed_values(1,34) = '-1'
      typed_keys(35) = 'nwpw:scf_algorithm'
      typed_types(35) = 3
      typed_value_counts(35) = 1
      typed_values(1,35) = '3'
      typed_keys(36) = 'nwpw:precondition'
      typed_types(36) = 4
      typed_value_counts(36) = 1
      typed_values(1,36) = 'false'
      typed_keys(37) = 'nwpw:kerker_g0'
      typed_types(37) = 2
      typed_value_counts(37) = 1
      typed_values(1,37) = '0.375'
      typed_keys(38) = 'nwpw:ks_alpha'
      typed_types(38) = 2
      typed_value_counts(38) = 1
      typed_values(1,38) = '0.125'
      typed_keys(39) = 'nwpw:ks_maxit_orb'
      typed_types(39) = 3
      typed_value_counts(39) = 1
      typed_values(1,39) = '12'
      typed_keys(40) = 'nwpw:ks_maxit_orbs'
      typed_types(40) = 3
      typed_value_counts(40) = 1
      typed_values(1,40) = '4'
      typed_keys(41) = 'nwpw:diis_histories'
      typed_types(41) = 3
      typed_value_counts(41) = 1
      typed_values(1,41) = '6'

      call nwchemc_apply_direct_typed_sets(rtdb, typed_count,
     $     typed_keys, typed_types, typed_value_counts, typed_values,
     $     errmsg, status)
      if (.not. status) then
         result = 2
         goto 900
      endif

      if (.not. nwchemc_probe_get_log(rtdb, 'nwpw:balance',
     $     lval)) then
         result = 10
         goto 900
      endif
      if (lval) then
         result = 11
         goto 900
      endif
      if (.not. nwchemc_probe_get_int_array(rtdb, 'nwpw:bo_steps',
     $     2, ivals)) then
         result = 12
         goto 900
      endif
      if (ivals(1) .ne. 13 .or. ivals(2) .ne. 17) then
         result = 13
         goto 900
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'nwpw:bo_time_step', 1, dvals)) then
         result = 14
         goto 900
      endif
      if (dabs(dvals(1) - 15.5d0) .gt. 1.0d-12) then
         result = 15
         goto 900
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'nwpw:bo_fake_mass', 1, dvals)) then
         result = 16
         goto 900
      endif
      if (dabs(dvals(1) - 502.5d0) .gt. 1.0d-12) then
         result = 17
         goto 900
      endif
      if (.not. nwchemc_probe_get_int_array(rtdb,
     $     'nwpw:np_dimensions', 3, ivals)) then
         result = 18
         goto 900
      endif
      if (ivals(1) .ne. 2 .or. ivals(2) .ne. 3 .or.
     $     ivals(3) .ne. 4) then
         result = 19
         goto 900
      endif
      if (.not. nwchemc_probe_get_int_array(rtdb,
     $     'nwpw:ewald_ngrid', 3, ivals)) then
         result = 20
         goto 900
      endif
      if (ivals(1) .ne. 16 .or. ivals(2) .ne. 18 .or.
     $     ivals(3) .ne. 20) then
         result = 21
         goto 900
      endif
      if (.not. nwchemc_probe_get_int_array(rtdb,
     $     'nwpw:fractional_orbitals', 2, ivals)) then
         result = 22
         goto 900
      endif
      if (ivals(1) .ne. 2 .or. ivals(2) .ne. 5) then
         result = 23
         goto 900
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'nwpw:fractional_temperature', 1, dvals)) then
         result = 24
         goto 900
      endif
      if (dabs(dvals(1) - 0.0025d0) .gt. 1.0d-12) then
         result = 25
         goto 900
      endif
      if (.not. nwchemc_probe_get_int(rtdb,
     $     'nwpw:fractional_smeartype', ival)) then
         result = 26
         goto 900
      endif
      if (ival .ne. 4) then
         result = 27
         goto 900
      endif
      if (.not. nwchemc_probe_get_log(rtdb, 'nwpw:born',
     $     lval)) then
         result = 28
         goto 900
      endif
      if (.not. lval) then
         result = 29
         goto 900
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'nwpw:born_dielec', 1, dvals)) then
         result = 30
         goto 900
      endif
      if (dabs(dvals(1) - 78.4d0) .gt. 1.0d-12) then
         result = 31
         goto 900
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'nwpw:born_vradii', 2, dvals)) then
         result = 32
         goto 900
      endif
      if (dabs(dvals(1) - 1.25d0) .gt. 1.0d-12 .or.
     $    dabs(dvals(2) - 1.5d0) .gt. 1.0d-12) then
         result = 33
         goto 900
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'nwpw:cpmd_properties', lval)) then
         result = 34
         goto 900
      endif
      if (.not. lval) then
         result = 35
         goto 900
      endif
      if (.not. nwchemc_probe_get_int_array(rtdb,
     $     'nwpw:cell_expand', 3, ivals)) then
         result = 36
         goto 900
      endif
      if (ivals(1) .ne. 1 .or. ivals(2) .ne. 2 .or.
     $     ivals(3) .ne. 3) then
         result = 37
         goto 900
      endif
      if (.not. nwchemc_probe_get_int(rtdb, 'nwpw:mapping',
     $     ival)) then
         result = 38
         goto 900
      endif
      if (ival .ne. 4) then
         result = 39
         goto 900
      endif
      if (.not. nwchemc_probe_get_log(rtdb, 'nwpw:rotation',
     $     lval)) then
         result = 40
         goto 900
      endif
      if (.not. lval) then
         result = 41
         goto 900
      endif
      if (.not. nwchemc_probe_get_int(rtdb,
     $     'nwpw:lmax_multipole', ival)) then
         result = 42
         goto 900
      endif
      if (ival .ne. 6) then
         result = 43
         goto 900
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'pspw:HFX_single_precision', lval)) then
         result = 44
         goto 900
      endif
      if (.not. lval) then
         result = 45
         goto 900
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'cgsd:geometry_optimize', lval)) then
         result = 46
         goto 900
      endif
      if (.not. lval) then
         result = 47
         goto 900
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'cgsd:allow_translation', lval)) then
         result = 48
         goto 900
      endif
      if (lval) then
         result = 49
         goto 900
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'nwpw:init_velocities_temperature', 1, dvals)) then
         result = 50
         goto 900
      endif
      if (dabs(dvals(1) - 325.0d0) .gt. 1.0d-12) then
         result = 51
         goto 900
      endif
      if (.not. nwchemc_probe_get_int(rtdb,
     $     'nwpw:init_velocities_seed', ival)) then
         result = 52
         goto 900
      endif
      if (ival .ne. 777) then
         result = 53
         goto 900
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'nwpw:init_velocities', lval)) then
         result = 54
         goto 900
      endif
      if (.not. lval) then
         result = 55
         goto 900
      endif
      if (.not. nwchemc_probe_get_log(rtdb, 'nwpw:makehmass2',
     $     lval)) then
         result = 56
         goto 900
      endif
      if (lval) then
         result = 57
         goto 900
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'nwpw:translate_vector', 3, dvals)) then
         result = 58
         goto 900
      endif
      if (dabs(dvals(1) - 0.1d0) .gt. 1.0d-12 .or.
     $    dabs(dvals(2) - 0.2d0) .gt. 1.0d-12 .or.
     $    dabs(dvals(3) - 0.3d0) .gt. 1.0d-12) then
         result = 59
         goto 900
      endif
      text_value = ' '
      if (.not. btdb_cget(rtdb, 'nwpw:translate_geom_name',
     $     1, text_value)) then
         result = 60
         goto 900
      endif
      if (text_value(1:6) .ne. 'mobile') then
         result = 61
         goto 900
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'nwpw:translate_reorder', lval)) then
         result = 62
         goto 900
      endif
      if (lval) then
         result = 63
         goto 900
      endif
      text_value = ' '
      if (.not. btdb_cget(rtdb, 'nwpw:socket_type',
     $     1, text_value)) then
         result = 64
         goto 900
      endif
      if (text_value(1:4) .ne. 'inet') then
         result = 65
         goto 900
      endif
      text_value = ' '
      if (.not. btdb_cget(rtdb, 'nwpw:socket_ip',
     $     1, text_value)) then
         result = 66
         goto 900
      endif
      if (text_value(1:9) .ne. '127.0.0.1') then
         result = 67
         goto 900
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'nwpw_APC:Gc', 1, dvals)) then
         result = 68
         goto 900
      endif
      if (dabs(dvals(1) - 1.25d0) .gt. 1.0d-12) then
         result = 69
         goto 900
      endif
      if (.not. nwchemc_probe_get_int(rtdb,
     $     'nwpw_APC:nga', ival)) then
         result = 70
         goto 900
      endif
      if (ival .ne. 3) then
         result = 71
         goto 900
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'nwpw_APC:gamma', 3, dvals)) then
         result = 72
         goto 900
      endif
      if (dabs(dvals(1) - 0.5d0) .gt. 1.0d-12 .or.
     $    dabs(dvals(2) - 0.25d0) .gt. 1.0d-12 .or.
     $    dabs(dvals(3) - 0.125d0) .gt. 1.0d-12) then
         result = 73
         goto 900
      endif
      if (.not. nwchemc_probe_get_int(rtdb,
     $     'nwpw:minimizer', ival)) then
         result = 74
         goto 900
      endif
      if (ival .ne. 8) then
         result = 75
         goto 900
      endif
      if (.not. nwchemc_probe_get_int(rtdb,
     $     'nwpw:ks_algorithm', ival)) then
         result = 76
         goto 900
      endif
      if (ival .ne. -1) then
         result = 77
         goto 900
      endif
      if (.not. nwchemc_probe_get_int(rtdb,
     $     'nwpw:scf_algorithm', ival)) then
         result = 78
         goto 900
      endif
      if (ival .ne. 3) then
         result = 79
         goto 900
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'nwpw:precondition', lval)) then
         result = 80
         goto 900
      endif
      if (lval) then
         result = 81
         goto 900
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'nwpw:kerker_g0', 1, dvals)) then
         result = 82
         goto 900
      endif
      if (dabs(dvals(1) - 0.375d0) .gt. 1.0d-12) then
         result = 83
         goto 900
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'nwpw:ks_alpha', 1, dvals)) then
         result = 84
         goto 900
      endif
      if (dabs(dvals(1) - 0.125d0) .gt. 1.0d-12) then
         result = 85
         goto 900
      endif
      if (.not. nwchemc_probe_get_int(rtdb,
     $     'nwpw:ks_maxit_orb', ival)) then
         result = 86
         goto 900
      endif
      if (ival .ne. 12) then
         result = 87
         goto 900
      endif
      if (.not. nwchemc_probe_get_int(rtdb,
     $     'nwpw:ks_maxit_orbs', ival)) then
         result = 88
         goto 900
      endif
      if (ival .ne. 4) then
         result = 89
         goto 900
      endif
      if (.not. nwchemc_probe_get_int(rtdb,
     $     'nwpw:diis_histories', ival)) then
         result = 90
         goto 900
      endif
      if (ival .ne. 6) then
         result = 91
         goto 900
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'nwpw:lcao_mask', lval)) then
         result = 92
         goto 900
      endif
      if (.not. lval) then
         result = 93
         goto 900
      endif
      if (.not. nwchemc_probe_get_int_array(rtdb,
     $     'nwpw:lcao_mask_uporbs', 3, ivals)) then
         result = 94
         goto 900
      endif
      if (ivals(1) .ne. 1 .or. ivals(2) .ne. 3 .or.
     $     ivals(3) .ne. 5) then
         result = 95
         goto 900
      endif
      if (.not. nwchemc_probe_get_int_array(rtdb,
     $     'nwpw:lcao_mask_downorbs', 2, ivals)) then
         result = 96
         goto 900
      endif
      if (ivals(1) .ne. 2 .or. ivals(2) .ne. 4) then
         result = 97
         goto 900
      endif

 900  continue
      finish_ok = 0
      call nwchem_legacy_finalize(rtdb, owns_mpi, finish_ok)
      if ((finish_ok .ne. 0) .and. (result .eq. 0)) result = 100
      return
      end

      subroutine nwchemc_test_configured_nwpw_rtdb(result) bind(C,
     $     name='nwchemc_test_configured_nwpw_rtdb')
      use, intrinsic :: iso_c_binding, only: c_int, c_long_long
      implicit none
#include "bafdecls.fh"
#include "btdb.fh"
      integer(c_int), intent(out) :: result
      integer rtdb
      integer ival
      integer ivals(3)
      integer(c_long_long) current_rtdb
      character*256 text_value
      double precision dvals(3)
      double precision r1, r2
      logical lval
      logical nwchemc_probe_get_int, nwchemc_probe_get_log,
     $     nwchemc_probe_get_int_array, nwchemc_probe_get_dbl_array
      external nwchemc_probe_get_int, nwchemc_probe_get_log,
     $     nwchemc_probe_get_int_array, nwchemc_probe_get_dbl_array

      interface
         function nwchemc_embed_current_rtdb() result(handle) bind(C,
     $        name='nwchemc_embed_current_rtdb')
            use, intrinsic :: iso_c_binding, only: c_long_long
            integer(c_long_long) handle
         end function nwchemc_embed_current_rtdb
      end interface

      result = 0
      current_rtdb = nwchemc_embed_current_rtdb()
      if (current_rtdb .lt. 0_c_long_long) then
         result = 201
         return
      endif
      rtdb = int(current_rtdb, kind(rtdb))

      if (.not. nwchemc_probe_get_log(rtdb, 'nwpw:balance',
     $     lval)) then
         result = 210
         return
      endif
      if (lval) then
         result = 211
         return
      endif
      if (.not. nwchemc_probe_get_int_array(rtdb, 'nwpw:bo_steps',
     $     2, ivals)) then
         result = 212
         return
      endif
      if (ivals(1) .ne. 13 .or. ivals(2) .ne. 17) then
         result = 213
         return
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'nwpw:bo_time_step', 1, dvals)) then
         result = 214
         return
      endif
      if (dabs(dvals(1) - 15.5d0) .gt. 1.0d-12) then
         result = 215
         return
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'nwpw:bo_fake_mass', 1, dvals)) then
         result = 216
         return
      endif
      if (dabs(dvals(1) - 502.5d0) .gt. 1.0d-12) then
         result = 217
         return
      endif
      if (.not. nwchemc_probe_get_int_array(rtdb,
     $     'nwpw:np_dimensions', 3, ivals)) then
         result = 218
         return
      endif
      if (ivals(1) .ne. 2 .or. ivals(2) .ne. 3 .or.
     $     ivals(3) .ne. 4) then
         result = 219
         return
      endif
      if (.not. nwchemc_probe_get_int_array(rtdb,
     $     'nwpw:ewald_ngrid', 3, ivals)) then
         result = 220
         return
      endif
      if (ivals(1) .ne. 16 .or. ivals(2) .ne. 18 .or.
     $     ivals(3) .ne. 20) then
         result = 221
         return
      endif
      if (.not. nwchemc_probe_get_int_array(rtdb,
     $     'nwpw:fractional_orbitals', 2, ivals)) then
         result = 222
         return
      endif
      if (ivals(1) .ne. 2 .or. ivals(2) .ne. 5) then
         result = 223
         return
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'nwpw:fractional_temperature', 1, dvals)) then
         result = 224
         return
      endif
      if (dabs(dvals(1) - 0.0025d0) .gt. 1.0d-12) then
         result = 225
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb,
     $     'nwpw:fractional_smeartype', ival)) then
         result = 226
         return
      endif
      if (ival .ne. 4) then
         result = 227
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb, 'nwpw:born',
     $     lval)) then
         result = 228
         return
      endif
      if (.not. lval) then
         result = 229
         return
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'nwpw:born_dielec', 1, dvals)) then
         result = 230
         return
      endif
      if (dabs(dvals(1) - 78.4d0) .gt. 1.0d-12) then
         result = 231
         return
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'nwpw:born_vradii', 2, dvals)) then
         result = 232
         return
      endif
      r1 = 1.25d0 / 0.529177d0
      r2 = 1.5d0 / 0.529177d0
      if (dabs(dvals(1) - r1) .gt. 1.0d-5 .or.
     $    dabs(dvals(2) - r2) .gt. 1.0d-5) then
         result = 233
         return
      endif
      text_value = ' '
      if (.not. btdb_cget(rtdb, 'nwpw:vfield_filenames',
     $     1, text_value)) then
         result = 308
         return
      endif
      if (text_value(1:23) .ne. 'vf_a.ascii vf_b.ascii') then
         result = 309
         return
      endif
      text_value = ' '
      if (.not. btdb_cget(rtdb, 'nwpw:dos:filename',
     $     1, text_value)) then
         result = 310
         return
      endif
      if (text_value(1:7) .ne. 'dos.dat') then
         result = 311
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'nwpw:cpmd_properties', lval)) then
         result = 234
         return
      endif
      if (.not. lval) then
         result = 235
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'nwpw:use_grid_cmp', lval)) then
         result = 338
         return
      endif
      if (lval) then
         result = 339
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'nwpw:use_director', lval)) then
         result = 340
         return
      endif
      if (.not. lval) then
         result = 341
         return
      endif
      text_value = ' '
      if (.not. btdb_cget(rtdb, 'nwpw:director_filename',
     $     1, text_value)) then
         result = 342
         return
      endif
      if (text_value(1:12) .ne. 'director.dat') then
         result = 343
         return
      endif
      if (.not. nwchemc_probe_get_int_array(rtdb,
     $     'nwpw:cell_expand', 3, ivals)) then
         result = 236
         return
      endif
      if (ivals(1) .ne. 1 .or. ivals(2) .ne. 2 .or.
     $     ivals(3) .ne. 3) then
         result = 237
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb, 'nwpw:mapping',
     $     ival)) then
         result = 238
         return
      endif
      if (ival .ne. 4) then
         result = 239
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb, 'nwpw:rotation',
     $     lval)) then
         result = 240
         return
      endif
      if (.not. lval) then
         result = 241
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb,
     $     'nwpw:lmax_multipole', ival)) then
         result = 242
         return
      endif
      if (ival .ne. 6) then
         result = 243
         return
      endif
      text_value = ' '
      if (.not. btdb_cget(rtdb, 'cpmd:fei_filename',
     $     1, text_value)) then
         result = 344
         return
      endif
      if (text_value(1:7) .ne. 'fei.dat') then
         result = 345
         return
      endif
      text_value = ' '
      if (.not. btdb_cget(rtdb, 'nwpw:fei_filename',
     $     1, text_value)) then
         result = 346
         return
      endif
      if (text_value(1:7) .ne. 'fei.dat') then
         result = 347
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'cpmd:fei', lval)) then
         result = 348
         return
      endif
      if (.not. lval) then
         result = 349
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'nwpw:fei', lval)) then
         result = 350
         return
      endif
      if (.not. lval) then
         result = 351
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'pspw:HFX_single_precision', lval)) then
         result = 244
         return
      endif
      if (.not. lval) then
         result = 245
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'cgsd:geometry_optimize', lval)) then
         result = 246
         return
      endif
      if (.not. lval) then
         result = 247
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'cpsd:geometry_optimize', lval)) then
         result = 312
         return
      endif
      if (.not. lval) then
         result = 313
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'band:geometry_optimize', lval)) then
         result = 314
         return
      endif
      if (.not. lval) then
         result = 315
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'pspw_qmmm_auxon', lval)) then
         result = 316
         return
      endif
      if (.not. lval) then
         result = 317
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb, 'cgsd:ispin',
     $     ival)) then
         result = 318
         return
      endif
      if (ival .ne. 2) then
         result = 319
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb, 'band:ispin',
     $     ival)) then
         result = 320
         return
      endif
      if (ival .ne. 2) then
         result = 321
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb, 'cpsd:ispin',
     $     ival)) then
         result = 322
         return
      endif
      if (ival .ne. 2) then
         result = 323
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb, 'cgsd:mult',
     $     ival)) then
         result = 324
         return
      endif
      if (ival .ne. 3) then
         result = 325
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb, 'band:mult',
     $     ival)) then
         result = 326
         return
      endif
      if (ival .ne. 3) then
         result = 327
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb, 'cpsd:mult',
     $     ival)) then
         result = 328
         return
      endif
      if (ival .ne. 3) then
         result = 329
         return
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'dos:alpha', 1, dvals)) then
         result = 330
         return
      endif
      if (dabs(dvals(1) - 0.0025d0) .gt. 1.0d-12) then
         result = 331
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb,
     $     'dos:npoints', ival)) then
         result = 332
         return
      endif
      if (ival .ne. 120) then
         result = 333
         return
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'dos:emin', 1, dvals)) then
         result = 334
         return
      endif
      if (dabs(dvals(1) + 0.5d0) .gt. 1.0d-12) then
         result = 335
         return
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'dos:emax', 1, dvals)) then
         result = 336
         return
      endif
      if (dabs(dvals(1) - 1.5d0) .gt. 1.0d-12) then
         result = 337
         return
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'nwpw:init_velocities_temperature', 1, dvals)) then
         result = 248
         return
      endif
      if (dabs(dvals(1) - 325.0d0) .gt. 1.0d-12) then
         result = 249
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb,
     $     'nwpw:init_velocities_seed', ival)) then
         result = 250
         return
      endif
      if (ival .ne. 777) then
         result = 251
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'nwpw:init_velocities', lval)) then
         result = 252
         return
      endif
      if (.not. lval) then
         result = 253
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb, 'nwpw:makehmass2',
     $     lval)) then
         result = 254
         return
      endif
      if (lval) then
         result = 255
         return
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'nwpw:translate_vector', 3, dvals)) then
         result = 256
         return
      endif
      if (dabs(dvals(1) - 0.1d0) .gt. 1.0d-12 .or.
     $    dabs(dvals(2) - 0.2d0) .gt. 1.0d-12 .or.
     $    dabs(dvals(3) - 0.3d0) .gt. 1.0d-12) then
         result = 257
         return
      endif
      text_value = ' '
      if (.not. btdb_cget(rtdb, 'nwpw:translate_geom_name',
     $     1, text_value)) then
         result = 258
         return
      endif
      if (text_value(1:6) .ne. 'mobile') then
         result = 259
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'nwpw:translate_reorder', lval)) then
         result = 260
         return
      endif
      if (lval) then
         result = 261
         return
      endif
      text_value = ' '
      if (.not. btdb_cget(rtdb, 'nwpw:socket_type',
     $     1, text_value)) then
         result = 262
         return
      endif
      if (text_value(1:4) .ne. 'inet') then
         result = 263
         return
      endif
      text_value = ' '
      if (.not. btdb_cget(rtdb, 'nwpw:socket_ip',
     $     1, text_value)) then
         result = 264
         return
      endif
      if (text_value(1:9) .ne. '127.0.0.1') then
         result = 265
         return
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'nwpw_APC:Gc', 1, dvals)) then
         result = 266
         return
      endif
      if (dabs(dvals(1) - 1.25d0) .gt. 1.0d-12) then
         result = 267
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb,
     $     'nwpw_APC:nga', ival)) then
         result = 268
         return
      endif
      if (ival .ne. 3) then
         result = 269
         return
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'nwpw_APC:gamma', 3, dvals)) then
         result = 270
         return
      endif
      if (dabs(dvals(1) - 0.5d0) .gt. 1.0d-12 .or.
     $    dabs(dvals(2) - 0.25d0) .gt. 1.0d-12 .or.
     $    dabs(dvals(3) - 0.125d0) .gt. 1.0d-12) then
         result = 271
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'cgsd:allow_translation', lval)) then
         result = 272
         return
      endif
      if (lval) then
         result = 273
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'band:allow_translation', lval)) then
         result = 274
         return
      endif
      if (lval) then
         result = 275
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb,
     $     'nwpw:minimizer', ival)) then
         result = 276
         return
      endif
      if (ival .ne. 8) then
         result = 277
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb,
     $     'nwpw:ks_algorithm', ival)) then
         result = 278
         return
      endif
      if (ival .ne. -1) then
         result = 279
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb,
     $     'nwpw:scf_algorithm', ival)) then
         result = 280
         return
      endif
      if (ival .ne. 3) then
         result = 281
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'nwpw:precondition', lval)) then
         result = 282
         return
      endif
      if (lval) then
         result = 283
         return
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'nwpw:kerker_g0', 1, dvals)) then
         result = 284
         return
      endif
      if (dabs(dvals(1) - 0.375d0) .gt. 1.0d-12) then
         result = 285
         return
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'nwpw:ks_alpha', 1, dvals)) then
         result = 286
         return
      endif
      if (dabs(dvals(1) - 0.125d0) .gt. 1.0d-12) then
         result = 287
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb,
     $     'nwpw:ks_maxit_orb', ival)) then
         result = 288
         return
      endif
      if (ival .ne. 12) then
         result = 289
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb,
     $     'nwpw:ks_maxit_orbs', ival)) then
         result = 290
         return
      endif
      if (ival .ne. 4) then
         result = 291
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb,
     $     'nwpw:diis_histories', ival)) then
         result = 292
         return
      endif
      if (ival .ne. 6) then
         result = 293
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb,
     $     'nwpw:H1_it_in', ival)) then
         result = 294
         return
      endif
      if (ival .ne. 50) then
         result = 295
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb,
     $     'nwpw:H1_it_out', ival)) then
         result = 296
         return
      endif
      if (ival .ne. 1) then
         result = 297
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb,
     $     'nwpw:H1_it_ortho', ival)) then
         result = 298
         return
      endif
      if (ival .ne. 1) then
         result = 299
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'nwpw:lcao_skip', lval)) then
         result = 300
         return
      endif
      if (.not. lval) then
         result = 301
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'nwpw:cutoff_boot_psi', lval)) then
         result = 302
         return
      endif
      if (lval) then
         result = 303
         return
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'nwpw:smooth_cutoff', 2, dvals)) then
         result = 304
         return
      endif
      if (dabs(dvals(1) - 2.75d0) .gt. 1.0d-12 .or.
     $    dabs(dvals(2) - 3.25d0) .gt. 1.0d-12) then
         result = 305
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'nwpw:fast_erf', lval)) then
         result = 306
         return
      endif
      if (.not. lval) then
         result = 307
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb, 'nwpw:spin_orbit',
     $     lval)) then
         result = 352
         return
      endif
      if (lval) then
         result = 353
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb, 'pspw:spin_orbit',
     $     lval)) then
         result = 354
         return
      endif
      if (lval) then
         result = 355
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb, 'band:spin_orbit',
     $     lval)) then
         result = 356
         return
      endif
      if (lval) then
         result = 357
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb, 'nwpw:parallel_io',
     $     lval)) then
         result = 358
         return
      endif
      if (.not. lval) then
         result = 359
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb, 'cpmd:nose',
     $     lval)) then
         result = 360
         return
      endif
      if (.not. lval) then
         result = 361
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb, 'nwpw:nose',
     $     lval)) then
         result = 362
         return
      endif
      if (.not. lval) then
         result = 363
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb, 'cpmd:nose_restart',
     $     lval)) then
         result = 364
         return
      endif
      if (lval) then
         result = 365
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb, 'nwpw:nose_restart',
     $     lval)) then
         result = 366
         return
      endif
      if (lval) then
         result = 367
         return
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb, 'cpmd:Pe',
     $     1, dvals)) then
         result = 368
         return
      endif
      if (dabs(dvals(1) - 12.0d0) .gt. 1.0d-12) then
         result = 369
         return
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb, 'nwpw:Pe',
     $     1, dvals)) then
         result = 370
         return
      endif
      if (dabs(dvals(1) - 12.0d0) .gt. 1.0d-12) then
         result = 371
         return
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb, 'cpmd:Te',
     $     1, dvals)) then
         result = 372
         return
      endif
      if (dabs(dvals(1) - 300.0d0) .gt. 1.0d-12) then
         result = 373
         return
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb, 'nwpw:Te',
     $     1, dvals)) then
         result = 374
         return
      endif
      if (dabs(dvals(1) - 300.0d0) .gt. 1.0d-12) then
         result = 375
         return
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb, 'cpmd:Pr',
     $     1, dvals)) then
         result = 376
         return
      endif
      if (dabs(dvals(1) - 34.0d0) .gt. 1.0d-12) then
         result = 377
         return
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb, 'nwpw:Pr',
     $     1, dvals)) then
         result = 378
         return
      endif
      if (dabs(dvals(1) - 34.0d0) .gt. 1.0d-12) then
         result = 379
         return
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb, 'cpmd:Tr',
     $     1, dvals)) then
         result = 380
         return
      endif
      if (dabs(dvals(1) - 400.0d0) .gt. 1.0d-12) then
         result = 381
         return
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb, 'nwpw:Tr',
     $     1, dvals)) then
         result = 382
         return
      endif
      if (dabs(dvals(1) - 400.0d0) .gt. 1.0d-12) then
         result = 383
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb, 'cpmd:Mchain',
     $     ival)) then
         result = 384
         return
      endif
      if (ival .ne. 2) then
         result = 385
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb, 'nwpw:Mchain',
     $     ival)) then
         result = 386
         return
      endif
      if (ival .ne. 2) then
         result = 387
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb, 'cpmd:Nchain',
     $     ival)) then
         result = 388
         return
      endif
      if (ival .ne. 3) then
         result = 389
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb, 'nwpw:Nchain',
     $     ival)) then
         result = 390
         return
      endif
      if (ival .ne. 3) then
         result = 391
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'nwpw:atom_efield', lval)) then
         result = 392
         return
      endif
      if (.not. lval) then
         result = 393
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'nwpw:atom_efield_grad', lval)) then
         result = 394
         return
      endif
      if (.not. lval) then
         result = 395
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb, 'nwpw:mulliken',
     $     lval)) then
         result = 396
         return
      endif
      if (.not. lval) then
         result = 397
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'nwpw:mulliken_kawai', lval)) then
         result = 398
         return
      endif
      if (.not. lval) then
         result = 399
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'nwpw:periodic_dipole', lval)) then
         result = 400
         return
      endif
      if (.not. lval) then
         result = 401
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb, 'nwpw:efield',
     $     lval)) then
         result = 402
         return
      endif
      if (.not. lval) then
         result = 403
         return
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'nwpw:efield_vector', 3, dvals)) then
         result = 404
         return
      endif
      if (dabs(dvals(1) - 0.4d0) .gt. 1.0d-12 .or.
     $    dabs(dvals(2) - 0.5d0) .gt. 1.0d-12 .or.
     $    dabs(dvals(3) - 0.6d0) .gt. 1.0d-12) then
         result = 405
         return
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'nwpw:efield_center', 3, dvals)) then
         result = 406
         return
      endif
      if (dabs(dvals(1) - 1.1d0) .gt. 1.0d-12 .or.
     $    dabs(dvals(2) - 1.2d0) .gt. 1.0d-12 .or.
     $    dabs(dvals(3) - 1.3d0) .gt. 1.0d-12) then
         result = 407
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb,
     $     'nwpw:efield_type', ival)) then
         result = 408
         return
      endif
      if (ival .ne. 1) then
         result = 409
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'nwpw:dipole_motion', lval)) then
         result = 410
         return
      endif
      if (.not. lval) then
         result = 411
         return
      endif
      text_value = ' '
      if (.not. btdb_cget(rtdb, 'nwpw:dipole_motion_filename',
     $     1, text_value)) then
         result = 412
         return
      endif
      if (text_value(1:10) .ne. 'dipole.dat') then
         result = 413
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'nwpw:rho_use_symmetry', lval)) then
         result = 414
         return
      endif
      if (lval) then
         result = 415
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb, 'nwpw:fmm',
     $     lval)) then
         result = 416
         return
      endif
      if (.not. lval) then
         result = 417
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb,
     $     'nwpw:fmm_lmax', ival)) then
         result = 418
         return
      endif
      if (ival .ne. 12) then
         result = 419
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb,
     $     'nwpw:fmm_lr', ival)) then
         result = 420
         return
      endif
      if (ival .ne. 2) then
         result = 421
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'nwpw:born_relax', lval)) then
         result = 422
         return
      endif
      if (.not. lval) then
         result = 423
         return
      endif

      return
      end
