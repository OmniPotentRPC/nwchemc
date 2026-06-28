cmake_minimum_required(VERSION 3.24)

project(nwchemc VERSION 1.2.0 LANGUAGES C Fortran)

include(CTest)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(NWCHEMC_BUILD_TESTS "Build nwchemc tests" ON)
option(NWCHEMC_WITH_NWCHEM "Build the real NWChem embed library" OFF)
option(NWCHEMC_BUILD_SHARED "Build nwchemc as a shared library" ON)
set(NWCHEMC_NWCHEM_ROOT "" CACHE PATH "NWChem source/build tree root")
set(NWCHEMC_NWCHEM_TARGET "LINUX64" CACHE STRING "NWChem target library directory")
set(NWCHEMC_NWCHEM_TEST_MPI_RANKS "1" CACHE STRING
  "MPI ranks used to launch real NWChem embed tests"
)

find_package(Python3 REQUIRED COMPONENTS Interpreter)
if(NWCHEMC_BUILD_TESTS)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(CMOCKA REQUIRED IMPORTED_TARGET cmocka)
endif()

set(NWCHEMC_COMMON_C_WARNINGS)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
  list(APPEND NWCHEMC_COMMON_C_WARNINGS -Wall -Wextra -Wpedantic)
endif()
find_library(NWCHEMC_M_LIBRARY m)

