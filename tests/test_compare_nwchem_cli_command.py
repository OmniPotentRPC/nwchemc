#!/usr/bin/env python3
import importlib.util
import unittest
from pathlib import Path


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


if __name__ == "__main__":
    raise SystemExit(unittest.main())
