      subroutine nwchemc_test_pseudopotential_text_rtdb(input_blocks,
     $     input_len, result) bind(C,
     $     name='nwchemc_test_pseudopotential_text_rtdb')
      use, intrinsic :: iso_c_binding, only: c_char, c_int
      implicit none
#include "bafdecls.fh"
#include "btdb.fh"
      character(kind=c_char), intent(in) :: input_blocks(*)
      integer(c_int), intent(in) :: input_len
      integer(c_int), intent(out) :: result
      integer rtdb, init_ok, finish_ok, owns_mpi
      integer i, n
      integer stored_type
      integer typed_ivals(2)
      character*8192 deck
      character*256 text_value
      double precision typed_dvals(2)
      logical typed_log, reset_value
      logical nw_inp_from_character
      logical nwchemc_probe_get_int, nwchemc_probe_get_log,
     $     nwchemc_probe_get_int_array, nwchemc_probe_get_dbl_array
      external nw_inp_from_character
      external nwchemc_probe_get_int, nwchemc_probe_get_log,
     $     nwchemc_probe_get_int_array, nwchemc_probe_get_dbl_array

      result = 0
      rtdb = -1
      init_ok = -1
      owns_mpi = 0
      n = input_len
      if (n .le. 0 .or. n .gt. 8192) then
         result = 1
         return
      endif
      deck = ' '
      do i = 1, n
         deck(i:i) = input_blocks(i)
      enddo

      call nwchem_legacy_init(rtdb, init_ok, owns_mpi)
      if (init_ok .ne. 0) then
         result = 2
         return
      endif

      if (.not. nw_inp_from_character(rtdb, deck(1:n))) then
         result = 3
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
      if (stored_type .ne. 2) then
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
     $     'nwpw:pspspin_iamup:_000002', typed_log)) then
         result = 109
         goto 900
      endif
      if (typed_log) then
         result = 110
         goto 900
      endif
      if (.not. nwchemc_probe_get_dbl_array(rtdb,
     $     'nwpw:pspspin_downscale:_000002', 1, typed_dvals)) then
         result = 111
         goto 900
      endif
      if (dabs(typed_dvals(1) - 0.75d0) .gt. 1.0d-12) then
         result = 112
         goto 900
      endif
      if (.not. nwchemc_probe_get_int(rtdb,
     $     'nwpw:pspspin_downl:_000002', stored_type)) then
         result = 113
         goto 900
      endif
      if (stored_type .ne. 2) then
         result = 114
         goto 900
      endif
      if (.not. nwchemc_probe_get_int(rtdb,
     $     'nwpw:pspspin_downm:_000002', stored_type)) then
         result = 115
         goto 900
      endif
      if (stored_type .ne. 0) then
         result = 116
         goto 900
      endif
      if (.not. nwchemc_probe_get_int_array(rtdb,
     $     'nwpw:pspspin_downions:_000002', 1, typed_ivals)) then
         result = 117
         goto 900
      endif
      if (typed_ivals(1) .ne. 2) then
         result = 118
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

  900 continue
      call nwchem_legacy_finalize(rtdb, owns_mpi, finish_ok)
      if (result .eq. 0 .and. finish_ok .ne. 0) result = 200
      return
      end

      subroutine nwchemc_test_pseudopotential_text_reset_rtdb(
     $     input_blocks, input_len, result) bind(C,
     $     name='nwchemc_test_pseudopotential_text_reset_rtdb')
      use, intrinsic :: iso_c_binding, only: c_char, c_int
      implicit none
#include "bafdecls.fh"
      character(kind=c_char), intent(in) :: input_blocks(*)
      integer(c_int), intent(in) :: input_len
      integer(c_int), intent(out) :: result
      integer rtdb, init_ok, finish_ok, owns_mpi
      integer i, n
      integer stored_type
      character*8192 deck
      logical typed_log
      logical nw_inp_from_character
      logical nwchemc_probe_get_int, nwchemc_probe_get_log
      external nw_inp_from_character
      external nwchemc_probe_get_int, nwchemc_probe_get_log

      result = 0
      rtdb = -1
      init_ok = -1
      owns_mpi = 0
      n = input_len
      if (n .le. 0 .or. n .gt. 8192) then
         result = 1
         return
      endif
      deck = ' '
      do i = 1, n
         deck(i:i) = input_blocks(i)
      enddo

      call nwchem_legacy_init(rtdb, init_ok, owns_mpi)
      if (init_ok .ne. 0) then
         result = 2
         return
      endif

      if (.not. nw_inp_from_character(rtdb, deck(1:n))) then
         result = 3
         goto 900
      endif

      if (.not. nwchemc_probe_get_log(rtdb, 'nwpw:pspspin',
     $     typed_log)) then
         result = 10
         goto 900
      endif
      if (typed_log) then
         result = 11
         goto 900
      endif
      if (.not. nwchemc_probe_get_int(rtdb, 'nwpw:pspspin_count',
     $     stored_type)) then
         result = 12
         goto 900
      endif
      if (stored_type .ne. 0) then
         result = 13
         goto 900
      endif
      if (.not. nwchemc_probe_get_log(rtdb,
     $     'nwpw:psp:semicore_small', typed_log)) then
         result = 14
         goto 900
      endif
      if (typed_log) then
         result = 15
         goto 900
      endif
      if (.not. nwchemc_probe_get_log(rtdb, 'nwpw:uterm',
     $     typed_log)) then
         result = 16
         goto 900
      endif
      if (typed_log) then
         result = 17
         goto 900
      endif
      if (.not. nwchemc_probe_get_int(rtdb, 'nwpw:nuterms',
     $     stored_type)) then
         result = 18
         goto 900
      endif
      if (stored_type .ne. 0) then
         result = 19
         goto 900
      endif

  900 continue
      call nwchem_legacy_finalize(rtdb, owns_mpi, finish_ok)
      if (result .eq. 0 .and. finish_ok .ne. 0) result = 200
      return
      end
