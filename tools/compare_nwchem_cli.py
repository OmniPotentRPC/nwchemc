#!/usr/bin/env python3
"""Compare pixi/conda-forge nwchem CLI vs optional embed on integration tasks.

CLI leg: run each tests/integration/nw/*.nw through `nwchem` on PATH.
Embed leg: when --embed-build is set, run the energy-gradient test binary with
NWCHEMC_COMPARE_JSON=1 and assert energy/gradient deltas within task_matrix tolerances.

Exit codes:
  0  all requested legs pass (CLI-only when embed not requested)
  1  numeric or CLI failure
  2  incomplete (CLI missing with --require-cli, or embed requested but unusable)
  3  incomplete-compare: embed requested, CLI ok, embed leg not available
"""
from __future__ import annotations

import argparse
import json
import os
import re
import shlex
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MATRIX = ROOT / "tests/integration/task_matrix.json"

# Method totals (prefer chronologically last match among these — never SCF first).
METHOD_ENERGY_RES = [
    re.compile(r"Excited\s+state\s+energy\s*=\s*([-+0-9.Ee+]+)", re.IGNORECASE),
    re.compile(r"Total\s+SO-DFT\s+energy\s*=\s*([-+0-9.Ee+]+)", re.IGNORECASE),
    re.compile(r"Total\s+MCSCF\s+energy\s*=\s*([-+0-9.Ee+]+)", re.IGNORECASE),
    re.compile(r">>>\|\s*MCSCF\s+energy:\s*([-+0-9.Ee+]+)", re.IGNORECASE),
    re.compile(r"Root\s+\d+\s+final\s+energy\s*=\s*([-+0-9.Ee+]+)", re.IGNORECASE),
    re.compile(r"Total\s+MP2\s+energy\s+([-+0-9.Ee+]+)", re.IGNORECASE),
    re.compile(r"Total\s+DFT\s+energy\s*=\s*([-+0-9.Ee+]+)", re.IGNORECASE),
    # Exclude SCS-CCSD (printed after plain CCSD; last-match would steal it).
    re.compile(r"Total\s+CCSD\s+energy:\s*([-+0-9.Ee+]+)", re.IGNORECASE),
    re.compile(r"Total\s+CCSD\(T\)\s+energy:\s*([-+0-9.Ee+]+)", re.IGNORECASE),
    re.compile(r"CCSD\s+total\s+energy\s*/\s*hartree\s*=\s*([-+0-9.Ee+]+)", re.IGNORECASE),
    re.compile(r"CCSD\(T\)\s+total\s+energy\s*/\s*hartree\s*=\s*([-+0-9.Ee+]+)", re.IGNORECASE),
    re.compile(r"CCSDt?\s+total\s+energy\s*/\s*hartree\s*=\s*([-+0-9.Ee+]+)", re.IGNORECASE),
    # TCE method totals: "CCD/CISD/MBPT(n)/CC2/LCCSD/... total energy / hartree ="
    re.compile(
        r"(?:L?CCD|CC2|CISDT?|LCCSD|MBPT\(\d+\)(?:\(SDQ\))?|CR-CCSD[\[(]T[\])]|"
        r"Lambda-CCSD[\[(]T[\])]|CCSD\[T\]|CCSDT|CCSDTQ)\s+total\s+energy\s*/\s*hartree\s*=\s*([-+0-9.Ee+]+)",
        re.IGNORECASE,
    ),
    re.compile(r"SELCI\s+energy\s*=\s*([-+0-9.Ee+]+)", re.IGNORECASE),
    re.compile(r"Final\s+CI\s+energy\s*=\s*([-+0-9.Ee+]+)", re.IGNORECASE),
    re.compile(r"ci\+pt\s+energy\s+([-+0-9.Ee+]+)", re.IGNORECASE),
]
# Only if no method-total line appears (pure SCF jobs).
FALLBACK_ENERGY_RES = [
    re.compile(r"Total\s+SCF\s+energy\s*=\s*([-+0-9.Ee+]+)", re.IGNORECASE),
    re.compile(r"^\s*Total\s+energy\s*=\s*([-+0-9.Ee+]+)", re.IGNORECASE | re.M),
]
# Back-compat aliases for callers/tests that imported these names.
ENERGY_RE = FALLBACK_ENERGY_RES[0]
ENERGY_RE_ALT = METHOD_ENERGY_RES + FALLBACK_ENERGY_RES

# NWChem prints gradients under several banners; only search after a banner.
GRAD_BANNER_RE = re.compile(
    r"(?:DFT\s+ENERGY\s+GRADIENTS|RHF\s+ENERGY\s+GRADIENTS|ENERGY\s+GRADIENTS|"
    r"Nuclear\s+Gradient)",
    re.IGNORECASE,
)
# Atom table: index symbol x y z gx gy gz (gradient columns are last three).
GRAD_TABLE_LINE_RE = re.compile(
    r"^\s*\d+\s+[A-Za-z]+\s+"
    r"[-+0-9.Ee]+\s+[-+0-9.Ee]+\s+[-+0-9.Ee]+\s+"
    r"([-+0-9.Ee]+)\s+([-+0-9.Ee]+)\s+([-+0-9.Ee]+)",
    re.MULTILINE,
)
# Compact: optional symbol before index, then three floats (legacy layouts).
GRAD_LINE_RE = re.compile(
    r"^\s*(?:[A-Za-z]+\s+)?\d+\s+([-+0-9.Ee+]+)\s+([-+0-9.Ee+]+)\s+([-+0-9.Ee+]+)\s*$",
    re.MULTILINE,
)

