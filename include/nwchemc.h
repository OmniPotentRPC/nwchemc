#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file nwchemc.h
 * @brief Stable C ABI for configuring and evaluating embedded NWChem.
 *
 * The parameter buffer accepted by this API is an unpacked flat Cap'n Proto
 * message whose root type is `NWChemParams` from `schema/Potentials.capnp`.
 */

/**
 * @brief Result returned by `nwchemc_energy_gradient()`.
 */
typedef struct NWChemCResult {
  /** Non-zero when the calculation succeeds. */
  int ok;
  /** Total energy in Hartree. */
  double energy_h;
  /** Null-terminated status or error message. */
  char message[512];
} NWChemCResult;

/** Opaque handle for repeated evaluations with one Cap'n Proto parameter set. */
typedef struct NWChemCSession NWChemCSession;

/**
 * @brief Apply NWChem method parameters from a Cap'n Proto message.
 *
 * @param params_capnp Pointer to an unpacked flat `NWChemParams` message.
 * @param params_capnp_size_bytes Size of `params_capnp` in bytes.
 * @return 0 on success, -1 on parse or configuration failure.
 */
int nwchemc_set_params(const void *params_capnp,
                       size_t params_capnp_size_bytes);

/**
 * @brief Compute energy and nuclear gradient for an atomic configuration.
 *
 * @param n_atoms Number of atoms.
 * @param positions_ang Flat Cartesian coordinate array in Angstrom, length
 *        `n_atoms * 3`.
 * @param atomic_numbers Atomic number array, length `n_atoms`.
 * @param params_capnp Pointer to an unpacked flat `NWChemParams` message.
 * @param params_capnp_size_bytes Size of `params_capnp` in bytes.
 * @param grad_h_bohr Output nuclear gradient array in Hartree/Bohr, length
 *        `n_atoms * 3`.
 * @return Calculation result and status message.
 */
NWChemCResult nwchemc_energy_gradient(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *grad_h_bohr);

/**
 * @brief Compute total energy only (no gradient/forces allocation).
 *
 * Same configuration carrier as gradient/hessian entry points. Prefer this
 * when callers only need the scalar energy for sampling or acceptance tests.
 */
NWChemCResult nwchemc_energy(int n_atoms, const double *positions_ang,
                             const int *atomic_numbers,
                             const void *params_capnp,
                             size_t params_capnp_size_bytes);

/**
 * @brief Compute energy and nuclear forces for an atomic configuration.
 *
 * Forces are the negative of the Cartesian nuclear gradient (Hartree/Bohr).
 * Output buffer layout matches `nwchemc_energy_gradient()`: length
 * `n_atoms * 3`.
 */
NWChemCResult nwchemc_energy_forces(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *forces_h_bohr);

/**
 * @brief Compute a Cartesian nuclear Hessian for an atomic configuration.
 *
 * @param n_atoms Number of atoms.
 * @param positions_ang Flat Cartesian coordinate array in Angstrom, length
 *        `n_atoms * 3`.
 * @param atomic_numbers Atomic number array, length `n_atoms`.
 * @param params_capnp Pointer to an unpacked flat `NWChemParams` message.
 * @param params_capnp_size_bytes Size of `params_capnp` in bytes.
 * @param hessian_h_bohr2 Output dense row-major Hessian in Hartree/Bohr^2,
 *        length `(n_atoms * 3) * (n_atoms * 3)`.
 * @return Calculation result and status message.
 */
NWChemCResult nwchemc_hessian(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *hessian_h_bohr2);

/**
 * @brief Compute total electric dipole for an atomic configuration.
 *
 * @param n_atoms Number of atoms.
 * @param positions_ang Flat Cartesian coordinate array in Angstrom, length
 *        `n_atoms * 3`.
 * @param atomic_numbers Atomic number array, length `n_atoms`.
 * @param params_capnp Pointer to an unpacked flat `NWChemParams` message.
 * @param params_capnp_size_bytes Size of `params_capnp` in bytes.
 * @param dipole_au Output total dipole vector in atomic units, length 3.
 * @return Calculation result and status message. `energy_h` carries the
 *         associated total energy in Hartree when available.
 */
NWChemCResult nwchemc_dipole(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *dipole_au);

