#!/usr/bin/env python3
import argparse
import os
import shlex
import subprocess
import sys
from pathlib import Path


CONSUMER_MAIN = """\
#include "nwchemc.h"

#include <stdio.h>

int main(void) {
  const char *version = nwchemc_version();
  if (version == NULL || version[0] == '\\0') {
    return 2;
  }
  int available = nwchemc_available();
  printf("nwchemc %s available=%d\\n", version, available);
  nwchemc_finalize();
  return available ? 0 : 3;
}
"""


def run(command, env=None):
    print("+ " + " ".join(str(part) for part in command), flush=True)
    subprocess.run([str(part) for part in command], check=True, env=env)


def output(command, env=None):
    print("+ " + " ".join(str(part) for part in command), flush=True)
    return subprocess.check_output(
        [str(part) for part in command], text=True, env=env
    ).strip()


def write_consumer(source_dir):
    source_dir.mkdir(parents=True, exist_ok=True)
    (source_dir / "main.c").write_text(CONSUMER_MAIN, encoding="utf-8")


def pkg_config_env(install_prefix):
    env = os.environ.copy()
    pkg_dirs = [
        install_prefix / "lib64" / "pkgconfig",
        install_prefix / "lib" / "pkgconfig",
        install_prefix / "share" / "pkgconfig",
    ]
    existing = env.get("PKG_CONFIG_PATH", "")
    paths = [str(path) for path in pkg_dirs]
    if existing:
        paths.append(existing)
    env["PKG_CONFIG_PATH"] = os.pathsep.join(paths)
    return env


def setup_or_reconfigure(args, package_build):
    if (package_build / "build.ninja").exists():
        run([args.meson, "setup", "--reconfigure", package_build])
        return
    run(
        [
            args.meson,
            "setup",
            package_build,
            args.source_dir,
            f"--prefix={args.install_prefix}",
            "-Dwith_nwchem=true",
            f"-Dnwchem_root={args.nwchem_root}",
            f"-Dnwchem_target={args.nwchem_target}",
            "-Dwith_tests=false",
            "-Ddefault_library=static",
        ]
    )


def parse_args(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-dir", required=True, type=Path)
    parser.add_argument("--build-root", required=True, type=Path)
    parser.add_argument("--install-prefix", required=True, type=Path)
    parser.add_argument("--meson", default="meson")
    parser.add_argument("--cc", default="cc")
    parser.add_argument("--pkg-config", default="pkg-config")
    parser.add_argument("--nwchem-root", required=True, type=Path)
    parser.add_argument("--nwchem-target", default="LINUX64")
    parser.add_argument("--mpi-ranks", type=int, default=1)
    parser.add_argument("--mpirun", default="mpirun")
    parser.add_argument("--parallel", type=int, default=4)
    return parser.parse_args(argv)


def main(argv):
    args = parse_args(argv)
    build_root = args.build_root
    package_build = build_root / "package-build"
    consumer_source = build_root / "consumer-source"
    consumer = build_root / "consumer"

    build_root.mkdir(parents=True, exist_ok=True)
    write_consumer(consumer_source)

    setup_or_reconfigure(args, package_build)
    run([args.meson, "compile", "-C", package_build, "-j", str(args.parallel)])
    run([args.meson, "install", "-C", package_build])

    env = pkg_config_env(args.install_prefix)
    output([args.pkg_config, "--modversion", "nwchemc"], env=env)
    output([args.pkg_config, "--cflags", "--libs", "nwchemc"], env=env)
    cflags = shlex.split(output([args.pkg_config, "--cflags", "nwchemc"], env=env))
    libs = shlex.split(
        output([args.pkg_config, "--libs", "--static", "nwchemc"], env=env)
    )
    run(
        [args.cc, consumer_source / "main.c", "-o", consumer] + cflags + libs,
        env=env,
    )

    if args.mpi_ranks > 1:
        run([args.mpirun, "-np", str(args.mpi_ranks), consumer], env=env)
    else:
        run([consumer], env=env)


if __name__ == "__main__":
    main(sys.argv[1:])
