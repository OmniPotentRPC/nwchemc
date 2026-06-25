#!/usr/bin/env bash
# Instrumented stub/parser test run + lcov summary for in-tree C sources.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT_DIR="${1:-${ROOT}/build-coverage}"
SCRATCH="${2:-}"
cd "$ROOT"

rm -rf "$OUT_DIR"
meson setup "$OUT_DIR" -Dwith_tests=true -Db_coverage=true
meson test -C "$OUT_DIR" --print-errorlogs
ninja -C "$OUT_DIR" coverage-html 2>/dev/null || true

LCOV_INFO="${OUT_DIR}/coverage.info"
if command -v lcov >/dev/null 2>&1; then
  lcov --capture --directory "$OUT_DIR" --output-file "$LCOV_INFO" \
    --ignore-errors mismatch,gcov,source,unused 2>/dev/null || \
  lcov --capture --directory "$OUT_DIR" --output-file "$LCOV_INFO" || true
  if [[ -f "$LCOV_INFO" ]]; then
    lcov --remove "$LCOV_INFO" \
      '*/subprojects/*' '*/build-coverage/*' '*/.pixi/*' '*/schema/*.capnp.c' \
      --output-file "${LCOV_INFO}.filtered" \
      --ignore-errors unused 2>/dev/null || cp "$LCOV_INFO" "${LCOV_INFO}.filtered"
    lcov --list "${LCOV_INFO}.filtered" | tee "${OUT_DIR}/coverage_summary.txt"
    if [[ -n "$SCRATCH" ]]; then
      mkdir -p "$SCRATCH"
      cp "${OUT_DIR}/coverage_summary.txt" "$SCRATCH/coverage_summary.txt"
      cp "${LCOV_INFO}.filtered" "$SCRATCH/coverage.info" 2>/dev/null || true
    fi
  fi
else
  echo "lcov not installed; meson coverage-html only" | tee "${OUT_DIR}/coverage_summary.txt"
fi

echo "--- gap notes (instrumentable in-tree C target) ---"
cat <<'GAPS'
Covered by stub/parser/cmocka suites (typical): nwchemc_params.c, nwchemc_features.c, nwchemc_stub.c
Excepted / embed-only (needs NWCHEM embed link): nwchemc.c NWCHEMC_HAS_NWCHEM path, nwchem_embed_*.f90/.F, rtdb helpers
Excepted / vendored: subprojects/c-capnproto, generated schema/Potentials.capnp.c
GAPS
if [[ -n "$SCRATCH" ]]; then
  cat >> "$SCRATCH/coverage_gaps.txt" <<'GAPS2'
file/unit	status	notes
src/nwchemc_params.c	covered	parser/render/extract cmocka+capnp-parser
src/nwchemc_features.c	covered	features-cmocka intern tests
src/nwchemc_stub.c	covered	stub-abi + feature-driver
src/nwchemc.c (stub #else)	covered	compiled without NWCHEMC_HAS_NWCHEM in default build
src/nwchemc.c (embed #ifdef)	excepted-embed	requires build-nwchem + real NWChem
src/nwchem_embed_c_api.f90	excepted-embed	Fortran embed only
src/nwchem_embed_legacy.F	excepted-embed	Fortran embed only
src/nwchem_rtdb_put_*.F	excepted-embed	Fortran embed only
subprojects/*	excepted-vendor	third_party
schema/Potentials.capnp.c	excepted-generated	capnpc-c output
GAPS2
fi
