      subroutine nwchemc_test_configured_pseudopotential_rtdb(result)
     $     bind(C, name='nwchemc_test_configured_pseudopotential_rtdb')
      use, intrinsic :: iso_c_binding, only: c_int, c_long_long
      implicit none
#include "bafdecls.fh"
#include "btdb.fh"
      integer(c_int), intent(out) :: result
      integer rtdb
      integer stored_type
      integer typed_ivals(2)
      integer(c_long_long) current_rtdb
      character*256 text_value
      double precision typed_dvals(2)
      logical reset_value, typed_log
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
         result = 1
         return
      endif
      rtdb = int(current_rtdb, kind(rtdb))

      text_value = ' '
      if (.not. btdb_cget(rtdb, 'Si:pspw_library', 1,
     $     text_value)) then
         result = 10
         return
      endif
      if (text_value(1:4) .ne. 'sg15') then
         result = 11
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb, 'Si:pspw_library_type',
     $     stored_type)) then
         result = 12
         return
      endif
      if (stored_type .ne. 0) then
         result = 13
         return
      endif

      text_value = ' '
      if (.not. btdb_cget(rtdb, 'H:pspw_library', 1,
     $     text_value)) then
         result = 20
         return
      endif
      if (text_value(1:7) .ne. 'hgh_lda') then
         result = 21
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb, 'H:pspw_library_type',
     $     stored_type)) then
         result = 22
         return
      endif
      if (stored_type .ne. 0) then
         result = 23
         return
      endif

      text_value = ' '
      if (.not. btdb_cget(rtdb, 'O:paw_library', 1,
     $     text_value)) then
         result = 30
         return
      endif
      if (text_value(1:11) .ne. 'paw_default') then
         result = 31
         return
      endif

      text_value = ' '
      if (.not. btdb_cget(rtdb, 'C:pspw_library', 1,
     $     text_value)) then
         result = 40
         return
      endif
      if (text_value(1:5) .ne. 'C.cpi') then
         result = 41
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb, 'C:pspw_library_type',
     $     stored_type)) then
         result = 42
         return
      endif
      if (stored_type .ne. 2) then
         result = 43
         return
      endif

      text_value = ' '
      if (.not. btdb_cget(rtdb, 'N:pspw_library', 1,
     $     text_value)) then
         result = 50
         return
      endif
      if (text_value(1:7) .ne. 'N.teter') then
         result = 51
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb, 'N:pspw_library_type',
     $     stored_type)) then
         result = 52
         return
      endif
      if (stored_type .ne. 3) then
         result = 53
         return
      endif

      text_value = ' '
      if (.not. btdb_cget(rtdb, '*:pspw_library', 1,
     $     text_value)) then
         result = 60
         return
      endif
      if (text_value(1:12) .ne. 'pspw_default') then
         result = 61
         return
      endif

      if (.not. nwchemc_probe_get_log(rtdb, 'Si:psp_reset',
     $     reset_value)) then
         result = 70
         return
      endif
      if (.not. reset_value) then
         result = 71
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb, '*:psp_reset',
     $     reset_value)) then
         result = 72
         return
      endif
      if (.not. reset_value) then
         result = 73
         return
      endif

      if (.not. nwchemc_probe_get_log(rtdb, 'nwpw:pspspin',
     $     typed_log)) then
         result = 81
         return
      endif
      if (.not. typed_log) then
         result = 82
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb, 'nwpw:pspspin_count',
     $     stored_type)) then
         result = 83
         return
      endif
      if (stored_type .ne. 1) then
         result = 84
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'nwpw:pspspin_iamup:_000001', typed_log)) then
         result = 85
         return
      endif
      if (.not. typed_log) then
         result = 86
         return
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'nwpw:pspspin_upscale:_000001', 1, typed_dvals)) then
         result = 87
         return
      endif
      if (dabs(typed_dvals(1) - 1.25d0) .gt. 1.0d-12) then
         result = 88
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb,
     $     'nwpw:pspspin_upl:_000001', stored_type)) then
         result = 91
         return
      endif
      if (stored_type .ne. 1) then
         result = 92
         return
      endif
      if (.not. nwchemc_probe_get_int_array(rtdb,
     $     'nwpw:pspspin_upions:_000001', 2, typed_ivals)) then
         result = 93
         return
      endif
      if (typed_ivals(1) .ne. 2 .or. typed_ivals(2) .ne. 3) then
         result = 94
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'nwpw:psp:semicore_small', typed_log)) then
         result = 95
         return
      endif
      if (.not. typed_log) then
         result = 96
         return
      endif
      if (.not. nwchemc_probe_get_log(rtdb, 'nwpw:uterm',
     $     typed_log)) then
         result = 97
         return
      endif
      if (.not. typed_log) then
         result = 98
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb, 'nwpw:nuterms',
     $     stored_type)) then
         result = 99
         return
      endif
      if (stored_type .ne. 1) then
         result = 100
         return
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'nwpw:uterm_scale:_000001', 1, typed_dvals)) then
         result = 101
         return
      endif
      if (dabs(typed_dvals(1) - 1.4d0) .gt. 1.0d-12) then
         result = 102
         return
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'nwpw:jterm_scale:_000001', 1, typed_dvals)) then
         result = 103
         return
      endif
      if (dabs(typed_dvals(1) - 0.2d0) .gt. 1.0d-12) then
         result = 104
         return
      endif
      if (.not. nwchemc_probe_get_int(rtdb, 'nwpw:uterm_l:_000001',
     $     stored_type)) then
         result = 105
         return
      endif
      if (stored_type .ne. 2) then
         result = 106
         return
      endif
      if (.not. nwchemc_probe_get_int_array(rtdb,
     $     'nwpw:uterm_ions:_000001', 2, typed_ivals)) then
         result = 107
         return
      endif
      if (typed_ivals(1) .ne. 4 .or. typed_ivals(2) .ne. 6) then
         result = 108
         return
      endif

      return
      end
