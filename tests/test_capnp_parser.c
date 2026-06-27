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
static const char *g_spin_mode_params_path = NULL;
static const char *g_allow_translation_params_path = NULL;
static const char *g_cutoff_alias_params_path = NULL;
static const char *g_mc_steps_params_path = NULL;
static const char *g_bo_steps_default_params_path = NULL;
static const char *g_bo_time_step_default_params_path = NULL;
static const char *g_bo_fake_mass_default_params_path = NULL;
static const char *g_scaling_default_params_path = NULL;
static const char *g_np_dimensions_default_params_path = NULL;
static const char *g_tolerances_default_params_path = NULL;
static const char *g_mc_steps_default_params_path = NULL;
static const char *g_brillouin_tetrahedron_params_path = NULL;
static const char *g_brillouin_dos_grid_params_path = NULL;
static const char *g_nwpw_et_params_path = NULL;
static const char *g_nwpw_temperature_params_path = NULL;
static const char *g_nwpw_mapping_alias_params_path = NULL;
static const char *g_nwpw_mapping_default_params_path = NULL;
static const char *g_nwpw_virtual_alias_params_path = NULL;
static const char *g_nwpw_one_electron_guess_defaults_params_path = NULL;
static const char *g_nwpw_fractional_orbitals_default_params_path = NULL;
static const char *g_nwpw_smear_orbitals_default_params_path = NULL;
static const char *g_nwpw_virtual_orbitals_default_params_path = NULL;
static const char *g_nwpw_translate_vector_default_params_path = NULL;
static const char *g_nwpw_cell_expand_default_params_path = NULL;
static const char *g_brillouin_monkhorst_default_params_path = NULL;
static const char *g_brillouin_dos_grid_default_params_path = NULL;

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
  assert_non_null(strstr(input_blocks, "uterm d 1.4 0.2 4 6"));
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
  assert_non_null(strstr(input_blocks, "virtual 7 8"));
  assert_non_null(strstr(input_blocks, "lcao_skip"));
  assert_non_null(strstr(input_blocks, "ewald_ngrid 9 10 11"));
  assert_non_null(strstr(input_blocks, "Nose-Hoover 12 300 34 400 start 2 3"));
  assert_non_null(strstr(input_blocks, "atom_efield"));
  assert_non_null(strstr(input_blocks, "atom_efield_grad"));
  assert_non_null(strstr(input_blocks, "mulliken kawai"));
  assert_non_null(strstr(input_blocks, "periodic_dipole true"));
  assert_non_null(
      strstr(input_blocks, "efield true 0.1 0.2 0.3 center 1 2 3 APC"));
  assert_non_null(strstr(input_blocks, "smooth_cutoff true 1.5 3.5"));
  assert_non_null(strstr(input_blocks, "cutoff_boot_wavefunction false"));
  assert_non_null(strstr(input_blocks, "fast_erf true"));
  assert_non_null(strstr(input_blocks, "dipole_motion dipole.mov"));
  assert_non_null(strstr(input_blocks, "symmetry false"));
  assert_non_null(strstr(input_blocks, "one_electron_guess 25 3 2"));
  assert_non_null(strstr(input_blocks, "fmm true 12 2"));
  assert_non_null(
      strstr(input_blocks, "born 78.4 relax true 0.529177 1.058354"));
  assert_non_null(strstr(input_blocks, "vfield vf_a.ascii vf_b.ascii"));
  assert_non_null(strstr(input_blocks, "single_precision_hfx"));
  assert_non_null(strstr(input_blocks, "geometry_optimize"));
  assert_non_null(strstr(input_blocks, "auxiliary_potentials"));
  assert_non_null(strstr(input_blocks, "mult 3"));
  assert_non_null(strstr(input_blocks, "dos 0.0025 120 -0.5 1.5"));
  assert_non_null(strstr(input_blocks, "dos_filename dos.dat"));
  assert_non_null(strstr(input_blocks, "cpmd_properties true"));
  assert_non_null(strstr(input_blocks, "use_grid_cmp false"));
  assert_non_null(strstr(input_blocks, "director director.dat"));
  assert_non_null(strstr(input_blocks, "expand_cell 2 3 4"));
  assert_non_null(strstr(input_blocks, "mapping 3"));
  assert_non_null(strstr(input_blocks, "rotation false"));
  assert_non_null(strstr(input_blocks, "integrate_mult_l 4"));
  assert_non_null(strstr(input_blocks, "Fei fei.dat"));
  assert_non_null(strstr(input_blocks, "initial_velocities 298.15 12345"));
  assert_non_null(strstr(input_blocks, "makehmass2 false"));
  assert_non_null(
      strstr(input_blocks, "translate_vector 0.1 0.2 0.3 geomA reorder"));
  assert_non_null(strstr(input_blocks, "socket ipi_client 127.0.0.1:31415"));
  assert_non_null(strstr(input_blocks, "apc 1.25 0.5 0.25 0.125"));
  assert_non_null(strstr(input_blocks, "translation false"));
  assert_non_null(strstr(input_blocks, "lmbfgs stiefel"));
  assert_non_null(strstr(input_blocks,
                         "scf density rmm-diis diis precondition kerker "
                         "0.375 alpha 0.125 iterations 12 "
                         "outer_iterations 4 diis_histories 6"));
  assert_non_null(strstr(input_blocks, "monkhorst-pack 3 4 -5 zoneA"));
  assert_non_null(strstr(input_blocks, "zone_name zoneA"));
  assert_non_null(strstr(input_blocks, "zone_structure_name structureA"));
  assert_non_null(strstr(input_blocks, "zone_fft_name fftA"));
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
  assert_null(strstr(input_blocks, "virtual 7 8"));
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
  assert_null(strstr(input_blocks, "efield true"));
  assert_null(strstr(input_blocks, "nwpw:efield"));
  assert_null(strstr(input_blocks, "smooth_cutoff"));
  assert_null(strstr(input_blocks, "nwpw:smooth_cutoff"));
  assert_null(strstr(input_blocks, "cutoff_boot_wavefunction"));
  assert_null(strstr(input_blocks, "nwpw:cutoff_boot_psi"));
  assert_null(strstr(input_blocks, "fast_erf"));
  assert_null(strstr(input_blocks, "nwpw:fast_erf"));
  assert_null(strstr(input_blocks, "dipole_motion"));
  assert_null(strstr(input_blocks, "nwpw:dipole_motion"));
  assert_null(strstr(input_blocks, "symmetry false"));
  assert_null(strstr(input_blocks, "nwpw:rho_use_symmetry"));
  assert_null(strstr(input_blocks, "one_electron_guess 25"));
  assert_null(strstr(input_blocks, "nwpw:H1_it_in"));
  assert_null(strstr(input_blocks, "fmm true"));
  assert_null(strstr(input_blocks, "nwpw:fmm"));
  assert_null(strstr(input_blocks, "born 78.4"));
  assert_null(strstr(input_blocks, "nwpw:born"));
  assert_null(strstr(input_blocks, "vfield vf_a.ascii"));
  assert_null(strstr(input_blocks, "nwpw:vfield_filenames"));
  assert_null(strstr(input_blocks, "single_precision_hfx"));
  assert_null(strstr(input_blocks, "pspw:HFX_single_precision"));
  assert_null(strstr(input_blocks, "geometry_optimize"));
  assert_null(strstr(input_blocks, "cgsd:geometry_optimize"));
  assert_null(strstr(input_blocks, "auxiliary_potentials"));
  assert_null(strstr(input_blocks, "pspw_qmmm_auxon"));
  assert_null(strstr(input_blocks, "mult 3"));
  assert_null(strstr(input_blocks, "cgsd:mult"));
  assert_null(strstr(input_blocks, "dos 0.0025"));
  assert_null(strstr(input_blocks, "dos_filename dos.dat"));
  assert_null(strstr(input_blocks, "dos:alpha"));
  assert_null(strstr(input_blocks, "nwpw:dos:filename"));
  assert_null(strstr(input_blocks, "cpmd_properties true"));
  assert_null(strstr(input_blocks, "nwpw:cpmd_properties"));
  assert_null(strstr(input_blocks, "use_grid_cmp false"));
  assert_null(strstr(input_blocks, "nwpw:use_grid_cmp"));
  assert_null(strstr(input_blocks, "director director.dat"));
  assert_null(strstr(input_blocks, "nwpw:use_director"));
  assert_null(strstr(input_blocks, "expand_cell 2 3 4"));
  assert_null(strstr(input_blocks, "nwpw:cell_expand"));
  assert_null(strstr(input_blocks, "mapping 3"));
  assert_null(strstr(input_blocks, "nwpw:mapping"));
  assert_null(strstr(input_blocks, "rotation false"));
  assert_null(strstr(input_blocks, "nwpw:rotation"));
  assert_null(strstr(input_blocks, "integrate_mult_l 4"));
  assert_null(strstr(input_blocks, "nwpw:lmax_multipole"));
  assert_null(strstr(input_blocks, "Fei fei.dat"));
  assert_null(strstr(input_blocks, "nwpw:fei"));
  assert_null(strstr(input_blocks, "cpmd:fei"));
  assert_null(strstr(input_blocks, "initial_velocities 298.15"));
  assert_null(strstr(input_blocks, "nwpw:init_velocities"));
  assert_null(strstr(input_blocks, "makehmass2 false"));
  assert_null(strstr(input_blocks, "nwpw:makehmass2"));
  assert_null(strstr(input_blocks, "translate_vector 0.1"));
  assert_null(strstr(input_blocks, "nwpw:translate_vector"));
  assert_null(strstr(input_blocks, "socket ipi_client"));
  assert_null(strstr(input_blocks, "nwpw:socket_type"));
  assert_null(strstr(input_blocks, "apc 1.25"));
  assert_null(strstr(input_blocks, "nwpw_APC:Gc"));
  assert_null(strstr(input_blocks, "translation false"));
  assert_null(strstr(input_blocks, "cgsd:allow_translation"));
  assert_null(strstr(input_blocks, "lmbfgs stiefel"));
  assert_null(strstr(input_blocks, "scf density rmm-diis"));
  assert_null(strstr(input_blocks, "nwpw:minimizer"));
  assert_null(strstr(input_blocks, "nwpw:ks_algorithm"));
  assert_null(strstr(input_blocks, "nwpw:kerker_g0"));
  assert_null(strstr(input_blocks, "band_structure:zone_name"));
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

  int has_efield = 0;
  int efield = 0;
  double efield_vector[3] = {0.0, 0.0, 0.0};
  int efield_has_center = 0;
  double efield_center[3] = {0.0, 0.0, 0.0};
  int efield_type = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_efield(
                       params_root, &has_efield, &efield, efield_vector,
                       &efield_has_center, efield_center, &efield_type),
                   0);
  assert_int_equal(has_efield, 1);
  assert_int_equal(efield, NWChemNwpwToggle_enabled);
  assert_true(efield_vector[0] > 0.099);
  assert_true(efield_vector[0] < 0.101);
  assert_true(efield_vector[1] > 0.199);
  assert_true(efield_vector[1] < 0.201);
  assert_true(efield_vector[2] > 0.299);
  assert_true(efield_vector[2] < 0.301);
  assert_int_equal(efield_has_center, 1);
  assert_true(efield_center[0] > 0.999);
  assert_true(efield_center[0] < 1.001);
  assert_true(efield_center[1] > 1.999);
  assert_true(efield_center[1] < 2.001);
  assert_true(efield_center[2] > 2.999);
  assert_true(efield_center[2] < 3.001);
  assert_int_equal(efield_type, NWChemNwpwEfieldType_apc);

  int has_smooth_cutoff = 0;
  int smooth_cutoff = 0;
  double smooth_cutoff_values[2] = {0.0, 0.0};
  assert_int_equal(nwchemc_params_extract_direct_nwpw_smooth_cutoff(
                       params_root, &has_smooth_cutoff, &smooth_cutoff,
                       smooth_cutoff_values),
                   0);
  assert_int_equal(has_smooth_cutoff, 1);
  assert_int_equal(smooth_cutoff, NWChemNwpwToggle_enabled);
  assert_true(smooth_cutoff_values[0] > 1.499);
  assert_true(smooth_cutoff_values[0] < 1.501);
  assert_true(smooth_cutoff_values[1] > 3.499);
  assert_true(smooth_cutoff_values[1] < 3.501);

  int has_cutoff_boot_wavefunction = 0;
  int cutoff_boot_wavefunction = 0;
  assert_int_equal(
      nwchemc_params_extract_direct_nwpw_cutoff_boot_wavefunction(
          params_root, &has_cutoff_boot_wavefunction,
          &cutoff_boot_wavefunction),
      0);
  assert_int_equal(has_cutoff_boot_wavefunction, 1);
  assert_int_equal(cutoff_boot_wavefunction, NWChemNwpwToggle_disabled);

  int has_fast_erf = 0;
  int fast_erf = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_fast_erf(
                       params_root, &has_fast_erf, &fast_erf),
                   0);
  assert_int_equal(has_fast_erf, 1);
  assert_int_equal(fast_erf, NWChemNwpwToggle_enabled);

  int has_dipole_motion = 0;
  int dipole_motion = 0;
  capn_text dipole_motion_filename = {0};
  assert_int_equal(nwchemc_params_extract_direct_nwpw_dipole_motion(
                       params_root, &has_dipole_motion, &dipole_motion,
                       &dipole_motion_filename),
                   0);
  assert_int_equal(has_dipole_motion, 1);
  assert_int_equal(dipole_motion, NWChemNwpwToggle_enabled);
  assert_true(text_equals(dipole_motion_filename, "dipole.mov"));

  int has_rho_use_symmetry = 0;
  int rho_use_symmetry = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_rho_use_symmetry(
                       params_root, &has_rho_use_symmetry, &rho_use_symmetry),
                   0);
  assert_int_equal(has_rho_use_symmetry, 1);
  assert_int_equal(rho_use_symmetry, NWChemNwpwToggle_disabled);

  int has_one_electron_guess = 0;
  int one_electron_guess_it_in = 0;
  int one_electron_guess_it_out = 0;
  int one_electron_guess_it_ortho = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_one_electron_guess(
                       params_root, &has_one_electron_guess,
                       &one_electron_guess_it_in, &one_electron_guess_it_out,
                       &one_electron_guess_it_ortho),
                   0);
  assert_int_equal(has_one_electron_guess, 1);
  assert_int_equal(one_electron_guess_it_in, 25);
  assert_int_equal(one_electron_guess_it_out, 3);
  assert_int_equal(one_electron_guess_it_ortho, 2);

  int has_fmm = 0;
  int fmm = 0;
  int fmm_lmax = 0;
  int fmm_long_range = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_fmm(
                       params_root, &has_fmm, &fmm, &fmm_lmax,
                       &fmm_long_range),
                   0);
  assert_int_equal(has_fmm, 1);
  assert_int_equal(fmm, NWChemNwpwToggle_enabled);
  assert_int_equal(fmm_lmax, 12);
  assert_int_equal(fmm_long_range, 2);

  int has_born = 0;
  int born = 0;
  double born_dielectric = 0.0;
  int born_relax = 0;
  double born_vradii_angstrom[4] = {0.0, 0.0, 0.0, 0.0};
  size_t born_vradii_count = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_born(
                       params_root, &has_born, &born, &born_dielectric,
                       &born_relax, born_vradii_angstrom, 4,
                       &born_vradii_count),
                   0);
  assert_int_equal(has_born, 1);
  assert_int_equal(born, NWChemNwpwToggle_enabled);
  assert_true(born_dielectric > 78.399);
  assert_true(born_dielectric < 78.401);
  assert_int_equal(born_relax, NWChemNwpwToggle_enabled);
  assert_int_equal((int)born_vradii_count, 2);
  assert_true(born_vradii_angstrom[0] > 0.529176);
  assert_true(born_vradii_angstrom[0] < 0.529178);
  assert_true(born_vradii_angstrom[1] > 1.058353);
  assert_true(born_vradii_angstrom[1] < 1.058355);

  int has_vfield = 0;
  capn_text vfield_filenames[4];
  size_t vfield_count = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_vfield(
                       params_root, &has_vfield, vfield_filenames, 4,
                       &vfield_count),
                   0);
  assert_int_equal(has_vfield, 1);
  assert_int_equal((int)vfield_count, 2);
  assert_true(text_equals(vfield_filenames[0], "vf_a.ascii"));
  assert_true(text_equals(vfield_filenames[1], "vf_b.ascii"));

  int has_single_precision_hfx = 0;
  int single_precision_hfx = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_single_precision_hfx(
                       params_root, &has_single_precision_hfx,
                       &single_precision_hfx),
                   0);
  assert_int_equal(has_single_precision_hfx, 1);
  assert_int_equal(single_precision_hfx, 1);

  int has_geometry_optimize = 0;
  int geometry_optimize = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_geometry_optimize(
                       params_root, &has_geometry_optimize,
                       &geometry_optimize),
                   0);
  assert_int_equal(has_geometry_optimize, 1);
  assert_int_equal(geometry_optimize, 1);

  int has_auxiliary_potentials = 0;
  int auxiliary_potentials = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_auxiliary_potentials(
                       params_root, &has_auxiliary_potentials,
                       &auxiliary_potentials),
                   0);
  assert_int_equal(has_auxiliary_potentials, 1);
  assert_int_equal(auxiliary_potentials, 1);

  int has_nwpw_multiplicity = 0;
  int nwpw_multiplicity = 0;
  int nwpw_ispin = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_multiplicity(
                       params_root, &has_nwpw_multiplicity,
                       &nwpw_multiplicity, &nwpw_ispin),
                   0);
  assert_int_equal(has_nwpw_multiplicity, 1);
  assert_int_equal(nwpw_multiplicity, 3);
  assert_int_equal(nwpw_ispin, 2);

  int has_dos = 0;
  int dos_alpha_set = 0;
  double dos_alpha = 0.0;
  int dos_npoints_set = 0;
  int dos_npoints = 0;
  int dos_emin_set = 0;
  double dos_emin = 0.0;
  int dos_emax_set = 0;
  double dos_emax = 0.0;
  capn_text dos_filename = {0};
  assert_int_equal(nwchemc_params_extract_direct_nwpw_dos(
                       params_root, &has_dos, &dos_alpha_set, &dos_alpha,
                       &dos_npoints_set, &dos_npoints, &dos_emin_set,
                       &dos_emin, &dos_emax_set, &dos_emax, &dos_filename),
                   0);
  assert_int_equal(has_dos, 1);
  assert_int_equal(dos_alpha_set, 1);
  assert_true(dos_alpha > 0.002499);
  assert_true(dos_alpha < 0.002501);
  assert_int_equal(dos_npoints_set, 1);
  assert_int_equal(dos_npoints, 120);
  assert_int_equal(dos_emin_set, 1);
  assert_true(dos_emin < -0.499);
  assert_true(dos_emin > -0.501);
  assert_int_equal(dos_emax_set, 1);
  assert_true(dos_emax > 1.499);
  assert_true(dos_emax < 1.501);
  assert_true(text_equals(dos_filename, "dos.dat"));

  int has_cpmd_grid = 0;
  int cpmd_properties = 0;
  int use_grid_comparison = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_cpmd_grid(
                       params_root, &has_cpmd_grid, &cpmd_properties,
                       &use_grid_comparison),
                   0);
  assert_int_equal(has_cpmd_grid, 1);
  assert_int_equal(cpmd_properties, NWChemNwpwToggle_enabled);
  assert_int_equal(use_grid_comparison, NWChemNwpwToggle_disabled);

  int has_director = 0;
  int director = 0;
  capn_text director_filename = {0};
  assert_int_equal(nwchemc_params_extract_direct_nwpw_director(
                       params_root, &has_director, &director,
                       &director_filename),
                   0);
  assert_int_equal(has_director, 1);
  assert_int_equal(director, NWChemNwpwToggle_enabled);
  assert_true(text_equals(director_filename, "director.dat"));

  int has_cell_mapping = 0;
  int cell_expand[3] = {0, 0, 0};
  int mapping = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_cell_mapping(
                       params_root, &has_cell_mapping, cell_expand, &mapping),
                   0);
  assert_int_equal(has_cell_mapping, 1);
  assert_int_equal(cell_expand[0], 2);
  assert_int_equal(cell_expand[1], 3);
  assert_int_equal(cell_expand[2], 4);
  assert_int_equal(mapping, 3);

  int has_rotation_multipole = 0;
  int rotation = 0;
  int lmax_multipole = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_rotation_multipole(
                       params_root, &has_rotation_multipole, &rotation,
                       &lmax_multipole),
                   0);
  assert_int_equal(has_rotation_multipole, 1);
  assert_int_equal(rotation, NWChemNwpwToggle_disabled);
  assert_int_equal(lmax_multipole, 4);

  int has_fei = 0;
  int fei = 0;
  capn_text fei_filename = {0};
  assert_int_equal(nwchemc_params_extract_direct_nwpw_fei(
                       params_root, &has_fei, &fei, &fei_filename),
                   0);
  assert_int_equal(has_fei, 1);
  assert_int_equal(fei, 1);
  assert_true(text_equals(fei_filename, "fei.dat"));

  int has_initial_velocities = 0;
  double initial_velocities_temperature = 0.0;
  int initial_velocities_seed = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_initial_velocities(
                       params_root, &has_initial_velocities,
                       &initial_velocities_temperature,
                       &initial_velocities_seed),
                   0);
  assert_int_equal(has_initial_velocities, 1);
  assert_true(initial_velocities_temperature > 298.149);
  assert_true(initial_velocities_temperature < 298.151);
  assert_int_equal(initial_velocities_seed, 12345);

  int has_make_hmass2 = 0;
  int make_hmass2 = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_make_hmass2(
                       params_root, &has_make_hmass2, &make_hmass2),
                   0);
  assert_int_equal(has_make_hmass2, 1);
  assert_int_equal(make_hmass2, NWChemNwpwToggle_disabled);

  int has_translate_vector = 0;
  double translate_vector[3] = {0.0, 0.0, 0.0};
  capn_text translate_geometry_name = {0};
  int translate_reorder = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_translate_vector(
                       params_root, &has_translate_vector, translate_vector,
                       &translate_geometry_name, &translate_reorder),
                   0);
  assert_int_equal(has_translate_vector, 1);
  assert_true(translate_vector[0] > 0.099);
  assert_true(translate_vector[0] < 0.101);
  assert_true(translate_vector[1] > 0.199);
  assert_true(translate_vector[1] < 0.201);
  assert_true(translate_vector[2] > 0.299);
  assert_true(translate_vector[2] < 0.301);
  assert_true(text_equals(translate_geometry_name, "geomA"));
  assert_int_equal(translate_reorder, NWChemNwpwToggle_enabled);

  int has_socket = 0;
  capn_text socket_type = {0};
  capn_text socket_ip = {0};
  assert_int_equal(nwchemc_params_extract_direct_nwpw_socket(
                       params_root, &has_socket, &socket_type, &socket_ip),
                   0);
  assert_int_equal(has_socket, 1);
  assert_true(text_equals(socket_type, "ipi_client"));
  assert_true(text_equals(socket_ip, "127.0.0.1:31415"));

  int has_apc = 0;
  double apc_gc = 0.0;
  double apc_gamma[4] = {0.0, 0.0, 0.0, 0.0};
  size_t apc_gamma_count = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_apc(
                       params_root, &has_apc, &apc_gc, apc_gamma, 4,
                       &apc_gamma_count),
                   0);
  assert_int_equal(has_apc, 1);
  assert_true(apc_gc > 1.249);
  assert_true(apc_gc < 1.251);
  assert_int_equal((int)apc_gamma_count, 3);
  assert_true(apc_gamma[0] > 0.499);
  assert_true(apc_gamma[0] < 0.501);
  assert_true(apc_gamma[1] > 0.249);
  assert_true(apc_gamma[1] < 0.251);
  assert_true(apc_gamma[2] > 0.124);
  assert_true(apc_gamma[2] < 0.126);

  int has_translation = 0;
  int translation = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_translation(
                       params_root, &has_translation, &translation),
                   0);
  assert_int_equal(has_translation, 1);
  assert_int_equal(translation, NWChemNwpwToggle_disabled);

  int has_minimizer = 0;
  int minimizer = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_minimizer(
                       params_root, &has_minimizer, &minimizer),
                   0);
  assert_int_equal(has_minimizer, 1);
  assert_int_equal(minimizer, NWChemNwpwMinimizer_scfDensity);

  int has_scf_algorithms = 0;
  int ks_algorithm = 0;
  int scf_algorithm = 0;
  int precondition = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_scf_algorithms(
                       params_root, &has_scf_algorithms, &ks_algorithm,
                       &scf_algorithm, &precondition),
                   0);
  assert_int_equal(has_scf_algorithms, 1);
  assert_int_equal(ks_algorithm, NWChemNwpwKsAlgorithm_rmmDiis);
  assert_int_equal(scf_algorithm, NWChemNwpwScfAlgorithm_diis);
  assert_int_equal(precondition, NWChemNwpwToggle_enabled);

  int has_scf_numeric = 0;
  NWChemNwpwScfNumericControls scf_numeric = {0};
  assert_int_equal(nwchemc_params_extract_direct_nwpw_scf_numeric(
                       params_root, &has_scf_numeric, &scf_numeric),
                   0);
  assert_int_equal(has_scf_numeric, 1);
  assert_int_equal(scf_numeric.kerker_g0_set, 1);
  assert_true(scf_numeric.kerker_g0 > 0.374);
  assert_true(scf_numeric.kerker_g0 < 0.376);
  assert_int_equal(scf_numeric.ks_alpha_set, 1);
  assert_true(scf_numeric.ks_alpha > 0.124);
  assert_true(scf_numeric.ks_alpha < 0.126);
  assert_int_equal(scf_numeric.ks_maxit_orb_set, 1);
  assert_int_equal(scf_numeric.ks_maxit_orb, 12);
  assert_int_equal(scf_numeric.ks_maxit_orbs_set, 1);
  assert_int_equal(scf_numeric.ks_maxit_orbs, 4);
  assert_int_equal(scf_numeric.diis_histories_set, 1);
  assert_int_equal(scf_numeric.diis_histories, 6);

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

