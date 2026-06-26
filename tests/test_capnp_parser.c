#include "nwchemc_params.h"

#include <errno.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cmocka.h>

static const char *g_params_path = NULL;

static unsigned char *read_file(const char *path, size_t *size) {
  FILE *fp = fopen(path, "rb");
  if (!fp) {
    fprintf(stderr, "open failed for %s: %s\n", path, strerror(errno));
    return NULL;
  }
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

static int text_equals(capn_text text, const char *expected) {
  size_t n = strlen(expected);
  return text.len == (int)n && text.str && memcmp(text.str, expected, n) == 0;
}

static void test_parser_renders_structured_input(void **state) {
  (void)state;

  size_t message_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  struct NWChemParams params;
  read_NWChemParams(&params, params_root);
  char input_blocks[NWCHEMC_BLOCKS];
  assert_true(text_equals(params.basis, "def2-svp"));
  assert_true(text_equals(params.theory, "dft"));
  assert_true(text_equals(params.scfType, "pbe0"));
  assert_int_equal(params.charge, -1);
  assert_int_equal(params.multiplicity, 3);
  assert_int_equal(params.memoryMb, 1024);
  assert_true(text_equals(params.nwchemRoot, "/opt/nwchem"));
  assert_true(text_equals(params.scratchDir, "/scratch/nw"));
  assert_true(text_equals(params.permanentDir, "/perm/nw"));
  assert_int_equal(nwchemc_params_render_input_blocks(
                       params_root, input_blocks, sizeof(input_blocks)),
                   0);
  assert_non_null(strstr(input_blocks, "smear 0.001 fixsz"));
  assert_non_null(strstr(input_blocks, "xc pbe0"));
  assert_non_null(strstr(input_blocks, "iterations 40"));
  assert_non_null(strstr(input_blocks, "driver"));
  assert_non_null(strstr(input_blocks, "maxiter 20"));
  assert_non_null(strstr(input_blocks, "scf"));
  assert_non_null(strstr(input_blocks, "thresh 1e-6"));
  assert_non_null(strstr(input_blocks, "tol2e 1e-9"));
  assert_non_null(strstr(input_blocks, "acc_std"));
  assert_non_null(strstr(input_blocks, "pseudopotentials"));
  assert_non_null(strstr(input_blocks, "Si library sg15"));
  assert_non_null(strstr(input_blocks, "H pspw_library hgh_lda"));
  assert_non_null(strstr(input_blocks, "O paw_library paw_default"));
  assert_non_null(strstr(input_blocks, "C cpi C.cpi"));
  assert_non_null(strstr(input_blocks, "N teter N.teter"));
  assert_non_null(strstr(input_blocks, "* pspw_library pspw_default"));
  assert_non_null(strstr(input_blocks, "pspspin off"));
  assert_non_null(strstr(input_blocks, "energy_cutoff 12.5"));
  assert_non_null(strstr(input_blocks, "wavefunction_cutoff 6.25"));
  assert_non_null(strstr(input_blocks, "ewald_rcut 3.5"));
  assert_non_null(strstr(input_blocks, "ewald_ncut 9"));
  assert_non_null(strstr(input_blocks, "cell_name cellA"));
  assert_non_null(strstr(input_blocks, "input_wavefunction_filename psi.in"));
  assert_non_null(strstr(input_blocks, "output_wavefunction_filename psi.out"));
  assert_non_null(strstr(input_blocks, "fake_mass 2.5"));
  assert_non_null(strstr(input_blocks, "time_step 4.5"));
  assert_non_null(strstr(input_blocks, "loop 3 7"));
  assert_non_null(strstr(input_blocks, "tolerances 0.125 0.25 0.5"));
  assert_non_null(strstr(input_blocks, "exchange_correlation pbe96"));

  nwchemc_params_release(&arena);
  free(message);
}

static void test_parser_extracts_direct_dft_options(void **state) {
  (void)state;

  size_t message_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  capn_text xc = {0};
  int direct_enabled = 0;
  int smearing_enabled = 0;
  double smear_sigma_hartree = 0.0;
  int smearing_spinset = 0;
  assert_int_equal(nwchemc_params_extract_direct_dft(
                       params_root, &xc, &direct_enabled, &smearing_enabled,
                       &smear_sigma_hartree, &smearing_spinset),
                   0);
  assert_true(text_equals(xc, "pbe0"));
  assert_int_equal(direct_enabled, 1);
  assert_int_equal(smearing_enabled, 1);
  assert_true(smear_sigma_hartree > 0.000999);
  assert_true(smear_sigma_hartree < 0.001001);
  assert_int_equal(smearing_spinset, 1);

  char input_blocks[NWCHEMC_BLOCKS];
  assert_int_equal(nwchemc_params_render_embed_input_blocks(
                       params_root, input_blocks, sizeof(input_blocks)),
                   0);
  assert_null(strstr(input_blocks, "smear 0.001"));
  assert_null(strstr(input_blocks, "xc pbe0"));
  assert_null(strstr(input_blocks, "  direct"));
  assert_null(strstr(input_blocks, "pseudopotentials"));
  assert_null(strstr(input_blocks, "Si library sg15"));
  assert_null(strstr(input_blocks, "H pspw_library hgh_lda"));
  assert_null(strstr(input_blocks, "O paw_library paw_default"));
  assert_null(strstr(input_blocks, "C cpi C.cpi"));
  assert_null(strstr(input_blocks, "N teter N.teter"));
  assert_null(strstr(input_blocks, "* pspw_library pspw_default"));
  assert_null(strstr(input_blocks, "energy_cutoff 12.5"));
  assert_null(strstr(input_blocks, "wavefunction_cutoff 6.25"));
  assert_null(strstr(input_blocks, "ewald_rcut 3.5"));
  assert_null(strstr(input_blocks, "ewald_ncut 9"));
  assert_null(strstr(input_blocks, "cell_name cellA"));
  assert_null(strstr(input_blocks, "input_wavefunction_filename psi.in"));
  assert_null(strstr(input_blocks, "output_wavefunction_filename psi.out"));
  assert_null(strstr(input_blocks, "fake_mass 2.5"));
  assert_null(strstr(input_blocks, "time_step 4.5"));
  assert_null(strstr(input_blocks, "loop 3 7"));
  assert_null(strstr(input_blocks, "tolerances 0.125 0.25 0.5"));
  assert_null(strstr(input_blocks, "exchange_correlation pbe96"));
  assert_non_null(strstr(input_blocks, "nwpw"));
  assert_non_null(strstr(input_blocks, "pspspin off"));
  assert_non_null(strstr(input_blocks, "dft"));
  assert_non_null(strstr(input_blocks, "iterations 40"));
  assert_non_null(strstr(input_blocks, "set int:acc_std 1e-8"));

  nwchemc_params_release(&arena);
  free(message);
}

static void test_parser_extracts_direct_nwpw_options(void **state) {
  (void)state;

  size_t message_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  int has_options = 0;
  double energy_cutoff = 0.0;
  double wavefunction_cutoff = 0.0;
  double ewald_rcut = 0.0;
  int ewald_ncut = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw(
                       params_root, &has_options, &energy_cutoff,
                       &wavefunction_cutoff, &ewald_rcut, &ewald_ncut),
                   0);
  assert_int_equal(has_options, 1);
  assert_true(energy_cutoff > 12.499);
  assert_true(energy_cutoff < 12.501);
  assert_true(wavefunction_cutoff > 6.249);
  assert_true(wavefunction_cutoff < 6.251);
  assert_true(ewald_rcut > 3.499);
  assert_true(ewald_rcut < 3.501);
  assert_int_equal(ewald_ncut, 9);

  int has_state = 0;
  capn_text cell_name = {0};
  capn_text input_wavefunction_filename = {0};
  capn_text output_wavefunction_filename = {0};
  double fake_mass = 0.0;
  double time_step = 0.0;
  int loop_start = 0;
  int loop_end = 0;
  int has_tolerances = 0;
  double tolerance_energy = 0.0;
  double tolerance_density = 0.0;
  double tolerance_gradient = 0.0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_state(
                       params_root, &has_state, &cell_name,
                       &input_wavefunction_filename,
                       &output_wavefunction_filename, &fake_mass, &time_step,
                       &loop_start, &loop_end, &has_tolerances,
                       &tolerance_energy, &tolerance_density,
                       &tolerance_gradient),
                   0);
  assert_int_equal(has_state, 1);
  assert_true(text_equals(cell_name, "cellA"));
  assert_true(text_equals(input_wavefunction_filename, "psi.in"));
  assert_true(text_equals(output_wavefunction_filename, "psi.out"));
  assert_true(fake_mass > 2.499);
  assert_true(fake_mass < 2.501);
  assert_true(time_step > 4.499);
  assert_true(time_step < 4.501);
  assert_int_equal(loop_start, 3);
  assert_int_equal(loop_end, 7);
  assert_int_equal(has_tolerances, 1);
  assert_true(tolerance_energy > 0.1249);
  assert_true(tolerance_energy < 0.1251);
  assert_true(tolerance_density > 0.2499);
  assert_true(tolerance_density < 0.2501);
  assert_true(tolerance_gradient > 0.4999);
  assert_true(tolerance_gradient < 0.5001);

  int has_xc = 0;
  capn_text exchange_correlation = {0};
  assert_int_equal(nwchemc_params_extract_direct_nwpw_xc(
                       params_root, &has_xc, &exchange_correlation),
                   0);
  assert_int_equal(has_xc, 1);
  assert_true(text_equals(exchange_correlation, "pbe96"));

  nwchemc_params_release(&arena);
  free(message);
}

