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
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MATRIX = ROOT / "tests/integration/task_matrix.json"

ENERGY_RE = re.compile(
    r"Total\s+SCF\s+energy\s*=\s*([-+0-9.Ee+]+)", re.IGNORECASE
)
ENERGY_RE_ALT = [
    re.compile(r"Total\s+DFT\s+energy\s*=\s*([-+0-9.Ee+]+)", re.IGNORECASE),
    re.compile(r"^\s*Total\s+energy\s*=\s*([-+0-9.Ee+]+)", re.IGNORECASE | re.M),
]

# NWChem prints gradients under several banners; capture triples of floats.
GRAD_BLOCK_RE = re.compile(
    r"(?:DFT\s+ENERGY\s+GRADIENTS|ENERGY\s+GRADIENTS|Nuclear\s+Gradient)"
    r"[\s\S]{0,4000}?"
    r"(?P<body>(?:^\s*(?:[A-Za-z]+\s+)?\d+\s+[-+0-9.Ee+]+\s+[-+0-9.Ee+]+\s+[-+0-9.Ee+]+.*$\n?)+)",
    re.IGNORECASE | re.MULTILINE,
)
GRAD_LINE_RE = re.compile(
    r"^\s*(?:[A-Za-z]+\s+)?\d+\s+([-+0-9.Ee+]+)\s+([-+0-9.Ee+]+)\s+([-+0-9.Ee+]+)",
    re.MULTILINE,
)
# Fallback: lines like "1     H     0.0000     0.0000    -0.0123" in gradient sections
GRAD_LINE_ALT_RE = re.compile(
    r"^\s*\d+\s+[A-Za-z]+\s+([-+0-9.Ee+]+)\s+([-+0-9.Ee+]+)\s+([-+0-9.Ee+]+)",
    re.MULTILINE,
)

EMBED_BIN_CANDIDATES = [
    "test_nwchem_energy_gradient",
    "tests/test_nwchem_energy_gradient",
]


def parse_energy(text: str) -> float | None:
    m = ENERGY_RE.search(text)
    if m:
        return float(m.group(1))
    for rx in ENERGY_RE_ALT:
        m = rx.search(text)
        if m:
            return float(m.group(1))
    return None


def parse_gradient(text: str) -> list[float] | None:
    """Return flat 3*N gradient in Ha/Bohr if parsable, else None."""
    grads: list[float] = []
    m = GRAD_BLOCK_RE.search(text)
    search_text = m.group("body") if m else text
    for rx in (GRAD_LINE_RE, GRAD_LINE_ALT_RE):
        grads = []
        for line_m in rx.finditer(search_text):
            grads.extend(float(line_m.group(i)) for i in (1, 2, 3))
        if grads:
            return grads
    # Last resort: scan whole file for atom gradient triples after "gradient"
    if "gradient" not in text.lower():
        return None
    for line_m in GRAD_LINE_ALT_RE.finditer(text):
        grads.extend(float(line_m.group(i)) for i in (1, 2, 3))
    return grads if grads else None


def max_abs_delta(a: list[float], b: list[float]) -> float:
    if len(a) != len(b):
        return float("inf")
    return max(abs(x - y) for x, y in zip(a, b)) if a else 0.0


