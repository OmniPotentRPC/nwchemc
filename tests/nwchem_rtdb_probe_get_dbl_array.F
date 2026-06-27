      logical function nwchemc_probe_get_dbl_array(rtdb, key, n, values)
      implicit none
#include "bafdecls.fh"
#include "btdb.fh"
      integer rtdb, n
      double precision values(*)
      character*(*) key
      nwchemc_probe_get_dbl_array = rtdb_get(rtdb, key, mt_dbl, n,
     $     values)
      return
      end