static void test_parser_extracts_direct_pseudopotentials(void **state) {
  (void)state;

  size_t message_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  capn_text elements[8];
  capn_text names[8];
  int types[8];
  size_t count = 0;
  assert_int_equal(nwchemc_params_extract_direct_pseudopotentials(
                       params_root, elements, types, names, 8, &count),
                   0);
  assert_int_equal((int)count, 6);
  assert_true(text_equals(elements[0], "Si"));
  assert_int_equal(types[0], NWChemPseudopotentialEntry_LibraryType_library);
  assert_true(text_equals(names[0], "sg15"));
  assert_true(text_equals(elements[1], "H"));
  assert_int_equal(types[1],
                   NWChemPseudopotentialEntry_LibraryType_pspwLibrary);
  assert_true(text_equals(names[1], "hgh_lda"));
  assert_true(text_equals(elements[2], "O"));
  assert_int_equal(types[2],
                   NWChemPseudopotentialEntry_LibraryType_pawLibrary);
  assert_true(text_equals(names[2], "paw_default"));
  assert_true(text_equals(elements[3], "C"));
  assert_int_equal(types[3], NWChemPseudopotentialEntry_LibraryType_cpi);
  assert_true(text_equals(names[3], "C.cpi"));
  assert_true(text_equals(elements[4], "N"));
  assert_int_equal(types[4], NWChemPseudopotentialEntry_LibraryType_teter);
  assert_true(text_equals(names[4], "N.teter"));
  assert_true(text_equals(elements[5], "*"));
  assert_int_equal(types[5],
                   NWChemPseudopotentialEntry_LibraryType_pspwLibrary);
  assert_true(text_equals(names[5], "pspw_default"));

  assert_int_equal(nwchemc_params_extract_direct_pseudopotentials(
                       params_root, elements, types, names, 4, &count),
                   -1);

  nwchemc_params_release(&arena);
  free(message);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s PARAMS_BIN\n", argv[0]);
    return 2;
  }
  g_params_path = argv[1];
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_parser_renders_structured_input),
      cmocka_unit_test(test_parser_extracts_direct_dft_options),
      cmocka_unit_test(test_parser_extracts_direct_nwpw_options),
      cmocka_unit_test(test_parser_extracts_direct_pseudopotentials),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
