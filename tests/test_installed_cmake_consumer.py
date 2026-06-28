#!/usr/bin/env python3
import argparse
import os
import subprocess
import sys
from pathlib import Path


CONSUMER_CMAKE = """\
cmake_minimum_required(VERSION 3.24)
project(nwchemc_consumer_smoke C)

find_package(nwchemc CONFIG REQUIRED)

add_executable(consumer main.c)
target_link_libraries(consumer PRIVATE nwchemc::nwchemc)
"""


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


def write_consumer(source_dir):
    source_dir.mkdir(parents=True, exist_ok=True)
    (source_dir / "CMakeLists.txt").write_text(CONSUMER_CMAKE, encoding="utf-8")
    (source_dir / "main.c").write_text(CONSUMER_MAIN, encoding="utf-8")


def parse_args(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-dir", required=True, type=Path)
    parser.add_argument("--build-root", required=True, type=Path)
    parser.add_argument("--install-prefix", required=True, type=Path)
    parser.add_argument("--cmake", default="cmake")
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
    consumer_build = build_root / "consumer-build"
    install_prefix = args.install_prefix

    build_root.mkdir(parents=True, exist_ok=True)
    write_consumer(consumer_source)

    run(
        [
            args.cmake,
            "-S",
            args.source_dir,
            "-B",
            package_build,
            "-DNWCHEMC_BUILD_TESTS=OFF",
            "-DNWCHEMC_WITH_NWCHEM=ON",
            "-DNWCHEMC_BUILD_SHARED=OFF",
            f"-DNWCHEMC_NWCHEM_ROOT={args.nwchem_root}",
            f"-DNWCHEMC_NWCHEM_TARGET={args.nwchem_target}",
        ]
    )
    run([args.cmake, "--build", package_build, "--parallel", str(args.parallel)])
    run([args.cmake, "--install", package_build, "--prefix", install_prefix])

    run(
        [
            args.cmake,
            "-S",
            consumer_source,
            "-B",
            consumer_build,
            f"-DCMAKE_PREFIX_PATH={install_prefix}",
        ]
    )
    run([args.cmake, "--build", consumer_build, "--parallel", str(args.parallel)])

    consumer = consumer_build / "consumer"
    env = os.environ.copy()
    if args.mpi_ranks > 1:
        run([args.mpirun, "-np", str(args.mpi_ranks), consumer], env=env)
    else:
        run([consumer], env=env)


if __name__ == "__main__":
    main(sys.argv[1:])
