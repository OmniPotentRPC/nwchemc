# rgpot integration guide

This is the C ABI contract rgpot should wire against for the NWChem backend.
The stable boundary is serialized Cap'n Proto data, not NWChem input text and
not C++ or Rust objects.

## Wiring stage

The backend is at the rgpot wiring stage when rgpot can treat `nwchemc` as an
installed package and call the serialized-message ABI directly:

- `PotentialConfig.nwchem` carries NWChem configuration.
- `ForceInput` carries each geometry, cell, unit, charge, and multiplicity
  step.
- `PotentialResult` carries energy, forces, Hessian, stress, dipole,
  polarizability, quadrupole, optimized geometry, frequencies, and
  intensities.
- Structured pseudopotentials are promoted to NWChem RTDB state by the embed
  bridge; rgpot does not need to render NWChem pseudopotential text.

The merge/pr release gate is a green local stub/parser suite, a green
real-NWChem suite for the probes listed below, and green installed-package
CMake/pkg-config consumers against the same NWChem build. Periodic stress is
part of that gate only when the release build links an NWPW-enabled NWChem.

## Data flow

Use `PotentialConfig.nwchem` for backend configuration and `ForceInput` for
each geometry step. Allocate the output buffer from the step message, then ask
nwchemc to write an unpacked flat `PotentialResult`.

The rgpot adapter should keep one `NWChemCSession` per fixed atom count and
species list. Coordinate changes, unit changes, cell-vector changes, charge,
and multiplicity can vary by `ForceInput` step. A changed atom count or species
list requires a separate session.

Leave `NWChemParams.enginePath` empty for this ABI. The current build links the
NWChem embed bridge in-process and rejects non-empty dynamic engine paths
before any NWChem configuration is applied.

```c
NWChemCSession *session =
    nwchemc_session_create_from_config(config_capnp, config_capnp_size);

size_t result_capacity =
    nwchemc_potential_result_size_for_force_input(force_input_capnp,
                                                  force_input_capnp_size);

NWChemCResult status =
    nwchemc_session_calculate_result(session, force_input_capnp,
                                     force_input_capnp_size, result_capnp,
                                     result_capacity, &result_size);
```

`nwchemc_calculate_result_from_config()` is the matching one-shot path for
callers that do not want to keep an `NWChemCSession`. The named result-carrier
entry points are available when rgpot wants one operation instead of the
energy-plus-forces compatibility result:

- `nwchemc_calculate_hessian_result_from_config()`
- `nwchemc_calculate_dipole_result_from_config()`
- `nwchemc_calculate_polarizability_result_from_config()`
- `nwchemc_calculate_quadrupole_result_from_config()`
- `nwchemc_calculate_stress_result_from_config()`
- `nwchemc_calculate_optimize_result_from_config()`
- `nwchemc_calculate_frequencies_result_from_config()`

The session variants use the same `ForceInput` and `PotentialResult` messages.
They are the preferred path for repeated geometry steps with fixed topology.

## Periodic cells

For rgpot periodic calculations, put the 3x3 cell vectors in `ForceInput.box`
for every periodic step. That is the path used by the energy, forces, stress,
Hessian, dipole, polarizability, quadrupole, optimization, and frequency
result-carrier calls.

`PotentialConfig.nwchem.inputStanzas[].simulationCell` remains useful for
static NWChem/NWPW configuration such as grids, lattice names, and compact
lattice directives. It does not replace `ForceInput.box` when rgpot needs a
cell per geometry step.

## Units

`NWChemCResult.energy_h` is always Hartree. `PotentialResult` follows the unit
fields in `ForceInput`:

| Field | Unit |
| --- | --- |
| `PotentialResult.energy` | `ForceInput.energyUnit` |
| `PotentialResult.forces` | `ForceInput.energyUnit / ForceInput.lengthUnit` |
| `PotentialResult.hessian` | `ForceInput.energyUnit / ForceInput.lengthUnit^2` |
| `PotentialResult.stress` | `ForceInput.energyUnit / ForceInput.lengthUnit^3` |
| `PotentialResult.optimizedPos` | `ForceInput.lengthUnit` |
| `PotentialResult.dipole` | atomic units |
| `PotentialResult.polarizability` | atomic units |
| `PotentialResult.quadrupole` | atomic units |
| `PotentialResult.frequencies` | cm^-1 |
| `PotentialResult.intensities` | atomic units |

