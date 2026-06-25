project(
  'nwchemc',
  ['c', 'fortran'],
  version: '1.1.0',
  default_options: ['warning_level=2', 'c_std=c11'],
)

cc = meson.get_compiler('c')
fc = meson.get_compiler('fortran')
python = find_program('python3')
math_dep = cc.find_library('m', required: false)
cmocka_dep = dependency('cmocka', required: get_option('with_tests'))

inc = include_directories('include', 'src')
nwchem_root = get_option('nwchem_root')
nwchem_target = get_option('nwchem_target')
has_nwchem = get_option('with_nwchem') and nwchem_root != ''
need_capnp_c = get_option('with_tests') or has_nwchem

common_c_args = []
if cc.get_id() != 'msvc'
  common_c_args += ['-Wall', '-Wextra', '-Wpedantic']
endif

nwchemc_params_sources = []
nwchemc_params_deps = []
nwchemc_params_includes = []
if has_nwchem
  nwchem_root_probe = run_command(
    python,
    [
      meson.current_source_dir() / 'tools/nwchem_root_probe.py',
      nwchem_root,
      '--target',
      nwchem_target,
    ],
    check: false,
  )
  if nwchem_root_probe.returncode() != 0
    message(nwchem_root_probe.stdout().strip())
    message(nwchem_root_probe.stderr().strip())
    error('NWChem embed root validation failed')
  endif
endif
if need_capnp_c
  capnp = find_program('capnp')
  capnp_c_project = subproject('c-capnproto')
  capnp_c_dep = capnp_c_project.get_variable('capnp_c_dep')
  capnpc_c = capnp_c_project.get_variable('capnpc_c')
  subdir('schema')
  potentials_capnp_c_dep = declare_dependency(sources: potentials_capnp_c)
  nwchemc_params_sources = ['src/nwchemc_params.c', potentials_capnp_c_src]
  nwchemc_params_deps = [capnp_c_dep, potentials_capnp_c_dep]
  nwchemc_params_includes = [include_directories('schema')]
endif

nwchemc_stub = static_library(
  'nwchemc_stub',
  'src/nwchemc_stub.c',
  include_directories: inc,
  c_args: common_c_args,
  install: false,
)

nwchemc_stub_dep = declare_dependency(
  link_with: nwchemc_stub,
  include_directories: inc,
)

