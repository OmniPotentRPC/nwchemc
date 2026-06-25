#!/usr/bin/env python3
import os
import subprocess
import unittest
from pathlib import Path
from tempfile import TemporaryDirectory


ROOT = Path(__file__).resolve().parents[1]
SCRATCH = Path(
    os.environ.get("NWCHEMC_TEST_SCRATCH_ROOT", ROOT / ".build-tmp" / "probe-tests")
)


class BuildEmbedRootProbeTest(unittest.TestCase):
    def setUp(self):
        SCRATCH.mkdir(parents=True, exist_ok=True)

    def runtime_prefix(self, parent: Path) -> Path:
        root = parent / "runtime-prefix"
        for rel in ["bin", "lib", "share/nwchem"]:
            (root / rel).mkdir(parents=True)
        (root / "bin/nwchem").write_text("#!/bin/sh\n")
        return root

    def env(self) -> dict[str, str]:
        env = os.environ.copy()
        env["TMPDIR"] = str(SCRATCH)
        return env

    def assert_probe_message(self, proc: subprocess.CompletedProcess[str]):
        combined = proc.stdout + proc.stderr
        self.assertNotEqual(proc.returncode, 0, combined)
        self.assertIn("runtime prefix", combined)
        self.assertIn("build tree", combined)

    def test_meson_rejects_runtime_prefix_with_embed_guidance(self):
        with TemporaryDirectory(dir=SCRATCH) as tmp_name:
            tmp = Path(tmp_name)
            root = self.runtime_prefix(tmp)
            build = tmp / "meson-build"

            proc = subprocess.run(
                [
                    "meson",
                    "setup",
                    str(build),
                    "-Dwith_nwchem=true",
                    "-Dwith_tests=false",
                    f"-Dnwchem_root={root}",
                    "-Dnwchem_target=LINUX64",
                ],
                cwd=ROOT,
                check=False,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                env=self.env(),
            )

            self.assert_probe_message(proc)

    def test_cmake_rejects_runtime_prefix_with_embed_guidance(self):
        with TemporaryDirectory(dir=SCRATCH) as tmp_name:
            tmp = Path(tmp_name)
            root = self.runtime_prefix(tmp)
            build = tmp / "cmake-build"

            proc = subprocess.run(
                [
                    "cmake",
                    "-S",
                    str(ROOT),
                    "-B",
                    str(build),
                    "-DNWCHEMC_WITH_NWCHEM=ON",
                    "-DNWCHEMC_BUILD_TESTS=OFF",
                    f"-DNWCHEMC_NWCHEM_ROOT={root}",
                    "-DNWCHEMC_NWCHEM_TARGET=LINUX64",
                ],
                check=False,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                env=self.env(),
            )

            self.assert_probe_message(proc)


if __name__ == "__main__":
    raise SystemExit(unittest.main())
