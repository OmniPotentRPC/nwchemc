# nwchemc

Stable C ABI for embedding NWChem from language-neutral Cap'n Proto
`PotentialConfig`, `NWChemParams`, `ForceInput`, and `PotentialResult`
messages.

`nwchemc` builds `libnwchemc.so` from a C ABI layer, modern Fortran
`iso_c_binding` / `iso_fortran_env` bridge code, and the legacy NWChem embed
entry points required for RTDB, geometry, basis, energy, gradient, Hessian,
dipole, polarizability, quadrupole, stress, optimization, and frequency calls.

## rgpot release status

The rgpot-facing ABI is the serialized
`PotentialConfig.nwchem + ForceInput -> PotentialResult` path. rgpot can wire
the backend against an installed `nwchemc` package without depending on
NWChem text decks, C++ objects, Rust objects, or files from this source tree.

The practical merge/pr trigger is:

- rgpot uses the installed package through CMake or pkg-config.
- rgpot sends the shared `PotentialConfig.nwchem` and `ForceInput` messages
  without a schema adapter.
- rgpot consumes only stable `PotentialResult` fields for the release surface:
  energy, forces, gradient, Hessian, stress, dipole, polarizability,
  quadrupole, optimized coordinates, frequencies, normal modes, and frequency
  thermochemistry.
- The real-NWChem gate below passes against the same NWChem build that the
  package was linked with.

Use this path for the rgpot adapter:

1. Link the installed package with CMake `find_package(nwchemc CONFIG
   REQUIRED)` or pkg-config `nwchemc`.
2. Serialize one `PotentialConfig.nwchem` for the backend configuration.
3. Create an `NWChemCSession` from that config for a fixed atom count and
   species list.
4. Serialize each geometry step as `ForceInput`. Put periodic cell vectors in
   `ForceInput.box` so the cell travels with the step.
5. Allocate the output buffer with
   `nwchemc_potential_result_size_for_force_input()` for energy+forces or the
   operation-specific sizing helper for gradient, Hessian, stress, dipole,
   polarizability, quadrupole, optimization, or frequencies.
6. Call `nwchemc_session_calculate_result()` for energy+forces, or the named
   session result-carrier entry point for a specific operation.

The merge/pr release gate is:

- rgpot creates `PotentialConfig.nwchem` and per-step `ForceInput` messages
  using the shared Cap'n Proto schema.
- rgpot allocates each result buffer with
  `nwchemc_potential_result_size_for_force_input()` or the matching
  operation-specific sizing helper.
- repeated steps use `nwchemc_session_calculate_result()` or a named session
  result-carrier function.
- one-shot calls use `nwchemc_calculate_result_from_config()` or a named
  result-carrier function.
- `nwchem-rgpot-smoke`, `nwchem-session-result`,
  `nwchem-potential-config-pseudopotential`, `nwchem-pseudopotential-rtdb`,
  `nwchem-forceinput-cell-rtdb`, `nwchem-stress`,
  `nwchem-pspw-pseudopotential-forces`, `nwchem-configured-nwpw-rtdb`, and the
  installed CMake/pkg-config consumers pass against the NWChem build used for
  the release.
- The installed CMake/pkg-config consumers compile, link, and run
  invalid-input ABI checks for the rgpot result-carrier, raw ForceInput,
  session, and coordinate entry points.

Mulliken and population-analysis controls are schema inputs. They are not part
of the rgpot release result carrier because the NWChem paths expose those
values through print/ECCE output rather than a stable RTDB result vector.

Run the gate against the release NWChem build with:

```sh
python3 scripts/rgpot_release_gate.py --build-dir <real-nwchem-builddir>
```

For a local package-only check:

```sh
meson test -C build --print-errorlogs
```

For a release build linked to NWChem:

```sh
meson test -C <builddir> --print-errorlogs --num-processes 1
```