if has_nwchem
  nwchem_config_h = nwchem_root / 'src/config/nwchem_config.h'
  nwchem_module_libs_raw = run_command(
    python,
    [
      '-c',
      'import pathlib, re, sys; text = pathlib.Path(sys.argv[1]).read_text(); match = re.search(r"^NW_MODULE_LIBS\\s*=\\s*(.*)$", text, re.M); sys.exit(1) if not match else print(match.group(1))',
      nwchem_config_h,
    ],
    check: true,
  ).stdout().strip()
  nwchem_module_libs = nwchem_module_libs_raw.split()
  ga_config = find_program(nwchem_root / 'src/tools/install/bin/ga-config')
  ga_ldflags = run_command(ga_config, '--ldflags', check: true).stdout().strip().split()
  ga_libs = run_command(ga_config, '--libs', check: true).stdout().strip().split()
  mpi_libdirs = run_command(
    python,
    [
      '-c',
      'import pathlib; dirs = ["/usr/lib", "/usr/lib64", "/usr/local/lib"]; print(" ".join("-L" + d for d in dirs if any(pathlib.Path(d).glob("libmpi.so*"))))',
    ],
    check: true,
  ).stdout().strip().split()
  nwchem_c_args = common_c_args + [
    '-DNWCHEMC_HAS_NWCHEM=1',
    '-DGFORTRAN=1',
    '-I' + nwchem_root + '/src/include',
    '-I' + nwchem_root + '/src/tools/install/include',
  ]
  nwchem_f90_args = [
    '-DNWCHEMC_HAS_NWCHEM=1',
    '-fdefault-integer-8',
    '-ffast-math',
  ]
  nwchem_legacy_args = [
    '-DNWCHEMC_HAS_NWCHEM=1',
    '-I' + nwchem_root + '/src/include',
    '-I' + nwchem_root + '/src/util',
    '-I' + nwchem_root + '/src/tools/install/include',
    '-fdefault-integer-8',
    '-ffast-math',
    '-std=legacy',
    '-DGFORTRAN',
    '-DLINUX64',
    '-DEXT_INT',
    '-DOPENBLAS',
    '-Wno-unused-parameter',
  ]
  nwchem_utility_libs = [
    '-lperfm',
    '-lpeigs_comm',
  ]
  nwchemc_legacy = static_library(
    'nwchemc_legacy',
    [
      'src/nwchem_embed_legacy.F',
      'src/nwchem_rtdb_put_dbl.F',
      'src/nwchem_rtdb_put_int.F',
    ],
    include_directories: inc,
    fortran_args: nwchem_legacy_args,
    install: false,
  )
  nwchem_link_args = [
    '-L' + nwchem_root + '/lib/' + nwchem_target,
    '-L' + nwchem_root + '/src/tools/install/lib',
    '-L' + nwchem_root + '/src/libext/lib',
  ] + ga_ldflags + mpi_libdirs + [
    '-Wl,--start-group',
    nwchem_root + '/src/stubs.o',
    '-lnwctask',
  ] + nwchem_module_libs + nwchem_utility_libs + [
    '-lcomex',
  ] + ga_libs + [
    '-Wl,--end-group',
    '-lgfortran',
    '-lpthread',
    '-lm',
  ]
  if host_machine.system() == 'linux' and cc.get_argument_syntax() == 'gcc'
    nwchem_link_args += ['-Wl,-z,defs']
  endif
  if host_machine.system() == 'linux' and cc.get_argument_syntax() == 'gcc'
    message('Checking NWChem archives can link into libnwchemc.so')
    nwchem_link_probe = run_command(
      cc,
      [
        '-shared',
        '-fPIC',
        meson.current_source_dir() / 'tools/nwchem_link_probe.c',
        '-o',
        meson.current_build_dir() / 'nwchem-link-probe.so',
      ] + nwchem_link_args,
      check: false,
    )
    if nwchem_link_probe.returncode() != 0
      message(nwchem_link_probe.stderr().strip())
      error('NWChem archives and link dependencies cannot form libnwchemc.so for ' + nwchem_target)
    endif
  endif
  nwchemc = shared_library(
    'nwchemc',
    ['src/nwchemc.c', 'src/nwchem_embed_c_api.f90'] + nwchemc_params_sources,
    include_directories: [inc] + nwchemc_params_includes,
    dependencies: nwchemc_params_deps,
    c_args: nwchem_c_args,
    fortran_args: nwchem_f90_args,
    link_with: nwchemc_legacy,
    link_args: nwchem_link_args,
    install: true,
  )
endif

