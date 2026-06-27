# nwchemc

Stable C ABI for embedding NWChem from language-neutral Cap'n Proto
`NWChemParams` messages.

`nwchemc` builds `libnwchemc.so` from a C ABI layer, modern Fortran
`iso_c_binding` / `iso_fortran_env` bridge code, and the legacy NWChem embed
entry points required for RTDB, geometry, basis, energy, gradient, Hessian, and
dipole, quadrupole, stress, optimization, and frequency calls.

The public ABI does not expose C++ or Rust types:

```c
int nwchemc_set_params(const void *params_capnp, size_t params_capnp_size_bytes);
NWChemCResult nwchemc_energy_gradient(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *grad_h_bohr);
NWChemCResult nwchemc_hessian(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *hessian_h_bohr2);
NWChemCResult nwchemc_dipole(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *dipole_au);
NWChemCResult nwchemc_stress(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *stress_au);
NWChemCSession *nwchemc_session_create(
    const void *params_capnp, size_t params_capnp_size_bytes);
size_t nwchemc_potential_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes);
NWChemCResult nwchemc_session_calculate_result(
    NWChemCSession *session,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
NWChemCResult nwchemc_calculate_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
NWChemCResult nwchemc_calculate_hessian(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *hessian_h_bohr2, size_t hessian_len);
size_t nwchemc_hessian_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes);
NWChemCResult nwchemc_session_calculate_hessian_result(
    NWChemCSession *session,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
NWChemCResult nwchemc_calculate_hessian_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
NWChemCResult nwchemc_calculate_dipole(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *dipole_au, size_t dipole_len);
size_t nwchemc_dipole_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes);
NWChemCResult nwchemc_session_calculate_dipole_result(
    NWChemCSession *session,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
NWChemCResult nwchemc_calculate_dipole_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
size_t nwchemc_quadrupole_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes);
NWChemCResult nwchemc_session_calculate_quadrupole_result(
    NWChemCSession *session,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
NWChemCResult nwchemc_calculate_quadrupole_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
NWChemCResult nwchemc_calculate_stress(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *stress_au, size_t stress_len);
size_t nwchemc_stress_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes);
NWChemCResult nwchemc_session_calculate_stress_result(
    NWChemCSession *session,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
NWChemCResult nwchemc_calculate_stress_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
size_t nwchemc_optimize_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes);
NWChemCResult nwchemc_session_calculate_optimize_result(
    NWChemCSession *session,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
NWChemCResult nwchemc_calculate_optimize_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
size_t nwchemc_frequencies_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes);
NWChemCResult nwchemc_session_calculate_frequencies_result(
    NWChemCSession *session,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
NWChemCResult nwchemc_calculate_frequencies_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
void nwchemc_session_destroy(NWChemCSession *session);
```

`params_capnp` is an unpacked flat Cap'n Proto message whose root is
`NWChemParams` from `schema/Potentials.capnp`. It can be produced by pycapnp,
rgpot, mmap-backed readers, or another Cap'n Proto binding that writes the
standard flat stream format. `nwchemc` reads that message through generated
`capnp-c` bindings; it does not define a parallel user configuration format.

Long-running callers should create one `NWChemCSession` from `NWChemParams`,
then pass a serialized `ForceInput` for each geometry step. The result sizing
helper parses the step message only, so callers can allocate or reuse an
unpacked flat `PotentialResult` buffer before calling
`nwchemc_session_calculate_result()`. The evaluating call keeps
`NWChemCResult.energy_h` in Hartree and writes `PotentialResult.energy` /
`PotentialResult.forces` in `ForceInput.energyUnit` and
`ForceInput.energyUnit / ForceInput.lengthUnit`.
`ForceInput.hasCharge` / `ForceInput.charge` and
`ForceInput.hasMultiplicity` / `ForceInput.multiplicity` can override the
session charge or spin multiplicity for that serialized step; unset flags keep
the values from `NWChemParams`.
The first accepted session evaluation fixes the atom count and ordered
atomic-number list for that session; later steps may change coordinates, units,
and cell vectors, but atom-count or species changes require a separate session.
Session calls reject topology-changing steps instead of resetting the handle
implicitly; callers that need a new topology or a new post-step configuration
create a separate session.
`nwchemc_calculate_result()` offers the same `NWChemParams + ForceInput`
carrier for one-shot callers and delegates through the session result path;
callers with multiple steps should reuse `NWChemCSession`.
`nwchemc_session_calculate_hessian_result()` and
`nwchemc_calculate_hessian_result()` populate `PotentialResult.hessian` in
`ForceInput.energyUnit / ForceInput.lengthUnit^2`. Dipole and quadrupole
result-carrier wrappers populate `PotentialResult.dipole` and
`PotentialResult.quadrupole` in atomic units. Stress result-carrier wrappers
populate `PotentialResult.stress` in
`ForceInput.energyUnit / ForceInput.lengthUnit^3`. Raw Hessian, dipole,
quadrupole, and stress wrappers use the same `NWChemParams + ForceInput`
carrier for callers that want native C buffers. Optimization result-carrier wrappers
populate `PotentialResult.optimizedPos` in `ForceInput.lengthUnit` and
`PotentialResult.energy` in `ForceInput.energyUnit`; frequency result-carrier
wrappers populate `PotentialResult.frequencies` in cm^-1 and
`PotentialResult.intensities` in atomic units.

