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

static int expect_result_failure(const char *label, NWChemCResult result) {
  if (result.ok != 0) {
    fprintf(stderr, "%s accepted invalid input\\n", label);
    return 5;
  }
  return 0;
}

static int expect_status_failure(const char *label, int status) {
  if (status == 0) {
    fprintf(stderr, "%s accepted invalid input\\n", label);
    return 6;
  }
  return 0;
}

static int expect_size_zero(const char *label, size_t value) {
  if (value != 0) {
    fprintf(stderr, "%s returned nonzero invalid-input size\\n", label);
    return 7;
  }
  return 0;
}

static int expect_null_session(const char *label, NWChemCSession *session) {
  if (session != NULL) {
    fprintf(stderr, "%s accepted invalid input\\n", label);
    nwchemc_session_destroy(session);
    return 8;
  }
  return 0;
}

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
  unsigned char result_bytes[1] = {0};
  size_t result_size = 0;
  size_t index;

  for (index = 0; index < sizeof(size_fns) / sizeof(size_fns[0]); ++index) {
    if (size_fns[index] == NULL) {
      return 4;
    }
    int status = expect_size_zero("result size", size_fns[index](NULL, 0));
    if (status != 0) {
      return status;
    }
  }
  for (index = 0; index < sizeof(result_fns) / sizeof(result_fns[0]); ++index) {
    if (result_fns[index] == NULL) {
      return 4;
    }
    result_size = 0;
    int status = expect_result_failure(
        "result function",
        result_fns[index](NULL, 0, NULL, 0, result_bytes,
                          sizeof(result_bytes), &result_size));
    if (status != 0) {
      return status;
    }
  }
  return 0;
}