if get_option('with_tests')
  probe_test_env = [
    'TMPDIR=' + meson.current_build_dir(),
    'NWCHEMC_TEST_SCRATCH_ROOT=' + meson.current_build_dir() / 'probe-tests',
  ]
  test(
    'nwchem-root-probe',
    python,
    args: [meson.current_source_dir() / 'tests/test_nwchem_root_probe.py'],
    env: probe_test_env,
  )
  test(
    'schema-module-coverage',
    python,
    args: [
      meson.current_source_dir() / 'tests/test_schema_module_coverage.py',
    ],
  )
  cmake_prog = find_program('cmake', required: false)
  if cmake_prog.found()
    meson_prog = find_program('meson')
    test(
      'nwchem-build-root-probe',
      python,
      args: [
        meson.current_source_dir() / 'tests/test_build_embed_root_probe.py',
      ],
      env: probe_test_env + [
        'CMAKE=' + cmake_prog.full_path(),
        'MESON=' + meson_prog.full_path(),
      ],
      timeout: 120,
    )
  endif
  test(
    'stub-abi',
    executable(
      'test_stub_abi',
      'tests/test_stub_abi.c',
      dependencies: [nwchemc_stub_dep, cmocka_dep],
      include_directories: inc,
      c_args: common_c_args,
    ),
  )
  test(
    'capnp-parser',
    executable(
      'test_capnp_parser',
      ['tests/test_capnp_parser.c'] + nwchemc_params_sources,
      include_directories: [inc] + nwchemc_params_includes,
      dependencies: nwchemc_params_deps + [cmocka_dep],
      c_args: common_c_args,
    ),
    args: [
      custom_target(
        'nwchem_parser_params_bin',
        input: 'tests/nwchem_params_parser.capnp.txt',
        output: 'nwchem_params_parser.bin',
        command: [
          python,
          meson.current_source_dir() / 'tests/encode_capnp.py',
          capnp,
          meson.current_source_dir() / 'schema/Potentials.capnp',
          'NWChemParams',
          '@INPUT@',
          '@OUTPUT@',
        ],
      ),
    ],
  )
  if has_nwchem
    test_scratch_dir = meson.current_build_dir() / 'nwchem-test-scratch'
    test_permanent_dir = meson.current_build_dir() / 'nwchem-test-permanent'
    params_conf = configuration_data()
    params_conf.set('NWCHEM_ROOT', nwchem_root)
    params_conf.set('NWCHEMC_TEST_SCRATCH', test_scratch_dir)
    params_conf.set('NWCHEMC_TEST_PERMANENT', test_permanent_dir)
    nwchem_params_text = configure_file(
      input: 'tests/nwchem_params.capnp.txt.in',
      output: 'nwchem_params.capnp.txt',
      configuration: params_conf,
    )
    nwchem_params_bin = custom_target(
      'nwchem_params_bin',
      input: nwchem_params_text,
      output: 'nwchem_params.bin',
      command: [
        python,
        meson.current_source_dir() / 'tests/encode_capnp.py',
        capnp,
        meson.current_source_dir() / 'schema/Potentials.capnp',
        'NWChemParams',
        '@INPUT@',
        '@OUTPUT@',
      ],
    )
    nwchem_input_blocks_params_text = configure_file(
      input: 'tests/nwchem_params_input_blocks.capnp.txt.in',
      output: 'nwchem_params_input_blocks.capnp.txt',
      configuration: params_conf,
    )
    nwchem_input_blocks_params_bin = custom_target(
      'nwchem_input_blocks_params_bin',
      input: nwchem_input_blocks_params_text,
      output: 'nwchem_params_input_blocks.bin',
      command: [
        python,
        meson.current_source_dir() / 'tests/encode_capnp.py',
        capnp,
        meson.current_source_dir() / 'schema/Potentials.capnp',
        'NWChemParams',
        '@INPUT@',
        '@OUTPUT@',
      ],
    )
    nwchem_structured_params_text = configure_file(
      input: 'tests/nwchem_params_structured.capnp.txt.in',
      output: 'nwchem_params_structured.capnp.txt',
      configuration: params_conf,
    )
    nwchem_structured_params_bin = custom_target(
      'nwchem_structured_params_bin',
      input: nwchem_structured_params_text,
      output: 'nwchem_params_structured.bin',
      command: [
        python,
        meson.current_source_dir() / 'tests/encode_capnp.py',
        capnp,
        meson.current_source_dir() / 'schema/Potentials.capnp',
        'NWChemParams',
        '@INPUT@',
        '@OUTPUT@',
      ],
    )
    nwchem_hessian_params_text = configure_file(
      input: 'tests/nwchem_params_hessian.capnp.txt.in',
      output: 'nwchem_params_hessian.capnp.txt',
      configuration: params_conf,
    )
    nwchem_hessian_params_bin = custom_target(
      'nwchem_hessian_params_bin',
      input: nwchem_hessian_params_text,
      output: 'nwchem_params_hessian.bin',
      command: [
        python,
        meson.current_source_dir() / 'tests/encode_capnp.py',
        capnp,
        meson.current_source_dir() / 'schema/Potentials.capnp',
        'NWChemParams',
        '@INPUT@',
        '@OUTPUT@',
      ],
    )
    test(
      'nwchem-energy-gradient',
      executable(
        'test_nwchem_energy_gradient',
        'tests/test_nwchem_energy_gradient.c',
        include_directories: inc,
        dependencies: [cmocka_dep],
        link_with: nwchemc,
        c_args: common_c_args + [
          '-DNWCHEMC_TEST_SCRATCH_DIR="' + test_scratch_dir + '"',
          '-DNWCHEMC_TEST_PERMANENT_DIR="' + test_permanent_dir + '"',
        ],
      ),
      args: [nwchem_params_bin],
      is_parallel: false,
    )
    test(
      'nwchem-energy-gradient-input-blocks',
      executable(
        'test_nwchem_energy_gradient_input_blocks',
        'tests/test_nwchem_energy_gradient.c',
        include_directories: inc,
        dependencies: [cmocka_dep],
        link_with: nwchemc,
        c_args: common_c_args + [
          '-DNWCHEMC_TEST_SCRATCH_DIR="' + test_scratch_dir + '"',
          '-DNWCHEMC_TEST_PERMANENT_DIR="' + test_permanent_dir + '"',
        ],
      ),
      args: [nwchem_input_blocks_params_bin],
      is_parallel: false,
    )
    test(
      'nwchem-energy-gradient-structured',
      executable(
        'test_nwchem_energy_gradient_structured',
        'tests/test_nwchem_energy_gradient.c',
        include_directories: inc,
        dependencies: [cmocka_dep],
        link_with: nwchemc,
        c_args: common_c_args + [
          '-DNWCHEMC_TEST_SCRATCH_DIR="' + test_scratch_dir + '"',
          '-DNWCHEMC_TEST_PERMANENT_DIR="' + test_permanent_dir + '"',
        ],
      ),
      args: [nwchem_structured_params_bin],
      is_parallel: false,
    )
    test(
      'nwchem-hessian',
      executable(
        'test_nwchem_hessian',
        'tests/test_nwchem_hessian.c',
        include_directories: inc,
        dependencies: [math_dep, cmocka_dep],
        link_with: nwchemc,
        c_args: common_c_args + [
          '-DNWCHEMC_TEST_SCRATCH_DIR="' + test_scratch_dir + '"',
          '-DNWCHEMC_TEST_PERMANENT_DIR="' + test_permanent_dir + '"',
        ],
      ),
      args: [nwchem_hessian_params_bin],
      is_parallel: false,
    )
    mpirun = find_program('mpirun', required: false)
    if mpirun.found()
      nwchem_mpi_test = executable(
        'test_nwchem_energy_gradient_mpi',
        'tests/test_nwchem_energy_gradient.c',
        include_directories: inc,
        dependencies: [cmocka_dep],
        link_with: nwchemc,
        c_args: common_c_args + [
          '-DNWCHEMC_TEST_SCRATCH_DIR="' + test_scratch_dir + '"',
          '-DNWCHEMC_TEST_PERMANENT_DIR="' + test_permanent_dir + '"',
        ],
      )
      test(
        'nwchem-energy-gradient-mpi',
        mpirun,
        args: ['-np', '2', nwchem_mpi_test, nwchem_params_bin],
        protocol: 'exitcode',
        suite: 'mpi',
        depends: nwchem_params_bin,
        workdir: meson.current_build_dir(),
        env: ['OMPI_MCA_prte_create_session_dirs=true'],
        is_parallel: false,
      )
    endif
  endif
endif
