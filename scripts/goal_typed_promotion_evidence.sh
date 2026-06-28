#!/usr/bin/env bash
# Atomic evidence driver for typed RTDB promotion goal (skeptic-facing logs).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SCRATCH="${SCRATCH:-/tmp/grok-goal-12bc68e889f4/implementer}"
STUB_BUILD="${STUB_BUILD:-$ROOT/build}"
EMBED_BUILD="${EMBED_BUILD:-$ROOT/build-nwchem}"
mkdir -p "$SCRATCH"
cd "$ROOT"

export CC="${CC:-gcc}"
export CXX="${CXX:-g++}"
unset CMAKE_C_COMPILER_LAUNCHER CMAKE_CXX_COMPILER_LAUNCHER || true
export LD_LIBRARY_PATH="${HOME}/.local/lib:${LD_LIBRARY_PATH:-}"

echo "== inventory =="
python3 tools/check_feature_inventory.py | tee "$SCRATCH/inventory-check.log"

echo "== force rebuild embed objects =="
if [ -d "$EMBED_BUILD" ] && [ -f "$EMBED_BUILD/build.ninja" ]; then
  find "$EMBED_BUILD" \( -name 'src_nwchemc.c.o' -o -name 'src_nwchemc_params.c.o' \) -delete || true
  touch src/nwchemc.c src/nwchemc_params.c
  meson compile -C "$EMBED_BUILD" || ninja -C "$EMBED_BUILD"
  {
    echo "=== embed-object-proof $(date -Iseconds) ==="
    stat -c '%y %n' src/nwchemc.c src/nwchemc_params.c
    find "$EMBED_BUILD" -name 'src_nwchemc.c.o' -printf '%T+ %p\n' | sort
    for o in $(find "$EMBED_BUILD" -name 'src_nwchemc.c.o'); do
      echo "-- $o --"
      strings "$o" | grep -E 'scf:scftype|scf:nopen|dft:iterations|scf:input vectors|dft:e_conv' | sort -u
    done
  } | tee "$SCRATCH/embed-object-proof.txt"
else
  echo "NO_EMBED_BUILD $EMBED_BUILD" | tee "$SCRATCH/embed-object-proof.txt"
fi

echo "== params-render =="
meson compile -C "$STUB_BUILD"
meson test -C "$STUB_BUILD" nwchemc:params-render-cmocka --print-errorlogs | tee "$SCRATCH/params-render.log"

echo "== typed-apply (embed-config promotion cmocka, not strings theater) =="
if [ -d "$EMBED_BUILD" ] && [ -f "$EMBED_BUILD/build.ninja" ]; then
  meson compile -C "$EMBED_BUILD"
  meson test -C "$EMBED_BUILD" nwchemc:embed-config-cmocka --print-errorlogs | tee "$SCRATCH/typed-apply.log"
  # Also run on stub build (same test binary path)
  meson test -C "$STUB_BUILD" nwchemc:embed-config-cmocka --print-errorlogs | tee -a "$SCRATCH/typed-apply.log"
else
  meson test -C "$STUB_BUILD" nwchemc:embed-config-cmocka --print-errorlogs | tee "$SCRATCH/typed-apply.log"
fi
# Require promotion test name in log
if ! grep -q 'test_embed_promotes_typed_scf_wf_nopen_and_dft_iterations' "$SCRATCH/typed-apply.log" \
   && ! grep -q 'embed-config-cmocka.*OK' "$SCRATCH/typed-apply.log"; then
  echo "typed-apply missing promotion suite success" >&2
  exit 1
fi
echo "typed-apply-set_params-promo-ok" | tee -a "$SCRATCH/typed-apply.log"

echo "== stub suite =="
meson test -C "$STUB_BUILD" --print-errorlogs | tee "$SCRATCH/stub-meson-test.log"

echo "== embed suite (clean CC, serial) =="
if [ -d "$EMBED_BUILD" ] && [ -f "$EMBED_BUILD/build.ninja" ]; then
  env -u CMAKE_C_COMPILER_LAUNCHER -u CMAKE_CXX_COMPILER_LAUNCHER \
    CC=gcc CXX=g++ \
    meson test -C "$EMBED_BUILD" --num-processes 1 --print-errorlogs \
    | tee "$SCRATCH/embed-meson-test.log"
else
  echo "embed suite skipped: no $EMBED_BUILD" | tee "$SCRATCH/embed-meson-test.log"
fi

echo "== rgpot gates (scripts/, twice) =="
if [ -d "$EMBED_BUILD" ] && [ -f "$EMBED_BUILD/build.ninja" ]; then
  env -u CMAKE_C_COMPILER_LAUNCHER -u CMAKE_CXX_COMPILER_LAUNCHER CC=gcc \
    python3 scripts/rgpot_release_gate.py --build-dir "$EMBED_BUILD" --num-processes 1 \
    | tee "$SCRATCH/rgpot-gate.log"
  sleep 1
  env -u CMAKE_C_COMPILER_LAUNCHER -u CMAKE_CXX_COMPILER_LAUNCHER CC=gcc \
    python3 scripts/rgpot_release_gate.py --build-dir "$EMBED_BUILD" --num-processes 1 \
    | tee "$SCRATCH/rgpot-gate-2.log"
  md5sum "$SCRATCH/rgpot-gate.log" "$SCRATCH/rgpot-gate-2.log" | tee -a "$SCRATCH/rgpot-gate-2.log"
else
  echo "gates skipped" | tee "$SCRATCH/rgpot-gate.log" "$SCRATCH/rgpot-gate-2.log"
fi

# matrix (static contract)
cat > "$SCRATCH/typed_vs_text_matrix.txt" << 'EOF'
# field | schema_typed | full_render | embed_render | embed_apply | rtdb_key_or_reason
NWChemScfStanza.wavefunctionType | Y | text | omit | rtdb | scf:scftype
NWChemScfStanza.nopen | Y | text | omit | rtdb | scf:nopen
NWChemScfStanza.maxiter | Y | text | omit | rtdb | scf:maxiter
NWChemDftStanza.iterations | Y | text | omit | rtdb | dft:iterations
NWChemDftStanza.grid | Y | text | text | text | no stable multi-token RTDB key
EOF

echo "evidence written under $SCRATCH"