`ForceInput.hasCharge` / `ForceInput.charge` and
`ForceInput.hasMultiplicity` / `ForceInput.multiplicity` override the session
configuration for a step. Unset flags keep the values from
`PotentialConfig.nwchem`.

## Supported merge scope

The rgpot merge path can wire the NWChem backend when it uses the data flow
above and these probes are green:

| Surface | Probe |
| --- | --- |
| `PotentialConfig.nwchem + ForceInput -> PotentialResult` energy and forces | `tests/test_nwchem_rgpot_smoke.c` |
| Session result carriers across repeated steps | `tests/test_nwchem_session_result.c` |
| Hessian, dipole, polarizability, quadrupole, optimize, and frequencies result carriers | `tests/test_nwchem_rgpot_smoke.c` |
| Periodic PSPW stress coordinate/session, ForceInput raw calls, and result carriers | `tests/test_nwchem_stress.c` |
| PSPW pseudopotential energy and forces one-shot/session result carriers | `tests/test_nwchem_pspw_pseudopotential_forces.c` |
| `ForceInput.box` cell-vector RTDB storage | `tests/test_nwchem_forceinput_cell_rtdb.c` |
| Structured pseudopotential RTDB storage from `PotentialConfig.nwchem` | `tests/test_nwchem_potential_config_pseudopotential.c` |
| Configured NWPW periodic controls reaching embedded RTDB | `tests/test_nwchem_nwpw_rtdb.c` |
| Direct pseudopotential RTDB storage | `tests/test_nwchem_pseudopotential_rtdb.c` |
| Stub/parser ABI and result-carrier shape | `tests/cmocka/test_embed_config_cmocka.c` |

Molecular energy, forces, Hessian, dipole, polarizability, quadrupole,
optimization, and frequencies are covered by real-NWChem smoke tests. Periodic
stress is covered by the PSPW stress smoke when rgpot is paired with an
NWPW-enabled NWChem build. Molecular NWChem modules reject periodic
geometries, so periodic coverage uses that NWPW-enabled path. The ABI, Cap'n
Proto result shape, unit conversion path, and direct RTDB cell storage are
still covered without a periodic-capable NWChem build.

## Release gate

A rgpot-side merge/pr release is at the wiring stage when:

- rgpot serializes `PotentialConfig.nwchem` and `ForceInput` directly, with no
  schema adapter between the two repositories.
- rgpot consumes an installed nwchemc package instead of reaching into the
  source tree. CMake integrations use `find_package(nwchemc CONFIG REQUIRED)`
  and `target_link_libraries(consumer PRIVATE nwchemc::nwchemc)`;
  pkg-config integrations set `PKG_CONFIG_PATH` and check
  `pkg-config --cflags --libs nwchemc`.
- rgpot allocates `PotentialResult` with
  `nwchemc_potential_result_size_for_force_input()` or the named sizing helper
  for the operation it calls.
- rgpot uses `nwchemc_session_calculate_result()` for repeated molecular steps
  or `nwchemc_calculate_result_from_config()` for one-shot evaluation.
- Operation-specific rgpot calls use the named result-carrier functions listed
  above.
- The local stub/cmocka suite and a real-NWChem suite both pass for the probes
  in the supported merge scope table.
- The installed package smoke `nwchem-installed-cmake-consumer` compiles and
  links the `PotentialResult` sizing and calculation ABI against the same
  NWChem build used for the real probe suite.
- The installed package smoke `nwchem-installed-pkgconfig-consumer` compiles
  and links the same `PotentialResult` ABI when rgpot uses pkg-config for
  discovery.

Stress is on the same release surface when rgpot is paired with an NWPW-enabled
NWChem build and `tests/test_nwchem_stress.c` passes in the real-NWChem suite.
That probe covers coordinate one-shot/session stress, ForceInput one-shot and
session stress, plus `PotentialResult` stress serialization.
