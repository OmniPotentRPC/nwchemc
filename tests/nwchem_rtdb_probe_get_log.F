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
