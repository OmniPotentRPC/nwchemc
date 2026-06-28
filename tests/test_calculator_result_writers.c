/* Public calculator *_result matrix: session APIs under weak/mock embed. */
#include "nwchemc.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <setjmp.h>
#include <cmocka.h>

#include "schema/Potentials.capnp.h"
#include <capn.h>

static const char *g_params_path = NULL;
static const char *g_force_step_path = NULL;
static double g_last_energy_h = -1.0;

void nwchemc_embed_init(void) {}
void nwchemc_embed_finalize(void) {}
int nwchemc_embed_available(void) { return 1; }
int nwchemc_embed_reset_rtdb(void) { return 0; }

int nwchemc_embed_set_config(const char *basis, int basis_len,
                             const char *theory, int theory_len,
                             const char *scf_type, int scf_len,
                             const int *charge, const int *mult,
                             const char *input_blocks, int input_blocks_len) {
  (void)basis; (void)basis_len; (void)theory; (void)theory_len;
  (void)scf_type; (void)scf_len; (void)charge; (void)mult;
  (void)input_blocks; (void)input_blocks_len;
  return 0;
}
int nwchemc_embed_set_dft_direct(const char *xc, int xc_len, int direct_enabled,
                                 int smearing_enabled,
                                 double smear_sigma_hartree,
                                 int smearing_spinset) {
  (void)xc; (void)xc_len; (void)direct_enabled; (void)smearing_enabled;
  (void)smear_sigma_hartree; (void)smearing_spinset;
  return 0;
}
int nwchemc_embed_set_scf_direct(int has_options, int maxiter, double thresh,
                                 double tol2e) {
  (void)has_options; (void)maxiter; (void)thresh; (void)tol2e;
  return 0;
}
int nwchemc_embed_set_driver_direct(int has_options, int maxiter,
                                    int tolerance_mode, double gmax_tol,
                                    double grms_tol, double xmax_tol,
                                    double xrms_tol) {
  (void)has_options; (void)maxiter; (void)tolerance_mode; (void)gmax_tol;
  (void)grms_tol; (void)xmax_tol; (void)xrms_tol;
  return 0;
}
int nwchemc_embed_set_nwpw_direct(int has_options, double energy_cutoff,
                                  double wavefunction_cutoff,
                                  double ewald_rcut, int ewald_ncut) {
  (void)has_options; (void)energy_cutoff; (void)wavefunction_cutoff;
  (void)ewald_rcut; (void)ewald_ncut;
  return 0;
}
int nwchemc_embed_set_brillouin_zone(int has_options, const char *zone_name,
                                     int zone_name_len, int monkhorst_pack_x,
                                     int monkhorst_pack_y, int monkhorst_pack_z,
                                     int max_kpoints_print,
                                     const double *kvectors,
                                     int kvector_count) {
  (void)has_options; (void)zone_name; (void)zone_name_len;
  (void)monkhorst_pack_x; (void)monkhorst_pack_y; (void)monkhorst_pack_z;
  (void)max_kpoints_print; (void)kvectors; (void)kvector_count;
  return 0;
}
int nwchemc_embed_set_brillouin_dos_zones(const char *zone_names,
                                          const int *zone_grids, int count) {
  (void)zone_names; (void)zone_grids; (void)count;
  return 0;
}
int nwchemc_embed_set_pseudopotentials(const char *elements,
                                       const int *library_types,
                                       const char *library_names, int count) {
  (void)elements; (void)library_types; (void)library_names; (void)count;
  return 0;
}
int nwchemc_embed_set_rtdb_strings(const char *keys, const char *values,
                                   int count) {
  (void)keys; (void)values; (void)count;
  return 0;
}
int nwchemc_embed_set_rtdb_values(const char *keys, const int *value_types,
                                  const int *value_counts, const char *values,
                                  int count) {
  (void)keys; (void)value_types; (void)value_counts; (void)values;
  (void)count;
  return 0;
}
int nwchemc_embed_last_energy(double *energy_h) {
  if (!energy_h)
    return -1;
  *energy_h = g_last_energy_h;
  return 0;
}
int nwchemc_embed_energy_only(const int *n_atoms, const double *positions_ang,
                              const int *atomic_numbers, const int *charge,
                              const int *multiplicity, double *energy_h,
                              char *errmsg, int errmsg_len) {
  (void)n_atoms; (void)positions_ang; (void)atomic_numbers; (void)charge;
  (void)multiplicity;
  g_last_energy_h = -1.0;
  *energy_h = g_last_energy_h;
  snprintf(errmsg, (size_t)errmsg_len, "ok");
  return 0;
}
int nwchemc_embed_energy_only_cell(const int *n_atoms,
                                   const double *positions_ang,
                                   const int *atomic_numbers,
                                   const double *cell_ang, const int *has_cell,
                                   const int *charge, const int *multiplicity,
                                   double *energy_h, char *errmsg,
                                   int errmsg_len) {
  (void)cell_ang; (void)has_cell;
  return nwchemc_embed_energy_only(n_atoms, positions_ang, atomic_numbers,
                                   charge, multiplicity, energy_h, errmsg,
                                   errmsg_len);
}
int nwchemc_embed_energy_grad(const int *n_atoms, const double *positions_ang,
                              const int *atomic_numbers, const int *charge,
                              const int *multiplicity, double *energy_h,
                              double *grad_h_bohr, char *errmsg,
                              int errmsg_len) {
  (void)positions_ang; (void)atomic_numbers; (void)charge; (void)multiplicity;
  int ndof = (*n_atoms) * 3;
  g_last_energy_h = -1.0;
  *energy_h = g_last_energy_h;
  for (int i = 0; i < ndof; ++i)
    grad_h_bohr[i] = (double)(i + 1);
  snprintf(errmsg, (size_t)errmsg_len, "ok");
  return 0;
}
int nwchemc_embed_energy_grad_cell(const int *n_atoms,
                                   const double *positions_ang,
                                   const int *atomic_numbers,
                                   const double *cell_ang, const int *has_cell,
                                   const int *charge, const int *multiplicity,
                                   double *energy_h, double *grad_h_bohr,
                                   char *errmsg, int errmsg_len) {
  (void)cell_ang; (void)has_cell;
  return nwchemc_embed_energy_grad(n_atoms, positions_ang, atomic_numbers,
                                   charge, multiplicity, energy_h, grad_h_bohr,
                                   errmsg, errmsg_len);
}
int nwchemc_embed_hessian(const int *n_atoms, const double *positions_ang,
                          const int *atomic_numbers, const int *charge,
                          const int *multiplicity, double *hessian_h_bohr2,
                          char *errmsg, int errmsg_len) {
  (void)positions_ang; (void)atomic_numbers; (void)charge; (void)multiplicity;
  int ndof = (*n_atoms) * 3;
  g_last_energy_h = -1.125;
  for (int i = 0; i < ndof * ndof; ++i)
    hessian_h_bohr2[i] = (double)(i + 1);
  snprintf(errmsg, (size_t)errmsg_len, "ok");
  return 0;
}
int nwchemc_embed_hessian_cell(const int *n_atoms, const double *positions_ang,
                               const int *atomic_numbers,
                               const double *cell_ang, const int *has_cell,
                               const int *charge, const int *multiplicity,
                               double *hessian_h_bohr2, char *errmsg,
                               int errmsg_len) {
  (void)cell_ang; (void)has_cell;
  return nwchemc_embed_hessian(n_atoms, positions_ang, atomic_numbers, charge,
                               multiplicity, hessian_h_bohr2, errmsg,
                               errmsg_len);
}
int nwchemc_embed_dipole(const int *n_atoms, const double *positions_ang,
                         const int *atomic_numbers, const int *charge,
                         const int *multiplicity, double *energy_h,
                         double *dipole_au, char *errmsg, int errmsg_len) {
  (void)n_atoms; (void)positions_ang; (void)atomic_numbers; (void)charge;
  (void)multiplicity;
  g_last_energy_h = -1.25;
  *energy_h = g_last_energy_h;
  dipole_au[0] = 0.1; dipole_au[1] = 0.2; dipole_au[2] = 0.3;
  snprintf(errmsg, (size_t)errmsg_len, "ok");
  return 0;
}
int nwchemc_embed_dipole_cell(const int *n_atoms, const double *positions_ang,
                              const int *atomic_numbers, const double *cell_ang,
                              const int *has_cell, const int *charge,
                              const int *multiplicity, double *energy_h,
                              double *dipole_au, char *errmsg, int errmsg_len) {
  (void)cell_ang; (void)has_cell;
  return nwchemc_embed_dipole(n_atoms, positions_ang, atomic_numbers, charge,
                              multiplicity, energy_h, dipole_au, errmsg,
                              errmsg_len);
}
int nwchemc_embed_polarizability(const int *n_atoms,
                                 const double *positions_ang,
                                 const int *atomic_numbers, const int *charge,
                                 const int *multiplicity, double *energy_h,
                                 double *polarizability_au, char *errmsg,
                                 int errmsg_len) {
  (void)n_atoms; (void)positions_ang; (void)atomic_numbers; (void)charge;
  (void)multiplicity;
  g_last_energy_h = -1.3;
  *energy_h = g_last_energy_h;
  for (int i = 0; i < 12; ++i)
    polarizability_au[i] = 0.01 * (double)(i + 1);
  snprintf(errmsg, (size_t)errmsg_len, "ok");
  return 0;
}
int nwchemc_embed_polarizability_cell(
    const int *n_atoms, const double *positions_ang, const int *atomic_numbers,
    const double *cell_ang, const int *has_cell, const int *charge,
    const int *multiplicity, double *energy_h, double *polarizability_au,
    char *errmsg, int errmsg_len) {
  (void)cell_ang; (void)has_cell;
  return nwchemc_embed_polarizability(n_atoms, positions_ang, atomic_numbers,
                                      charge, multiplicity, energy_h,
                                      polarizability_au, errmsg, errmsg_len);
}
int nwchemc_embed_quadrupole(const int *n_atoms, const double *positions_ang,
                             const int *atomic_numbers, const int *charge,
                             const int *multiplicity, double *energy_h,
                             double *quadrupole_au, char *errmsg,
                             int errmsg_len) {
  (void)n_atoms; (void)positions_ang; (void)atomic_numbers; (void)charge;
  (void)multiplicity;
  g_last_energy_h = -1.4;
  *energy_h = g_last_energy_h;
  for (int i = 0; i < 6; ++i)
    quadrupole_au[i] = 0.5 * (double)(i + 1);
  snprintf(errmsg, (size_t)errmsg_len, "ok");
  return 0;
}
int nwchemc_embed_quadrupole_cell(const int *n_atoms,
                                  const double *positions_ang,
                                  const int *atomic_numbers,
                                  const double *cell_ang, const int *has_cell,
                                  const int *charge, const int *multiplicity,
                                  double *energy_h, double *quadrupole_au,
                                  char *errmsg, int errmsg_len) {
  (void)cell_ang; (void)has_cell;
  return nwchemc_embed_quadrupole(n_atoms, positions_ang, atomic_numbers,
                                  charge, multiplicity, energy_h,
                                  quadrupole_au, errmsg, errmsg_len);
}
int nwchemc_embed_stress(const int *n_atoms, const double *positions_ang,
                         const int *atomic_numbers, const int *charge,
                         const int *multiplicity, double *energy_h,
                         double *stress_h_bohr3, char *errmsg, int errmsg_len) {
  (void)n_atoms; (void)positions_ang; (void)atomic_numbers; (void)charge;
  (void)multiplicity;
  g_last_energy_h = -1.5;
  *energy_h = g_last_energy_h;
  for (int i = 0; i < 9; ++i)
    stress_h_bohr3[i] = 0.001 * (double)(i + 1);
  snprintf(errmsg, (size_t)errmsg_len, "ok");
  return 0;
}
int nwchemc_embed_stress_cell(const int *n_atoms, const double *positions_ang,
                              const int *atomic_numbers, const double *cell_ang,
                              const int *has_cell, const int *charge,
                              const int *multiplicity, double *energy_h,
                              double *stress_h_bohr3, char *errmsg,
                              int errmsg_len) {
  (void)cell_ang; (void)has_cell;
  return nwchemc_embed_stress(n_atoms, positions_ang, atomic_numbers, charge,
                              multiplicity, energy_h, stress_h_bohr3, errmsg,
                              errmsg_len);
}
int nwchemc_embed_optimize(const int *n_atoms, const double *positions_ang,
                           const int *atomic_numbers, const int *charge,
                           const int *multiplicity, double *energy_h,
                           double *optimized_positions_ang, char *errmsg,
                           int errmsg_len) {
  (void)atomic_numbers; (void)charge; (void)multiplicity;
  int ndof = (*n_atoms) * 3;
  g_last_energy_h = -1.6;
  *energy_h = g_last_energy_h;
  for (int i = 0; i < ndof; ++i)
    optimized_positions_ang[i] = positions_ang[i] + 0.01;
  snprintf(errmsg, (size_t)errmsg_len, "ok");
  return 0;
}
int nwchemc_embed_optimize_cell(const int *n_atoms,
                                const double *positions_ang,
                                const int *atomic_numbers,
                                const double *cell_ang, const int *has_cell,
                                const int *charge, const int *multiplicity,
                                double *energy_h,
                                double *optimized_positions_ang, char *errmsg,
                                int errmsg_len) {
  (void)cell_ang; (void)has_cell;
  return nwchemc_embed_optimize(n_atoms, positions_ang, atomic_numbers, charge,
                                multiplicity, energy_h,
                                optimized_positions_ang, errmsg, errmsg_len);
}
int nwchemc_embed_frequencies(
    const int *n_atoms, const double *positions_ang,
    const int *atomic_numbers, const int *charge, const int *multiplicity,
    double *frequencies_cm1, double *intensities_au, char *errmsg,
    int errmsg_len) {
  (void)positions_ang; (void)atomic_numbers; (void)charge; (void)multiplicity;
  int ndof = (*n_atoms) * 3;
  g_last_energy_h = -1.7;
  for (int i = 0; i < ndof; ++i) {
    frequencies_cm1[i] = 100.0 * (double)(i + 1);
    intensities_au[i] = 0.1 * (double)(i + 1);
  }
  snprintf(errmsg, (size_t)errmsg_len, "ok");
  return 0;
}
int nwchemc_embed_frequencies_cell(
    const int *n_atoms, const double *positions_ang, const int *atomic_numbers,
    const double *cell_ang, const int *has_cell, const int *charge,
    const int *multiplicity, double *frequencies_cm1,
    double *intensities_au, char *errmsg, int errmsg_len) {
  (void)cell_ang; (void)has_cell;
  return nwchemc_embed_frequencies(n_atoms, positions_ang, atomic_numbers,
                                   charge, multiplicity, frequencies_cm1,
                                   intensities_au, errmsg, errmsg_len);
}
int nwchemc_embed_frequencies_modes(
    const int *n_atoms, const double *positions_ang, const int *atomic_numbers,
    const int *charge, const int *multiplicity, double *frequencies_cm1,
    double *intensities_au, double *normal_modes, char *errmsg,
    int errmsg_len) {
  int rc = nwchemc_embed_frequencies(n_atoms, positions_ang, atomic_numbers,
                                     charge, multiplicity, frequencies_cm1,
                                     intensities_au, errmsg, errmsg_len);
  int ndof = (*n_atoms) * 3;
  for (int i = 0; normal_modes && i < ndof * ndof; ++i)
    normal_modes[i] = 0.001 * (double)(i + 1);
  return rc;
}
int nwchemc_embed_frequencies_modes_cell(
    const int *n_atoms, const double *positions_ang, const int *atomic_numbers,
    const double *cell_ang, const int *has_cell, const int *charge,
    const int *multiplicity, double *frequencies_cm1,
    double *intensities_au, double *normal_modes, char *errmsg,
    int errmsg_len) {
  (void)cell_ang; (void)has_cell;
  return nwchemc_embed_frequencies_modes(n_atoms, positions_ang, atomic_numbers,
                                         charge, multiplicity, frequencies_cm1,
                                         intensities_au, normal_modes, errmsg,
                                         errmsg_len);
}
int nwchemc_embed_frequencies_detail_cell(
    const int *n_atoms, const double *positions_ang, const int *atomic_numbers,
    const double *cell_ang, const int *has_cell, const int *charge,
    const int *multiplicity, double *frequencies_cm1,
    double *intensities_au, double *normal_modes,
    double *projected_frequencies_cm1, double *projected_intensities_au,
    double *thermochemistry, char *errmsg, int errmsg_len) {
  int rc = nwchemc_embed_frequencies_modes_cell(
      n_atoms, positions_ang, atomic_numbers, cell_ang, has_cell, charge,
      multiplicity, frequencies_cm1, intensities_au, normal_modes, errmsg,
      errmsg_len);
  int ndof = (*n_atoms) * 3;
  if (thermochemistry) {
    for (int i = 0; i < 5; ++i)
      thermochemistry[i] = 0.001 * (double)(i + 1);
  }
  for (int i = 0; projected_frequencies_cm1 && i < ndof; ++i)
    projected_frequencies_cm1[i] = 90.0 + (double)i;
  for (int i = 0; projected_intensities_au && i < ndof; ++i)
    projected_intensities_au[i] = 0.02 * (double)(i + 1);
  return rc;
}

