# Changelog

All notable user-facing changes to this project are documented in this file.
This project follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

<!-- towncrier release notes start -->

## [1.0.0](https://github.com/OmniPotentRPC/nwchemc/tree/v1.0.0) - 2026-06-25

### Added

- Added CMake support for the stub/parser build and the real NWChem C/Fortran
  shared library path, including CTest coverage for serial and MPI gradient
  calculations. ([#cmake](https://github.com/OmniPotentRPC/nwchemc/issues/cmake))
- Add typed NWChem module stanzas for common NWChem input blocks while preserving
  tokenized directives and raw input fallback paths. ([#module-stanzas](https://github.com/OmniPotentRPC/nwchemc/issues/module-stanzas))

### Fixed

- Clarify and validate that NWChem embed builds require a source/build tree rather
  than an executable runtime prefix. ([#embed-root-validation](https://github.com/OmniPotentRPC/nwchemc/issues/embed-root-validation))


## [0.1.0](https://github.com/OmniPotentRPC/nwchemc/tree/v0.1.0) - 2026-06-25

### Added

- Stable C ABI for passing flat Cap'n Proto `NWChemParams` messages into an
  embedded NWChem gradient calculation.
- Vendored `capnp-c` reader generation for the C runtime path.
- Meson build with stub tests, parser tests, serial NWChem gradient tests,
  structured stanza tests, raw input-block tests, and an MPI test.