static void test_parser_extracts_direct_nwpw_spin_mode(void **state) {
  (void)state;

  size_t message_size = 0;
  unsigned char *message = read_file(g_spin_mode_params_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  char full_blocks[NWCHEMC_BLOCKS];
  char embed_blocks[NWCHEMC_BLOCKS];
  assert_int_equal(nwchemc_params_render_input_blocks(
                       params_root, full_blocks, sizeof(full_blocks)),
                   0);
  assert_non_null(strstr(full_blocks, "  dft\n"));
  assert_non_null(strstr(full_blocks, "  odft\n"));
  assert_int_equal(nwchemc_params_render_embed_input_blocks(
                       params_root, embed_blocks, sizeof(embed_blocks)),
                   0);
  assert_null(strstr(embed_blocks, "  dft\n"));
  assert_null(strstr(embed_blocks, "  odft\n"));

  int has_spin_mode = 0;
  int spin_mode = 0;
  int ispin = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_spin_mode(
                       params_root, &has_spin_mode, &spin_mode, &ispin),
                   0);
  assert_int_equal(has_spin_mode, 1);
  assert_int_equal(spin_mode, NWChemNwpwSpinMode_unrestricted);
  assert_int_equal(ispin, 2);

  nwchemc_params_release(&arena);
  free(message);
}

