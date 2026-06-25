cmake_minimum_required(VERSION 3.24)

project(nwchemc VERSION 1.1.0 LANGUAGES C Fortran)

include(CTest)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(NWCHEMC_BUILD_TESTS "Build nwchemc tests" ON)
option(NWCHEMC_WITH_NWCHEM "Build the real NWChem embed library" OFF)
set(NWCHEMC_NWCHEM_ROOT "" CACHE PATH "NWChem source/build tree root")
set(NWCHEMC_NWCHEM_TARGET "LINUX64" CACHE STRING "NWChem target library directory")

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
  target_include_directories(capnp_c_runtime PUBLIC ${CAPNP_C_INCLUDES})
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
    ${NWCHEMC_INCLUDE_DIRS}
    "${CMAKE_CURRENT_SOURCE_DIR}/schema"
    "${NWCHEMC_SCHEMA_BUILD_DIR}"
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
    src/nwchem_rtdb_put_dbl.F
    src/nwchem_rtdb_put_int.F
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

  add_library(nwchemc SHARED
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
    "-lperfm"
    "-lpeigs_comm"
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

  install(TARGETS nwchemc EXPORT nwchemcTargets
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  )
  install(FILES include/nwchemc.h DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
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

  add_executable(test_capnp_parser tests/test_capnp_parser.c)
  target_link_libraries(test_capnp_parser PRIVATE
    nwchemc_params
    PkgConfig::CMOCKA
  )
  target_compile_options(test_capnp_parser PRIVATE ${NWCHEMC_COMMON_C_WARNINGS})
  add_dependencies(test_capnp_parser nwchem_parser_params_bin)
  add_test(NAME capnp-parser
    COMMAND test_capnp_parser "${NWCHEMC_PARSER_PARAMS_BIN}"
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
    force_input_step_a_bin
    force_input_step_b_bin
    force_input_step_ev_bin
  )
  add_test(NAME embed-config-cmocka
    COMMAND test_embed_config_cmocka
      "${NWCHEMC_DIRECT_DFT_PARAMS_BIN}"
      "${NWCHEMC_FORCE_INPUT_STEP_A_BIN}"
      "${NWCHEMC_FORCE_INPUT_STEP_B_BIN}"
      "${NWCHEMC_FORCE_INPUT_STEP_EV_BIN}"
  )

  if(NWCHEMC_WITH_NWCHEM)
    set(NWCHEM_ROOT "${NWCHEMC_NWCHEM_ROOT}")
    set(NWCHEMC_TEST_SCRATCH
      "${CMAKE_CURRENT_BINARY_DIR}/nwchem-test-scratch"
    )
    set(NWCHEMC_TEST_PERMANENT
      "${CMAKE_CURRENT_BINARY_DIR}/nwchem-test-permanent"
    )

    function(nwchemc_configure_params input_name output_name)
      configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/${input_name}.capnp.txt.in"
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
          "NWChemParams"
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

    nwchemc_configure_params(nwchem_params nwchem_params)
    nwchemc_configure_params(nwchem_params_input_blocks nwchem_params_input_blocks)
    nwchemc_configure_params(nwchem_params_structured nwchem_params_structured)
    nwchemc_configure_params(nwchem_params_hessian nwchem_params_hessian)

    function(nwchemc_add_gradient_test target_name params_bin_target params_bin)
      add_executable("${target_name}" tests/test_nwchem_energy_gradient.c)
      target_link_libraries("${target_name}" PRIVATE nwchemc PkgConfig::CMOCKA)
      target_compile_options("${target_name}" PRIVATE ${NWCHEMC_COMMON_C_WARNINGS})
      target_compile_definitions("${target_name}" PRIVATE
        "NWCHEMC_TEST_SCRATCH_DIR=\"${NWCHEMC_TEST_SCRATCH}\""
        "NWCHEMC_TEST_PERMANENT_DIR=\"${NWCHEMC_TEST_PERMANENT}\""
      )
      add_dependencies("${target_name}" "${params_bin_target}")
      add_test(NAME "${target_name}" COMMAND "${target_name}" "${params_bin}")
      set_tests_properties("${target_name}" PROPERTIES RUN_SERIAL TRUE)
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
    add_test(
      NAME nwchem-energy-forces
      COMMAND nwchem-energy-forces "${nwchem_params_BIN}"
    )
    set_tests_properties(nwchem-energy-forces PROPERTIES RUN_SERIAL TRUE)

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
    add_test(
      NAME nwchem-hessian
      COMMAND nwchem-hessian "${nwchem_params_hessian_BIN}"
    )
    set_tests_properties(nwchem-hessian PROPERTIES RUN_SERIAL TRUE)

    find_program(MPIRUN_EXECUTABLE mpirun)
    if(MPIRUN_EXECUTABLE)
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
