      subroutine nwchemc_rtdb_put_log_array(rtdb, key, n, values,
     $     status)
      implicit none
#include "mafdecls.fh"
#include "rtdb.fh"
      integer rtdb, n
      logical values(*)
      character*(*) key
      logical status
      status = rtdb_put(rtdb, key, mt_log, n, values)
      return
      end
