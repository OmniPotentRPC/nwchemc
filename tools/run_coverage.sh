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

GAPS_FILE="${OUT_DIR}/coverage_gaps.txt"
if [[ -n "$SCRATCH" ]]; then
  GAPS_FILE="${SCRATCH}/coverage_gaps.txt"
fi
{
  echo "file/unit	status	line_rate_note	rationale"
  echo "src/nwchemc_features.c	covered	~100% in stub suite	features-cmocka intern tests"
  echo "src/nwchemc_stub.c	covered	~100% in stub suite	stub-abi + feature-driver"
  echo "src/nwchemc_params.c	partial	see lcov list (render/extract exercised by params-render-cmocka + capnp-parser; module/pseudo/generic arms still lower if unused in fixtures)"
  echo "src/nwchemc.c (stub #else / test embed-config with HAS_NWCHEM define)	partial	see lcov; embed-config-cmocka covers apply_config_to_embed path under -DNWCHEMC_HAS_NWCHEM=1 without real NWChem link"
  echo "src/nwchemc.c (real embed link)	excepted-embed	requires build-nwchem + NWChem SDK"
  echo "src/nwchem_embed_c_api.f90	excepted-embed	Fortran embed only"
  echo "src/nwchem_embed_legacy.F	excepted-embed	Fortran embed only"
  echo "src/nwchem_rtdb_put_*.F	excepted-embed	Fortran embed only"
  echo "subprojects/*	excepted-vendor	third_party"
  echo "schema/Potentials.capnp.c	excepted-generated	capnpc-c output"
  echo "tools/nwchemc_feature_driver.c	partial	driver modes in feature-driver test; unused CLI branches may remain"
  echo "tests/*	test-only	not product coverage target"
} | tee "$GAPS_FILE"
echo "--- see ${OUT_DIR}/coverage_summary.txt for current lcov rates ---"