static unsigned char *read_file(const char *path, size_t *size) {
  FILE *fp = fopen(path, "rb");
  if (!fp)
    return NULL;
  if (fseek(fp, 0, SEEK_END) != 0) {
    fclose(fp);
    return NULL;
  }
  long n = ftell(fp);
  if (n <= 0) {
    fclose(fp);
    return NULL;
  }
  rewind(fp);
  unsigned char *buf = (unsigned char *)malloc((size_t)n);
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

static struct PotentialResult decode_result(const unsigned char *bytes,
                                            size_t nbytes, struct capn *arena) {
  assert_int_equal(capn_init_mem(arena, bytes, (size_t)nbytes, 0), 0);
  PotentialResult_ptr root;
  root.p = capn_getp(capn_root(arena), 0, 1);
  assert_int_equal(root.p.type, CAPN_STRUCT);
  struct PotentialResult view;
  read_PotentialResult(&view, root);
  return view;
}

static int list64_len(capn_list64 list) {
  capn_resolve(&list.p);
  if (list.p.type != CAPN_LIST)
    return -1;
  return list.p.len;
}

static void assert_finite_list(capn_list64 list, int expected_len,
                               const char *field) {
  int len = list64_len(list);
  assert_int_equal(len, expected_len);
  for (int i = 0; i < expected_len; ++i) {
    double v = capn_to_f64(capn_get64(list, i));
    if (!isfinite(v))
      fail_msg("%s[%d] not finite", field, i);
  }
}

static void test_public_calculator_matrix(void **state) {
  (void)state;
  assert_non_null(g_params_path);
  assert_non_null(g_force_step_path);
  size_t params_size = 0;
  size_t step_size = 0;
  unsigned char *params = read_file(g_params_path, &params_size);
  unsigned char *step = read_file(g_force_step_path, &step_size);
  assert_non_null(params);
  assert_non_null(step);

  NWChemCSession *session = nwchemc_session_create(params, params_size);
  assert_non_null(session);

  unsigned char buf[65536];
  size_t n = 0;
  const int natoms = 2;
  const int ndof = natoms * 3;

  /* energy (twice for consistency) */
  NWChemCResult e1 = nwchemc_session_calculate_energy_result(
      session, step, step_size, buf, sizeof(buf), &n);
  assert_int_equal(e1.ok, 1);
  assert_true(isfinite(e1.energy_h));
  {
    struct capn arena;
    struct PotentialResult pr = decode_result(buf, n, &arena);
    assert_true(isfinite(pr.energy));
    printf("calc_matrix energy ok energy_finite=1 forces_len=%d nbytes=%zu\n",
           list64_len(pr.forces), n);
    double e_wire = pr.energy;
    capn_free(&arena);
    n = 0;
    NWChemCResult e2 = nwchemc_session_calculate_energy_result(
        session, step, step_size, buf, sizeof(buf), &n);
    assert_int_equal(e2.ok, 1);
    assert_true(fabs(e1.energy_h - e2.energy_h) < 1e-12);
    struct capn arena2;
    struct PotentialResult pr2 = decode_result(buf, n, &arena2);
    assert_true(fabs(e_wire - pr2.energy) < 1e-12);
    printf("calc_matrix energy_rerun ok delta=0 energy_finite=1\n");
    capn_free(&arena2);
  }

  /* forces (twice) */
  n = 0;
  NWChemCResult f1 = nwchemc_session_calculate_forces_result(
      session, step, step_size, buf, sizeof(buf), &n);
  assert_int_equal(f1.ok, 1);
  {
    struct capn arena;
    struct PotentialResult pr = decode_result(buf, n, &arena);
    assert_finite_list(pr.forces, ndof, "forces");
    printf("calc_matrix forces ok energy_finite=1 forces_len=%d nbytes=%zu\n",
           list64_len(pr.forces), n);
    double f0 = capn_to_f64(capn_get64(pr.forces, 0));
    capn_free(&arena);
    n = 0;
    NWChemCResult f2 = nwchemc_session_calculate_forces_result(
        session, step, step_size, buf, sizeof(buf), &n);
    assert_int_equal(f2.ok, 1);
    assert_true(fabs(f1.energy_h - f2.energy_h) < 1e-12);
    struct capn arena2;
    struct PotentialResult pr2 = decode_result(buf, n, &arena2);
    assert_finite_list(pr2.forces, ndof, "forces");
    assert_true(fabs(f0 - capn_to_f64(capn_get64(pr2.forces, 0))) < 1e-12);
    printf("calc_matrix forces_rerun ok delta=0 forces_len=%d\n",
           list64_len(pr2.forces));
    capn_free(&arena2);
  }

  /* gradient */
  n = 0;
  NWChemCResult g = nwchemc_session_calculate_gradient_result(
      session, step, step_size, buf, sizeof(buf), &n);
  assert_int_equal(g.ok, 1);
  {
    struct capn arena;
    struct PotentialResult pr = decode_result(buf, n, &arena);
    assert_true(isfinite(pr.energy));
    assert_finite_list(pr.gradient, ndof, "gradient");
    printf("calc_matrix gradient ok energy_finite=1 gradient_len=%d nbytes=%zu\n",
           list64_len(pr.gradient), n);
    capn_free(&arena);
  }

  /* hessian square ndof^2 */
  n = 0;
  NWChemCResult h = nwchemc_session_calculate_hessian_result(
      session, step, step_size, buf, sizeof(buf), &n);
  assert_int_equal(h.ok, 1);
  {
    struct capn arena;
    struct PotentialResult pr = decode_result(buf, n, &arena);
    assert_true(isfinite(pr.energy));
    assert_finite_list(pr.hessian, ndof * ndof, "hessian");
    printf("calc_matrix hessian ok energy_finite=1 hessian_len=%d nbytes=%zu\n",
           list64_len(pr.hessian), n);
    capn_free(&arena);
  }

  /* dipole 3 */
  n = 0;
  NWChemCResult d = nwchemc_session_calculate_dipole_result(
      session, step, step_size, buf, sizeof(buf), &n);
  assert_int_equal(d.ok, 1);
  {
    struct capn arena;
    struct PotentialResult pr = decode_result(buf, n, &arena);
    assert_true(isfinite(pr.energy));
    assert_finite_list(pr.dipole, 3, "dipole");
    printf("calc_matrix dipole ok energy_finite=1 dipole_len=%d nbytes=%zu\n",
           list64_len(pr.dipole), n);
    capn_free(&arena);
  }

  /* polarizability non-empty */
  n = 0;
  NWChemCResult p = nwchemc_session_calculate_polarizability_result(
      session, step, step_size, buf, sizeof(buf), &n);
  assert_int_equal(p.ok, 1);
  {
    struct capn arena;
    struct PotentialResult pr = decode_result(buf, n, &arena);
    assert_true(isfinite(pr.energy));
    int plen = list64_len(pr.polarizability);
    assert_true(plen > 0);
    for (int i = 0; i < plen; ++i)
      assert_true(isfinite(capn_to_f64(capn_get64(pr.polarizability, i))));
    printf("calc_matrix polarizability ok energy_finite=1 pol_len=%d nbytes=%zu\n",
           list64_len(pr.polarizability), n);
    capn_free(&arena);
  }

  /* quadrupole 6 */
  n = 0;
  NWChemCResult q = nwchemc_session_calculate_quadrupole_result(
      session, step, step_size, buf, sizeof(buf), &n);
  assert_int_equal(q.ok, 1);
  {
    struct capn arena;
    struct PotentialResult pr = decode_result(buf, n, &arena);
    assert_true(isfinite(pr.energy));
    assert_finite_list(pr.quadrupole, 6, "quadrupole");
    printf("calc_matrix quadrupole ok energy_finite=1 quad_len=%d nbytes=%zu\n",
           list64_len(pr.quadrupole), n);
    capn_free(&arena);
  }

  /* stress 9 */
  n = 0;
  NWChemCResult s = nwchemc_session_calculate_stress_result(
      session, step, step_size, buf, sizeof(buf), &n);
  assert_int_equal(s.ok, 1);
  {
    struct capn arena;
    struct PotentialResult pr = decode_result(buf, n, &arena);
    assert_true(isfinite(pr.energy));
    assert_finite_list(pr.stress, 9, "stress");
    printf("calc_matrix stress ok energy_finite=1 stress_len=%d nbytes=%zu\n",
           list64_len(pr.stress), n);
    capn_free(&arena);
  }

  /* optimize natoms*3 */
  n = 0;
  NWChemCResult o = nwchemc_session_calculate_optimize_result(
      session, step, step_size, buf, sizeof(buf), &n);
  assert_int_equal(o.ok, 1);
  {
    struct capn arena;
    struct PotentialResult pr = decode_result(buf, n, &arena);
    assert_true(isfinite(pr.energy));
    assert_finite_list(pr.optimizedPos, ndof, "optimizedPos");
    printf("calc_matrix optimize ok energy_finite=1 optimized_len=%d nbytes=%zu\n",
           list64_len(pr.optimizedPos), n);
    capn_free(&arena);
  }

  /* frequencies + intensities modes match */
  n = 0;
  NWChemCResult fr = nwchemc_session_calculate_frequencies_result(
      session, step, step_size, buf, sizeof(buf), &n);
  assert_int_equal(fr.ok, 1);
  {
    struct capn arena;
    struct PotentialResult pr = decode_result(buf, n, &arena);
    assert_true(isfinite(pr.energy));
    int flen = list64_len(pr.frequencies);
    int ilen = list64_len(pr.intensities);
    assert_true(flen > 0);
    assert_int_equal(flen, ilen);
    assert_int_equal(flen, ndof);
    for (int i = 0; i < flen; ++i) {
      assert_true(isfinite(capn_to_f64(capn_get64(pr.frequencies, i))));
      assert_true(isfinite(capn_to_f64(capn_get64(pr.intensities, i))));
    }
    printf("calc_matrix frequencies ok energy_finite=1 freq_len=%d "
           "inten_len=%d nbytes=%zu\n",
           list64_len(pr.frequencies), list64_len(pr.intensities), n);
    capn_free(&arena);
  }

  /* one-shot energy/forces for dual path */
  n = 0;
  NWChemCResult os = nwchemc_calculate_energy_result(
      params, params_size, step, step_size, buf, sizeof(buf), &n);
  assert_int_equal(os.ok, 1);
  {
    struct capn arena;
    struct PotentialResult pr = decode_result(buf, n, &arena);
    assert_true(isfinite(pr.energy));
    printf("calc_matrix oneshot_energy ok energy_finite=1 nbytes=%zu\n", n);
    capn_free(&arena);
  }
  n = 0;
  NWChemCResult osf = nwchemc_calculate_forces_result(
      params, params_size, step, step_size, buf, sizeof(buf), &n);
  assert_int_equal(osf.ok, 1);
  {
    struct capn arena;
    struct PotentialResult pr = decode_result(buf, n, &arena);
    assert_finite_list(pr.forces, ndof, "oneshot_forces");
    printf("calc_matrix oneshot_forces ok forces_len=%d nbytes=%zu\n",
           list64_len(pr.forces), n);
    capn_free(&arena);
  }

  nwchemc_session_destroy(session);
  free(step);
  free(params);
}

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s PARAMS_BIN FORCE_INPUT_BIN\n", argv[0]);
    return 2;
  }
  g_params_path = argv[1];
  g_force_step_path = argv[2];
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_public_calculator_matrix),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
