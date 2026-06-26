      subroutine nwchemc_rtdb_put_dbl_array(rtdb, key, n, values,
     $     status)
      implicit none
#include "mafdecls.fh"
#include "rtdb.fh"
      integer rtdb, n
      double precision values(*)
      character*(*) key
      logical status
      status = rtdb_put(rtdb, key, mt_dbl, n, values)
      return
      end
