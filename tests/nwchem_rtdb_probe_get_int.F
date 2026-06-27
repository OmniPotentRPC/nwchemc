      logical function nwchemc_probe_get_int(rtdb, key, value)
      implicit none
#include "bafdecls.fh"
#include "btdb.fh"
      integer rtdb, value
      character*(*) key
      nwchemc_probe_get_int = btdb_get(rtdb, key, mt_int, 1, value)
      return
      end
