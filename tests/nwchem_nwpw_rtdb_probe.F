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

 900  continue
      finish_ok = 0
      call nwchem_legacy_finalize(rtdb, owns_mpi, finish_ok)
      if ((finish_ok .ne. 0) .and. (result .eq. 0)) result = 100
      return
      end