Configuration is layered: top-level `NWChemParams` fields for embed/ABI knobs,
typed `NWChemInputStanza` kinds (DFT, SCF, driver, task, property, basis,
geometry, module, pseudopotential, set, generic), and escape hatches (`raw`,
`inputBlocks`, `set`, `custom` module) for the long NWChem option tail. See
`docs/orgmode/reference/nwchem-options.org` for the full contract. Embed builds
write typed SCF and driver scalar controls directly to RTDB, including driver
`maxiter`, `tight` / `loose`, and explicit `gmax` / `grms` / `xmax` / `xrms`
tolerances. Structured NWPW cutoff controls are also promoted directly to the
matching PSPW/BAND RTDB keys instead of going through rendered text, including
NWPW cell names, wavefunction filenames, fake mass, time step, loop, and
tolerance controls. Simple NWPW `exchangeCorrelation` names are promoted to
the PSPW/BAND/CPSD/CPMD exchange-correlation RTDB keys with matching SIC/HFX
defaults reset explicitly; complex `exchange_correlation new ...` grammar
stays in the text/directive path. Born-Oppenheimer controls such as
`balanceMode`, `boStepStart` / `boStepEnd`, `boTimeStep`, `boAlgorithm`,
`boFakeMass`, and `scalingFirst` / `scalingSecond` are also promoted directly
to the NWPW/CPMD RTDB keys used by NWChem. Execution controls for
`np_dimensions`, `spin_orbit`, and `parallel_io` have structured fields and use
direct RTDB writes in embed builds. NWPW output and motion filename fields are
also structured and promoted to the paired `cpmd:*` / `nwpw:*` RTDB keys.
Fractional orbital and NWPW smear controls expose the `nwpw:fractional_*`
RTDB values directly while preserving structured full-deck rendering. Virtual
orbital counts, LCAO skip/use mode, and Ewald grid dimensions are also
structured and promoted to direct `nwpw:*` RTDB values. Nose-Hoover thermostat
controls expose the paired `cpmd:*` / `nwpw:*` nose, restart, period,
temperature, and chain-length RTDB keys directly. Born solvation controls
expose `nwpw:born`, `nwpw:born_dielec`, `nwpw:born_relax`, and
`nwpw:born_vradii` directly. CPMD property and grid-comparison flags expose
`nwpw:cpmd_properties` and `nwpw:use_grid_cmp` directly. Director controls
expose `nwpw:use_director` and `nwpw:director_filename` directly. Cell
expansion and mapping controls expose `nwpw:cell_expand` and `nwpw:mapping`
directly. Rotation and multipole controls expose `nwpw:rotation` and
`nwpw:lmax_multipole` directly. FEI controls expose the paired
`cpmd:fei`, `nwpw:fei`, `cpmd:fei_filename`, and `nwpw:fei_filename` keys
directly. Initial-velocity controls expose `nwpw:init_velocities_temperature`,
`nwpw:init_velocities_seed`, and `nwpw:init_velocities` directly.
`makehmass2` exposes `nwpw:makehmass2` directly. Translate-vector controls
expose `nwpw:translate_vector`, `nwpw:translate_geom_name`, and
`nwpw:translate_reorder` directly. Socket controls expose `nwpw:socket_type`
and `nwpw:socket_ip` directly. APC controls expose `nwpw_APC:Gc`,
`nwpw_APC:nga`, and `nwpw_APC:gamma` directly. Translation controls expose
`cgsd:allow_translation` and `band:allow_translation` directly. Minimizer
controls expose `nwpw:minimizer` directly, while SCF algorithm controls expose
`nwpw:ks_algorithm`, `nwpw:scf_algorithm`, and `nwpw:precondition` directly.
NWPW Brillouin-zone and
simulation-cell stanzas cover k-point grids, explicit k-vectors, boundary
conditions, 3x3 cell vectors, FFT grids, and related periodic-cell RTDB state.
Classic CCSD scalar controls cover iteration, threshold, DIIS, frozen-orbital,
disk-use, and SCS scaling settings through direct `ccsd:*` RTDB writes.

