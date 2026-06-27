      subroutine nwchemc_test_pseudopotential_rtdb(result) bind(C,
     $     name='nwchemc_test_pseudopotential_rtdb')
      use, intrinsic :: iso_c_binding, only: c_int
      implicit none
#include "bafdecls.fh"
#include "btdb.fh"
      integer(c_int), intent(out) :: result
      integer rtdb, init_ok, finish_ok, owns_mpi
      integer i, j
      integer psp_types(6)
      integer stored_type
      integer typed_count
      integer typed_types(13)
      integer typed_value_counts(13)
      integer typed_ivals(2)
      character*16 psp_elements(6)
      character*256 psp_names(6)
      character*128 typed_keys(13)
      character*256 typed_values(64,13)
      character*256 text_value
      character*256 errmsg
      double precision typed_dvals(2)
      logical status, reset_value
      logical typed_log
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

      psp_elements(1) = 'Si'
      psp_names(1) = 'sg15'
      psp_types(1) = 0
      psp_elements(2) = 'H'
      psp_names(2) = 'hgh_lda'
      psp_types(2) = 1
      psp_elements(3) = 'O'
      psp_names(3) = 'paw_default'
      psp_types(3) = 2
      psp_elements(4) = 'C'
      psp_names(4) = 'C.cpi'
      psp_types(4) = 3
      psp_elements(5) = 'N'
      psp_names(5) = 'N.teter'
      psp_types(5) = 4
      psp_elements(6) = '*'
      psp_names(6) = 'pspw_default'
      psp_types(6) = 1

      call nwchemc_store_pseudopotentials(rtdb, 6, psp_elements,
     $     psp_types, psp_names, errmsg, status)
      if (.not. status) then
         result = 2
         goto 900
      endif

      text_value = ' '
      if (.not. btdb_cget(rtdb, 'Si:pspw_library', 1,
     $     text_value)) then
         result = 10
         goto 900
      endif
      if (text_value(1:4) .ne. 'sg15') then
         result = 11
         goto 900
      endif
      if (.not. nwchemc_probe_get_int(rtdb, 'Si:pspw_library_type',
     $     stored_type)) then
         result = 12
         goto 900
      endif
      if (stored_type .ne. 0) then
         result = 13
         goto 900
      endif

      text_value = ' '
      if (.not. btdb_cget(rtdb, 'H:pspw_library', 1,
     $     text_value)) then
         result = 20
         goto 900
      endif
      if (text_value(1:7) .ne. 'hgh_lda') then
         result = 21
         goto 900
      endif
      if (.not. nwchemc_probe_get_int(rtdb, 'H:pspw_library_type',
     $     stored_type)) then
         result = 22
         goto 900
      endif
      if (stored_type .ne. 0) then
         result = 23
         goto 900
      endif

      text_value = ' '
      if (.not. btdb_cget(rtdb, 'O:paw_library', 1,
     $     text_value)) then
         result = 30
         goto 900
      endif
      if (text_value(1:11) .ne. 'paw_default') then
         result = 31
         goto 900
      endif

      text_value = ' '
      if (.not. btdb_cget(rtdb, 'C:pspw_library', 1,
     $     text_value)) then
         result = 40
         goto 900
      endif
      if (text_value(1:5) .ne. 'C.cpi') then
         result = 41
         goto 900
      endif
      if (.not. nwchemc_probe_get_int(rtdb, 'C:pspw_library_type',
     $     stored_type)) then
         result = 42
         goto 900
      endif
      if (stored_type .ne. 2) then
         result = 43
         goto 900
      endif

      text_value = ' '
      if (.not. btdb_cget(rtdb, 'N:pspw_library', 1,
     $     text_value)) then
         result = 50
         goto 900
      endif
      if (text_value(1:7) .ne. 'N.teter') then
         result = 51
         goto 900
      endif
      if (.not. nwchemc_probe_get_int(rtdb, 'N:pspw_library_type',
     $     stored_type)) then
         result = 52
         goto 900
      endif
      if (stored_type .ne. 3) then
         result = 53
         goto 900
      endif

      text_value = ' '
      if (.not. btdb_cget(rtdb, '*:pspw_library', 1,
     $     text_value)) then
         result = 60
         goto 900
      endif
      if (text_value(1:12) .ne. 'pspw_default') then
         result = 61
         goto 900
      endif

      if (.not. nwchemc_probe_get_log(rtdb, 'Si:psp_reset',
     $     reset_value)) then
         result = 70
         goto 900
      endif
      if (.not. reset_value) then
         result = 71
         goto 900
      endif
      if (.not. nwchemc_probe_get_log(rtdb, '*:psp_reset',
     $     reset_value)) then
         result = 72
         goto 900
      endif
      if (.not. reset_value) then
         result = 73
         goto 900
      endif

      do i = 1, 13
         typed_keys(i) = ' '
         typed_types(i) = 0
         typed_value_counts(i) = 0
         do j = 1, 64
            typed_values(j,i) = ' '
         enddo
      enddo
      typed_count = 13
      typed_keys(1) = 'nwpw:pspspin'
      typed_types(1) = 4
      typed_value_counts(1) = 1
      typed_values(1,1) = 'true'
      typed_keys(2) = 'nwpw:pspspin_count'
      typed_types(2) = 3
      typed_value_counts(2) = 1
      typed_values(1,2) = '1'
      typed_keys(3) = 'nwpw:pspspin_iamup:_000001'
      typed_types(3) = 4
      typed_value_counts(3) = 1
      typed_values(1,3) = 'true'
      typed_keys(4) = 'nwpw:pspspin_upscale:_000001'
      typed_types(4) = 2
      typed_value_counts(4) = 1
      typed_values(1,4) = '1.25'
      typed_keys(5) = 'nwpw:pspspin_upl:_000001'
      typed_types(5) = 3
      typed_value_counts(5) = 1
      typed_values(1,5) = '1'
      typed_keys(6) = 'nwpw:pspspin_upions:_000001'
      typed_types(6) = 3
      typed_value_counts(6) = 2
      typed_values(1,6) = '2'
      typed_values(2,6) = '3'
      typed_keys(7) = 'nwpw:psp:semicore_small'
      typed_types(7) = 4
      typed_value_counts(7) = 1
      typed_values(1,7) = 'true'
      typed_keys(8) = 'nwpw:uterm'
      typed_types(8) = 4
      typed_value_counts(8) = 1
      typed_values(1,8) = 'true'
      typed_keys(9) = 'nwpw:nuterms'
      typed_types(9) = 3
      typed_value_counts(9) = 1
      typed_values(1,9) = '1'
      typed_keys(10) = 'nwpw:uterm_scale:_000001'
      typed_types(10) = 2
      typed_value_counts(10) = 1
      typed_values(1,10) = '1.4'
      typed_keys(11) = 'nwpw:jterm_scale:_000001'
      typed_types(11) = 2
      typed_value_counts(11) = 1
      typed_values(1,11) = '0.2'
      typed_keys(12) = 'nwpw:uterm_l:_000001'
      typed_types(12) = 3
      typed_value_counts(12) = 1
      typed_values(1,12) = '2'
      typed_keys(13) = 'nwpw:uterm_ions:_000001'
      typed_types(13) = 3
      typed_value_counts(13) = 2
      typed_values(1,13) = '4'
      typed_values(2,13) = '6'

      call nwchemc_apply_direct_typed_sets(rtdb, typed_count,
     $     typed_keys, typed_types, typed_value_counts, typed_values,
     $     errmsg, status)
      if (.not. status) then
         result = 80
         goto 900
      endif

      if (.not. nwchemc_probe_get_log(rtdb, 'nwpw:pspspin',
     $     typed_log)) then
         result = 81
         goto 900
      endif
      if (.not. typed_log) then
         result = 82
         goto 900
      endif
      if (.not. nwchemc_probe_get_int(rtdb, 'nwpw:pspspin_count',
     $     stored_type)) then
         result = 83
         goto 900
      endif
      if (stored_type .ne. 1) then
         result = 84
         goto 900
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'nwpw:pspspin_iamup:_000001', typed_log)) then
         result = 85
         goto 900
      endif
      if (.not. typed_log) then
         result = 86
         goto 900
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'nwpw:pspspin_upscale:_000001', 1, typed_dvals)) then
         result = 87
         goto 900
      endif
      if (dabs(typed_dvals(1) - 1.25d0) .gt. 1.0d-12) then
         result = 88
         goto 900
      endif
      if (.not. nwchemc_probe_get_int(rtdb,
     $     'nwpw:pspspin_upl:_000001', stored_type)) then
         result = 91
         goto 900
      endif
      if (stored_type .ne. 1) then
         result = 92
         goto 900
      endif
      if (.not. nwchemc_probe_get_int_array(rtdb,
     $     'nwpw:pspspin_upions:_000001', 2, typed_ivals)) then
         result = 93
         goto 900
      endif
      if (typed_ivals(1) .ne. 2 .or. typed_ivals(2) .ne. 3) then
         result = 94
         goto 900
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'nwpw:psp:semicore_small', typed_log)) then
         result = 95
         goto 900
      endif
      if (.not. typed_log) then
         result = 96
         goto 900
      endif
      if (.not. nwchemc_probe_get_log(rtdb, 'nwpw:uterm',
     $     typed_log)) then
         result = 97
         goto 900
      endif
      if (.not. typed_log) then
         result = 98
         goto 900
      endif
      if (.not. nwchemc_probe_get_int(rtdb, 'nwpw:nuterms',
     $     stored_type)) then
         result = 99
         goto 900
      endif
      if (stored_type .ne. 1) then
         result = 100
         goto 900
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'nwpw:uterm_scale:_000001', 1, typed_dvals)) then
         result = 101
         goto 900
      endif
      if (dabs(typed_dvals(1) - 1.4d0) .gt. 1.0d-12) then
         result = 102
         goto 900
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'nwpw:jterm_scale:_000001', 1, typed_dvals)) then
         result = 103
         goto 900
      endif
      if (dabs(typed_dvals(1) - 0.2d0) .gt. 1.0d-12) then
         result = 104
         goto 900
      endif
      if (.not. nwchemc_probe_get_int(rtdb, 'nwpw:uterm_l:_000001',
     $     stored_type)) then
         result = 105
         goto 900
      endif
      if (stored_type .ne. 2) then
         result = 106
         goto 900
      endif
      if (.not. nwchemc_probe_get_int_array(rtdb,
     $     'nwpw:uterm_ions:_000001', 2, typed_ivals)) then
         result = 107
         goto 900
      endif
      if (typed_ivals(1) .ne. 4 .or. typed_ivals(2) .ne. 6) then
         result = 108
         goto 900
      endif

 900  continue
      finish_ok = 0
      call nwchem_legacy_finalize(rtdb, owns_mpi, finish_ok)
      if ((finish_ok .ne. 0) .and. (result .eq. 0)) result = 90
      return
      end

      logical function nwchemc_probe_get_int(rtdb, key, value)
      implicit none
#include "bafdecls.fh"
#include "btdb.fh"
      integer rtdb, value
      character*(*) key
      nwchemc_probe_get_int = btdb_get(rtdb, key, mt_int, 1, value)
      return
      end

      logical function nwchemc_probe_get_log(rtdb, key, value)
      implicit none
#include "bafdecls.fh"
#include "btdb.fh"
      integer rtdb
      logical value
      character*(*) key
      nwchemc_probe_get_log = rtdb_get(rtdb, key, mt_log, 1, value)
      return
      end

      logical function nwchemc_probe_get_int_array(rtdb, key, n, values)
      implicit none
#include "bafdecls.fh"
#include "btdb.fh"
      integer rtdb, n
      integer values(*)
      character*(*) key
      nwchemc_probe_get_int_array = rtdb_get(rtdb, key, mt_int, n,
     $     values)
      return
      end
