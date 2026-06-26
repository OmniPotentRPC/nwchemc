# Changelog

All notable user-facing changes to this project are documented in this file.
This project follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

<!-- towncrier release notes start -->

## [1.2.0](https://github.com/OmniPotentRPC/nwchemc/tree/v1.2.0) - 2026-06-26

### Added

- Classic CCSD scalar controls now have a structured Cap'n Proto stanza with full-deck rendering and direct `ccsd:*` RTDB writes in embed builds. ([#ccsd-direct-controls](https://github.com/OmniPotentRPC/nwchemc/issues/ccsd-direct-controls))
- Added one-shot, session, and ForceInput C ABI entry points for total dipole extraction through NWChem RTDB state. ([#dipole-abi](https://github.com/OmniPotentRPC/nwchemc/issues/dipole-abi))
- Added structured NWPW Born-Oppenheimer controls for balance, BO steps, BO time
  step, BO algorithm, BO fake mass, and scaling, with embed builds writing the
  matching NWPW/CPMD RTDB values directly. ([#nwpw-bo-controls](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-bo-controls))
- Added structured NWPW cutoff controls and promoted them to direct RTDB writes for embed calculations. ([#nwpw-cutoff-controls](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-cutoff-controls))
- Added structured NWPW `exchangeCorrelation` support for simple named
  functionals, with embed builds writing the matching PSPW/BAND/CPSD/CPMD RTDB
  keys directly. ([#nwpw-exchange-correlation](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-exchange-correlation))
- Added structured NWPW execution controls for processor dimensions, spin-orbit,
  and parallel I/O, with embed builds writing the matching RTDB values directly. ([#nwpw-execution-controls](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-execution-controls))
- Added structured NWPW fractional orbital and smear controls, with embed builds
  writing the matching `nwpw:fractional_*` RTDB values directly. ([#nwpw-fractional-smearing](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-fractional-smearing))
- Added structured NWPW output and motion filename fields, with embed builds
  writing the matching CPMD and NWPW RTDB string values directly. ([#nwpw-motion-filenames](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-motion-filenames))
- Structured NWPW Nose-Hoover thermostat fields now render full-deck syntax and write the paired CPMD/NWPW RTDB keys directly in embed builds. ([#nwpw-nose-controls](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-nose-controls))
- Added structured NWPW virtual orbital, LCAO, and Ewald grid controls, with
  embed builds writing the matching `nwpw:*` RTDB values directly. ([#nwpw-orbital-grid-controls](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-orbital-grid-controls))
- Added structured NWPW state controls for cell names, wavefunction filenames, fake mass, time step, loop, and tolerance RTDB values. ([#nwpw-state-controls](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-state-controls))
- Added one-shot ForceInput Hessian and dipole C ABI wrappers that reuse the session validation path without requiring callers to manage a session. ([#oneshot-forceinput-wrappers](https://github.com/OmniPotentRPC/nwchemc/issues/oneshot-forceinput-wrappers))
- Added the `nwchemc_hessian()` C ABI, cmocka-backed C tests, and structured Cap'n Proto pseudopotential stanzas for NWPW library selection. ([#v2-hessian-pseudopotential](https://github.com/OmniPotentRPC/nwchemc/issues/v2-hessian-pseudopotential))

### Fixed

- Embed rendering omits explicit task stanzas and relies on the C ABI entry point to write NWChem task RTDB keys directly. ([#embed-task-direct](https://github.com/OmniPotentRPC/nwchemc/issues/embed-task-direct))
- Persistent sessions now reject parameter replacement after the first accepted topology; create a separate session for a new post-step configuration. ([#session-param-replacement](https://github.com/OmniPotentRPC/nwchemc/issues/session-param-replacement))
- Persistent sessions reject later force/result/Hessian steps whose atom count or species ordering differs from the session's first accepted topology. ([#session-topology](https://github.com/OmniPotentRPC/nwchemc/issues/session-topology))


## [1.1.0](https://github.com/OmniPotentRPC/nwchemc/tree/v1.1.0) - 2026-06-25

### Added

- Expand typed NWChem module stanzas to cover the curated local NWChem
  input-handler set and add a schema coverage test. ([#module-coverage](https://github.com/OmniPotentRPC/nwchemc/issues/module-coverage))


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
