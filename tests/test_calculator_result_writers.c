#include "nwchemc_params.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <setjmp.h>
#include <cmocka.h>

#include "schema/Potentials.capnp.h"
#include <capn.h>

static void test_energy_forces_payload(void **state) {
  (void)state;
  double forces[6] = {0.1, -0.2, 0.3, -0.4, 0.5, -0.6};
  unsigned char buf[8192];
  size_t n = 0;
  assert_int_equal(
      nwchemc_potential_result_write(-1.5, forces, 6, buf, sizeof(buf), &n), 0);
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, buf, (int)n, 0), 0);
  PotentialResult_ptr pr;
  pr.p = capn_getp(capn_root(&arena), 0, 1);
  struct PotentialResult view;
  read_PotentialResult(&view, pr);
  assert_true(fabs(view.energy + 1.5) < 1e-12);
  assert_int_equal(capn_len(view.forces), 6);
  printf("calc_matrix energy_forces ok\n");
  capn_free(&arena);
}

static void test_dipole_payload(void **state) {
  (void)state;
  double dip[3] = {0.01, 0.02, -0.03};
  unsigned char buf[8192];
  size_t n = 0;
  assert_int_equal(
      nwchemc_potential_result_write_dipole(-2.0, dip, buf, sizeof(buf), &n), 0);
  struct capn arena;
  assert_int_equal(capn_init_mem(&arena, buf, (int)n, 0), 0);
  PotentialResult_ptr pr;
  pr.p = capn_getp(capn_root(&arena), 0, 1);
  struct PotentialResult view;
  read_PotentialResult(&view, pr);
  assert_int_equal(capn_len(view.dipole), 3);
  printf("calc_matrix dipole ok\n");
  capn_free(&arena);
}

static void test_hessian_gradient_payload(void **state) {
  (void)state;
  double H[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
  double g[6] = {1, 2, 3, 4, 5, 6};
  unsigned char buf[16384];
  size_t n = 0;
  assert_int_equal(
      nwchemc_potential_result_write_hessian(-3.0, H, 9, buf, sizeof(buf), &n),
      0);
  assert_int_equal(nwchemc_potential_result_write_gradient(-4.0, g, 6, buf,
                                                           sizeof(buf), &n),
                   0);
  printf("calc_matrix hessian_gradient ok\n");
}

static void test_remaining_payloads(void **state) {
  (void)state;
  unsigned char buf[65536];
  size_t n = 0;
  double pol[12] = {0};
  double quad[6] = {1, 2, 3, 4, 5, 6};
  double stress[9] = {0};
  double pos[6] = {0};
  double freq[3] = {100, 200, 300};
  double inten[3] = {0.1, 0.2, 0.3};
  double modes[9] = {0};
  double thermo[4] = {0};
  double pfreq[3] = {0};
  double pinten[3] = {0};
  assert_int_equal(nwchemc_potential_result_write_polarizability(
                       -5.0, pol, buf, sizeof(buf), &n),
                   0);
  printf("calc_matrix polarizability ok n=%zu\n", n);
  assert_int_equal(
      nwchemc_potential_result_write_quadrupole(-5.1, quad, buf, sizeof(buf), &n),
      0);
  printf("calc_matrix quadrupole ok\n");
  assert_int_equal(
      nwchemc_potential_result_write_stress(-5.2, stress, buf, sizeof(buf), &n),
      0);
  printf("calc_matrix stress ok\n");
  assert_int_equal(nwchemc_potential_result_write_optimized(
                       -5.3, pos, 6, buf, sizeof(buf), &n),
                   0);
  printf("calc_matrix optimized ok\n");
  assert_int_equal(nwchemc_potential_result_write_frequencies(
                       -5.4, freq, inten, modes, thermo, pfreq, pinten, 3, buf,
                       sizeof(buf), &n),
                   0);
  printf("calc_matrix frequencies ok n=%zu\n", n);
}

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_energy_forces_payload),
      cmocka_unit_test(test_dipole_payload),
      cmocka_unit_test(test_hessian_gradient_payload),
      cmocka_unit_test(test_remaining_payloads),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