# Final optimized geometry table under "Output coordinates in angstroms".
OUTPUT_COORDS_BANNER_RE = re.compile(
    r"Output\s+coordinates\s+in\s+angstroms", re.IGNORECASE
)
ATOM_XYZ_LINE_RE = re.compile(
    r"^\s*\d+\s+\S+\s+[-+0-9.Ee]+\s+([-+0-9.Ee]+)\s+([-+0-9.Ee]+)\s+([-+0-9.Ee]+)\s*$",
    re.MULTILINE,
)
# Harmonic frequencies line (cm-1); take chronologically last match.
# Use [ \t] not \s so values cannot spill into following mode-table lines.
FREQUENCY_LINE_RE = re.compile(
    r"^\s*Frequency[ \t]+((?:[-+0-9.Ee]+[ \t]*)+)$",
    re.MULTILINE | re.IGNORECASE,
)
PFREQUENCY_LINE_RE = re.compile(
    r"^\s*P\.Frequency[ \t]+((?:[-+0-9.Ee]+[ \t]*)+)$",
    re.MULTILINE | re.IGNORECASE,
)
DMX_RE = re.compile(r"^\s*DMX\s+([-+0-9.Ee]+)\s+DMXEFC", re.MULTILINE)
DMY_RE = re.compile(r"^\s*DMY\s+([-+0-9.Ee]+)\s+DMYEFC", re.MULTILINE)
DMZ_RE = re.compile(r"^\s*DMZ\s+([-+0-9.Ee]+)\s+DMZEFC", re.MULTILINE)
QUAD_BANNER_RE = re.compile(
    r"Quadrupole\s+moments\s+in\s+atomic\s+units", re.IGNORECASE
)
QUAD_COMP_RE = re.compile(
    r"^\s*(XX|YY|ZZ|XY|XZ|YZ)\s+([-+0-9.Ee]+)\s+([-+0-9.Ee]+)\s+([-+0-9.Ee]+)\s*$",
    re.MULTILINE,
)
HESS_EIGEN_BANNER_RE = re.compile(r"Final\s+eigenvalues", re.IGNORECASE)
HESS_EIGEN_VAL_RE = re.compile(
    r"^\s*([-+0-9.]+(?:[DdEe][-+]?[0-9]+)?)\s*$", re.MULTILINE
)

EMBED_BIN_CANDIDATES = [
    "test_nwchem_energy_gradient",
    "tests/test_nwchem_energy_gradient",
]


def split_command(command: str | None) -> list[str] | None:
    if not command:
        return None
    parts = shlex.split(command)
    return parts if parts else None


def format_command(command: list[str] | None) -> str | None:
    if command is None:
        return None
    return " ".join(shlex.quote(part) for part in command)


def embed_launch_command(
    embed_command: list[str] | None, bin_path: Path, params: Path
) -> list[str]:
    launch = embed_command if embed_command is not None else []
    return launch + [str(bin_path), str(params)]


def parse_energy(text: str) -> float | None:
    """Return the method total energy (Ha), not a prior SCF/guess line.

    Collects all matches from method-total patterns and returns the
    chronologically last one (final NWChem printout). Falls back to SCF /
    bare ``Total energy`` only when no method total is present.
    """
    candidates: list[tuple[int, float]] = []
    for rx in METHOD_ENERGY_RES:
        for m in rx.finditer(text):
            try:
                candidates.append((m.start(), float(m.group(1))))
            except ValueError:
                continue
    if candidates:
        candidates.sort(key=lambda item: item[0])
        return candidates[-1][1]
    for rx in FALLBACK_ENERGY_RES:
        for m in rx.finditer(text):
            try:
                candidates.append((m.start(), float(m.group(1))))
            except ValueError:
                continue
    if candidates:
        candidates.sort(key=lambda item: item[0])
        return candidates[-1][1]
    return None


def parse_gradient(text: str) -> list[float] | None:
    """Return flat 3*N gradient in Ha/Bohr if parsable, else None.

    Only searches after a recognized NWChem gradient banner so energy-only
    runs (which may mention "gradient" in prose) do not invent triples from
    unrelated coordinate tables.
    """
    m = GRAD_BANNER_RE.search(text)
    if not m:
        return None
    # Cap scan window to avoid later modules; prefer table lines (6 floats).
    search_text = text[m.start() : m.start() + 8000]
    grads: list[float] = []
    for line_m in GRAD_TABLE_LINE_RE.finditer(search_text):
        grads.extend(float(line_m.group(i)) for i in (1, 2, 3))
    if grads:
        return grads
    grads = []
    for line_m in GRAD_LINE_RE.finditer(search_text):
        grads.extend(float(line_m.group(i)) for i in (1, 2, 3))
    return grads if grads else None


def max_abs_delta(a: list[float], b: list[float]) -> float:
    if len(a) != len(b):
        return float("inf")
    return max(abs(x - y) for x, y in zip(a, b)) if a else 0.0


