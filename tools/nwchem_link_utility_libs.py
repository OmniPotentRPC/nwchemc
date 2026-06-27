#!/usr/bin/env python3
import argparse
import sys
from pathlib import Path


OPTIONAL_ARCHIVES = {
    "peigs": ("libpeigs.a", "libpeigs.so", "libpeigs.dylib"),
    "peigs_comm": ("libpeigs_comm.a", "libpeigs_comm.so", "libpeigs_comm.dylib"),
}


def utility_libs(root: Path, target: str) -> list[str]:
    libdir = root / "lib" / target
    libs = ["-lperfm"]
    for name, archive_names in OPTIONAL_ARCHIVES.items():
        if any((libdir / archive_name).exists() for archive_name in archive_names):
            libs.append(f"-l{name}")
    return libs


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(
        description="Emit NWChem utility libraries available in a build tree."
    )
    parser.add_argument("root", type=Path)
    parser.add_argument("--target", default="LINUX64")
    args = parser.parse_args(argv)

    print(" ".join(utility_libs(args.root.resolve(), args.target)))
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
