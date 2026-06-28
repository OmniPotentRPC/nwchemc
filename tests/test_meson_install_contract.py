#!/usr/bin/env python3
import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
MESON = ROOT / "meson.build"


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


if __name__ == "__main__":
    raise SystemExit(unittest.main())
