#!/usr/bin/env python3
import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
CMAKE = ROOT / "CMakeLists.txt"


CMOCKA_FIXTURES = [
    ("nwchem_direct_dft_params_bin", "NWCHEMC_DIRECT_DFT_PARAMS_BIN"),
    ("nwchem_config_options_params_bin", "NWCHEMC_CONFIG_OPTIONS_PARAMS_BIN"),
    ("nwchem_pspspin_params_bin", "NWCHEMC_PSPSPIN_PARAMS_BIN"),
    ("nwchem_pspspin_many_params_bin", "NWCHEMC_PSPSPIN_MANY_PARAMS_BIN"),
    ("nwchem_nwpw_spin_mode_params_bin", "NWCHEMC_NWPW_SPIN_MODE_PARAMS_BIN"),
    (
        "nwchem_nwpw_allow_translation_params_bin",
        "NWCHEMC_NWPW_ALLOW_TRANSLATION_PARAMS_BIN",
    ),
    (
        "nwchem_nwpw_cutoff_alias_params_bin",
        "NWCHEMC_NWPW_CUTOFF_ALIAS_PARAMS_BIN",
    ),
    ("nwchem_nwpw_mc_steps_params_bin", "NWCHEMC_NWPW_MC_STEPS_PARAMS_BIN"),
    (
        "nwchem_nwpw_bo_steps_default_params_bin",
        "NWCHEMC_NWPW_BO_STEPS_DEFAULT_PARAMS_BIN",
    ),
    (
        "nwchem_nwpw_bo_time_step_default_params_bin",
        "NWCHEMC_NWPW_BO_TIME_STEP_DEFAULT_PARAMS_BIN",
    ),
    (
        "nwchem_nwpw_bo_fake_mass_default_params_bin",
        "NWCHEMC_NWPW_BO_FAKE_MASS_DEFAULT_PARAMS_BIN",
    ),
    (
        "nwchem_nwpw_scaling_default_params_bin",
        "NWCHEMC_NWPW_SCALING_DEFAULT_PARAMS_BIN",
    ),
    (
        "nwchem_nwpw_np_dimensions_default_params_bin",
        "NWCHEMC_NWPW_NP_DIMENSIONS_DEFAULT_PARAMS_BIN",
    ),
    (
        "nwchem_nwpw_tolerances_default_params_bin",
        "NWCHEMC_NWPW_TOLERANCES_DEFAULT_PARAMS_BIN",
    ),
    (
        "nwchem_nwpw_mc_steps_default_params_bin",
        "NWCHEMC_NWPW_MC_STEPS_DEFAULT_PARAMS_BIN",
    ),
    (
        "nwchem_brillouin_tetrahedron_params_bin",
        "NWCHEMC_BRILLOUIN_TETRAHEDRON_PARAMS_BIN",
    ),
    (
        "nwchem_brillouin_dos_grid_params_bin",
        "NWCHEMC_BRILLOUIN_DOS_GRID_PARAMS_BIN",
    ),
    ("nwchem_nwpw_et_params_bin", "NWCHEMC_NWPW_ET_PARAMS_BIN"),
    (
        "nwchem_nwpw_temperature_params_bin",
        "NWCHEMC_NWPW_TEMPERATURE_PARAMS_BIN",
    ),
    (
        "nwchem_nwpw_dos_default_params_bin",
        "NWCHEMC_NWPW_DOS_DEFAULT_PARAMS_BIN",
    ),
    (
        "nwchem_nwpw_mapping_alias_params_bin",
        "NWCHEMC_NWPW_MAPPING_ALIAS_PARAMS_BIN",
    ),
    (
        "nwchem_nwpw_mapping_default_params_bin",
        "NWCHEMC_NWPW_MAPPING_DEFAULT_PARAMS_BIN",
    ),
    (
        "nwchem_nwpw_virtual_alias_params_bin",
        "NWCHEMC_NWPW_VIRTUAL_ALIAS_PARAMS_BIN",
    ),
    (
        "nwchem_nwpw_one_electron_guess_defaults_params_bin",
        "NWCHEMC_NWPW_ONE_ELECTRON_GUESS_DEFAULTS_PARAMS_BIN",
    ),
    (
        "nwchem_nwpw_fractional_orbitals_default_params_bin",
        "NWCHEMC_NWPW_FRACTIONAL_ORBITALS_DEFAULT_PARAMS_BIN",
    ),
    (
        "nwchem_nwpw_smear_orbitals_default_params_bin",
        "NWCHEMC_NWPW_SMEAR_ORBITALS_DEFAULT_PARAMS_BIN",
    ),
    (
        "nwchem_nwpw_virtual_orbitals_default_params_bin",
        "NWCHEMC_NWPW_VIRTUAL_ORBITALS_DEFAULT_PARAMS_BIN",
    ),
    ("force_input_step_a_bin", "NWCHEMC_FORCE_INPUT_STEP_A_BIN"),
    ("force_input_step_b_bin", "NWCHEMC_FORCE_INPUT_STEP_B_BIN"),
    ("force_input_step_ev_bin", "NWCHEMC_FORCE_INPUT_STEP_EV_BIN"),
    (
        "force_input_step_changed_species_bin",
        "NWCHEMC_FORCE_INPUT_STEP_CHANGED_SPECIES_BIN",
    ),
    ("force_input_step_state_bin", "NWCHEMC_FORCE_INPUT_STEP_STATE_BIN"),
    ("nwchem_tce_methods_params_bin", "NWCHEMC_TCE_METHODS_PARAMS_BIN"),
    ("nwchem_compact_cells_params_bin", "NWCHEMC_COMPACT_CELLS_PARAMS_BIN"),
    (
        "nwchem_nwpw_translate_vector_default_params_bin",
        "NWCHEMC_NWPW_TRANSLATE_VECTOR_DEFAULT_PARAMS_BIN",
    ),
    (
        "nwchem_brillouin_monkhorst_default_params_bin",
        "NWCHEMC_BRILLOUIN_MONKHORST_DEFAULT_PARAMS_BIN",
    ),
    (
        "nwchem_brillouin_dos_grid_default_params_bin",
        "NWCHEMC_BRILLOUIN_DOS_GRID_DEFAULT_PARAMS_BIN",
    ),
]


