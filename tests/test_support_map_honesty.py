#!/usr/bin/env python3
"""Drive tools/build_nwchem_support_map.py and assert honest module tiers."""
from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main() -> int:
    subprocess.check_call(
        [sys.executable, str(ROOT / "tools/build_nwchem_support_map.py"), str(ROOT)],
        cwd=ROOT,
    )
    status = json.loads(
        (ROOT / "schema/inventory/nwchem_support_status.json").read_text(
            encoding="utf-8"
        )
    )
    by = {f["feature_id"]: f for f in status["features"]}

    # Must not be embed/tested (substring false positives previously).
    for fid in (
        "module.md",
        "module.mm",
        "module.string",
        "module.occup",
        "module.cpmd",
        "module.vscf",
        "module.ddscf",
    ):
        assert fid in by, fid
        assert by[fid]["support_tier"] in {"schema", "text"}, (
            fid,
            by[fid]["support_tier"],
        )
        assert by[fid]["done"] is False

    # Known embed/tested anchors.
    assert by["module.dft"]["support_tier"] == "tested"
    assert by["module.scf"]["support_tier"] == "tested"
    assert by["stanza.basis"]["support_tier"] == "tested"
    for fid in (
        "module.cosmo",
        "stanza.cosmo",
        "module.esp",
        "stanza.esp",
        "module.constraints",
        "stanza.constraints",
    ):
        assert by[fid]["support_tier"] == "tested", (fid, by[fid]["support_tier"])
        assert by[fid]["done"] is True

    # Former embed-only nine must be tested once shipped-path markers exist.
    nine = (
        "module.band",
        "module.basis",
        "module.brillouinZone",
        "module.driver",
        "module.geometry",
        "module.hessian",
        "module.property",
        "module.simulationCell",
        "stanza.simulationCell",
    )
    for fid in nine:
        assert fid in by, fid
        assert by[fid]["support_tier"] == "tested", (fid, by[fid]["support_tier"])
        assert by[fid]["done"] is True

    # Structural: each of the nine has a real test/source marker in-tree.
    tests_root = ROOT / "tests"
    blob = "\n".join(
        p.read_text(encoding="utf-8", errors="replace")
        for p in tests_root.rglob("*")
        if p.suffix in {".c", ".F", ".f90", ".txt", ".py", ".json"}
    )
    required_snippets = {
        "module.basis": "nwchemc_test_geometry_basis_rtdb",
        "module.geometry": "nwchemc_store_geometry_cell",
        "module.driver": "nwchemc_optimize(",
        "module.property": "nwchemc_dipole(",
        "module.hessian": "nwchemc_hessian(",
        "module.brillouinZone": "nwchemc_test_brillouin_dos_zones_rtdb",
        "module.band": 'assert_typed_set_scalar("band:wcut"',
        "module.simulationCell": "kind = simulationCell",
        "stanza.simulationCell": "nwchem_params_compact_cells",
    }
    for fid, snip in required_snippets.items():
        assert snip in blob, (fid, snip)

    print("support_map_honesty_ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