## Build

Frontend/stub build without NWChem:

```bash
meson setup build -Dwith_tests=true
meson compile -C build
meson test -C build --print-errorlogs
```

Tests use cmocka through pkg-config. The pixi environment includes the test
dependency.

```bash
pixi run test-stub      # stub/parser Meson suite
pixi run test-cmocka    # intern inventory + cmocka feature tests + driver
pixi run inventory-check
pixi run inventory-gen  # regenerate schema/inventory + C tables after schema edits
pixi run coverage       # gcov/lcov on instrumented stub build (see build-coverage/)
pixi run prek-validate  # validate prek.toml
pixi run prek           # all hooks: baseline + Fortran fprettify + lychee + inventory
pixi run fortran-lint   # fprettify check on src/*.f90 and src/*.F
pixi run lychee         # link check README + docs/orgmode + newsfragments
pixi run test-integration-matrix   # validate tests/integration/task_matrix.json
pixi run debug-backtrace-smoke     # feature driver debug handlers + crash-test backtrace
# CLI integration vs conda-forge nwchem (NOT embed SDK; linux-64 pixi env):
pixi run -e nwchem-runtime test-nwchem-compare
# CLI + embed numeric compare (tolerances in task_matrix.json; needs build-nwchem):
#   python3 tools/compare_nwchem_cli.py --out-dir build/nwchem-compare-embed \
#     --embed-build build-nwchem --require-cli
#   # or: pixi run test-nwchem-compare-embed  (after build-nwchem exists)
# Embed integration (separate NWChem *build tree* with PIC libs):
#   meson setup build-nwchem -Dwith_nwchem=true -Dnwchem_root=$NWCHEM_TOP -Dwith_tests=true
#   meson test -C build-nwchem nwchem-energy-gradient nwchem-energy-forces nwchem-hessian
# Embed test writes NWCHEMC_COMPARE_JSON for the harness (energy + gradient vectors).
```

Debug: `tools/nwchemc_debug.c` installs `SIGSEGV`/`SIGABRT`/`SIGFPE` handlers and
prints `backtrace(3)` frames (execinfo; optional C++ `backward.hpp` path not required).
Meson option `-Dwith_debug_backtrace=true` (default) links debug helpers into
`nwchemc_feature_driver`. Enable handlers with `NWCHEMC_DEBUG=1`, or run
`./build/nwchemc_feature_driver crash-test` for a deliberate abort smoke test.

Integration/CI: `tests/integration/task_matrix.json` lists CLI `.nw` tasks under
`tests/integration/nw/`. `.github/workflows/ci.yml` runs stub/cmocka + debug smoke
always and optional `nwchem-runtime` CLI compare over the matrix (embed SDK is not
on GHA by default; local embed-vs-CLI uses `--embed-build` and exit 3 if incomplete).

Lint hooks live in `prek.toml` (`prek install` for commit hooks). Lychee scope is
controlled by `.lychee.toml`. Coverage is tracked for instrumentable in-tree C
exercised by stub/parser/cmocka suites; embed/Fortran/NWChem-only units require
a real embed build and are listed as explicit exceptions in the coverage report.

## Feature inventory and C driver

Every `NWChemModuleName`, input stanza kind, top-level `NWChemParams` field, and
public `nwchemc_*` ABI entrypoint is interned in
`schema/inventory/nwchem_features.json` with C accessors in
`include/nwchemc_features.h` / `src/nwchemc_features.c`. Regenerate with
`pixi run inventory-gen`; cross-check with `pixi run inventory-check`.

`tools/nwchemc_feature_driver.c` exercises interned classes through shipped
entry points (inventory/validate/stub-abi/params/all). Meson registers it under
the `cmocka` / `inventory` / `drivers` suites. Params mode needs a flat
`NWChemParams` fixture (`NWCHEMC_PARSER_FIXTURE` or argv[2]).

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

`nwchemc_hessian()` fills a dense row-major Cartesian Hessian in
Hartree/Bohr^2. It accepts the same `NWChemParams` Cap'n Proto message as the
energy/gradient call.
`nwchemc_dipole()` fills a three-element total dipole vector in atomic units and
returns the associated total energy in `NWChemCResult.energy_h`.
`nwchemc_stress()` fills a row-major 3x3 stress tensor in NWChem atomic stress
units and returns the associated total energy in `NWChemCResult.energy_h` when
available.

## Cap'n Proto C

The build vendors the MIT-licensed `capnp-c` runtime and compiler plugin. Meson
builds `capnpc-c`, generates C bindings for `schema/Potentials.capnp`, and
compiles those generated readers into `nwchemc`. The public ABI remains the
checked-in C header and serialized Cap'n Proto bytes.