See [docs/rgpot-integration.md](docs/rgpot-integration.md) for the exact data
flow, units, operation matrix, and NWPW stress expectations.

The public ABI does not expose C++ or Rust types:

```c
int nwchemc_set_params(const void *params_capnp, size_t params_capnp_size_bytes);
int nwchemc_configure(const void *config_capnp, size_t config_capnp_size_bytes);
NWChemCResult nwchemc_energy_gradient(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *grad_h_bohr);
NWChemCResult nwchemc_energy(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes);
NWChemCResult nwchemc_energy_forces(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *forces_h_bohr);
NWChemCResult nwchemc_hessian(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *hessian_h_bohr2);
NWChemCResult nwchemc_dipole(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *dipole_au);
NWChemCResult nwchemc_polarizability(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *polarizability_au);
NWChemCResult nwchemc_quadrupole(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *quadrupole_au);
NWChemCResult nwchemc_stress(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *stress_au);
NWChemCResult nwchemc_optimize(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *optimized_positions_ang);
NWChemCResult nwchemc_frequencies(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *frequencies_cm1, double *intensities_au);
NWChemCResult nwchemc_energy_gradient_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *grad_h_bohr);
NWChemCResult nwchemc_energy_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes);
NWChemCResult nwchemc_energy_forces_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *forces_h_bohr);
NWChemCResult nwchemc_hessian_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *hessian_h_bohr2);
NWChemCResult nwchemc_dipole_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *dipole_au);
NWChemCResult nwchemc_polarizability_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *polarizability_au);
NWChemCResult nwchemc_quadrupole_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *quadrupole_au);
NWChemCResult nwchemc_stress_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *stress_au);
NWChemCResult nwchemc_optimize_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *optimized_positions_ang);
NWChemCResult nwchemc_frequencies_from_config(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *config_capnp, size_t config_capnp_size_bytes,
    double *frequencies_cm1, double *intensities_au);
NWChemCSession *nwchemc_session_create(
    const void *params_capnp, size_t params_capnp_size_bytes);
NWChemCSession *nwchemc_session_create_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes);
int nwchemc_session_set_params(NWChemCSession *session,
                               const void *params_capnp,
                               size_t params_capnp_size_bytes);
int nwchemc_session_configure(NWChemCSession *session,
                              const void *config_capnp,
                              size_t config_capnp_size_bytes);
void nwchemc_session_destroy(NWChemCSession *session);
NWChemCResult nwchemc_session_energy_gradient(
    NWChemCSession *session, int n_atoms, const double *positions_ang,
    const int *atomic_numbers, double *grad_h_bohr);
NWChemCResult nwchemc_session_energy(
    NWChemCSession *session, int n_atoms, const double *positions_ang,
    const int *atomic_numbers);
NWChemCResult nwchemc_session_energy_forces(
    NWChemCSession *session, int n_atoms, const double *positions_ang,
    const int *atomic_numbers, double *forces_h_bohr);
NWChemCResult nwchemc_session_dipole(
    NWChemCSession *session, int n_atoms, const double *positions_ang,
    const int *atomic_numbers, double *dipole_au);
NWChemCResult nwchemc_session_polarizability(
    NWChemCSession *session, int n_atoms, const double *positions_ang,
    const int *atomic_numbers, double *polarizability_au);
NWChemCResult nwchemc_session_quadrupole(
    NWChemCSession *session, int n_atoms, const double *positions_ang,
    const int *atomic_numbers, double *quadrupole_au);
NWChemCResult nwchemc_session_optimize(
    NWChemCSession *session, int n_atoms, const double *positions_ang,
    const int *atomic_numbers, double *optimized_positions_ang);
NWChemCResult nwchemc_session_frequencies(
    NWChemCSession *session, int n_atoms, const double *positions_ang,
    const int *atomic_numbers, double *frequencies_cm1,
    double *intensities_au);
NWChemCResult nwchemc_session_calculate_forces(
    NWChemCSession *session,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *forces_h_bohr, size_t forces_len);
NWChemCResult nwchemc_session_calculate_gradient(
    NWChemCSession *session,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *gradient_h_bohr, size_t gradient_len);
NWChemCResult nwchemc_session_calculate_energy(
    NWChemCSession *session,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes);
NWChemCResult nwchemc_calculate_forces(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *forces_h_bohr, size_t forces_len);
NWChemCResult nwchemc_calculate_gradient(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *gradient_h_bohr, size_t gradient_len);
NWChemCResult nwchemc_calculate_forces_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *forces_h_bohr, size_t forces_len);
NWChemCResult nwchemc_calculate_gradient_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *gradient_h_bohr, size_t gradient_len);
NWChemCResult nwchemc_calculate_energy(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes);
NWChemCResult nwchemc_calculate_energy_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes);
size_t nwchemc_energy_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes);
NWChemCResult nwchemc_session_calculate_energy_result(
    NWChemCSession *session,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
NWChemCResult nwchemc_calculate_energy_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
NWChemCResult nwchemc_calculate_energy_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
size_t nwchemc_forces_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes);
size_t nwchemc_gradient_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes);
NWChemCResult nwchemc_session_calculate_forces_result(
    NWChemCSession *session,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
NWChemCResult nwchemc_session_calculate_gradient_result(
    NWChemCSession *session,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
NWChemCResult nwchemc_calculate_forces_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
NWChemCResult nwchemc_calculate_gradient_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
NWChemCResult nwchemc_calculate_forces_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
NWChemCResult nwchemc_calculate_gradient_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
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
NWChemCResult nwchemc_calculate_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
NWChemCResult nwchemc_calculate_hessian(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *hessian_h_bohr2, size_t hessian_len);
NWChemCResult nwchemc_calculate_hessian_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
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
NWChemCResult nwchemc_calculate_hessian_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
NWChemCResult nwchemc_calculate_dipole(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *dipole_au, size_t dipole_len);
NWChemCResult nwchemc_calculate_dipole_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
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
NWChemCResult nwchemc_calculate_dipole_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
NWChemCResult nwchemc_calculate_polarizability(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *polarizability_au, size_t polarizability_len);
NWChemCResult nwchemc_calculate_polarizability_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *polarizability_au, size_t polarizability_len);
size_t nwchemc_polarizability_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes);
NWChemCResult nwchemc_session_calculate_polarizability_result(
    NWChemCSession *session,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
NWChemCResult nwchemc_calculate_polarizability_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
NWChemCResult nwchemc_calculate_polarizability_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
NWChemCResult nwchemc_calculate_quadrupole(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *quadrupole_au, size_t quadrupole_len);
NWChemCResult nwchemc_calculate_quadrupole_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *quadrupole_au, size_t quadrupole_len);
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
NWChemCResult nwchemc_calculate_quadrupole_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
NWChemCResult nwchemc_calculate_stress(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *stress_au, size_t stress_len);
NWChemCResult nwchemc_calculate_stress_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
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
NWChemCResult nwchemc_calculate_stress_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
NWChemCResult nwchemc_calculate_optimize(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *optimized_positions_ang, size_t optimized_positions_len);
NWChemCResult nwchemc_calculate_optimize_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *optimized_positions_ang, size_t optimized_positions_len);
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
NWChemCResult nwchemc_calculate_optimize_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
NWChemCResult nwchemc_calculate_frequencies(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *frequencies_cm1, size_t frequencies_len,
    double *intensities_au, size_t intensities_len);
NWChemCResult nwchemc_calculate_frequencies_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *frequencies_cm1, size_t frequencies_len,
    double *intensities_au, size_t intensities_len);
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
NWChemCResult nwchemc_calculate_frequencies_result_from_config(
    const void *config_capnp, size_t config_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);
size_t nwchemc_potential_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes);
NWChemCResult nwchemc_session_calculate_hessian(
    NWChemCSession *session,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *hessian_h_bohr2, size_t hessian_len);
NWChemCResult nwchemc_session_calculate_dipole(
    NWChemCSession *session,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *dipole_au, size_t dipole_len);
NWChemCResult nwchemc_session_calculate_polarizability(
    NWChemCSession *session,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *polarizability_au, size_t polarizability_len);
NWChemCResult nwchemc_session_calculate_quadrupole(
    NWChemCSession *session,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *quadrupole_au, size_t quadrupole_len);
NWChemCResult nwchemc_session_calculate_stress(
    NWChemCSession *session,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *stress_au, size_t stress_len);
NWChemCResult nwchemc_session_calculate_optimize(
    NWChemCSession *session,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *optimized_positions_ang, size_t optimized_positions_len);
NWChemCResult nwchemc_session_calculate_frequencies(
    NWChemCSession *session,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *frequencies_cm1, size_t frequencies_len,
    double *intensities_au, size_t intensities_len);
NWChemCResult nwchemc_session_stress(
    NWChemCSession *session, int n_atoms, const double *positions_ang,
    const int *atomic_numbers, double *stress_au);
NWChemCResult nwchemc_session_hessian(
    NWChemCSession *session, int n_atoms, const double *positions_ang,
    const int *atomic_numbers, double *hessian_h_bohr2);
const char *nwchemc_version(void);
int nwchemc_available(void);
void nwchemc_finalize(void);
```

