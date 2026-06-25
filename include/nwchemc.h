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
