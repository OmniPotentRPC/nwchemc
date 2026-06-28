#!/usr/bin/env python3
import re
import unittest
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SCHEMA = ROOT / "schema" / "Potentials.capnp"
INVENTORY = ROOT / "schema" / "inventory" / "nwchem_features.json"

EXPECTED_MODULE_ENUMS = {
    "analysis",
    "argos",
    "argosDiana",
    "argosPrep",
    "argosPrepare",
    "basis",
    "band",
    "bandDplot",
    "bq",
    "brillouinZone",
    "bsemol",
    "cckohn",
    "ccsd",
    "cellOptimize",
    "cgsd",
    "constraints",
    "cosmo",
    "cpmd",
    "cpsd",
    "ddscf",
    "dft",
    "diana",
    "dimpar",
    "dimqm",
    "dk",
    "dmd",
    "dntmc",
    "dplot",
    "drdy",
    "driver",
    "esp",
    "etrans",
    "fractionalOccupations",
    "freeze",
    "geometry",
    "gw",
    "hessian",
    "intgrl",
    "mcscf",
    "md",
    "mdXs",
    "mepgs",
    "metadynamics",
    "mm",
    "modelpotential",
    "mp2",
    "ncc",
    "neb",
    "nwpw",
    "occup",
    "prepare",
    "property",
    "pspFormatter",
    "pspGenerator",
    "pspw",
    "pspwDplot",
    "pspwQmmm",
    "pspwWannier",
    "python",
    "qmd",
    "qmdNamd",
    "qmmm",
    "raman",
    "rel",
    "rimp2",
    "rism",
    "rtTddft",
    "scf",
    "selci",
    "simulationCell",
    "smd",
    "string",
    "tamd",
    "task",
    "taskShell",
    "tce",
    "tceMrcc",
    "tddft",
    "tddftGradient",
    "tropt",
    "vib",
    "vibZone",
    "vscf",
    "waterPseudopotential",
    "x2c",
    "xtb",
    "zora",
}

EXPECTED_POTENTIAL_METHODS = {
    "calculate": 0,
    "configure": 1,
    "calculateEnergy": 2,
    "calculateForces": 3,
    "calculateHessian": 4,
    "calculateDipole": 5,
    "calculateQuadrupole": 6,
    "calculateStress": 7,
    "calculateOptimize": 8,
    "calculateFrequencies": 9,
    "calculatePolarizability": 10,
    "calculateGradient": 11,
}

EXPECTED_POTENTIAL_RESULT_FIELDS = {
    "energy": 0,
    "forces": 1,
    "hessian": 2,
    "dipole": 3,
    "quadrupole": 4,
    "optimizedPos": 5,
    "frequencies": 6,
    "intensities": 7,
    "stress": 8,
    "polarizability": 9,
    "gradient": 10,
}


class SchemaModuleCoverageTest(unittest.TestCase):
    def test_nwchem_module_enum_covers_curated_input_handlers(self):
        text = SCHEMA.read_text()
        match = re.search(r"enum NWChemModuleName \{(?P<body>.*?)\n\}", text, re.S)
        self.assertIsNotNone(match)
        body = match.group("body")
        actual = set(re.findall(r"^\s*([a-z][A-Za-z0-9]*)\s+@", body, re.M))
        missing = sorted(EXPECTED_MODULE_ENUMS - actual)

        self.assertEqual(missing, [])

    def test_potential_service_exposes_c_abi_operations(self):
        text = SCHEMA.read_text()
        match = re.search(r"interface Potential \{(?P<body>.*?)\n\}", text, re.S)
        self.assertIsNotNone(match)
        body = match.group("body")
        actual = {
            name: int(ordinal)
            for name, ordinal in re.findall(
                r"^\s*([a-z][A-Za-z0-9]*)\s+@([0-9]+)\s*\(", body, re.M
            )
        }

        self.assertEqual(actual, EXPECTED_POTENTIAL_METHODS)

    def test_potential_result_exposes_nwchem_gradient(self):
        text = SCHEMA.read_text()
        match = re.search(r"struct PotentialResult \{(?P<body>.*?)\n\}", text, re.S)
        self.assertIsNotNone(match)
        body = match.group("body")
        actual = {
            name: int(ordinal)
            for name, ordinal in re.findall(
                r"^\s*([a-z][A-Za-z0-9]*)\s+@([0-9]+)\s*:", body, re.M
            )
        }

        self.assertEqual(actual, EXPECTED_POTENTIAL_RESULT_FIELDS)

    def test_inventory_tracks_potential_operation_methods(self):
        inv = json.loads(INVENTORY.read_text(encoding="utf-8"))
        methods = {
            entry["name"]: entry
            for entry in inv.get("schema_methods", [])
            if entry["interface"] == "Potential"
        }

        self.assertEqual(set(methods), set(EXPECTED_POTENTIAL_METHODS))
        for name, ordinal in EXPECTED_POTENTIAL_METHODS.items():
            entry = methods[name]
            self.assertEqual(entry["method_id"], ordinal)
            self.assertEqual(entry["feature_id"], f"method.Potential.{name}")
            self.assertEqual(entry["schema_path"], f"Potential.{name}")


if __name__ == "__main__":
    raise SystemExit(unittest.main())