static int exercise_rgpot_raw_and_session_abi(void) {
  PotentialConfigEnergyFn params_energy_fns[] = {
      nwchemc_calculate_energy,
  };
  PotentialConfigBufferFn params_buffer_fns[] = {
      nwchemc_calculate_forces,
      nwchemc_calculate_hessian,
      nwchemc_calculate_dipole,
      nwchemc_calculate_polarizability,
      nwchemc_calculate_quadrupole,
      nwchemc_calculate_stress,
      nwchemc_calculate_optimize,
  };
  PotentialConfigFrequenciesFn params_frequency_fns[] = {
      nwchemc_calculate_frequencies,
  };
  PotentialResultConfigFn params_result_fns[] = {
      nwchemc_calculate_result,
      nwchemc_calculate_energy_result,
      nwchemc_calculate_forces_result,
      nwchemc_calculate_hessian_result,
      nwchemc_calculate_dipole_result,
      nwchemc_calculate_polarizability_result,
      nwchemc_calculate_quadrupole_result,
      nwchemc_calculate_stress_result,
      nwchemc_calculate_optimize_result,
      nwchemc_calculate_frequencies_result,
  };
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
  double doubles[36] = {0.0};
  unsigned char result_bytes[1] = {0};
  size_t result_size = 0;
  size_t index;

  for (index = 0; index < sizeof(params_energy_fns) / sizeof(params_energy_fns[0]);
       ++index) {
    if (params_energy_fns[index] == NULL) {
      return 4;
    }
    int status = expect_result_failure(
        "params energy function", params_energy_fns[index](NULL, 0, NULL, 0));
    if (status != 0) {
      return status;
    }
  }
  for (index = 0; index < sizeof(params_buffer_fns) / sizeof(params_buffer_fns[0]);
       ++index) {
    if (params_buffer_fns[index] == NULL) {
      return 4;
    }
    int status = expect_result_failure(
        "params buffer function",
        params_buffer_fns[index](NULL, 0, NULL, 0, doubles, 36));
    if (status != 0) {
      return status;
    }
  }
  for (index = 0;
       index < sizeof(params_frequency_fns) / sizeof(params_frequency_fns[0]);
       ++index) {
    if (params_frequency_fns[index] == NULL) {
      return 4;
    }
    int status = expect_result_failure(
        "params frequency function",
        params_frequency_fns[index](NULL, 0, NULL, 0, doubles, 36, doubles,
                                    36));
    if (status != 0) {
      return status;
    }
  }
  for (index = 0; index < sizeof(params_result_fns) / sizeof(params_result_fns[0]);
       ++index) {
    if (params_result_fns[index] == NULL) {
      return 4;
    }
    result_size = 0;
    int status = expect_result_failure(
        "params result function",
        params_result_fns[index](NULL, 0, NULL, 0, result_bytes,
                                 sizeof(result_bytes), &result_size));
    if (status != 0) {
      return status;
    }
  }
  for (index = 0;
       index < sizeof(config_energy_fns) / sizeof(config_energy_fns[0]);
       ++index) {
    if (config_energy_fns[index] == NULL) {
      return 4;
    }
    int status = expect_result_failure(
        "config energy function", config_energy_fns[index](NULL, 0, NULL, 0));
    if (status != 0) {
      return status;
    }
  }
  for (index = 0;
       index < sizeof(config_buffer_fns) / sizeof(config_buffer_fns[0]);
       ++index) {
    if (config_buffer_fns[index] == NULL) {
      return 4;
    }
    int status = expect_result_failure(
        "config buffer function",
        config_buffer_fns[index](NULL, 0, NULL, 0, doubles, 36));
    if (status != 0) {
      return status;
    }
  }
  for (index = 0;
       index < sizeof(config_frequency_fns) / sizeof(config_frequency_fns[0]);
       ++index) {
    if (config_frequency_fns[index] == NULL) {
      return 4;
    }
    int status = expect_result_failure(
        "config frequency function",
        config_frequency_fns[index](NULL, 0, NULL, 0, doubles, 36, doubles,
                                    36));
    if (status != 0) {
      return status;
    }
  }
  for (index = 0;
       index < sizeof(session_energy_fns) / sizeof(session_energy_fns[0]);
       ++index) {
    if (session_energy_fns[index] == NULL) {
      return 4;
    }
    int status = expect_result_failure(
        "session energy function", session_energy_fns[index](NULL, NULL, 0));
    if (status != 0) {
      return status;
    }
  }
  for (index = 0;
       index < sizeof(session_buffer_fns) / sizeof(session_buffer_fns[0]);
       ++index) {
    if (session_buffer_fns[index] == NULL) {
      return 4;
    }
    int status = expect_result_failure(
        "session buffer function",
        session_buffer_fns[index](NULL, NULL, 0, doubles, 36));
    if (status != 0) {
      return status;
    }
  }
  for (index = 0;
       index < sizeof(session_frequency_fns) / sizeof(session_frequency_fns[0]);
       ++index) {
    if (session_frequency_fns[index] == NULL) {
      return 4;
    }
    int status = expect_result_failure(
        "session frequency function",
        session_frequency_fns[index](NULL, NULL, 0, doubles, 36, doubles, 36));
    if (status != 0) {
      return status;
    }
  }
  for (index = 0;
       index < sizeof(session_result_fns) / sizeof(session_result_fns[0]);
       ++index) {
    if (session_result_fns[index] == NULL) {
      return 4;
    }
    result_size = 0;
    int status = expect_result_failure(
        "session result function",
        session_result_fns[index](NULL, NULL, 0, result_bytes,
                                  sizeof(result_bytes), &result_size));
    if (status != 0) {
      return status;
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
  SessionCoordinateEnergyFn session_coordinate_energy_fns[] = {
      nwchemc_session_energy,
  };
  SessionCoordinateBufferFn session_coordinate_buffer_fns[] = {
      nwchemc_session_energy_gradient,
      nwchemc_session_energy_forces,
      nwchemc_session_dipole,
      nwchemc_session_polarizability,
      nwchemc_session_quadrupole,
      nwchemc_session_optimize,
      nwchemc_session_stress,
      nwchemc_session_hessian,
  };
  SessionCoordinateFrequenciesFn session_coordinate_frequency_fns[] = {
      nwchemc_session_frequencies,
  };
  double doubles[36] = {0.0};
  size_t index;

  for (index = 0; index < sizeof(config_fns) / sizeof(config_fns[0]);
       ++index) {
    if (config_fns[index] == NULL) {
      return 4;
    }
    int status = expect_status_failure("config function",
                                       config_fns[index](NULL, 0));
    if (status != 0) {
      return status;
    }
  }
  for (index = 0;
       index < sizeof(coordinate_energy_fns) / sizeof(coordinate_energy_fns[0]);
       ++index) {
    if (coordinate_energy_fns[index] == NULL) {
      return 4;
    }
    int status = expect_result_failure(
        "coordinate energy function",
        coordinate_energy_fns[index](0, NULL, NULL, NULL, 0));
    if (status != 0) {
      return status;
    }
  }
  for (index = 0;
       index < sizeof(coordinate_buffer_fns) / sizeof(coordinate_buffer_fns[0]);
       ++index) {
    if (coordinate_buffer_fns[index] == NULL) {
      return 4;
    }
    int status = expect_result_failure(
        "coordinate buffer function",
        coordinate_buffer_fns[index](0, NULL, NULL, NULL, 0, doubles));
    if (status != 0) {
      return status;
    }
  }
  for (index = 0;
       index <
           sizeof(coordinate_frequency_fns) / sizeof(coordinate_frequency_fns[0]);
       ++index) {
    if (coordinate_frequency_fns[index] == NULL) {
      return 4;
    }
    int status = expect_result_failure(
        "coordinate frequency function",
        coordinate_frequency_fns[index](0, NULL, NULL, NULL, 0, doubles,
                                        doubles));
    if (status != 0) {
      return status;
    }
  }
  for (index = 0;
       index < sizeof(session_create_fns) / sizeof(session_create_fns[0]);
       ++index) {
    if (session_create_fns[index] == NULL) {
      return 4;
    }
    int status = expect_null_session("session create function",
                                     session_create_fns[index](NULL, 0));
    if (status != 0) {
      return status;
    }
  }
  for (index = 0;
       index < sizeof(session_config_fns) / sizeof(session_config_fns[0]);
       ++index) {
    if (session_config_fns[index] == NULL) {
      return 4;
    }
    int status = expect_status_failure(
        "session config function", session_config_fns[index](NULL, NULL, 0));
    if (status != 0) {
      return status;
    }
  }
  for (index = 0;
       index < sizeof(session_destroy_fns) / sizeof(session_destroy_fns[0]);
       ++index) {
    if (session_destroy_fns[index] == NULL) {
      return 4;
    }
    session_destroy_fns[index](NULL);
  }
  for (index = 0; index < sizeof(session_coordinate_energy_fns) /
                          sizeof(session_coordinate_energy_fns[0]);
       ++index) {
    if (session_coordinate_energy_fns[index] == NULL) {
      return 4;
    }
    int status = expect_result_failure(
        "session coordinate energy function",
        session_coordinate_energy_fns[index](NULL, 0, NULL, NULL));
    if (status != 0) {
      return status;
    }
  }
  for (index = 0; index < sizeof(session_coordinate_buffer_fns) /
                          sizeof(session_coordinate_buffer_fns[0]);
       ++index) {
    if (session_coordinate_buffer_fns[index] == NULL) {
      return 4;
    }
    int status = expect_result_failure(
        "session coordinate buffer function",
        session_coordinate_buffer_fns[index](NULL, 0, NULL, NULL, doubles));
    if (status != 0) {
      return status;
    }
  }
  for (index = 0; index < sizeof(session_coordinate_frequency_fns) /
                          sizeof(session_coordinate_frequency_fns[0]);
       ++index) {
    if (session_coordinate_frequency_fns[index] == NULL) {
      return 4;
    }
    int status = expect_result_failure(
        "session coordinate frequency function",
        session_coordinate_frequency_fns[index](NULL, 0, NULL, NULL, doubles,
                                               doubles));
    if (status != 0) {
      return status;
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
