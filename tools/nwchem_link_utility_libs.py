#!/usr/bin/env python3
import argparse
import sys
from pathlib import Path


# Always-linked support archives from a standard NWChem binary link line
# (see src/config/makefile.h LIB_DEFINES / final nwchem link). Not always in
# NW_MODULE_LIBS for small module sets (nwdft driver stepper), but required
# for undefined refs from task_energy / geom / vectors when building shared
# libnwchemc.so.
ALWAYS_IF_PRESENT = (
    "cons",
    "bq",
    "64to32",
)

OPTIONAL_ARCHIVES = {
    "peigs": ("libpeigs.a", "libpeigs.so", "libpeigs.dylib"),
    "peigs_comm": ("libpeigs_comm.a", "libpeigs_comm.so", "libpeigs_comm.dylib"),
}


def utility_libs(root: Path, target: str) -> list[str]:
    libdir = root / "lib" / target
    libs = ["-lperfm"]
    for name in ALWAYS_IF_PRESENT:
        if (libdir / f"lib{name}.a").exists() or (libdir / f"lib{name}.so").exists():
            libs.append(f"-l{name}")
    for name, archive_names in OPTIONAL_ARCHIVES.items():
        if any((libdir / archive_name).exists() for archive_name in archive_names):
            libs.append(f"-l{name}")
    # USE_LIBXC builds leave nwc_xc / nwc_xcf03 under libext/libxc/install/lib.
    # nwdft objects (nwchem_libxc_*) need these on the shared libnwchemc link line.
    libxc_dir = root / "src" / "libext" / "libxc" / "install" / "lib"
    has_nwc = (libxc_dir / "libnwc_xc.a").exists() or (libxc_dir / "libnwc_xc.so").exists()
    has_plain = (libxc_dir / "libxc.a").exists() or (libxc_dir / "libxc.so").exists()
    if has_nwc or has_plain:
        libs = [f"-L{libxc_dir}"] + libs
        if has_nwc:
            libs.extend(["-lnwc_xcf03", "-lnwc_xc"])
        else:
            libs.extend(["-lxcf03", "-lxc"])
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
