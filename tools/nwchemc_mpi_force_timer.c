/* Multi-rank timer for public nwchemc_energy_gradient (strong-scaling probe).
 *
 * Usage:
 *   mpirun -np P nwchemc_mpi_force_timer params.bin [libnwchemc.so]
 *
 * Geometry (priority order):
 *   1) NWCHEMC_TIMER_GEOM=path  — text file: first line n_atoms, then
 *      "Z x y z" per atom (Angstrom)
 *   2) NWCHEMC_TIMER_SYSTEM=h2|water|benzene (built-in)
 *
 * Rank 0 prints:
 *   nwchemc_mpi_force system=... ranks=P wall_s=... energy_h=... maxabs_g=... ok=... msg=...
 */
#include <dlfcn.h>
#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

static unsigned char *read_all(const char *path, size_t *size) {
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

/* Load geom file: n_atoms\n Z x y z ...  Returns 0 on success. */
static int load_geom_file(const char *path, int *n_atoms, int **Z, double **xyz) {
  FILE *fp = fopen(path, "r");
  int n, i, z;
  double x, y, zz;
  if (!fp)
    return -1;
  if (fscanf(fp, "%d", &n) != 1 || n < 1 || n > 512) {
    fclose(fp);
    return -1;
  }
  *Z = (int *)calloc((size_t)n, sizeof(int));
  *xyz = (double *)calloc((size_t)n * 3u, sizeof(double));
  if (!*Z || !*xyz) {
    free(*Z);
    free(*xyz);
    fclose(fp);
    return -1;
  }
  for (i = 0; i < n; ++i) {
    if (fscanf(fp, "%d %lf %lf %lf", &z, &x, &y, &zz) != 4) {
      free(*Z);
      free(*xyz);
      fclose(fp);
      return -1;
    }
    (*Z)[i] = z;
    (*xyz)[3 * i] = x;
    (*xyz)[3 * i + 1] = y;
    (*xyz)[3 * i + 2] = zz;
  }
  fclose(fp);
  *n_atoms = n;
  return 0;
}

static void fill_builtin(const char *name, int *n_atoms, int *Z, double *xyz) {
  if (strcmp(name, "water") == 0) {
    *n_atoms = 3;
    Z[0] = 8;
    Z[1] = 1;
    Z[2] = 1;
    xyz[0] = 0.0;
    xyz[1] = 0.0;
    xyz[2] = 0.1173;
    xyz[3] = 0.0;
    xyz[4] = 0.7572;
    xyz[5] = -0.4692;
    xyz[6] = 0.0;
    xyz[7] = -0.7572;
    xyz[8] = -0.4692;
  } else if (strcmp(name, "benzene") == 0) {
    /* Planar benzene, C-C ~1.39 A, C-H ~1.09 A (Angstrom). */
    *n_atoms = 12;
    {
      const double cc = 1.397;
      const double ch = 1.084;
      int i;
      for (i = 0; i < 6; ++i) {
        double a = (3.14159265358979323846 / 3.0) * (double)i;
        double cx = cc * cos(a);
        double cy = cc * sin(a);
        Z[i] = 6;
        xyz[3 * i] = cx;
        xyz[3 * i + 1] = cy;
        xyz[3 * i + 2] = 0.0;
        Z[6 + i] = 1;
        xyz[3 * (6 + i)] = (cc + ch) * cos(a);
        xyz[3 * (6 + i) + 1] = (cc + ch) * sin(a);
        xyz[3 * (6 + i) + 2] = 0.0;
      }
    }
  } else {
    /* h2 default */
    *n_atoms = 2;
    Z[0] = 1;
    Z[1] = 1;
    xyz[0] = 0.0;
    xyz[1] = 0.0;
    xyz[2] = -0.3707;
    xyz[3] = 0.0;
    xyz[4] = 0.0;
    xyz[5] = 0.3707;
  }
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
  const char *geom_path = getenv("NWCHEMC_TIMER_GEOM");
  const char *system_name =
      getenv("NWCHEMC_TIMER_SYSTEM") ? getenv("NWCHEMC_TIMER_SYSTEM") : "h2";
  int n_atoms = 0;
  int *atomic_numbers = NULL;
  double *positions_ang = NULL;
  double *grad = NULL;
  int heap_geom = 0;
  int Z_store[64];
  double xyz_store[192];
  double grad_store[192];
  double t0, t1, maxg;
  int i, rc = 1, ncoord;
  NWChemCResult result;

  mpi_err = MPI_Init(&argc, &argv);
  if (mpi_err != MPI_SUCCESS) {
    fprintf(stderr, "MPI_Init failed\n");
    return 1;
  }
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  if (argc < 2) {
    if (rank == 0)
      fprintf(stderr,
              "usage: %s params.bin [libnwchemc.so]\n"
              "  NWCHEMC_TIMER_SYSTEM=h2|water|benzene\n"
              "  or NWCHEMC_TIMER_GEOM=file (n_atoms then Z x y z lines)\n",
              argv[0]);
    MPI_Finalize();
    return 2;
  }

  if (geom_path && geom_path[0]) {
    if (load_geom_file(geom_path, &n_atoms, &atomic_numbers, &positions_ang) !=
        0) {
      if (rank == 0)
        fprintf(stderr, "failed to load geom %s\n", geom_path);
      MPI_Finalize();
      return 1;
    }
    heap_geom = 1;
    system_name = geom_path;
    grad = (double *)calloc((size_t)n_atoms * 3u, sizeof(double));
    if (!grad) {
      MPI_Finalize();
      return 1;
    }
  } else {
    atomic_numbers = Z_store;
    positions_ang = xyz_store;
    fill_builtin(system_name, &n_atoms, Z_store, xyz_store);
    grad = grad_store;
    memset(grad_store, 0, sizeof(grad_store));
  }
  ncoord = n_atoms * 3;

  params_path = argv[1];
  libpath = (argc >= 3) ? argv[2] : getenv("NWCHEMC_LIBRARY");
  if (!libpath || !libpath[0])
    libpath = "libnwchemc.so";

  params = read_all(params_path, &params_size);
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
      fprintf(stderr, "missing nwchemc symbols\n");
    free(params);
    MPI_Finalize();
    return 1;
  }
  if (!available()) {
    if (rank == 0)
      fprintf(stderr, "nwchemc_available()=0\n");
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

  maxg = 0.0;
  for (i = 0; i < ncoord; ++i) {
    double a = fabs(grad[i]);
    if (a > maxg)
      maxg = a;
  }

  if (rank == 0) {
    printf("nwchemc_mpi_force system=%s n_atoms=%d ranks=%d wall_s=%.6f "
           "energy_h=%.12g maxabs_g=%.6e ok=%d msg=%s\n",
           system_name, n_atoms, nprocs, t1 - t0, result.energy_h, maxg,
           result.ok, result.message[0] ? result.message : "");
    fflush(stdout);
  }

  if (finalize)
    finalize();
  if (heap_geom) {
    free(atomic_numbers);
    free(positions_ang);
    free(grad);
  }

  rc = (result.ok && isfinite(result.energy_h) && isfinite(maxg)) ? 0 : 1;
  MPI_Finalize();
  return rc;
}
