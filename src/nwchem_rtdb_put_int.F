      subroutine nwchemc_rtdb_put_int(rtdb, key, value, status)
      implicit none
#include "mafdecls.fh"
#include "rtdb.fh"
      integer rtdb, value
      character*(*) key
      logical status
      status = rtdb_put(rtdb, key, mt_int, 1, value)
      return
      end
