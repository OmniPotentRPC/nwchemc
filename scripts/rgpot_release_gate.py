#!/usr/bin/env python3
import argparse
import shlex
import subprocess


RGPOT_RELEASE_TESTS = [
    "readme-abi-surface",
    "meson-install-contract",
    "cmake-real-nwchem-contract",
    "nwchem-rgpot-smoke",
    "nwchem-session-result",
    "nwchem-hessian",
    "nwchem-stress",
    "nwchem-pspw-pseudopotential-forces",
    "nwchem-potential-config-pseudopotential",
    "nwchem-pseudopotential-rtdb",
    "nwchem-forceinput-cell-rtdb",
    "nwchem-configured-nwpw-rtdb",
    "nwchem-installed-cmake-consumer",
    "nwchem-installed-pkgconfig-consumer",
]


def rgpot_gate_command(meson: str, build_dir: str, num_processes: int) -> list[str]:
    return [
        meson,
        "test",
        "-C",
        build_dir,
        "--print-errorlogs",
        "--num-processes",
        str(num_processes),
        *RGPOT_RELEASE_TESTS,
    ]


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Run the nwchemc real-NWChem rgpot release gate."
    )
    parser.add_argument(
        "--build-dir",
        default="build-nwchem",
        help="Meson build directory linked against the release NWChem build.",
    )
    parser.add_argument(
        "--meson",
        default="meson",
        help="Meson executable to run.",
    )
    parser.add_argument(
        "--num-processes",
        default=1,
        type=int,
        help="Meson test process count. Keep this at 1 for shared NWChem state.",
    )
    parser.add_argument(
        "--list-tests",
        action="store_true",
        help="Print the rgpot release gate test names and exit.",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Print the Meson command without executing it.",
    )
    args = parser.parse_args()

    if args.num_processes < 1:
        parser.error("--num-processes must be positive")

    if args.list_tests:
        for name in RGPOT_RELEASE_TESTS:
            print(name)
        return 0

    command = rgpot_gate_command(args.meson, args.build_dir, args.num_processes)
    if args.dry_run:
        print(shlex.join(command))
        return 0

    return subprocess.run(command, check=False).returncode


if __name__ == "__main__":
    raise SystemExit(main())
