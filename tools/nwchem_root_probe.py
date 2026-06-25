#!/usr/bin/env python3
import argparse
import sys
from pathlib import Path


REQUIRED_PATHS = (
    ("file", "src/config/nwchem_config.h"),
    ("file", "src/tools/install/bin/ga-config"),
    ("dir", "src/tools/install/include"),
    ("dir", "src/tools/install/lib"),
    ("dir", "src/include"),
    ("dir", "src/util"),
    ("dir", "src/libext/lib"),
    ("file", "src/stubs.o"),
)


def validate_root(root: Path, target: str) -> list[str]:
    missing: list[str] = []
    for kind, rel in REQUIRED_PATHS:
        path = root / rel
        if kind == "file" and not path.is_file():
            missing.append(rel)
        elif kind == "dir" and not path.is_dir():
            missing.append(rel)

    target_lib = Path("lib") / target
    if not (root / target_lib).is_dir():
        missing.append(str(target_lib))
    return missing


def looks_like_runtime_prefix(root: Path) -> bool:
    return (
        (root / "bin").is_dir()
        and (root / "lib").is_dir()
        and (root / "share" / "nwchem").is_dir()
    )


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(
        description="Validate an NWChem root for the in-process nwchemc embed ABI."
    )
    parser.add_argument("root", type=Path)
    parser.add_argument("--target", default="LINUX64")
    args = parser.parse_args(argv)

    root = args.root.resolve()
    missing = validate_root(root, args.target)
    if not missing:
        print(f"{root} contains the required NWChem embed build-tree artifacts")
        return 0

    print(
        f"{root} is not an NWChem build tree usable by the nwchemc embed ABI.",
        file=sys.stderr,
    )
    if looks_like_runtime_prefix(root):
        print(
            "The path looks like a runtime prefix; nwchemc embed mode needs "
            "a build tree with NWChem source headers, config metadata, "
            "Global Arrays tools, target libraries, and stubs.o.",
            file=sys.stderr,
        )
    print("Missing paths:", file=sys.stderr)
    for rel in missing:
        print(f"  - {rel}", file=sys.stderr)
    return 1


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
