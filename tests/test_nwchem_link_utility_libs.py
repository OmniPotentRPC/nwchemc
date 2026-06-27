#!/usr/bin/env python3
import importlib.util
import subprocess
import sys
import unittest
from pathlib import Path
from tempfile import TemporaryDirectory


ROOT = Path(__file__).resolve().parents[1]
MODULE_PATH = ROOT / "tools" / "nwchem_link_utility_libs.py"


def load_link_libs():
    spec = importlib.util.spec_from_file_location("nwchem_link_utility_libs", MODULE_PATH)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(module)
    return module


class NWChemLinkUtilityLibsTest(unittest.TestCase):
    def test_omits_absent_peigs_comm_archive(self):
        link_libs = load_link_libs()
        with TemporaryDirectory(prefix="nwchemc-link-libs-") as root_name:
            root = Path(root_name)
            (root / "lib/LINUX64").mkdir(parents=True)

            self.assertEqual(link_libs.utility_libs(root, "LINUX64"), ["-lperfm"])

    def test_includes_present_peigs_comm_archive(self):
        link_libs = load_link_libs()
        with TemporaryDirectory(prefix="nwchemc-link-libs-") as root_name:
            root = Path(root_name)
            libdir = root / "lib/LINUX64"
            libdir.mkdir(parents=True)
            (libdir / "libpeigs_comm.a").write_bytes(b"archive")

            self.assertEqual(
                link_libs.utility_libs(root, "LINUX64"),
                ["-lperfm", "-lpeigs_comm"],
            )

    def test_cli_emits_shell_split_list(self):
        with TemporaryDirectory(prefix="nwchemc-link-libs-") as root_name:
            root = Path(root_name)
            libdir = root / "lib/LINUX64"
            libdir.mkdir(parents=True)
            (libdir / "libpeigs_comm.so").write_bytes(b"shared")

            proc = subprocess.run(
                [
                    sys.executable,
                    str(MODULE_PATH),
                    str(root),
                    "--target",
                    "LINUX64",
                ],
                check=True,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            )

            self.assertEqual(proc.stdout.strip(), "-lperfm -lpeigs_comm")


if __name__ == "__main__":
    raise SystemExit(unittest.main())
