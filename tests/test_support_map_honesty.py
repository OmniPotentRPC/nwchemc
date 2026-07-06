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
    assert by["module.cosmo"]["support_tier"] in {"schema", "text"}
    assert by["stanza.simulationCell"]["support_tier"] in {"embed", "tested"}
    assert by["stanza.basis"]["support_tier"] in {"embed", "tested"}

    print("support_map_honesty_ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
