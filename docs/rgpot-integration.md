# rgpot integration guide

This is the C ABI contract rgpot should wire against for the NWChem backend.
The stable boundary is serialized Cap'n Proto data, not NWChem input text and
not C++ or Rust objects.

## Data flow

Use `PotentialConfig.nwchem` for backend configuration and `ForceInput` for
each geometry step. Allocate the output buffer from the step message, then ask
nwchemc to write an unpacked flat `PotentialResult`.

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
- `nwchemc_calculate_quadrupole_result_from_config()`
- `nwchemc_calculate_stress_result_from_config()`
- `nwchemc_calculate_optimize_result_from_config()`
- `nwchemc_calculate_frequencies_result_from_config()`

The session variants use the same `ForceInput` and `PotentialResult` messages.
They are the preferred path for repeated geometry steps with fixed topology.

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
| Hessian, dipole, quadrupole, optimize, and frequencies result carriers | `tests/test_nwchem_rgpot_smoke.c` |
| Periodic PSPW stress raw buffer and result carrier | `tests/test_nwchem_stress.c` |
| `ForceInput.box` cell-vector RTDB storage | `tests/test_nwchem_forceinput_cell_rtdb.c` |
| Structured pseudopotential RTDB storage from `PotentialConfig.nwchem` | `tests/test_nwchem_potential_config_pseudopotential.c` |
| Direct pseudopotential RTDB storage | `tests/test_nwchem_pseudopotential_rtdb.c` |
| Stub/parser ABI and result-carrier shape | `tests/cmocka/test_embed_config_cmocka.c` |

Molecular energy, forces, Hessian, dipole, quadrupole, optimization, and
frequencies are covered by real-NWChem smoke tests. Periodic stress is covered
by the PSPW stress smoke when rgpot is paired with an NWPW-enabled NWChem
build. Molecular NWChem modules reject periodic geometries, so periodic
coverage uses that NWPW-enabled path. The ABI, Cap'n Proto result shape, unit
conversion path, and direct RTDB cell storage are still covered without a
periodic-capable NWChem build.

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
- The installed package smoke `nwchem-installed-cmake-consumer` passes against
  the same NWChem build used for the real probe suite.
- The installed package smoke `nwchem-installed-pkgconfig-consumer` passes
  against the same NWChem build when rgpot uses pkg-config for discovery.

Stress is on the same release surface when rgpot is paired with an NWPW-enabled
NWChem build and `tests/test_nwchem_stress.c` passes in the real-NWChem suite.
