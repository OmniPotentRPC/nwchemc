#!/usr/bin/env python3
import importlib.util
import unittest
from pathlib import Path
from tempfile import TemporaryDirectory


ROOT = Path(__file__).resolve().parents[1]
MODULE_PATH = ROOT / "tools" / "compare_nwchem_cli.py"


def load_compare():
    spec = importlib.util.spec_from_file_location("compare_nwchem_cli", MODULE_PATH)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(module)
    return module


class CompareNWChemCommandTest(unittest.TestCase):
    def test_split_command_preserves_launcher_arguments(self):
        compare = load_compare()

        command = compare.split_command(
            "mpirun -np 2 /opt/nwchem/bin/nwchem"
        )

        self.assertEqual(
            command,
            ["mpirun", "-np", "2", "/opt/nwchem/bin/nwchem"],
        )

    def test_split_command_keeps_quoted_executable(self):
        compare = load_compare()

        command = compare.split_command("'/opt/nw chem/bin/nwchem' -v")

        self.assertEqual(command, ["/opt/nw chem/bin/nwchem", "-v"])

    def test_format_command_round_trips_shell_quoting(self):
        compare = load_compare()

        text = compare.format_command(["/opt/nw chem/bin/nwchem", "h2.nw"])

        self.assertEqual(text, "'/opt/nw chem/bin/nwchem' h2.nw")

    def test_embed_launch_command_wraps_binary_and_params(self):
        compare = load_compare()

        command = compare.embed_launch_command(
            ["/opt/mpi/bin/mpirun", "-np", "2"],
            Path("/tmp/build/test_nwchem_energy_gradient"),
            Path("/tmp/build/nwchem_params.bin"),
        )

        self.assertEqual(
            command,
            [
                "/opt/mpi/bin/mpirun",
                "-np",
                "2",
                "/tmp/build/test_nwchem_energy_gradient",
                "/tmp/build/nwchem_params.bin",
            ],
        )

    def test_find_params_bin_prefers_real_nwchem_fixture(self):
        compare = load_compare()

        with TemporaryDirectory(prefix="nwchemc-compare-") as tmp_name:
            build = Path(tmp_name)
            real_params = build / "nwchem_params.bin"
            parser_params = build / "nwchem_parser_params.bin"
            real_params.write_bytes(b"real")
            parser_params.write_bytes(b"parser")

            self.assertEqual(compare.find_params_bin(build), real_params)

    def test_parse_gradient_uses_gradient_columns_from_nwchem_table(self):
        compare = load_compare()

        text = """
                         RHF ENERGY GRADIENTS

    atom               coordinates                        gradient
                 x          y          z           x          y          z
   1 H       0.000000   0.000000  -0.700521    0.000000   0.000000  -0.028956
   2 H       0.000000   0.000000   0.700521    0.000000   0.000000   0.028956
"""

        self.assertEqual(
            compare.parse_gradient(text),
            [0.0, 0.0, -0.028956, 0.0, 0.0, 0.028956],
        )


if __name__ == "__main__":
    raise SystemExit(unittest.main())
