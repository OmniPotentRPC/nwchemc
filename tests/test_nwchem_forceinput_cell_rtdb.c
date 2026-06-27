#include "nwchemc.h"
#include "nwchemc_params.h"

#include <errno.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cmocka.h>

enum { MAX_FORCE_INPUT_ATOMS = 16 };

typedef struct ParsedForceInput {
  int n_atoms;
  int has_cell;
  double positions_ang[MAX_FORCE_INPUT_ATOMS * 3];
  int atomic_numbers[MAX_FORCE_INPUT_ATOMS];
  double cell_ang[9];
} ParsedForceInput;

static const char *g_force_input_ang_path = NULL;
static const char *g_force_input_bohr_path = NULL;

extern void nwchemc_test_forceinput_cell_rtdb(
    const int *n_atoms_a, const double *positions_ang_a,
    const int *atomic_numbers_a, const double *cell_ang_a,
    const int *has_cell_a, const double *expected_amatrix_a,
    const int *n_atoms_b, const double *positions_ang_b,
    const int *atomic_numbers_b, const double *cell_ang_b,
    const int *has_cell_b, const double *expected_amatrix_b, int *result);

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

static void parse_force_input_cell(const char *label,
                                   const char *force_input_path,
                                   ParsedForceInput *parsed) {
  size_t force_input_size = 0;
  unsigned char *force_input = read_file(force_input_path, &force_input_size);
  assert_non_null(force_input);

  struct capn arena;
  ForceInput_ptr root;
  if (nwchemc_force_input_root(force_input, force_input_size, &arena, &root) !=
      0) {
    free(force_input);
    fail_msg("%s ForceInput parse failed", label);
  }

  size_t n_atoms_size = 0;
  int has_cell = 0;
  if (nwchemc_force_input_atom_count(root, &n_atoms_size, &has_cell) != 0 ||
      n_atoms_size > MAX_FORCE_INPUT_ATOMS) {
    nwchemc_params_release(&arena);
    free(force_input);
    fail_msg("%s ForceInput atom count failed", label);
  }

  if (nwchemc_force_input_copy_geometry(
          root, parsed->positions_ang, parsed->atomic_numbers,
          MAX_FORCE_INPUT_ATOMS, parsed->cell_ang, &has_cell) != 0) {
    nwchemc_params_release(&arena);
    free(force_input);
    fail_msg("%s ForceInput geometry copy failed", label);
  }
  nwchemc_params_release(&arena);
  free(force_input);
  assert_int_equal(has_cell, 1);
  parsed->n_atoms = (int)n_atoms_size;
  parsed->has_cell = has_cell;
}

static void test_forceinput_box_reaches_geometry_rtdb(void **state) {
  (void)state;

  const double ang_to_bohr = 1.0 / 0.529177210903;
  const double expected_ang[9] = {
      11.0 * ang_to_bohr, 0.0, 0.0,
      0.0, 12.0 * ang_to_bohr, 0.0,
      0.0, 0.0, 13.0 * ang_to_bohr,
  };
  ParsedForceInput ang;
  parse_force_input_cell("angstrom ForceInput.box", g_force_input_ang_path,
                         &ang);

  const double expected_bohr[9] = {
      21.0, 0.0, 0.0,
      0.0, 22.0, 0.0,
      0.0, 0.0, 23.0,
  };
  ParsedForceInput bohr;
  parse_force_input_cell("bohr ForceInput.box", g_force_input_bohr_path,
                         &bohr);

  int probe_result = -1;
  nwchemc_test_forceinput_cell_rtdb(
      &ang.n_atoms, ang.positions_ang, ang.atomic_numbers, ang.cell_ang,
      &ang.has_cell, expected_ang, &bohr.n_atoms, bohr.positions_ang,
      bohr.atomic_numbers, bohr.cell_ang, &bohr.has_cell, expected_bohr,
      &probe_result);
  assert_int_equal(probe_result, 0);
}

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr,
            "usage: %s force-input-ang.bin force-input-bohr.bin\n",
            argv[0]);
    return 2;
  }
  g_force_input_ang_path = argv[1];
  g_force_input_bohr_path = argv[2];

  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_forceinput_box_reaches_geometry_rtdb),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
