project(
  'nwchemc',
  ['c', 'fortran'],
  version: '0.1.0',
  default_options: ['warning_level=2', 'c_std=c11'],
)

cc = meson.get_compiler('c')
fc = meson.get_compiler('fortran')

inc = include_directories('include', 'src')
nwchem_root = get_option('nwchem_root')
nwchem_target = get_option('nwchem_target')
has_nwchem = get_option('with_nwchem') and nwchem_root != ''

common_c_args = []
if cc.get_id() != 'msvc'
  common_c_args += ['-Wall', '-Wextra', '-Wpedantic']
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
  ]
  nwchemc_legacy = static_library(
    'nwchemc_legacy',
    'src/nwchem_embed_legacy.F',
    include_directories: inc,
    fortran_args: nwchem_legacy_args,
    install: false,
  )
  nwchem_link_args = [
    '-L' + nwchem_root + '/lib/' + nwchem_target,
    '-L' + nwchem_root + '/src/tools/install/lib',
    '-L' + nwchem_root + '/src/libext/lib',
    '-Wl,--start-group',
    '-lnwctask',
    '-lnwdft',
    '-lddscf',
    '-lgradients',
    '-lnwints',
    '-lnwcutil',
    '-lga',
    '-larmci',
    '-lcomex',
    '-lnwc_openblas',
    '-Wl,--end-group',
    '-lgfortran',
    '-lpthread',
    '-lm',
  ]
  if host_machine.system() == 'linux'
    nwchem_link_args += ['-Wl,--unresolved-symbols=ignore-all']
  endif
  nwchemc = shared_library(
    'nwchemc',
    ['src/nwchemc.c', 'src/nwchem_embed_c_api.f90'],
    include_directories: inc,
    c_args: nwchem_c_args,
    fortran_args: nwchem_f90_args,
    link_with: nwchemc_legacy,
    link_args: nwchem_link_args,
    install: true,
  )
endif

if get_option('with_tests')
  test(
    'stub-abi',
    executable(
      'test_stub_abi',
      'tests/test_stub_abi.c',
      dependencies: [nwchemc_stub_dep],
      include_directories: inc,
      c_args: common_c_args,
    ),
  )
  test(
    'capnp-parser',
    executable(
      'test_capnp_parser',
      ['tests/test_capnp_parser.c', 'src/nwchemc_params.c'],
      include_directories: inc,
      c_args: common_c_args,
    ),
  )
endif