def parse_optimized_positions(text: str) -> list[float] | None:
    """Return flat 3*N optimized Cartesian positions (Angstrom).

    Uses the last ``Output coordinates in angstroms`` atom table (after
    driver convergence when present). Does not invent coords from prose.
    """
    banners = list(OUTPUT_COORDS_BANNER_RE.finditer(text))
    if not banners:
        return None
    # Prefer the table after the last banner (final optimized geometry).
    start = banners[-1].end()
    window = text[start : start + 2000]
    # Require the Tag/Charge header then only consecutive atom rows.
    header = re.search(
        r"No\.\s+Tag\s+Charge\s+X\s+Y\s+Z\s*\n\s*----[^\n]*\n",
        window,
        re.IGNORECASE,
    )
    if not header:
        return None
    pos: list[float] = []
    for line in window[header.end() :].splitlines():
        line_m = ATOM_XYZ_LINE_RE.match(line)
        if not line_m:
            if pos:
                break
            continue
        pos.extend(float(line_m.group(i)) for i in (1, 2, 3))
    return pos if pos else None


def parse_frequencies(text: str) -> list[float] | None:
    """Return frequency magnitudes (cm^-1) from the last Frequency line.

    Prefers unprojected ``Frequency`` (full 3N modes). Does not invent
    modes from thermochemistry prose. Stops at 3N values when N is known
    from a prior geometry table; otherwise takes the first numeric run
    on the Frequency line only (not mode eigenvector tables).
    """
    matches = list(FREQUENCY_LINE_RE.finditer(text))
    if not matches:
        matches = list(PFREQUENCY_LINE_RE.finditer(text))
    if not matches:
        return None
    # Frequency line is space-separated floats only on that line.
    line = matches[-1].group(0)
    # Strip label
    nums = re.findall(r"[-+0-9.Ee]+", line.split(None, 1)[-1] if line.strip() else "")
    # The banner line itself is "Frequency   v1 v2 ..." — take floats from group 1
    vals: list[float] = []
    for tok in matches[-1].group(1).split():
        try:
            vals.append(float(tok))
        except ValueError:
            break
    # Cap at reasonable 3N for small molecules if we over-read (should not)
    if len(vals) > 30:
        vals = vals[:30]
    return vals if vals else None


def _fortran_float(tok: str) -> float:
    return float(tok.replace("D", "E").replace("d", "e"))


def parse_dipole(text: str) -> list[float] | None:
    """Return DMX,DMY,DMZ in a.u. from the first Dipole Moment A.U. block."""
    # Prefer values under the A.U. section (before Debye).
    au = re.search(
        r"Dipole\s+Moment.*?Dipole\s+moment\s+([-+0-9.Ee]+)\s+A\.U\.(.*?)(?:Debye|-----)",
        text,
        re.IGNORECASE | re.DOTALL,
    )
    if not au:
        return None
    block = au.group(0)
    mx = DMX_RE.search(block)
    my = DMY_RE.search(block)
    mz = DMZ_RE.search(block)
    if not (mx and my and mz):
        return None
    return [float(mx.group(1)), float(my.group(1)), float(mz.group(1))]


def parse_quadrupole(text: str) -> list[float] | None:
    """Return xx,xy,xz,yy,yz,zz from Quadrupole moments in atomic units Total."""
    banners = list(QUAD_BANNER_RE.finditer(text))
    if not banners:
        return None
    window = text[banners[0].end() : banners[0].end() + 2500]
    comps: dict[str, float] = {}
    for m in QUAD_COMP_RE.finditer(window):
        # Total column is group 4
        comps[m.group(1).upper()] = float(m.group(4))
        if len(comps) >= 6:
            break
    order = ["XX", "XY", "XZ", "YY", "YZ", "ZZ"]
    if not all(k in comps for k in order):
        return None
    return [comps[k] for k in order]


def parse_hessian_eigenvalues(text: str) -> list[float] | None:
    """Return Final eigenvalues from analytic Hessian output (D-float lines)."""
    m = HESS_EIGEN_BANNER_RE.search(text)
    if not m:
        return None
    window = text[m.end() : m.end() + 4000]
    vals: list[float] = []
    for line in window.splitlines():
        line = line.strip()
        if not line:
            if vals:
                break
            continue
        if HESS_EIGEN_VAL_RE.match(line):
            try:
                vals.append(_fortran_float(line))
            except ValueError:
                break
        elif vals:
            break
    return vals if vals else None


def parse_hessian_file(work: Path) -> list[float] | None:
    """Parse NWChem .hess lower-triangle file into full row-major n*n matrix.

    For H2 (n=6) the file has 21 lower-triangular entries.
    """
    candidates = list(work.glob("*.hess"))
    if not candidates:
        return None
    path = candidates[0]
    toks: list[float] = []
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        for tok in line.split():
            try:
                toks.append(_fortran_float(tok))
            except ValueError:
                continue
    if not toks:
        return None
    # Infer n from n(n+1)/2 == len(toks)
    n = int(round((-1 + (1 + 8 * len(toks)) ** 0.5) / 2))
    if n * (n + 1) // 2 != len(toks):
        # Treat as full matrix already
        return toks
    mat = [0.0] * (n * n)
    k = 0
    for i in range(n):
        for j in range(i + 1):
            mat[i * n + j] = toks[k]
            mat[j * n + i] = toks[k]
            k += 1
    return mat


