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
  assert_non_null(
      strstr(input_blocks, "set nwpw:psp:semicore_small logical true"));
  assert_non_null(strstr(input_blocks, "pspspin up p 1.25 1 3"));
  assert_non_null(strstr(input_blocks, "pspspin not_m 0 down d 0.75 2"));
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
  assert_non_null(strstr(input_blocks, "nobalance"));
  assert_non_null(strstr(input_blocks, "bo_steps 11 22"));
  assert_non_null(strstr(input_blocks, "bo_time_step 0.75"));
  assert_non_null(strstr(input_blocks, "bo_algorithm velocity-verlet"));
  assert_non_null(strstr(input_blocks, "bo_fake_mass 333"));
  assert_non_null(strstr(input_blocks, "scaling 1.5 2.5"));
  assert_non_null(strstr(input_blocks, "np_dimensions 2 3 4"));
  assert_non_null(strstr(input_blocks, "spin_orbit off"));
  assert_non_null(strstr(input_blocks, "parallel_io on"));
  assert_non_null(strstr(input_blocks, "xyz_filename traj.xyz"));
  assert_non_null(strstr(input_blocks, "ion_motion_filename ion.mov"));
  assert_non_null(strstr(input_blocks, "emotion_filename electron.mov"));
  assert_non_null(strstr(input_blocks, "hmotion_filename h.mov"));
  assert_non_null(strstr(input_blocks, "omotion_filename orb.mov"));
  assert_non_null(strstr(input_blocks, "eigmotion_filename eig.mov"));
  assert_non_null(strstr(input_blocks, "fractional_orbitals 5 6"));
  assert_non_null(strstr(input_blocks,
                         "smear temperature 0.02 alpha 0.7 fermi orbitals 5 6"));
  assert_non_null(strstr(input_blocks, "virtual_orbitals 7 8"));
  assert_non_null(strstr(input_blocks, "lcao_skip"));
  assert_non_null(strstr(input_blocks, "ewald_ngrid 9 10 11"));
  assert_non_null(strstr(input_blocks, "Nose-Hoover 12 300 34 400 start 2 3"));
  assert_non_null(strstr(input_blocks, "atom_efield"));
  assert_non_null(strstr(input_blocks, "atom_efield_grad"));
  assert_non_null(strstr(input_blocks, "mulliken kawai"));
  assert_non_null(strstr(input_blocks, "periodic_dipole true"));
  assert_non_null(strstr(input_blocks, "monkhorst-pack 3 4 -5 zoneA"));
  assert_non_null(strstr(input_blocks, "zone_name zoneA"));
  assert_non_null(strstr(input_blocks, "max_kpoints_print 12"));
  assert_non_null(strstr(input_blocks, "simulation_cell"));
  assert_non_null(strstr(input_blocks, "boundary_conditions periodic"));
  assert_non_null(strstr(input_blocks, "lattice_vectors"));
  assert_non_null(strstr(input_blocks, "1 0 0"));
  assert_non_null(strstr(input_blocks, "0 2 0"));
  assert_non_null(strstr(input_blocks, "0 0 3"));
  assert_non_null(strstr(input_blocks, "ngrid 20 22 24"));
  assert_non_null(strstr(input_blocks, "ngrid_small 10 11 12"));
  assert_non_null(strstr(input_blocks, "box_delta 1"));
  assert_non_null(strstr(input_blocks, "box_orient"));
  assert_non_null(strstr(input_blocks, "box_different_lengths"));
  assert_non_null(strstr(input_blocks, "ccsd\n  maxiter 20"));
  assert_non_null(strstr(input_blocks, "thresh 1e-07"));
  assert_non_null(strstr(input_blocks, "tol2e 1e-09"));
  assert_non_null(strstr(input_blocks, "iprt 2"));
  assert_non_null(strstr(input_blocks, "diisbas 6"));
  assert_non_null(strstr(input_blocks, "freeze 1 virtual 2"));
  assert_non_null(strstr(input_blocks, "nodisk"));
  assert_non_null(strstr(input_blocks, "fss 1.2"));
  assert_non_null(strstr(input_blocks, "fos 0.8"));
  assert_non_null(strstr(input_blocks, "print high reference"));
  assert_non_null(strstr(input_blocks, "noprint byproduct energies"));
  assert_non_null(strstr(input_blocks, "doa 1 0 1"));
  assert_non_null(strstr(input_blocks, "dob 2 0"));
  assert_non_null(strstr(input_blocks, "dog 3"));
  assert_non_null(strstr(input_blocks, "doh 4 5"));
  assert_non_null(strstr(input_blocks, "dojk 6"));
  assert_non_null(strstr(input_blocks, "dos 7 8 9"));
  assert_non_null(strstr(input_blocks, "dod 10"));
  assert_non_null(
      strstr(input_blocks, "set ccsd:use_trpdrv_nb logical true"));
  assert_non_null(strstr(input_blocks, "set ccsd:use_ccsd_omp logical true"));
  assert_non_null(
      strstr(input_blocks, "set ccsd:use_trpdrv_omp logical false"));
  assert_non_null(
      strstr(input_blocks, "set ccsd:use_trpdrv_offload logical true"));
  assert_non_null(strstr(input_blocks, "tce\n  dft"));
  assert_non_null(strstr(input_blocks, "freeze 1 virtual 2"));
  assert_non_null(strstr(input_blocks, "2eorb"));
  assert_non_null(strstr(input_blocks, "cr-ccsd(t)"));
  assert_non_null(strstr(input_blocks, "thresh 1e-08"));
  assert_non_null(strstr(input_blocks, "lshift 0.01"));
  assert_non_null(strstr(input_blocks, "lshiftl 0.02"));
  assert_non_null(strstr(input_blocks, "lshift2 0.03 0.04"));
  assert_non_null(strstr(input_blocks, "lshift3 0.05 0.06"));
  assert_non_null(strstr(input_blocks, "io ga_eaf"));
  assert_non_null(strstr(input_blocks, "eomsol 2"));
  assert_non_null(strstr(input_blocks, "hbard 600"));
  assert_non_null(strstr(input_blocks, "nroots 4"));
  assert_non_null(strstr(input_blocks, "targetsym b2"));
  assert_non_null(strstr(input_blocks, "densmat dens.dat"));
  assert_non_null(strstr(input_blocks, "multipole 3"));
  assert_non_null(strstr(input_blocks, "nofock"));
  assert_non_null(strstr(input_blocks, "active_oa 5"));
  assert_non_null(strstr(input_blocks, "active_vb 7"));
  assert_non_null(strstr(input_blocks, "oact 8"));
  assert_non_null(strstr(input_blocks, "uact 9"));
  assert_non_null(strstr(input_blocks, "emin_act 0.1"));
  assert_non_null(strstr(input_blocks, "emax_act 0.9"));
  assert_non_null(strstr(input_blocks, "t3a_lvl 2"));
  assert_non_null(strstr(input_blocks, "maxdiff 0.25"));
  assert_non_null(strstr(input_blocks, "attilesize 44"));
  assert_non_null(strstr(input_blocks, "split 2"));
  assert_non_null(strstr(input_blocks, "2emet 3"));
  assert_non_null(strstr(input_blocks, "idiskx 1"));
  assert_non_null(strstr(input_blocks, "tilesize 40"));
  assert_non_null(strstr(input_blocks, "cuda 1"));
  assert_non_null(strstr(input_blocks, "tcc_spaces"));
  assert_non_null(strstr(input_blocks, "print debug tile time"));
  assert_non_null(strstr(input_blocks, "mrccdata\n  se4t"));
  assert_non_null(strstr(input_blocks, "  no_aposteriori\n"));
  assert_non_null(strstr(input_blocks, "  root 1\n"));
  assert_non_null(strstr(input_blocks, "  cas 2 2\n"));
  assert_non_null(strstr(input_blocks, "  nref 4\n"));
  assert_non_null(strstr(input_blocks, "  222220\n"));
  assert_non_null(strstr(input_blocks, "  2222ba\n"));
  assert_non_null(strstr(input_blocks, "  subgroupsize 2\n"));
  assert_non_null(strstr(input_blocks, "  improvetiling\n"));
  assert_non_null(strstr(input_blocks, "  usspt\n"));
  assert_non_null(
      strstr(input_blocks, "tce\n  freeze core atomic\n  dipole\nend"));
  assert_non_null(strstr(input_blocks, "print debug tile time"));

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
  assert_null(strstr(input_blocks, "nobalance"));
  assert_null(strstr(input_blocks, "bo_steps 11 22"));
  assert_null(strstr(input_blocks, "bo_time_step 0.75"));
  assert_null(strstr(input_blocks, "bo_algorithm velocity-verlet"));
  assert_null(strstr(input_blocks, "bo_fake_mass 333"));
  assert_null(strstr(input_blocks, "scaling 1.5 2.5"));
  assert_null(strstr(input_blocks, "np_dimensions 2 3 4"));
  assert_null(strstr(input_blocks, "spin_orbit off"));
  assert_null(strstr(input_blocks, "parallel_io on"));
  assert_null(strstr(input_blocks, "xyz_filename traj.xyz"));
  assert_null(strstr(input_blocks, "ion_motion_filename ion.mov"));
  assert_null(strstr(input_blocks, "emotion_filename electron.mov"));
  assert_null(strstr(input_blocks, "hmotion_filename h.mov"));
  assert_null(strstr(input_blocks, "omotion_filename orb.mov"));
  assert_null(strstr(input_blocks, "eigmotion_filename eig.mov"));
  assert_null(strstr(input_blocks, "fractional_orbitals 5 6"));
  assert_null(strstr(input_blocks,
                     "smear temperature 0.02 alpha 0.7 fermi orbitals 5 6"));
  assert_null(strstr(input_blocks, "virtual_orbitals 7 8"));
  assert_null(strstr(input_blocks, "lcao_skip"));
  assert_null(strstr(input_blocks, "ewald_ngrid 9 10 11"));
  assert_null(strstr(input_blocks, "Nose-Hoover 12 300 34 400 start 2 3"));
  assert_null(strstr(input_blocks, "monkhorst-pack 3 4 -5 zoneA"));
  assert_null(strstr(input_blocks, "zone_name zoneA"));
  assert_null(strstr(input_blocks, "max_kpoints_print 12"));
  assert_null(strstr(input_blocks, "simulation_cell"));
  assert_null(strstr(input_blocks, "boundary_conditions periodic"));
  assert_null(strstr(input_blocks, "lattice_vectors"));
  assert_null(strstr(input_blocks, "ngrid 20 22 24"));
  assert_null(strstr(input_blocks, "ngrid_small 10 11 12"));
  assert_null(strstr(input_blocks, "box_delta 1"));
  assert_null(strstr(input_blocks, "box_orient"));
  assert_null(strstr(input_blocks, "box_different_lengths"));
  assert_null(strstr(input_blocks, "ccsd\n  maxiter 20"));
  assert_null(strstr(input_blocks, "freeze 1 virtual 2"));
  assert_null(strstr(input_blocks, "nodisk"));
  assert_non_null(
      strstr(input_blocks,
             "ccsd\n"
             "  print high reference\n"
             "  noprint byproduct energies\n"
             "  doa 1 0 1\n"
             "  dob 2 0\n"
             "  dog 3\n"
             "  doh 4 5\n"
             "  dojk 6\n"
             "  dos 7 8 9\n"
             "  dod 10\n"
             "end"));
  assert_null(strstr(input_blocks, "set ccsd:use_trpdrv_nb logical true"));
  assert_null(strstr(input_blocks, "set ccsd:use_ccsd_omp logical true"));
  assert_null(strstr(input_blocks, "set ccsd:use_trpdrv_omp logical false"));
  assert_null(strstr(input_blocks, "set ccsd:use_trpdrv_offload logical true"));
  assert_null(strstr(input_blocks, "tce\n  dft"));
  assert_null(strstr(input_blocks, "cr-ccsd(t)"));
  assert_null(strstr(input_blocks, "lshift 0.01"));
  assert_null(strstr(input_blocks, "lshift2 0.03 0.04"));
  assert_null(strstr(input_blocks, "io ga_eaf"));
  assert_null(strstr(input_blocks, "densmat dens.dat"));
  assert_null(strstr(input_blocks, "nofock"));
  assert_null(strstr(input_blocks, "active_oa 5"));
  assert_null(strstr(input_blocks, "tcc_spaces"));
  assert_non_null(strstr(input_blocks, "mrccdata\n  se4t"));
  assert_non_null(strstr(input_blocks, "  cas 2 2\n"));
  assert_non_null(strstr(input_blocks, "  2222ba\n"));
  assert_non_null(strstr(input_blocks, "tce\n  freeze core atomic\nend"));
  assert_null(strstr(input_blocks, "dipole"));
  assert_null(strstr(input_blocks, "nwpw"));
  assert_null(strstr(input_blocks, "atom_efield"));
  assert_null(strstr(input_blocks, "atom_efield_grad"));
  assert_null(strstr(input_blocks, "mulliken kawai"));
  assert_null(strstr(input_blocks, "nwpw:mulliken"));
  assert_null(strstr(input_blocks, "periodic_dipole"));
  assert_null(strstr(input_blocks, "nwpw:periodic_dipole"));
  assert_null(strstr(input_blocks, "pspspin off"));
  assert_null(strstr(input_blocks, "nwpw:psp:semicore_small"));
  assert_non_null(strstr(input_blocks, "print debug"));
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

  int has_bo = 0;
  int balance_mode = 0;
  int bo_step_start = 0;
  int bo_step_end = 0;
  double bo_time_step = 0.0;
  int bo_algorithm = 0;
  double bo_fake_mass = 0.0;
  int has_scaling = 0;
  double scaling_first = 0.0;
  double scaling_second = 0.0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_bo(
                       params_root, &has_bo, &balance_mode, &bo_step_start,
                       &bo_step_end, &bo_time_step, &bo_algorithm,
                       &bo_fake_mass, &has_scaling, &scaling_first,
                       &scaling_second),
                   0);
  assert_int_equal(has_bo, 1);
  assert_int_equal(balance_mode, NWChemNwpwBalanceMode_nobalance);
  assert_int_equal(bo_step_start, 11);
  assert_int_equal(bo_step_end, 22);
  assert_true(bo_time_step > 0.749);
  assert_true(bo_time_step < 0.751);
  assert_int_equal(bo_algorithm, NWChemNwpwBoAlgorithm_velocityVerlet);
  assert_true(bo_fake_mass > 332.999);
  assert_true(bo_fake_mass < 333.001);
  assert_int_equal(has_scaling, 1);
  assert_true(scaling_first > 1.499);
  assert_true(scaling_first < 1.501);
  assert_true(scaling_second > 2.499);
  assert_true(scaling_second < 2.501);

  int has_execution = 0;
  int np_fft = 0;
  int np_orbital = 0;
  int np_kspace = 0;
  int spin_orbit = 0;
  int parallel_io = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_execution(
                       params_root, &has_execution, &np_fft, &np_orbital,
                       &np_kspace, &spin_orbit, &parallel_io),
                   0);
  assert_int_equal(has_execution, 1);
  assert_int_equal(np_fft, 2);
  assert_int_equal(np_orbital, 3);
  assert_int_equal(np_kspace, 4);
  assert_int_equal(spin_orbit, NWChemNwpwToggle_disabled);
  assert_int_equal(parallel_io, NWChemNwpwToggle_enabled);

  int has_filenames = 0;
  capn_text xyz_filename = {0};
  capn_text ion_motion_filename = {0};
  capn_text electron_motion_filename = {0};
  capn_text hamiltonian_motion_filename = {0};
  capn_text orbital_motion_filename = {0};
  capn_text eigenvalue_motion_filename = {0};
  assert_int_equal(nwchemc_params_extract_direct_nwpw_filenames(
                       params_root, &has_filenames, &xyz_filename,
                       &ion_motion_filename, &electron_motion_filename,
                       &hamiltonian_motion_filename, &orbital_motion_filename,
                       &eigenvalue_motion_filename),
                   0);
  assert_int_equal(has_filenames, 1);
  assert_true(text_equals(xyz_filename, "traj.xyz"));
  assert_true(text_equals(ion_motion_filename, "ion.mov"));
  assert_true(text_equals(electron_motion_filename, "electron.mov"));
  assert_true(text_equals(hamiltonian_motion_filename, "h.mov"));
  assert_true(text_equals(orbital_motion_filename, "orb.mov"));
  assert_true(text_equals(eigenvalue_motion_filename, "eig.mov"));

  int has_fractional = 0;
  int fractional_orbitals_start = 0;
  int fractional_orbitals_end = 0;
  int has_smear = 0;
  double smear_temperature = 0.0;
  double smear_alpha = 0.0;
  int smear_type = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_fractional(
                       params_root, &has_fractional,
                       &fractional_orbitals_start, &fractional_orbitals_end,
                       &has_smear, &smear_temperature, &smear_alpha,
                       &smear_type),
                   0);
  assert_int_equal(has_fractional, 1);
  assert_int_equal(fractional_orbitals_start, 5);
  assert_int_equal(fractional_orbitals_end, 6);
  assert_int_equal(has_smear, 1);
  assert_true(smear_temperature > 0.0199);
  assert_true(smear_temperature < 0.0201);
  assert_true(smear_alpha > 0.699);
  assert_true(smear_alpha < 0.701);
  assert_int_equal(smear_type, NWChemNwpwSmearType_fermi);

  int has_orbital_grid = 0;
  int virtual_orbitals_start = 0;
  int virtual_orbitals_end = 0;
  int lcao_mode = 0;
  int ewald_grid_x = 0;
  int ewald_grid_y = 0;
  int ewald_grid_z = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_orbital_grid(
                       params_root, &has_orbital_grid,
                       &virtual_orbitals_start, &virtual_orbitals_end,
                       &lcao_mode, &ewald_grid_x, &ewald_grid_y,
                       &ewald_grid_z),
                   0);
  assert_int_equal(has_orbital_grid, 1);
  assert_int_equal(virtual_orbitals_start, 7);
  assert_int_equal(virtual_orbitals_end, 8);
  assert_int_equal(lcao_mode, NWChemNwpwLcaoMode_skip);
  assert_int_equal(ewald_grid_x, 9);
  assert_int_equal(ewald_grid_y, 10);
  assert_int_equal(ewald_grid_z, 11);

  int has_nose = 0;
  int nose_hoover = 0;
  int nose_restart = 0;
  double nose_electron_period = 0.0;
  double nose_electron_temperature = 0.0;
  double nose_ion_period = 0.0;
  double nose_ion_temperature = 0.0;
  int nose_electron_chain_length = 0;
  int nose_ion_chain_length = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_nose(
                       params_root, &has_nose, &nose_hoover, &nose_restart,
                       &nose_electron_period, &nose_electron_temperature,
                       &nose_ion_period, &nose_ion_temperature,
                       &nose_electron_chain_length, &nose_ion_chain_length),
                   0);
  assert_int_equal(has_nose, 1);
  assert_int_equal(nose_hoover, NWChemNwpwToggle_enabled);
  assert_int_equal(nose_restart, NWChemNwpwToggle_disabled);
  assert_true(nose_electron_period > 11.999);
  assert_true(nose_electron_period < 12.001);
  assert_true(nose_electron_temperature > 299.999);
  assert_true(nose_electron_temperature < 300.001);
  assert_true(nose_ion_period > 33.999);
  assert_true(nose_ion_period < 34.001);
  assert_true(nose_ion_temperature > 399.999);
  assert_true(nose_ion_temperature < 400.001);
  assert_int_equal(nose_electron_chain_length, 2);
  assert_int_equal(nose_ion_chain_length, 3);

  int has_electric_field = 0;
  int atom_efield = 0;
  int atom_efield_gradient = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_electric_field(
                       params_root, &has_electric_field, &atom_efield,
                       &atom_efield_gradient),
                   0);
  assert_int_equal(has_electric_field, 1);
  assert_int_equal(atom_efield, NWChemNwpwToggle_enabled);
  assert_int_equal(atom_efield_gradient, NWChemNwpwToggle_enabled);

  int has_mulliken = 0;
  int nwpw_mulliken = 0;
  int nwpw_mulliken_kawai = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_mulliken(
                       params_root, &has_mulliken, &nwpw_mulliken,
                       &nwpw_mulliken_kawai),
                   0);
  assert_int_equal(has_mulliken, 1);
  assert_int_equal(nwpw_mulliken, NWChemNwpwToggle_enabled);
  assert_int_equal(nwpw_mulliken_kawai, NWChemNwpwToggle_enabled);

  int has_periodic_dipole = 0;
  int periodic_dipole = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_periodic_dipole(
                       params_root, &has_periodic_dipole, &periodic_dipole),
                   0);
  assert_int_equal(has_periodic_dipole, 1);
  assert_int_equal(periodic_dipole, NWChemNwpwToggle_enabled);

  int has_brillouin_zone = 0;
  capn_text brillouin_zone_name = {0};
  int monkhorst_pack[3] = {0, 0, 0};
  int max_kpoints_print = 0;
  double brillouin_kvectors[8] = {0.0};
  size_t brillouin_kvector_count = 0;
  assert_int_equal(nwchemc_params_extract_direct_brillouin_zone(
                       params_root, &has_brillouin_zone,
                       &brillouin_zone_name, monkhorst_pack,
                       &max_kpoints_print, brillouin_kvectors, 2,
                       &brillouin_kvector_count),
                   0);
  assert_int_equal(has_brillouin_zone, 1);
  assert_true(text_equals(brillouin_zone_name, "zoneA"));
  assert_int_equal(monkhorst_pack[0], 3);
  assert_int_equal(monkhorst_pack[1], 4);
  assert_int_equal(monkhorst_pack[2], -5);
  assert_int_equal(max_kpoints_print, 12);
  assert_int_equal((int)brillouin_kvector_count, 2);
  assert_true(brillouin_kvectors[0] > -0.001);
  assert_true(brillouin_kvectors[0] < 0.001);
  assert_true(brillouin_kvectors[1] > -0.001);
  assert_true(brillouin_kvectors[1] < 0.001);
  assert_true(brillouin_kvectors[2] > -0.001);
  assert_true(brillouin_kvectors[2] < 0.001);
  assert_true(brillouin_kvectors[3] > 0.499);
  assert_true(brillouin_kvectors[3] < 0.501);
  assert_true(brillouin_kvectors[4] > 0.499);
  assert_true(brillouin_kvectors[4] < 0.501);
  assert_true(brillouin_kvectors[5] > -0.001);
  assert_true(brillouin_kvectors[5] < 0.001);
  assert_true(brillouin_kvectors[6] > -0.001);
  assert_true(brillouin_kvectors[6] < 0.001);
  assert_true(brillouin_kvectors[7] > 0.499);
  assert_true(brillouin_kvectors[7] < 0.501);

  int has_ccsd = 0;
  int ccsd_maxiter = 0;
  double ccsd_thresh = 0.0;
  double ccsd_tol2e = 0.0;
  int ccsd_iprt = 0;
  int ccsd_max_diis = 0;
  int ccsd_frozen_core = 0;
  int ccsd_frozen_virtual = 0;
  int ccsd_use_disk = 0;
  int ccsd_use_trpdrv_nb = 0;
  int ccsd_use_ccsd_omp = 0;
  int ccsd_use_trpdrv_omp = 0;
  int ccsd_use_trpdrv_offload = 0;
  double ccsd_same_spin_scale = 0.0;
  double ccsd_opposite_spin_scale = 0.0;
  assert_int_equal(nwchemc_params_extract_direct_ccsd(
                       params_root, &has_ccsd, &ccsd_maxiter, &ccsd_thresh,
                       &ccsd_tol2e, &ccsd_iprt, &ccsd_max_diis,
                       &ccsd_frozen_core, &ccsd_frozen_virtual,
                       &ccsd_use_disk, &ccsd_same_spin_scale,
                       &ccsd_opposite_spin_scale, &ccsd_use_trpdrv_nb,
                       &ccsd_use_ccsd_omp, &ccsd_use_trpdrv_omp,
                       &ccsd_use_trpdrv_offload),
                   0);
  assert_int_equal(has_ccsd, 1);
  assert_int_equal(ccsd_maxiter, 20);
  assert_true(ccsd_thresh > 0.999e-7);
  assert_true(ccsd_thresh < 1.001e-7);
  assert_true(ccsd_tol2e > 0.999e-9);
  assert_true(ccsd_tol2e < 1.001e-9);
  assert_int_equal(ccsd_iprt, 2);
  assert_int_equal(ccsd_max_diis, 6);
  assert_int_equal(ccsd_frozen_core, 1);
  assert_int_equal(ccsd_frozen_virtual, 2);
  assert_int_equal(ccsd_use_disk, NWChemToggle_disabled);
  assert_int_equal(ccsd_use_trpdrv_nb, NWChemToggle_enabled);
  assert_int_equal(ccsd_use_ccsd_omp, NWChemToggle_enabled);
  assert_int_equal(ccsd_use_trpdrv_omp, NWChemToggle_disabled);
  assert_int_equal(ccsd_use_trpdrv_offload, NWChemToggle_enabled);
  assert_true(ccsd_same_spin_scale > 1.199);
  assert_true(ccsd_same_spin_scale < 1.201);
  assert_true(ccsd_opposite_spin_scale > 0.799);
  assert_true(ccsd_opposite_spin_scale < 0.801);

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

  int has_psp_spin = 0;
  int pspspin_enabled = 0;
  int pspspin_count = 0;
  int semicore_small = 0;
  assert_int_equal(nwchemc_params_extract_direct_pseudopotential_spin(
                       params_root, &has_psp_spin, &pspspin_enabled,
                       &pspspin_count, &semicore_small),
                   0);
  assert_int_equal(has_psp_spin, 1);
  assert_int_equal(pspspin_enabled, 0);
  assert_int_equal(pspspin_count, 0);
  assert_int_equal(semicore_small, NWChemToggle_enabled);

  nwchemc_params_release(&arena);
  free(message);
}