`params_capnp` is an unpacked flat Cap'n Proto message whose root is
`NWChemParams` from `schema/Potentials.capnp`. `config_capnp` is the schema-level
configuration carrier whose root is `PotentialConfig`; the `nwchem` union arm
carries the embedded `NWChemParams` payload. Both formats can be produced by
pycapnp, rgpot, mmap-backed readers, or another Cap'n Proto binding that writes
the standard flat stream format. `nwchemc` reads those messages through
generated `capnp-c` bindings; it does not define a parallel user configuration
format.

Long-running callers should create one `NWChemCSession` from `PotentialConfig`
with `nwchemc_session_create_from_config()` or from raw `NWChemParams` with
`nwchemc_session_create()`, then pass a serialized `ForceInput` for each
geometry step. The result sizing helper parses the step message only, so
callers can allocate or reuse an unpacked flat `PotentialResult` buffer before
calling
`nwchemc_session_calculate_result()`. The evaluating call keeps
`NWChemCResult.energy_h` in Hartree and writes `PotentialResult.energy` /
`PotentialResult.forces` in `ForceInput.energyUnit` and
`ForceInput.energyUnit / ForceInput.lengthUnit`. The named gradient carrier
writes `PotentialResult.gradient` in the same energy/length unit without the
force sign flip.
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
`nwchemc_session_calculate_energy()` returns only the native Hartree energy for
callers that do not need a result carrier. `nwchemc_session_calculate_energy_result()`
writes an energy-only `PotentialResult`, converting `PotentialResult.energy` to
`ForceInput.energyUnit` while leaving `NWChemCResult.energy_h` in Hartree.
`nwchemc_session_calculate_forces_result()` mirrors
`Potential.calculateForces`; it is the method-named form of the existing
energy+forces result-carrier path. `nwchemc_session_calculate_gradient_result()`
mirrors `Potential.calculateGradient` and writes the native derivative sign.
`nwchemc_calculate_energy()`, `nwchemc_calculate_gradient()`,
`nwchemc_calculate_energy_result()`, `nwchemc_calculate_forces_result()`,
`nwchemc_calculate_gradient_result()`, and `nwchemc_calculate_result()` offer
the same `NWChemParams + ForceInput` carrier for one-shot callers and delegate
through the session paths; callers with multiple steps should reuse
`NWChemCSession`. Matching `*_from_config()` one-shot wrappers accept
`PotentialConfig + ForceInput` for raw C buffers and for `PotentialResult`
carriers, reading the `nwchem` union arm before evaluation.
`nwchemc_session_calculate_hessian_result()` and
`nwchemc_calculate_hessian_result()` populate `PotentialResult.hessian` in
`ForceInput.energyUnit / ForceInput.lengthUnit^2`. Dipole, polarizability, and
quadrupole result-carrier wrappers populate `PotentialResult.dipole`,
`PotentialResult.polarizability`, and `PotentialResult.quadrupole` in atomic
units. `PotentialResult.polarizability` stores NWChem `aoresponse:alpha` as
frequency, xx, xy, xz, yy, yz, zz, three eigenvalues, isotropic, and
anisotropic entries. Stress result-carrier wrappers populate
`PotentialResult.stress` in `ForceInput.energyUnit / ForceInput.lengthUnit^3`.
Raw force, gradient, Hessian, dipole, polarizability, quadrupole, and stress
wrappers use the same `NWChemParams + ForceInput` carrier for callers that want
native C buffers. Optimization result-carrier wrappers
populate `PotentialResult.optimizedPos` in `ForceInput.lengthUnit` and
`PotentialResult.energy` in `ForceInput.energyUnit`; frequency result-carrier
wrappers populate `PotentialResult.frequencies` in cm^-1 and
`PotentialResult.intensities` in atomic units. Frequency result-carrier
wrappers also populate `PotentialResult.normalModes` as a dense Cartesian
normal-mode matrix with `(3 * natoms) * (3 * natoms)` entries, plus NWChem
frequency thermochemistry scalars: `PotentialResult.zeroPointEnergy`,
`PotentialResult.thermalEnergy`, `PotentialResult.thermalEnthalpy`,
`PotentialResult.entropy`, and `PotentialResult.heatCapacityCv`.