/**
 * @brief Compute total traceless electric quadrupole for an atomic configuration.
 *
 * @param n_atoms Number of atoms.
 * @param positions_ang Flat Cartesian coordinate array in Angstrom, length
 *        `n_atoms * 3`.
 * @param atomic_numbers Atomic number array, length `n_atoms`.
 * @param params_capnp Pointer to an unpacked flat `NWChemParams` message.
 * @param params_capnp_size_bytes Size of `params_capnp` in bytes.
 * @param quadrupole_au Output quadrupole tensor components in atomic units,
 *        length 6, ordered `xx, xy, xz, yy, yz, zz`.
 * @return Calculation result and status message. `energy_h` carries the
 *         associated total energy in Hartree when available.
 */
NWChemCResult nwchemc_quadrupole(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *quadrupole_au);

/**
 * @brief Compute a periodic stress tensor for an atomic configuration.
 *
 * @param n_atoms Number of atoms.
 * @param positions_ang Flat Cartesian coordinate array in Angstrom, length
 *        `n_atoms * 3`.
 * @param atomic_numbers Atomic number array, length `n_atoms`.
 * @param params_capnp Pointer to an unpacked flat `NWChemParams` message.
 * @param params_capnp_size_bytes Size of `params_capnp` in bytes.
 * @param stress_au Output row-major 3x3 stress tensor in NWChem atomic stress
 *        units, length 9.
 * @return Calculation result and status message. `energy_h` carries the
 *         associated total energy in Hartree when available.
 */
NWChemCResult nwchemc_stress(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *stress_au);

/**
 * @brief Optimize an atomic configuration and return final coordinates.
 *
 * @param n_atoms Number of atoms.
 * @param positions_ang Flat Cartesian coordinate array in Angstrom, length
 *        `n_atoms * 3`.
 * @param atomic_numbers Atomic number array, length `n_atoms`.
 * @param params_capnp Pointer to an unpacked flat `NWChemParams` message.
 * @param params_capnp_size_bytes Size of `params_capnp` in bytes.
 * @param optimized_positions_ang Output optimized Cartesian coordinates in
 *        Angstrom, length `n_atoms * 3`.
 * @return Calculation result and status message. `energy_h` carries the final
 *         optimized energy in Hartree when available.
 */
NWChemCResult nwchemc_optimize(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *optimized_positions_ang);

/**
 * @brief Compute harmonic vibrational frequencies for an atomic configuration.
 *
 * The frequency buffer has length `n_atoms * 3` and is returned in cm^-1.
 * When `intensities_au` is non-NULL, it must also have length `n_atoms * 3`
 * and receives the atomic-unit IR intensity values stored by NWChem under
 * `vib:intensities`; entries are zero when NWChem does not store intensities.
 */
NWChemCResult nwchemc_frequencies(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *frequencies_cm1, double *intensities_au);

/**
 * @brief Create a persistent evaluation session from a Cap'n Proto message.
 *
 * The session owns a copy of the serialized message so callers may pass a
 * buffer produced by pycapnp, capnp-c, or mmap-backed storage and release it
 * after this call returns.
 */
NWChemCSession *nwchemc_session_create(const void *params_capnp,
                                       size_t params_capnp_size_bytes);

/**
 * @brief Replace the Cap'n Proto parameter message for an existing session.
 *
 * Replacement is accepted only before the session has accepted a topology from
 * an evaluation call. After that point, create a separate session for a new
 * configuration.
 */
int nwchemc_session_set_params(NWChemCSession *session,
                               const void *params_capnp,
                               size_t params_capnp_size_bytes);

/**
 * @brief Release a persistent evaluation session.
 */
void nwchemc_session_destroy(NWChemCSession *session);

/**
 * @brief Compute energy and gradient using a persistent session.
 */
NWChemCResult
nwchemc_session_energy_gradient(NWChemCSession *session, int n_atoms,
                                const double *positions_ang,
                                const int *atomic_numbers,
                                double *grad_h_bohr);

/**
 * @brief Compute energy using a persistent session.
 */
NWChemCResult nwchemc_session_energy(NWChemCSession *session, int n_atoms,
                                     const double *positions_ang,
                                     const int *atomic_numbers);

/**
 * @brief Compute energy and forces using a persistent session.
 */
NWChemCResult nwchemc_session_energy_forces(NWChemCSession *session,
                                            int n_atoms,
                                            const double *positions_ang,
                                            const int *atomic_numbers,
                                            double *forces_h_bohr);

/**
 * @brief Compute total electric dipole using a persistent session.
 */
