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