def run_nwchem_cli(
    nw_path: Path, work: Path, nwchem_command: list[str]
) -> tuple[
    int,
    str,
    float | None,
    list[float] | None,
    list[float] | None,
    list[float] | None,
    dict,
]:
    src = (ROOT / nw_path).resolve()
    extras: dict = {}
    if not src.is_file():
        return 2, f"missing input {src}", None, None, None, None, extras
    work.mkdir(parents=True, exist_ok=True)
    local_nw = work / src.name
    local_nw.write_text(src.read_text(encoding="utf-8"), encoding="utf-8")
    log_path = work / (src.stem + ".out")
    proc = subprocess.run(
        nwchem_command + [str(local_nw)],
        cwd=str(work),
        capture_output=True,
        text=True,
        timeout=600,
        check=False,
    )
    combined = (proc.stdout or "") + "\n" + (proc.stderr or "")
    log_path.write_text(combined, encoding="utf-8")
    energy = parse_energy(combined)
    grad = parse_gradient(combined)
    positions = parse_optimized_positions(combined)
    frequencies = parse_frequencies(combined)
    extras["dipole_au"] = parse_dipole(combined)
    extras["quadrupole_au"] = parse_quadrupole(combined)
    extras["hessian_eigenvalues"] = parse_hessian_eigenvalues(combined)
    extras["hessian_ha_bohr2"] = parse_hessian_file(work)
    return (
        proc.returncode,
        combined,
        energy,
        grad,
        positions,
        frequencies,
        extras,
    )


def find_build_exe(build_dir: Path, name: str) -> Path | None:
    p = build_dir / name
    if p.is_file() and os.access(p, os.X_OK):
        return p
    for cand in build_dir.rglob(name):
        if cand.is_file() and os.access(cand, os.X_OK):
            return cand
    return None


def find_params_for_meson_test(build_dir: Path, meson_test: str) -> Path | None:
    """Map embed_meson_test name to params blob under the Meson build dir."""
    # Defaults / SCF energy-gradient
    if meson_test in (
        "nwchem-energy-gradient",
        "nwchem-rgpot-smoke",
        "nwchem-optimize-scf",
        "nwchem-frequencies-scf",
    ):
        for name in ("nwchem_params.bin", "nwchem_params_h2_scf.bin"):
            p = build_dir / name
            if p.is_file():
                return p
        for p in build_dir.rglob("nwchem_params_h2_scf.bin"):
            if p.is_file():
                return p
        for p in build_dir.rglob("nwchem_params.bin"):
            if p.is_file():
                return p
        return None

    label = None
    if meson_test.startswith("nwchem-postscf-energy-"):
        label = meson_test[len("nwchem-postscf-energy-") :]
    elif meson_test.startswith("nwchem-energy-forces-"):
        label = meson_test[len("nwchem-energy-forces-") :]
    elif meson_test.startswith("nwchem-dipole-"):
        label = meson_test[len("nwchem-dipole-") :]
    elif meson_test.startswith("nwchem-polarizability-"):
        label = meson_test[len("nwchem-polarizability-") :]
    elif meson_test.startswith("nwchem-quadrupole-"):
        label = meson_test[len("nwchem-quadrupole-") :]
    elif meson_test.startswith("nwchem-hessian-"):
        label = meson_test[len("nwchem-hessian-") :]
    if not label:
        return None
    # Meson output: nwchem_params_h2_<label with - -> _>.bin
    stem = "nwchem_params_h2_" + label.replace("-", "_") + ".bin"
    p = build_dir / stem
    if p.is_file():
        return p
    for cand in build_dir.rglob(stem):
        if cand.is_file():
            return cand
    return None


def embed_launch_for_task(
    embed_command: list[str] | None,
    bin_path: Path,
    meson_test: str,
    params: Path,
) -> list[str]:
    """Build argv for the embed test binary matching embed_meson_test."""
    launch = list(embed_command) if embed_command else []
    launch.append(str(bin_path))
    if meson_test.startswith("nwchem-postscf-energy-"):
        label = meson_test[len("nwchem-postscf-energy-") :]
        launch.extend([label, str(params)])
    elif meson_test.startswith("nwchem-energy-forces-"):
        label = meson_test[len("nwchem-energy-forces-") :]
        launch.extend([label, str(params)])
    elif meson_test == "nwchem-optimize-scf":
        launch.extend(["optimize", str(params)])
    elif meson_test == "nwchem-frequencies-scf":
        launch.extend(["frequencies", str(params)])
    elif meson_test.startswith("nwchem-dipole-"):
        launch.extend(["dipole", str(params)])
    elif meson_test.startswith("nwchem-polarizability-"):
        launch.extend(["polarizability", str(params)])
    elif meson_test.startswith("nwchem-quadrupole-"):
        launch.extend(["quadrupole", str(params)])
    elif meson_test.startswith("nwchem-hessian-"):
        launch.extend(["hessian", str(params)])
    else:
        # test_nwchem_energy_gradient PARAMS_BIN
        launch.append(str(params))
    return launch


