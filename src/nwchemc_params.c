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
                           1) != 0)
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
                           0) != 0)
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