static void test_parser_extracts_direct_nwpw_allow_translation(void **state) {
  (void)state;

  size_t message_size = 0;
  unsigned char *message =
      read_file(g_allow_translation_params_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  char full_blocks[NWCHEMC_BLOCKS];
  char embed_blocks[NWCHEMC_BLOCKS];
  assert_int_equal(nwchemc_params_render_input_blocks(
                       params_root, full_blocks, sizeof(full_blocks)),
                   0);
  assert_non_null(strstr(full_blocks, "  allow_translation\n"));
  assert_int_equal(nwchemc_params_render_embed_input_blocks(
                       params_root, embed_blocks, sizeof(embed_blocks)),
                   0);
  assert_null(strstr(embed_blocks, "  allow_translation\n"));

  int has_allow_translation = 0;
  int allow_translation = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_allow_translation(
                       params_root, &has_allow_translation,
                       &allow_translation),
                   0);
  assert_int_equal(has_allow_translation, 1);
  assert_int_equal(allow_translation, 1);

  nwchemc_params_release(&arena);
  free(message);
}

static void test_parser_extracts_direct_nwpw_cutoff_alias(void **state) {
  (void)state;

  size_t message_size = 0;
  unsigned char *message = read_file(g_cutoff_alias_params_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  char full_blocks[NWCHEMC_BLOCKS];
  char embed_blocks[NWCHEMC_BLOCKS];
  assert_int_equal(nwchemc_params_render_input_blocks(
                       params_root, full_blocks, sizeof(full_blocks)),
                   0);
  assert_non_null(strstr(full_blocks, "  cutoff 7.5\n"));
  assert_int_equal(nwchemc_params_render_embed_input_blocks(
                       params_root, embed_blocks, sizeof(embed_blocks)),
                   0);
  assert_null(strstr(embed_blocks, "  cutoff 7.5\n"));

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
  assert_true(energy_cutoff > 14.999);
  assert_true(energy_cutoff < 15.001);
  assert_true(wavefunction_cutoff > 7.499);
  assert_true(wavefunction_cutoff < 7.501);

  nwchemc_params_release(&arena);
  free(message);
}

static void test_parser_extracts_direct_nwpw_mc_steps(void **state) {
  (void)state;

  size_t message_size = 0;
  unsigned char *message = read_file(g_mc_steps_params_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  char full_blocks[NWCHEMC_BLOCKS];
  char embed_blocks[NWCHEMC_BLOCKS];
  assert_int_equal(nwchemc_params_render_input_blocks(
                       params_root, full_blocks, sizeof(full_blocks)),
                   0);
  assert_non_null(strstr(full_blocks, "  mc_steps 13 17\n"));
  assert_null(strstr(full_blocks, "  bo_steps 13 17\n"));
  assert_int_equal(nwchemc_params_render_embed_input_blocks(
                       params_root, embed_blocks, sizeof(embed_blocks)),
                   0);
  assert_null(strstr(embed_blocks, "  mc_steps 13 17\n"));

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
  assert_int_equal(balance_mode, NWChemNwpwBalanceMode_unspecified);
  assert_int_equal(bo_step_start, 13);
  assert_int_equal(bo_step_end, 17);
  assert_true(bo_time_step == 0.0);
  assert_int_equal(bo_algorithm, NWChemNwpwBoAlgorithm_unspecified);
  assert_true(bo_fake_mass == 0.0);
  assert_int_equal(has_scaling, 0);

  nwchemc_params_release(&arena);
  free(message);
}

static void test_parser_extracts_direct_nwpw_bo_steps_default(void **state) {
  (void)state;

  size_t message_size = 0;
  unsigned char *message =
      read_file(g_bo_steps_default_params_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  char full_blocks[NWCHEMC_BLOCKS];
  char embed_blocks[NWCHEMC_BLOCKS];
  assert_int_equal(nwchemc_params_render_input_blocks(
                       params_root, full_blocks, sizeof(full_blocks)),
                   0);
  assert_non_null(strstr(full_blocks, "  bo_steps 12 100\n"));
  assert_int_equal(nwchemc_params_render_embed_input_blocks(
                       params_root, embed_blocks, sizeof(embed_blocks)),
                   0);
  assert_null(strstr(embed_blocks, "  bo_steps 12 100\n"));

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
  assert_int_equal(balance_mode, NWChemNwpwBalanceMode_unspecified);
  assert_int_equal(bo_step_start, 12);
  assert_int_equal(bo_step_end, 100);
  assert_true(bo_time_step == 0.0);
  assert_int_equal(bo_algorithm, NWChemNwpwBoAlgorithm_unspecified);
  assert_true(bo_fake_mass == 0.0);
  assert_int_equal(has_scaling, 0);

  nwchemc_params_release(&arena);
  free(message);
}

static void
test_parser_extracts_direct_nwpw_bo_time_step_default(void **state) {
  (void)state;

  size_t message_size = 0;
  unsigned char *message =
      read_file(g_bo_time_step_default_params_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  char full_blocks[NWCHEMC_BLOCKS];
  char embed_blocks[NWCHEMC_BLOCKS];
  assert_int_equal(nwchemc_params_render_input_blocks(
                       params_root, full_blocks, sizeof(full_blocks)),
                   0);
  assert_non_null(strstr(full_blocks, "  bo_time_step 15\n"));
  assert_int_equal(nwchemc_params_render_embed_input_blocks(
                       params_root, embed_blocks, sizeof(embed_blocks)),
                   0);
  assert_null(strstr(embed_blocks, "  bo_time_step 15\n"));

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
  assert_int_equal(balance_mode, NWChemNwpwBalanceMode_unspecified);
  assert_int_equal(bo_step_start, 0);
  assert_int_equal(bo_step_end, 0);
  assert_true(bo_time_step > 14.999);
  assert_true(bo_time_step < 15.001);
  assert_int_equal(bo_algorithm, NWChemNwpwBoAlgorithm_unspecified);
  assert_true(bo_fake_mass == 0.0);
  assert_int_equal(has_scaling, 0);

  nwchemc_params_release(&arena);
  free(message);
}

static void
test_parser_extracts_direct_nwpw_bo_fake_mass_default(void **state) {
  (void)state;

  size_t message_size = 0;
  unsigned char *message =
      read_file(g_bo_fake_mass_default_params_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  char full_blocks[NWCHEMC_BLOCKS];
  char embed_blocks[NWCHEMC_BLOCKS];
  assert_int_equal(nwchemc_params_render_input_blocks(
                       params_root, full_blocks, sizeof(full_blocks)),
                   0);
  assert_non_null(strstr(full_blocks, "  bo_fake_mass 500\n"));
  assert_int_equal(nwchemc_params_render_embed_input_blocks(
                       params_root, embed_blocks, sizeof(embed_blocks)),
                   0);
  assert_null(strstr(embed_blocks, "  bo_fake_mass 500\n"));

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
  assert_int_equal(balance_mode, NWChemNwpwBalanceMode_unspecified);
  assert_int_equal(bo_step_start, 0);
  assert_int_equal(bo_step_end, 0);
  assert_true(bo_time_step == 0.0);
  assert_int_equal(bo_algorithm, NWChemNwpwBoAlgorithm_unspecified);
  assert_true(bo_fake_mass > 499.999);
  assert_true(bo_fake_mass < 500.001);
  assert_int_equal(has_scaling, 0);

  nwchemc_params_release(&arena);
  free(message);
}

static void test_parser_extracts_direct_nwpw_scaling_default(void **state) {
  (void)state;

  size_t message_size = 0;
  unsigned char *message =
      read_file(g_scaling_default_params_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  char full_blocks[NWCHEMC_BLOCKS];
  char embed_blocks[NWCHEMC_BLOCKS];
  assert_int_equal(nwchemc_params_render_input_blocks(
                       params_root, full_blocks, sizeof(full_blocks)),
                   0);
  assert_non_null(strstr(full_blocks, "  scaling 1 1\n"));
  assert_int_equal(nwchemc_params_render_embed_input_blocks(
                       params_root, embed_blocks, sizeof(embed_blocks)),
                   0);
  assert_null(strstr(embed_blocks, "  scaling 1 1\n"));

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
  assert_int_equal(balance_mode, NWChemNwpwBalanceMode_unspecified);
  assert_int_equal(bo_step_start, 0);
  assert_int_equal(bo_step_end, 0);
  assert_true(bo_time_step == 0.0);
  assert_int_equal(bo_algorithm, NWChemNwpwBoAlgorithm_unspecified);
  assert_true(bo_fake_mass == 0.0);
  assert_int_equal(has_scaling, 1);
  assert_true(scaling_first > 0.999);
  assert_true(scaling_first < 1.001);
  assert_true(scaling_second > 0.999);
  assert_true(scaling_second < 1.001);

  nwchemc_params_release(&arena);
  free(message);
}

static void
test_parser_extracts_direct_nwpw_np_dimensions_default(void **state) {
  (void)state;

  size_t message_size = 0;
  unsigned char *message =
      read_file(g_np_dimensions_default_params_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  char full_blocks[NWCHEMC_BLOCKS];
  char embed_blocks[NWCHEMC_BLOCKS];
  assert_int_equal(nwchemc_params_render_input_blocks(
                       params_root, full_blocks, sizeof(full_blocks)),
                   0);
  assert_non_null(strstr(full_blocks, "  np_dimensions -1 -1 -1\n"));
  assert_int_equal(nwchemc_params_render_embed_input_blocks(
                       params_root, embed_blocks, sizeof(embed_blocks)),
                   0);
  assert_null(strstr(embed_blocks, "  np_dimensions -1 -1 -1\n"));

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
  assert_int_equal(np_fft, -1);
  assert_int_equal(np_orbital, -1);
  assert_int_equal(np_kspace, -1);
  assert_int_equal(spin_orbit, NWChemNwpwToggle_unspecified);
  assert_int_equal(parallel_io, NWChemNwpwToggle_unspecified);

  nwchemc_params_release(&arena);
  free(message);
}

static void test_parser_extracts_direct_nwpw_tolerances_default(void **state) {
  (void)state;

  size_t message_size = 0;
  unsigned char *message =
      read_file(g_tolerances_default_params_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  char full_blocks[NWCHEMC_BLOCKS];
  char embed_blocks[NWCHEMC_BLOCKS];
  assert_int_equal(nwchemc_params_render_input_blocks(
                       params_root, full_blocks, sizeof(full_blocks)),
                   0);
  assert_non_null(strstr(full_blocks, "  tolerances 1e-07 1e-07 0.0001\n"));
  assert_int_equal(nwchemc_params_render_embed_input_blocks(
                       params_root, embed_blocks, sizeof(embed_blocks)),
                   0);
  assert_null(strstr(embed_blocks, "  tolerances 1e-07 1e-07 0.0001\n"));

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
  assert_int_equal(cell_name.len, 0);
  assert_int_equal(input_wavefunction_filename.len, 0);
  assert_int_equal(output_wavefunction_filename.len, 0);
  assert_true(fake_mass == 0.0);
  assert_true(time_step == 0.0);
  assert_int_equal(loop_start, 0);
  assert_int_equal(loop_end, 0);
  assert_int_equal(has_tolerances, 1);
  assert_true(tolerance_energy > 0.000000099);
  assert_true(tolerance_energy < 0.000000101);
  assert_true(tolerance_density > 0.000000099);
  assert_true(tolerance_density < 0.000000101);
  assert_true(tolerance_gradient > 0.0000999);
  assert_true(tolerance_gradient < 0.0001001);

  nwchemc_params_release(&arena);
  free(message);
}

static void test_parser_extracts_direct_nwpw_mc_steps_default(void **state) {
  (void)state;

  size_t message_size = 0;
  unsigned char *message =
      read_file(g_mc_steps_default_params_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  char full_blocks[NWCHEMC_BLOCKS];
  char embed_blocks[NWCHEMC_BLOCKS];
  assert_int_equal(nwchemc_params_render_input_blocks(
                       params_root, full_blocks, sizeof(full_blocks)),
                   0);
  assert_non_null(strstr(full_blocks, "  mc_steps 14 100\n"));
  assert_null(strstr(full_blocks, "  bo_steps 14 100\n"));
  assert_int_equal(nwchemc_params_render_embed_input_blocks(
                       params_root, embed_blocks, sizeof(embed_blocks)),
                   0);
  assert_null(strstr(embed_blocks, "  mc_steps 14 100\n"));

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
  assert_int_equal(balance_mode, NWChemNwpwBalanceMode_unspecified);
  assert_int_equal(bo_step_start, 14);
  assert_int_equal(bo_step_end, 100);
  assert_true(bo_time_step == 0.0);
  assert_int_equal(bo_algorithm, NWChemNwpwBoAlgorithm_unspecified);
  assert_true(bo_fake_mass == 0.0);
  assert_int_equal(has_scaling, 0);

  nwchemc_params_release(&arena);
  free(message);
}

static void test_parser_renders_brillouin_tetrahedron(void **state) {
  (void)state;

  size_t message_size = 0;
  unsigned char *message =
      read_file(g_brillouin_tetrahedron_params_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  char full_blocks[NWCHEMC_BLOCKS];
  char embed_blocks[NWCHEMC_BLOCKS];
  assert_int_equal(nwchemc_params_render_input_blocks(
                       params_root, full_blocks, sizeof(full_blocks)),
                   0);
  assert_non_null(strstr(full_blocks, "  tetrahedron 4 5 6 tetraA\n"));
  assert_int_equal(nwchemc_params_render_embed_input_blocks(
                       params_root, embed_blocks, sizeof(embed_blocks)),
                   0);
  assert_null(strstr(embed_blocks, "  tetrahedron 4 5 6 tetraA\n"));

  nwchemc_params_release(&arena);
  free(message);
}

static void test_parser_renders_brillouin_dos_grid(void **state) {
  (void)state;

  size_t message_size = 0;
  unsigned char *message =
      read_file(g_brillouin_dos_grid_params_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  char full_blocks[NWCHEMC_BLOCKS];
  char embed_blocks[NWCHEMC_BLOCKS];
  assert_int_equal(nwchemc_params_render_input_blocks(
                       params_root, full_blocks, sizeof(full_blocks)),
                   0);
  assert_non_null(strstr(full_blocks, "  dos-grid 7 8 9 dosA\n"));
  assert_int_equal(nwchemc_params_render_embed_input_blocks(
                       params_root, embed_blocks, sizeof(embed_blocks)),
                   0);
  assert_null(strstr(embed_blocks, "  dos-grid 7 8 9 dosA\n"));

  nwchemc_params_release(&arena);
  free(message);
}

static void test_parser_renders_brillouin_dos_grid_default(void **state) {
  (void)state;

  size_t message_size = 0;
  unsigned char *message =
      read_file(g_brillouin_dos_grid_default_params_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  char full_blocks[NWCHEMC_BLOCKS];
  char embed_blocks[NWCHEMC_BLOCKS];
  assert_int_equal(nwchemc_params_render_input_blocks(
                       params_root, full_blocks, sizeof(full_blocks)),
                   0);
  assert_non_null(
      strstr(full_blocks, "  dos-grid 7 2 2 structure_default\n"));
  assert_int_equal(nwchemc_params_render_embed_input_blocks(
                       params_root, embed_blocks, sizeof(embed_blocks)),
                   0);
  assert_null(strstr(embed_blocks, "  dos-grid 7 2 2 structure_default\n"));

  int has_dos_grid = 0;
  int dos_grid[3] = {0, 0, 0};
  assert_int_equal(nwchemc_params_extract_direct_brillouin_dos_grid(
                       params_root, &has_dos_grid, dos_grid),
                   0);
  assert_int_equal(has_dos_grid, 1);
  assert_int_equal(dos_grid[0], 7);
  assert_int_equal(dos_grid[1], 2);
  assert_int_equal(dos_grid[2], 2);

  nwchemc_params_release(&arena);
  free(message);
}

static void test_parser_extracts_direct_nwpw_et(void **state) {
  (void)state;

  size_t message_size = 0;
  unsigned char *message = read_file(g_nwpw_et_params_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  char full_blocks[NWCHEMC_BLOCKS];
  char embed_blocks[NWCHEMC_BLOCKS];
  assert_int_equal(nwchemc_params_render_input_blocks(
                       params_root, full_blocks, sizeof(full_blocks)),
                   0);
  assert_non_null(
      strstr(full_blocks,
             "  et movecs_a.dat movecs_b.dat ion_a.xyz ion_b.xyz\n"));
  assert_int_equal(nwchemc_params_render_embed_input_blocks(
                       params_root, embed_blocks, sizeof(embed_blocks)),
                   0);
  assert_null(strstr(embed_blocks, "  et movecs_a.dat"));

  nwchemc_params_release(&arena);
  free(message);
}

static void test_parser_extracts_direct_nwpw_temperature(void **state) {
  (void)state;

  size_t message_size = 0;
  unsigned char *message =
      read_file(g_nwpw_temperature_params_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  char full_blocks[NWCHEMC_BLOCKS];
  char embed_blocks[NWCHEMC_BLOCKS];
  assert_int_equal(nwchemc_params_render_input_blocks(
                       params_root, full_blocks, sizeof(full_blocks)),
                   0);
  assert_non_null(
      strstr(full_blocks, "  temperature 400 1200 350 900 start 4 5\n"));
  assert_int_equal(nwchemc_params_render_embed_input_blocks(
                       params_root, embed_blocks, sizeof(embed_blocks)),
                   0);
  assert_null(strstr(embed_blocks, "  temperature 400"));

  int has_nose = 0;
  int nose_hoover = 0;
  int nose_restart = 0;
  double electron_period = 0.0;
  double electron_temperature = 0.0;
  double ion_period = 0.0;
  double ion_temperature = 0.0;
  int electron_chain_length = 0;
  int ion_chain_length = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_nose(
                       params_root, &has_nose, &nose_hoover, &nose_restart,
                       &electron_period, &electron_temperature, &ion_period,
                       &ion_temperature, &electron_chain_length,
                       &ion_chain_length),
                   0);
  assert_int_equal(has_nose, 1);
  assert_int_equal(nose_hoover, NWChemNwpwToggle_enabled);
  assert_int_equal(nose_restart, NWChemNwpwToggle_disabled);
  assert_true(electron_period > 899.999);
  assert_true(electron_period < 900.001);
  assert_true(electron_temperature > 349.999);
  assert_true(electron_temperature < 350.001);
  assert_true(ion_period > 1199.999);
  assert_true(ion_period < 1200.001);
  assert_true(ion_temperature > 399.999);
  assert_true(ion_temperature < 400.001);
  assert_int_equal(electron_chain_length, 5);
  assert_int_equal(ion_chain_length, 4);

  nwchemc_params_release(&arena);
  free(message);
}

static void test_parser_extracts_direct_nwpw_mapping_alias(void **state) {
  (void)state;

  size_t message_size = 0;
  unsigned char *message =
      read_file(g_nwpw_mapping_alias_params_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  char full_blocks[NWCHEMC_BLOCKS];
  char embed_blocks[NWCHEMC_BLOCKS];
  assert_int_equal(nwchemc_params_render_input_blocks(
                       params_root, full_blocks, sizeof(full_blocks)),
                   0);
  assert_non_null(strstr(full_blocks, "  2d-hcurve\n"));
  assert_int_equal(nwchemc_params_render_embed_input_blocks(
                       params_root, embed_blocks, sizeof(embed_blocks)),
                   0);
  assert_null(strstr(embed_blocks, "  2d-hcurve\n"));

  int has_cell_mapping = 0;
  int cell_expand[3] = {0, 0, 0};
  int mapping = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_cell_mapping(
                       params_root, &has_cell_mapping, cell_expand, &mapping),
                   0);
  assert_int_equal(has_cell_mapping, 1);
  assert_int_equal(cell_expand[0], 0);
  assert_int_equal(cell_expand[1], 0);
  assert_int_equal(cell_expand[2], 0);
  assert_int_equal(mapping, 3);

  nwchemc_params_release(&arena);
  free(message);
}

static void test_parser_extracts_direct_nwpw_mapping_default(void **state) {
  (void)state;

  size_t message_size = 0;
  unsigned char *message =
      read_file(g_nwpw_mapping_default_params_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  char full_blocks[NWCHEMC_BLOCKS];
  char embed_blocks[NWCHEMC_BLOCKS];
  assert_int_equal(nwchemc_params_render_input_blocks(
                       params_root, full_blocks, sizeof(full_blocks)),
                   0);
  assert_non_null(strstr(full_blocks, "  mapping 1\n"));
  assert_int_equal(nwchemc_params_render_embed_input_blocks(
                       params_root, embed_blocks, sizeof(embed_blocks)),
                   0);
  assert_null(strstr(embed_blocks, "  mapping 1\n"));

  int has_cell_mapping = 0;
  int cell_expand[3] = {0, 0, 0};
  int mapping = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_cell_mapping(
                       params_root, &has_cell_mapping, cell_expand, &mapping),
                   0);
  assert_int_equal(has_cell_mapping, 1);
  assert_int_equal(cell_expand[0], 0);
  assert_int_equal(cell_expand[1], 0);
  assert_int_equal(cell_expand[2], 0);
  assert_int_equal(mapping, 1);

  nwchemc_params_release(&arena);
  free(message);
}

static void test_parser_extracts_direct_nwpw_virtual_alias(void **state) {
  (void)state;

  size_t message_size = 0;
  unsigned char *message =
      read_file(g_nwpw_virtual_alias_params_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  char full_blocks[NWCHEMC_BLOCKS];
  char embed_blocks[NWCHEMC_BLOCKS];
  assert_int_equal(nwchemc_params_render_input_blocks(
                       params_root, full_blocks, sizeof(full_blocks)),
                   0);
  assert_non_null(strstr(full_blocks, "  virtual 5 5\n"));
  assert_int_equal(nwchemc_params_render_embed_input_blocks(
                       params_root, embed_blocks, sizeof(embed_blocks)),
                   0);
  assert_null(strstr(embed_blocks, "  virtual 5 5\n"));

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
  assert_int_equal(virtual_orbitals_start, 5);
  assert_int_equal(virtual_orbitals_end, 5);

  nwchemc_params_release(&arena);
  free(message);
}

static void
test_parser_extracts_direct_nwpw_one_electron_guess_defaults(void **state) {
  (void)state;

  size_t message_size = 0;
  unsigned char *message = read_file(
      g_nwpw_one_electron_guess_defaults_params_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  char full_blocks[NWCHEMC_BLOCKS];
  char embed_blocks[NWCHEMC_BLOCKS];
  assert_int_equal(nwchemc_params_render_input_blocks(
                       params_root, full_blocks, sizeof(full_blocks)),
                   0);
  assert_non_null(strstr(full_blocks, "  one_electron_guess 50 1 1\n"));
  assert_int_equal(nwchemc_params_render_embed_input_blocks(
                       params_root, embed_blocks, sizeof(embed_blocks)),
                   0);
  assert_null(strstr(embed_blocks, "  one_electron_guess"));

  int has_one_electron_guess = 0;
  int it_in = 0;
  int it_out = 0;
  int it_ortho = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_one_electron_guess(
                       params_root, &has_one_electron_guess, &it_in, &it_out,
                       &it_ortho),
                   0);
  assert_int_equal(has_one_electron_guess, 1);
  assert_int_equal(it_in, 50);
  assert_int_equal(it_out, 1);
  assert_int_equal(it_ortho, 1);

  nwchemc_params_release(&arena);
  free(message);
}

static void
test_parser_extracts_direct_nwpw_fractional_orbitals_default(void **state) {
  (void)state;

  size_t message_size = 0;
  unsigned char *message = read_file(
      g_nwpw_fractional_orbitals_default_params_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  char full_blocks[NWCHEMC_BLOCKS];
  char embed_blocks[NWCHEMC_BLOCKS];
  assert_int_equal(nwchemc_params_render_input_blocks(
                       params_root, full_blocks, sizeof(full_blocks)),
                   0);
  assert_non_null(strstr(full_blocks, "  fractional_orbitals 6 6\n"));
  assert_int_equal(nwchemc_params_render_embed_input_blocks(
                       params_root, embed_blocks, sizeof(embed_blocks)),
                   0);
  assert_null(strstr(embed_blocks, "  fractional_orbitals"));

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
  assert_int_equal(fractional_orbitals_start, 6);
  assert_int_equal(fractional_orbitals_end, 6);

  nwchemc_params_release(&arena);
  free(message);
}

static void
test_parser_extracts_direct_nwpw_smear_orbitals_default(void **state) {
  (void)state;

  size_t message_size = 0;
  unsigned char *message =
      read_file(g_nwpw_smear_orbitals_default_params_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  char full_blocks[NWCHEMC_BLOCKS];
  char embed_blocks[NWCHEMC_BLOCKS];
  assert_int_equal(nwchemc_params_render_input_blocks(
                       params_root, full_blocks, sizeof(full_blocks)),
                   0);
  assert_non_null(strstr(full_blocks, "  fractional_orbitals 9 9\n"));
  assert_non_null(strstr(full_blocks,
                         "  smear temperature 0.03 orbitals 9 9\n"));
  assert_int_equal(nwchemc_params_render_embed_input_blocks(
                       params_root, embed_blocks, sizeof(embed_blocks)),
                   0);
  assert_null(strstr(embed_blocks, "  fractional_orbitals"));
  assert_null(strstr(embed_blocks, "  smear"));

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
  assert_int_equal(fractional_orbitals_start, 9);
  assert_int_equal(fractional_orbitals_end, 9);
  assert_int_equal(has_smear, 1);
  assert_true(smear_temperature > 0.0299);
  assert_true(smear_temperature < 0.0301);

  nwchemc_params_release(&arena);
  free(message);
}

static void
test_parser_extracts_direct_nwpw_virtual_orbitals_default(void **state) {
  (void)state;

  size_t message_size = 0;
  unsigned char *message = read_file(
      g_nwpw_virtual_orbitals_default_params_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  char full_blocks[NWCHEMC_BLOCKS];
  char embed_blocks[NWCHEMC_BLOCKS];
  assert_int_equal(nwchemc_params_render_input_blocks(
                       params_root, full_blocks, sizeof(full_blocks)),
                   0);
  assert_non_null(strstr(full_blocks, "  virtual 8 8\n"));
  assert_int_equal(nwchemc_params_render_embed_input_blocks(
                       params_root, embed_blocks, sizeof(embed_blocks)),
                   0);
  assert_null(strstr(embed_blocks, "  virtual 8 8\n"));

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
  assert_int_equal(virtual_orbitals_start, 8);
  assert_int_equal(virtual_orbitals_end, 8);

  nwchemc_params_release(&arena);
  free(message);
}

static void
test_parser_extracts_direct_nwpw_translate_vector_default(void **state) {
  (void)state;

  size_t message_size = 0;
  unsigned char *message = read_file(
      g_nwpw_translate_vector_default_params_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  char full_blocks[NWCHEMC_BLOCKS];
  char embed_blocks[NWCHEMC_BLOCKS];
  assert_int_equal(nwchemc_params_render_input_blocks(
                       params_root, full_blocks, sizeof(full_blocks)),
                   0);
  assert_non_null(
      strstr(full_blocks, "  translate_vector 0.25 0.25 0.25\n"));
  assert_int_equal(nwchemc_params_render_embed_input_blocks(
                       params_root, embed_blocks, sizeof(embed_blocks)),
                   0);
  assert_null(strstr(embed_blocks, "  translate_vector 0.25"));

  int has_translate_vector = 0;
  double translate_vector[3] = {0.0, 0.0, 0.0};
  capn_text translate_geometry_name = {0};
  int translate_reorder = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_translate_vector(
                       params_root, &has_translate_vector, translate_vector,
                       &translate_geometry_name, &translate_reorder),
                   0);
  assert_int_equal(has_translate_vector, 1);
  assert_true(translate_vector[0] > 0.249);
  assert_true(translate_vector[0] < 0.251);
  assert_true(translate_vector[1] > 0.249);
  assert_true(translate_vector[1] < 0.251);
  assert_true(translate_vector[2] > 0.249);
  assert_true(translate_vector[2] < 0.251);
  assert_int_equal(translate_geometry_name.len, 0);
  assert_int_equal(translate_reorder, NWChemNwpwToggle_unspecified);

  nwchemc_params_release(&arena);
  free(message);
}

static void test_parser_renders_direct_nwpw_cell_expand_default(void **state) {
  (void)state;

  size_t message_size = 0;
  unsigned char *message =
      read_file(g_nwpw_cell_expand_default_params_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  char full_blocks[NWCHEMC_BLOCKS];
  char embed_blocks[NWCHEMC_BLOCKS];
  assert_int_equal(nwchemc_params_render_input_blocks(
                       params_root, full_blocks, sizeof(full_blocks)),
                   0);
  assert_non_null(strstr(full_blocks, "  expand_cell 1 3 1\n"));
  assert_int_equal(nwchemc_params_render_embed_input_blocks(
                       params_root, embed_blocks, sizeof(embed_blocks)),
                   0);
  assert_null(strstr(embed_blocks, "  expand_cell 1 3 1\n"));

  int has_cell_mapping = 0;
  int cell_expand[3] = {0, 0, 0};
  int mapping = 0;
  assert_int_equal(nwchemc_params_extract_direct_nwpw_cell_mapping(
                       params_root, &has_cell_mapping, cell_expand, &mapping),
                   0);
  assert_int_equal(has_cell_mapping, 1);
  assert_int_equal(cell_expand[0], 1);
  assert_int_equal(cell_expand[1], 3);
  assert_int_equal(cell_expand[2], 1);
  assert_int_equal(mapping, 0);

  nwchemc_params_release(&arena);
  free(message);
}

static void test_parser_renders_brillouin_monkhorst_default(void **state) {
  (void)state;

  size_t message_size = 0;
  unsigned char *message =
      read_file(g_brillouin_monkhorst_default_params_path, &message_size);
  assert_non_null(message);

  struct capn arena;
  NWChemParams_ptr params_root;
  assert_int_equal(
      nwchemc_params_root(message, message_size, &arena, &params_root), 0);

  char full_blocks[NWCHEMC_BLOCKS];
  char embed_blocks[NWCHEMC_BLOCKS];
  assert_int_equal(nwchemc_params_render_input_blocks(
                       params_root, full_blocks, sizeof(full_blocks)),
                   0);
  assert_non_null(
      strstr(full_blocks, "  monkhorst-pack 3 1 1 zone_default\n"));
  assert_int_equal(nwchemc_params_render_embed_input_blocks(
                       params_root, embed_blocks, sizeof(embed_blocks)),
                   0);
  assert_null(strstr(embed_blocks, "  monkhorst-pack 3 1 1 zone_default\n"));

  int has_brillouin_zone = 0;
  capn_text brillouin_zone_name = {0};
  int monkhorst_pack[3] = {0, 0, 0};
  int max_kpoints_print = 0;
  size_t brillouin_kvector_count = 0;
  assert_int_equal(nwchemc_params_extract_direct_brillouin_zone(
                       params_root, &has_brillouin_zone,
                       &brillouin_zone_name, monkhorst_pack,
                       &max_kpoints_print, NULL, 0, &brillouin_kvector_count),
                   0);
  assert_int_equal(has_brillouin_zone, 1);
  assert_int_equal(brillouin_zone_name.len, 0);
  assert_int_equal(monkhorst_pack[0], 3);
  assert_int_equal(monkhorst_pack[1], 1);
  assert_int_equal(monkhorst_pack[2], 1);
  assert_int_equal(max_kpoints_print, 0);
  assert_int_equal((int)brillouin_kvector_count, 0);

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

  int has_uterm = 0;
  int uterm_enabled = 0;
  int uterm_count = 0;
  assert_int_equal(nwchemc_params_extract_direct_pseudopotential_uterm(
                       params_root, &has_uterm, &uterm_enabled,
                       &uterm_count),
                   0);
  assert_int_equal(has_uterm, 1);
  assert_int_equal(uterm_enabled, 1);
  assert_int_equal(uterm_count, 1);

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
  if (argc != 28) {
    fprintf(stderr,
            "usage: %s PARAMS_BIN NWPW_SPIN_MODE_PARAMS_BIN "
            "NWPW_ALLOW_TRANSLATION_PARAMS_BIN NWPW_CUTOFF_ALIAS_PARAMS_BIN "
            "NWPW_MC_STEPS_PARAMS_BIN NWPW_BO_STEPS_DEFAULT_PARAMS_BIN "
            "NWPW_BO_TIME_STEP_DEFAULT_PARAMS_BIN "
            "NWPW_BO_FAKE_MASS_DEFAULT_PARAMS_BIN "
            "NWPW_SCALING_DEFAULT_PARAMS_BIN "
            "NWPW_NP_DIMENSIONS_DEFAULT_PARAMS_BIN "
            "NWPW_TOLERANCES_DEFAULT_PARAMS_BIN "
            "NWPW_MC_STEPS_DEFAULT_PARAMS_BIN "
            "BRILLOUIN_TETRAHEDRON_PARAMS_BIN "
            "BRILLOUIN_DOS_GRID_PARAMS_BIN NWPW_ET_PARAMS_BIN "
            "NWPW_TEMPERATURE_PARAMS_BIN NWPW_MAPPING_ALIAS_PARAMS_BIN "
            "NWPW_MAPPING_DEFAULT_PARAMS_BIN "
            "NWPW_VIRTUAL_ALIAS_PARAMS_BIN "
            "NWPW_ONE_ELECTRON_GUESS_DEFAULTS_PARAMS_BIN "
            "NWPW_FRACTIONAL_ORBITALS_DEFAULT_PARAMS_BIN "
            "NWPW_SMEAR_ORBITALS_DEFAULT_PARAMS_BIN "
            "NWPW_VIRTUAL_ORBITALS_DEFAULT_PARAMS_BIN "
            "NWPW_TRANSLATE_VECTOR_DEFAULT_PARAMS_BIN "
            "NWPW_CELL_EXPAND_DEFAULT_PARAMS_BIN "
            "BRILLOUIN_MONKHORST_DEFAULT_PARAMS_BIN "
            "BRILLOUIN_DOS_GRID_DEFAULT_PARAMS_BIN\n",
            argv[0]);
    return 2;
  }
  g_params_path = argv[1];
  g_spin_mode_params_path = argv[2];
  g_allow_translation_params_path = argv[3];
  g_cutoff_alias_params_path = argv[4];
  g_mc_steps_params_path = argv[5];
  g_bo_steps_default_params_path = argv[6];
  g_bo_time_step_default_params_path = argv[7];
  g_bo_fake_mass_default_params_path = argv[8];
  g_scaling_default_params_path = argv[9];
  g_np_dimensions_default_params_path = argv[10];
  g_tolerances_default_params_path = argv[11];
  g_mc_steps_default_params_path = argv[12];
  g_brillouin_tetrahedron_params_path = argv[13];
  g_brillouin_dos_grid_params_path = argv[14];
  g_nwpw_et_params_path = argv[15];
  g_nwpw_temperature_params_path = argv[16];
  g_nwpw_mapping_alias_params_path = argv[17];
  g_nwpw_mapping_default_params_path = argv[18];
  g_nwpw_virtual_alias_params_path = argv[19];
  g_nwpw_one_electron_guess_defaults_params_path = argv[20];
  g_nwpw_fractional_orbitals_default_params_path = argv[21];
  g_nwpw_smear_orbitals_default_params_path = argv[22];
  g_nwpw_virtual_orbitals_default_params_path = argv[23];
  g_nwpw_translate_vector_default_params_path = argv[24];
  g_nwpw_cell_expand_default_params_path = argv[25];
  g_brillouin_monkhorst_default_params_path = argv[26];
  g_brillouin_dos_grid_default_params_path = argv[27];
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_parser_renders_structured_input),
      cmocka_unit_test(test_parser_extracts_direct_dft_options),
      cmocka_unit_test(test_parser_extracts_direct_nwpw_options),
      cmocka_unit_test(test_parser_extracts_direct_nwpw_spin_mode),
      cmocka_unit_test(test_parser_extracts_direct_nwpw_allow_translation),
      cmocka_unit_test(test_parser_extracts_direct_nwpw_cutoff_alias),
      cmocka_unit_test(test_parser_extracts_direct_nwpw_mc_steps),
      cmocka_unit_test(test_parser_extracts_direct_nwpw_bo_steps_default),
      cmocka_unit_test(
          test_parser_extracts_direct_nwpw_bo_time_step_default),
      cmocka_unit_test(
          test_parser_extracts_direct_nwpw_bo_fake_mass_default),
      cmocka_unit_test(test_parser_extracts_direct_nwpw_scaling_default),
      cmocka_unit_test(
          test_parser_extracts_direct_nwpw_np_dimensions_default),
      cmocka_unit_test(test_parser_extracts_direct_nwpw_tolerances_default),
      cmocka_unit_test(test_parser_extracts_direct_nwpw_mc_steps_default),
      cmocka_unit_test(test_parser_renders_brillouin_tetrahedron),
      cmocka_unit_test(test_parser_renders_brillouin_dos_grid),
      cmocka_unit_test(test_parser_renders_brillouin_dos_grid_default),
      cmocka_unit_test(test_parser_extracts_direct_nwpw_et),
      cmocka_unit_test(test_parser_extracts_direct_nwpw_temperature),
      cmocka_unit_test(test_parser_extracts_direct_nwpw_mapping_alias),
      cmocka_unit_test(test_parser_extracts_direct_nwpw_mapping_default),
      cmocka_unit_test(test_parser_extracts_direct_nwpw_virtual_alias),
      cmocka_unit_test(
          test_parser_extracts_direct_nwpw_one_electron_guess_defaults),
      cmocka_unit_test(
          test_parser_extracts_direct_nwpw_fractional_orbitals_default),
      cmocka_unit_test(
          test_parser_extracts_direct_nwpw_smear_orbitals_default),
      cmocka_unit_test(
          test_parser_extracts_direct_nwpw_virtual_orbitals_default),
      cmocka_unit_test(
          test_parser_extracts_direct_nwpw_translate_vector_default),
      cmocka_unit_test(test_parser_renders_direct_nwpw_cell_expand_default),
      cmocka_unit_test(test_parser_renders_brillouin_monkhorst_default),
      cmocka_unit_test(test_parser_extracts_direct_pseudopotentials),
      cmocka_unit_test(test_parser_walks_direct_pseudopotential_capnp_entries),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