def run_embed_compare_task(
    build_dir: Path,
    work: Path,
    embed_command: list[str] | None,
    meson_test: str,
) -> tuple[str, dict | None, str]:
    """Run per-task embed binary; return (status, payload_or_none, detail)."""
    build_dir = Path(build_dir).expanduser().resolve()
    # Prefer single-case drivers that write NWCHEMC_COMPARE_JSON without
    # multi-test suites that can abort after the H2 case.
    effective_test = meson_test
    if meson_test == "nwchem-energy-gradient":
        # Use forces driver with SCF params (one cmocka case, writes JSON).
        effective_test = "nwchem-energy-forces-scf"

    if effective_test.startswith("nwchem-postscf-energy-"):
        bin_name = "test_nwchem_postscf_energy"
    elif effective_test.startswith("nwchem-energy-forces-"):
        bin_name = "test_nwchem_energy_forces"
    elif effective_test in ("nwchem-optimize-scf", "nwchem-frequencies-scf"):
        bin_name = "test_nwchem_optimize_freq"
    elif (
        effective_test.startswith("nwchem-dipole-")
        or effective_test.startswith("nwchem-polarizability-")
        or effective_test.startswith("nwchem-quadrupole-")
        or effective_test.startswith("nwchem-hessian-")
    ):
        bin_name = "test_nwchem_primary_props"
    else:
        bin_name = "test_nwchem_energy_gradient"

    bin_path = find_build_exe(build_dir, bin_name)
    if not bin_path:
        return "unavailable", None, f"no {bin_name} under {build_dir}"
    bin_path = bin_path.resolve()
    params = find_params_for_meson_test(build_dir, effective_test)
    if not params:
        return "unavailable", None, f"params bin for {effective_test} not found"
    params = params.resolve()
    work = Path(work).expanduser().resolve()
    work.mkdir(parents=True, exist_ok=True)
    json_path = work / "embed_compare.json"
    env = os.environ.copy()
    env["NWCHEMC_COMPARE_JSON"] = str(json_path)
    scratch = work / "embed_scratch"
    perm = work / "embed_permanent"
    scratch.mkdir(parents=True, exist_ok=True)
    perm.mkdir(parents=True, exist_ok=True)
    env.setdefault("NWCHEMC_TEST_SCRATCH_DIR", str(scratch))
    env.setdefault("NWCHEMC_TEST_PERMANENT_DIR", str(perm))
    argv = embed_launch_for_task(
        embed_command, bin_path, effective_test, params
    )
    proc = subprocess.run(
        argv,
        cwd=str(work),
        capture_output=True,
        text=True,
        timeout=600,
        check=False,
        env=env,
    )
    log_path = work / "embed_run.log"
    log_path.write_text(
        (proc.stdout or "") + "\n--- stderr ---\n" + (proc.stderr or ""),
        encoding="utf-8",
    )
    if proc.returncode != 0:
        return "fail", None, f"embed rc={proc.returncode} (see {log_path.name})"
    if not json_path.is_file():
        combined = (proc.stdout or "") + (proc.stderr or "")
        if "stub" in combined.lower() or "not available" in combined.lower():
            return "stub-only", None, "embed binary ran but is stub/unavailable"
        return (
            "unavailable",
            None,
            f"no {json_path.name} written (need NWCHEMC_COMPARE_JSON support + real embed)",
        )
    try:
        payload = json.loads(json_path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        return "fail", None, f"bad embed JSON: {exc}"
    return "ok", payload, str(json_path)


def run_embed_compare(
    build_dir: Path, work: Path, embed_command: list[str] | None
) -> tuple[str, dict | None, str]:
    """Backward-compat: SCF energy-gradient embed once."""
    return run_embed_compare_task(
        build_dir, work, embed_command, "nwchem-energy-gradient"
    )


def compare_task(
    task: dict,
    cli_energy: float | None,
    cli_grad: list[float] | None,
    embed: dict | None,
    tol_e: float,
    tol_g: float,
    cli_positions: list[float] | None = None,
    cli_frequencies: list[float] | None = None,
    cli_extras: dict | None = None,
    tol_pos: float = 1.0e-3,
    tol_freq: float = 50.0,
    tol_dipole: float = 1.0e-4,
    tol_quad: float = 1.0e-3,
    tol_hess: float = 1.0e-3,
) -> tuple[bool, list[str]]:
    """Return (pass, notes). Energy always required from CLI; embed extras optional."""
    notes: list[str] = []
    ok = True
    quantities = set(task.get("quantities", ["energy"]))
    extras = cli_extras or {}

    if cli_energy is None:
        return False, ["cli energy missing"]

    # Forces are shipped as -gradient; CLI prints gradients under gradient banners.
    want_grad = "gradient" in quantities or "forces" in quantities
    want_pos = "optimized_positions" in quantities
    want_freq = "frequencies" in quantities
    want_dipole = "dipole" in quantities
    want_quad = "quadrupole" in quantities
    want_hess = "hessian" in quantities
    multi_qty = (
        want_grad or want_pos or want_freq or want_dipole or want_quad or want_hess
    )

    if embed and embed.get("energy_ha") is not None:
        de = abs(float(embed["energy_ha"]) - cli_energy)
        # Gradient/forces/opt/freq jobs may print a slightly different total
        # than a pure energy embed; allow a looser energy window then.
        e_tol = tol_e if not multi_qty else max(tol_e, 1.0e-3)
        notes.append(f"delta_energy_ha={de:.3e} (tol={e_tol:.1e})")
        if de > e_tol:
            ok = False
            notes.append("FAIL energy delta")
    elif embed is not None:
        notes.append("embed payload missing energy_ha")

    if want_grad:
        eg = embed.get("gradient_ha_bohr") if embed else None
        if cli_grad is not None and eg is not None:
            dg = max_abs_delta(cli_grad, [float(x) for x in eg])
            notes.append(f"delta_grad_max_abs={dg:.3e} (tol={tol_g:.1e})")
            if dg > tol_g:
                ok = False
                notes.append("FAIL gradient delta")
        elif cli_grad is None:
            notes.append("cli gradient not parsed (CLI leg energy-only still valid)")
        elif eg is None and embed is not None:
            notes.append("embed gradient missing")

    if want_pos:
        ep = embed.get("optimized_positions_ang") if embed else None
        if cli_positions is not None and ep is not None:
            # Allow rigid translation/reflection of H2: compare bond length and
            # sorted |z| components for diatomic along z; fall back to max abs.
            dp = max_abs_delta(cli_positions, [float(x) for x in ep])
            # For H2 optimize, positions may differ by overall sign flip on z.
            ep_f = [float(x) for x in ep]
            if len(cli_positions) == len(ep_f) == 6:
                # Try sign flip on all coords (reflection through origin).
                dp_flip = max_abs_delta(cli_positions, [-x for x in ep_f])
                dp = min(dp, dp_flip)
            notes.append(f"delta_pos_max_abs_ang={dp:.3e} (tol={tol_pos:.1e})")
            if dp > tol_pos:
                ok = False
                notes.append("FAIL optimized positions delta")
        elif cli_positions is None:
            notes.append("cli optimized positions not parsed")
            ok = False
        elif ep is None and embed is not None:
            notes.append("embed optimized_positions_ang missing")
            ok = False

    if want_freq:
        ef = embed.get("frequencies_cm1") if embed else None
        if cli_frequencies is not None and ef is not None:
            # Compare sorted absolute magnitudes (mode order may differ).
            cli_s = sorted(abs(float(x)) for x in cli_frequencies)
            emb_s = sorted(abs(float(x)) for x in ef)
            # Pad shorter list with zeros if lengths differ (e.g. projected).
            n = max(len(cli_s), len(emb_s))
            cli_s = cli_s + [0.0] * (n - len(cli_s))
            emb_s = emb_s + [0.0] * (n - len(emb_s))
            df = max_abs_delta(cli_s, emb_s)
            notes.append(f"delta_freq_max_abs_cm1={df:.3e} (tol={tol_freq:.1e})")
            if df > tol_freq:
                ok = False
                notes.append("FAIL frequency magnitudes delta")
        elif cli_frequencies is None:
            notes.append("cli frequencies not parsed")
            ok = False
        elif ef is None and embed is not None:
            notes.append("embed frequencies_cm1 missing")
            ok = False

    if want_dipole:
        cd = extras.get("dipole_au")
        ed = embed.get("dipole_au") if embed else None
        if cd is not None and ed is not None:
            dd = max_abs_delta(cd, [float(x) for x in ed])
            notes.append(f"delta_dipole_max_abs_au={dd:.3e} (tol={tol_dipole:.1e})")
            if dd > tol_dipole:
                ok = False
                notes.append("FAIL dipole delta")
        elif cd is None:
            notes.append("cli dipole not parsed")
            ok = False
        elif ed is None and embed is not None:
            notes.append("embed dipole_au missing")
            ok = False

    if want_quad:
        cq = extras.get("quadrupole_au")
        eq = embed.get("quadrupole_au") if embed else None
        if cq is not None and eq is not None:
            dq = max_abs_delta(cq, [float(x) for x in eq])
            notes.append(f"delta_quad_max_abs_au={dq:.3e} (tol={tol_quad:.1e})")
            if dq > tol_quad:
                ok = False
                notes.append("FAIL quadrupole delta")
        elif cq is None:
            notes.append("cli quadrupole not parsed")
            ok = False
        elif eq is None and embed is not None:
            notes.append("embed quadrupole_au missing")
            ok = False

    if want_hess:
        ch = extras.get("hessian_ha_bohr2")
        eh = embed.get("hessian_ha_bohr2") if embed else None
        if ch is not None and eh is not None:
            # Compare sorted |elements| to tolerate storage order differences.
            cs = sorted(abs(float(x)) for x in ch)
            es = sorted(abs(float(x)) for x in eh)
            n = max(len(cs), len(es))
            cs = cs + [0.0] * (n - len(cs))
            es = es + [0.0] * (n - len(es))
            dh = max_abs_delta(cs, es)
            notes.append(f"delta_hess_max_abs={dh:.3e} (tol={tol_hess:.1e})")
            if dh > tol_hess:
                ok = False
                notes.append("FAIL hessian delta")
        elif ch is None:
            notes.append("cli hessian (.hess / eigenvalues) not parsed")
            ok = False
        elif eh is None and embed is not None:
            notes.append("embed hessian_ha_bohr2 missing")
            ok = False

    return ok, notes


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument(
        "--out-dir",
        type=Path,
        default=ROOT / "build" / "nwchem-cli-ref",
        help="Directory for CLI outputs and JSON report",
    )
    ap.add_argument(
        "--embed-build",
        type=Path,
        default=None,
        help="Meson build dir with real embed (e.g. build-nwchem); enables embed leg",
    )
    ap.add_argument(
        "--require-cli",
        action="store_true",
        help="Exit 2 if nwchem CLI is missing",
    )
    ap.add_argument(
        "--require-embed",
        action="store_true",
        help="Exit 2/3 if embed leg cannot run (implies --embed-build must work)",
    )
    ap.add_argument(
        "--nwchem-command",
        default=os.environ.get("NWCHEM_COMMAND"),
        help=(
            "Command used to launch NWChem; defaults to `nwchem` on PATH. "
            "Use quotes for launchers, for example: "
            "`mpirun -np 2 /path/to/nwchem`."
        ),
    )
    ap.add_argument(
        "--embed-command",
        default=os.environ.get("NWCHEMC_EMBED_COMMAND"),
        help=(
            "Launcher prefix used for the embed test binary. "
            "Use quotes for launchers, for example: `mpirun -np 2`."
        ),
    )
    args = ap.parse_args()

    if args.require_embed and not args.embed_build:
        print("--require-embed needs --embed-build", file=sys.stderr)
        return 2

    matrix = json.loads(MATRIX.read_text(encoding="utf-8"))
    tol_e = float(matrix["tolerances"]["energy_ha_abs"])
    tol_g = float(matrix["tolerances"]["gradient_ha_bohr_abs"])
    tol_pos = float(matrix["tolerances"].get("position_ang_abs", 1.0e-3))
    tol_freq = float(matrix["tolerances"].get("frequency_cm1_abs", 50.0))
    tol_dipole = float(matrix["tolerances"].get("dipole_au_abs", 1.0e-4))
    tol_quad = float(matrix["tolerances"].get("quadrupole_au_abs", 1.0e-3))
    tol_hess = float(matrix["tolerances"].get("hessian_ha_bohr2_abs", 1.0e-3))
    out_dir: Path = args.out_dir
    out_dir.mkdir(parents=True, exist_ok=True)

    nwchem_command = split_command(args.nwchem_command)
    if nwchem_command is None:
        nwchem_path = shutil.which("nwchem")
        nwchem_command = [nwchem_path] if nwchem_path else None
    else:
        exe = nwchem_command[0]
        if not Path(exe).is_file() and shutil.which(exe) is None:
            nwchem_command = None

    want_embed = args.embed_build is not None
    embed_command = split_command(args.embed_command)
    if embed_command is not None:
        exe = embed_command[0]
        if not Path(exe).is_file() and shutil.which(exe) is None:
            embed_command = None
    report: dict = {
        "matrix": str(MATRIX.relative_to(ROOT)),
        "nwchem": format_command(nwchem_command),
        "embed_build": str(args.embed_build) if args.embed_build else None,
        "embed_command": format_command(embed_command),
        "tolerances": matrix["tolerances"],
        "tasks": [],
    }

    if nwchem_command is None:
        msg = "SKIP-INCOMPLETE: nwchem CLI command not available"
        print(msg, file=sys.stderr)
        report["status"] = "skipped-incomplete-cli"
        (out_dir / "nwchem_compare_report.json").write_text(
            json.dumps(report, indent=2), encoding="utf-8"
        )
        (out_dir / "nwchem_compare_report.txt").write_text(msg + "\n", encoding="utf-8")
        return 2 if args.require_cli else 0

    all_ok = True
    incomplete_embed = False
    lines = [
        f"nwchem_cli={report['nwchem']}",
        f"tol_energy_ha={tol_e}",
        f"tol_gradient_ha_bohr={tol_g}",
        f"tol_position_ang={tol_pos}",
        f"tol_frequency_cm1={tol_freq}",
        f"embed_build={report['embed_build'] or '(none — CLI only)'}",
        f"embed_command={report['embed_command'] or '(direct)'}",
    ]
    if want_embed:
        lines.append("embed_mode=per-task (embed_meson_test → binary+params)")

    for task in matrix["tasks"]:
        tid = task["id"]
        work = out_dir / tid
        rc, _text, energy, grad, positions, frequencies, extras = run_nwchem_cli(
            Path(task["nw_input"]), work, nwchem_command
        )
        entry: dict = {
            "id": tid,
            "quantities": task.get("quantities", ["energy"]),
            "cli_rc": rc,
            "cli_energy_ha": energy,
            "cli_gradient_ha_bohr": grad,
            "cli_gradient_n": len(grad) if grad else 0,
            "cli_optimized_positions_ang": positions,
            "cli_positions_n": len(positions) if positions else 0,
            "cli_frequencies_cm1": frequencies,
            "cli_frequencies_n": len(frequencies) if frequencies else 0,
            "cli_dipole_au": extras.get("dipole_au"),
            "cli_quadrupole_au": extras.get("quadrupole_au"),
            "cli_hessian_n": (
                len(extras["hessian_ha_bohr2"])
                if extras.get("hessian_ha_bohr2")
                else 0
            ),
            "embed_status": "not-requested",
            "embed_energy_ha": None,
            "embed_gradient_ha_bohr": None,
            "pass": False,
        }

        if rc != 0 or energy is None:
            all_ok = False
            entry["error"] = f"cli failed rc={rc} energy={energy}"
            lines.append(f"{tid}: FAIL cli ({entry['error']})")
            report["tasks"].append(entry)
            continue

        lines.append(f"{tid}: CLI energy={energy:.12f} Ha")
        if grad is not None:
            lines.append(f"{tid}: CLI gradient n={len(grad)} parsed")
        if positions is not None:
            lines.append(
                f"{tid}: CLI optimized positions n={len(positions)} parsed"
            )
        if frequencies is not None:
            lines.append(
                f"{tid}: CLI frequencies n={len(frequencies)} parsed"
            )

        task_wants_embed = bool(task.get("embed_compare", True))
        meson_test = task.get("embed_meson_test") or "nwchem-energy-gradient"
        if want_embed and task_wants_embed:
            embed_status, embed_payload, embed_detail = run_embed_compare_task(
                args.embed_build,  # type: ignore[arg-type]
                work / "_embed",
                embed_command,
                meson_test,
            )
            entry["embed_status"] = embed_status
            entry["embed_detail"] = embed_detail
            entry["embed_meson_test"] = meson_test
            if embed_status == "ok" and embed_payload:
                entry["embed_energy_ha"] = embed_payload.get("energy_ha")
                entry["embed_gradient_ha_bohr"] = embed_payload.get(
                    "gradient_ha_bohr"
                )
                if embed_payload.get("forces_ha_bohr") is not None:
                    entry["embed_forces_ha_bohr"] = embed_payload.get(
                        "forces_ha_bohr"
                    )
                if embed_payload.get("optimized_positions_ang") is not None:
                    entry["embed_optimized_positions_ang"] = embed_payload.get(
                        "optimized_positions_ang"
                    )
                if embed_payload.get("frequencies_cm1") is not None:
                    entry["embed_frequencies_cm1"] = embed_payload.get(
                        "frequencies_cm1"
                    )
                for k in (
                    "dipole_au",
                    "quadrupole_au",
                    "polarizability_au",
                    "hessian_ha_bohr2",
                ):
                    if embed_payload.get(k) is not None:
                        entry[f"embed_{k}"] = embed_payload.get(k)
                cmp_ok, notes = compare_task(
                    task,
                    energy,
                    grad,
                    embed_payload,
                    tol_e,
                    tol_g,
                    cli_positions=positions,
                    cli_frequencies=frequencies,
                    cli_extras=extras,
                    tol_pos=tol_pos,
                    tol_freq=tol_freq,
                    tol_dipole=tol_dipole,
                    tol_quad=tol_quad,
                    tol_hess=tol_hess,
                )
                entry["compare_notes"] = notes
                entry["pass"] = cmp_ok
                if not cmp_ok:
                    all_ok = False
                for n in notes:
                    lines.append(f"{tid}: {n}")
                lines.append(
                    f"{tid}: {'PASS' if cmp_ok else 'FAIL'} embed-vs-CLI "
                    f"({meson_test})"
                )
            elif embed_status in ("unavailable", "stub-only"):
                entry["pass"] = False
                entry["compare_notes"] = [
                    f"embed leg incomplete: {embed_status} ({embed_detail})"
                ]
                lines.append(
                    f"{tid}: incomplete-compare (embed {embed_status}: {embed_detail})"
                )
                incomplete_embed = True
            else:
                entry["pass"] = False
                entry["compare_notes"] = [
                    f"embed fail: {embed_status} ({embed_detail})"
                ]
                lines.append(f"{tid}: FAIL embed ({embed_detail})")
                all_ok = False
        elif want_embed and not task_wants_embed:
            # CLI proves the .nw path; embed proof is Meson-only for this op.
            entry["pass"] = True
            entry["embed_status"] = "skipped-task"
            entry["compare_notes"] = [
                "cli-only task (embed_compare=false; Meson covers embed)"
            ]
            lines.append(f"{tid}: PASS cli-only (embed_compare=false)")
        else:
            # CLI-only mode: task passes if CLI energy ok
            entry["pass"] = True
            entry["compare_notes"] = ["cli-only (no --embed-build)"]
            lines.append(f"{tid}: PASS cli-only")

        report["tasks"].append(entry)

    if incomplete_embed and want_embed:
        report["status"] = "incomplete-compare"
    elif all_ok:
        report["status"] = "ok"
    else:
        report["status"] = "fail"

    report_txt = "\n".join(lines) + "\n"
    (out_dir / "nwchem_compare_report.json").write_text(
        json.dumps(report, indent=2), encoding="utf-8"
    )
    (out_dir / "nwchem_compare_report.txt").write_text(report_txt, encoding="utf-8")
    print(report_txt)

    if report["status"] == "fail":
        return 1
    if report["status"] == "incomplete-compare":
        return 3 if args.require_embed else 0
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
