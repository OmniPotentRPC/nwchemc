#include <dlfcn.h>
#include <stdio.h>

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

  available_fn available = (available_fn)dlsym(handle, "nwchemc_available");
  finalize_fn finalize = (finalize_fn)dlsym(handle, "nwchemc_finalize");
  if (!available || !finalize) {
    fprintf(stderr, "missing lifecycle symbols\n");
    return 1;
  }
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
