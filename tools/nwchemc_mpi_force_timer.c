/* Multi-rank timer for public nwchemc_energy_gradient (strong-scaling probe).
 *
 * Usage:
 *   mpirun -np P nwchemc_mpi_force_timer params.bin [libnwchemc.so]
 *
 * Rank 0 prints one machine-readable line:
 *   nwchemc_mpi_force ranks=P wall_s=... energy_h=... maxabs_g=... ok=... msg=...
 *
 * Requires host MPI (Open MPI). Embed may co-own GA/MPI via pbeginf; calling
 * MPI_Init here before dlopen matches the shipped multi-rank test contract.
 */
#include <dlfcn.h>
#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Must match include/nwchemc.h NWChemCResult (message[512]). */
typedef struct {
  int ok;
  double energy_h;
  char message[512];
} NWChemCResult;

typedef int (*available_fn)(void);
typedef NWChemCResult (*energy_gradient_fn)(int, const double *, const int *,
                                            const void *, size_t, double *);
typedef void (*finalize_fn)(void);

static double wall_seconds(void) {
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
    return 0.0;
  return (double)ts.tv_sec + 1e-9 * (double)ts.tv_nsec;
}

static unsigned char *read_file(const char *path, size_t *size) {
  FILE *fp = fopen(path, "rb");
  long n;
  unsigned char *buf;
  if (!fp)
    return NULL;
  if (fseek(fp, 0, SEEK_END) != 0) {
    fclose(fp);
    return NULL;
  }
  n = ftell(fp);
  if (n <= 0) {
    fclose(fp);
    return NULL;
  }
  rewind(fp);
  buf = (unsigned char *)malloc((size_t)n);
  if (!buf) {
    fclose(fp);
    return NULL;
  }
  if (fread(buf, 1, (size_t)n, fp) != (size_t)n) {
    free(buf);
    fclose(fp);
    return NULL;
  }
  fclose(fp);
  *size = (size_t)n;
  return buf;
}

int main(int argc, char **argv) {
  int rank = 0, nprocs = 1, mpi_err;
  size_t params_size = 0;
  unsigned char *params = NULL;
  void *h = NULL;
  available_fn available = NULL;
  energy_gradient_fn energy_gradient = NULL;
  finalize_fn finalize = NULL;
  const char *params_path;
  const char *libpath;
  /* Geometry: env NWCHEMC_TIMER_SYSTEM=h2 (default) | water */
  const char *system_name =
      getenv("NWCHEMC_TIMER_SYSTEM") ? getenv("NWCHEMC_TIMER_SYSTEM") : "h2";
  int n_atoms = 2;
  int atomic_numbers_store[16];
  double positions_ang_store[48];
  double grad_store[48];
  const int *atomic_numbers = atomic_numbers_store;
  const double *positions_ang = positions_ang_store;
  double *grad = grad_store;
  double t0, t1, maxg;
  int i, rc = 1, ncoord;
  NWChemCResult result;

  if (strcmp(system_name, "water") == 0) {
    n_atoms = 3;
    atomic_numbers_store[0] = 8;
    atomic_numbers_store[1] = 1;
    atomic_numbers_store[2] = 1;
    /* Match tools large-scale water_scf_gradient.nw */
    positions_ang_store[0] = 0.0;
    positions_ang_store[1] = 0.0;
    positions_ang_store[2] = 0.1173;
    positions_ang_store[3] = 0.0;
    positions_ang_store[4] = 0.7572;
    positions_ang_store[5] = -0.4692;
    positions_ang_store[6] = 0.0;
    positions_ang_store[7] = -0.7572;
    positions_ang_store[8] = -0.4692;
  } else {
    n_atoms = 2;
    atomic_numbers_store[0] = 1;
    atomic_numbers_store[1] = 1;
    positions_ang_store[0] = 0.0;
    positions_ang_store[1] = 0.0;
    positions_ang_store[2] = -0.3707;
    positions_ang_store[3] = 0.0;
    positions_ang_store[4] = 0.0;
    positions_ang_store[5] = 0.3707;
  }
  ncoord = n_atoms * 3;
  for (i = 0; i < ncoord; ++i)
    grad_store[i] = 0.0;

  mpi_err = MPI_Init(&argc, &argv);
  if (mpi_err != MPI_SUCCESS) {
    fprintf(stderr, "MPI_Init failed\n");
    return 1;
  }
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  if (argc < 2) {
    if (rank == 0)
      fprintf(stderr, "usage: %s params.bin [libnwchemc.so]\n", argv[0]);
    MPI_Finalize();
    return 2;
  }
  params_path = argv[1];
  libpath = (argc >= 3) ? argv[2] : getenv("NWCHEMC_LIBRARY");
  if (!libpath || !libpath[0])
    libpath = "libnwchemc.so";

  params = read_file(params_path, &params_size);
  if (!params) {
    if (rank == 0)
      fprintf(stderr, "failed to read %s\n", params_path);
    MPI_Finalize();
    return 1;
  }

  h = dlopen(libpath, RTLD_NOW | RTLD_GLOBAL);
  if (!h) {
    if (rank == 0)
      fprintf(stderr, "dlopen %s: %s\n", libpath, dlerror());
    free(params);
    MPI_Finalize();
    return 1;
  }
  available = (available_fn)dlsym(h, "nwchemc_available");
  energy_gradient = (energy_gradient_fn)dlsym(h, "nwchemc_energy_gradient");
  finalize = (finalize_fn)dlsym(h, "nwchemc_finalize");
  if (!available || !energy_gradient) {
    if (rank == 0)
      fprintf(stderr, "missing nwchemc_available / nwchemc_energy_gradient\n");
    free(params);
    MPI_Finalize();
    return 1;
  }
  if (!available()) {
    if (rank == 0)
      fprintf(stderr, "nwchemc_available() returned 0\n");
    free(params);
    MPI_Finalize();
    return 1;
  }

  MPI_Barrier(MPI_COMM_WORLD);
  t0 = wall_seconds();
  result = energy_gradient(n_atoms, positions_ang, atomic_numbers, params,
                           params_size, grad);
  t1 = wall_seconds();
  free(params);
  params = NULL;

  maxg = 0.0;
  for (i = 0; i < ncoord; ++i) {
    double a = fabs(grad[i]);
    if (a > maxg)
      maxg = a;
  }

  if (rank == 0) {
    printf("nwchemc_mpi_force system=%s ranks=%d wall_s=%.6f energy_h=%.12g "
           "maxabs_g=%.6e ok=%d msg=%s\n",
           system_name, nprocs, t1 - t0, result.energy_h, maxg, result.ok,
           result.message[0] ? result.message : "");
    fflush(stdout);
  }

  if (finalize)
    finalize();

  rc = (result.ok && isfinite(result.energy_h) && isfinite(maxg)) ? 0 : 1;
  MPI_Finalize();
  return rc;
}