The Cap'n Proto `Potential` RPC interface mirrors the operation surface with
explicit `calculateEnergy`, `calculateForces`, `calculateHessian`,
`calculateDipole`, `calculatePolarizability`, `calculateQuadrupole`,
`calculateStress`, `calculateOptimize`, `calculateFrequencies`, and
`calculateGradient` methods.
`Potential.configure` maps to `nwchemc_configure()` and
`nwchemc_session_configure()` for C ABI callers. The original `calculate`
method remains the compatibility energy/forces call.

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
tolerance controls. The NWPW `cutoff` alias derives the energy cutoff as
`2*wcut` when the alias omits an explicit energy value. Simple NWPW
`exchangeCorrelation` names are promoted to
the PSPW/BAND/CPSD/CPMD exchange-correlation RTDB keys with matching SIC/HFX
defaults reset explicitly; complex `exchange_correlation new ...` grammar
stays in the text/directive path. Born-Oppenheimer controls such as
`balanceMode`, `boStepStart` / `boStepEnd`, `mcStepStart` / `mcStepEnd`,
`boTimeStep`, `boAlgorithm`, `boFakeMass`, and `scalingFirst` /
`scalingSecond` are also promoted directly to the NWPW/CPMD RTDB keys used by
NWChem. Execution controls for
`np_dimensions`, `spin_orbit`, and `parallel_io` have structured fields and use
direct RTDB writes in embed builds. NWPW output and motion filename fields are
also structured and promoted to the paired `cpmd:*` / `nwpw:*` RTDB keys.
Fractional orbital and NWPW smear controls expose the `nwpw:fractional_*`
RTDB values directly while preserving structured full-deck rendering. Virtual
orbital counts and the NWPW `virtual` alias, LCAO skip/use mode, and Ewald
grid dimensions are also structured and promoted to direct `nwpw:*` RTDB
values. Nose-Hoover thermostat controls, including the NWPW `temperature`
alias, expose the paired `cpmd:*` / `nwpw:*` nose, restart, period,
temperature, and chain-length RTDB keys directly. Born solvation controls
expose `nwpw:born`, `nwpw:born_dielec`, `nwpw:born_relax`, and
`nwpw:born_vradii` directly. Vfield filename lists expose
`nwpw:vfield_filenames` directly. Single-precision HFX exposes
`pspw:HFX_single_precision` directly. Geometry optimization exposes
`cgsd:geometry_optimize`, `cpsd:geometry_optimize`, and
`band:geometry_optimize` directly. Auxiliary potentials expose
`pspw_qmmm_auxon` directly. NWPW multiplicity exposes the CGSD/BAND/CPSD
`ispin` and `mult` keys directly, and spin mode exposes the CGSD/BAND/CPSD
`ispin` keys directly. The standalone allow-translation alias exposes the
CGSD/BAND `allow_translation` keys directly. DOS controls expose `dos:alpha`,
`dos:npoints`, `dos:emin`, `dos:emax`, and `nwpw:dos:filename` directly. CPMD
property and grid-comparison flags expose `nwpw:cpmd_properties` and
`nwpw:use_grid_cmp` directly. Director controls
expose `nwpw:use_director` and `nwpw:director_filename` directly. Cell
expansion, numeric mapping, and named mapping-alias controls expose
`nwpw:cell_expand` and `nwpw:mapping` directly. Rotation and multipole
controls expose `nwpw:rotation` and `nwpw:lmax_multipole` directly. FEI
controls expose the paired `cpmd:fei`, `nwpw:fei`, `cpmd:fei_filename`, and
`nwpw:fei_filename` keys directly. ET controls expose `pspw:et:movecs_a`,
`pspw:et:movecs_b`,
`pspw:et:ion_a`, and `pspw:et:ion_b` directly. Initial-velocity controls expose
`nwpw:init_velocities_temperature`,
`nwpw:init_velocities_seed`, and `nwpw:init_velocities` directly.
`makehmass2` exposes `nwpw:makehmass2` directly. Translate-vector controls
expose `nwpw:translate_vector`, `nwpw:translate_geom_name`, and
`nwpw:translate_reorder` directly. Socket controls expose `nwpw:socket_type`
and `nwpw:socket_ip` directly. APC controls expose `nwpw_APC:Gc`,
`nwpw_APC:nga`, and `nwpw_APC:gamma` directly. Translation controls expose
`cgsd:allow_translation` and `band:allow_translation` directly. Minimizer
controls expose `nwpw:minimizer` directly, while SCF algorithm controls expose
`nwpw:ks_algorithm`, `nwpw:scf_algorithm`, and `nwpw:precondition` directly.
SCF numeric controls expose `nwpw:kerker_g0`, `nwpw:ks_alpha`,
`nwpw:ks_maxit_orb`, `nwpw:ks_maxit_orbs`, and `nwpw:diis_histories` directly.
One-electron guess controls expose `nwpw:H1_it_in`, `nwpw:H1_it_out`, and
`nwpw:H1_it_ortho` directly.
Pseudopotential U-term rules expose `nwpw:uterm`, `nwpw:nuterms`, and indexed
U/J scale, angular-momentum, and ion-list RTDB entries directly.
NWPW Brillouin-zone and
simulation-cell stanzas cover k-point grids, DOS and tetrahedron grids,
explicit k-vectors, boundary conditions, zone aliases, 3x3 cell vectors, FFT
grids, and related periodic-cell RTDB state.
Classic CCSD scalar controls cover iteration, threshold, DIIS, frozen-orbital,
disk-use, and SCS scaling settings through direct `ccsd:*` RTDB writes.

