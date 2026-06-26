      subroutine nwchemc_test_pseudopotential_rtdb(result) bind(C,
     $     name='nwchemc_test_pseudopotential_rtdb')
      use, intrinsic :: iso_c_binding, only: c_int
      implicit none
#include "bafdecls.fh"
#include "btdb.fh"
      integer(c_int), intent(out) :: result
      integer rtdb, init_ok, finish_ok, owns_mpi
      integer psp_types(6)
      integer stored_type
      character*16 psp_elements(6)
      character*256 psp_names(6)
      character*256 text_value
      character*256 errmsg
      logical status, reset_value
      logical nwchemc_probe_get_int, nwchemc_probe_get_log
      external nwchemc_probe_get_int, nwchemc_probe_get_log

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
      if (.not. reset_value) result = 73

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
