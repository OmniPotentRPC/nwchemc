      subroutine nwchemc_rtdb_put_int_array(rtdb, key, n, values,
     $     status)
      implicit none
#include "mafdecls.fh"
#include "rtdb.fh"
      integer rtdb, n
      integer values(*)
      character*(*) key
      logical status
      status = rtdb_put(rtdb, key, mt_int, n, values)
      return
      end