## rgpot integration contract

The short form is `PotentialConfig.nwchem + ForceInput -> PotentialResult`.
rgpot should allocate the result buffer from the serialized `ForceInput`, then
call a session or one-shot result-carrier entry point:

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

One-shot callers can use `nwchemc_calculate_result_from_config()` with the same
serialized messages. Method-specific calls such as
`nwchemc_calculate_forces_result_from_config()` and
`nwchemc_calculate_gradient_result_from_config()` are available when rgpot wants
a narrower result carrier; frequency carriers include
`PotentialResult.normalModes` and frequency thermochemistry scalar fields. See
`docs/rgpot-integration.md` for the merge/pr release gate, supported operation
matrix, real-NWChem probes, and the NWPW stress probe.

## Build

### Stub and parser build

This path does not link NWChem. It builds the C ABI stub, Cap'n Proto parser,
feature inventory, and cmocka tests.

```bash
meson setup build -Dwith_tests=true
meson compile -C build
meson test -C build --print-errorlogs
```

The pixi environment provides cmocka, Cap'n Proto, Meson, and the lint/test
tools used by the normal developer loop:

```bash
pixi run test-stub
pixi run test-cmocka
pixi run inventory-check
pixi run inventory-gen
pixi run test-integration-matrix
pixi run debug-backtrace-smoke
pixi run fortran-lint
pixi run lychee
pixi run prek
```

