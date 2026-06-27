#!/usr/bin/env python3
import os
import json
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

    def build_tree_skeleton(self, parent: Path) -> Path:
        root = parent / "nwchem-build"
        for rel in [
            "src/config",
            "src/tools/install/bin",
            "src/tools/install/include",
            "src/tools/install/lib",
            "src/include",
            "src/util",
            "src/libext/lib",
            "src/nwpw/nwpwlib/utilities",
            "lib/LINUX64",
        ]:
            (root / rel).mkdir(parents=True)
        (root / "src/config/nwchem_config.h").write_text(
            "NW_MODULE_LIBS =\n", encoding="utf-8"
        )
        ga_config = root / "src/tools/install/bin/ga-config"
        ga_config.write_text(
            "#!/bin/sh\n"
            "case \"$1\" in\n"
            "  --ldflags) echo ;;\n"
            "  --libs) echo ;;\n"
            "esac\n",
            encoding="utf-8",
        )
        ga_config.chmod(0o755)
        (root / "src/stubs.o").write_bytes(b"stub")
        (root / "src/nwpw/nwpwlib/utilities/btdb.F").write_text(
            "      subroutine btdb\n"
            "      return\n"
            "      end\n",
            encoding="utf-8",
        )
        return root

    def env(self, path_prefix: Path | None = None) -> dict[str, str]:
        env = os.environ.copy()
        env["TMPDIR"] = str(SCRATCH)
        if path_prefix is not None:
            env["PATH"] = str(path_prefix) + os.pathsep + env["PATH"]
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
            meson = os.environ.get("MESON", "meson")

            proc = subprocess.run(
                [
                    meson,
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

    def test_meson_static_embed_config_skips_shared_archive_probe(self):
        with TemporaryDirectory(dir=SCRATCH) as tmp_name:
            tmp = Path(tmp_name)
            root = self.build_tree_skeleton(tmp)
            build = tmp / "meson-static-build"
            meson = os.environ.get("MESON", "meson")

            proc = subprocess.run(
                [
                    meson,
                    "setup",
                    str(build),
                    "-Dwith_nwchem=true",
                    "-Dwith_tests=false",
                    "-Ddefault_library=static",
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

            self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)

    def test_meson_real_nwchem_tests_can_use_mpirun_launcher(self):
        with TemporaryDirectory(dir=SCRATCH) as tmp_name:
            tmp = Path(tmp_name)
            root = self.build_tree_skeleton(tmp)
            fake_bin = tmp / "fake-bin"
            fake_bin.mkdir()
            mpirun = fake_bin / "mpirun"
            mpirun.write_text("#!/bin/sh\nexec \"$@\"\n", encoding="utf-8")
            mpirun.chmod(0o755)
            build = tmp / "meson-mpi-test-build"
            meson = os.environ.get("MESON", "meson")
            env = self.env(fake_bin)

            proc = subprocess.run(
                [
                    meson,
                    "setup",
                    str(build),
                    "-Dwith_nwchem=true",
                    "-Dwith_tests=true",
                    "-Ddefault_library=static",
                    "-Dnwchem_test_mpi_ranks=2",
                    f"-Dnwchem_root={root}",
                    "-Dnwchem_target=LINUX64",
                ],
                cwd=ROOT,
                check=False,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                env=env,
            )
            self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)

            tests = subprocess.run(
                [meson, "introspect", "--tests", str(build)],
                check=True,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                env=env,
            )
            command = next(
                entry["cmd"]
                for entry in json.loads(tests.stdout)
                if entry["name"].endswith("nwchem-energy-gradient")
            )
            self.assertEqual(
                command[:3],
                [str(mpirun), "-np", "2"],
            )
            self.assertIn("test_nwchem_energy_gradient", command[3])

    def test_cmake_rejects_runtime_prefix_with_embed_guidance(self):
        with TemporaryDirectory(dir=SCRATCH) as tmp_name:
            tmp = Path(tmp_name)
            root = self.runtime_prefix(tmp)
            build = tmp / "cmake-build"
            cmake = os.environ.get("CMAKE", "cmake")

            proc = subprocess.run(
                [
                    cmake,
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

    def test_cmake_real_nwchem_tests_can_use_mpirun_launcher(self):
        with TemporaryDirectory(dir=SCRATCH) as tmp_name:
            tmp = Path(tmp_name)
            root = self.build_tree_skeleton(tmp)
            fake_bin = tmp / "fake-bin"
            fake_bin.mkdir()
            mpirun = fake_bin / "mpirun"
            mpirun.write_text("#!/bin/sh\nexec \"$@\"\n", encoding="utf-8")
            mpirun.chmod(0o755)
            build = tmp / "cmake-mpi-test-build"
            cmake = os.environ.get("CMAKE", "cmake")
            env = self.env(fake_bin)

            proc = subprocess.run(
                [
                    cmake,
                    "-S",
                    str(ROOT),
                    "-B",
                    str(build),
                    "-DNWCHEMC_WITH_NWCHEM=ON",
                    "-DNWCHEMC_BUILD_TESTS=ON",
                    "-DNWCHEMC_NWCHEM_TEST_MPI_RANKS=2",
                    f"-DNWCHEMC_NWCHEM_ROOT={root}",
                    "-DNWCHEMC_NWCHEM_TARGET=LINUX64",
                ],
                check=False,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                env=env,
            )
            self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)

            ctest_file = (build / "CTestTestfile.cmake").read_text(encoding="utf-8")
            self.assertIn(
                f'add_test([=[nwchem-energy-gradient]=] "{mpirun}" "-np" "2"',
                ctest_file,
            )


if __name__ == "__main__":
    raise SystemExit(unittest.main())
