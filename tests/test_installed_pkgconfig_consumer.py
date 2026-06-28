#!/usr/bin/env python3
import argparse
import os
import shlex
import subprocess
import sys
from pathlib import Path


CONSUMER_MAIN = """\
#include "nwchemc.h"

#include <stddef.h>
#include <stdio.h>

typedef size_t (*PotentialResultSizeFn)(const void *, size_t);
typedef NWChemCResult (*PotentialResultConfigFn)(
    const void *, size_t, const void *, size_t, void *, size_t, size_t *);
typedef NWChemCResult (*PotentialConfigEnergyFn)(
    const void *, size_t, const void *, size_t);
typedef NWChemCResult (*PotentialConfigBufferFn)(
    const void *, size_t, const void *, size_t, double *, size_t);
typedef NWChemCResult (*PotentialConfigFrequenciesFn)(
    const void *, size_t, const void *, size_t, double *, size_t, double *,
    size_t);
typedef NWChemCResult (*SessionEnergyFn)(NWChemCSession *, const void *,
                                         size_t);
typedef NWChemCResult (*SessionBufferFn)(NWChemCSession *, const void *,
                                         size_t, double *, size_t);
typedef NWChemCResult (*SessionFrequenciesFn)(NWChemCSession *, const void *,
                                              size_t, double *, size_t,
                                              double *, size_t);
typedef NWChemCResult (*SessionResultFn)(NWChemCSession *, const void *,
                                         size_t, void *, size_t, size_t *);
typedef int (*ConfigFn)(const void *, size_t);
typedef NWChemCSession *(*SessionCreateFn)(const void *, size_t);
typedef int (*SessionConfigFn)(NWChemCSession *, const void *, size_t);
typedef void (*SessionDestroyFn)(NWChemCSession *);
typedef NWChemCResult (*CoordinateEnergyFn)(int, const double *, const int *,
                                            const void *, size_t);
typedef NWChemCResult (*CoordinateBufferFn)(int, const double *, const int *,
                                            const void *, size_t, double *);
typedef NWChemCResult (*CoordinateFrequenciesFn)(
    int, const double *, const int *, const void *, size_t, double *,
    double *);
typedef NWChemCResult (*SessionCoordinateEnergyFn)(
    NWChemCSession *, int, const double *, const int *);
typedef NWChemCResult (*SessionCoordinateBufferFn)(
    NWChemCSession *, int, const double *, const int *, double *);
typedef NWChemCResult (*SessionCoordinateFrequenciesFn)(
    NWChemCSession *, int, const double *, const int *, double *, double *);

static int exercise_rgpot_result_abi(void) {
  PotentialResultSizeFn size_fns[] = {
      nwchemc_potential_result_size_for_force_input,
      nwchemc_energy_result_size_for_force_input,
      nwchemc_forces_result_size_for_force_input,
      nwchemc_hessian_result_size_for_force_input,
      nwchemc_dipole_result_size_for_force_input,
      nwchemc_polarizability_result_size_for_force_input,
      nwchemc_quadrupole_result_size_for_force_input,
      nwchemc_stress_result_size_for_force_input,
      nwchemc_optimize_result_size_for_force_input,
      nwchemc_frequencies_result_size_for_force_input,
  };
  PotentialResultConfigFn result_fns[] = {
      nwchemc_calculate_result_from_config,
      nwchemc_calculate_energy_result_from_config,
      nwchemc_calculate_forces_result_from_config,
      nwchemc_calculate_hessian_result_from_config,
      nwchemc_calculate_dipole_result_from_config,
      nwchemc_calculate_polarizability_result_from_config,
      nwchemc_calculate_quadrupole_result_from_config,
      nwchemc_calculate_stress_result_from_config,
      nwchemc_calculate_optimize_result_from_config,
      nwchemc_calculate_frequencies_result_from_config,
  };
  size_t index;

  for (index = 0; index < sizeof(size_fns) / sizeof(size_fns[0]); ++index) {
    if (size_fns[index] == NULL) {
      return 4;
    }
  }
  for (index = 0; index < sizeof(result_fns) / sizeof(result_fns[0]); ++index) {
    if (result_fns[index] == NULL) {
      return 4;
    }
  }
  return 0;
}

static int exercise_rgpot_raw_and_session_abi(void) {
  PotentialConfigEnergyFn config_energy_fns[] = {
      nwchemc_calculate_energy_from_config,
  };
  PotentialConfigBufferFn config_buffer_fns[] = {
      nwchemc_calculate_forces_from_config,
      nwchemc_calculate_hessian_from_config,
      nwchemc_calculate_dipole_from_config,
      nwchemc_calculate_polarizability_from_config,
      nwchemc_calculate_quadrupole_from_config,
      nwchemc_calculate_stress_from_config,
      nwchemc_calculate_optimize_from_config,
  };
  PotentialConfigFrequenciesFn config_frequency_fns[] = {
      nwchemc_calculate_frequencies_from_config,
  };
  SessionEnergyFn session_energy_fns[] = {
      nwchemc_session_calculate_energy,
  };
  SessionBufferFn session_buffer_fns[] = {
      nwchemc_session_calculate_forces,
      nwchemc_session_calculate_hessian,
      nwchemc_session_calculate_dipole,
      nwchemc_session_calculate_polarizability,
      nwchemc_session_calculate_quadrupole,
      nwchemc_session_calculate_stress,
      nwchemc_session_calculate_optimize,
  };
  SessionFrequenciesFn session_frequency_fns[] = {
      nwchemc_session_calculate_frequencies,
  };
  SessionResultFn session_result_fns[] = {
      nwchemc_session_calculate_energy_result,
      nwchemc_session_calculate_forces_result,
      nwchemc_session_calculate_result,
      nwchemc_session_calculate_hessian_result,
      nwchemc_session_calculate_dipole_result,
      nwchemc_session_calculate_polarizability_result,
      nwchemc_session_calculate_quadrupole_result,
      nwchemc_session_calculate_stress_result,
      nwchemc_session_calculate_optimize_result,
      nwchemc_session_calculate_frequencies_result,
  };
  size_t index;

  for (index = 0;
       index < sizeof(config_energy_fns) / sizeof(config_energy_fns[0]);
       ++index) {
    if (config_energy_fns[index] == NULL) {
      return 4;
    }
  }
  for (index = 0;
       index < sizeof(config_buffer_fns) / sizeof(config_buffer_fns[0]);
       ++index) {
    if (config_buffer_fns[index] == NULL) {
      return 4;
    }
  }
  for (index = 0;
       index < sizeof(config_frequency_fns) / sizeof(config_frequency_fns[0]);
       ++index) {
    if (config_frequency_fns[index] == NULL) {
      return 4;
    }
  }
  for (index = 0;
       index < sizeof(session_energy_fns) / sizeof(session_energy_fns[0]);
       ++index) {
    if (session_energy_fns[index] == NULL) {
      return 4;
    }
  }
  for (index = 0;
       index < sizeof(session_buffer_fns) / sizeof(session_buffer_fns[0]);
       ++index) {
    if (session_buffer_fns[index] == NULL) {
      return 4;
    }
  }
  for (index = 0;
       index < sizeof(session_frequency_fns) / sizeof(session_frequency_fns[0]);
       ++index) {
    if (session_frequency_fns[index] == NULL) {
      return 4;
    }
  }
  for (index = 0;
       index < sizeof(session_result_fns) / sizeof(session_result_fns[0]);
       ++index) {
    if (session_result_fns[index] == NULL) {
      return 4;
    }
  }
  return 0;
}

static int exercise_coordinate_session_abi(void) {
  ConfigFn config_fns[] = {
      nwchemc_set_params,
      nwchemc_configure,
  };
  CoordinateEnergyFn coordinate_energy_fns[] = {
      nwchemc_energy,
      nwchemc_energy_from_config,
  };
  CoordinateBufferFn coordinate_buffer_fns[] = {
      nwchemc_energy_gradient,
      nwchemc_energy_forces,
      nwchemc_hessian,
      nwchemc_dipole,
      nwchemc_polarizability,
      nwchemc_quadrupole,
      nwchemc_stress,
      nwchemc_optimize,
      nwchemc_energy_gradient_from_config,
      nwchemc_energy_forces_from_config,
      nwchemc_hessian_from_config,
      nwchemc_dipole_from_config,
      nwchemc_polarizability_from_config,
      nwchemc_quadrupole_from_config,
      nwchemc_stress_from_config,
      nwchemc_optimize_from_config,
  };
  CoordinateFrequenciesFn coordinate_frequency_fns[] = {
      nwchemc_frequencies,
      nwchemc_frequencies_from_config,
  };
  SessionCreateFn session_create_fns[] = {
      nwchemc_session_create,
      nwchemc_session_create_from_config,
  };
  SessionConfigFn session_config_fns[] = {
      nwchemc_session_set_params,
      nwchemc_session_configure,
  };
  SessionDestroyFn session_destroy_fns[] = {
      nwchemc_session_destroy,
  };
  SessionCoordinateEnergyFn session_energy_fns[] = {
      nwchemc_session_energy,
  };
  SessionCoordinateBufferFn session_buffer_fns[] = {
      nwchemc_session_energy_gradient,
      nwchemc_session_energy_forces,
      nwchemc_session_dipole,
      nwchemc_session_polarizability,
      nwchemc_session_quadrupole,
      nwchemc_session_optimize,
      nwchemc_session_stress,
      nwchemc_session_hessian,
  };
  SessionCoordinateFrequenciesFn session_frequency_fns[] = {
      nwchemc_session_frequencies,
  };
  size_t index;

  for (index = 0; index < sizeof(config_fns) / sizeof(config_fns[0]);
       ++index) {
    if (config_fns[index] == NULL) {
      return 4;
    }
  }
  for (index = 0;
       index < sizeof(coordinate_energy_fns) / sizeof(coordinate_energy_fns[0]);
       ++index) {
    if (coordinate_energy_fns[index] == NULL) {
      return 4;
    }
  }
  for (index = 0;
       index < sizeof(coordinate_buffer_fns) / sizeof(coordinate_buffer_fns[0]);
       ++index) {
    if (coordinate_buffer_fns[index] == NULL) {
      return 4;
    }
  }
  for (index = 0;
       index <
           sizeof(coordinate_frequency_fns) / sizeof(coordinate_frequency_fns[0]);
       ++index) {
    if (coordinate_frequency_fns[index] == NULL) {
      return 4;
    }
  }
  for (index = 0;
       index < sizeof(session_create_fns) / sizeof(session_create_fns[0]);
       ++index) {
    if (session_create_fns[index] == NULL) {
      return 4;
    }
  }
  for (index = 0;
       index < sizeof(session_config_fns) / sizeof(session_config_fns[0]);
       ++index) {
    if (session_config_fns[index] == NULL) {
      return 4;
    }
  }
  for (index = 0;
       index < sizeof(session_destroy_fns) / sizeof(session_destroy_fns[0]);
       ++index) {
    if (session_destroy_fns[index] == NULL) {
      return 4;
    }
  }
  for (index = 0;
       index < sizeof(session_energy_fns) / sizeof(session_energy_fns[0]);
       ++index) {
    if (session_energy_fns[index] == NULL) {
      return 4;
    }
  }
  for (index = 0;
       index < sizeof(session_buffer_fns) / sizeof(session_buffer_fns[0]);
       ++index) {
    if (session_buffer_fns[index] == NULL) {
      return 4;
    }
  }
  for (index = 0;
       index < sizeof(session_frequency_fns) / sizeof(session_frequency_fns[0]);
       ++index) {
    if (session_frequency_fns[index] == NULL) {
      return 4;
    }
  }
  return 0;
}

int main(void) {
  int abi_status = exercise_rgpot_result_abi();
  if (abi_status != 0) {
    return abi_status;
  }
  abi_status = exercise_rgpot_raw_and_session_abi();
  if (abi_status != 0) {
    return abi_status;
  }
  abi_status = exercise_coordinate_session_abi();
  if (abi_status != 0) {
    return abi_status;
  }
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


def setup_package(args, package_build):
    command = [
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
    if package_build.exists():
        command.append("--wipe")
    run(command)


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

    setup_package(args, package_build)
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
