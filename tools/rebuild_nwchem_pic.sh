#!/usr/bin/env bash
# Rebuild NWChem LINUX64 archives with -fPIC so meson can link libnwchemc.so.
#
# Usage (from a host with the NWChem tree already configured for MPI):
#   export NWCHEM_TOP=/path/to/nwchem
#   export MPI_LOC=/path/to/openmpi   # optional if FORCE_MPI_ENV
#   ./tools/rebuild_nwchem_pic.sh
#
# Requirements already present in NWCHEM_TOP:
#   - src/config/nwchem_config.h (modules)
#   - src/tools/install (GA)
#   - src/libext (OpenBLAS etc.)
#
# Env overrides: NWCHEM_TARGET, NWCHEM_MODULES, EXTRA_FOPTIONS, jobs (-j).

set -euo pipefail

: "${NWCHEM_TOP:?set NWCHEM_TOP to the NWChem source tree}"
export NWCHEM_TARGET="${NWCHEM_TARGET:-LINUX64}"
export USE_MPI="${USE_MPI:-y}"
export USE_MPIF="${USE_MPIF:-y}"
export USE_MPIF4="${USE_MPIF4:-y}"
export USE_SERIALEIGENSOLVERS="${USE_SERIALEIGENSOLVERS:-y}"
export EXTRA_FOPTIONS="${EXTRA_FOPTIONS:--fPIC}"
export EXTRA_COPTIONS="${EXTRA_COPTIONS:--fPIC}"
export LARGE_FILES="${LARGE_FILES:-TRUE}"
export USE_NOFSCHECK="${USE_NOFSCHECK:-TRUE}"
export FC="${FC:-gfortran}"
export CC="${CC:-gcc}"
export CXX="${CXX:-g++}"
JOBS="${JOBS:-$(nproc 2>/dev/null || echo 8)}"

if [[ -n "${MPI_LOC:-}" ]]; then
  export FORCE_MPI_ENV=y
  export MPI_LIB="${MPI_LIB:-$MPI_LOC/lib}"
  export MPI_INCLUDE="${MPI_INCLUDE:-$MPI_LOC/include}"
  export LIBMPI="${LIBMPI:--lmpi_mpifh -lmpi}"
  export PATH="$MPI_LOC/bin:${PATH}"
fi

if [[ -z "${BLASOPT:-}" && -d "$NWCHEM_TOP/src/libext/lib" ]]; then
  export BLASOPT="-L$NWCHEM_TOP/src/libext/lib -lnwc_openblas"
  export LAPACK_LIB="${LAPACK_LIB:-$BLASOPT}"
  export BLAS_SIZE="${BLAS_SIZE:-8}"
fi

export CFLAGS_FORGA="${CFLAGS_FORGA:--fPIC -std=gnu17}"
export FFLAGS_FORGA="${FFLAGS_FORGA:--fPIC}"

if [[ "${REBUILD_GA:-y}" == "y" ]]; then
  echo "==> rebuild GA/tools with CFLAGS_FORGA=$CFLAGS_FORGA"
  (
    cd "$NWCHEM_TOP/src/tools"
    rm -rf build install
    make -j"$JOBS"
  )
fi

cd "$NWCHEM_TOP/src"
echo "==> clean module objects (libext kept)"
make clean
if [[ -n "${NWCHEM_MODULES:-}" ]]; then
  make nwchem_config NWCHEM_MODULES="$NWCHEM_MODULES"
fi
echo "==> make -j$JOBS with EXTRA_FOPTIONS=$EXTRA_FOPTIONS"
make -j"$JOBS"
# NWChem removes stubs.o after linking the binary; restore for meson probe.
make stubs.o

# Archive PIC checks (objects live in .a after install).
check_archive_symbol() {
  local archive=$1 object=$2 pattern=$3
  local tmp
  tmp=$(mktemp -d)
  (
    cd "$tmp"
    ar x "$archive" "$object"
    if readelf -r "$object" | grep -qE "$pattern"; then
      echo "error: $archive($object) still matches $pattern" >&2
      readelf -r "$object" | grep -E "$pattern" | head -5 >&2
      exit 1
    fi
  )
  rm -rf "$tmp"
  echo "==> PIC check ok: $archive($object)"
}

if [[ -f "$NWCHEM_TOP/lib/$NWCHEM_TARGET/libccsd.a" ]]; then
  check_archive_symbol \
    "$NWCHEM_TOP/lib/$NWCHEM_TARGET/libccsd.a" moints_trp.o \
    'R_X86_64_PC32.*mbc_dbl'
fi
if [[ -f "$NWCHEM_TOP/src/tools/install/lib/libga.a" ]]; then
  check_archive_symbol \
    "$NWCHEM_TOP/src/tools/install/lib/libga.a" random.o \
    'R_X86_64_PC32.*stderr'
fi
echo "==> done: rebuild $NWCHEM_TOP for shared nwchemc link"
