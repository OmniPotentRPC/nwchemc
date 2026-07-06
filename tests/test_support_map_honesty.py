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

    # CommonMethodSpec (NOMAD overlay): every inventory field is tested via
    # potential_config_common_overlay / planewave embed-config paths.
    common_fields = (
        "field.CommonMethodSpec.xcFunctionals",
        "field.CommonMethodSpec.basisSet",
        "field.CommonMethodSpec.planewaveCutoffEv",
        "field.CommonMethodSpec.charge",
        "field.CommonMethodSpec.spinMultiplicity",
        "field.CommonMethodSpec.scfEnergyToleranceEv",
        "field.CommonMethodSpec.scfMaxIterations",
        "field.CommonMethodSpec.kMesh",
        "field.CommonMethodSpec.smearing",
        "field.CommonMethodSpec.vanDerWaalsMethod",
        "field.CommonMethodSpec.relativityMethod",
        "field.CommonMethodSpec.vanDerWaalsS6",
        "field.CommonMethodSpec.kind",
        "field.CommonMethodSpec.widthEv",
    )
    for fid in common_fields:
        assert fid in by, fid
        assert by[fid]["support_tier"] == "tested", (fid, by[fid]["support_tier"])
        assert by[fid]["done"] is True
    common_snippets = {
        "field.CommonMethodSpec.xcFunctionals": 'assert_string_equal(g_dft_xc, "xpbe96 cpbe96")',
        "field.CommonMethodSpec.basisSet": 'assert_string_equal(g_basis, "6-31g")',
        "field.CommonMethodSpec.planewaveCutoffEv": "test_common_overlay_planewave_kmesh_and_cutoff",
        "field.CommonMethodSpec.charge": "assert_int_equal(g_config_charge, 1)",
        "field.CommonMethodSpec.spinMultiplicity": "assert_int_equal(g_config_mult, 2)",
        "field.CommonMethodSpec.scfEnergyToleranceEv": 'strcmp(g_typed_set_keys[i], "dft:e_conv")',
        "field.CommonMethodSpec.scfMaxIterations": "assert_int_equal(g_scf_maxiter, 42)",
        "field.CommonMethodSpec.kMesh": "g_brillouin_monkhorst_pack",
        "field.CommonMethodSpec.smearing": "assert_int_equal(g_dft_smearing_enabled, 1)",
        "field.CommonMethodSpec.vanDerWaalsMethod": 'strcmp(g_typed_set_keys[i], "dft:ivdw")',
        "field.CommonMethodSpec.relativityMethod": 'strcmp(g_typed_set_keys[i], "zora")',
        "field.CommonMethodSpec.vanDerWaalsS6": 'strcmp(g_typed_set_keys[i], "dft:vdw")',
        "field.CommonMethodSpec.kind": "smearing = (kind = fermi",
        "field.CommonMethodSpec.widthEv": "widthEv = 0.27211386245988",
    }
    for fid, snip in common_snippets.items():
        assert snip in blob, (fid, snip)

    print("support_map_honesty_ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