NWChemCResult nwchemc_session_dipole(NWChemCSession *session, int n_atoms,
                                     const double *positions_ang,
                                     const int *atomic_numbers,
                                     double *dipole_au);

/**
 * @brief Compute total traceless electric quadrupole using a persistent session.
 */
NWChemCResult nwchemc_session_quadrupole(NWChemCSession *session, int n_atoms,
                                         const double *positions_ang,
                                         const int *atomic_numbers,
                                         double *quadrupole_au);

/**
 * @brief Optimize an atomic configuration using a persistent session.
 */
NWChemCResult nwchemc_session_optimize(NWChemCSession *session, int n_atoms,
                                       const double *positions_ang,
                                       const int *atomic_numbers,
                                       double *optimized_positions_ang);

/**
 * @brief Compute harmonic vibrational frequencies using a persistent session.
 */
NWChemCResult nwchemc_session_frequencies(
    NWChemCSession *session, int n_atoms, const double *positions_ang,
    const int *atomic_numbers, double *frequencies_cm1,
    double *intensities_au);

/**
 * @brief Compute energy and forces for one Cap'n Proto `ForceInput` step.
 *
 * This is the session-oriented potential loop entry point. The session keeps
 * the persistent `NWChemParams` method state while every call supplies a
 * `ForceInput` message containing positions, atomic numbers, and optional
 * 3x3 cell vectors. Returned energy and forces use NWChem native units:
 * Hartree and Hartree/Bohr.
 *
 * @param session Persistent session created from `NWChemParams`.
 * @param force_input_capnp Pointer to an unpacked flat `ForceInput` message.
 * @param force_input_capnp_size_bytes Size of `force_input_capnp` in bytes.
 * @param forces_h_bohr Output force array in Hartree/Bohr.
 * @param forces_len Number of doubles available in `forces_h_bohr`.
 */
NWChemCResult nwchemc_session_calculate_forces(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *forces_h_bohr,
    size_t forces_len);

/**
 * @brief Compute energy for one Cap'n Proto `ForceInput` step.
 *
 * This is the session-oriented energy-only potential loop entry point. The
 * returned scalar energy is in Hartree.
 */
NWChemCResult nwchemc_session_calculate_energy(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes);

/**
 * @brief Compute one `ForceInput` step and write a raw force buffer.
 *
 * This is the one-shot Cap'n Proto force entry point for callers that do not
 * keep a persistent session. The force buffer is returned in Hartree/Bohr.
 */
NWChemCResult nwchemc_calculate_forces(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *forces_h_bohr, size_t forces_len);

/**
 * @brief Compute one `ForceInput` step and return scalar energy.
 *
 * This is the one-shot Cap'n Proto energy entry point for callers that do not
 * keep a persistent session. The returned scalar energy is in Hartree.
 */
NWChemCResult nwchemc_calculate_energy(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes);

/**
 * @brief Return the byte count needed for an energy-only `PotentialResult`.
 *
 * This parses the serialized `ForceInput` geometry and returns the size of the
 * unpacked flat `PotentialResult` message with only its `energy` field
 * populated. The function does not initialize or evaluate NWChem.
 */
size_t nwchemc_energy_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes);

/**
 * @brief Compute energy for one `ForceInput` step and write `PotentialResult`.
 *
 * The returned `NWChemCResult.energy_h` remains in Hartree. The output message
 * writes `PotentialResult.energy` in `ForceInput.energyUnit` and leaves
 * `PotentialResult.forces` empty.
 *
 * When `potential_result_capnp_capacity_bytes` is too small, the function
 * returns `ok == 0`, writes the required byte count to
 * `potential_result_capnp_size_bytes`, and does not evaluate NWChem.
 */
NWChemCResult nwchemc_session_calculate_energy_result(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);

/**
 * @brief One-shot `NWChemParams + ForceInput -> PotentialResult.energy`.
 */
NWChemCResult nwchemc_calculate_energy_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);

/**
 * @brief Compute forces for one `ForceInput` step and write `PotentialResult`.
 *
 * This is the Cap'n Proto result-carrier entry point for RPC-style callers.
 * The session keeps persistent `NWChemParams` method state, each call supplies
 * a serialized `ForceInput`, and the caller provides a byte buffer that is
 * filled with an unpacked flat `PotentialResult` message. The output message
 * converts energy and forces to `ForceInput.energyUnit` and
 * `ForceInput.energyUnit / ForceInput.lengthUnit`.
 *
 * When `potential_result_capnp_capacity_bytes` is too small, the function
 * returns `ok == 0`, writes the required byte count to
 * `potential_result_capnp_size_bytes`, and does not evaluate NWChem.
 */
