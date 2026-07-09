Rebuild NWChem archives with `-fPIC` (and GA with `CFLAGS_FORGA`/`FFLAGS_FORGA`)
so meson can produce a shared `libnwchemc.so`. Add `tools/rebuild_nwchem_pic.sh`
and keep applied_* config integers as `int64` so they match the NWChem
`-fdefault-integer-8` embed ABI under gfortran 14+.
