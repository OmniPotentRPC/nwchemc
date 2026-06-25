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
standard flat stream format. `nwchemc` reads that message through generated
`capnp-c` bindings; it does not define a parallel user configuration format.

## Build

Frontend/stub build without NWChem:

```bash
meson setup build -Dwith_tests=true
meson compile -C build
meson test -C build --print-errorlogs
```

Embed build against an NWChem source/build tree:

```bash
export NWCHEM_TOP=/path/to/nwchem
meson setup build-nwchem \
  -Dwith_nwchem=true \
  -Dnwchem_root="$NWCHEM_TOP" \
  -Dnwchem_target=LINUX64 \
  -Dwith_tests=true
meson compile -C build-nwchem
```

`libnwchemc.so` links NWChem into a shared library, so `nwchem_root` must point
at the NWChem build-tree layout containing `src/config/nwchem_config.h`,
`src/tools/install/bin/ga-config`, `src/stubs.o`, source include directories,
and `lib/$NWCHEM_TARGET`. The conda-forge `nwchem` package is useful as an
executable/runtime package, but it is not the embed SDK shape required by this
in-process ABI.

Static NWChem archives must be built as position-independent code. If Meson
fails during the NWChem link probe with `relocation R_X86_64_PC32 ... can not
be used when making a shared object`, rebuild the NWChem archives with PIC
flags for the selected target.

The resulting shared library exports `nwchemc_*` symbols. Downstream projects
load it with `dlopen()` and pass the serialized `NWChemParams` message bytes
directly.

## Cap'n Proto C

The build vendors the MIT-licensed `capnp-c` runtime and compiler plugin. Meson
builds `capnpc-c`, generates C bindings for `schema/Potentials.capnp`, and
compiles those generated readers into `nwchemc`. The public ABI remains the
checked-in C header and serialized Cap'n Proto bytes.