NWChemCResult nwchemc_session_calculate_result(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);

/**
 * @brief Compute one `ForceInput` step and write `PotentialResult`.
 *
 * This is the one-shot Cap'n Proto entry point for callers that do not keep a
 * persistent session. Multi-step callers should create one session and call
 * `nwchemc_session_calculate_result()` for each step so the `NWChemParams`
 * method state is applied once and reused across the loop.
 *
 * When `potential_result_capnp_capacity_bytes` is too small, the function
 * returns `ok == 0`, writes the required byte count to
 * `potential_result_capnp_size_bytes`, and does not initialize or evaluate
 * NWChem.
 */
NWChemCResult nwchemc_calculate_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);

/**
 * @brief Compute one `ForceInput` step and write a dense Hessian buffer.
 *
 * This is the one-shot Cap'n Proto Hessian entry point for callers that do not
 * keep a persistent session. The dense row-major Hessian is returned in
 * Hartree/Bohr^2.
 */
NWChemCResult nwchemc_calculate_hessian(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *hessian_h_bohr2, size_t hessian_len);

/**
 * @brief Return the byte count needed for a Hessian `PotentialResult`.
 *
 * This parses the serialized `ForceInput` geometry and returns the size of the
 * unpacked flat `PotentialResult` message with its `hessian` field populated.
 * The function does not initialize or evaluate NWChem.
 */
size_t nwchemc_hessian_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes);

/**
 * @brief Compute one `ForceInput` Hessian and write `PotentialResult.hessian`.
 *
 * The Hessian values are converted from Hartree/Bohr^2 to
 * `ForceInput.energyUnit / ForceInput.lengthUnit^2`.
 */
NWChemCResult nwchemc_session_calculate_hessian_result(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);

/**
 * @brief One-shot `NWChemParams + ForceInput -> PotentialResult.hessian`.
 */
NWChemCResult nwchemc_calculate_hessian_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);

/**
 * @brief Compute one `ForceInput` step and write a total dipole vector.
 *
 * This is the one-shot Cap'n Proto dipole entry point for callers that do not
 * keep a persistent session. The three-vector is returned in atomic units.
 */
NWChemCResult nwchemc_calculate_dipole(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *dipole_au, size_t dipole_len);

/** @brief Return the byte count needed for a dipole `PotentialResult`. */
size_t nwchemc_dipole_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes);

/** @brief Compute one `ForceInput` dipole and write `PotentialResult.dipole`. */
NWChemCResult nwchemc_session_calculate_dipole_result(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);

/** @brief One-shot `NWChemParams + ForceInput -> PotentialResult.dipole`. */
NWChemCResult nwchemc_calculate_dipole_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);

/**
 * @brief Compute one `ForceInput` step and write a quadrupole tensor.
 *
 * This is the one-shot Cap'n Proto quadrupole entry point for callers that do
 * not keep a persistent session. The six tensor components are returned in
 * atomic units as `xx, xy, xz, yy, yz, zz`.
 */
NWChemCResult nwchemc_calculate_quadrupole(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *quadrupole_au, size_t quadrupole_len);

/** @brief Return the byte count needed for a quadrupole `PotentialResult`. */
size_t nwchemc_quadrupole_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes);

/**
 * @brief Compute one `ForceInput` quadrupole and write
 * `PotentialResult.quadrupole`.
 */
NWChemCResult nwchemc_session_calculate_quadrupole_result(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);

/** @brief One-shot `NWChemParams + ForceInput -> PotentialResult.quadrupole`. */
NWChemCResult nwchemc_calculate_quadrupole_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);

/**
 * @brief Compute one `ForceInput` step and write a stress tensor.
 *
 * This is the one-shot Cap'n Proto stress entry point for callers that do not
 * keep a persistent session. The nine tensor components are returned in
 * NWChem atomic stress units in row-major 3x3 order.
 */
NWChemCResult nwchemc_calculate_stress(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *stress_au, size_t stress_len);

/** @brief Return the byte count needed for a stress `PotentialResult`. */
size_t nwchemc_stress_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes);

/**
 * @brief Compute one `ForceInput` stress and write `PotentialResult.stress`.
 *
 * Stress values are converted from NWChem atomic stress units to
 * `ForceInput.energyUnit / ForceInput.lengthUnit^3`.
 */
