#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

typedef int (*available_fn)(void);
typedef void (*finalize_fn)(void);

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s LIBNWCHEMC\n", argv[0]);
    return 2;
  }

  void *handle = dlopen(argv[1], RTLD_NOW | RTLD_GLOBAL);
  if (!handle) {
    fprintf(stderr, "dlopen failed: %s\n", dlerror());
    return 1;
  }

  void *available_symbol = dlsym(handle, "nwchemc_available");
  void *finalize_symbol = dlsym(handle, "nwchemc_finalize");
  if (!available_symbol || !finalize_symbol ||
      sizeof(available_fn) != sizeof(available_symbol) ||
      sizeof(finalize_fn) != sizeof(finalize_symbol)) {
    fprintf(stderr, "missing lifecycle symbols\n");
    return 1;
  }
  available_fn available = NULL;
  finalize_fn finalize = NULL;
  memcpy(&available, &available_symbol, sizeof(available));
  memcpy(&finalize, &finalize_symbol, sizeof(finalize));
  if (available() != 1) {
    fprintf(stderr, "embedded runtime is unavailable\n");
    return 1;
  }

  finalize();
  if (dlclose(handle) != 0) {
    fprintf(stderr, "dlclose failed: %s\n", dlerror());
    return 1;
  }

  return 0;
}