def run_nwchem_cli(nw_path: Path, work: Path) -> tuple[int, str, float | None, list[float] | None]:
    nwchem = shutil.which("nwchem")
    if not nwchem:
        return 127, "nwchem not on PATH (enable pixi feature nwchem-runtime)", None, None
    src = (ROOT / nw_path).resolve()
    if not src.is_file():
        return 2, f"missing input {src}", None, None
    work.mkdir(parents=True, exist_ok=True)
    local_nw = work / src.name
    local_nw.write_text(src.read_text(encoding="utf-8"), encoding="utf-8")
    log_path = work / (src.stem + ".out")
    proc = subprocess.run(
        [nwchem, str(local_nw)],
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
    return proc.returncode, combined, energy, grad


def find_embed_binary(build_dir: Path) -> Path | None:
    for name in EMBED_BIN_CANDIDATES:
        p = build_dir / name
        if p.is_file() and os.access(p, os.X_OK):
            return p
    # meson places test executables at build dir root on most setups
    for p in build_dir.rglob("test_nwchem_energy_gradient"):
        if p.is_file() and os.access(p, os.X_OK):
            return p
    return None


def find_params_bin(build_dir: Path) -> Path | None:
    for name in (
        "nwchem_parser_params.bin",
        "tests/nwchem_parser_params.bin",
        "schema/nwchem_parser_params.bin",
    ):
        p = build_dir / name
        if p.is_file():
            return p
    for p in build_dir.rglob("nwchem_parser_params.bin"):
        if p.is_file():
            return p
    # source-tree fixtures used by meson tests
    for p in (
        ROOT / "tests" / "fixtures" / "nwchem_parser_params.bin",
        ROOT / "build" / "nwchem_parser_params.bin",
    ):
        if p.is_file():
            return p
    return None


def run_embed_compare(
    build_dir: Path, work: Path
) -> tuple[str, dict | None, str]:
    """Run embed test binary; return (status, payload_or_none, detail)."""
    bin_path = find_embed_binary(build_dir)
    if not bin_path:
        return "unavailable", None, f"no test_nwchem_energy_gradient under {build_dir}"
    params = find_params_bin(build_dir)
    if not params:
        return "unavailable", None, "nwchem_parser_params.bin not found"
    json_path = work / "embed_compare.json"
    env = os.environ.copy()
    env["NWCHEMC_COMPARE_JSON"] = str(json_path)
    # scratch/permanent dirs for embed init
    scratch = work / "embed_scratch"
    perm = work / "embed_permanent"
    scratch.mkdir(parents=True, exist_ok=True)
    perm.mkdir(parents=True, exist_ok=True)
    env.setdefault("NWCHEMC_TEST_SCRATCH_DIR", str(scratch))
    env.setdefault("NWCHEMC_TEST_PERMANENT_DIR", str(perm))
    proc = subprocess.run(
        [str(bin_path), str(params)],
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
        # binary may be stub build without embed SDK
        combined = (proc.stdout or "") + (proc.stderr or "")
        if "stub" in combined.lower() or "not available" in combined.lower():
            return "stub-only", None, "embed binary ran but is stub/unavailable"
        return "unavailable", None, f"no {json_path.name} written (need NWCHEMC_COMPARE_JSON support + real embed)"
    try:
        payload = json.loads(json_path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        return "fail", None, f"bad embed JSON: {exc}"
    return "ok", payload, str(json_path)


def compare_task(
    task: dict,
    cli_energy: float | None,
    cli_grad: list[float] | None,
    embed: dict | None,
    tol_e: float,
    tol_g: float,
) -> tuple[bool, list[str]]:
    """Return (pass, notes). Energy always required from CLI; embed/grad optional per task."""
    notes: list[str] = []
    ok = True
    quantities = set(task.get("quantities", ["energy"]))

    if cli_energy is None:
        return False, ["cli energy missing"]

    if embed and embed.get("energy_ha") is not None:
        de = abs(float(embed["energy_ha"]) - cli_energy)
        notes.append(f"delta_energy_ha={de:.3e} (tol={tol_e:.1e})")
        if de > tol_e:
            ok = False
            notes.append("FAIL energy delta")
    elif embed is not None:
        notes.append("embed payload missing energy_ha")

    if "gradient" in quantities:
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
    args = ap.parse_args()

    if args.require_embed and not args.embed_build:
        print("--require-embed needs --embed-build", file=sys.stderr)
        return 2

    matrix = json.loads(MATRIX.read_text(encoding="utf-8"))
    tol_e = float(matrix["tolerances"]["energy_ha_abs"])
    tol_g = float(matrix["tolerances"]["gradient_ha_bohr_abs"])
    out_dir: Path = args.out_dir
    out_dir.mkdir(parents=True, exist_ok=True)

    want_embed = args.embed_build is not None
    report: dict = {
        "matrix": str(MATRIX.relative_to(ROOT)),
        "nwchem": shutil.which("nwchem"),
        "embed_build": str(args.embed_build) if args.embed_build else None,
        "tolerances": matrix["tolerances"],
        "tasks": [],
    }

    if not report["nwchem"]:
        msg = "SKIP-INCOMPLETE: nwchem CLI not installed (pixi run -e nwchem-runtime ...)"
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
        f"embed_build={report['embed_build'] or '(none — CLI only)'}",
    ]

    # Run embed once per build (same H2 geometry in all tasks); cache payload.
    embed_payload: dict | None = None
    embed_status = "not-requested"
    embed_detail = ""
    if want_embed:
        embed_work = out_dir / "_embed"
        embed_status, embed_payload, embed_detail = run_embed_compare(
            args.embed_build, embed_work  # type: ignore[arg-type]
        )
        lines.append(f"embed_status={embed_status} ({embed_detail})")
        if embed_status in ("unavailable", "stub-only"):
            incomplete_embed = True
        elif embed_status == "fail":
            all_ok = False

    for task in matrix["tasks"]:
        tid = task["id"]
        work = out_dir / tid
        rc, _text, energy, grad = run_nwchem_cli(Path(task["nw_input"]), work)
        entry: dict = {
            "id": tid,
            "quantities": task.get("quantities", ["energy"]),
            "cli_rc": rc,
            "cli_energy_ha": energy,
            "cli_gradient_ha_bohr": grad,
            "cli_gradient_n": len(grad) if grad else 0,
            "embed_status": embed_status,
            "embed_energy_ha": embed_payload.get("energy_ha") if embed_payload else None,
            "embed_gradient_ha_bohr": (
                embed_payload.get("gradient_ha_bohr") if embed_payload else None
            ),
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

        if want_embed and embed_status == "ok" and embed_payload:
            entry["embed_energy_ha"] = embed_payload.get("energy_ha")
            entry["embed_gradient_ha_bohr"] = embed_payload.get("gradient_ha_bohr")
            cmp_ok, notes = compare_task(
                task, energy, grad, embed_payload, tol_e, tol_g
            )
            entry["compare_notes"] = notes
            entry["pass"] = cmp_ok
            if not cmp_ok:
                all_ok = False
            for n in notes:
                lines.append(f"{tid}: {n}")
            lines.append(f"{tid}: {'PASS' if cmp_ok else 'FAIL'} embed-vs-CLI")
        elif want_embed:
            entry["pass"] = False
            entry["compare_notes"] = [f"embed leg incomplete: {embed_status}"]
            lines.append(f"{tid}: incomplete-compare (embed {embed_status})")
            incomplete_embed = True
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
