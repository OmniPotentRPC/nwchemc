#!/usr/bin/env python3
import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SCHEMA = ROOT / "schema" / "Potentials.capnp"

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


class SchemaModuleCoverageTest(unittest.TestCase):
    def test_nwchem_module_enum_covers_curated_input_handlers(self):
        text = SCHEMA.read_text()
        match = re.search(r"enum NWChemModuleName \{(?P<body>.*?)\n\}", text, re.S)
        self.assertIsNotNone(match)
        body = match.group("body")
        actual = set(re.findall(r"^\s*([a-z][A-Za-z0-9]*)\s+@", body, re.M))
        missing = sorted(EXPECTED_MODULE_ENUMS - actual)

        self.assertEqual(missing, [])


if __name__ == "__main__":
    raise SystemExit(unittest.main())
