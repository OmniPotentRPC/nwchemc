#include "nwchemc.h"

#include <stdio.h>
#include <string.h>

int main(void) {
  if (nwchemc_available() != 0) {
    fprintf(stderr, "stub should report unavailable\n");
    return 1;
  }
  const char *version = nwchemc_version();
  if (!version || strstr(version, "stub") == NULL) {
    fprintf(stderr, "unexpected version: %s\n", version ? version : "(null)");
    return 1;
  }
  if (nwchemc_set_params(NULL, 0) == 0) {
    fprintf(stderr, "stub set_params should fail\n");
    return 1;
  }
  NWChemCResult result =
      nwchemc_energy_gradient(0, NULL, NULL, NULL, 0, NULL);
  if (result.ok != 0) {
    fprintf(stderr, "stub energy_gradient should fail\n");
    return 1;
  }
  return 0;
}