set(NWCHEMC_INCLUDE_DIRS
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

add_library(nwchemc_stub STATIC src/nwchemc_stub.c)
target_include_directories(nwchemc_stub PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)
target_compile_options(nwchemc_stub PRIVATE ${NWCHEMC_COMMON_C_WARNINGS})

set(NWCHEMC_NEEDS_CAPNP_C OFF)
if(NWCHEMC_BUILD_TESTS OR NWCHEMC_WITH_NWCHEM)
  set(NWCHEMC_NEEDS_CAPNP_C ON)
endif()

if(NWCHEMC_NEEDS_CAPNP_C)
  find_program(CAPNP_EXECUTABLE capnp REQUIRED)

  set(CAPNP_C_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/subprojects/c-capnproto")
  set(CAPNP_C_INCLUDES
    "${CAPNP_C_ROOT}"
    "${CAPNP_C_ROOT}/compiler"
  )
  set(CAPNP_C_WARNINGS)
  if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    list(APPEND CAPNP_C_WARNINGS
      -Wno-discarded-qualifiers
      -Wno-maybe-uninitialized
      -Wno-unused-function
      -Wno-unused-parameter
    )
  endif()

  add_library(capnp_c_runtime STATIC
    "${CAPNP_C_ROOT}/capn.c"
    "${CAPNP_C_ROOT}/capn-malloc.c"
    "${CAPNP_C_ROOT}/capn-stream.c"
  )
  set_target_properties(capnp_c_runtime PROPERTIES POSITION_INDEPENDENT_CODE ON)
  target_include_directories(capnp_c_runtime PUBLIC
    "$<BUILD_INTERFACE:${CAPNP_C_ROOT}>"
    "$<BUILD_INTERFACE:${CAPNP_C_ROOT}/compiler>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/nwchemc/internal/c-capnproto>"
  )
  target_compile_options(capnp_c_runtime PRIVATE ${CAPNP_C_WARNINGS})

  add_executable(capnpc-c
    "${CAPNP_C_ROOT}/compiler/capnpc-c.c"
    "${CAPNP_C_ROOT}/compiler/schema.capnp.c"
    "${CAPNP_C_ROOT}/compiler/c++.capnp.c"
    "${CAPNP_C_ROOT}/compiler/str.c"
  )
  target_include_directories(capnpc-c PRIVATE ${CAPNP_C_INCLUDES})
  target_link_libraries(capnpc-c PRIVATE capnp_c_runtime)
  target_compile_options(capnpc-c PRIVATE ${CAPNP_C_WARNINGS})

  set(NWCHEMC_SCHEMA_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/schema")
  file(MAKE_DIRECTORY "${NWCHEMC_SCHEMA_BUILD_DIR}")
  set(NWCHEMC_POTENTIALS_CAPNP_C
    "${NWCHEMC_SCHEMA_BUILD_DIR}/Potentials.capnp.c"
  )
  set(NWCHEMC_POTENTIALS_CAPNP_H
    "${NWCHEMC_SCHEMA_BUILD_DIR}/Potentials.capnp.h"
  )
  add_custom_command(
    OUTPUT
      "${NWCHEMC_POTENTIALS_CAPNP_C}"
      "${NWCHEMC_POTENTIALS_CAPNP_H}"
    COMMAND
      "${Python3_EXECUTABLE}"
      "${CMAKE_CURRENT_SOURCE_DIR}/tools/capnpc_c_compile.py"
      "${CAPNP_EXECUTABLE}"
      "$<TARGET_FILE:capnpc-c>"
      "${CMAKE_CURRENT_SOURCE_DIR}/schema"
      "${NWCHEMC_SCHEMA_BUILD_DIR}"
      "${CMAKE_CURRENT_SOURCE_DIR}/schema/Potentials.capnp"
    DEPENDS
      capnpc-c
      "${CMAKE_CURRENT_SOURCE_DIR}/schema/Potentials.capnp"
      "${CMAKE_CURRENT_SOURCE_DIR}/tools/capnpc_c_compile.py"
    VERBATIM
  )
  if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    set_source_files_properties("${NWCHEMC_POTENTIALS_CAPNP_C}" PROPERTIES
      COMPILE_OPTIONS -Wno-missing-field-initializers
    )
  endif()

  add_library(nwchemc_params STATIC
    src/nwchemc_params.c
    "${NWCHEMC_POTENTIALS_CAPNP_C}"
  )
  set_target_properties(nwchemc_params PROPERTIES POSITION_INDEPENDENT_CODE ON)
  target_include_directories(nwchemc_params PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/schema>"
    "$<BUILD_INTERFACE:${NWCHEMC_SCHEMA_BUILD_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/nwchemc/internal>"
  )
  target_link_libraries(nwchemc_params PUBLIC capnp_c_runtime)
  target_compile_options(nwchemc_params PRIVATE ${NWCHEMC_COMMON_C_WARNINGS})
endif()

if(NWCHEMC_WITH_NWCHEM)
  if(NOT NWCHEMC_NWCHEM_ROOT)
    message(FATAL_ERROR "NWCHEMC_NWCHEM_ROOT is required when NWCHEMC_WITH_NWCHEM=ON")
  endif()

  execute_process(
    COMMAND
      "${Python3_EXECUTABLE}"
      "${CMAKE_CURRENT_SOURCE_DIR}/tools/nwchem_root_probe.py"
      "${NWCHEMC_NWCHEM_ROOT}"
      "--target"
      "${NWCHEMC_NWCHEM_TARGET}"
    RESULT_VARIABLE NWCHEM_ROOT_PROBE_STATUS
    OUTPUT_VARIABLE NWCHEM_ROOT_PROBE_STDOUT
    ERROR_VARIABLE NWCHEM_ROOT_PROBE_STDERR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE
  )
  if(NOT NWCHEM_ROOT_PROBE_STATUS EQUAL 0)
    message(FATAL_ERROR
      "NWChem embed root validation failed:\n"
      "${NWCHEM_ROOT_PROBE_STDERR}\n"
      "${NWCHEM_ROOT_PROBE_STDOUT}"
    )
  endif()

  set(NWCHEM_CONFIG_H "${NWCHEMC_NWCHEM_ROOT}/src/config/nwchem_config.h")
  if(NOT EXISTS "${NWCHEM_CONFIG_H}")
    message(FATAL_ERROR "NWChem config header not found: ${NWCHEM_CONFIG_H}")
  endif()

  execute_process(
    COMMAND
      "${Python3_EXECUTABLE}" -c
      "import pathlib, re, sys; text = pathlib.Path(sys.argv[1]).read_text(); match = re.search(r'^NW_MODULE_LIBS\\s*=\\s*(.*)$', text, re.M); sys.exit(1) if not match else print(match.group(1))"
      "${NWCHEM_CONFIG_H}"
    OUTPUT_VARIABLE NWCHEM_MODULE_LIBS_RAW
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
  )
  separate_arguments(NWCHEM_MODULE_LIBS UNIX_COMMAND "${NWCHEM_MODULE_LIBS_RAW}")

  find_program(GA_CONFIG_EXECUTABLE ga-config
    HINTS "${NWCHEMC_NWCHEM_ROOT}/src/tools/install/bin"
    NO_DEFAULT_PATH
    REQUIRED
  )
  execute_process(
    COMMAND "${GA_CONFIG_EXECUTABLE}" --ldflags
    OUTPUT_VARIABLE GA_LDFLAGS_RAW
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
  )
  execute_process(
    COMMAND "${GA_CONFIG_EXECUTABLE}" --libs
    OUTPUT_VARIABLE GA_LIBS_RAW
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
  )
  separate_arguments(GA_LDFLAGS UNIX_COMMAND "${GA_LDFLAGS_RAW}")
  separate_arguments(GA_LIBS UNIX_COMMAND "${GA_LIBS_RAW}")

  execute_process(
    COMMAND
      "${Python3_EXECUTABLE}"
      "${CMAKE_CURRENT_SOURCE_DIR}/tools/nwchem_link_utility_libs.py"
      "${NWCHEMC_NWCHEM_ROOT}"
      "--target"
      "${NWCHEMC_NWCHEM_TARGET}"
    OUTPUT_VARIABLE NWCHEM_UTILITY_LIBS_RAW
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
  )
  separate_arguments(NWCHEM_UTILITY_LIBS UNIX_COMMAND "${NWCHEM_UTILITY_LIBS_RAW}")

  set(MPI_LIBDIRS)
  foreach(_mpi_dir /usr/lib /usr/lib64 /usr/local/lib)
    file(GLOB _mpi_libs "${_mpi_dir}/libmpi.so*")
    if(_mpi_libs)
      list(APPEND MPI_LIBDIRS "-L${_mpi_dir}")
    endif()
  endforeach()

  set(NWCHEM_EMBED_INCLUDES
    "${NWCHEMC_NWCHEM_ROOT}/src/basis"
    "${NWCHEMC_NWCHEM_ROOT}/src/include"
    "${NWCHEMC_NWCHEM_ROOT}/src/nwpw/nwpwlib/utilities"
    "${NWCHEMC_NWCHEM_ROOT}/src/util"
    "${NWCHEMC_NWCHEM_ROOT}/src/tools/install/include"
  )

  set(NWCHEM_FORTRAN_OPTIONS)
  if(CMAKE_Fortran_COMPILER_ID MATCHES "GNU")
    list(APPEND NWCHEM_FORTRAN_OPTIONS
      -fdefault-integer-8
      -ffast-math
    )
  endif()

  add_library(nwchemc_legacy STATIC
    src/nwchem_embed_legacy.F
    src/nwchem_rtdb_put_dbl_array.F
    src/nwchem_rtdb_put_dbl.F
    src/nwchem_rtdb_put_int_array.F
    src/nwchem_rtdb_put_int.F
    src/nwchem_rtdb_put_log_array.F
    "${NWCHEMC_NWCHEM_ROOT}/src/nwpw/nwpwlib/utilities/btdb.F"
  )
  set_target_properties(nwchemc_legacy PROPERTIES POSITION_INDEPENDENT_CODE ON)
  target_include_directories(nwchemc_legacy PRIVATE ${NWCHEM_EMBED_INCLUDES})
  target_compile_definitions(nwchemc_legacy PRIVATE
    NWCHEMC_HAS_NWCHEM
    GFORTRAN
    LINUX64
    EXT_INT
    OPENBLAS
  )
  target_compile_options(nwchemc_legacy PRIVATE
    ${NWCHEM_FORTRAN_OPTIONS}
    $<$<COMPILE_LANG_AND_ID:Fortran,GNU>:-std=legacy>
  )

  set(NWCHEMC_LIBRARY_TYPE STATIC)
  if(NWCHEMC_BUILD_SHARED)
    set(NWCHEMC_LIBRARY_TYPE SHARED)
  endif()

  add_library(nwchemc ${NWCHEMC_LIBRARY_TYPE}
    src/nwchemc.c
    src/nwchem_embed_c_api.f90
  )
  target_include_directories(nwchemc PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
  )
  target_include_directories(nwchemc PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${NWCHEMC_NWCHEM_ROOT}/src/include"
    "${NWCHEMC_NWCHEM_ROOT}/src/tools/install/include"
  )
  target_compile_definitions(nwchemc PRIVATE NWCHEMC_HAS_NWCHEM GFORTRAN)
  target_compile_options(nwchemc PRIVATE
    $<$<COMPILE_LANGUAGE:C>:-Wall>
    $<$<COMPILE_LANGUAGE:C>:-Wextra>
    $<$<COMPILE_LANGUAGE:C>:-Wpedantic>
    $<$<COMPILE_LANG_AND_ID:Fortran,GNU>:-fdefault-integer-8>
    $<$<COMPILE_LANG_AND_ID:Fortran,GNU>:-ffast-math>
  )

  set(NWCHEM_LINK_ITEMS
    "-L${NWCHEMC_NWCHEM_ROOT}/lib/${NWCHEMC_NWCHEM_TARGET}"
    "-L${NWCHEMC_NWCHEM_ROOT}/src/tools/install/lib"
    "-L${NWCHEMC_NWCHEM_ROOT}/src/libext/lib"
    ${GA_LDFLAGS}
    ${MPI_LIBDIRS}
    "-Wl,--start-group"
    "${NWCHEMC_NWCHEM_ROOT}/src/stubs.o"
    "-lnwctask"
    ${NWCHEM_MODULE_LIBS}
    ${NWCHEM_UTILITY_LIBS}
    "-lcomex"
    ${GA_LIBS}
    "-Wl,--end-group"
    "-lgfortran"
    "-lpthread"
    "-lm"
  )
  target_link_libraries(nwchemc PRIVATE
    nwchemc_legacy
    nwchemc_params
    ${NWCHEM_LINK_ITEMS}
  )

  install(TARGETS
    nwchemc
    nwchemc_legacy
    nwchemc_params
    capnp_c_runtime
    EXPORT nwchemcTargets
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  )
  install(FILES include/nwchemc.h DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
  install(FILES
    src/nwchemc_params.h
    "${NWCHEMC_POTENTIALS_CAPNP_H}"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/nwchemc/internal"
  )
  install(FILES
    "${CAPNP_C_ROOT}/capn.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/nwchemc/internal/c-capnproto"
  )
  install(EXPORT nwchemcTargets
    NAMESPACE nwchemc::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/nwchemc"
  )
  configure_package_config_file(
    cmake/nwchemcConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/nwchemcConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/nwchemc"
  )
  write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/nwchemcConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion
  )
  install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/nwchemcConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/nwchemcConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/nwchemc"
  )
endif()

if(NWCHEMC_BUILD_TESTS AND BUILD_TESTING)
  add_test(
    NAME nwchem-root-probe
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_nwchem_root_probe.py"
  )
  set_tests_properties(nwchem-root-probe PROPERTIES
    ENVIRONMENT "TMPDIR=${CMAKE_CURRENT_BINARY_DIR};NWCHEMC_TEST_SCRATCH_ROOT=${CMAKE_CURRENT_BINARY_DIR}/probe-tests"
  )

  add_test(
    NAME schema-module-coverage
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_schema_module_coverage.py"
  )

  find_program(MESON_EXECUTABLE meson)
  if(MESON_EXECUTABLE)
    add_test(
      NAME nwchem-build-root-probe
      COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_build_embed_root_probe.py"
    )
    set_tests_properties(nwchem-build-root-probe PROPERTIES
      ENVIRONMENT "TMPDIR=${CMAKE_CURRENT_BINARY_DIR};NWCHEMC_TEST_SCRATCH_ROOT=${CMAKE_CURRENT_BINARY_DIR}/probe-tests;CMAKE=${CMAKE_COMMAND};MESON=${MESON_EXECUTABLE}"
      TIMEOUT 120
    )
  endif()

  add_executable(test_stub_abi tests/test_stub_abi.c)
  target_link_libraries(test_stub_abi PRIVATE nwchemc_stub PkgConfig::CMOCKA)
  target_compile_options(test_stub_abi PRIVATE ${NWCHEMC_COMMON_C_WARNINGS})
  add_test(NAME stub-abi COMMAND test_stub_abi)

  function(nwchemc_add_encoded_fixture target_name output_var schema_type
           input_file output_file)
    set(bin_path "${CMAKE_CURRENT_BINARY_DIR}/${output_file}")
    add_custom_command(
      OUTPUT "${bin_path}"
      COMMAND
        "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_capnp.py"
        "${CAPNP_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/schema/Potentials.capnp"
        "${schema_type}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/${input_file}"
        "${bin_path}"
      DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_capnp.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/${input_file}"
        "${CMAKE_CURRENT_SOURCE_DIR}/schema/Potentials.capnp"
      VERBATIM
    )
    add_custom_target("${target_name}" DEPENDS "${bin_path}")
    set("${output_var}" "${bin_path}" PARENT_SCOPE)
  endfunction()

  set(NWCHEMC_PARSER_PARAMS_BIN
    "${CMAKE_CURRENT_BINARY_DIR}/nwchem_params_parser.bin"
  )
  add_custom_command(
    OUTPUT "${NWCHEMC_PARSER_PARAMS_BIN}"
    COMMAND
      "${Python3_EXECUTABLE}"
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_capnp.py"
      "${CAPNP_EXECUTABLE}"
      "${CMAKE_CURRENT_SOURCE_DIR}/schema/Potentials.capnp"
      "NWChemParams"
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/nwchem_params_parser.capnp.txt"
      "${NWCHEMC_PARSER_PARAMS_BIN}"
    DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_capnp.py"
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/nwchem_params_parser.capnp.txt"
      "${CMAKE_CURRENT_SOURCE_DIR}/schema/Potentials.capnp"
    VERBATIM
  )
  add_custom_target(nwchem_parser_params_bin
    DEPENDS "${NWCHEMC_PARSER_PARAMS_BIN}"
  )

  set(NWCHEMC_DIRECT_DFT_PARAMS_BIN
    "${CMAKE_CURRENT_BINARY_DIR}/nwchem_params_direct_dft.bin"
  )
  add_custom_command(
    OUTPUT "${NWCHEMC_DIRECT_DFT_PARAMS_BIN}"
    COMMAND
      "${Python3_EXECUTABLE}"
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_capnp.py"
      "${CAPNP_EXECUTABLE}"
      "${CMAKE_CURRENT_SOURCE_DIR}/schema/Potentials.capnp"
      "NWChemParams"
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/nwchem_params_direct_dft.capnp.txt"
      "${NWCHEMC_DIRECT_DFT_PARAMS_BIN}"
    DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_capnp.py"
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/nwchem_params_direct_dft.capnp.txt"
      "${CMAKE_CURRENT_SOURCE_DIR}/schema/Potentials.capnp"
    VERBATIM
  )
  add_custom_target(nwchem_direct_dft_params_bin
    DEPENDS "${NWCHEMC_DIRECT_DFT_PARAMS_BIN}"
  )

  set(NWCHEMC_CONFIG_OPTIONS_PARAMS_BIN
    "${CMAKE_CURRENT_BINARY_DIR}/nwchem_params_config_options.bin"
  )
  add_custom_command(
    OUTPUT "${NWCHEMC_CONFIG_OPTIONS_PARAMS_BIN}"
    COMMAND
      "${Python3_EXECUTABLE}"
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_capnp.py"
      "${CAPNP_EXECUTABLE}"
      "${CMAKE_CURRENT_SOURCE_DIR}/schema/Potentials.capnp"
      "NWChemParams"
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/nwchem_params_config_options.capnp.txt"
      "${NWCHEMC_CONFIG_OPTIONS_PARAMS_BIN}"
    DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_capnp.py"
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/nwchem_params_config_options.capnp.txt"
      "${CMAKE_CURRENT_SOURCE_DIR}/schema/Potentials.capnp"
    VERBATIM
  )
  add_custom_target(nwchem_config_options_params_bin
    DEPENDS "${NWCHEMC_CONFIG_OPTIONS_PARAMS_BIN}"
  )

  nwchemc_add_encoded_fixture(
    nwchem_pspspin_params_bin
    NWCHEMC_PSPSPIN_PARAMS_BIN
    NWChemParams
    nwchem_params_pspspin.capnp.txt
    nwchem_params_pspspin.bin
  )
  nwchemc_add_encoded_fixture(
    nwchem_pspspin_many_params_bin
    NWCHEMC_PSPSPIN_MANY_PARAMS_BIN
    NWChemParams
    nwchem_params_pspspin_many.capnp.txt
    nwchem_params_pspspin_many.bin
  )
  nwchemc_add_encoded_fixture(
    nwchem_nwpw_spin_mode_params_bin
    NWCHEMC_NWPW_SPIN_MODE_PARAMS_BIN
    NWChemParams
    nwchem_params_nwpw_spin_mode.capnp.txt
    nwchem_params_nwpw_spin_mode.bin
  )
  nwchemc_add_encoded_fixture(
    nwchem_nwpw_allow_translation_params_bin
    NWCHEMC_NWPW_ALLOW_TRANSLATION_PARAMS_BIN
    NWChemParams
    nwchem_params_nwpw_allow_translation.capnp.txt
    nwchem_params_nwpw_allow_translation.bin
  )
  nwchemc_add_encoded_fixture(
    nwchem_nwpw_cutoff_alias_params_bin
    NWCHEMC_NWPW_CUTOFF_ALIAS_PARAMS_BIN
    NWChemParams
    nwchem_params_nwpw_cutoff_alias.capnp.txt
    nwchem_params_nwpw_cutoff_alias.bin
  )
  nwchemc_add_encoded_fixture(
    nwchem_nwpw_mc_steps_params_bin
    NWCHEMC_NWPW_MC_STEPS_PARAMS_BIN
    NWChemParams
    nwchem_params_nwpw_mc_steps.capnp.txt
    nwchem_params_nwpw_mc_steps.bin
  )
  nwchemc_add_encoded_fixture(
    nwchem_nwpw_bo_steps_default_params_bin
    NWCHEMC_NWPW_BO_STEPS_DEFAULT_PARAMS_BIN
    NWChemParams
    nwchem_params_nwpw_bo_steps_default.capnp.txt
    nwchem_params_nwpw_bo_steps_default.bin
  )
  nwchemc_add_encoded_fixture(
    nwchem_nwpw_bo_time_step_default_params_bin
    NWCHEMC_NWPW_BO_TIME_STEP_DEFAULT_PARAMS_BIN
    NWChemParams
    nwchem_params_nwpw_bo_time_step_default.capnp.txt
    nwchem_params_nwpw_bo_time_step_default.bin
  )
  nwchemc_add_encoded_fixture(
    nwchem_nwpw_bo_fake_mass_default_params_bin
    NWCHEMC_NWPW_BO_FAKE_MASS_DEFAULT_PARAMS_BIN
    NWChemParams
    nwchem_params_nwpw_bo_fake_mass_default.capnp.txt
    nwchem_params_nwpw_bo_fake_mass_default.bin
  )
  nwchemc_add_encoded_fixture(
    nwchem_nwpw_scaling_default_params_bin
    NWCHEMC_NWPW_SCALING_DEFAULT_PARAMS_BIN
    NWChemParams
    nwchem_params_nwpw_scaling_default.capnp.txt
    nwchem_params_nwpw_scaling_default.bin
  )
  nwchemc_add_encoded_fixture(
    nwchem_nwpw_np_dimensions_default_params_bin
    NWCHEMC_NWPW_NP_DIMENSIONS_DEFAULT_PARAMS_BIN
    NWChemParams
    nwchem_params_nwpw_np_dimensions_default.capnp.txt
    nwchem_params_nwpw_np_dimensions_default.bin
  )
  nwchemc_add_encoded_fixture(
    nwchem_nwpw_tolerances_default_params_bin
    NWCHEMC_NWPW_TOLERANCES_DEFAULT_PARAMS_BIN
    NWChemParams
    nwchem_params_nwpw_tolerances_default.capnp.txt
    nwchem_params_nwpw_tolerances_default.bin
  )
  nwchemc_add_encoded_fixture(
    nwchem_nwpw_mc_steps_default_params_bin
    NWCHEMC_NWPW_MC_STEPS_DEFAULT_PARAMS_BIN
    NWChemParams
    nwchem_params_nwpw_mc_steps_default.capnp.txt
    nwchem_params_nwpw_mc_steps_default.bin
  )
  nwchemc_add_encoded_fixture(
    nwchem_brillouin_tetrahedron_params_bin
    NWCHEMC_BRILLOUIN_TETRAHEDRON_PARAMS_BIN
    NWChemParams
    nwchem_params_brillouin_tetrahedron.capnp.txt
    nwchem_params_brillouin_tetrahedron.bin
  )
  nwchemc_add_encoded_fixture(
    nwchem_brillouin_dos_grid_params_bin
    NWCHEMC_BRILLOUIN_DOS_GRID_PARAMS_BIN
    NWChemParams
    nwchem_params_brillouin_dos_grid.capnp.txt
    nwchem_params_brillouin_dos_grid.bin
  )
  nwchemc_add_encoded_fixture(
    nwchem_nwpw_et_params_bin
    NWCHEMC_NWPW_ET_PARAMS_BIN
    NWChemParams
    nwchem_params_nwpw_et.capnp.txt
    nwchem_params_nwpw_et.bin
  )
  nwchemc_add_encoded_fixture(
    nwchem_nwpw_temperature_params_bin
    NWCHEMC_NWPW_TEMPERATURE_PARAMS_BIN
    NWChemParams
    nwchem_params_nwpw_temperature.capnp.txt
    nwchem_params_nwpw_temperature.bin
  )
  nwchemc_add_encoded_fixture(
    nwchem_nwpw_dos_default_params_bin
    NWCHEMC_NWPW_DOS_DEFAULT_PARAMS_BIN
    NWChemParams
    nwchem_params_nwpw_dos_default.capnp.txt
    nwchem_params_nwpw_dos_default.bin
  )
  nwchemc_add_encoded_fixture(
    nwchem_nwpw_mapping_alias_params_bin
    NWCHEMC_NWPW_MAPPING_ALIAS_PARAMS_BIN
    NWChemParams
    nwchem_params_nwpw_mapping_alias.capnp.txt
    nwchem_params_nwpw_mapping_alias.bin
  )
  nwchemc_add_encoded_fixture(
    nwchem_nwpw_mapping_default_params_bin
    NWCHEMC_NWPW_MAPPING_DEFAULT_PARAMS_BIN
    NWChemParams
    nwchem_params_nwpw_mapping_default.capnp.txt
    nwchem_params_nwpw_mapping_default.bin
  )
  nwchemc_add_encoded_fixture(
    nwchem_nwpw_virtual_alias_params_bin
    NWCHEMC_NWPW_VIRTUAL_ALIAS_PARAMS_BIN
    NWChemParams
    nwchem_params_nwpw_virtual_alias.capnp.txt
    nwchem_params_nwpw_virtual_alias.bin
  )
  nwchemc_add_encoded_fixture(
    nwchem_nwpw_one_electron_guess_defaults_params_bin
    NWCHEMC_NWPW_ONE_ELECTRON_GUESS_DEFAULTS_PARAMS_BIN
    NWChemParams
    nwchem_params_nwpw_one_electron_guess_defaults.capnp.txt
    nwchem_params_nwpw_one_electron_guess_defaults.bin
  )
  nwchemc_add_encoded_fixture(
    nwchem_nwpw_fractional_orbitals_default_params_bin
    NWCHEMC_NWPW_FRACTIONAL_ORBITALS_DEFAULT_PARAMS_BIN
    NWChemParams
    nwchem_params_nwpw_fractional_orbitals_default.capnp.txt
    nwchem_params_nwpw_fractional_orbitals_default.bin
  )
  nwchemc_add_encoded_fixture(
    nwchem_nwpw_smear_orbitals_default_params_bin
    NWCHEMC_NWPW_SMEAR_ORBITALS_DEFAULT_PARAMS_BIN
    NWChemParams
    nwchem_params_nwpw_smear_orbitals_default.capnp.txt
    nwchem_params_nwpw_smear_orbitals_default.bin
  )
  nwchemc_add_encoded_fixture(
    nwchem_nwpw_virtual_orbitals_default_params_bin
    NWCHEMC_NWPW_VIRTUAL_ORBITALS_DEFAULT_PARAMS_BIN
    NWChemParams
    nwchem_params_nwpw_virtual_orbitals_default.capnp.txt
    nwchem_params_nwpw_virtual_orbitals_default.bin
  )
  nwchemc_add_encoded_fixture(
    nwchem_nwpw_translate_vector_default_params_bin
    NWCHEMC_NWPW_TRANSLATE_VECTOR_DEFAULT_PARAMS_BIN
    NWChemParams
    nwchem_params_nwpw_translate_vector_default.capnp.txt
    nwchem_params_nwpw_translate_vector_default.bin
  )
  nwchemc_add_encoded_fixture(
    nwchem_nwpw_cell_expand_default_params_bin
    NWCHEMC_NWPW_CELL_EXPAND_DEFAULT_PARAMS_BIN
    NWChemParams
    nwchem_params_nwpw_cell_expand_default.capnp.txt
    nwchem_params_nwpw_cell_expand_default.bin
  )
  nwchemc_add_encoded_fixture(
    nwchem_brillouin_monkhorst_default_params_bin
    NWCHEMC_BRILLOUIN_MONKHORST_DEFAULT_PARAMS_BIN
    NWChemParams
    nwchem_params_brillouin_monkhorst_default.capnp.txt
    nwchem_params_brillouin_monkhorst_default.bin
  )
  nwchemc_add_encoded_fixture(
    nwchem_brillouin_dos_grid_default_params_bin
    NWCHEMC_BRILLOUIN_DOS_GRID_DEFAULT_PARAMS_BIN
    NWChemParams
    nwchem_params_brillouin_dos_grid_default.capnp.txt
    nwchem_params_brillouin_dos_grid_default.bin
  )
  nwchemc_add_encoded_fixture(
    nwchem_tce_methods_params_bin
    NWCHEMC_TCE_METHODS_PARAMS_BIN
    NWChemParams
    nwchem_params_tce_methods.capnp.txt
    nwchem_params_tce_methods.bin
  )
  nwchemc_add_encoded_fixture(
    nwchem_compact_cells_params_bin
    NWCHEMC_COMPACT_CELLS_PARAMS_BIN
    NWChemParams
    nwchem_params_compact_cells.capnp.txt
    nwchem_params_compact_cells.bin
  )

  set(NWCHEMC_FORCE_INPUT_STEP_A_BIN
    "${CMAKE_CURRENT_BINARY_DIR}/force_input_step_a.bin"
  )
  add_custom_command(
    OUTPUT "${NWCHEMC_FORCE_INPUT_STEP_A_BIN}"
    COMMAND
      "${Python3_EXECUTABLE}"
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_capnp.py"
      "${CAPNP_EXECUTABLE}"
      "${CMAKE_CURRENT_SOURCE_DIR}/schema/Potentials.capnp"
      "ForceInput"
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/force_input_step_a.capnp.txt"
      "${NWCHEMC_FORCE_INPUT_STEP_A_BIN}"
    DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_capnp.py"
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/force_input_step_a.capnp.txt"
      "${CMAKE_CURRENT_SOURCE_DIR}/schema/Potentials.capnp"
    VERBATIM
  )
  add_custom_target(force_input_step_a_bin
    DEPENDS "${NWCHEMC_FORCE_INPUT_STEP_A_BIN}"
  )

  set(NWCHEMC_FORCE_INPUT_STEP_B_BIN
    "${CMAKE_CURRENT_BINARY_DIR}/force_input_step_b.bin"
  )
  add_custom_command(
    OUTPUT "${NWCHEMC_FORCE_INPUT_STEP_B_BIN}"
    COMMAND
      "${Python3_EXECUTABLE}"
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_capnp.py"
      "${CAPNP_EXECUTABLE}"
      "${CMAKE_CURRENT_SOURCE_DIR}/schema/Potentials.capnp"
      "ForceInput"
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/force_input_step_b.capnp.txt"
      "${NWCHEMC_FORCE_INPUT_STEP_B_BIN}"
    DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_capnp.py"
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/force_input_step_b.capnp.txt"
      "${CMAKE_CURRENT_SOURCE_DIR}/schema/Potentials.capnp"
    VERBATIM
  )
  add_custom_target(force_input_step_b_bin
    DEPENDS "${NWCHEMC_FORCE_INPUT_STEP_B_BIN}"
  )

  set(NWCHEMC_FORCE_INPUT_STEP_EV_BIN
    "${CMAKE_CURRENT_BINARY_DIR}/force_input_step_ev.bin"
  )
  add_custom_command(
    OUTPUT "${NWCHEMC_FORCE_INPUT_STEP_EV_BIN}"
    COMMAND
      "${Python3_EXECUTABLE}"
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_capnp.py"
      "${CAPNP_EXECUTABLE}"
      "${CMAKE_CURRENT_SOURCE_DIR}/schema/Potentials.capnp"
      "ForceInput"
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/force_input_step_ev.capnp.txt"
      "${NWCHEMC_FORCE_INPUT_STEP_EV_BIN}"
    DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_capnp.py"
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/force_input_step_ev.capnp.txt"
      "${CMAKE_CURRENT_SOURCE_DIR}/schema/Potentials.capnp"
    VERBATIM
  )
  add_custom_target(force_input_step_ev_bin
    DEPENDS "${NWCHEMC_FORCE_INPUT_STEP_EV_BIN}"
  )

  set(NWCHEMC_FORCE_INPUT_STEP_CHANGED_SPECIES_BIN
    "${CMAKE_CURRENT_BINARY_DIR}/force_input_step_changed_species.bin"
  )
  add_custom_command(
    OUTPUT "${NWCHEMC_FORCE_INPUT_STEP_CHANGED_SPECIES_BIN}"
    COMMAND
      "${Python3_EXECUTABLE}"
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_capnp.py"
      "${CAPNP_EXECUTABLE}"
      "${CMAKE_CURRENT_SOURCE_DIR}/schema/Potentials.capnp"
      "ForceInput"
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/force_input_step_changed_species.capnp.txt"
      "${NWCHEMC_FORCE_INPUT_STEP_CHANGED_SPECIES_BIN}"
    DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_capnp.py"
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/force_input_step_changed_species.capnp.txt"
      "${CMAKE_CURRENT_SOURCE_DIR}/schema/Potentials.capnp"
    VERBATIM
  )
  add_custom_target(force_input_step_changed_species_bin
    DEPENDS "${NWCHEMC_FORCE_INPUT_STEP_CHANGED_SPECIES_BIN}"
  )

  set(NWCHEMC_FORCE_INPUT_H2_EQ_BIN
    "${CMAKE_CURRENT_BINARY_DIR}/force_input_h2_eq.bin"
  )
  add_custom_command(
    OUTPUT "${NWCHEMC_FORCE_INPUT_H2_EQ_BIN}"
    COMMAND
      "${Python3_EXECUTABLE}"
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_capnp.py"
      "${CAPNP_EXECUTABLE}"
      "${CMAKE_CURRENT_SOURCE_DIR}/schema/Potentials.capnp"
      "ForceInput"
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/force_input_h2_eq.capnp.txt"
      "${NWCHEMC_FORCE_INPUT_H2_EQ_BIN}"
    DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_capnp.py"
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/force_input_h2_eq.capnp.txt"
      "${CMAKE_CURRENT_SOURCE_DIR}/schema/Potentials.capnp"
    VERBATIM
  )
  add_custom_target(force_input_h2_eq_bin
    DEPENDS "${NWCHEMC_FORCE_INPUT_H2_EQ_BIN}"
  )

  nwchemc_add_encoded_fixture(
    force_input_h2_ev_bin
    NWCHEMC_FORCE_INPUT_H2_EV_BIN
    ForceInput
    force_input_h2_ev.capnp.txt
    force_input_h2_ev.bin
  )

  set(NWCHEMC_FORCE_INPUT_H2_STRETCHED_BIN
    "${CMAKE_CURRENT_BINARY_DIR}/force_input_h2_stretched.bin"
  )
  add_custom_command(
    OUTPUT "${NWCHEMC_FORCE_INPUT_H2_STRETCHED_BIN}"
    COMMAND
      "${Python3_EXECUTABLE}"
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_capnp.py"
      "${CAPNP_EXECUTABLE}"
      "${CMAKE_CURRENT_SOURCE_DIR}/schema/Potentials.capnp"
      "ForceInput"
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/force_input_h2_stretched.capnp.txt"
      "${NWCHEMC_FORCE_INPUT_H2_STRETCHED_BIN}"
    DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_capnp.py"
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/force_input_h2_stretched.capnp.txt"
      "${CMAKE_CURRENT_SOURCE_DIR}/schema/Potentials.capnp"
    VERBATIM
  )
  add_custom_target(force_input_h2_stretched_bin
    DEPENDS "${NWCHEMC_FORCE_INPUT_H2_STRETCHED_BIN}"
  )

  nwchemc_add_encoded_fixture(
    force_input_step_state_bin
    NWCHEMC_FORCE_INPUT_STEP_STATE_BIN
    ForceInput
    force_input_step_state.capnp.txt
    force_input_step_state.bin
  )
  nwchemc_add_encoded_fixture(
    force_input_h2_box_ang_bin
    NWCHEMC_FORCE_INPUT_H2_BOX_ANG_BIN
    ForceInput
    force_input_h2_box_ang.capnp.txt
    force_input_h2_box_ang.bin
  )
  nwchemc_add_encoded_fixture(
    force_input_h2_box_bohr_bin
    NWCHEMC_FORCE_INPUT_H2_BOX_BOHR_BIN
    ForceInput
    force_input_h2_box_bohr.capnp.txt
    force_input_h2_box_bohr.bin
  )
  nwchemc_add_encoded_fixture(
    force_input_si_box_ang_bin
    NWCHEMC_FORCE_INPUT_SI_BOX_ANG_BIN
    ForceInput
    force_input_si_box_ang.capnp.txt
    force_input_si_box_ang.bin
  )

  add_executable(test_capnp_parser tests/test_capnp_parser.c)
  target_link_libraries(test_capnp_parser PRIVATE
    nwchemc_params
    PkgConfig::CMOCKA
  )
  target_compile_options(test_capnp_parser PRIVATE ${NWCHEMC_COMMON_C_WARNINGS})
  add_dependencies(test_capnp_parser
    nwchem_parser_params_bin
    nwchem_pspspin_params_bin
    nwchem_nwpw_spin_mode_params_bin
    nwchem_nwpw_allow_translation_params_bin
    nwchem_nwpw_cutoff_alias_params_bin
    nwchem_nwpw_mc_steps_params_bin
    nwchem_nwpw_bo_steps_default_params_bin
    nwchem_nwpw_bo_time_step_default_params_bin
    nwchem_nwpw_bo_fake_mass_default_params_bin
    nwchem_nwpw_scaling_default_params_bin
    nwchem_nwpw_np_dimensions_default_params_bin
    nwchem_nwpw_tolerances_default_params_bin
    nwchem_nwpw_mc_steps_default_params_bin
    nwchem_brillouin_tetrahedron_params_bin
    nwchem_brillouin_dos_grid_params_bin
    nwchem_nwpw_et_params_bin
    nwchem_nwpw_temperature_params_bin
    nwchem_nwpw_dos_default_params_bin
    nwchem_nwpw_mapping_alias_params_bin
    nwchem_nwpw_mapping_default_params_bin
    nwchem_nwpw_virtual_alias_params_bin
    nwchem_nwpw_one_electron_guess_defaults_params_bin
    nwchem_nwpw_fractional_orbitals_default_params_bin
    nwchem_nwpw_smear_orbitals_default_params_bin
    nwchem_nwpw_virtual_orbitals_default_params_bin
    nwchem_nwpw_translate_vector_default_params_bin
    nwchem_nwpw_cell_expand_default_params_bin
    nwchem_brillouin_monkhorst_default_params_bin
    nwchem_brillouin_dos_grid_default_params_bin
    nwchem_compact_cells_params_bin
  )
  add_test(NAME capnp-parser
    COMMAND test_capnp_parser
      "${NWCHEMC_PARSER_PARAMS_BIN}"
      "${NWCHEMC_PSPSPIN_PARAMS_BIN}"
      "${NWCHEMC_NWPW_SPIN_MODE_PARAMS_BIN}"
      "${NWCHEMC_NWPW_ALLOW_TRANSLATION_PARAMS_BIN}"
      "${NWCHEMC_NWPW_CUTOFF_ALIAS_PARAMS_BIN}"
      "${NWCHEMC_NWPW_MC_STEPS_PARAMS_BIN}"
      "${NWCHEMC_NWPW_BO_STEPS_DEFAULT_PARAMS_BIN}"
      "${NWCHEMC_NWPW_BO_TIME_STEP_DEFAULT_PARAMS_BIN}"
      "${NWCHEMC_NWPW_BO_FAKE_MASS_DEFAULT_PARAMS_BIN}"
      "${NWCHEMC_NWPW_SCALING_DEFAULT_PARAMS_BIN}"
      "${NWCHEMC_NWPW_NP_DIMENSIONS_DEFAULT_PARAMS_BIN}"
      "${NWCHEMC_NWPW_TOLERANCES_DEFAULT_PARAMS_BIN}"
      "${NWCHEMC_NWPW_MC_STEPS_DEFAULT_PARAMS_BIN}"
      "${NWCHEMC_BRILLOUIN_TETRAHEDRON_PARAMS_BIN}"
      "${NWCHEMC_BRILLOUIN_DOS_GRID_PARAMS_BIN}"
      "${NWCHEMC_NWPW_ET_PARAMS_BIN}"
      "${NWCHEMC_NWPW_TEMPERATURE_PARAMS_BIN}"
      "${NWCHEMC_NWPW_DOS_DEFAULT_PARAMS_BIN}"
      "${NWCHEMC_NWPW_MAPPING_ALIAS_PARAMS_BIN}"
      "${NWCHEMC_NWPW_MAPPING_DEFAULT_PARAMS_BIN}"
      "${NWCHEMC_NWPW_VIRTUAL_ALIAS_PARAMS_BIN}"
      "${NWCHEMC_NWPW_ONE_ELECTRON_GUESS_DEFAULTS_PARAMS_BIN}"
      "${NWCHEMC_NWPW_FRACTIONAL_ORBITALS_DEFAULT_PARAMS_BIN}"
      "${NWCHEMC_NWPW_SMEAR_ORBITALS_DEFAULT_PARAMS_BIN}"
      "${NWCHEMC_NWPW_VIRTUAL_ORBITALS_DEFAULT_PARAMS_BIN}"
      "${NWCHEMC_NWPW_TRANSLATE_VECTOR_DEFAULT_PARAMS_BIN}"
      "${NWCHEMC_NWPW_CELL_EXPAND_DEFAULT_PARAMS_BIN}"
      "${NWCHEMC_BRILLOUIN_MONKHORST_DEFAULT_PARAMS_BIN}"
      "${NWCHEMC_BRILLOUIN_DOS_GRID_DEFAULT_PARAMS_BIN}"
      "${NWCHEMC_COMPACT_CELLS_PARAMS_BIN}"
  )

  add_executable(test_embed_config_cmocka
    tests/cmocka/test_embed_config_cmocka.c
    src/nwchemc.c
  )
  target_link_libraries(test_embed_config_cmocka PRIVATE
    nwchemc_params
    PkgConfig::CMOCKA
  )
  target_compile_definitions(test_embed_config_cmocka PRIVATE
    NWCHEMC_HAS_NWCHEM=1
  )
  target_compile_options(test_embed_config_cmocka PRIVATE
    ${NWCHEMC_COMMON_C_WARNINGS}
  )
  add_dependencies(test_embed_config_cmocka
    nwchem_direct_dft_params_bin
    nwchem_config_options_params_bin
    nwchem_pspspin_params_bin
    nwchem_pspspin_many_params_bin
    nwchem_nwpw_spin_mode_params_bin
    nwchem_nwpw_allow_translation_params_bin
    nwchem_nwpw_cutoff_alias_params_bin
    nwchem_nwpw_mc_steps_params_bin
    nwchem_nwpw_bo_steps_default_params_bin
    nwchem_nwpw_bo_time_step_default_params_bin
    nwchem_nwpw_bo_fake_mass_default_params_bin
    nwchem_nwpw_scaling_default_params_bin
    nwchem_nwpw_np_dimensions_default_params_bin
    nwchem_nwpw_tolerances_default_params_bin
    nwchem_nwpw_mc_steps_default_params_bin
    nwchem_brillouin_tetrahedron_params_bin
    nwchem_brillouin_dos_grid_params_bin
    nwchem_nwpw_et_params_bin
    nwchem_nwpw_temperature_params_bin
    nwchem_nwpw_dos_default_params_bin
    nwchem_nwpw_mapping_alias_params_bin
    nwchem_nwpw_mapping_default_params_bin
    nwchem_nwpw_virtual_alias_params_bin
    nwchem_nwpw_one_electron_guess_defaults_params_bin
    nwchem_nwpw_fractional_orbitals_default_params_bin
    nwchem_nwpw_smear_orbitals_default_params_bin
    nwchem_nwpw_virtual_orbitals_default_params_bin
    force_input_step_a_bin
    force_input_step_b_bin
    force_input_step_ev_bin
    force_input_step_changed_species_bin
    force_input_step_state_bin
    nwchem_tce_methods_params_bin
    nwchem_compact_cells_params_bin
    nwchem_nwpw_translate_vector_default_params_bin
    nwchem_brillouin_monkhorst_default_params_bin
    nwchem_brillouin_dos_grid_default_params_bin
  )
  add_test(NAME embed-config-cmocka
    COMMAND test_embed_config_cmocka
      "${NWCHEMC_DIRECT_DFT_PARAMS_BIN}"
      "${NWCHEMC_CONFIG_OPTIONS_PARAMS_BIN}"
      "${NWCHEMC_PSPSPIN_PARAMS_BIN}"
      "${NWCHEMC_PSPSPIN_MANY_PARAMS_BIN}"
      "${NWCHEMC_NWPW_SPIN_MODE_PARAMS_BIN}"
      "${NWCHEMC_NWPW_ALLOW_TRANSLATION_PARAMS_BIN}"
      "${NWCHEMC_NWPW_CUTOFF_ALIAS_PARAMS_BIN}"
      "${NWCHEMC_NWPW_MC_STEPS_PARAMS_BIN}"
      "${NWCHEMC_NWPW_BO_STEPS_DEFAULT_PARAMS_BIN}"
      "${NWCHEMC_NWPW_BO_TIME_STEP_DEFAULT_PARAMS_BIN}"
      "${NWCHEMC_NWPW_BO_FAKE_MASS_DEFAULT_PARAMS_BIN}"
      "${NWCHEMC_NWPW_SCALING_DEFAULT_PARAMS_BIN}"
      "${NWCHEMC_NWPW_NP_DIMENSIONS_DEFAULT_PARAMS_BIN}"
      "${NWCHEMC_NWPW_TOLERANCES_DEFAULT_PARAMS_BIN}"
      "${NWCHEMC_NWPW_MC_STEPS_DEFAULT_PARAMS_BIN}"
      "${NWCHEMC_BRILLOUIN_TETRAHEDRON_PARAMS_BIN}"
      "${NWCHEMC_BRILLOUIN_DOS_GRID_PARAMS_BIN}"
      "${NWCHEMC_NWPW_ET_PARAMS_BIN}"
      "${NWCHEMC_NWPW_TEMPERATURE_PARAMS_BIN}"
      "${NWCHEMC_NWPW_DOS_DEFAULT_PARAMS_BIN}"
      "${NWCHEMC_NWPW_MAPPING_ALIAS_PARAMS_BIN}"
      "${NWCHEMC_NWPW_MAPPING_DEFAULT_PARAMS_BIN}"
      "${NWCHEMC_NWPW_VIRTUAL_ALIAS_PARAMS_BIN}"
      "${NWCHEMC_NWPW_ONE_ELECTRON_GUESS_DEFAULTS_PARAMS_BIN}"
      "${NWCHEMC_NWPW_FRACTIONAL_ORBITALS_DEFAULT_PARAMS_BIN}"
      "${NWCHEMC_NWPW_SMEAR_ORBITALS_DEFAULT_PARAMS_BIN}"
      "${NWCHEMC_NWPW_VIRTUAL_ORBITALS_DEFAULT_PARAMS_BIN}"
      "${NWCHEMC_FORCE_INPUT_STEP_A_BIN}"
      "${NWCHEMC_FORCE_INPUT_STEP_B_BIN}"
      "${NWCHEMC_FORCE_INPUT_STEP_EV_BIN}"
      "${NWCHEMC_FORCE_INPUT_STEP_CHANGED_SPECIES_BIN}"
      "${NWCHEMC_FORCE_INPUT_STEP_STATE_BIN}"
      "${NWCHEMC_TCE_METHODS_PARAMS_BIN}"
      "${NWCHEMC_COMPACT_CELLS_PARAMS_BIN}"
      "${NWCHEMC_NWPW_TRANSLATE_VECTOR_DEFAULT_PARAMS_BIN}"
      "${NWCHEMC_BRILLOUIN_MONKHORST_DEFAULT_PARAMS_BIN}"
      "${NWCHEMC_BRILLOUIN_DOS_GRID_DEFAULT_PARAMS_BIN}"
  )

  if(NWCHEMC_WITH_NWCHEM)
    set(NWCHEM_ROOT "${NWCHEMC_NWCHEM_ROOT}")
    set(NWCHEMC_TEST_SCRATCH
      "${CMAKE_CURRENT_BINARY_DIR}/nwchem-test-scratch"
    )
    set(NWCHEMC_TEST_PERMANENT
      "${CMAKE_CURRENT_BINARY_DIR}/nwchem-test-permanent"
    )
    if(NWCHEMC_NWCHEM_TEST_MPI_RANKS LESS 1)
      message(FATAL_ERROR "NWCHEMC_NWCHEM_TEST_MPI_RANKS must be at least 1")
    endif()
    set(NWCHEMC_NWCHEM_TEST_ENV)
    if(NWCHEMC_NWCHEM_TEST_MPI_RANKS GREATER 1)
      find_program(MPIRUN_EXECUTABLE mpirun REQUIRED)
      list(APPEND NWCHEMC_NWCHEM_TEST_ENV
        "OMPI_MCA_prte_create_session_dirs=true"
      )
    endif()

    function(nwchemc_add_real_nwchem_test test_name target_name)
      if(NWCHEMC_NWCHEM_TEST_MPI_RANKS GREATER 1)
        add_test(
          NAME "${test_name}"
          COMMAND
            "${MPIRUN_EXECUTABLE}" -np "${NWCHEMC_NWCHEM_TEST_MPI_RANKS}"
            "$<TARGET_FILE:${target_name}>"
            ${ARGN}
        )
      else()
        add_test(
          NAME "${test_name}"
          COMMAND "$<TARGET_FILE:${target_name}>" ${ARGN}
        )
      endif()
      set_tests_properties("${test_name}" PROPERTIES RUN_SERIAL TRUE)
      if(NWCHEMC_NWCHEM_TEST_ENV)
        set_tests_properties("${test_name}" PROPERTIES
          ENVIRONMENT "${NWCHEMC_NWCHEM_TEST_ENV}"
        )
      endif()
    endfunction()

    function(nwchemc_configure_fixture input_file output_name schema_type)
      configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/${input_file}"
        "${CMAKE_CURRENT_BINARY_DIR}/${output_name}.capnp.txt"
        @ONLY
      )
      set(bin_path "${CMAKE_CURRENT_BINARY_DIR}/${output_name}.bin")
      add_custom_command(
        OUTPUT "${bin_path}"
        COMMAND
          "${Python3_EXECUTABLE}"
          "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_capnp.py"
          "${CAPNP_EXECUTABLE}"
          "${CMAKE_CURRENT_SOURCE_DIR}/schema/Potentials.capnp"
          "${schema_type}"
          "${CMAKE_CURRENT_BINARY_DIR}/${output_name}.capnp.txt"
          "${bin_path}"
        DEPENDS
          "${CMAKE_CURRENT_BINARY_DIR}/${output_name}.capnp.txt"
          "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_capnp.py"
          "${CMAKE_CURRENT_SOURCE_DIR}/schema/Potentials.capnp"
        VERBATIM
      )
      add_custom_target("${output_name}_bin" DEPENDS "${bin_path}")
      set("${output_name}_BIN" "${bin_path}" PARENT_SCOPE)
    endfunction()

    nwchemc_configure_fixture(
      nwchem_params.capnp.txt.in
      nwchem_params
      NWChemParams
    )
    nwchemc_configure_fixture(
      nwchem_params_input_blocks.capnp.txt.in
      nwchem_params_input_blocks
      NWChemParams
    )
    nwchemc_configure_fixture(
      nwchem_params_structured.capnp.txt.in
      nwchem_params_structured
      NWChemParams
    )
    nwchemc_configure_fixture(
      nwchem_params_hessian.capnp.txt.in
      nwchem_params_hessian
      NWChemParams
    )
    nwchemc_configure_fixture(
      potential_config_structured.capnp.txt.in
      potential_config_structured
      PotentialConfig
    )
    nwchemc_configure_fixture(
      potential_config_pseudopotential.capnp.txt.in
      potential_config_pseudopotential
      PotentialConfig
    )
    nwchemc_configure_fixture(
      potential_config_pspw_stress.capnp.txt.in
      potential_config_pspw_stress
      PotentialConfig
    )
    nwchemc_configure_fixture(
      potential_config_pspw_pseudopotential_forces.capnp.txt.in
      potential_config_pspw_pseudopotential_forces
      PotentialConfig
    )
    nwchemc_configure_fixture(
      nwchem_params_pspw_pseudopotential_forces.capnp.txt.in
      nwchem_params_pspw_pseudopotential_forces
      NWChemParams
    )
    nwchemc_configure_fixture(
      potential_config_nwpw_controls.capnp.txt.in
      potential_config_nwpw_controls
      PotentialConfig
    )

    function(nwchemc_add_gradient_test target_name params_bin_target params_bin)
      add_executable("${target_name}" tests/test_nwchem_energy_gradient.c)
      target_link_libraries("${target_name}" PRIVATE nwchemc PkgConfig::CMOCKA)
      target_compile_options("${target_name}" PRIVATE ${NWCHEMC_COMMON_C_WARNINGS})
      target_compile_definitions("${target_name}" PRIVATE
        "NWCHEMC_TEST_SCRATCH_DIR=\"${NWCHEMC_TEST_SCRATCH}\""
        "NWCHEMC_TEST_PERMANENT_DIR=\"${NWCHEMC_TEST_PERMANENT}\""
      )
      add_dependencies("${target_name}" "${params_bin_target}")
      nwchemc_add_real_nwchem_test("${target_name}" "${target_name}" "${params_bin}")
    endfunction()

    nwchemc_add_gradient_test(
      nwchem-energy-gradient
      nwchem_params_bin
      "${nwchem_params_BIN}"
    )
    nwchemc_add_gradient_test(
      nwchem-energy-gradient-input-blocks
      nwchem_params_input_blocks_bin
      "${nwchem_params_input_blocks_BIN}"
    )
    nwchemc_add_gradient_test(
      nwchem-energy-gradient-structured
      nwchem_params_structured_bin
      "${nwchem_params_structured_BIN}"
    )

    add_executable(nwchem-config-parity tests/test_nwchem_config_parity.c)
    target_link_libraries(nwchem-config-parity PRIVATE nwchemc PkgConfig::CMOCKA)
    if(NWCHEMC_M_LIBRARY)
      target_link_libraries(nwchem-config-parity PRIVATE "${NWCHEMC_M_LIBRARY}")
    endif()
    target_compile_options(nwchem-config-parity PRIVATE ${NWCHEMC_COMMON_C_WARNINGS})
    target_compile_definitions(nwchem-config-parity PRIVATE
      "NWCHEMC_TEST_SCRATCH_DIR=\"${NWCHEMC_TEST_SCRATCH}\""
      "NWCHEMC_TEST_PERMANENT_DIR=\"${NWCHEMC_TEST_PERMANENT}\""
    )
    add_dependencies(nwchem-config-parity
      nwchem_params_input_blocks_bin
      potential_config_structured_bin
      force_input_h2_eq_bin
    )
    nwchemc_add_real_nwchem_test(
      nwchem-config-parity
      nwchem-config-parity
      "${nwchem_params_input_blocks_BIN}"
      "${potential_config_structured_BIN}"
      "${NWCHEMC_FORCE_INPUT_H2_EQ_BIN}"
    )

    add_executable(nwchem-rgpot-smoke tests/test_nwchem_rgpot_smoke.c)
    target_sources(nwchem-rgpot-smoke PRIVATE "${NWCHEMC_POTENTIALS_CAPNP_C}")
    target_include_directories(nwchem-rgpot-smoke PRIVATE
      ${NWCHEMC_INCLUDE_DIRS}
      "${CMAKE_CURRENT_SOURCE_DIR}/schema"
      "${NWCHEMC_SCHEMA_BUILD_DIR}"
      ${CAPNP_C_INCLUDES}
    )
    target_link_libraries(nwchem-rgpot-smoke PRIVATE
      nwchemc
      capnp_c_runtime
      PkgConfig::CMOCKA
    )
    if(NWCHEMC_M_LIBRARY)
      target_link_libraries(nwchem-rgpot-smoke PRIVATE "${NWCHEMC_M_LIBRARY}")
    endif()
    target_compile_options(nwchem-rgpot-smoke PRIVATE ${NWCHEMC_COMMON_C_WARNINGS})
    target_compile_definitions(nwchem-rgpot-smoke PRIVATE
      "NWCHEMC_TEST_SCRATCH_DIR=\"${NWCHEMC_TEST_SCRATCH}\""
      "NWCHEMC_TEST_PERMANENT_DIR=\"${NWCHEMC_TEST_PERMANENT}\""
    )
    add_dependencies(nwchem-rgpot-smoke
      potential_config_structured_bin
      force_input_h2_eq_bin
      force_input_h2_ev_bin
    )
    nwchemc_add_real_nwchem_test(
      nwchem-rgpot-smoke
      nwchem-rgpot-smoke
      "${potential_config_structured_BIN}"
      "${NWCHEMC_FORCE_INPUT_H2_EQ_BIN}"
      "${NWCHEMC_FORCE_INPUT_H2_EV_BIN}"
    )

    add_executable(nwchem-potential-config-pseudopotential
      tests/test_nwchem_potential_config_pseudopotential.c
      tests/nwchem_configured_pseudopotential_rtdb_probe.F
      tests/nwchem_rtdb_probe_get_log.F
      tests/nwchem_rtdb_probe_get_int.F
      tests/nwchem_rtdb_probe_get_dbl_array.F
      tests/nwchem_rtdb_probe_get_int_array.F
    )
    target_link_libraries(nwchem-potential-config-pseudopotential PRIVATE
      nwchemc
      PkgConfig::CMOCKA
    )
    if(NWCHEMC_M_LIBRARY)
      target_link_libraries(nwchem-potential-config-pseudopotential
        PRIVATE "${NWCHEMC_M_LIBRARY}"
      )
    endif()
    target_include_directories(nwchem-potential-config-pseudopotential PRIVATE
      ${NWCHEM_EMBED_INCLUDES}
    )
    target_compile_options(nwchem-potential-config-pseudopotential PRIVATE
      "$<$<COMPILE_LANGUAGE:C>:${NWCHEMC_COMMON_C_WARNINGS}>"
      "$<$<COMPILE_LANGUAGE:Fortran>:${NWCHEM_FORTRAN_OPTIONS}>"
      $<$<COMPILE_LANG_AND_ID:Fortran,GNU>:-std=legacy>
    )
    target_compile_definitions(nwchem-potential-config-pseudopotential PRIVATE
      NWCHEMC_HAS_NWCHEM
      GFORTRAN
      LINUX64
      EXT_INT
      OPENBLAS
      "NWCHEMC_TEST_SCRATCH_DIR=\"${NWCHEMC_TEST_SCRATCH}\""
      "NWCHEMC_TEST_PERMANENT_DIR=\"${NWCHEMC_TEST_PERMANENT}\""
    )
    add_dependencies(nwchem-potential-config-pseudopotential
      potential_config_pseudopotential_bin
      force_input_h2_eq_bin
    )
    nwchemc_add_real_nwchem_test(
      nwchem-potential-config-pseudopotential
      nwchem-potential-config-pseudopotential
      "${potential_config_pseudopotential_BIN}"
      "${NWCHEMC_FORCE_INPUT_H2_EQ_BIN}"
    )

    add_executable(nwchem-forceinput-cell-rtdb
      tests/test_nwchem_forceinput_cell_rtdb.c
      tests/nwchem_forceinput_cell_rtdb_probe.F
      tests/nwchem_rtdb_probe_get_int.F
      tests/nwchem_rtdb_probe_get_dbl_array.F
    )
    target_link_libraries(nwchem-forceinput-cell-rtdb PRIVATE
      nwchemc
      nwchemc_params
      PkgConfig::CMOCKA
    )
    target_include_directories(nwchem-forceinput-cell-rtdb PRIVATE
      ${NWCHEM_EMBED_INCLUDES}
    )
    target_compile_options(nwchem-forceinput-cell-rtdb PRIVATE
      "$<$<COMPILE_LANGUAGE:C>:${NWCHEMC_COMMON_C_WARNINGS}>"
      "$<$<COMPILE_LANGUAGE:Fortran>:${NWCHEM_FORTRAN_OPTIONS}>"
      $<$<COMPILE_LANG_AND_ID:Fortran,GNU>:-std=legacy>
    )
    target_compile_definitions(nwchem-forceinput-cell-rtdb PRIVATE
      NWCHEMC_HAS_NWCHEM
      GFORTRAN
      LINUX64
      EXT_INT
      OPENBLAS
    )
    add_dependencies(nwchem-forceinput-cell-rtdb
      force_input_h2_box_ang_bin
      force_input_h2_box_bohr_bin
      force_input_step_state_bin
    )
    nwchemc_add_real_nwchem_test(
      nwchem-forceinput-cell-rtdb
      nwchem-forceinput-cell-rtdb
      "${NWCHEMC_FORCE_INPUT_H2_BOX_ANG_BIN}"
      "${NWCHEMC_FORCE_INPUT_H2_BOX_BOHR_BIN}"
      "${NWCHEMC_FORCE_INPUT_STEP_STATE_BIN}"
    )

    add_executable(nwchem-stress tests/test_nwchem_stress.c)
    target_link_libraries(nwchem-stress PRIVATE nwchemc PkgConfig::CMOCKA)
    if(NWCHEMC_M_LIBRARY)
      target_link_libraries(nwchem-stress PRIVATE "${NWCHEMC_M_LIBRARY}")
    endif()
    target_compile_options(nwchem-stress PRIVATE ${NWCHEMC_COMMON_C_WARNINGS})
    target_compile_definitions(nwchem-stress PRIVATE
      "NWCHEMC_TEST_SCRATCH_DIR=\"${NWCHEMC_TEST_SCRATCH}\""
      "NWCHEMC_TEST_PERMANENT_DIR=\"${NWCHEMC_TEST_PERMANENT}\""
    )
    add_dependencies(nwchem-stress
      potential_config_pspw_stress_bin
      force_input_si_box_ang_bin
    )
    nwchemc_add_real_nwchem_test(
      nwchem-stress
      nwchem-stress
      "${potential_config_pspw_stress_BIN}"
      "${NWCHEMC_FORCE_INPUT_SI_BOX_ANG_BIN}"
    )

    add_executable(nwchem-pspw-pseudopotential-forces
      tests/test_nwchem_pspw_pseudopotential_forces.c
    )
    target_sources(nwchem-pspw-pseudopotential-forces
      PRIVATE "${NWCHEMC_POTENTIALS_CAPNP_C}"
    )
    target_include_directories(nwchem-pspw-pseudopotential-forces PRIVATE
      ${NWCHEMC_INCLUDE_DIRS}
      "${CMAKE_CURRENT_SOURCE_DIR}/schema"
      "${NWCHEMC_SCHEMA_BUILD_DIR}"
      ${CAPNP_C_INCLUDES}
    )
    target_link_libraries(nwchem-pspw-pseudopotential-forces PRIVATE
      nwchemc
      capnp_c_runtime
      PkgConfig::CMOCKA
    )
    if(NWCHEMC_M_LIBRARY)
      target_link_libraries(nwchem-pspw-pseudopotential-forces
        PRIVATE "${NWCHEMC_M_LIBRARY}"
      )
    endif()
    target_compile_options(nwchem-pspw-pseudopotential-forces
      PRIVATE ${NWCHEMC_COMMON_C_WARNINGS}
    )
    target_compile_definitions(nwchem-pspw-pseudopotential-forces PRIVATE
      "NWCHEMC_TEST_SCRATCH_DIR=\"${NWCHEMC_TEST_SCRATCH}\""
      "NWCHEMC_TEST_PERMANENT_DIR=\"${NWCHEMC_TEST_PERMANENT}\""
    )
    add_dependencies(nwchem-pspw-pseudopotential-forces
      nwchem_params_pspw_pseudopotential_forces_bin
      potential_config_pspw_pseudopotential_forces_bin
      force_input_si_box_ang_bin
    )
    nwchemc_add_real_nwchem_test(
      nwchem-pspw-pseudopotential-forces
      nwchem-pspw-pseudopotential-forces
      "${nwchem_params_pspw_pseudopotential_forces_BIN}"
      "${potential_config_pspw_pseudopotential_forces_BIN}"
      "${NWCHEMC_FORCE_INPUT_SI_BOX_ANG_BIN}"
    )

    add_executable(nwchem-energy-forces tests/test_nwchem_energy_forces.c)
    target_link_libraries(nwchem-energy-forces PRIVATE nwchemc PkgConfig::CMOCKA)
    if(NWCHEMC_M_LIBRARY)
      target_link_libraries(nwchem-energy-forces PRIVATE "${NWCHEMC_M_LIBRARY}")
    endif()
    target_compile_options(nwchem-energy-forces PRIVATE ${NWCHEMC_COMMON_C_WARNINGS})
    target_compile_definitions(nwchem-energy-forces PRIVATE
      "NWCHEMC_TEST_SCRATCH_DIR=\"${NWCHEMC_TEST_SCRATCH}\""
      "NWCHEMC_TEST_PERMANENT_DIR=\"${NWCHEMC_TEST_PERMANENT}\""
    )
    add_dependencies(nwchem-energy-forces nwchem_params_bin)
    nwchemc_add_real_nwchem_test(
      nwchem-energy-forces
      nwchem-energy-forces
      "${nwchem_params_BIN}"
    )

    add_executable(nwchem-session-result tests/test_nwchem_session_result.c)
    target_sources(nwchem-session-result PRIVATE "${NWCHEMC_POTENTIALS_CAPNP_C}")
    target_include_directories(nwchem-session-result PRIVATE
      ${NWCHEMC_INCLUDE_DIRS}
      "${CMAKE_CURRENT_SOURCE_DIR}/schema"
      "${NWCHEMC_SCHEMA_BUILD_DIR}"
      ${CAPNP_C_INCLUDES}
    )
    target_link_libraries(nwchem-session-result PRIVATE
      nwchemc
      capnp_c_runtime
      PkgConfig::CMOCKA
    )
    if(NWCHEMC_M_LIBRARY)
      target_link_libraries(nwchem-session-result PRIVATE "${NWCHEMC_M_LIBRARY}")
    endif()
    target_compile_options(nwchem-session-result PRIVATE ${NWCHEMC_COMMON_C_WARNINGS})
    target_compile_definitions(nwchem-session-result PRIVATE
      "NWCHEMC_TEST_SCRATCH_DIR=\"${NWCHEMC_TEST_SCRATCH}\""
      "NWCHEMC_TEST_PERMANENT_DIR=\"${NWCHEMC_TEST_PERMANENT}\""
    )
    add_dependencies(nwchem-session-result
      nwchem_params_bin
      force_input_h2_eq_bin
      force_input_h2_stretched_bin
    )
    nwchemc_add_real_nwchem_test(
      nwchem-session-result
      nwchem-session-result
      "${nwchem_params_BIN}"
      "${NWCHEMC_FORCE_INPUT_H2_EQ_BIN}"
      "${NWCHEMC_FORCE_INPUT_H2_STRETCHED_BIN}"
    )

    add_executable(nwchem-hessian tests/test_nwchem_hessian.c)
    target_link_libraries(nwchem-hessian PRIVATE nwchemc PkgConfig::CMOCKA)
    if(NWCHEMC_M_LIBRARY)
      target_link_libraries(nwchem-hessian PRIVATE "${NWCHEMC_M_LIBRARY}")
    endif()
    target_compile_options(nwchem-hessian PRIVATE ${NWCHEMC_COMMON_C_WARNINGS})
    target_compile_definitions(nwchem-hessian PRIVATE
      "NWCHEMC_TEST_SCRATCH_DIR=\"${NWCHEMC_TEST_SCRATCH}\""
      "NWCHEMC_TEST_PERMANENT_DIR=\"${NWCHEMC_TEST_PERMANENT}\""
    )
    add_dependencies(nwchem-hessian nwchem_params_hessian_bin)
    nwchemc_add_real_nwchem_test(
      nwchem-hessian
      nwchem-hessian
      "${nwchem_params_hessian_BIN}"
    )

    add_executable(nwchem-pseudopotential-rtdb
      tests/test_nwchem_pseudopotential_rtdb.c
      tests/nwchem_pseudopotential_rtdb_probe.F
      tests/nwchem_rtdb_probe_get_log.F
      tests/nwchem_rtdb_probe_get_int.F
      tests/nwchem_rtdb_probe_get_dbl_array.F
      tests/nwchem_rtdb_probe_get_int_array.F
    )
    target_link_libraries(nwchem-pseudopotential-rtdb PRIVATE
      nwchemc_legacy
      ${NWCHEM_LINK_ITEMS}
      PkgConfig::CMOCKA
    )
    target_include_directories(nwchem-pseudopotential-rtdb PRIVATE
      ${NWCHEM_EMBED_INCLUDES}
    )
    target_compile_definitions(nwchem-pseudopotential-rtdb PRIVATE
      NWCHEMC_HAS_NWCHEM
      GFORTRAN
      LINUX64
      EXT_INT
      OPENBLAS
    )
    target_compile_options(nwchem-pseudopotential-rtdb PRIVATE
      "$<$<COMPILE_LANGUAGE:C>:${NWCHEMC_COMMON_C_WARNINGS}>"
      "$<$<COMPILE_LANGUAGE:Fortran>:${NWCHEM_FORTRAN_OPTIONS}>"
      $<$<COMPILE_LANG_AND_ID:Fortran,GNU>:-std=legacy>
    )
    nwchemc_add_real_nwchem_test(
      nwchem-pseudopotential-rtdb
      nwchem-pseudopotential-rtdb
    )

    add_executable(nwchem-nwpw-rtdb
      tests/test_nwchem_nwpw_rtdb.c
      tests/nwchem_nwpw_rtdb_probe.F
      tests/nwchem_rtdb_probe_get_log.F
      tests/nwchem_rtdb_probe_get_int.F
      tests/nwchem_rtdb_probe_get_dbl_array.F
      tests/nwchem_rtdb_probe_get_int_array.F
    )
    target_link_libraries(nwchem-nwpw-rtdb PRIVATE
      nwchemc
      PkgConfig::CMOCKA
    )
    if(NWCHEMC_M_LIBRARY)
      target_link_libraries(nwchem-nwpw-rtdb PRIVATE "${NWCHEMC_M_LIBRARY}")
    endif()
    target_include_directories(nwchem-nwpw-rtdb PRIVATE
      ${NWCHEM_EMBED_INCLUDES}
    )
    target_compile_definitions(nwchem-nwpw-rtdb PRIVATE
      NWCHEMC_HAS_NWCHEM
      GFORTRAN
      LINUX64
      EXT_INT
      OPENBLAS
      "NWCHEMC_TEST_SCRATCH_DIR=\"${NWCHEMC_TEST_SCRATCH}\""
      "NWCHEMC_TEST_PERMANENT_DIR=\"${NWCHEMC_TEST_PERMANENT}\""
    )
    target_compile_options(nwchem-nwpw-rtdb PRIVATE
      "$<$<COMPILE_LANGUAGE:C>:${NWCHEMC_COMMON_C_WARNINGS}>"
      "$<$<COMPILE_LANGUAGE:Fortran>:${NWCHEM_FORTRAN_OPTIONS}>"
      $<$<COMPILE_LANG_AND_ID:Fortran,GNU>:-std=legacy>
    )
    add_dependencies(nwchem-nwpw-rtdb
      potential_config_nwpw_controls_bin
    )
    nwchemc_add_real_nwchem_test(
      nwchem-nwpw-rtdb
      nwchem-nwpw-rtdb
      direct
    )
    nwchemc_add_real_nwchem_test(
      nwchem-configured-nwpw-rtdb
      nwchem-nwpw-rtdb
      configured
      "${potential_config_nwpw_controls_BIN}"
    )

    add_executable(nwchem-geometry-basis-rtdb
      tests/test_nwchem_geometry_basis_rtdb.c
      tests/nwchem_geometry_basis_rtdb_probe.F
    )
    target_link_libraries(nwchem-geometry-basis-rtdb PRIVATE
      nwchemc_legacy
      ${NWCHEM_LINK_ITEMS}
      PkgConfig::CMOCKA
    )
    target_include_directories(nwchem-geometry-basis-rtdb PRIVATE
      ${NWCHEM_EMBED_INCLUDES}
    )
    target_compile_definitions(nwchem-geometry-basis-rtdb PRIVATE
      NWCHEMC_HAS_NWCHEM
      GFORTRAN
      LINUX64
      EXT_INT
      OPENBLAS
    )
    target_compile_options(nwchem-geometry-basis-rtdb PRIVATE
      "$<$<COMPILE_LANGUAGE:C>:${NWCHEMC_COMMON_C_WARNINGS}>"
      "$<$<COMPILE_LANGUAGE:Fortran>:${NWCHEM_FORTRAN_OPTIONS}>"
      $<$<COMPILE_LANG_AND_ID:Fortran,GNU>:-std=legacy>
    )
    nwchemc_add_real_nwchem_test(
      nwchem-geometry-basis-rtdb
      nwchem-geometry-basis-rtdb
    )

    if(NWCHEMC_NWCHEM_TEST_MPI_RANKS EQUAL 1)
      find_program(MPIRUN_EXECUTABLE mpirun)
    endif()
    if(MPIRUN_EXECUTABLE AND NWCHEMC_NWCHEM_TEST_MPI_RANKS EQUAL 1)
      add_executable(nwchem-energy-gradient-mpi
        tests/test_nwchem_energy_gradient.c
      )
      target_link_libraries(nwchem-energy-gradient-mpi PRIVATE nwchemc)
      target_link_libraries(nwchem-energy-gradient-mpi PRIVATE
        PkgConfig::CMOCKA
      )
      target_compile_options(nwchem-energy-gradient-mpi PRIVATE
        ${NWCHEMC_COMMON_C_WARNINGS}
      )
      target_compile_definitions(nwchem-energy-gradient-mpi PRIVATE
        "NWCHEMC_TEST_SCRATCH_DIR=\"${NWCHEMC_TEST_SCRATCH}\""
        "NWCHEMC_TEST_PERMANENT_DIR=\"${NWCHEMC_TEST_PERMANENT}\""
      )
      add_dependencies(nwchem-energy-gradient-mpi nwchem_params_bin)
      add_test(NAME nwchem-energy-gradient-mpi
        COMMAND
          "${MPIRUN_EXECUTABLE}" -np 2
          "$<TARGET_FILE:nwchem-energy-gradient-mpi>"
          "${nwchem_params_BIN}"
      )
      set_tests_properties(nwchem-energy-gradient-mpi PROPERTIES
        LABELS mpi
        RUN_SERIAL TRUE
        ENVIRONMENT "OMPI_MCA_prte_create_session_dirs=true"
      )
    endif()
  endif()
endif()
