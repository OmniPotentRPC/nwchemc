project(
  'nwchemc',
  ['c', 'fortran'],
  version: '1.2.0',
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

nwchemc_features = static_library(
  'nwchemc_features',
  'src/nwchemc_features.c',
  include_directories: inc,
  c_args: common_c_args,
  install: false,
)

nwchemc_features_dep = declare_dependency(
  link_with: nwchemc_features,
  include_directories: inc,
)

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
    '-I' + nwchem_root + '/src/basis',
    '-I' + nwchem_root + '/src/include',
    '-I' + nwchem_root + '/src/nwpw/nwpwlib/utilities',
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
      'src/nwchem_rtdb_put_dbl_array.F',
      'src/nwchem_rtdb_put_dbl.F',
      'src/nwchem_rtdb_put_int_array.F',
      'src/nwchem_rtdb_put_int.F',
      'src/nwchem_rtdb_put_log_array.F',
      nwchem_root + '/src/nwpw/nwpwlib/utilities/btdb.F',
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
    'feature-inventory-check',
    python,
    args: [meson.current_source_dir() / 'tools/check_feature_inventory.py'],
    suite: ['cmocka', 'inventory'],
  )
  test(
    'features-cmocka',
    executable(
      'test_features_cmocka',
      'tests/cmocka/test_features_cmocka.c',
      dependencies: [nwchemc_features_dep, cmocka_dep],
      include_directories: inc,
      c_args: common_c_args,
    ),
    suite: ['cmocka', 'inventory'],
  )
  nwchem_parser_params_bin = custom_target(
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
  )
  nwchem_direct_dft_params_bin = custom_target(
    'nwchem_direct_dft_params_bin',
    input: 'tests/nwchem_params_direct_dft.capnp.txt',
    output: 'nwchem_params_direct_dft.bin',
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
  nwchem_compact_cells_params_bin = custom_target(
    'nwchem_compact_cells_params_bin',
    input: 'tests/nwchem_params_compact_cells.capnp.txt',
    output: 'nwchem_params_compact_cells.bin',
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
  nwchem_tce_methods_params_bin = custom_target(
    'nwchem_tce_methods_params_bin',
    input: 'tests/nwchem_params_tce_methods.capnp.txt',
    output: 'nwchem_params_tce_methods.bin',
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
  nwchem_pspspin_params_bin = custom_target(
    'nwchem_pspspin_params_bin',
    input: 'tests/nwchem_params_pspspin.capnp.txt',
    output: 'nwchem_params_pspspin.bin',
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
  nwchem_pspspin_many_params_bin = custom_target(
    'nwchem_pspspin_many_params_bin',
    input: 'tests/nwchem_params_pspspin_many.capnp.txt',
    output: 'nwchem_params_pspspin_many.bin',
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
  nwchem_nwpw_spin_mode_params_bin = custom_target(
    'nwchem_nwpw_spin_mode_params_bin',
    input: 'tests/nwchem_params_nwpw_spin_mode.capnp.txt',
    output: 'nwchem_params_nwpw_spin_mode.bin',
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
  nwchem_nwpw_allow_translation_params_bin = custom_target(
    'nwchem_nwpw_allow_translation_params_bin',
    input: 'tests/nwchem_params_nwpw_allow_translation.capnp.txt',
    output: 'nwchem_params_nwpw_allow_translation.bin',
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
  nwchem_nwpw_cutoff_alias_params_bin = custom_target(
    'nwchem_nwpw_cutoff_alias_params_bin',
    input: 'tests/nwchem_params_nwpw_cutoff_alias.capnp.txt',
    output: 'nwchem_params_nwpw_cutoff_alias.bin',
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
  nwchem_nwpw_mc_steps_params_bin = custom_target(
    'nwchem_nwpw_mc_steps_params_bin',
    input: 'tests/nwchem_params_nwpw_mc_steps.capnp.txt',
    output: 'nwchem_params_nwpw_mc_steps.bin',
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
  nwchem_brillouin_tetrahedron_params_bin = custom_target(
    'nwchem_brillouin_tetrahedron_params_bin',
    input: 'tests/nwchem_params_brillouin_tetrahedron.capnp.txt',
    output: 'nwchem_params_brillouin_tetrahedron.bin',
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
  nwchem_brillouin_dos_grid_params_bin = custom_target(
    'nwchem_brillouin_dos_grid_params_bin',
    input: 'tests/nwchem_params_brillouin_dos_grid.capnp.txt',
    output: 'nwchem_params_brillouin_dos_grid.bin',
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
  force_input_step_a_bin = custom_target(
    'force_input_step_a_bin',
    input: 'tests/force_input_step_a.capnp.txt',
    output: 'force_input_step_a.bin',
    command: [
      python,
      meson.current_source_dir() / 'tests/encode_capnp.py',
      capnp,
      meson.current_source_dir() / 'schema/Potentials.capnp',
      'ForceInput',
      '@INPUT@',
      '@OUTPUT@',
    ],
  )
  force_input_step_b_bin = custom_target(
    'force_input_step_b_bin',
    input: 'tests/force_input_step_b.capnp.txt',
    output: 'force_input_step_b.bin',
    command: [
      python,
      meson.current_source_dir() / 'tests/encode_capnp.py',
      capnp,
      meson.current_source_dir() / 'schema/Potentials.capnp',
      'ForceInput',
      '@INPUT@',
      '@OUTPUT@',
    ],
  )
  force_input_step_ev_bin = custom_target(
    'force_input_step_ev_bin',
    input: 'tests/force_input_step_ev.capnp.txt',
    output: 'force_input_step_ev.bin',
    command: [
      python,
      meson.current_source_dir() / 'tests/encode_capnp.py',
      capnp,
      meson.current_source_dir() / 'schema/Potentials.capnp',
      'ForceInput',
      '@INPUT@',
      '@OUTPUT@',
    ],
  )
  force_input_step_changed_species_bin = custom_target(
    'force_input_step_changed_species_bin',
    input: 'tests/force_input_step_changed_species.capnp.txt',
    output: 'force_input_step_changed_species.bin',
    command: [
      python,
      meson.current_source_dir() / 'tests/encode_capnp.py',
      capnp,
      meson.current_source_dir() / 'schema/Potentials.capnp',
      'ForceInput',
      '@INPUT@',
      '@OUTPUT@',
    ],
  )
  force_input_step_state_bin = custom_target(
    'force_input_step_state_bin',
    input: 'tests/force_input_step_state.capnp.txt',
    output: 'force_input_step_state.bin',
    command: [
      python,
      meson.current_source_dir() / 'tests/encode_capnp.py',
      capnp,
      meson.current_source_dir() / 'schema/Potentials.capnp',
      'ForceInput',
      '@INPUT@',
      '@OUTPUT@',
    ],
  )
  force_input_h2_eq_bin = custom_target(
    'force_input_h2_eq_bin',
    input: 'tests/force_input_h2_eq.capnp.txt',
    output: 'force_input_h2_eq.bin',
    command: [
      python,
      meson.current_source_dir() / 'tests/encode_capnp.py',
      capnp,
      meson.current_source_dir() / 'schema/Potentials.capnp',
      'ForceInput',
      '@INPUT@',
      '@OUTPUT@',
    ],
  )
  force_input_h2_stretched_bin = custom_target(
    'force_input_h2_stretched_bin',
    input: 'tests/force_input_h2_stretched.capnp.txt',
    output: 'force_input_h2_stretched.bin',
    command: [
      python,
      meson.current_source_dir() / 'tests/encode_capnp.py',
      capnp,
      meson.current_source_dir() / 'schema/Potentials.capnp',
      'ForceInput',
      '@INPUT@',
      '@OUTPUT@',
    ],
  )
  nwchem_config_options_params_bin = custom_target(
    'nwchem_config_options_params_bin',
    input: 'tests/nwchem_params_config_options.capnp.txt',
    output: 'nwchem_params_config_options.bin',
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
    'feature-driver',
    executable(
      'nwchemc_feature_driver',
      ['tools/nwchemc_feature_driver.c'] +
      (get_option('with_debug_backtrace') ? ['tools/nwchemc_debug.c'] : []) +
      nwchemc_params_sources,
      dependencies: [nwchemc_features_dep, nwchemc_stub_dep] + nwchemc_params_deps,
      include_directories: [inc, include_directories('tools')] + nwchemc_params_includes,
      c_args: common_c_args +
      (get_option('with_debug_backtrace') ? ['-DNWCHEMC_WITH_DEBUG_BACKTRACE=1'] : []),
    ),
    args: ['all', nwchem_parser_params_bin],
    suite: ['cmocka', 'inventory', 'drivers'],
  )
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
      nwchem_parser_params_bin,
      nwchem_nwpw_spin_mode_params_bin,
      nwchem_nwpw_allow_translation_params_bin,
      nwchem_nwpw_cutoff_alias_params_bin,
      nwchem_nwpw_mc_steps_params_bin,
      nwchem_brillouin_tetrahedron_params_bin,
      nwchem_brillouin_dos_grid_params_bin,
    ],
  )
  test(
    'embed-config-cmocka',
    executable(
      'test_embed_config_cmocka',
      ['tests/cmocka/test_embed_config_cmocka.c', 'src/nwchemc.c'] + nwchemc_params_sources,
      include_directories: [inc] + nwchemc_params_includes,
      dependencies: nwchemc_params_deps + [cmocka_dep],
      c_args: common_c_args + ['-DNWCHEMC_HAS_NWCHEM=1'],
    ),
    args: [
      nwchem_direct_dft_params_bin,
      nwchem_config_options_params_bin,
      nwchem_pspspin_params_bin,
      nwchem_pspspin_many_params_bin,
      nwchem_nwpw_spin_mode_params_bin,
      nwchem_nwpw_allow_translation_params_bin,
      nwchem_nwpw_cutoff_alias_params_bin,
      nwchem_nwpw_mc_steps_params_bin,
      nwchem_brillouin_tetrahedron_params_bin,
      nwchem_brillouin_dos_grid_params_bin,
      force_input_step_a_bin,
      force_input_step_b_bin,
      force_input_step_ev_bin,
      force_input_step_changed_species_bin,
      force_input_step_state_bin,
      nwchem_tce_methods_params_bin,
      nwchem_compact_cells_params_bin,
    ],
    suite: ['cmocka', 'parser'],
  )
  test(
    'params-render-cmocka',
    executable(
      'test_params_render_cmocka',
      ['tests/cmocka/test_params_render_cmocka.c'] + nwchemc_params_sources,
      include_directories: [inc] + nwchemc_params_includes,
      dependencies: nwchemc_params_deps + [cmocka_dep],
      c_args: common_c_args,
    ),
    args: [nwchem_config_options_params_bin, nwchem_direct_dft_params_bin],
    suite: ['cmocka', 'parser', 'coverage'],
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
      'nwchem-energy-forces',
      executable(
        'test_nwchem_energy_forces',
        'tests/test_nwchem_energy_forces.c',
        include_directories: inc,
        dependencies: [math_dep, cmocka_dep],
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
      'nwchem-session-result',
      executable(
        'test_nwchem_session_result',
        'tests/test_nwchem_session_result.c',
        include_directories: [inc] + nwchemc_params_includes,
        dependencies: [math_dep, cmocka_dep] + nwchemc_params_deps,
        link_with: nwchemc,
        c_args: common_c_args + [
          '-DNWCHEMC_TEST_SCRATCH_DIR="' + test_scratch_dir + '"',
          '-DNWCHEMC_TEST_PERMANENT_DIR="' + test_permanent_dir + '"',
        ],
      ),
      args: [
        nwchem_params_bin,
        force_input_h2_eq_bin,
        force_input_h2_stretched_bin,
      ],
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
    test(
      'nwchem-pseudopotential-rtdb',
      executable(
        'test_nwchem_pseudopotential_rtdb',
        [
          'tests/test_nwchem_pseudopotential_rtdb.c',
          'tests/nwchem_pseudopotential_rtdb_probe.F',
        ],
        include_directories: inc,
        dependencies: [cmocka_dep],
        link_with: nwchemc_legacy,
        link_args: nwchem_link_args,
        c_args: common_c_args,
        fortran_args: nwchem_legacy_args,
      ),
      is_parallel: false,
    )
    test(
      'nwchem-geometry-basis-rtdb',
      executable(
        'test_nwchem_geometry_basis_rtdb',
        [
          'tests/test_nwchem_geometry_basis_rtdb.c',
          'tests/nwchem_geometry_basis_rtdb_probe.F',
        ],
        include_directories: inc,
        dependencies: [cmocka_dep],
        link_with: nwchemc_legacy,
        link_args: nwchem_link_args,
        c_args: common_c_args,
        fortran_args: nwchem_legacy_args,
      ),
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