`tools/nwchemc_debug.c` installs optional `SIGSEGV` / `SIGABRT` / `SIGFPE`
handlers and prints `backtrace(3)` frames. Meson option
`-Dwith_debug_backtrace=true` is enabled by default for
`nwchemc_feature_driver`. Use `NWCHEMC_DEBUG=1` to enable handlers in tools, or
run `./build/nwchemc_feature_driver crash-test` for the deliberate abort smoke
test.

### Real NWChem embed build

`-Dnwchem_root` must point at an NWChem source/build tree, not at a packaged
runtime prefix. The tree must contain:

- `src/config/nwchem_config.h`
- `src/tools/install/bin/ga-config`
- `src/stubs.o`
- source include directories under `src/`
- archives under `lib/$NWCHEM_TARGET`

Build `src/stubs.o` through NWChem if the object is missing:

```bash
export NWCHEM_TOP=/path/to/nwchem
export NWCHEM_TARGET=LINUX64
make -C "$NWCHEM_TOP/src" stubs.o
```

Many NWChem builds provide non-PIC static archives. In that case, build
`nwchemc` as a static library and link the real tests as executables:

```bash
meson setup build-nwchem \
  -Dwith_nwchem=true \
  -Dnwchem_root="$NWCHEM_TOP" \
  -Dnwchem_target="$NWCHEM_TARGET" \
  -Dwith_tests=true \
  -Ddefault_library=static
meson compile -C build-nwchem
meson test -C build-nwchem --print-errorlogs
```

