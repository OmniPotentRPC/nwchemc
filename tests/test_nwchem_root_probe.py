#!/usr/bin/env python3
import importlib.util
import subprocess
import sys
import unittest
from pathlib import Path
from tempfile import TemporaryDirectory


ROOT = Path(__file__).resolve().parents[1]
MODULE_PATH = ROOT / "tools" / "nwchem_root_probe.py"


def load_probe():
    spec = importlib.util.spec_from_file_location("nwchem_root_probe", MODULE_PATH)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(module)
    return module


class NWChemRootProbeTest(unittest.TestCase):
    def test_accepts_build_tree_skeleton(self):
        probe = load_probe()
        with TemporaryDirectory(prefix="nwchemc-root-probe-") as root_name:
            root = Path(root_name)
            for rel in [
                "src/config",
                "src/tools/install/bin",
                "src/tools/install/include",
                "src/tools/install/lib",
                "src/include",
                "src/util",
                "src/libext/lib",
                "lib/LINUX64",
            ]:
                (root / rel).mkdir(parents=True)
            (root / "src/config/nwchem_config.h").write_text(
                "NW_MODULE_LIBS = -lnwdft\n"
            )
            (root / "src/tools/install/bin/ga-config").write_text("#!/bin/sh\n")
            (root / "src/stubs.o").write_bytes(b"stub")

            result = probe.validate_root(root, "LINUX64")

            self.assertEqual(result, [])

    def test_rejects_runtime_prefix_without_embed_artifacts(self):
        probe = load_probe()
        with TemporaryDirectory(prefix="nwchemc-root-probe-") as root_name:
            root = Path(root_name)
            for rel in ["bin", "lib", "share/nwchem"]:
                (root / rel).mkdir(parents=True)
            (root / "bin/nwchem").write_text("#!/bin/sh\n")
            (root / "lib/libnwchem.a").write_bytes(b"archive")

            result = probe.validate_root(root, "LINUX64")

            self.assertIn("src/config/nwchem_config.h", "\n".join(result))
            self.assertIn("src/stubs.o", "\n".join(result))

    def test_cli_reports_conda_prefix_guidance(self):
        with TemporaryDirectory(prefix="nwchemc-root-probe-") as root_name:
            root = Path(root_name)
            (root / "bin").mkdir()
            (root / "lib").mkdir()
            (root / "share/nwchem").mkdir(parents=True)

            proc = subprocess.run(
                [
                    sys.executable,
                    str(MODULE_PATH),
                    str(root),
                    "--target",
                    "LINUX64",
                ],
                check=False,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            )

            self.assertNotEqual(proc.returncode, 0)
            self.assertIn("runtime prefix", proc.stderr)
            self.assertIn("build tree", proc.stderr)


if __name__ == "__main__":
    raise SystemExit(unittest.main())
