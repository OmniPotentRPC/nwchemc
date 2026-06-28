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
            "nwchem-installed-cmake-consumer",
        ]
        missing = [term for term in required_terms if term not in docs]
        self.assertEqual(missing, [])


if __name__ == "__main__":
    raise SystemExit(unittest.main())
