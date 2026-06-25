#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NWChemCResult {
  int ok;
  double energy_h;
  char message[512];
} NWChemCResult;

int nwchemc_set_params(const void *params_capnp,
                       size_t params_capnp_size_bytes);

NWChemCResult nwchemc_energy_gradient(
    int n_atoms, const double *positions_ang, const int *atomic_numbers,
    const void *params_capnp, size_t params_capnp_size_bytes,
    double *grad_h_bohr);

const char *nwchemc_version(void);

int nwchemc_available(void);

#ifdef __cplusplus
}
#endif
