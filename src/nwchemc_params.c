#include "nwchemc_params.h"

#include <limits.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static const double NWCHEMC_BOHR_TO_ANGSTROM = 0.529177210903;
static const double NWCHEMC_HARTREE_TO_EV = 27.211386245988;
static const double NWCHEMC_HARTREE_TO_J = 4.359744722206048e-18;
static const double NWCHEMC_AVOGADRO = 6.02214076e23;

static const capn_text empty_text = {0, "", 0};

const char *nwchemc_params_text_or(capn_text text, const char *fallback) {
  if (text.str && text.len > 0)
    return text.str;
  return fallback;
}

static int append_bytes(char *dst, size_t dst_size, const char *src,
                        size_t n) {
  size_t used = strlen(dst);
  if (n == 0)
    return 0;
  if (used + n >= dst_size)
    return -1;
  memcpy(dst + used, src, n);
  dst[used + n] = '\0';
  return 0;
}

static int append_text(char *dst, size_t dst_size, capn_text text) {
  if (!text.str || text.len <= 0)
    return 0;
  return append_bytes(dst, dst_size, text.str, (size_t)text.len);
}

static int append_block(char *dst, size_t dst_size, const char *block) {
  size_t used = strlen(dst);
  size_t n = strlen(block);
  if (n == 0)
    return 0;
  if (used != 0) {
    if (used + 1 >= dst_size)
      return -1;
    dst[used++] = '\n';
    dst[used] = '\0';
  }
  return append_bytes(dst, dst_size, block, n);
}

static int append_format(char *dst, size_t dst_size, const char *fmt, ...) {
  size_t used = strlen(dst);
  if (used >= dst_size)
    return -1;
  va_list ap;
  va_start(ap, fmt);
  int n = vsnprintf(dst + used, dst_size - used, fmt, ap);
  va_end(ap);
  if (n < 0 || (size_t)n >= dst_size - used)
    return -1;
  return 0;
}

static int pointer_list_len(capn_ptr *ptr) {
  capn_resolve(ptr);
  if (ptr->type == CAPN_NULL)
    return 0;
  if (ptr->type != CAPN_PTR_LIST)
    return -1;
  return ptr->len;
}

static int struct_list_len(capn_ptr *ptr) {
  capn_resolve(ptr);
  if (ptr->type == CAPN_NULL)
    return 0;
  if (ptr->type != CAPN_LIST)
    return -1;
  return ptr->len;
}

static int list64_len(capn_list64 list) {
  capn_resolve(&list.p);
  if (list.p.type == CAPN_NULL)
    return 0;
  if (list.p.type != CAPN_LIST || list.p.datasz != 8)
    return -1;
  return list.p.len;
}

static int list32_len(capn_list32 list) {
  capn_resolve(&list.p);
  if (list.p.type == CAPN_NULL)
    return 0;
  if (list.p.type != CAPN_LIST || list.p.datasz != 4)
    return -1;
  return list.p.len;
}

static int text_equals_ascii_ci(capn_text text, const char *literal) {
  size_t i = 0;
  if (!literal)
    return 0;
  while (literal[i] != '\0')
    ++i;
  if (!text.str || text.len != (int)i)
    return 0;
  for (size_t j = 0; j < i; ++j) {
    char a = text.str[j];
    char b = literal[j];
    if (a >= 'A' && a <= 'Z')
      a = (char)(a - 'A' + 'a');
    if (b >= 'A' && b <= 'Z')
      b = (char)(b - 'A' + 'a');
    if (a != b)
      return 0;
  }
  return 1;
}

static int force_input_length_factor(capn_text unit, double *factor) {
  if (!factor)
    return -1;
  if (text_equals_ascii_ci(unit, "angstrom") ||
      text_equals_ascii_ci(unit, "angstroms") ||
      text_equals_ascii_ci(unit, "ang") || text_equals_ascii_ci(unit, "a")) {
    *factor = 1.0;
    return 0;
  }
  if (text_equals_ascii_ci(unit, "bohr") || text_equals_ascii_ci(unit, "au") ||
      text_equals_ascii_ci(unit, "a0") || text_equals_ascii_ci(unit, "a.u.")) {
    *factor = 0.529177210903;
    return 0;
  }
  if (text_equals_ascii_ci(unit, "nm") ||
      text_equals_ascii_ci(unit, "nanometer") ||
      text_equals_ascii_ci(unit, "nanometers")) {
    *factor = 10.0;
    return 0;
  }
  if (text_equals_ascii_ci(unit, "pm") ||
      text_equals_ascii_ci(unit, "picometer") ||
      text_equals_ascii_ci(unit, "picometers")) {
    *factor = 0.01;
    return 0;
  }
  return -1;
}

static int force_input_energy_factor(capn_text unit, double *factor) {
  if (!factor)
    return -1;
  if (!unit.str || unit.len <= 0 || text_equals_ascii_ci(unit, "eV") ||
      text_equals_ascii_ci(unit, "electronvolt") ||
      text_equals_ascii_ci(unit, "electronvolts")) {
    *factor = NWCHEMC_HARTREE_TO_EV;
    return 0;
  }
  if (text_equals_ascii_ci(unit, "hartree") || text_equals_ascii_ci(unit, "ha") ||
      text_equals_ascii_ci(unit, "au") || text_equals_ascii_ci(unit, "a.u.")) {
    *factor = 1.0;
    return 0;
  }
  if (text_equals_ascii_ci(unit, "meV")) {
    *factor = NWCHEMC_HARTREE_TO_EV * 1000.0;
    return 0;
  }
  if (text_equals_ascii_ci(unit, "ry") || text_equals_ascii_ci(unit, "rydberg")) {
    *factor = 2.0;
    return 0;
  }
  if (text_equals_ascii_ci(unit, "j") || text_equals_ascii_ci(unit, "joule") ||
      text_equals_ascii_ci(unit, "joules")) {
    *factor = NWCHEMC_HARTREE_TO_J;
    return 0;
  }
  if (text_equals_ascii_ci(unit, "kJ") || text_equals_ascii_ci(unit, "kilojoule") ||
      text_equals_ascii_ci(unit, "kilojoules")) {
    *factor = NWCHEMC_HARTREE_TO_J / 1000.0;
    return 0;
  }
  if (text_equals_ascii_ci(unit, "kJ/mol") ||
      text_equals_ascii_ci(unit, "kilojoule/mol") ||
      text_equals_ascii_ci(unit, "kilojoules/mol")) {
    *factor = NWCHEMC_HARTREE_TO_J * NWCHEMC_AVOGADRO / 1000.0;
    return 0;
  }
  if (text_equals_ascii_ci(unit, "kcal")) {
    *factor = NWCHEMC_HARTREE_TO_J / 4184.0;
    return 0;
  }
  if (text_equals_ascii_ci(unit, "kcal/mol")) {
    *factor = NWCHEMC_HARTREE_TO_J * NWCHEMC_AVOGADRO / 4184.0;
    return 0;
  }
  return -1;
}

static int append_text_args(capn_ptr args, char *dst, size_t dst_size) {
  int n = pointer_list_len(&args);
  if (n < 0)
    return -1;
  for (int i = 0; i < n; ++i) {
    capn_text arg = capn_get_text(args, i, empty_text);
    if (arg.len > 0) {
      if (append_format(dst, dst_size, " ") != 0 ||
          append_text(dst, dst_size, arg) != 0)
        return -1;
    }
  }
  return 0;
}

static int render_directives(NWChemDirective_list directives, char *dst,
                             size_t dst_size, const char *indent) {
  int n = struct_list_len(&directives.p);
  if (n < 0)
    return -1;
  for (int i = 0; i < n; ++i) {
    struct NWChemDirective directive;
    get_NWChemDirective(&directive, directives, i);
    if (directive.keyword.len <= 0)
      continue;
    if (append_format(dst, dst_size, "%s", indent) != 0 ||
        append_text(dst, dst_size, directive.keyword) != 0 ||
        append_text_args(directive.args, dst, dst_size) != 0 ||
        append_format(dst, dst_size, "\n") != 0)
      return -1;
  }
  return 0;
}

static int directives_have_keywords(NWChemDirective_list directives) {
  int n = struct_list_len(&directives.p);
  if (n < 0)
    return -1;
  for (int i = 0; i < n; ++i) {
    struct NWChemDirective directive;
    get_NWChemDirective(&directive, directives, i);
    if (directive.keyword.len > 0)
      return 1;
  }
  return 0;
}

static int render_generic_stanza(NWChemGenericStanza_ptr ptr, char *dst,
                                 size_t dst_size) {
  if (ptr.p.type == CAPN_NULL)
    return 0;

  struct NWChemGenericStanza generic;
  char block[2048];
  block[0] = '\0';
  read_NWChemGenericStanza(&generic, ptr);
  if (generic.name.len <= 0)
    return 0;
  if (append_text(block, sizeof(block), generic.name) != 0 ||
      append_format(block, sizeof(block), "\n") != 0 ||
      render_directives(generic.directives, block, sizeof(block), "  ") != 0 ||
      append_format(block, sizeof(block), "end") != 0)
    return -1;
  return append_block(dst, dst_size, block);
}

static int render_set_stanza(NWChemSetDirective_ptr ptr, char *dst,
                             size_t dst_size) {
  if (ptr.p.type == CAPN_NULL)
    return 0;

  struct NWChemSetDirective set;
  char block[512];
  block[0] = '\0';
  read_NWChemSetDirective(&set, ptr);
  if (set.key.len <= 0)
    return 0;
  if (append_format(block, sizeof(block), "set ") != 0 ||
      append_text(block, sizeof(block), set.key) != 0)
    return -1;
  const char *type_literal = NULL;
  switch (set.valueType) {
  case NWChemSetDirective_ValueType_text:
    type_literal = "string";
    break;
  case NWChemSetDirective_ValueType_double:
    type_literal = "double";
    break;
  case NWChemSetDirective_ValueType_integer:
    type_literal = "integer";
    break;
  case NWChemSetDirective_ValueType_logical:
    type_literal = "logical";
    break;
  case NWChemSetDirective_ValueType_auto:
  default:
    break;
  }
  if (type_literal &&
      append_format(block, sizeof(block), " %s", type_literal) != 0)
    return -1;
  int nvalues = pointer_list_len(&set.values);
  if (nvalues < 0)
    return -1;
  if (nvalues > 0) {
    for (int i = 0; i < nvalues; ++i) {
      capn_text value = capn_get_text(set.values, i, empty_text);
      if (append_format(block, sizeof(block), " ") != 0 ||
          append_text(block, sizeof(block), value) != 0)
        return -1;
    }
  } else if (set.value.len > 0) {
    if (append_format(block, sizeof(block), " ") != 0 ||
        append_text(block, sizeof(block), set.value) != 0)
      return -1;
  }
  return append_block(dst, dst_size, block);
}

