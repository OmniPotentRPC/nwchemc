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
    "nwchem-brillouin-dos-zones-rtdb",
    "nwchem-installed-cmake-consumer",
    "nwchem-installed-pkgconfig-consumer",
]


def listed_meson_tests(meson: str, build_dir: str) -> set[str]:
    """Return test names registered in the Meson build (short names only)."""
    proc = subprocess.run(
        [meson, "test", "-C", build_dir, "--list"],
        check=False,
        capture_output=True,
        text=True,
    )
    names: set[str] = set()
    for line in (proc.stdout or "").splitlines():
        line = line.strip()
        if not line:
            continue
        # Meson --list prints "project:name" or suite-prefixed forms.
        name = line.split(":")[-1].strip()
        names.add(name)
        names.add(line)
    return names


def rgpot_gate_command(meson: str, build_dir: str, num_processes: int) -> list[str]:
    available = listed_meson_tests(meson, build_dir)
    selected = [name for name in RGPOT_RELEASE_TESTS if name in available]
    missing = [name for name in RGPOT_RELEASE_TESTS if name not in available]
    if missing:
        raise SystemExit(
            "rgpot release gate tests missing from this build "
            f"(renamed or not configured): {', '.join(missing)}"
        )
    return [
        meson,
        "test",
        "-C",
        build_dir,
        "--print-errorlogs",
        "--num-processes",
        str(num_processes),
        *selected,
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
