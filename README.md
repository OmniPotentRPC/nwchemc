# nwchemc

Stable C ABI for embedding NWChem from language-neutral Cap'n Proto
`NWChemParams` messages.

`nwchemc` builds `libnwchemc.so` from a C ABI layer, modern Fortran
`iso_c_binding` / `iso_fortran_env` bridge code, and the legacy NWChem embed
entry points required for RTDB, geometry, basis, energy, and gradient calls.

The public ABI does not expose C++ or Rust types:

```c
int nwchemc_set_params(const void *params_capnp, size_t params_capnp_size_bytes);
NWChemCResult nwchemc_energy_gradient(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *grad_h_bohr);
```

`params_capnp` is an unpacked flat Cap'n Proto message whose root is
`NWChemParams` from `schema/Potentials.capnp`. It can be produced by pycapnp,
rgpot, mmap-backed readers, or another Cap'n Proto binding that writes the
standard flat stream format.

## Build

Frontend/stub build without NWChem:

```bash
meson setup build -Dwith_tests=true
meson compile -C build
meson test -C build --print-errorlogs
```

Embed build against an NWChem source/install tree:

```bash
export NWCHEM_TOP=/path/to/nwchem
meson setup build-nwchem \
  -Dwith_nwchem=true \
  -Dnwchem_root="$NWCHEM_TOP" \
  -Dnwchem_target=LINUX64 \
  -Dwith_tests=true
meson compile -C build-nwchem
```

The resulting shared library exports `nwchemc_*` symbols. Downstream projects
load it with `dlopen()` and pass the serialized `NWChemParams` message bytes
directly.

## Rust

Rust plus `cbindgen` is a viable internal implementation option if this project
needs richer Cap'n Proto validation and schema handling than the C parser. The
ABI remains the checked-in C header either way. The current scaffold keeps the
build C/Fortran-first so the NWChem embed link is explicit and does not require
Cargo for users who only need the C shim.