class CMakeRealNWChemContractTest(unittest.TestCase):
    maxDiff = None

    def setUp(self):
        self.text = CMAKE.read_text(encoding="utf-8")

    def _block(self, start_pattern):
        match = re.search(start_pattern, self.text, re.S)
        self.assertIsNotNone(match, start_pattern)
        start = match.start()
        tail = self.text[start:]
        end = re.search(r"\n  \)", tail)
        self.assertIsNotNone(end, start_pattern)
        return tail[: end.end()]

    def assert_contains_all(self, haystack, needles):
        missing = [needle for needle in needles if needle not in haystack]
        self.assertEqual(missing, [])

    def test_embed_config_cmocka_receives_all_fixture_arguments(self):
        dependencies = self._block(r"add_dependencies\(\s*test_embed_config_cmocka\b")
        command = self._block(r"add_test\(\s*NAME\s+embed-config-cmocka\b")

        self.assert_contains_all(
            dependencies, [target for target, _variable in CMOCKA_FIXTURES]
        )
        self.assert_contains_all(
            command, [variable for _target, variable in CMOCKA_FIXTURES]
        )

    def test_generates_rgpot_potential_config_and_cell_fixtures(self):
        self.assert_contains_all(
            self.text,
            [
                "potential_config_structured.capnp.txt.in",
                "potential_config_pseudopotential.capnp.txt.in",
                "PotentialConfig",
                "force_input_h2_box_ang.capnp.txt",
                "force_input_h2_box_bohr.capnp.txt",
            ],
        )

    def test_registers_mesons_real_nwchem_rgpot_and_rtdb_probes(self):
        self.assert_contains_all(
            self.text,
            [
                "nwchem-config-parity",
                "tests/test_nwchem_config_parity.c",
                "nwchem-rgpot-smoke",
                "tests/test_nwchem_rgpot_smoke.c",
                "nwchem-potential-config-pseudopotential",
                "tests/test_nwchem_potential_config_pseudopotential.c",
                "tests/nwchem_configured_pseudopotential_rtdb_probe.F",
                "nwchem-forceinput-cell-rtdb",
                "tests/test_nwchem_forceinput_cell_rtdb.c",
                "tests/nwchem_forceinput_cell_rtdb_probe.F",
                "nwchem-pseudopotential-rtdb",
                "tests/test_nwchem_pseudopotential_rtdb.c",
                "tests/nwchem_pseudopotential_rtdb_probe.F",
                "nwchem-nwpw-rtdb",
                "tests/test_nwchem_nwpw_rtdb.c",
                "tests/nwchem_nwpw_rtdb_probe.F",
            ],
        )


if __name__ == "__main__":
    raise SystemExit(unittest.main())
