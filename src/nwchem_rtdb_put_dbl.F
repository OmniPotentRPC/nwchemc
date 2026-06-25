      subroutine nwchemc_rtdb_put_dbl(rtdb, key, value, status)
      implicit none
#include "mafdecls.fh"
#include "rtdb.fh"
      integer rtdb
      double precision value
      character*(*) key
      logical status
      status = rtdb_put(rtdb, key, mt_dbl, 1, value)
      return
      end
