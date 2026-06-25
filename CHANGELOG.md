# Changelog

All notable user-facing changes to this project are documented in this file.
This project follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

<!-- towncrier release notes start -->

## [0.1.0](https://github.com/OmniPotentRPC/nwchemc/tree/v0.1.0) - 2026-06-25

### Added

- Stable C ABI for passing flat Cap'n Proto `NWChemParams` messages into an
  embedded NWChem gradient calculation.
- Vendored `capnp-c` reader generation for the C runtime path.
- Meson build with stub tests, parser tests, serial NWChem gradient tests,
  structured stanza tests, raw input-block tests, and an MPI test.