If the NWChem build requires an MPI launch even for small in-process tests, set
the real-test rank count at configure time. The `OMPI_MCA_*` variables are
Open MPI transport controls and are only needed on hosts that require them:

```bash
meson setup build-nwchem \
  -Dwith_nwchem=true \
  -Dnwchem_root="$NWCHEM_TOP" \
  -Dnwchem_target="$NWCHEM_TARGET" \
  -Dwith_tests=true \
  -Ddefault_library=static \
  -Dnwchem_test_mpi_ranks=2
env OMPI_MCA_pml=ob1 OMPI_MCA_btl=self,tcp \
  meson test -C build-nwchem --print-errorlogs --num-processes 2
```

#### Installed package smoke

Real NWChem builds install a package surface for consumers instead of requiring
them to include this repository as a subproject. CMake consumers use the
exported target:

```cmake
find_package(nwchemc CONFIG REQUIRED)

add_executable(consumer main.c)
target_link_libraries(consumer PRIVATE nwchemc::nwchemc)
```

Pkg-config consumers use the installed `.pc` file:

```bash
export PKG_CONFIG_PATH="$prefix/lib64/pkgconfig:$prefix/lib/pkgconfig:${PKG_CONFIG_PATH:-}"
pkg-config --cflags --libs nwchemc
pkg-config --static --libs nwchemc
```