NWChemCResult nwchemc_session_calculate_stress_result(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);

/** @brief One-shot `NWChemParams + ForceInput -> PotentialResult.stress`. */
NWChemCResult nwchemc_calculate_stress_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);

/**
 * @brief Optimize one `ForceInput` step and write final coordinates.
 *
 * This is the one-shot Cap'n Proto optimization entry point for callers that
 * do not keep a persistent session. The output coordinates are returned in
 * Angstrom with length `n_atoms * 3`.
 */
NWChemCResult nwchemc_calculate_optimize(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *optimized_positions_ang, size_t optimized_positions_len);

/** @brief Return the byte count needed for an optimization `PotentialResult`. */
size_t nwchemc_optimize_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes);

/**
 * @brief Optimize one `ForceInput` step and write
 * `PotentialResult.optimizedPos`.
 *
 * Final coordinates are converted from Angstrom to `ForceInput.lengthUnit`;
 * `PotentialResult.energy` uses `ForceInput.energyUnit`.
 */
NWChemCResult nwchemc_session_calculate_optimize_result(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);

/**
 * @brief One-shot `NWChemParams + ForceInput -> PotentialResult.optimizedPos`.
 */
NWChemCResult nwchemc_calculate_optimize_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);

/**
 * @brief Compute one `ForceInput` step and write vibrational frequencies.
 *
 * This is the one-shot Cap'n Proto frequency entry point for callers that do
 * not keep a persistent session. The frequency output length is `n_atoms * 3`;
 * the intensity output is optional and uses the same length.
 */
NWChemCResult nwchemc_calculate_frequencies(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    double *frequencies_cm1, size_t frequencies_len, double *intensities_au,
    size_t intensities_len);

/** @brief Return the byte count needed for a frequencies `PotentialResult`. */
size_t nwchemc_frequencies_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes);

/**
 * @brief Compute harmonic frequencies and write `PotentialResult` lists.
 *
 * `PotentialResult.frequencies` is in cm^-1 and
 * `PotentialResult.intensities` is in atomic units.
 */
NWChemCResult nwchemc_session_calculate_frequencies_result(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);

/**
 * @brief One-shot `NWChemParams + ForceInput -> PotentialResult.frequencies`.
 */
NWChemCResult nwchemc_calculate_frequencies_result(
    const void *params_capnp, size_t params_capnp_size_bytes,
    const void *force_input_capnp, size_t force_input_capnp_size_bytes,
    void *potential_result_capnp,
    size_t potential_result_capnp_capacity_bytes,
    size_t *potential_result_capnp_size_bytes);

/**
 * @brief Return the byte count needed for a `PotentialResult` step output.
 *
 * This parses the serialized `ForceInput` geometry and returns the size of the
 * unpacked flat `PotentialResult` message that
 * `nwchemc_session_calculate_result()` writes for the same step. The function
 * does not initialize or evaluate NWChem. It returns 0 when the `ForceInput`
 * message is invalid or too large for the C ABI.
 */
size_t nwchemc_potential_result_size_for_force_input(
    const void *force_input_capnp, size_t force_input_capnp_size_bytes);

/**
 * @brief Compute a Cartesian Hessian for one Cap'n Proto `ForceInput` step.
 *
 * The session keeps persistent `NWChemParams` method state while the step
 * message supplies positions, atomic numbers, and optional 3x3 cell vectors.
 * The dense row-major Hessian is returned in Hartree/Bohr^2.
 *
 * @param session Persistent session created from `NWChemParams`.
 * @param force_input_capnp Pointer to an unpacked flat `ForceInput` message.
 * @param force_input_capnp_size_bytes Size of `force_input_capnp` in bytes.
 * @param hessian_h_bohr2 Output Hessian buffer in Hartree/Bohr^2.
 * @param hessian_len Number of doubles available in `hessian_h_bohr2`.
 */
NWChemCResult nwchemc_session_calculate_hessian(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *hessian_h_bohr2,
    size_t hessian_len);

/**
 * @brief Compute dipole for one Cap'n Proto `ForceInput` step.
 *
 * The session keeps persistent `NWChemParams` method state while the step
 * message supplies positions, atomic numbers, and optional 3x3 cell vectors.
 * The dipole vector is returned in atomic units.
 *
 * @param session Persistent session created from `NWChemParams`.
 * @param force_input_capnp Pointer to an unpacked flat `ForceInput` message.
 * @param force_input_capnp_size_bytes Size of `force_input_capnp` in bytes.
 * @param dipole_au Output total dipole vector in atomic units.
 * @param dipole_len Number of doubles available in `dipole_au`.
 */
