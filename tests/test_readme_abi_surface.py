#!/usr/bin/env python3
import os
import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
HEADER = Path(os.environ.get("NWCHEMC_HEADER_PATH", ROOT / "include" / "nwchemc.h"))
README = Path(os.environ.get("NWCHEMC_README_PATH", ROOT / "README.md"))
RGPOT_GUIDE = Path(
    os.environ.get("NWCHEMC_RGPOT_GUIDE_PATH", ROOT / "docs" / "rgpot-integration.md")
)
V2_PLAN = Path(
    os.environ.get(
        "NWCHEMC_V2_PLAN_PATH",
        ROOT / "docs" / "orgmode" / "contributing" / "developer" / "v2-plan.org",
    )
)

DECL_RE = re.compile(
    r"""
    (?:^|\n)\s*
    (?:
      NWChemCResult\s+|
      NWChemCSession\s+\*\s*|
      const\s+char\s+\*\s*|
      size_t\s+|
      int\s+|
      void\s+
    )
    (nwchemc_[A-Za-z0-9_]+)\s*\(
    """,
    re.VERBOSE,
)


def unique_in_order(names):
    seen = set()
    result = []
    for name in names:
        if name in seen:
            continue
        seen.add(name)
        result.append(name)
    return result


def declared_abi_names(text):
    return unique_in_order(DECL_RE.findall(text))


def readme_abi_block(text):
    match = re.search(r"```c\n(?P<body>.*?)\n```", text, re.S)
    if not match:
        raise AssertionError("README.md does not contain a C ABI code block")
    return match.group("body")


def v2_plan_current_state_block(text):
    match = re.search(r"\* Current State\n(?P<body>.*?)\n\* RTDB Path", text, re.S)
    if not match:
        raise AssertionError("v2-plan.org does not contain a Current State block")
    return match.group("body")


def org_abi_names(text):
    return unique_in_order(re.findall(r"~(nwchemc_[A-Za-z0-9_]+)~", text))


class ReadmeAbiSurfaceTest(unittest.TestCase):
    maxDiff = None

    def test_readme_c_block_matches_header_exports(self):
        header_names = declared_abi_names(HEADER.read_text(encoding="utf-8"))
        readme_names = declared_abi_names(
            readme_abi_block(README.read_text(encoding="utf-8"))
        )

        missing = sorted(set(header_names) - set(readme_names))
        extra = sorted(set(readme_names) - set(header_names))
        self.assertEqual(missing, [])
        self.assertEqual(extra, [])
        self.assertEqual(readme_names, header_names)

    def test_v2_plan_current_state_matches_header_exports(self):
        header_names = declared_abi_names(HEADER.read_text(encoding="utf-8"))
        plan_names = org_abi_names(
            v2_plan_current_state_block(V2_PLAN.read_text(encoding="utf-8"))
        )

        missing = sorted(set(header_names) - set(plan_names))
        extra = sorted(set(plan_names) - set(header_names))
        self.assertEqual(missing, [])
        self.assertEqual(extra, [])
        self.assertEqual(plan_names, header_names)

    def test_rgpot_integration_guide_covers_release_contract(self):
        readme = README.read_text(encoding="utf-8")
        self.assertIn("docs/rgpot-integration.md", readme)
        self.assertTrue(RGPOT_GUIDE.exists())

        guide = RGPOT_GUIDE.read_text(encoding="utf-8")
        required_terms = [
            "PotentialConfig.nwchem",
            "ForceInput",
            "PotentialResult",
            "PotentialResult.normalModes",
            "PotentialResult.projectedFrequencies",
            "PotentialResult.projectedIntensities",
            "PotentialResult.zeroPointEnergy",
            "nwchemc_session_create_from_config",
            "nwchemc_potential_result_size_for_force_input",
            "nwchemc_gradient_result_size_for_force_input",
            "nwchemc_session_calculate_result",
            "nwchemc_calculate_result_from_config",
            "nwchemc_calculate_gradient_result_from_config",
            "nwchemc_calculate_hessian_result_from_config",
            "nwchemc_calculate_dipole_result_from_config",
            "nwchemc_calculate_polarizability_result_from_config",
            "nwchemc_calculate_quadrupole_result_from_config",
            "nwchemc_calculate_stress_result_from_config",
            "nwchemc_calculate_optimize_result_from_config",
            "nwchemc_calculate_frequencies_result_from_config",
            "nwchemc_session_calculate_frequencies_detail",
            "nwchemc_calculate_frequencies_detail_from_config",
            "tests/test_nwchem_rgpot_smoke.c",
            "PSPW pseudopotential energy, forces, and gradient",
            "tests/test_nwchem_forceinput_cell_rtdb.c",
            "tests/test_nwchem_potential_config_pseudopotential.c",
            "NWPW-enabled NWChem",
        ]
        for term in required_terms:
            with self.subTest(term=term):
                self.assertIn(term, guide)


if __name__ == "__main__":
    raise SystemExit(unittest.main())