The real-NWChem Meson suite registers `nwchem-installed-cmake-consumer` when
CMake is available and `nwchem-installed-pkgconfig-consumer` when Meson and
pkg-config are available. Those tests configure package builds of `nwchemc`,
install them to scratch prefixes, build separate consumers through
`find_package(nwchemc CONFIG REQUIRED)` and `pkg-config --cflags --libs
nwchemc`, and run them with the configured MPI rank count.

The build reads `NW_MODULE_LIBS` from `nwchem_config.h`, GA linker flags from
`ga-config`, and optional support archives from the NWChem library directory.
`libperfm` is always requested; `libpeigs` and `libpeigs_comm` are added only
when the corresponding archive exists.

Shared `libnwchemc.so` builds are still supported when the selected NWChem
archives are PIC-compatible:

```bash
meson setup build-nwchem-shared \
  -Dwith_nwchem=true \
  -Dnwchem_root="$NWCHEM_TOP" \
  -Dnwchem_target="$NWCHEM_TARGET" \
  -Dwith_tests=true
```

If the shared link probe reports `relocation R_X86_64_PC32 ... can not be used
when making a shared object`, use the static build above or rebuild the NWChem
archives with PIC flags for the target.

### CLI and embed comparison

The CLI matrix uses `.nw` files under `tests/integration/nw/` and tolerances in
`tests/integration/task_matrix.json`.

CLI-only comparison:

```bash
pixi run -e nwchem-runtime test-nwchem-compare
```

CLI-vs-embed comparison with direct executables:

```bash
python3 tools/compare_nwchem_cli.py \
  --out-dir build/nwchem-compare-embed \
  --embed-build build-nwchem \
  --require-cli \
  --require-embed
```

CLI-vs-embed comparison when both legs need an MPI launcher:

```bash
python3 tools/compare_nwchem_cli.py \
  --out-dir build/nwchem-compare-embed \
  --embed-build build-nwchem \
  --require-cli \
  --require-embed \
  --nwchem-command "mpirun -np 2 $NWCHEM_TOP/bin/$NWCHEM_TARGET/nwchem" \
  --embed-command "mpirun -np 2"
```

The comparison report is written as `nwchem_compare_report.txt` and
`nwchem_compare_report.json` in the selected output directory. The embed test
leg writes `NWCHEMC_COMPARE_JSON` with the energy and gradient vector captured
from the C ABI test binary.

Integration/CI: `.github/workflows/ci.yml` runs the stub/cmocka/debug smoke
path and optional CLI comparison. CI does not provide an NWChem embed build
tree by default.

Lint hooks live in `prek.toml` (`prek install` for commit hooks). Lychee scope
is controlled by `.lychee.toml`. Coverage tracks instrumentable in-tree C from
the stub/parser/cmocka suites; real embed coverage needs an NWChem build tree.

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

`nwchemc_hessian()` fills a dense row-major Cartesian Hessian in
Hartree/Bohr^2. It accepts the same `NWChemParams` Cap'n Proto message as the
energy/gradient call.
`nwchemc_dipole()` fills a three-element total dipole vector in atomic units and
returns the associated total energy in `NWChemCResult.energy_h`.
`nwchemc_polarizability()` fills the 12-element NWChem `aoresponse:alpha`
vector in atomic units and returns the associated total energy in
`NWChemCResult.energy_h`.
`nwchemc_stress()` fills a row-major 3x3 stress tensor in NWChem atomic stress
units and returns the associated total energy in `NWChemCResult.energy_h` when
available.

## Cap'n Proto C

The build vendors the MIT-licensed `capnp-c` runtime and compiler plugin. Meson
builds `capnpc-c`, generates C bindings for `schema/Potentials.capnp`, and
compiles those generated readers into `nwchemc`. The public ABI remains the
checked-in C header and serialized Cap'n Proto bytes.
