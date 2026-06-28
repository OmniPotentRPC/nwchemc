#!/usr/bin/env python3
import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
MESON = ROOT / "meson.build"
README = ROOT / "README.md"
RGPOT_GUIDE = ROOT / "docs" / "rgpot-integration.md"


class MesonInstallContractTest(unittest.TestCase):
    maxDiff = None

    def setUp(self):
        self.text = MESON.read_text(encoding="utf-8")

    def test_public_header_is_installed(self):
        self.assertRegex(
            self.text,
            re.compile(
                r"install_headers\(\s*'include/nwchemc\.h'\s*,\s*"
                r"subdir:\s*'nwchemc'\s*,?\s*\)",
                re.S,
            ),
        )

    def test_pkg_config_file_exports_nwchemc_library(self):
        self.assertIn("pkgconfig = import('pkgconfig')", self.text)
        self.assertRegex(
            self.text,
            re.compile(
                r"pkgconfig\.generate\(\s*"
                r"name:\s*'nwchemc'\s*,.*"
                r"filebase:\s*'nwchemc'\s*,.*"
                r"libraries:\s*nwchemc\s*,.*"
                r"subdirs:\s*'nwchemc'\s*,.*"
                r"version:\s*meson\.project_version\(\)\s*,.*"
                r"description:\s*'Stable C ABI for embedding NWChem'\s*,?\s*\)",
                re.S,
            ),
        )

    def test_real_nwchem_registers_installed_cmake_consumer_smoke(self):
        required_terms = [
            "nwchem-installed-cmake-consumer",
            "tests/test_installed_cmake_consumer.py",
            "--nwchem-root",
            "--nwchem-target",
            "--install-prefix",
            "--mpi-ranks",
        ]
        missing = [term for term in required_terms if term not in self.text]
        self.assertEqual(missing, [])

    def test_real_nwchem_registers_installed_pkgconfig_consumer_smoke(self):
        required_terms = [
            "nwchem-installed-pkgconfig-consumer",
            "tests/test_installed_pkgconfig_consumer.py",
            "--build-root",
            "--install-prefix",
            "--cc",
            "--pkg-config",
            "--mpi-ranks",
        ]
        missing = [term for term in required_terms if term not in self.text]
        self.assertEqual(missing, [])

    def test_installed_consumers_compile_rgpot_result_abi(self):
        required_terms = [
            "nwchemc_potential_result_size_for_force_input",
            "nwchemc_energy_result_size_for_force_input",
            "nwchemc_forces_result_size_for_force_input",
            "nwchemc_hessian_result_size_for_force_input",
            "nwchemc_dipole_result_size_for_force_input",
            "nwchemc_polarizability_result_size_for_force_input",
            "nwchemc_quadrupole_result_size_for_force_input",
            "nwchemc_stress_result_size_for_force_input",
            "nwchemc_optimize_result_size_for_force_input",
            "nwchemc_frequencies_result_size_for_force_input",
            "nwchemc_calculate_result_from_config",
            "nwchemc_calculate_energy_result_from_config",
            "nwchemc_calculate_forces_result_from_config",
            "nwchemc_calculate_hessian_result_from_config",
            "nwchemc_calculate_dipole_result_from_config",
            "nwchemc_calculate_polarizability_result_from_config",
            "nwchemc_calculate_quadrupole_result_from_config",
            "nwchemc_calculate_stress_result_from_config",
            "nwchemc_calculate_optimize_result_from_config",
            "nwchemc_calculate_frequencies_result_from_config",
        ]
        consumer_scripts = [
            ROOT / "tests" / "test_installed_cmake_consumer.py",
            ROOT / "tests" / "test_installed_pkgconfig_consumer.py",
        ]
        for script in consumer_scripts:
            with self.subTest(script=script.name):
                consumer = script.read_text(encoding="utf-8")
                missing = [term for term in required_terms if term not in consumer]
                self.assertEqual(missing, [])

    def test_installed_consumers_compile_rgpot_raw_and_session_abi(self):
        required_terms = [
            "nwchemc_session_calculate_energy",
            "nwchemc_session_calculate_forces",
            "nwchemc_session_calculate_hessian",
            "nwchemc_session_calculate_dipole",
            "nwchemc_session_calculate_polarizability",
            "nwchemc_session_calculate_quadrupole",
            "nwchemc_session_calculate_stress",
            "nwchemc_session_calculate_optimize",
            "nwchemc_session_calculate_frequencies",
            "nwchemc_session_calculate_energy_result",
            "nwchemc_session_calculate_forces_result",
            "nwchemc_session_calculate_result",
            "nwchemc_session_calculate_hessian_result",
            "nwchemc_session_calculate_dipole_result",
            "nwchemc_session_calculate_polarizability_result",
            "nwchemc_session_calculate_quadrupole_result",
            "nwchemc_session_calculate_stress_result",
            "nwchemc_session_calculate_optimize_result",
            "nwchemc_session_calculate_frequencies_result",
            "nwchemc_calculate_energy_from_config",
            "nwchemc_calculate_forces_from_config",
            "nwchemc_calculate_hessian_from_config",
            "nwchemc_calculate_dipole_from_config",
            "nwchemc_calculate_polarizability_from_config",
            "nwchemc_calculate_quadrupole_from_config",
            "nwchemc_calculate_stress_from_config",
            "nwchemc_calculate_optimize_from_config",
            "nwchemc_calculate_frequencies_from_config",
        ]
        consumer_scripts = [
            ROOT / "tests" / "test_installed_cmake_consumer.py",
            ROOT / "tests" / "test_installed_pkgconfig_consumer.py",
        ]
        for script in consumer_scripts:
            with self.subTest(script=script.name):
                consumer = script.read_text(encoding="utf-8")
                missing = [term for term in required_terms if term not in consumer]
                self.assertEqual(missing, [])

    def test_installed_consumers_compile_coordinate_session_abi(self):
        required_terms = [
            "nwchemc_set_params",
            "nwchemc_configure",
            "nwchemc_energy_gradient",
            "nwchemc_energy",
            "nwchemc_energy_forces",
            "nwchemc_hessian",
            "nwchemc_dipole",
            "nwchemc_polarizability",
            "nwchemc_quadrupole",
            "nwchemc_stress",
            "nwchemc_optimize",
            "nwchemc_frequencies",
            "nwchemc_energy_gradient_from_config",
            "nwchemc_energy_from_config",
            "nwchemc_energy_forces_from_config",
            "nwchemc_hessian_from_config",
            "nwchemc_dipole_from_config",
            "nwchemc_polarizability_from_config",
            "nwchemc_quadrupole_from_config",
            "nwchemc_stress_from_config",
            "nwchemc_optimize_from_config",
            "nwchemc_frequencies_from_config",
            "nwchemc_session_create",
            "nwchemc_session_create_from_config",
            "nwchemc_session_set_params",
            "nwchemc_session_configure",
            "nwchemc_session_destroy",
            "nwchemc_session_energy_gradient",
            "nwchemc_session_energy",
            "nwchemc_session_energy_forces",
            "nwchemc_session_dipole",
            "nwchemc_session_polarizability",
            "nwchemc_session_quadrupole",
            "nwchemc_session_optimize",
            "nwchemc_session_frequencies",
            "nwchemc_session_stress",
            "nwchemc_session_hessian",
        ]
        consumer_scripts = [
            ROOT / "tests" / "test_installed_cmake_consumer.py",
            ROOT / "tests" / "test_installed_pkgconfig_consumer.py",
        ]
        for script in consumer_scripts:
            with self.subTest(script=script.name):
                consumer = script.read_text(encoding="utf-8")
                missing = [term for term in required_terms if term not in consumer]
                self.assertEqual(missing, [])

    def test_installed_consumers_compile_params_forceinput_abi(self):
        required_symbols = [
            "nwchemc_calculate_energy",
            "nwchemc_calculate_forces",
            "nwchemc_calculate_hessian",
            "nwchemc_calculate_dipole",
            "nwchemc_calculate_polarizability",
            "nwchemc_calculate_quadrupole",
            "nwchemc_calculate_stress",
            "nwchemc_calculate_optimize",
            "nwchemc_calculate_frequencies",
            "nwchemc_calculate_result",
            "nwchemc_calculate_energy_result",
            "nwchemc_calculate_forces_result",
            "nwchemc_calculate_hessian_result",
            "nwchemc_calculate_dipole_result",
            "nwchemc_calculate_polarizability_result",
            "nwchemc_calculate_quadrupole_result",
            "nwchemc_calculate_stress_result",
            "nwchemc_calculate_optimize_result",
            "nwchemc_calculate_frequencies_result",
        ]
        consumer_scripts = [
            ROOT / "tests" / "test_installed_cmake_consumer.py",
            ROOT / "tests" / "test_installed_pkgconfig_consumer.py",
        ]
        for script in consumer_scripts:
            with self.subTest(script=script.name):
                consumer = script.read_text(encoding="utf-8")
                missing = [
                    symbol
                    for symbol in required_symbols
                    if not re.search(rf"\b{re.escape(symbol)}\b", consumer)
                ]
                self.assertEqual(missing, [])

    def test_installed_consumers_exercise_invalid_input_abi(self):
        required_terms = [
            "expect_result_failure",
            "expect_size_zero",
            "expect_status_failure",
            "expect_null_session",
            "size_fns[index](NULL, 0)",
            "result_fns[index](NULL, 0, NULL, 0, result_bytes",
            "params_energy_fns[index](NULL, 0, NULL, 0)",
            "params_buffer_fns[index](NULL, 0, NULL, 0, doubles",
            "params_frequency_fns[index](NULL, 0, NULL, 0, doubles",
            "params_result_fns[index](NULL, 0, NULL, 0, result_bytes",
            "config_energy_fns[index](NULL, 0, NULL, 0)",
            "config_buffer_fns[index](NULL, 0, NULL, 0, doubles",
            "config_frequency_fns[index](NULL, 0, NULL, 0, doubles",
            "session_energy_fns[index](NULL, NULL, 0)",
            "session_buffer_fns[index](NULL, NULL, 0, doubles",
            "session_frequency_fns[index](NULL, NULL, 0, doubles",
            "session_result_fns[index](NULL, NULL, 0, result_bytes",
            "config_fns[index](NULL, 0)",
            "session_create_fns[index](NULL, 0)",
            "session_config_fns[index](NULL, NULL, 0)",
            "coordinate_energy_fns[index](0, NULL, NULL, NULL, 0)",
            "coordinate_buffer_fns[index](0, NULL, NULL, NULL, 0, doubles)",
            "coordinate_frequency_fns[index](0, NULL, NULL, NULL, 0, doubles",
            "session_coordinate_energy_fns[index](NULL, 0, NULL, NULL)",
            "session_coordinate_buffer_fns[index](NULL, 0, NULL, NULL, doubles)",
            "session_coordinate_frequency_fns[index](NULL, 0, NULL, NULL, doubles",
        ]
        consumer_scripts = [
            ROOT / "tests" / "test_installed_cmake_consumer.py",
            ROOT / "tests" / "test_installed_pkgconfig_consumer.py",
        ]
        for script in consumer_scripts:
            with self.subTest(script=script.name):
                consumer = script.read_text(encoding="utf-8")
                missing = [term for term in required_terms if term not in consumer]
                self.assertEqual(missing, [])

    def test_docs_explain_installed_consumer_release_gate(self):
        docs = README.read_text(encoding="utf-8") + "\n" + RGPOT_GUIDE.read_text(
            encoding="utf-8"
        )
        required_terms = [
            "Installed package smoke",
            "find_package(nwchemc CONFIG REQUIRED)",
            "target_link_libraries(consumer PRIVATE nwchemc::nwchemc)",
            "PKG_CONFIG_PATH",
            "pkg-config --cflags --libs nwchemc",
            "nwchem-installed-pkgconfig-consumer",
            "nwchem-installed-cmake-consumer",
            "invalid-input ABI checks",
        ]
        missing = [term for term in required_terms if term not in docs]
        self.assertEqual(missing, [])


if __name__ == "__main__":
    raise SystemExit(unittest.main())