static int render_dft_stanza(NWChemDftStanza_ptr ptr, char *dst,
                             size_t dst_size, int include_direct_promoted) {
  if (ptr.p.type == CAPN_NULL)
    return 0;

  struct NWChemDftStanza dft;
  char block[2048];
  block[0] = '\0';
  read_NWChemDftStanza(&dft, ptr);
  int has_directives = directives_have_keywords(dft.directives);
  if (has_directives < 0)
    return -1;
  if (!include_direct_promoted && !has_directives)
    return 0;
  if (append_format(block, sizeof(block), "dft\n") != 0)
    return -1;
  if (include_direct_promoted && dft.direct &&
      append_format(block, sizeof(block), "  direct\n") != 0)
    return -1;
  if (include_direct_promoted && dft.smearing.p.type != CAPN_NULL) {
    struct NWChemDftSmearing smearing;
    read_NWChemDftSmearing(&smearing, dft.smearing);
    if (smearing.sigmaHartree != 0.0 &&
        append_format(block, sizeof(block), "  smear %.12g %s\n",
                      smearing.sigmaHartree,
                      smearing.mode == NWChemDftSmearing_Mode_nofixsz
                          ? "nofixsz"
                          : "fixsz") != 0)
      return -1;
  }
  if (include_direct_promoted && dft.xc.len > 0) {
    if (append_format(block, sizeof(block), "  xc ") != 0 ||
        append_text(block, sizeof(block), dft.xc) != 0 ||
        append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (render_directives(dft.directives, block, sizeof(block), "  ") != 0 ||
      append_format(block, sizeof(block), "end") != 0)
    return -1;
  return append_block(dst, dst_size, block);
}

static const char *module_name_literal(enum NWChemModuleName name) {
  switch (name) {
  case NWChemModuleName_basis:
    return "basis";
  case NWChemModuleName_bq:
    return "bq";
  case NWChemModuleName_ccsd:
    return "ccsd";
  case NWChemModuleName_cosmo:
    return "cosmo";
  case NWChemModuleName_dft:
    return "dft";
  case NWChemModuleName_dplot:
    return "dplot";
  case NWChemModuleName_drdy:
    return "drdy";
  case NWChemModuleName_driver:
    return "driver";
  case NWChemModuleName_esp:
    return "esp";
  case NWChemModuleName_etrans:
    return "etrans";
  case NWChemModuleName_geometry:
    return "geometry";
  case NWChemModuleName_gw:
    return "gw";
  case NWChemModuleName_hessian:
    return "hessian";
  case NWChemModuleName_mcscf:
    return "mcscf";
  case NWChemModuleName_md:
    return "md";
  case NWChemModuleName_mm:
    return "mm";
  case NWChemModuleName_mp2:
    return "mp2";
  case NWChemModuleName_ncc:
    return "ncc";
  case NWChemModuleName_nwpw:
    return "nwpw";
  case NWChemModuleName_property:
    return "property";
  case NWChemModuleName_python:
    return "python";
  case NWChemModuleName_qmd:
    return "qmd";
  case NWChemModuleName_qmmm:
    return "qmmm";
  case NWChemModuleName_rimp2:
    return "rimp2";
  case NWChemModuleName_rism:
    return "rism";
  case NWChemModuleName_scf:
    return "scf";
  case NWChemModuleName_selci:
    return "selci";
  case NWChemModuleName_smd:
    return "smd";
  case NWChemModuleName_tce:
    return "tce";
  case NWChemModuleName_vib:
    return "vib";
  case NWChemModuleName_vscf:
    return "vscf";
  case NWChemModuleName_xtb:
    return "xtb";
  case NWChemModuleName_analysis:
    return "analysis";
  case NWChemModuleName_argos:
    return "argos";
  case NWChemModuleName_argosDiana:
    return "argos_diana";
  case NWChemModuleName_argosPrep:
    return "argos_prep";
  case NWChemModuleName_argosPrepare:
    return "argos_prepare";
  case NWChemModuleName_band:
    return "band";
  case NWChemModuleName_bandDplot:
    return "band_dplot";
  case NWChemModuleName_brillouinZone:
    return "brillouin_zone";
  case NWChemModuleName_bsemol:
    return "bsemol";
  case NWChemModuleName_cckohn:
    return "cckohn";
  case NWChemModuleName_cellOptimize:
    return "cell_optimize";
  case NWChemModuleName_cgsd:
    return "cgsd";
  case NWChemModuleName_constraints:
    return "constraints";
  case NWChemModuleName_cpmd:
    return "cpmd";
  case NWChemModuleName_cpsd:
    return "cpsd";
  case NWChemModuleName_ddscf:
    return "ddscf";
  case NWChemModuleName_diana:
    return "diana";
  case NWChemModuleName_dimpar:
    return "dimpar";
  case NWChemModuleName_dimqm:
    return "dimqm";
  case NWChemModuleName_dk:
    return "dk";
  case NWChemModuleName_dmd:
    return "dmd";
  case NWChemModuleName_dntmc:
    return "dntmc";
  case NWChemModuleName_fractionalOccupations:
    return "frac_occ";
  case NWChemModuleName_freeze:
    return "freeze";
  case NWChemModuleName_intgrl:
    return "intgrl";
  case NWChemModuleName_mdXs:
    return "md_xs";
  case NWChemModuleName_mepgs:
    return "mepgs";
  case NWChemModuleName_metadynamics:
    return "metadynamics";
  case NWChemModuleName_modelpotential:
    return "modelpotential";
  case NWChemModuleName_neb:
    return "neb";
  case NWChemModuleName_occup:
    return "occup";
  case NWChemModuleName_prepare:
    return "prepare";
  case NWChemModuleName_pspFormatter:
    return "psp_formatter";
  case NWChemModuleName_pspGenerator:
    return "psp_generator";
  case NWChemModuleName_pspw:
    return "pspw";
  case NWChemModuleName_pspwDplot:
    return "pspw_dplot";
  case NWChemModuleName_pspwQmmm:
    return "pspw_qmmm";
  case NWChemModuleName_pspwWannier:
    return "pspw_wannier";
  case NWChemModuleName_qmdNamd:
    return "qmd_namd";
  case NWChemModuleName_raman:
    return "raman";
  case NWChemModuleName_rel:
    return "rel";
  case NWChemModuleName_rtTddft:
    return "rt_tddft";
  case NWChemModuleName_simulationCell:
    return "simulation_cell";
  case NWChemModuleName_string:
    return "string";
  case NWChemModuleName_tamd:
    return "tamd";
  case NWChemModuleName_task:
    return "task";
  case NWChemModuleName_taskShell:
    return "task_shell";
  case NWChemModuleName_tceMrcc:
    return "tce_mrcc";
  case NWChemModuleName_tddft:
    return "tddft";
  case NWChemModuleName_tddftGradient:
    return "tddft_grad";
  case NWChemModuleName_tropt:
    return "tropt";
  case NWChemModuleName_vibZone:
    return "vib_zone";
  case NWChemModuleName_waterPseudopotential:
    return "water_pseudopotential";
  case NWChemModuleName_x2c:
    return "x2c";
  case NWChemModuleName_zora:
    return "zora";
  case NWChemModuleName_custom:
  default:
    return NULL;
  }
}

static int render_module_stanza(NWChemModuleStanza_ptr ptr, char *dst,
                                size_t dst_size) {
  if (ptr.p.type == CAPN_NULL)
    return 0;

  struct NWChemModuleStanza module;
  char block[2048];
  block[0] = '\0';
  read_NWChemModuleStanza(&module, ptr);
  const char *name = module_name_literal(module.name);
  if (!name && module.customName.len <= 0)
    return 0;
  if (name) {
    if (append_format(block, sizeof(block), "%s\n", name) != 0)
      return -1;
  } else if (append_text(block, sizeof(block), module.customName) != 0 ||
             append_format(block, sizeof(block), "\n") != 0) {
    return -1;
  }
  if (render_directives(module.directives, block, sizeof(block), "  ") != 0 ||
      append_format(block, sizeof(block), "end") != 0)
    return -1;
  return append_block(dst, dst_size, block);
}

static const char *pseudopotential_library_type_literal(
    enum NWChemPseudopotentialEntry_LibraryType library_type) {
  switch (library_type) {
  case NWChemPseudopotentialEntry_LibraryType_pspwLibrary:
    return "pspw_library";
  case NWChemPseudopotentialEntry_LibraryType_pawLibrary:
    return "paw_library";
  case NWChemPseudopotentialEntry_LibraryType_cpi:
    return "cpi";
  case NWChemPseudopotentialEntry_LibraryType_teter:
    return "teter";
  case NWChemPseudopotentialEntry_LibraryType_library:
  default:
    return "library";
  }
}

static capn_text
pseudopotential_entry_target(const struct NWChemPseudopotentialEntry *entry) {
  static const capn_text all_elements = {1, "*", NULL};
  if (entry->allElements)
    return all_elements;
  return entry->element;
}

static int render_pseudopotential_entries(
    NWChemPseudopotentialEntry_list entries, char *dst, size_t dst_size) {
  int n = struct_list_len(&entries.p);
  if (n < 0)
    return -1;
  for (int i = 0; i < n; ++i) {
    struct NWChemPseudopotentialEntry entry;
    get_NWChemPseudopotentialEntry(&entry, entries, i);
    capn_text target = pseudopotential_entry_target(&entry);
    if (target.len <= 0 || entry.libraryName.len <= 0)
      continue;
    if (append_format(dst, dst_size, "    ") != 0 ||
        append_text(dst, dst_size, target) != 0 ||
        append_format(dst, dst_size, " %s ",
                      pseudopotential_library_type_literal(
                          entry.libraryType)) != 0 ||
        append_text(dst, dst_size, entry.libraryName) != 0 ||
        append_format(dst, dst_size, "\n") != 0)
      return -1;
  }
  return 0;
}

static int render_pseudopotential_stanza(NWChemPseudopotentialStanza_ptr ptr,
                                         char *dst, size_t dst_size,
                                         int include_direct_entries) {
  if (ptr.p.type == CAPN_NULL)
    return 0;

  struct NWChemPseudopotentialStanza pseudopotential;
  char block[4096];
  block[0] = '\0';
  read_NWChemPseudopotentialStanza(&pseudopotential, ptr);
  if (append_format(block, sizeof(block), "nwpw\n") != 0)
    return -1;
  if (include_direct_entries) {
    if (append_format(block, sizeof(block), "  pseudopotentials\n") != 0 ||
        render_pseudopotential_entries(pseudopotential.entries, block,
                                       sizeof(block)) != 0 ||
        append_format(block, sizeof(block), "  end\n") != 0)
      return -1;
  }
  if (render_directives(pseudopotential.directives, block, sizeof(block),
                        "  ") != 0)
    return -1;
  if (!include_direct_entries &&
      strcmp(block, "nwpw\n") == 0)
    return 0;
  if (append_format(block, sizeof(block), "end") != 0)
    return -1;
  return append_block(dst, dst_size, block);
}

static int render_scf_stanza(NWChemScfStanza_ptr ptr, char *dst,
                             size_t dst_size, int include_direct_promoted) {
  if (ptr.p.type == CAPN_NULL)
    return 0;

  struct NWChemScfStanza scf;
  char block[4096];
  block[0] = '\0';
  read_NWChemScfStanza(&scf, ptr);
  int has_directives = directives_have_keywords(scf.directives);
  if (has_directives < 0)
    return -1;
  int has_promoted = scf.maxiter > 0 || scf.thresh > 0.0 || scf.tol2e > 0.0;
  int has_remaining = scf.vectorsInput.len > 0 || scf.vectorsOutput.len > 0 ||
                      scf.noprint || has_directives ||
                      (include_direct_promoted && has_promoted);
  if (!has_remaining)
    return 0;
  if (append_format(block, sizeof(block), "scf\n") != 0)
    return -1;
  if (scf.vectorsInput.len > 0) {
    if (append_format(block, sizeof(block), "  vectors input ") != 0 ||
        append_text(block, sizeof(block), scf.vectorsInput) != 0 ||
        append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (scf.vectorsOutput.len > 0) {
    if (append_format(block, sizeof(block), "  vectors output ") != 0 ||
        append_text(block, sizeof(block), scf.vectorsOutput) != 0 ||
        append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (include_direct_promoted && scf.maxiter > 0 &&
      append_format(block, sizeof(block), "  maxiter %d\n", scf.maxiter) != 0)
    return -1;
  if (include_direct_promoted && scf.thresh > 0.0 &&
      append_format(block, sizeof(block), "  thresh %.12g\n", scf.thresh) != 0)
    return -1;
  if (include_direct_promoted && scf.tol2e > 0.0 &&
      append_format(block, sizeof(block), "  tol2e %.12g\n", scf.tol2e) != 0)
    return -1;
  if (scf.noprint && append_format(block, sizeof(block), "  noprint\n") != 0)
    return -1;
  if (render_directives(scf.directives, block, sizeof(block), "  ") != 0 ||
      append_format(block, sizeof(block), "end") != 0)
    return -1;
  return append_block(dst, dst_size, block);
}

static int render_ccsd_stanza(NWChemCcsdStanza_ptr ptr, char *dst,
                              size_t dst_size, int include_direct_promoted) {
  if (ptr.p.type == CAPN_NULL)
    return 0;

  struct NWChemCcsdStanza ccsd;
  char block[4096];
  block[0] = '\0';
  read_NWChemCcsdStanza(&ccsd, ptr);
  int has_directives = directives_have_keywords(ccsd.directives);
  if (has_directives < 0)
    return -1;
  int has_promoted =
      ccsd.maxiter > 0 || ccsd.thresh > 0.0 || ccsd.tol2e > 0.0 ||
      ccsd.iprt > 0 || ccsd.maxDiis > 0 || ccsd.frozenCore > 0 ||
      ccsd.frozenVirtual > 0 || ccsd.useDisk != NWChemToggle_unspecified ||
      ccsd.sameSpinScale > 0.0 || ccsd.oppositeSpinScale > 0.0;
  int has_remaining = has_directives || (include_direct_promoted && has_promoted);
  if (!has_remaining)
    return 0;
  if (append_format(block, sizeof(block), "ccsd\n") != 0)
    return -1;
  if (include_direct_promoted && ccsd.maxiter > 0 &&
      append_format(block, sizeof(block), "  maxiter %d\n", ccsd.maxiter) != 0)
    return -1;
  if (include_direct_promoted && ccsd.thresh > 0.0 &&
      append_format(block, sizeof(block), "  thresh %.12g\n", ccsd.thresh) != 0)
    return -1;
  if (include_direct_promoted && ccsd.tol2e > 0.0 &&
      append_format(block, sizeof(block), "  tol2e %.12g\n", ccsd.tol2e) != 0)
    return -1;
  if (include_direct_promoted && ccsd.iprt > 0 &&
      append_format(block, sizeof(block), "  iprt %d\n", ccsd.iprt) != 0)
    return -1;
  if (include_direct_promoted && ccsd.maxDiis > 0 &&
      append_format(block, sizeof(block), "  diisbas %d\n", ccsd.maxDiis) != 0)
    return -1;
  if (include_direct_promoted &&
      (ccsd.frozenCore > 0 || ccsd.frozenVirtual > 0)) {
    if (append_format(block, sizeof(block), "  freeze") != 0)
      return -1;
    if (ccsd.frozenCore > 0 &&
        append_format(block, sizeof(block), " %d", ccsd.frozenCore) != 0)
      return -1;
    if (ccsd.frozenVirtual > 0 &&
        append_format(block, sizeof(block), " virtual %d",
                      ccsd.frozenVirtual) != 0)
      return -1;
    if (append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (include_direct_promoted && ccsd.useDisk == NWChemToggle_disabled &&
      append_format(block, sizeof(block), "  nodisk\n") != 0)
    return -1;
  if (include_direct_promoted && ccsd.sameSpinScale > 0.0 &&
      append_format(block, sizeof(block), "  fss %.12g\n",
                    ccsd.sameSpinScale) != 0)
    return -1;
  if (include_direct_promoted && ccsd.oppositeSpinScale > 0.0 &&
      append_format(block, sizeof(block), "  fos %.12g\n",
                    ccsd.oppositeSpinScale) != 0)
    return -1;
  if (render_directives(ccsd.directives, block, sizeof(block), "  ") != 0 ||
      append_format(block, sizeof(block), "end") != 0)
    return -1;
  return append_block(dst, dst_size, block);
}

static const char *tce_reference_keyword(enum NWChemTceReference reference) {
  switch (reference) {
  case NWChemTceReference_dft:
    return "dft";
  case NWChemTceReference_hf:
    return "hf";
  case NWChemTceReference_scf:
    return "scf";
  case NWChemTceReference_unspecified:
  default:
    return NULL;
  }
}

static const char *
tce_two_electron_keyword(enum NWChemTceTwoElectronStorage storage) {
  switch (storage) {
  case NWChemTceTwoElectronStorage_orbital:
    return "2eorb";
  case NWChemTceTwoElectronStorage_spin:
    return "2espin";
  case NWChemTceTwoElectronStorage_default:
  case NWChemTceTwoElectronStorage_unspecified:
  default:
    return NULL;
  }
}

static const char *tce_io_keyword(enum NWChemTceIoAlgorithm io) {
  switch (io) {
  case NWChemTceIoAlgorithm_fortran:
    return "fortran";
  case NWChemTceIoAlgorithm_eaf:
    return "eaf";
  case NWChemTceIoAlgorithm_ga:
    return "ga";
  case NWChemTceIoAlgorithm_sf:
    return "sf";
  case NWChemTceIoAlgorithm_replicated:
    return "replicated";
  case NWChemTceIoAlgorithm_dra:
    return "dra";
  case NWChemTceIoAlgorithm_gaEaf:
    return "ga_eaf";
  case NWChemTceIoAlgorithm_unspecified:
  default:
    return NULL;
  }
}

static const char *tce_freeze_keyword(enum NWChemTceFreezeMode freeze_mode) {
  switch (freeze_mode) {
  case NWChemTceFreezeMode_atomic:
    return "freeze atomic";
  case NWChemTceFreezeMode_core:
    return "freeze core";
  case NWChemTceFreezeMode_coreAtomic:
    return "freeze core atomic";
  case NWChemTceFreezeMode_unspecified:
  default:
    return NULL;
  }
}

static capn_text tce_method_text(const struct NWChemTceStanza *tce) {
  if (tce->method.len > 0)
    return tce->method;
  if (tce->ccsdVariant.len > 0)
    return tce->ccsdVariant;
  return tce->model;
}

static int render_tce_stanza(NWChemTceStanza_ptr ptr, char *dst,
                             size_t dst_size, int include_direct_promoted) {
  if (ptr.p.type == CAPN_NULL)
    return 0;

  struct NWChemTceStanza tce;
  char block[8192];
  block[0] = '\0';
  read_NWChemTceStanza(&tce, ptr);
  int has_directives = directives_have_keywords(tce.directives);
  if (has_directives < 0)
    return -1;
  capn_text method = tce_method_text(&tce);
  const char *freeze_mode = tce_freeze_keyword(tce.freezeMode);
  int has_renderable_promoted =
      tce.reference != NWChemTceReference_unspecified ||
      tce.frozenCore > 0 || tce.frozenVirtual > 0 ||
      tce.model2e != NWChemTceTwoElectronStorage_unspecified ||
      method.len > 0 || tce.thresh > 0.0 || tce.levelShift > 0.0 ||
      tce.leftLevelShift > 0.0 || tce.levelShift2Alpha > 0.0 ||
      tce.levelShift2Beta > 0.0 || tce.levelShift3Alpha > 0.0 ||
      tce.levelShift3Beta > 0.0 || tce.maxiter > 0 ||
      tce.ioAlgorithm != NWChemTceIoAlgorithm_unspecified || tce.diis > 0 ||
      tce.diis2 > 0 || tce.diis3 > 0 || tce.eomSolver > 0 ||
      tce.hbarDimension > 0 || tce.nroots > 0 || tce.target > 0 ||
      tce.targetSymmetry.len > 0 ||
      tce.symmetry == NWChemToggle_enabled ||
      tce.densityMatrix == NWChemToggle_enabled || tce.multipole > 0 ||
      tce.fragment >= 0 || tce.recomputeFock != NWChemToggle_unspecified ||
      tce.activeOccupiedAlpha > 0 || tce.activeOccupiedBeta > 0 ||
      tce.activeVirtualAlpha > 0 || tce.activeVirtualBeta > 0 ||
      tce.activeOccupied > 0 || tce.activeUnoccupied > 0 ||
      tce.activeEnergyMin > 0.0 || tce.activeEnergyMax > 0.0 ||
      tce.activeExcitationLevel > 0 || tce.maxDiff > 0.0 ||
      tce.atomicTileSize > 0 || tce.split > 0 ||
      tce.twoElectronMethod > 0 || tce.diskBackend >= 0 ||
      tce.tileSize > 0 || tce.cudaDevices > 0 ||
      tce.tccSpaces == NWChemToggle_enabled ||
      tce.eaCcsd == NWChemToggle_enabled ||
      tce.ipCcsd == NWChemToggle_enabled;
  if (!has_directives && !freeze_mode &&
      !(include_direct_promoted && has_renderable_promoted))
    return 0;
  if (append_format(block, sizeof(block), "tce\n") != 0)
    return -1;
  if (freeze_mode &&
      append_format(block, sizeof(block), "  %s\n", freeze_mode) != 0)
    return -1;
  if (include_direct_promoted) {
    const char *reference = tce_reference_keyword(tce.reference);
    const char *model2e = tce_two_electron_keyword(tce.model2e);
    const char *io = tce_io_keyword(tce.ioAlgorithm);
    if (reference &&
        append_format(block, sizeof(block), "  %s\n", reference) != 0)
      return -1;
    if (!freeze_mode && (tce.frozenCore > 0 || tce.frozenVirtual > 0)) {
      if (append_format(block, sizeof(block), "  freeze") != 0)
        return -1;
      if (tce.frozenCore > 0 &&
          append_format(block, sizeof(block), " %d", tce.frozenCore) != 0)
        return -1;
      if (tce.frozenVirtual > 0 &&
          append_format(block, sizeof(block), " virtual %d",
                        tce.frozenVirtual) != 0)
        return -1;
      if (append_format(block, sizeof(block), "\n") != 0)
        return -1;
    }
    if (model2e &&
        append_format(block, sizeof(block), "  %s\n", model2e) != 0)
      return -1;
    if (method.len > 0 &&
        (append_format(block, sizeof(block), "  ") != 0 ||
         append_text(block, sizeof(block), method) != 0 ||
         append_format(block, sizeof(block), "\n") != 0))
      return -1;
    if (tce.thresh > 0.0 &&
        append_format(block, sizeof(block), "  thresh %.12g\n", tce.thresh) !=
            0)
      return -1;
    if (tce.levelShift > 0.0 &&
        append_format(block, sizeof(block), "  lshift %.12g\n",
                      tce.levelShift) != 0)
      return -1;
    if (tce.leftLevelShift > 0.0 &&
        append_format(block, sizeof(block), "  lshiftl %.12g\n",
                      tce.leftLevelShift) != 0)
      return -1;
    if ((tce.levelShift2Alpha > 0.0 || tce.levelShift2Beta > 0.0) &&
        append_format(block, sizeof(block), "  lshift2 %.12g %.12g\n",
                      tce.levelShift2Alpha, tce.levelShift2Beta) != 0)
      return -1;
    if ((tce.levelShift3Alpha > 0.0 || tce.levelShift3Beta > 0.0) &&
        append_format(block, sizeof(block), "  lshift3 %.12g %.12g\n",
                      tce.levelShift3Alpha, tce.levelShift3Beta) != 0)
      return -1;
    if (tce.maxiter > 0 &&
        append_format(block, sizeof(block), "  maxiter %d\n", tce.maxiter) !=
            0)
      return -1;
    if (io && append_format(block, sizeof(block), "  io %s\n", io) != 0)
      return -1;
    if (tce.eomSolver > 0 &&
        append_format(block, sizeof(block), "  eomsol %d\n",
                      tce.eomSolver) != 0)
      return -1;
    if (tce.diis > 0 &&
        append_format(block, sizeof(block), "  diis %d\n", tce.diis) != 0)
      return -1;
    if (tce.diis2 > 0 &&
        append_format(block, sizeof(block), "  diis2 %d\n", tce.diis2) != 0)
      return -1;
    if (tce.diis3 > 0 &&
        append_format(block, sizeof(block), "  diis3 %d\n", tce.diis3) != 0)
      return -1;
    if (tce.hbarDimension > 0 &&
        append_format(block, sizeof(block), "  hbard %d\n",
                      tce.hbarDimension) != 0)
      return -1;
    if (tce.nroots > 0 &&
        append_format(block, sizeof(block), "  nroots %d\n", tce.nroots) != 0)
      return -1;
    if (tce.target > 0 &&
        append_format(block, sizeof(block), "  target %d\n", tce.target) != 0)
      return -1;
    if (tce.targetSymmetry.len > 0 &&
        (append_format(block, sizeof(block), "  targetsym ") != 0 ||
         append_text(block, sizeof(block), tce.targetSymmetry) != 0 ||
         append_format(block, sizeof(block), "\n") != 0))
      return -1;
    if (tce.symmetry == NWChemToggle_enabled &&
        append_format(block, sizeof(block), "  symmetry\n") != 0)
      return -1;
    if (tce.densityMatrix == NWChemToggle_enabled) {
      if (append_format(block, sizeof(block), "  densmat") != 0)
        return -1;
      if (tce.densityMatrixFile.len > 0 &&
          (append_format(block, sizeof(block), " ") != 0 ||
           append_text(block, sizeof(block), tce.densityMatrixFile) != 0))
        return -1;
      if (append_format(block, sizeof(block), "\n") != 0)
        return -1;
    }
    if (tce.multipole > 0 &&
        append_format(block, sizeof(block), "  multipole %d\n",
                      tce.multipole) != 0)
      return -1;
    if (tce.fragment >= 0 &&
        append_format(block, sizeof(block), "  fragment %d\n", tce.fragment) !=
            0)
      return -1;
    if (tce.recomputeFock == NWChemToggle_enabled &&
        append_format(block, sizeof(block), "  fock\n") != 0)
      return -1;
    if (tce.recomputeFock == NWChemToggle_disabled &&
        append_format(block, sizeof(block), "  nofock\n") != 0)
      return -1;
    if (tce.activeOccupied > 0 &&
        append_format(block, sizeof(block), "  oact %d\n",
                      tce.activeOccupied) != 0)
      return -1;
    if (tce.activeUnoccupied > 0 &&
        append_format(block, sizeof(block), "  uact %d\n",
                      tce.activeUnoccupied) != 0)
      return -1;
    if (tce.activeEnergyMin > 0.0 &&
        append_format(block, sizeof(block), "  emin_act %.12g\n",
                      tce.activeEnergyMin) != 0)
      return -1;
    if (tce.activeEnergyMax > 0.0 &&
        append_format(block, sizeof(block), "  emax_act %.12g\n",
                      tce.activeEnergyMax) != 0)
      return -1;
    if (tce.activeOccupiedAlpha > 0 &&
        append_format(block, sizeof(block), "  active_oa %d\n",
                      tce.activeOccupiedAlpha) != 0)
      return -1;
    if (tce.activeOccupiedBeta > 0 &&
        append_format(block, sizeof(block), "  active_ob %d\n",
                      tce.activeOccupiedBeta) != 0)
      return -1;
    if (tce.activeVirtualAlpha > 0 &&
        append_format(block, sizeof(block), "  active_va %d\n",
                      tce.activeVirtualAlpha) != 0)
      return -1;
    if (tce.activeVirtualBeta > 0 &&
        append_format(block, sizeof(block), "  active_vb %d\n",
                      tce.activeVirtualBeta) != 0)
      return -1;
    if (tce.activeExcitationLevel > 0 &&
        append_format(block, sizeof(block), "  t3a_lvl %d\n",
                      tce.activeExcitationLevel) != 0)
      return -1;
    if (tce.maxDiff > 0.0 &&
        append_format(block, sizeof(block), "  maxdiff %.12g\n",
                      tce.maxDiff) != 0)
      return -1;
    if (tce.atomicTileSize > 0 &&
        append_format(block, sizeof(block), "  attilesize %d\n",
                      tce.atomicTileSize) != 0)
      return -1;
    if (tce.split > 0 &&
        append_format(block, sizeof(block), "  split %d\n", tce.split) != 0)
      return -1;
    if (tce.twoElectronMethod > 0 &&
        append_format(block, sizeof(block), "  2emet %d\n",
                      tce.twoElectronMethod) != 0)
      return -1;
    if (tce.diskBackend >= 0 &&
        append_format(block, sizeof(block), "  idiskx %d\n",
                      tce.diskBackend) != 0)
      return -1;
    if (tce.tileSize > 0 &&
        append_format(block, sizeof(block), "  tilesize %d\n",
                      tce.tileSize) != 0)
      return -1;
    if (tce.cudaDevices > 0 &&
        append_format(block, sizeof(block), "  cuda %d\n",
                      tce.cudaDevices) != 0)
      return -1;
    if (tce.tccSpaces == NWChemToggle_enabled &&
        append_format(block, sizeof(block), "  tcc_spaces\n") != 0)
      return -1;
    if (tce.eaCcsd == NWChemToggle_enabled &&
        append_format(block, sizeof(block), "  eaccsd\n") != 0)
      return -1;
    if (tce.ipCcsd == NWChemToggle_enabled &&
        append_format(block, sizeof(block), "  ipccsd\n") != 0)
      return -1;
  }
  if (render_directives(tce.directives, block, sizeof(block), "  ") != 0 ||
      append_format(block, sizeof(block), "end") != 0)
    return -1;
  return append_block(dst, dst_size, block);
}

static int render_mrcc_data_stanza(NWChemMrccDataStanza_ptr ptr, char *dst,
                                   size_t dst_size) {
  if (ptr.p.type == CAPN_NULL)
    return 0;

  struct NWChemMrccDataStanza mrcc;
  char block[4096];
  block[0] = '\0';
  read_NWChemMrccDataStanza(&mrcc, ptr);
  int nrefs = pointer_list_len(&mrcc.references);
  if (nrefs < 0)
    return -1;
  int has_directives = directives_have_keywords(mrcc.directives);
  if (has_directives < 0)
    return -1;
  int has_fields = mrcc.root > 0 || mrcc.casElectrons > 0 ||
                   mrcc.casOrbitals > 0 || mrcc.nref > 0 || nrefs > 0 ||
                   mrcc.se4t || mrcc.noAposteriori ||
                   mrcc.subgroupSize > 0 || mrcc.improveTiling || mrcc.usspt;
  if (!has_fields && !has_directives)
    return 0;

  if (append_format(block, sizeof(block), "mrccdata\n") != 0)
    return -1;
  if (mrcc.se4t && append_format(block, sizeof(block), "  se4t\n") != 0)
    return -1;
  if (mrcc.noAposteriori &&
      append_format(block, sizeof(block), "  no_aposteriori\n") != 0)
    return -1;
  if (mrcc.root > 0 &&
      append_format(block, sizeof(block), "  root %d\n", mrcc.root) != 0)
    return -1;
  if ((mrcc.casElectrons > 0 || mrcc.casOrbitals > 0) &&
      append_format(block, sizeof(block), "  cas %d %d\n",
                    mrcc.casElectrons, mrcc.casOrbitals) != 0)
    return -1;
  int emitted_nref = mrcc.nref > 0 ? mrcc.nref : nrefs;
  if (emitted_nref > 0 &&
      append_format(block, sizeof(block), "  nref %d\n", emitted_nref) != 0)
    return -1;
  for (int i = 0; i < nrefs; ++i) {
    capn_text reference = capn_get_text(mrcc.references, i, empty_text);
    if (reference.len <= 0)
      continue;
    if (append_format(block, sizeof(block), "  ") != 0 ||
        append_text(block, sizeof(block), reference) != 0 ||
        append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (mrcc.subgroupSize > 0 &&
      append_format(block, sizeof(block), "  subgroupsize %d\n",
                    mrcc.subgroupSize) != 0)
    return -1;
  if (mrcc.improveTiling &&
      append_format(block, sizeof(block), "  improvetiling\n") != 0)
    return -1;
  if (mrcc.usspt && append_format(block, sizeof(block), "  usspt\n") != 0)
    return -1;
  if (render_directives(mrcc.directives, block, sizeof(block), "  ") != 0 ||
      append_format(block, sizeof(block), "end") != 0)
    return -1;
  return append_block(dst, dst_size, block);
}

static int render_task_stanza(NWChemTaskStanza_ptr ptr, char *dst,
                              size_t dst_size) {
  if (ptr.p.type == CAPN_NULL)
    return 0;

  struct NWChemTaskStanza task;
  char block[512];
  block[0] = '\0';
  read_NWChemTaskStanza(&task, ptr);
  if (append_format(block, sizeof(block), "task") != 0)
    return -1;
  if (task.theory.len > 0) {
    if (append_format(block, sizeof(block), " ") != 0 ||
        append_text(block, sizeof(block), task.theory) != 0)
      return -1;
  }
  if (task.operation.len > 0) {
    if (append_format(block, sizeof(block), " ") != 0 ||
        append_text(block, sizeof(block), task.operation) != 0)
      return -1;
  }
  if (task.ignore && append_format(block, sizeof(block), " ignore") != 0)
    return -1;
  return append_block(dst, dst_size, block);
}

static int render_driver_stanza(NWChemDriverStanza_ptr ptr, char *dst,
                                size_t dst_size,
                                int include_direct_promoted) {
  if (ptr.p.type == CAPN_NULL)
    return 0;

  struct NWChemDriverStanza driver;
  char block[4096];
  block[0] = '\0';
  read_NWChemDriverStanza(&driver, ptr);
  int has_directives = directives_have_keywords(driver.directives);
  if (has_directives < 0)
    return -1;
  int has_promoted = driver.maxiter > 0 || driver.tight || driver.loose ||
                     driver.gmaxTol > 0.0 || driver.grmsTol > 0.0 ||
                     driver.xmaxTol > 0.0 || driver.xrmsTol > 0.0;
  int has_remaining = driver.xyz.len > 0 || has_directives ||
                      (include_direct_promoted && has_promoted);
  if (!has_remaining)
    return 0;
  if (append_format(block, sizeof(block), "driver\n") != 0)
    return -1;
  if (include_direct_promoted && driver.maxiter > 0 &&
      append_format(block, sizeof(block), "  maxiter %d\n", driver.maxiter) != 0)
    return -1;
  if (include_direct_promoted && driver.tight &&
      append_format(block, sizeof(block), "  tight\n") != 0)
    return -1;
  if (include_direct_promoted && driver.loose &&
      append_format(block, sizeof(block), "  loose\n") != 0)
    return -1;
  if (include_direct_promoted && driver.gmaxTol > 0.0 &&
      append_format(block, sizeof(block), "  gmax %.15g\n",
                    driver.gmaxTol) != 0)
    return -1;
  if (include_direct_promoted && driver.grmsTol > 0.0 &&
      append_format(block, sizeof(block), "  grms %.15g\n",
                    driver.grmsTol) != 0)
    return -1;
  if (include_direct_promoted && driver.xmaxTol > 0.0 &&
      append_format(block, sizeof(block), "  xmax %.15g\n",
                    driver.xmaxTol) != 0)
    return -1;
  if (include_direct_promoted && driver.xrmsTol > 0.0 &&
      append_format(block, sizeof(block), "  xrms %.15g\n",
                    driver.xrmsTol) != 0)
    return -1;
  if (driver.xyz.len > 0) {
    if (append_format(block, sizeof(block), "  xyz ") != 0 ||
        append_text(block, sizeof(block), driver.xyz) != 0 ||
        append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (render_directives(driver.directives, block, sizeof(block), "  ") != 0 ||
      append_format(block, sizeof(block), "end") != 0)
    return -1;
  return append_block(dst, dst_size, block);
}

static int render_property_stanza(NWChemPropertyStanza_ptr ptr, char *dst,
                                  size_t dst_size) {
  if (ptr.p.type == CAPN_NULL)
    return 0;

  struct NWChemPropertyStanza property;
  char block[4096];
  block[0] = '\0';
  read_NWChemPropertyStanza(&property, ptr);
  if (append_format(block, sizeof(block), "property\n") != 0)
    return -1;
  if (property.dipole &&
      append_format(block, sizeof(block), "  dipole\n") != 0)
    return -1;
  if (property.mulliken &&
      append_format(block, sizeof(block), "  mulliken\n") != 0)
    return -1;
  if (property.quadrupol &&
      append_format(block, sizeof(block), "  quadrupole\n") != 0)
    return -1;
  if (render_directives(property.directives, block, sizeof(block), "  ") != 0 ||
      append_format(block, sizeof(block), "end") != 0)
    return -1;
  return append_block(dst, dst_size, block);
}

static int render_basis_stanza(NWChemBasisStanza_ptr ptr, char *dst,
                               size_t dst_size) {
  if (ptr.p.type == CAPN_NULL)
    return 0;

  struct NWChemBasisStanza basis;
  char block[4096];
  block[0] = '\0';
  read_NWChemBasisStanza(&basis, ptr);
  if (append_format(block, sizeof(block), "basis") != 0)
    return -1;
  if (basis.spherical && append_format(block, sizeof(block), " spherical") != 0)
    return -1;
  if (append_format(block, sizeof(block), "\n") != 0)
    return -1;
  if (basis.segment.len > 0) {
    if (append_format(block, sizeof(block), "  * library ") != 0 ||
        append_text(block, sizeof(block), basis.segment) != 0 ||
        append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (render_directives(basis.directives, block, sizeof(block), "  ") != 0)
    return -1;
  if (basis.ecp.len > 0) {
    if (append_format(block, sizeof(block), "  ecp ") != 0 ||
        append_text(block, sizeof(block), basis.ecp) != 0 ||
        append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (append_format(block, sizeof(block), "end") != 0)
    return -1;
  return append_block(dst, dst_size, block);
}

static int render_geometry_stanza(NWChemGeometryStanza_ptr ptr, char *dst,
                                  size_t dst_size) {
  if (ptr.p.type == CAPN_NULL)
    return 0;

  struct NWChemGeometryStanza geometry;
  char block[4096];
  block[0] = '\0';
  read_NWChemGeometryStanza(&geometry, ptr);
  if (append_format(block, sizeof(block), "geometry") != 0)
    return -1;
  if (geometry.units.len > 0) {
    if (append_format(block, sizeof(block), " units ") != 0 ||
        append_text(block, sizeof(block), geometry.units) != 0)
      return -1;
  }
  if (geometry.noautosym &&
      append_format(block, sizeof(block), " noautosym") != 0)
    return -1;
  if (geometry.noautoz && append_format(block, sizeof(block), " noautoz") != 0)
    return -1;
  if (geometry.center && append_format(block, sizeof(block), " center") != 0)
    return -1;
  if (append_format(block, sizeof(block), "\n") != 0)
    return -1;
  if (geometry.symmetry.len > 0) {
    if (append_format(block, sizeof(block), "  symmetry ") != 0 ||
        append_text(block, sizeof(block), geometry.symmetry) != 0 ||
        append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (render_directives(geometry.directives, block, sizeof(block), "  ") != 0 ||
      append_format(block, sizeof(block), "end") != 0)
    return -1;
  return append_block(dst, dst_size, block);
}

static int render_nwpw_stanza(NWChemNwpwStanza_ptr ptr, char *dst,
                              size_t dst_size,
                              int include_direct_promoted) {
  if (ptr.p.type == CAPN_NULL)
    return 0;

  struct NWChemNwpwStanza nwpw;
  char block[4096];
  block[0] = '\0';
  read_NWChemNwpwStanza(&nwpw, ptr);
  if (append_format(block, sizeof(block), "nwpw\n") != 0)
    return -1;
  if (include_direct_promoted && nwpw.energyCutoff > 0.0 &&
      append_format(block, sizeof(block), "  energy_cutoff %.15g\n",
                    nwpw.energyCutoff) != 0)
    return -1;
  if (include_direct_promoted && nwpw.wavefunctionCutoff > 0.0 &&
      append_format(block, sizeof(block), "  wavefunction_cutoff %.15g\n",
                    nwpw.wavefunctionCutoff) != 0)
    return -1;
  if (include_direct_promoted && nwpw.ewaldRcut > 0.0 &&
      append_format(block, sizeof(block), "  ewald_rcut %.15g\n",
                    nwpw.ewaldRcut) != 0)
    return -1;
  if (include_direct_promoted && nwpw.ewaldNcut > 0 &&
      append_format(block, sizeof(block), "  ewald_ncut %d\n",
                    nwpw.ewaldNcut) != 0)
    return -1;
  if (include_direct_promoted && nwpw.cellName.len > 0) {
    if (append_format(block, sizeof(block), "  cell_name ") != 0 ||
        append_text(block, sizeof(block), nwpw.cellName) != 0 ||
        append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (include_direct_promoted && nwpw.inputWavefunctionFilename.len > 0) {
    if (append_format(block, sizeof(block),
                      "  input_wavefunction_filename ") != 0 ||
        append_text(block, sizeof(block), nwpw.inputWavefunctionFilename) !=
            0 ||
        append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (include_direct_promoted && nwpw.outputWavefunctionFilename.len > 0) {
    if (append_format(block, sizeof(block),
                      "  output_wavefunction_filename ") != 0 ||
        append_text(block, sizeof(block), nwpw.outputWavefunctionFilename) !=
            0 ||
        append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (include_direct_promoted && nwpw.fakeMass > 0.0 &&
      append_format(block, sizeof(block), "  fake_mass %.15g\n",
                    nwpw.fakeMass) != 0)
    return -1;
  if (include_direct_promoted && nwpw.timeStep > 0.0 &&
      append_format(block, sizeof(block), "  time_step %.15g\n",
                    nwpw.timeStep) != 0)
    return -1;
  if (include_direct_promoted && nwpw.loopStart > 0 && nwpw.loopEnd > 0 &&
      append_format(block, sizeof(block), "  loop %d %d\n",
                    nwpw.loopStart, nwpw.loopEnd) != 0)
    return -1;
  if (include_direct_promoted &&
      (nwpw.toleranceEnergy > 0.0 || nwpw.toleranceDensity > 0.0 ||
       nwpw.toleranceGradient > 0.0)) {
    double tol_energy =
        nwpw.toleranceEnergy > 0.0 ? nwpw.toleranceEnergy : 1.0e-7;
    double tol_density =
        nwpw.toleranceDensity > 0.0 ? nwpw.toleranceDensity : tol_energy;
    double tol_gradient =
        nwpw.toleranceGradient > 0.0 ? nwpw.toleranceGradient : 1.0e-4;
    if (append_format(block, sizeof(block), "  tolerances %.15g %.15g %.15g\n",
                      tol_energy, tol_density, tol_gradient) != 0)
      return -1;
  }
  if (nwpw.exchangeCorrelation.len > 0 &&
      (include_direct_promoted ||
       text_equals_ascii_ci(nwpw.exchangeCorrelation, "new"))) {
    if (append_format(block, sizeof(block), "  exchange_correlation ") != 0 ||
        append_text(block, sizeof(block), nwpw.exchangeCorrelation) != 0 ||
        append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (include_direct_promoted &&
      nwpw.balanceMode == NWChemNwpwBalanceMode_balance &&
      append_format(block, sizeof(block), "  balance\n") != 0)
    return -1;
  if (include_direct_promoted &&
      nwpw.balanceMode == NWChemNwpwBalanceMode_nobalance &&
      append_format(block, sizeof(block), "  nobalance\n") != 0)
    return -1;
  if (include_direct_promoted && nwpw.boStepStart > 0 &&
      nwpw.boStepEnd > 0 &&
      append_format(block, sizeof(block), "  bo_steps %d %d\n",
                    nwpw.boStepStart, nwpw.boStepEnd) != 0)
    return -1;
  if (include_direct_promoted && nwpw.boTimeStep > 0.0 &&
      append_format(block, sizeof(block), "  bo_time_step %.15g\n",
                    nwpw.boTimeStep) != 0)
    return -1;
  if (include_direct_promoted &&
      nwpw.boAlgorithm != NWChemNwpwBoAlgorithm_unspecified) {
    const char *algorithm = NULL;
    if (nwpw.boAlgorithm == NWChemNwpwBoAlgorithm_verlet)
      algorithm = "verlet";
    else if (nwpw.boAlgorithm == NWChemNwpwBoAlgorithm_velocityVerlet)
      algorithm = "velocity-verlet";
    else if (nwpw.boAlgorithm == NWChemNwpwBoAlgorithm_leapFrog)
      algorithm = "leap-frog";
    if (!algorithm ||
        append_format(block, sizeof(block), "  bo_algorithm %s\n",
                      algorithm) != 0)
      return -1;
  }
  if (include_direct_promoted && nwpw.boFakeMass > 0.0 &&
      append_format(block, sizeof(block), "  bo_fake_mass %.15g\n",
                    nwpw.boFakeMass) != 0)
    return -1;
  if (include_direct_promoted &&
      (nwpw.scalingFirst > 0.0 || nwpw.scalingSecond > 0.0)) {
    double scaling_first = nwpw.scalingFirst > 0.0 ? nwpw.scalingFirst : 1.0;
    double scaling_second =
        nwpw.scalingSecond > 0.0 ? nwpw.scalingSecond : scaling_first;
    if (append_format(block, sizeof(block), "  scaling %.15g %.15g\n",
                      scaling_first, scaling_second) != 0)
      return -1;
  }
  if (include_direct_promoted &&
      (nwpw.npFftProcesses > 0 || nwpw.npOrbitalProcesses > 0 ||
       nwpw.npKspaceProcesses > 0)) {
    int np_fft = nwpw.npFftProcesses > 0 ? nwpw.npFftProcesses : -1;
    int np_orbital =
        nwpw.npOrbitalProcesses > 0 ? nwpw.npOrbitalProcesses : -1;
    int np_kspace = nwpw.npKspaceProcesses > 0 ? nwpw.npKspaceProcesses : -1;
    if (append_format(block, sizeof(block), "  np_dimensions %d %d %d\n",
                      np_fft, np_orbital, np_kspace) != 0)
      return -1;
  }
  if (include_direct_promoted &&
      nwpw.spinOrbit != NWChemNwpwToggle_unspecified) {
    const char *value =
        nwpw.spinOrbit == NWChemNwpwToggle_enabled ? "on" : "off";
    if (append_format(block, sizeof(block), "  spin_orbit %s\n", value) != 0)
      return -1;
  }
  if (include_direct_promoted &&
      nwpw.parallelIo != NWChemNwpwToggle_unspecified) {
    const char *value =
        nwpw.parallelIo == NWChemNwpwToggle_enabled ? "on" : "off";
    if (append_format(block, sizeof(block), "  parallel_io %s\n", value) != 0)
      return -1;
  }
  if (include_direct_promoted && nwpw.xyzFilename.len > 0) {
    if (append_format(block, sizeof(block), "  xyz_filename ") != 0 ||
        append_text(block, sizeof(block), nwpw.xyzFilename) != 0 ||
        append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (include_direct_promoted && nwpw.ionMotionFilename.len > 0) {
    if (append_format(block, sizeof(block), "  ion_motion_filename ") != 0 ||
        append_text(block, sizeof(block), nwpw.ionMotionFilename) != 0 ||
        append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (include_direct_promoted && nwpw.electronMotionFilename.len > 0) {
    if (append_format(block, sizeof(block), "  emotion_filename ") != 0 ||
        append_text(block, sizeof(block), nwpw.electronMotionFilename) != 0 ||
        append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (include_direct_promoted && nwpw.hamiltonianMotionFilename.len > 0) {
    if (append_format(block, sizeof(block), "  hmotion_filename ") != 0 ||
        append_text(block, sizeof(block), nwpw.hamiltonianMotionFilename) !=
            0 ||
        append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (include_direct_promoted && nwpw.orbitalMotionFilename.len > 0) {
    if (append_format(block, sizeof(block), "  omotion_filename ") != 0 ||
        append_text(block, sizeof(block), nwpw.orbitalMotionFilename) != 0 ||
        append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (include_direct_promoted && nwpw.eigenvalueMotionFilename.len > 0) {
    if (append_format(block, sizeof(block), "  eigmotion_filename ") != 0 ||
        append_text(block, sizeof(block), nwpw.eigenvalueMotionFilename) != 0 ||
        append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (include_direct_promoted && nwpw.fractionalOrbitalsStart > 0 &&
      nwpw.fractionalOrbitalsEnd > 0 &&
      append_format(block, sizeof(block), "  fractional_orbitals %d %d\n",
                    nwpw.fractionalOrbitalsStart,
                    nwpw.fractionalOrbitalsEnd) != 0)
    return -1;
  if (include_direct_promoted &&
      (nwpw.smearTemperature > 0.0 || nwpw.smearAlpha > 0.0 ||
       nwpw.smearType != NWChemNwpwSmearType_unspecified)) {
    if (append_format(block, sizeof(block), "  smear") != 0)
      return -1;
    if (nwpw.smearTemperature > 0.0 &&
        append_format(block, sizeof(block), " temperature %.15g",
                      nwpw.smearTemperature) != 0)
      return -1;
    if (nwpw.smearAlpha > 0.0 &&
        append_format(block, sizeof(block), " alpha %.15g",
                      nwpw.smearAlpha) != 0)
      return -1;
    if (nwpw.smearType != NWChemNwpwSmearType_unspecified) {
      const char *smear_type = NULL;
      if (nwpw.smearType == NWChemNwpwSmearType_fixed)
        smear_type = "fixed";
      else if (nwpw.smearType == NWChemNwpwSmearType_step)
        smear_type = "step";
      else if (nwpw.smearType == NWChemNwpwSmearType_fermi)
        smear_type = "fermi";
      else if (nwpw.smearType == NWChemNwpwSmearType_gaussian)
        smear_type = "gaussian";
      else if (nwpw.smearType == NWChemNwpwSmearType_marzariVanderbilt)
        smear_type = "marzari-vanderbilt";
      if (!smear_type ||
          append_format(block, sizeof(block), " %s", smear_type) != 0)
        return -1;
    }
    if (nwpw.fractionalOrbitalsStart > 0 && nwpw.fractionalOrbitalsEnd > 0 &&
        append_format(block, sizeof(block), " orbitals %d %d",
                      nwpw.fractionalOrbitalsStart,
                      nwpw.fractionalOrbitalsEnd) != 0)
      return -1;
    if (append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (include_direct_promoted && nwpw.virtualOrbitalsStart > 0 &&
      nwpw.virtualOrbitalsEnd > 0 &&
      append_format(block, sizeof(block), "  virtual_orbitals %d %d\n",
                    nwpw.virtualOrbitalsStart,
                    nwpw.virtualOrbitalsEnd) != 0)
    return -1;
  if (include_direct_promoted &&
      nwpw.lcaoMode == NWChemNwpwLcaoMode_skip &&
      append_format(block, sizeof(block), "  lcao_skip\n") != 0)
    return -1;
  if (include_direct_promoted &&
      nwpw.lcaoMode == NWChemNwpwLcaoMode_lcao &&
      append_format(block, sizeof(block), "  lcao\n") != 0)
    return -1;
  if (include_direct_promoted && nwpw.ewaldGridX > 0 &&
      append_format(block, sizeof(block), "  ewald_ngrid %d %d %d\n",
                    nwpw.ewaldGridX,
                    nwpw.ewaldGridY > 0 ? nwpw.ewaldGridY : nwpw.ewaldGridX,
                    nwpw.ewaldGridZ > 0 ? nwpw.ewaldGridZ
                                        : (nwpw.ewaldGridY > 0
                                               ? nwpw.ewaldGridY
                                               : nwpw.ewaldGridX)) != 0)
    return -1;
  if (include_direct_promoted &&
      (nwpw.noseHoover != NWChemNwpwToggle_unspecified ||
       nwpw.noseRestart != NWChemNwpwToggle_unspecified ||
       nwpw.noseElectronPeriod > 0.0 ||
       nwpw.noseElectronTemperature > 0.0 || nwpw.noseIonPeriod > 0.0 ||
       nwpw.noseIonTemperature > 0.0 || nwpw.noseElectronChainLength > 0 ||
       nwpw.noseIonChainLength > 0)) {
    if (nwpw.noseHoover == NWChemNwpwToggle_disabled) {
      if (append_format(block, sizeof(block), "  energy\n") != 0)
        return -1;
    } else {
      double electron_period =
          nwpw.noseElectronPeriod > 0.0 ? nwpw.noseElectronPeriod : 100.0;
      double electron_temperature = nwpw.noseElectronTemperature > 0.0
                                        ? nwpw.noseElectronTemperature
                                        : 298.15;
      double ion_period =
          nwpw.noseIonPeriod > 0.0 ? nwpw.noseIonPeriod : 100.0;
      double ion_temperature =
          nwpw.noseIonTemperature > 0.0 ? nwpw.noseIonTemperature : 298.15;
      int electron_chain_length = nwpw.noseElectronChainLength > 0
                                      ? nwpw.noseElectronChainLength
                                      : 1;
      int ion_chain_length =
          nwpw.noseIonChainLength > 0 ? nwpw.noseIonChainLength : 1;
      const char *restart =
          nwpw.noseRestart == NWChemNwpwToggle_disabled ? "start" : "restart";
      if (append_format(block, sizeof(block),
                        "  Nose-Hoover %.15g %.15g %.15g %.15g %s %d %d\n",
                        electron_period, electron_temperature, ion_period,
                        ion_temperature, restart, electron_chain_length,
                        ion_chain_length) != 0)
        return -1;
    }
  }
  if (render_directives(nwpw.directives, block, sizeof(block), "  ") != 0)
    return -1;
  if (!include_direct_promoted && strcmp(block, "nwpw\n") == 0)
    return 0;
  if (append_format(block, sizeof(block), "end") != 0)
    return -1;
  return append_block(dst, dst_size, block);
}

static int render_input_stanzas(NWChemInputStanza_list stanzas, char *dst,
                                size_t dst_size,
                                int include_direct_promoted_dft,
                                int include_direct_promoted_scf,
                                int include_direct_pseudopotentials,
                                int include_direct_promoted_driver,
                                int include_direct_promoted_nwpw,
                                int include_direct_promoted_tce,
                                int include_direct_set_strings,
                                int include_task_stanzas) {
  int n = struct_list_len(&stanzas.p);
  if (n < 0)
    return -1;
  for (int i = 0; i < n; ++i) {
    struct NWChemInputStanza stanza;
    get_NWChemInputStanza(&stanza, stanzas, i);
    switch (stanza.kind) {
    case NWChemInputStanza_Kind_dft:
      if (render_dft_stanza(stanza.dft, dst, dst_size,
                            include_direct_promoted_dft) != 0)
        return -1;
      break;
    case NWChemInputStanza_Kind_set:
      if (include_direct_set_strings &&
          render_set_stanza(stanza.set, dst, dst_size) != 0)
        return -1;
      break;
    case NWChemInputStanza_Kind_raw:
      if (append_block(dst, dst_size,
                       nwchemc_params_text_or(stanza.raw, "")) != 0)
        return -1;
      break;
    case NWChemInputStanza_Kind_module:
      if (render_module_stanza(stanza._module, dst, dst_size) != 0)
        return -1;
      break;
    case NWChemInputStanza_Kind_pseudopotential:
      if (render_pseudopotential_stanza(stanza.pseudopotential, dst,
                                        dst_size,
                                        include_direct_pseudopotentials) != 0)
        return -1;
      break;
    case NWChemInputStanza_Kind_scf:
      if (render_scf_stanza(stanza.scf, dst, dst_size,
                            include_direct_promoted_scf) != 0)
        return -1;
      break;
    case NWChemInputStanza_Kind_ccsd:
      if (render_ccsd_stanza(stanza.ccsd, dst, dst_size,
                             include_direct_promoted_scf) != 0)
        return -1;
      break;
    case NWChemInputStanza_Kind_tce:
      if (render_tce_stanza(stanza.tce, dst, dst_size,
                            include_direct_promoted_tce) != 0)
        return -1;
      break;
    case NWChemInputStanza_Kind_mrccData:
      if (render_mrcc_data_stanza(stanza.mrccData, dst, dst_size) != 0)
        return -1;
      break;
    case NWChemInputStanza_Kind_task:
      if (include_task_stanzas &&
          render_task_stanza(stanza.taskStanza, dst, dst_size) != 0)
        return -1;
      break;
    case NWChemInputStanza_Kind_driver:
      if (render_driver_stanza(stanza.driver, dst, dst_size,
                               include_direct_promoted_driver) != 0)
        return -1;
      break;
    case NWChemInputStanza_Kind_property:
      if (render_property_stanza(stanza.property, dst, dst_size) != 0)
        return -1;
      break;
    case NWChemInputStanza_Kind_basis:
      if (render_basis_stanza(stanza.basisStanza, dst, dst_size) != 0)
        return -1;
      break;
    case NWChemInputStanza_Kind_geometry:
      if (render_geometry_stanza(stanza.geometry, dst, dst_size) != 0)
        return -1;
      break;
    case NWChemInputStanza_Kind_nwpw:
      if (render_nwpw_stanza(stanza.nwpw, dst, dst_size,
                             include_direct_promoted_nwpw) != 0)
        return -1;
      break;
    case NWChemInputStanza_Kind_generic:
    default:
      if (render_generic_stanza(stanza.generic, dst, dst_size) != 0)
        return -1;
      break;
    }
  }
  return 0;
}

static int render_input_blocks(capn_ptr input_blocks, char *dst,
                               size_t dst_size) {
  int n = pointer_list_len(&input_blocks);
  if (n < 0)
    return -1;
  for (int i = 0; i < n; ++i) {
    capn_text block = capn_get_text(input_blocks, i, empty_text);
    if (append_block(dst, dst_size, nwchemc_params_text_or(block, "")) != 0)
      return -1;
  }
  return 0;
}

int nwchemc_params_render_input_blocks(NWChemParams_ptr params, char *dst,
                                       size_t dst_size) {
  if (params.p.type == CAPN_NULL || !dst || dst_size == 0)
    return -1;
  struct NWChemParams view;
  read_NWChemParams(&view, params);
  dst[0] = '\0';
  if (render_input_stanzas(view.inputStanzas, dst, dst_size, 1, 1, 1, 1, 1, 1,
                           1, 1) != 0)
    return -1;
  return render_input_blocks(view.inputBlocks, dst, dst_size);
}

int nwchemc_params_render_embed_input_blocks(NWChemParams_ptr params, char *dst,
                                             size_t dst_size) {
  if (params.p.type == CAPN_NULL || !dst || dst_size == 0)
    return -1;
  struct NWChemParams view;
  read_NWChemParams(&view, params);
  dst[0] = '\0';
  if (render_input_stanzas(view.inputStanzas, dst, dst_size, 0, 0, 0, 0, 0, 0,
                           0, 0) != 0)
    return -1;
  return render_input_blocks(view.inputBlocks, dst, dst_size);
}

int nwchemc_params_extract_direct_dft(NWChemParams_ptr params, capn_text *xc,
                                      int *direct_enabled,
                                      int *smearing_enabled,
                                      double *smear_sigma_hartree,
                                      int *smearing_spinset) {
  if (params.p.type == CAPN_NULL || !xc || !direct_enabled ||
      !smearing_enabled || !smear_sigma_hartree || !smearing_spinset)
    return -1;

  *xc = empty_text;
  *direct_enabled = 0;
  *smearing_enabled = 0;
  *smear_sigma_hartree = 0.0;
  *smearing_spinset = 1;

  struct NWChemParams view;
  read_NWChemParams(&view, params);
  int n = struct_list_len(&view.inputStanzas.p);
  if (n < 0)
    return -1;
  for (int i = 0; i < n; ++i) {
    struct NWChemInputStanza stanza;
    get_NWChemInputStanza(&stanza, view.inputStanzas, i);
    if (stanza.kind != NWChemInputStanza_Kind_dft ||
        stanza.dft.p.type == CAPN_NULL)
      continue;

    struct NWChemDftStanza dft;
    read_NWChemDftStanza(&dft, stanza.dft);
    if (dft.direct)
      *direct_enabled = 1;
    if (dft.xc.len > 0)
      *xc = dft.xc;
    if (dft.smearing.p.type != CAPN_NULL) {
      struct NWChemDftSmearing smearing;
      read_NWChemDftSmearing(&smearing, dft.smearing);
      if (smearing.sigmaHartree != 0.0) {
        *smearing_enabled = 1;
        *smear_sigma_hartree = smearing.sigmaHartree;
        *smearing_spinset =
            smearing.mode == NWChemDftSmearing_Mode_nofixsz ? 0 : 1;
      }
    }
  }
  return 0;
}

int nwchemc_params_extract_direct_scf(NWChemParams_ptr params, int *has_options,
                                      int *maxiter, double *thresh,
                                      double *tol2e) {
  if (params.p.type == CAPN_NULL || !has_options || !maxiter || !thresh ||
      !tol2e)
    return -1;

  *has_options = 0;
  *maxiter = 0;
  *thresh = 0.0;
  *tol2e = 0.0;

  struct NWChemParams view;
  read_NWChemParams(&view, params);
  int n = struct_list_len(&view.inputStanzas.p);
  if (n < 0)
    return -1;

  for (int i = 0; i < n; ++i) {
    struct NWChemInputStanza stanza;
    get_NWChemInputStanza(&stanza, view.inputStanzas, i);
    if (stanza.kind != NWChemInputStanza_Kind_scf ||
        stanza.scf.p.type == CAPN_NULL)
      continue;

    struct NWChemScfStanza scf;
    read_NWChemScfStanza(&scf, stanza.scf);
    if (scf.maxiter > 0) {
      *has_options = 1;
      *maxiter = scf.maxiter;
    }
    if (scf.thresh > 0.0) {
      *has_options = 1;
      *thresh = scf.thresh;
    }
    if (scf.tol2e > 0.0) {
      *has_options = 1;
      *tol2e = scf.tol2e;
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_ccsd(
    NWChemParams_ptr params, int *has_options, int *maxiter, double *thresh,
    double *tol2e, int *iprt, int *max_diis, int *frozen_core,
    int *frozen_virtual, int *use_disk, double *same_spin_scale,
    double *opposite_spin_scale) {
  if (params.p.type == CAPN_NULL || !has_options || !maxiter || !thresh ||
      !tol2e || !iprt || !max_diis || !frozen_core || !frozen_virtual ||
      !use_disk || !same_spin_scale || !opposite_spin_scale)
    return -1;

  *has_options = 0;
  *maxiter = 0;
  *thresh = 0.0;
  *tol2e = 0.0;
  *iprt = 0;
  *max_diis = 0;
  *frozen_core = 0;
  *frozen_virtual = 0;
  *use_disk = NWChemToggle_unspecified;
  *same_spin_scale = 0.0;
  *opposite_spin_scale = 0.0;

  struct NWChemParams view;
  read_NWChemParams(&view, params);
  int n = struct_list_len(&view.inputStanzas.p);
  if (n < 0)
    return -1;

  for (int i = 0; i < n; ++i) {
    struct NWChemInputStanza stanza;
    get_NWChemInputStanza(&stanza, view.inputStanzas, i);
    if (stanza.kind != NWChemInputStanza_Kind_ccsd ||
        stanza.ccsd.p.type == CAPN_NULL)
      continue;

    struct NWChemCcsdStanza ccsd;
    read_NWChemCcsdStanza(&ccsd, stanza.ccsd);
    if (ccsd.maxiter > 0) {
      *has_options = 1;
      *maxiter = ccsd.maxiter;
    }
    if (ccsd.thresh > 0.0) {
      *has_options = 1;
      *thresh = ccsd.thresh;
    }
    if (ccsd.tol2e > 0.0) {
      *has_options = 1;
      *tol2e = ccsd.tol2e;
    }
    if (ccsd.iprt > 0) {
      *has_options = 1;
      *iprt = ccsd.iprt;
    }
    if (ccsd.maxDiis > 0) {
      *has_options = 1;
      *max_diis = ccsd.maxDiis;
    }
    if (ccsd.frozenCore > 0) {
      *has_options = 1;
      *frozen_core = ccsd.frozenCore;
    }
    if (ccsd.frozenVirtual > 0) {
      *has_options = 1;
      *frozen_virtual = ccsd.frozenVirtual;
    }
    if (ccsd.useDisk != NWChemToggle_unspecified) {
      *has_options = 1;
      *use_disk = ccsd.useDisk;
    }
    if (ccsd.sameSpinScale > 0.0) {
      *has_options = 1;
      *same_spin_scale = ccsd.sameSpinScale;
    }
    if (ccsd.oppositeSpinScale > 0.0) {
      *has_options = 1;
      *opposite_spin_scale = ccsd.oppositeSpinScale;
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_driver(NWChemParams_ptr params,
                                         int *has_options, int *maxiter,
                                         int *tolerance_mode,
                                         double *gmax_tol,
                                         double *grms_tol,
                                         double *xmax_tol,
                                         double *xrms_tol) {
  if (params.p.type == CAPN_NULL || !has_options || !maxiter ||
      !tolerance_mode || !gmax_tol || !grms_tol || !xmax_tol || !xrms_tol)
    return -1;

  *has_options = 0;
  *maxiter = 0;
  *tolerance_mode = NWCHEMC_DRIVER_TOLERANCE_NONE;
  *gmax_tol = 0.0;
  *grms_tol = 0.0;
  *xmax_tol = 0.0;
  *xrms_tol = 0.0;

  struct NWChemParams view;
  read_NWChemParams(&view, params);
  int n = struct_list_len(&view.inputStanzas.p);
  if (n < 0)
    return -1;

  for (int i = 0; i < n; ++i) {
    struct NWChemInputStanza stanza;
    get_NWChemInputStanza(&stanza, view.inputStanzas, i);
    if (stanza.kind != NWChemInputStanza_Kind_driver ||
        stanza.driver.p.type == CAPN_NULL)
      continue;

    struct NWChemDriverStanza driver;
    read_NWChemDriverStanza(&driver, stanza.driver);
    if (driver.maxiter > 0) {
      *has_options = 1;
      *maxiter = driver.maxiter;
    }
    if (driver.tight) {
      *has_options = 1;
      *tolerance_mode = NWCHEMC_DRIVER_TOLERANCE_TIGHT;
    }
    if (driver.loose) {
      *has_options = 1;
      *tolerance_mode = NWCHEMC_DRIVER_TOLERANCE_LOOSE;
    }
    if (driver.gmaxTol > 0.0) {
      *has_options = 1;
      *gmax_tol = driver.gmaxTol;
    }
    if (driver.grmsTol > 0.0) {
      *has_options = 1;
      *grms_tol = driver.grmsTol;
    }
    if (driver.xmaxTol > 0.0) {
      *has_options = 1;
      *xmax_tol = driver.xmaxTol;
    }
    if (driver.xrmsTol > 0.0) {
      *has_options = 1;
      *xrms_tol = driver.xrmsTol;
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw(NWChemParams_ptr params,
                                       int *has_options,
                                       double *energy_cutoff,
                                       double *wavefunction_cutoff,
                                       double *ewald_rcut,
                                       int *ewald_ncut) {
  if (params.p.type == CAPN_NULL || !has_options || !energy_cutoff ||
      !wavefunction_cutoff || !ewald_rcut || !ewald_ncut)
    return -1;

  *has_options = 0;
  *energy_cutoff = 0.0;
  *wavefunction_cutoff = 0.0;
  *ewald_rcut = 0.0;
  *ewald_ncut = 0;

  struct NWChemParams view;
  read_NWChemParams(&view, params);
  int n = struct_list_len(&view.inputStanzas.p);
  if (n < 0)
    return -1;

  for (int i = 0; i < n; ++i) {
    struct NWChemInputStanza stanza;
    get_NWChemInputStanza(&stanza, view.inputStanzas, i);
    if (stanza.kind != NWChemInputStanza_Kind_nwpw ||
        stanza.nwpw.p.type == CAPN_NULL)
      continue;

    struct NWChemNwpwStanza nwpw;
    read_NWChemNwpwStanza(&nwpw, stanza.nwpw);
    if (nwpw.energyCutoff > 0.0) {
      *has_options = 1;
      *energy_cutoff = nwpw.energyCutoff;
    }
    if (nwpw.wavefunctionCutoff > 0.0) {
      *has_options = 1;
      *wavefunction_cutoff = nwpw.wavefunctionCutoff;
    }
    if (nwpw.ewaldRcut > 0.0) {
      *has_options = 1;
      *ewald_rcut = nwpw.ewaldRcut;
    }
    if (nwpw.ewaldNcut > 0) {
      *has_options = 1;
      *ewald_ncut = nwpw.ewaldNcut;
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_state(
    NWChemParams_ptr params, int *has_options, capn_text *cell_name,
    capn_text *input_wavefunction_filename,
    capn_text *output_wavefunction_filename, double *fake_mass,
    double *time_step, int *loop_start, int *loop_end, int *has_tolerances,
    double *tolerance_energy, double *tolerance_density,
    double *tolerance_gradient) {
  if (params.p.type == CAPN_NULL || !has_options || !cell_name ||
      !input_wavefunction_filename || !output_wavefunction_filename ||
      !fake_mass || !time_step || !loop_start || !loop_end ||
      !has_tolerances || !tolerance_energy || !tolerance_density ||
      !tolerance_gradient)
    return -1;

  *has_options = 0;
  *cell_name = (capn_text){0};
  *input_wavefunction_filename = (capn_text){0};
  *output_wavefunction_filename = (capn_text){0};
  *fake_mass = 0.0;
  *time_step = 0.0;
  *loop_start = 0;
  *loop_end = 0;
  *has_tolerances = 0;
  *tolerance_energy = 0.0;
  *tolerance_density = 0.0;
  *tolerance_gradient = 0.0;

  struct NWChemParams view;
  read_NWChemParams(&view, params);
  int n = struct_list_len(&view.inputStanzas.p);
  if (n < 0)
    return -1;

  for (int i = 0; i < n; ++i) {
    struct NWChemInputStanza stanza;
    get_NWChemInputStanza(&stanza, view.inputStanzas, i);
    if (stanza.kind != NWChemInputStanza_Kind_nwpw ||
        stanza.nwpw.p.type == CAPN_NULL)
      continue;

    struct NWChemNwpwStanza nwpw;
    read_NWChemNwpwStanza(&nwpw, stanza.nwpw);
    if (nwpw.cellName.len > 0) {
      *has_options = 1;
      *cell_name = nwpw.cellName;
    }
    if (nwpw.inputWavefunctionFilename.len > 0) {
      *has_options = 1;
      *input_wavefunction_filename = nwpw.inputWavefunctionFilename;
    }
    if (nwpw.outputWavefunctionFilename.len > 0) {
      *has_options = 1;
      *output_wavefunction_filename = nwpw.outputWavefunctionFilename;
    }
    if (nwpw.fakeMass > 0.0) {
      *has_options = 1;
      *fake_mass = nwpw.fakeMass;
    }
    if (nwpw.timeStep > 0.0) {
      *has_options = 1;
      *time_step = nwpw.timeStep;
    }
    if (nwpw.loopStart > 0 && nwpw.loopEnd > 0) {
      *has_options = 1;
      *loop_start = nwpw.loopStart;
      *loop_end = nwpw.loopEnd;
    }
    if (nwpw.toleranceEnergy > 0.0 || nwpw.toleranceDensity > 0.0 ||
        nwpw.toleranceGradient > 0.0) {
      *has_options = 1;
      *has_tolerances = 1;
      *tolerance_energy =
          nwpw.toleranceEnergy > 0.0 ? nwpw.toleranceEnergy : 1.0e-7;
      *tolerance_density =
          nwpw.toleranceDensity > 0.0 ? nwpw.toleranceDensity
                                      : *tolerance_energy;
      *tolerance_gradient =
          nwpw.toleranceGradient > 0.0 ? nwpw.toleranceGradient : 1.0e-4;
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_xc(NWChemParams_ptr params,
                                          int *has_options,
                                          capn_text *exchange_correlation) {
  if (params.p.type == CAPN_NULL || !has_options || !exchange_correlation)
    return -1;

  *has_options = 0;
  *exchange_correlation = (capn_text){0};

  struct NWChemParams view;
  read_NWChemParams(&view, params);
  int n = struct_list_len(&view.inputStanzas.p);
  if (n < 0)
    return -1;

  for (int i = 0; i < n; ++i) {
    struct NWChemInputStanza stanza;
    get_NWChemInputStanza(&stanza, view.inputStanzas, i);
    if (stanza.kind != NWChemInputStanza_Kind_nwpw ||
        stanza.nwpw.p.type == CAPN_NULL)
      continue;

    struct NWChemNwpwStanza nwpw;
    read_NWChemNwpwStanza(&nwpw, stanza.nwpw);
    if (nwpw.exchangeCorrelation.len > 0 &&
        !text_equals_ascii_ci(nwpw.exchangeCorrelation, "new")) {
      *has_options = 1;
      *exchange_correlation = nwpw.exchangeCorrelation;
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_bo(
    NWChemParams_ptr params, int *has_options, int *balance_mode,
    int *bo_step_start, int *bo_step_end, double *bo_time_step,
    int *bo_algorithm, double *bo_fake_mass, int *has_scaling,
    double *scaling_first, double *scaling_second) {
  if (params.p.type == CAPN_NULL || !has_options || !balance_mode ||
      !bo_step_start || !bo_step_end || !bo_time_step || !bo_algorithm ||
      !bo_fake_mass || !has_scaling || !scaling_first || !scaling_second)
    return -1;

  *has_options = 0;
  *balance_mode = NWChemNwpwBalanceMode_unspecified;
  *bo_step_start = 0;
  *bo_step_end = 0;
  *bo_time_step = 0.0;
  *bo_algorithm = NWChemNwpwBoAlgorithm_unspecified;
  *bo_fake_mass = 0.0;
  *has_scaling = 0;
  *scaling_first = 0.0;
  *scaling_second = 0.0;

  struct NWChemParams view;
  read_NWChemParams(&view, params);
  int n = struct_list_len(&view.inputStanzas.p);
  if (n < 0)
    return -1;

  for (int i = 0; i < n; ++i) {
    struct NWChemInputStanza stanza;
    get_NWChemInputStanza(&stanza, view.inputStanzas, i);
    if (stanza.kind != NWChemInputStanza_Kind_nwpw ||
        stanza.nwpw.p.type == CAPN_NULL)
      continue;

    struct NWChemNwpwStanza nwpw;
    read_NWChemNwpwStanza(&nwpw, stanza.nwpw);
    if (nwpw.balanceMode != NWChemNwpwBalanceMode_unspecified) {
      *has_options = 1;
      *balance_mode = nwpw.balanceMode;
    }
    if (nwpw.boStepStart > 0 && nwpw.boStepEnd > 0) {
      *has_options = 1;
      *bo_step_start = nwpw.boStepStart;
      *bo_step_end = nwpw.boStepEnd;
    }
    if (nwpw.boTimeStep > 0.0) {
      *has_options = 1;
      *bo_time_step = nwpw.boTimeStep;
    }
    if (nwpw.boAlgorithm != NWChemNwpwBoAlgorithm_unspecified) {
      *has_options = 1;
      *bo_algorithm = nwpw.boAlgorithm;
    }
    if (nwpw.boFakeMass > 0.0) {
      *has_options = 1;
      *bo_fake_mass = nwpw.boFakeMass;
    }
    if (nwpw.scalingFirst > 0.0 || nwpw.scalingSecond > 0.0) {
      *has_options = 1;
      *has_scaling = 1;
      *scaling_first = nwpw.scalingFirst > 0.0 ? nwpw.scalingFirst : 1.0;
      *scaling_second =
          nwpw.scalingSecond > 0.0 ? nwpw.scalingSecond : *scaling_first;
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_execution(
    NWChemParams_ptr params, int *has_options, int *np_fft,
    int *np_orbital, int *np_kspace, int *spin_orbit, int *parallel_io) {
  if (params.p.type == CAPN_NULL || !has_options || !np_fft ||
      !np_orbital || !np_kspace || !spin_orbit || !parallel_io)
    return -1;

  *has_options = 0;
  *np_fft = 0;
  *np_orbital = 0;
  *np_kspace = 0;
  *spin_orbit = NWChemNwpwToggle_unspecified;
  *parallel_io = NWChemNwpwToggle_unspecified;

  struct NWChemParams view;
  read_NWChemParams(&view, params);
  int n = struct_list_len(&view.inputStanzas.p);
  if (n < 0)
    return -1;

  for (int i = 0; i < n; ++i) {
    struct NWChemInputStanza stanza;
    get_NWChemInputStanza(&stanza, view.inputStanzas, i);
    if (stanza.kind != NWChemInputStanza_Kind_nwpw ||
        stanza.nwpw.p.type == CAPN_NULL)
      continue;

    struct NWChemNwpwStanza nwpw;
    read_NWChemNwpwStanza(&nwpw, stanza.nwpw);
    if (nwpw.npFftProcesses > 0 || nwpw.npOrbitalProcesses > 0 ||
        nwpw.npKspaceProcesses > 0) {
      *has_options = 1;
      *np_fft = nwpw.npFftProcesses > 0 ? nwpw.npFftProcesses : -1;
      *np_orbital =
          nwpw.npOrbitalProcesses > 0 ? nwpw.npOrbitalProcesses : -1;
      *np_kspace = nwpw.npKspaceProcesses > 0 ? nwpw.npKspaceProcesses : -1;
    }
    if (nwpw.spinOrbit != NWChemNwpwToggle_unspecified) {
      *has_options = 1;
      *spin_orbit = nwpw.spinOrbit;
    }
    if (nwpw.parallelIo != NWChemNwpwToggle_unspecified) {
      *has_options = 1;
      *parallel_io = nwpw.parallelIo;
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_filenames(
    NWChemParams_ptr params, int *has_options, capn_text *xyz_filename,
    capn_text *ion_motion_filename, capn_text *electron_motion_filename,
    capn_text *hamiltonian_motion_filename,
    capn_text *orbital_motion_filename,
    capn_text *eigenvalue_motion_filename) {
  if (params.p.type == CAPN_NULL || !has_options || !xyz_filename ||
      !ion_motion_filename || !electron_motion_filename ||
      !hamiltonian_motion_filename || !orbital_motion_filename ||
      !eigenvalue_motion_filename)
    return -1;

  *has_options = 0;
  *xyz_filename = (capn_text){0};
  *ion_motion_filename = (capn_text){0};
  *electron_motion_filename = (capn_text){0};
  *hamiltonian_motion_filename = (capn_text){0};
  *orbital_motion_filename = (capn_text){0};
  *eigenvalue_motion_filename = (capn_text){0};

  struct NWChemParams view;
  read_NWChemParams(&view, params);
  int n = struct_list_len(&view.inputStanzas.p);
  if (n < 0)
    return -1;

  for (int i = 0; i < n; ++i) {
    struct NWChemInputStanza stanza;
    get_NWChemInputStanza(&stanza, view.inputStanzas, i);
    if (stanza.kind != NWChemInputStanza_Kind_nwpw ||
        stanza.nwpw.p.type == CAPN_NULL)
      continue;

    struct NWChemNwpwStanza nwpw;
    read_NWChemNwpwStanza(&nwpw, stanza.nwpw);
    if (nwpw.xyzFilename.len > 0) {
      *has_options = 1;
      *xyz_filename = nwpw.xyzFilename;
    }
    if (nwpw.ionMotionFilename.len > 0) {
      *has_options = 1;
      *ion_motion_filename = nwpw.ionMotionFilename;
    }
    if (nwpw.electronMotionFilename.len > 0) {
      *has_options = 1;
      *electron_motion_filename = nwpw.electronMotionFilename;
    }
    if (nwpw.hamiltonianMotionFilename.len > 0) {
      *has_options = 1;
      *hamiltonian_motion_filename = nwpw.hamiltonianMotionFilename;
    }
    if (nwpw.orbitalMotionFilename.len > 0) {
      *has_options = 1;
      *orbital_motion_filename = nwpw.orbitalMotionFilename;
    }
    if (nwpw.eigenvalueMotionFilename.len > 0) {
      *has_options = 1;
      *eigenvalue_motion_filename = nwpw.eigenvalueMotionFilename;
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_fractional(
    NWChemParams_ptr params, int *has_fractional,
    int *fractional_orbitals_start, int *fractional_orbitals_end,
    int *has_smear, double *smear_temperature, double *smear_alpha,
    int *smear_type) {
  if (params.p.type == CAPN_NULL || !has_fractional ||
      !fractional_orbitals_start || !fractional_orbitals_end || !has_smear ||
      !smear_temperature || !smear_alpha || !smear_type)
    return -1;

  *has_fractional = 0;
  *fractional_orbitals_start = 0;
  *fractional_orbitals_end = 0;
  *has_smear = 0;
  *smear_temperature = 0.0;
  *smear_alpha = 0.0;
  *smear_type = NWChemNwpwSmearType_unspecified;

  struct NWChemParams view;
  read_NWChemParams(&view, params);
  int n = struct_list_len(&view.inputStanzas.p);
  if (n < 0)
    return -1;

  for (int i = 0; i < n; ++i) {
    struct NWChemInputStanza stanza;
    get_NWChemInputStanza(&stanza, view.inputStanzas, i);
    if (stanza.kind != NWChemInputStanza_Kind_nwpw ||
        stanza.nwpw.p.type == CAPN_NULL)
      continue;

    struct NWChemNwpwStanza nwpw;
    read_NWChemNwpwStanza(&nwpw, stanza.nwpw);
    if (nwpw.fractionalOrbitalsStart > 0 &&
        nwpw.fractionalOrbitalsEnd > 0) {
      *has_fractional = 1;
      *fractional_orbitals_start = nwpw.fractionalOrbitalsStart;
      *fractional_orbitals_end = nwpw.fractionalOrbitalsEnd;
    }
    if (nwpw.smearTemperature > 0.0 || nwpw.smearAlpha > 0.0 ||
        nwpw.smearType != NWChemNwpwSmearType_unspecified) {
      *has_smear = 1;
      if (!*has_fractional) {
        *has_fractional = 1;
        *fractional_orbitals_start = 4;
        *fractional_orbitals_end = 4;
      }
      *smear_temperature = nwpw.smearTemperature;
      *smear_alpha = nwpw.smearAlpha;
      *smear_type = nwpw.smearType;
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_orbital_grid(
    NWChemParams_ptr params, int *has_options, int *virtual_orbitals_start,
    int *virtual_orbitals_end, int *lcao_mode, int *ewald_grid_x,
    int *ewald_grid_y, int *ewald_grid_z) {
  if (params.p.type == CAPN_NULL || !has_options ||
      !virtual_orbitals_start || !virtual_orbitals_end || !lcao_mode ||
      !ewald_grid_x || !ewald_grid_y || !ewald_grid_z)
    return -1;

  *has_options = 0;
  *virtual_orbitals_start = 0;
  *virtual_orbitals_end = 0;
  *lcao_mode = NWChemNwpwLcaoMode_unspecified;
  *ewald_grid_x = 0;
  *ewald_grid_y = 0;
  *ewald_grid_z = 0;

  struct NWChemParams view;
  read_NWChemParams(&view, params);
  int n = struct_list_len(&view.inputStanzas.p);
  if (n < 0)
    return -1;

  for (int i = 0; i < n; ++i) {
    struct NWChemInputStanza stanza;
    get_NWChemInputStanza(&stanza, view.inputStanzas, i);
    if (stanza.kind != NWChemInputStanza_Kind_nwpw ||
        stanza.nwpw.p.type == CAPN_NULL)
      continue;

    struct NWChemNwpwStanza nwpw;
    read_NWChemNwpwStanza(&nwpw, stanza.nwpw);
    if (nwpw.virtualOrbitalsStart > 0 && nwpw.virtualOrbitalsEnd > 0) {
      *has_options = 1;
      *virtual_orbitals_start = nwpw.virtualOrbitalsStart;
      *virtual_orbitals_end = nwpw.virtualOrbitalsEnd;
    }
    if (nwpw.lcaoMode != NWChemNwpwLcaoMode_unspecified) {
      *has_options = 1;
      *lcao_mode = nwpw.lcaoMode;
    }
    if (nwpw.ewaldGridX > 0) {
      *has_options = 1;
      *ewald_grid_x = nwpw.ewaldGridX;
      *ewald_grid_y = nwpw.ewaldGridY > 0 ? nwpw.ewaldGridY : nwpw.ewaldGridX;
      *ewald_grid_z =
          nwpw.ewaldGridZ > 0 ? nwpw.ewaldGridZ : *ewald_grid_y;
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_nose(
    NWChemParams_ptr params, int *has_options, int *nose_hoover,
    int *nose_restart, double *electron_period, double *electron_temperature,
    double *ion_period, double *ion_temperature, int *electron_chain_length,
    int *ion_chain_length) {
  if (params.p.type == CAPN_NULL || !has_options || !nose_hoover ||
      !nose_restart || !electron_period || !electron_temperature ||
      !ion_period || !ion_temperature || !electron_chain_length ||
      !ion_chain_length)
    return -1;

  *has_options = 0;
  *nose_hoover = NWChemNwpwToggle_unspecified;
  *nose_restart = NWChemNwpwToggle_unspecified;
  *electron_period = 0.0;
  *electron_temperature = 0.0;
  *ion_period = 0.0;
  *ion_temperature = 0.0;
  *electron_chain_length = 0;
  *ion_chain_length = 0;

  struct NWChemParams view;
  read_NWChemParams(&view, params);
  int n = struct_list_len(&view.inputStanzas.p);
  if (n < 0)
    return -1;

  for (int i = 0; i < n; ++i) {
    struct NWChemInputStanza stanza;
    get_NWChemInputStanza(&stanza, view.inputStanzas, i);
    if (stanza.kind != NWChemInputStanza_Kind_nwpw ||
        stanza.nwpw.p.type == CAPN_NULL)
      continue;

    struct NWChemNwpwStanza nwpw;
    read_NWChemNwpwStanza(&nwpw, stanza.nwpw);
    if (nwpw.noseHoover == NWChemNwpwToggle_unspecified &&
        nwpw.noseRestart == NWChemNwpwToggle_unspecified &&
        nwpw.noseElectronPeriod <= 0.0 &&
        nwpw.noseElectronTemperature <= 0.0 && nwpw.noseIonPeriod <= 0.0 &&
        nwpw.noseIonTemperature <= 0.0 &&
        nwpw.noseElectronChainLength <= 0 && nwpw.noseIonChainLength <= 0)
      continue;

    *has_options = 1;
    *nose_hoover = nwpw.noseHoover == NWChemNwpwToggle_unspecified
                       ? NWChemNwpwToggle_enabled
                       : nwpw.noseHoover;
    *nose_restart = nwpw.noseRestart == NWChemNwpwToggle_unspecified
                        ? NWChemNwpwToggle_enabled
                        : nwpw.noseRestart;
    if (*nose_hoover == NWChemNwpwToggle_disabled) {
      *electron_period = 0.0;
      *electron_temperature = 0.0;
      *ion_period = 0.0;
      *ion_temperature = 0.0;
      *electron_chain_length = 0;
      *ion_chain_length = 0;
      continue;
    }
    *electron_period =
        nwpw.noseElectronPeriod > 0.0 ? nwpw.noseElectronPeriod : 100.0;
    *electron_temperature = nwpw.noseElectronTemperature > 0.0
                                ? nwpw.noseElectronTemperature
                                : 298.15;
    *ion_period = nwpw.noseIonPeriod > 0.0 ? nwpw.noseIonPeriod : 100.0;
    *ion_temperature =
        nwpw.noseIonTemperature > 0.0 ? nwpw.noseIonTemperature : 298.15;
    *electron_chain_length =
        nwpw.noseElectronChainLength > 0 ? nwpw.noseElectronChainLength : 1;
    *ion_chain_length =
        nwpw.noseIonChainLength > 0 ? nwpw.noseIonChainLength : 1;
  }

  return 0;
}

int nwchemc_params_extract_direct_pseudopotentials(
    NWChemParams_ptr params, capn_text *elements, int *library_types,
    capn_text *library_names, size_t capacity, size_t *count) {
  if (params.p.type == CAPN_NULL || !elements || !library_types ||
      !library_names || !count)
    return -1;

  *count = 0;

  struct NWChemParams view;
  read_NWChemParams(&view, params);
  int nstanzas = struct_list_len(&view.inputStanzas.p);
  if (nstanzas < 0)
    return -1;

  for (int i = 0; i < nstanzas; ++i) {
    struct NWChemInputStanza stanza;
    get_NWChemInputStanza(&stanza, view.inputStanzas, i);
    if (stanza.kind != NWChemInputStanza_Kind_pseudopotential ||
        stanza.pseudopotential.p.type == CAPN_NULL)
      continue;

    struct NWChemPseudopotentialStanza pseudopotential;
    read_NWChemPseudopotentialStanza(&pseudopotential,
                                     stanza.pseudopotential);
    int nentries = struct_list_len(&pseudopotential.entries.p);
    if (nentries < 0)
      return -1;
    for (int j = 0; j < nentries; ++j) {
      struct NWChemPseudopotentialEntry entry;
      get_NWChemPseudopotentialEntry(&entry, pseudopotential.entries, j);
      capn_text target = pseudopotential_entry_target(&entry);
      if (target.len <= 0 || entry.libraryName.len <= 0)
        continue;
      if (*count >= capacity)
        return -1;
      elements[*count] = target;
      library_types[*count] = entry.libraryType;
      library_names[*count] = entry.libraryName;
      ++*count;
    }
  }
  return 0;
}

int nwchemc_params_extract_direct_set_strings(NWChemParams_ptr params,
                                              capn_text *keys,
                                              capn_text *values,
                                              size_t capacity,
                                              size_t *count) {
  if (params.p.type == CAPN_NULL || !keys || !values || !count)
    return -1;

  *count = 0;

  struct NWChemParams view;
  read_NWChemParams(&view, params);
  int nstanzas = struct_list_len(&view.inputStanzas.p);
  if (nstanzas < 0)
    return -1;

  for (int i = 0; i < nstanzas; ++i) {
    struct NWChemInputStanza stanza;
    get_NWChemInputStanza(&stanza, view.inputStanzas, i);
    if (stanza.kind != NWChemInputStanza_Kind_set ||
        stanza.set.p.type == CAPN_NULL)
      continue;

    struct NWChemSetDirective set;
    read_NWChemSetDirective(&set, stanza.set);
    int nvalues = pointer_list_len(&set.values);
    if (nvalues < 0)
      return -1;
    if (nvalues > 0 ||
        (set.valueType != NWChemSetDirective_ValueType_auto &&
         set.valueType != NWChemSetDirective_ValueType_text))
      continue;
    if (set.key.len <= 0 || set.value.len <= 0)
      continue;
    if (*count >= capacity)
      return -1;
    keys[*count] = set.key;
    values[*count] = set.value;
    ++*count;
  }
  return 0;
}

static int map_set_value_type(int schema_value_type) {
  switch (schema_value_type) {
  case NWChemSetDirective_ValueType_text:
    return NWCHEMC_DIRECT_SET_VALUE_TEXT;
  case NWChemSetDirective_ValueType_double:
    return NWCHEMC_DIRECT_SET_VALUE_DOUBLE;
  case NWChemSetDirective_ValueType_integer:
    return NWCHEMC_DIRECT_SET_VALUE_INTEGER;
  case NWChemSetDirective_ValueType_logical:
    return NWCHEMC_DIRECT_SET_VALUE_LOGICAL;
  case NWChemSetDirective_ValueType_auto:
  default:
    return NWCHEMC_DIRECT_SET_VALUE_AUTO;
  }
}

int nwchemc_params_extract_direct_set_values(
    NWChemParams_ptr params, capn_text *keys, int *value_types,
    int *value_counts, capn_text *values, size_t set_capacity,
    size_t value_capacity, size_t *count) {
  if (params.p.type == CAPN_NULL || !keys || !value_types || !value_counts ||
      !values || !count || value_capacity == 0)
    return -1;

  *count = 0;

  struct NWChemParams view;
  read_NWChemParams(&view, params);
  int nstanzas = struct_list_len(&view.inputStanzas.p);
  if (nstanzas < 0)
    return -1;

  for (int i = 0; i < nstanzas; ++i) {
    struct NWChemInputStanza stanza;
    get_NWChemInputStanza(&stanza, view.inputStanzas, i);
    if (stanza.kind != NWChemInputStanza_Kind_set ||
        stanza.set.p.type == CAPN_NULL)
      continue;

    struct NWChemSetDirective set;
    read_NWChemSetDirective(&set, stanza.set);
    int nvalues = pointer_list_len(&set.values);
    if (nvalues < 0)
      return -1;
    if (nvalues == 0)
      continue;
    if (set.key.len <= 0)
      continue;
    if (*count >= set_capacity || (size_t)nvalues > value_capacity)
      return -1;

    keys[*count] = set.key;
    value_types[*count] = map_set_value_type(set.valueType);
    value_counts[*count] = nvalues;
    for (int j = 0; j < nvalues; ++j) {
      capn_text value = capn_get_text(set.values, j, empty_text);
      if (value.len <= 0)
        return -1;
      values[*count * value_capacity + (size_t)j] = value;
    }
    ++*count;
  }
  return 0;
}

int nwchemc_force_input_root(const void *force_input_capnp,
                             size_t force_input_capnp_size_bytes,
                             struct capn *arena, ForceInput_ptr *force_input) {
  if (!force_input_capnp || force_input_capnp_size_bytes == 0 || !arena ||
      !force_input)
    return -1;

  memset(arena, 0, sizeof(*arena));
  memset(force_input, 0, sizeof(*force_input));
  if (capn_init_mem(arena, (const uint8_t *)force_input_capnp,
                    force_input_capnp_size_bytes, 0) != 0)
    return -1;

  force_input->p = capn_getp(capn_root(arena), 0, 1);
  if (force_input->p.type != CAPN_STRUCT) {
    nwchemc_params_release(arena);
    memset(force_input, 0, sizeof(*force_input));
    return -1;
  }
  return 0;
}

int nwchemc_force_input_atom_count(ForceInput_ptr force_input,
                                   size_t *n_atoms, int *has_cell) {
  if (force_input.p.type == CAPN_NULL || !n_atoms || !has_cell)
    return -1;

  struct ForceInput view;
  read_ForceInput(&view, force_input);
  int n_pos = list64_len(view.pos);
  int n_z = list32_len(view.atmnrs);
  int n_box = list64_len(view.box);
  if (n_pos < 0 || n_z <= 0 || n_box < 0)
    return -1;
  if (n_pos != n_z * 3)
    return -1;
  if (n_box != 0 && n_box != 9)
    return -1;
  *n_atoms = (size_t)n_z;
  *has_cell = n_box == 9 ? 1 : 0;
  return 0;
}

int nwchemc_force_input_copy_geometry(ForceInput_ptr force_input,
                                      double *positions_ang,
                                      int *atomic_numbers,
                                      size_t atom_capacity, double *cell_ang,
                                      int *has_cell) {
  if (force_input.p.type == CAPN_NULL || !positions_ang || !atomic_numbers ||
      !cell_ang || !has_cell)
    return -1;

  struct ForceInput view;
  read_ForceInput(&view, force_input);
  size_t n_atoms = 0;
  int local_has_cell = 0;
  if (nwchemc_force_input_atom_count(force_input, &n_atoms, &local_has_cell) !=
      0)
    return -1;
  if (atom_capacity < n_atoms)
    return -1;

  double length_factor = 1.0;
  if (force_input_length_factor(view.lengthUnit, &length_factor) != 0)
    return -1;

  capn_resolve(&view.pos.p);
  capn_resolve(&view.atmnrs.p);
  capn_resolve(&view.box.p);

  for (size_t i = 0; i < n_atoms; ++i)
    atomic_numbers[i] = (int)(int32_t)capn_get32(view.atmnrs, (int)i);
  for (size_t i = 0; i < n_atoms * 3u; ++i)
    positions_ang[i] = capn_to_f64(capn_get64(view.pos, (int)i)) *
                       length_factor;
  for (size_t i = 0; i < 9; ++i)
    cell_ang[i] = local_has_cell
                      ? capn_to_f64(capn_get64(view.box, (int)i)) *
                            length_factor
                      : 0.0;
  *has_cell = local_has_cell;
  return 0;
}

int nwchemc_force_input_result_factors(ForceInput_ptr force_input,
                                       double *energy_factor,
                                       double *force_factor) {
  if (force_input.p.type == CAPN_NULL || !energy_factor || !force_factor)
    return -1;

  struct ForceInput view;
  read_ForceInput(&view, force_input);
  double length_factor = 1.0;
  double energy = 1.0;
  if (force_input_length_factor(view.lengthUnit, &length_factor) != 0 ||
      force_input_energy_factor(view.energyUnit, &energy) != 0)
    return -1;

  *energy_factor = energy;
  *force_factor = energy * length_factor / NWCHEMC_BOHR_TO_ANGSTROM;
  return 0;
}

size_t nwchemc_potential_result_flat_size(size_t force_count) {
  if (force_count > (SIZE_MAX - 32u) / 8u)
    return 0;
  return 32u + force_count * 8u;
}

int nwchemc_potential_result_write(double energy, const double *forces,
                                   size_t force_count,
                                   void *potential_result_capnp,
                                   size_t potential_result_capacity_bytes,
                                   size_t *potential_result_size_bytes) {
  if (!forces || !potential_result_capnp || !potential_result_size_bytes ||
      force_count > (size_t)INT_MAX)
    return -1;

  size_t required = nwchemc_potential_result_flat_size(force_count);
  *potential_result_size_bytes = required;
  if (required == 0 || potential_result_capacity_bytes < required)
    return -1;

  struct capn arena;
  capn_init_malloc(&arena);
  capn_ptr root = capn_root(&arena);
  if (root.type == CAPN_NULL) {
    capn_free(&arena);
    return -1;
  }

  PotentialResult_ptr result = new_PotentialResult(root.seg);
  capn_list64 force_list = capn_new_list64(root.seg, (int)force_count);
  if (result.p.type == CAPN_NULL ||
      (force_count > 0 && force_list.p.type == CAPN_NULL)) {
    capn_free(&arena);
    return -1;
  }
  for (size_t i = 0; i < force_count; ++i)
    capn_set64(force_list, (int)i, capn_from_f64(forces[i]));

  struct PotentialResult view;
  view.energy = energy;
  view.forces = force_list;
  write_PotentialResult(&view, result);
  if (capn_setp(root, 0, result.p) != 0) {
    capn_free(&arena);
    return -1;
  }

  int written = capn_write_mem(&arena, (uint8_t *)potential_result_capnp,
                               potential_result_capacity_bytes, 0);
  capn_free(&arena);
  if (written < 0)
    return -1;
  *potential_result_size_bytes = (size_t)written;
  return 0;
}

int nwchemc_params_root(const void *params_capnp,
                        size_t params_capnp_size_bytes, struct capn *arena,
                        NWChemParams_ptr *params) {
  if (!params_capnp || params_capnp_size_bytes == 0 || !arena || !params)
    return -1;

  memset(arena, 0, sizeof(*arena));
  memset(params, 0, sizeof(*params));
  if (capn_init_mem(arena, (const uint8_t *)params_capnp,
                    params_capnp_size_bytes, 0) != 0)
    return -1;

  params->p = capn_getp(capn_root(arena), 0, 1);
  if (params->p.type != CAPN_STRUCT) {
    nwchemc_params_release(arena);
    memset(params, 0, sizeof(*params));
    return -1;
  }
  return 0;
}

void nwchemc_params_release(struct capn *arena) {
  if (!arena)
    return;
  capn_free(arena);
  memset(arena, 0, sizeof(*arena));
}