typedef int (*direct_pseudopotential_entry_fn)(
    void *user_data, capn_text target,
    const struct NWChemPseudopotentialEntry *entry);
extern int nwchemc_params_for_each_direct_pseudopotential(
    NWChemParams_ptr params, direct_pseudopotential_entry_fn callback,
    void *user_data, size_t *count);

struct pseudopotential_walk_capture {
  capn_text targets[8];
  capn_text names[8];
  int types[8];
  size_t count;
};

static int capture_direct_pseudopotential_entry(
    void *user_data, capn_text target,
    const struct NWChemPseudopotentialEntry *entry) {
  struct pseudopotential_walk_capture *capture = user_data;
  assert_non_null(capture);
  assert_non_null(entry);
  assert_true(capture->count < 8);
  capture->targets[capture->count] = target;
  capture->names[capture->count] = entry->libraryName;
  capture->types[capture->count] = entry->libraryType;
  ++capture->count;
  return 0;
}

static void test_parser_walks_direct_pseudopotential_capnp_entries(
    void **state) {
  (void)state;

  size_t message_size = 0;
  unsigned char *message = read_file(g_params_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  struct pseudopotential_walk_capture capture = {0};
  size_t count = 0;
  assert_int_equal(nwchemc_params_for_each_direct_pseudopotential(
                       params_root, capture_direct_pseudopotential_entry,
                       &capture, &count),
                   0);
  assert_int_equal((int)count, 6);
  assert_int_equal((int)capture.count, 6);
  assert_true(text_equals(capture.targets[0], "Si"));
  assert_int_equal(capture.types[0],
                   NWChemPseudopotentialEntry_LibraryType_library);
  assert_true(text_equals(capture.names[0], "sg15"));
  assert_true(text_equals(capture.targets[5], "*"));
  assert_int_equal(capture.types[5],
                   NWChemPseudopotentialEntry_LibraryType_pspwLibrary);
  assert_true(text_equals(capture.names[5], "pspw_default"));

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
      cmocka_unit_test(test_parser_walks_direct_pseudopotential_capnp_entries),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