NWChemCResult nwchemc_session_calculate_dipole(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *dipole_au, size_t dipole_len);

/**
 * @brief Compute quadrupole for one Cap'n Proto `ForceInput` step.
 *
 * The session keeps persistent `NWChemParams` method state while the step
 * message supplies positions, atomic numbers, and optional 3x3 cell vectors.
 * The quadrupole tensor is returned in atomic units as
 * `xx, xy, xz, yy, yz, zz`.
 *
 * @param session Persistent session created from `NWChemParams`.
 * @param force_input_capnp Pointer to an unpacked flat `ForceInput` message.
 * @param force_input_capnp_size_bytes Size of `force_input_capnp` in bytes.
 * @param quadrupole_au Output quadrupole tensor components in atomic units.
 * @param quadrupole_len Number of doubles available in `quadrupole_au`.
 */
NWChemCResult nwchemc_session_calculate_quadrupole(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *quadrupole_au,
    size_t quadrupole_len);

/**
 * @brief Compute stress for one Cap'n Proto `ForceInput` step.
 *
 * The session keeps persistent `NWChemParams` method state while the step
 * message supplies positions, atomic numbers, and optional 3x3 cell vectors.
 * The stress tensor is returned in NWChem atomic stress units in row-major
 * 3x3 order.
 *
 * @param session Persistent session created from `NWChemParams`.
 * @param force_input_capnp Pointer to an unpacked flat `ForceInput` message.
 * @param force_input_capnp_size_bytes Size of `force_input_capnp` in bytes.
 * @param stress_au Output stress tensor in NWChem atomic stress units.
 * @param stress_len Number of doubles available in `stress_au`.
 */
NWChemCResult nwchemc_session_calculate_stress(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *stress_au,
    size_t stress_len);

/**
 * @brief Optimize one Cap'n Proto `ForceInput` step.
 *
 * The session keeps persistent `NWChemParams` method state while the step
 * message supplies positions, atomic numbers, and optional 3x3 cell vectors.
 * The output coordinates are returned in Angstrom.
 *
 * @param session Persistent session created from `NWChemParams`.
 * @param force_input_capnp Pointer to an unpacked flat `ForceInput` message.
 * @param force_input_capnp_size_bytes Size of `force_input_capnp` in bytes.
 * @param optimized_positions_ang Output optimized coordinates in Angstrom.
 * @param optimized_positions_len Number of doubles available in
 *        `optimized_positions_ang`.
 */
NWChemCResult nwchemc_session_calculate_optimize(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *optimized_positions_ang,
    size_t optimized_positions_len);

/**
 * @brief Compute harmonic frequencies for one Cap'n Proto `ForceInput` step.
 *
 * The session keeps persistent `NWChemParams` method state while the step
 * message supplies positions, atomic numbers, and optional 3x3 cell vectors.
 * The frequency output is returned in cm^-1 with length `n_atoms * 3`.
 * Atomic-unit intensity output is optional and uses the same length.
 */
NWChemCResult nwchemc_session_calculate_frequencies(
    NWChemCSession *session, const void *force_input_capnp,
    size_t force_input_capnp_size_bytes, double *frequencies_cm1,
    size_t frequencies_len, double *intensities_au, size_t intensities_len);

/**
 * @brief Compute stress using a persistent session.
 */
NWChemCResult nwchemc_session_stress(NWChemCSession *session, int n_atoms,
                                     const double *positions_ang,
                                     const int *atomic_numbers,
                                     double *stress_au);

/**
 * @brief Compute Hessian using a persistent session.
 */
NWChemCResult nwchemc_session_hessian(NWChemCSession *session, int n_atoms,
                                      const double *positions_ang,
                                      const int *atomic_numbers,
                                      double *hessian_h_bohr2);

/**
 * @brief Return the compiled library version string.
 */
const char *nwchemc_version(void);

/**
 * @brief Return 1 when the embedded NWChem runtime is available.
 */
int nwchemc_available(void);

/**
 * @brief Finalize an owned embedded NWChem runtime.
 */
void nwchemc_finalize(void);

#ifdef __cplusplus
}
#endif
