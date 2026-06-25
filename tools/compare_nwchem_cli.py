#!/usr/bin/env python3
"""Run pixi/conda-forge nwchem CLI on integration inputs; optionally compare embed.

Embed leg requires a configured build-nwchem with nwchem-* meson tests.
CLI leg is always attempted when `nwchem` is on PATH (pixi feature nwchem-runtime).
"""
from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MATRIX = ROOT / "tests/integration/task_matrix.json"

ENERGY_RE = re.compile(
    r"Total\s+SCF\s+energy\s*=\s*([-+0-9.Ee+]+)", re.IGNORECASE
)
# Fallback patterns used by some NWChem builds / print formats.
ENERGY_RE_ALT = [
    re.compile(r"Total\s+DFT\s+energy\s*=\s*([-+0-9.Ee+]+)", re.IGNORECASE),
    re.compile(r"^\s*Total\s+energy\s*=\s*([-+0-9.Ee+]+)", re.IGNORECASE | re.M),
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


def run_nwchem_cli(nw_path: Path, work: Path) -> tuple[int, str, float | None]:
    nwchem = shutil.which("nwchem")
    if not nwchem:
        return 127, "nwchem not on PATH (enable pixi feature nwchem-runtime)", None
    src = (ROOT / nw_path).resolve()
    if not src.is_file():
        return 2, f"missing input {src}", None
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
    return proc.returncode, combined, energy


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
        help="Optional Meson build dir with embed tests (e.g. build-nwchem)",
    )
    ap.add_argument(
        "--require-cli",
        action="store_true",
        help="Exit non-zero if nwchem CLI is missing",
    )
    args = ap.parse_args()

    matrix = json.loads(MATRIX.read_text(encoding="utf-8"))
    tol_e = float(matrix["tolerances"]["energy_ha_abs"])
    out_dir: Path = args.out_dir
    out_dir.mkdir(parents=True, exist_ok=True)

    report = {
        "matrix": str(MATRIX.relative_to(ROOT)),
        "nwchem": shutil.which("nwchem"),
        "embed_build": str(args.embed_build) if args.embed_build else None,
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
    lines = [f"nwchem_cli={report['nwchem']}", f"tol_energy_ha={tol_e}"]

    for task in matrix["tasks"]:
        tid = task["id"]
        work = out_dir / tid
        rc, text, energy = run_nwchem_cli(Path(task["nw_input"]), work)
        entry = {
            "id": tid,
            "cli_rc": rc,
            "cli_energy_ha": energy,
            "embed_status": "not-run",
            "pass": False,
        }
        if rc != 0 or energy is None:
            all_ok = False
            entry["error"] = f"cli failed rc={rc} energy={energy}"
            lines.append(f"{tid}: FAIL cli ({entry['error']})")
        else:
            entry["pass"] = True
            lines.append(f"{tid}: CLI energy={energy:.12f} Ha OK")
            # Embed optional: only mark if build dir exists and meson test binary present.
            if args.embed_build and args.embed_build.is_dir():
                entry["embed_status"] = "skipped-no-auto-runner"
                entry["embed_note"] = (
                    "Run meson test -C "
                    + str(args.embed_build)
                    + " "
                    + task.get("embed_meson_test", "")
                    + " manually; numeric embed vs CLI auto-diff requires embed SDK."
                )
                lines.append(
                    f"{tid}: embed leg optional at {args.embed_build} "
                    f"(test {task.get('embed_meson_test')})"
                )
        report["tasks"].append(entry)

    report["status"] = "ok" if all_ok else "fail"
    report_txt = "\n".join(lines) + "\n"
    (out_dir / "nwchem_compare_report.json").write_text(
        json.dumps(report, indent=2), encoding="utf-8"
    )
    (out_dir / "nwchem_compare_report.txt").write_text(report_txt, encoding="utf-8")
    print(report_txt)
    return 0 if all_ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
