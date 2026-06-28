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
static const size_t NWCHEMC_POTENTIAL_RESULT_BASE_SIZE = 168u;

static const capn_text empty_text = {0, "", 0};

static double nwpw_dos_default_alpha(void) { return 0.05 / 27.2116; }

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

static int append_int32_args(capn_list32 values, char *dst, size_t dst_size) {
  capn_resolve(&values.p);
  int n = list32_len(values);
  if (n < 0)
    return -1;
  for (int i = 0; i < n; ++i) {
    int value = (int)(int32_t)capn_get32(values, i);
    if (append_format(dst, dst_size, " %d", value) != 0)
      return -1;
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

static int pseudopotential_entries_have_renderable(
    NWChemPseudopotentialEntry_list entries) {
  int n = struct_list_len(&entries.p);
  if (n < 0)
    return -1;
  for (int i = 0; i < n; ++i) {
    struct NWChemPseudopotentialEntry entry;
    get_NWChemPseudopotentialEntry(&entry, entries, i);
    capn_text target = pseudopotential_entry_target(&entry);
    if (target.len > 0 && entry.libraryName.len > 0)
      return 1;
  }
  return 0;
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

static const char *pseudopotential_block_name_literal(
    enum NWChemPseudopotentialBlockName block_name) {
  switch (block_name) {
  case NWChemPseudopotentialBlockName_pseudopotentialLibraries:
    return "pseudopotential_libraries";
  case NWChemPseudopotentialBlockName_pseudopotentials:
  default:
    return "pseudopotentials";
  }
}

static const char *pseudopotential_spin_literal(
    enum NWChemPseudopotentialSpinMode spin_mode) {
  switch (spin_mode) {
  case NWChemPseudopotentialSpinMode_disabled:
    return "off";
  case NWChemPseudopotentialSpinMode_enabled:
  case NWChemPseudopotentialSpinMode_unspecified:
  default:
    return NULL;
  }
}

static const char *pseudopotential_spin_channel_literal(
    enum NWChemPseudopotentialSpinRule_Channel channel) {
  switch (channel) {
  case NWChemPseudopotentialSpinRule_Channel_down:
    return "down";
  case NWChemPseudopotentialSpinRule_Channel_up:
  default:
    return "up";
  }
}

static const char *pseudopotential_spin_angular_literal(
    enum NWChemPseudopotentialSpinRule_AngularMomentum angular_momentum) {
  switch (angular_momentum) {
  case NWChemPseudopotentialSpinRule_AngularMomentum_p:
    return "p";
  case NWChemPseudopotentialSpinRule_AngularMomentum_d:
    return "d";
  case NWChemPseudopotentialSpinRule_AngularMomentum_f:
    return "f";
  case NWChemPseudopotentialSpinRule_AngularMomentum_s:
  default:
    return "s";
  }
}

static const char *pseudopotential_uterm_angular_literal(
    enum NWChemPseudopotentialUtermRule_AngularMomentum angular_momentum) {
  switch (angular_momentum) {
  case NWChemPseudopotentialUtermRule_AngularMomentum_p:
    return "p";
  case NWChemPseudopotentialUtermRule_AngularMomentum_d:
    return "d";
  case NWChemPseudopotentialUtermRule_AngularMomentum_f:
    return "f";
  case NWChemPseudopotentialUtermRule_AngularMomentum_s:
  default:
    return "s";
  }
}

static int render_pseudopotential_spin_rules(
    NWChemPseudopotentialSpinRule_list rules, char *dst, size_t dst_size) {
  int n = struct_list_len(&rules.p);
  if (n < 0)
    return -1;
  for (int i = 0; i < n; ++i) {
    struct NWChemPseudopotentialSpinRule rule;
    get_NWChemPseudopotentialSpinRule(&rule, rules, i);
    capn_list32 ion_indices = rule.ionIndices;
    capn_resolve(&ion_indices.p);
    if (ion_indices.p.type == CAPN_NULL)
      continue;
    if (ion_indices.p.type != CAPN_LIST || ion_indices.p.datasz != 4)
      return -1;
    int nions = ion_indices.p.len;
    if (nions == 0)
      continue;
    if (rule.hasMagneticQuantumNumber) {
      if (append_format(dst, dst_size, "  pspspin not_m %d %s %s %.15g",
                        rule.magneticQuantumNumber,
                        pseudopotential_spin_channel_literal(rule.channel),
                        pseudopotential_spin_angular_literal(
                            rule.angularMomentum),
                        rule.scale) != 0)
        return -1;
    } else if (append_format(dst, dst_size, "  pspspin %s %s %.15g",
                             pseudopotential_spin_channel_literal(
                                 rule.channel),
                             pseudopotential_spin_angular_literal(
                                 rule.angularMomentum),
                             rule.scale) != 0) {
      return -1;
    }
    for (int j = 0; j < nions; ++j) {
      int ion_index = (int)(int32_t)capn_get32(ion_indices, j);
      if (append_format(dst, dst_size, " %d", ion_index) != 0)
        return -1;
    }
    if (append_format(dst, dst_size, "\n") != 0)
      return -1;
  }
  return 0;
}

static int render_pseudopotential_uterm_rules(
    NWChemPseudopotentialUtermRule_list rules, char *dst, size_t dst_size) {
  int n = struct_list_len(&rules.p);
  if (n < 0)
    return -1;
  for (int i = 0; i < n; ++i) {
    struct NWChemPseudopotentialUtermRule rule;
    get_NWChemPseudopotentialUtermRule(&rule, rules, i);
    capn_list32 ion_indices = rule.ionIndices;
    capn_resolve(&ion_indices.p);
    if (ion_indices.p.type == CAPN_NULL)
      continue;
    if (ion_indices.p.type != CAPN_LIST || ion_indices.p.datasz != 4)
      return -1;
    int nions = ion_indices.p.len;
    if (nions == 0)
      continue;
    if (append_format(dst, dst_size, "  uterm %s %.15g %.15g",
                      pseudopotential_uterm_angular_literal(
                          rule.angularMomentum),
                      rule.uScale, rule.jScale) != 0)
      return -1;
    for (int j = 0; j < nions; ++j) {
      int ion_index = (int)(int32_t)capn_get32(ion_indices, j);
      if (append_format(dst, dst_size, " %d", ion_index) != 0)
        return -1;
    }
    if (append_format(dst, dst_size, "\n") != 0)
      return -1;
  }
  return 0;
}

static const char *pseudopotential_semicore_small_logical(
    enum NWChemToggle toggle) {
  switch (toggle) {
  case NWChemToggle_enabled:
    return "true";
  case NWChemToggle_disabled:
    return "false";
  case NWChemToggle_unspecified:
  default:
    return NULL;
  }
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
  int has_entries = pseudopotential_entries_have_renderable(
      pseudopotential.entries);
  if (has_entries < 0)
    return -1;
  if (append_format(block, sizeof(block), "nwpw\n") != 0)
    return -1;
  if (include_direct_entries && has_entries) {
    if (append_format(block, sizeof(block), "  %s\n",
                      pseudopotential_block_name_literal(
                          pseudopotential.blockName)) != 0 ||
        render_pseudopotential_entries(pseudopotential.entries, block,
                                       sizeof(block)) != 0 ||
        append_format(block, sizeof(block), "  end\n") != 0)
      return -1;
  }
  const char *psp_spin = pseudopotential_spin_literal(pseudopotential.pspSpin);
  if (include_direct_entries && psp_spin &&
      append_format(block, sizeof(block), "  pspspin %s\n", psp_spin) != 0)
    return -1;
  if (include_direct_entries &&
      pseudopotential.uterm == NWChemToggle_disabled &&
      append_format(block, sizeof(block), "  uterm off\n") != 0)
    return -1;
  if (include_direct_entries &&
      render_pseudopotential_uterm_rules(pseudopotential.utermRules, block,
                                         sizeof(block)) != 0)
    return -1;
  if (include_direct_entries &&
      render_pseudopotential_spin_rules(pseudopotential.spinRules, block,
                                        sizeof(block)) != 0)
    return -1;
  if (render_directives(pseudopotential.directives, block, sizeof(block),
                        "  ") != 0)
    return -1;
  if (!include_direct_entries &&
      strcmp(block, "nwpw\n") == 0)
    return 0;
  if (append_format(block, sizeof(block), "end") != 0)
    return -1;
  const char *semicore_small = pseudopotential_semicore_small_logical(
      pseudopotential.semicoreSmall);
  if (include_direct_entries && semicore_small &&
      append_format(block, sizeof(block),
                    "\nset nwpw:psp:semicore_small logical %s",
                    semicore_small) != 0)
    return -1;
  return append_block(dst, dst_size, block);
}

static int append_text_or_default(char *dst, size_t dst_size, capn_text text,
                                  const char *default_text) {
  if (text.len > 0)
    return append_text(dst, dst_size, text);
  return append_format(dst, dst_size, "%s", default_text);
}

static int append_zone_name_or_default(char *dst, size_t dst_size,
                                       capn_text zone_name) {
  return append_text_or_default(dst, dst_size, zone_name, "zone_default");
}

static int render_brillouin_zone_stanza(NWChemBrillouinZoneStanza_ptr ptr,
                                        char *dst, size_t dst_size,
                                        int include_direct_promoted) {
  if (ptr.p.type == CAPN_NULL || !include_direct_promoted)
    return 0;

  struct NWChemBrillouinZoneStanza zone;
  char block[4096];
  block[0] = '\0';
  read_NWChemBrillouinZoneStanza(&zone, ptr);
  int nk = struct_list_len(&zone.kVectors.p);
  if (nk < 0)
    return -1;
  int has_monkhorst = zone.monkhorstPackX != 0 ||
                      zone.monkhorstPackY != 0 ||
                      zone.monkhorstPackZ != 0;
  int has_dos_grid =
      zone.dosGridX > 0 || zone.dosGridY > 0 || zone.dosGridZ > 0;
  int has_dos_fft_grid = zone.dosFftGridX > 0 || zone.dosFftGridY > 0 ||
                         zone.dosFftGridZ > 0;
  int has_tetrahedron = zone.tetrahedronGridX > 0 ||
                        zone.tetrahedronGridY > 0 ||
                        zone.tetrahedronGridZ > 0;
  int has_directives = directives_have_keywords(zone.directives);
  if (has_directives < 0)
    return -1;
  if (!has_monkhorst && !has_dos_grid && !has_dos_fft_grid &&
      !has_tetrahedron && nk == 0 && zone.maxKpointsPrint <= 0 &&
      !has_directives && zone.zoneName.len <= 0 &&
      zone.zoneStructureName.len <= 0 && zone.zoneFftName.len <= 0)
    return 0;

  if (append_format(block, sizeof(block), "nwpw\n") != 0)
    return -1;
  if (has_monkhorst) {
    int mx = zone.monkhorstPackX != 0 ? zone.monkhorstPackX : 1;
    int my = zone.monkhorstPackY != 0 ? zone.monkhorstPackY : 1;
    int mz = zone.monkhorstPackZ != 0 ? zone.monkhorstPackZ : 1;
    if (append_format(block, sizeof(block), "  monkhorst-pack %d %d %d ",
                      mx, my, mz) != 0 ||
        append_zone_name_or_default(block, sizeof(block), zone.zoneName) != 0 ||
        append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (has_dos_grid) {
    int dx = zone.dosGridX > 0 ? zone.dosGridX : 2;
    int dy = zone.dosGridY > 0 ? zone.dosGridY : 2;
    int dz = zone.dosGridZ > 0 ? zone.dosGridZ : 2;
    if (append_format(block, sizeof(block), "  dos-grid %d %d %d ", dx, dy,
                      dz) != 0 ||
        append_text_or_default(block, sizeof(block), zone.dosGridZoneName,
                               "structure_default") != 0 ||
        append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (has_dos_fft_grid) {
    int dx = zone.dosFftGridX > 0 ? zone.dosFftGridX : 2;
    int dy = zone.dosFftGridY > 0 ? zone.dosFftGridY : 2;
    int dz = zone.dosFftGridZ > 0 ? zone.dosFftGridZ : 2;
    if (append_format(block, sizeof(block), "  dos-fft-grid %d %d %d ", dx,
                      dy, dz) != 0 ||
        append_text_or_default(block, sizeof(block), zone.dosFftGridZoneName,
                               "zone_fft_default") != 0 ||
        append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (has_tetrahedron) {
    int tx = zone.tetrahedronGridX > 0 ? zone.tetrahedronGridX : 2;
    int ty = zone.tetrahedronGridY > 0 ? zone.tetrahedronGridY : 2;
    int tz = zone.tetrahedronGridZ > 0 ? zone.tetrahedronGridZ : 2;
    if (append_format(block, sizeof(block), "  tetrahedron %d %d %d ", tx,
                      ty, tz) != 0 ||
        append_zone_name_or_default(block, sizeof(block),
                                    zone.tetrahedronZoneName) != 0 ||
        append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (nk > 0 || zone.maxKpointsPrint > 0 || has_directives) {
    if (append_format(block, sizeof(block), "  brillouin_zone\n") != 0)
      return -1;
    if (zone.zoneName.len > 0) {
      if (append_format(block, sizeof(block), "    zone_name ") != 0 ||
          append_text(block, sizeof(block), zone.zoneName) != 0 ||
          append_format(block, sizeof(block), "\n") != 0)
        return -1;
    }
    for (int i = 0; i < nk; ++i) {
      struct NWChemKVector kvector;
      get_NWChemKVector(&kvector, zone.kVectors, i);
      if (append_format(block, sizeof(block), "    kvector %.15g %.15g %.15g",
                        kvector.x, kvector.y, kvector.z) != 0)
        return -1;
      if (kvector.weight != 0.0 &&
          append_format(block, sizeof(block), " %.15g", kvector.weight) != 0)
        return -1;
      if (append_format(block, sizeof(block), "\n") != 0)
        return -1;
    }
    if (zone.maxKpointsPrint > 0 &&
        append_format(block, sizeof(block), "    max_kpoints_print %d\n",
                      zone.maxKpointsPrint) != 0)
      return -1;
    if (render_directives(zone.directives, block, sizeof(block), "    ") != 0 ||
        append_format(block, sizeof(block), "  end\n") != 0)
      return -1;
  }
  if (zone.zoneName.len > 0) {
    if (append_format(block, sizeof(block), "  zone_name ") != 0 ||
        append_text(block, sizeof(block), zone.zoneName) != 0 ||
        append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (zone.zoneStructureName.len > 0) {
    if (append_format(block, sizeof(block), "  zone_structure_name ") != 0 ||
        append_text(block, sizeof(block), zone.zoneStructureName) != 0 ||
        append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (zone.zoneFftName.len > 0) {
    if (append_format(block, sizeof(block), "  zone_fft_name ") != 0 ||
        append_text(block, sizeof(block), zone.zoneFftName) != 0 ||
        append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (append_format(block, sizeof(block), "end") != 0)
    return -1;
  return append_block(dst, dst_size, block);
}

static const char *simulation_cell_lattice_kind_literal(
    enum NWChemSimulationCellLatticeKind lattice_kind) {
  switch (lattice_kind) {
  case NWChemSimulationCellLatticeKind_sc:
    return "sc";
  case NWChemSimulationCellLatticeKind_fcc:
    return "fcc";
  case NWChemSimulationCellLatticeKind_bcc:
    return "bcc";
  case NWChemSimulationCellLatticeKind_unspecified:
  default:
    return NULL;
  }
}

static int render_simulation_cell_stanza(NWChemSimulationCellStanza_ptr ptr,
                                         char *dst, size_t dst_size,
                                         int include_direct_promoted) {
  if (ptr.p.type == CAPN_NULL || !include_direct_promoted)
    return 0;

  struct NWChemSimulationCellStanza cell;
  char block[4096];
  block[0] = '\0';
  read_NWChemSimulationCellStanza(&cell, ptr);
  capn_list64 lattice_vectors = cell.latticeVectorsBohr;
  capn_resolve(&lattice_vectors.p);
  int nvector = 0;
  if (lattice_vectors.p.type != CAPN_NULL) {
    if (lattice_vectors.p.type != CAPN_LIST || lattice_vectors.p.datasz != 8)
      return -1;
    nvector = lattice_vectors.p.len;
  }
  if (nvector != 0 && nvector != 9)
    return -1;
  int has_ngrid = cell.ngridX > 0 && cell.ngridY > 0 && cell.ngridZ > 0;
  int has_ngrid_small = cell.ngridSmallX > 0 && cell.ngridSmallY > 0 &&
                        cell.ngridSmallZ > 0;
  const char *lattice_kind =
      simulation_cell_lattice_kind_literal(cell.latticeKind);
  int has_lattice_kind = lattice_kind && cell.latticeLengthBohr > 0.0;
  int has_directives = directives_have_keywords(cell.directives);
  if (has_directives < 0)
    return -1;
  if (cell.cellName.len <= 0 && cell.boundaryConditions.len <= 0 &&
      nvector == 0 && !has_ngrid && !has_ngrid_small &&
      cell.boxDeltaBohr <= 0.0 && !cell.boxOrient &&
      !cell.boxDifferentLengths && !has_lattice_kind && !has_directives)
    return 0;

  if (append_format(block, sizeof(block), "nwpw\n  simulation_cell\n") != 0)
    return -1;
  if (cell.cellName.len > 0) {
    if (append_format(block, sizeof(block), "    cell_name ") != 0 ||
        append_text(block, sizeof(block), cell.cellName) != 0 ||
        append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (cell.boundaryConditions.len > 0) {
    if (append_format(block, sizeof(block), "    boundary_conditions ") != 0 ||
        append_text(block, sizeof(block), cell.boundaryConditions) != 0 ||
        append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (nvector == 9) {
    if (append_format(block, sizeof(block), "    lattice_vectors\n") != 0)
      return -1;
    for (int row = 0; row < 3; ++row) {
      double first =
          capn_to_f64(capn_get64(lattice_vectors, row * 3));
      double second =
          capn_to_f64(capn_get64(lattice_vectors, row * 3 + 1));
      double third =
          capn_to_f64(capn_get64(lattice_vectors, row * 3 + 2));
      if (append_format(block, sizeof(block), "      %.15g %.15g %.15g\n",
                        first, second, third) != 0)
        return -1;
    }
  }
  if (has_ngrid &&
      append_format(block, sizeof(block), "    ngrid %d %d %d\n", cell.ngridX,
                    cell.ngridY, cell.ngridZ) != 0)
    return -1;
  if (has_ngrid_small &&
      append_format(block, sizeof(block), "    ngrid_small %d %d %d\n",
                    cell.ngridSmallX, cell.ngridSmallY,
                    cell.ngridSmallZ) != 0)
    return -1;
  if (cell.boxDeltaBohr > 0.0 &&
      append_format(block, sizeof(block), "    box_delta %.15g\n",
                    cell.boxDeltaBohr) != 0)
    return -1;
  if (cell.boxOrient &&
      append_format(block, sizeof(block), "    box_orient\n") != 0)
    return -1;
  if (cell.boxDifferentLengths &&
      append_format(block, sizeof(block), "    box_different_lengths\n") != 0)
    return -1;
  if (has_lattice_kind &&
      append_format(block, sizeof(block), "    %s %.15g\n", lattice_kind,
                    cell.latticeLengthBohr) != 0)
    return -1;
  if (render_directives(cell.directives, block, sizeof(block), "    ") != 0 ||
      append_format(block, sizeof(block), "  end\nend") != 0)
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

static const char *print_level_keyword(enum NWChemPrintLevel print_level);

static int render_int32_list_directive(capn_list32 values, const char *keyword,
                                       char *dst, size_t dst_size) {
  capn_resolve(&values.p);
  if (values.p.type == CAPN_NULL)
    return 0;
  if (values.p.type != CAPN_LIST || values.p.datasz != 4)
    return -1;
  int nvalues = values.p.len;
  if (nvalues == 0)
    return 0;
  if (append_format(dst, dst_size, "  %s", keyword) != 0)
    return -1;
  for (int i = 0; i < nvalues; ++i) {
    int value = (int)(int32_t)capn_get32(values, i);
    if (append_format(dst, dst_size, " %d", value) != 0)
      return -1;
  }
  return append_format(dst, dst_size, "\n");
}

static const char *nwchem_toggle_logical_keyword(enum NWChemToggle toggle) {
  switch (toggle) {
  case NWChemToggle_enabled:
    return "true";
  case NWChemToggle_disabled:
    return "false";
  case NWChemToggle_unspecified:
  default:
    return NULL;
  }
}

static const char *
nwpw_toggle_logical_keyword(enum NWChemNwpwToggle toggle) {
  switch (toggle) {
  case NWChemNwpwToggle_enabled:
    return "true";
  case NWChemNwpwToggle_disabled:
    return "false";
  case NWChemNwpwToggle_unspecified:
  default:
    return NULL;
  }
}

static const char *
nwpw_spin_mode_keyword(enum NWChemNwpwSpinMode spin_mode) {
  switch (spin_mode) {
  case NWChemNwpwSpinMode_restricted:
    return "dft";
  case NWChemNwpwSpinMode_unrestricted:
    return "odft";
  case NWChemNwpwSpinMode_unspecified:
  default:
    return NULL;
  }
}

static int nwpw_spin_mode_ispin(enum NWChemNwpwSpinMode spin_mode) {
  switch (spin_mode) {
  case NWChemNwpwSpinMode_restricted:
    return 1;
  case NWChemNwpwSpinMode_unrestricted:
    return 2;
  case NWChemNwpwSpinMode_unspecified:
  default:
    return 0;
  }
}

static const char *
nwpw_minimizer_command(enum NWChemNwpwMinimizer minimizer) {
  switch (minimizer) {
  case NWChemNwpwMinimizer_cgGrassman:
  case NWChemNwpwMinimizer_cgStiefel:
  case NWChemNwpwMinimizer_cgStich:
    return "cg";
  case NWChemNwpwMinimizer_lmbfgsGrassman:
  case NWChemNwpwMinimizer_lmbfgsStiefel:
  case NWChemNwpwMinimizer_lmbfgsStich:
    return "lmbfgs";
  case NWChemNwpwMinimizer_scfDensity:
  case NWChemNwpwMinimizer_scfPotential:
    return "scf";
  case NWChemNwpwMinimizer_unspecified:
  default:
    return NULL;
  }
}

static const char *
nwpw_minimizer_variant(enum NWChemNwpwMinimizer minimizer) {
  switch (minimizer) {
  case NWChemNwpwMinimizer_cgGrassman:
  case NWChemNwpwMinimizer_lmbfgsGrassman:
    return "grassman";
  case NWChemNwpwMinimizer_cgStiefel:
  case NWChemNwpwMinimizer_lmbfgsStiefel:
    return "stiefel";
  case NWChemNwpwMinimizer_cgStich:
  case NWChemNwpwMinimizer_lmbfgsStich:
    return "stich";
  case NWChemNwpwMinimizer_scfDensity:
    return "density";
  case NWChemNwpwMinimizer_scfPotential:
    return "potential";
  case NWChemNwpwMinimizer_unspecified:
  default:
    return NULL;
  }
}

static int nwpw_minimizer_is_scf(enum NWChemNwpwMinimizer minimizer) {
  return minimizer == NWChemNwpwMinimizer_scfDensity ||
         minimizer == NWChemNwpwMinimizer_scfPotential;
}

static const char *
nwpw_ks_algorithm_keyword(enum NWChemNwpwKsAlgorithm algorithm) {
  switch (algorithm) {
  case NWChemNwpwKsAlgorithm_blockCg:
    return "block-cg";
  case NWChemNwpwKsAlgorithm_cg:
    return "cg";
  case NWChemNwpwKsAlgorithm_rmmDiis:
    return "rmm-diis";
  case NWChemNwpwKsAlgorithm_unspecified:
  default:
    return NULL;
  }
}

static const char *
nwpw_scf_algorithm_keyword(enum NWChemNwpwScfAlgorithm algorithm) {
  switch (algorithm) {
  case NWChemNwpwScfAlgorithm_simple:
    return "simple";
  case NWChemNwpwScfAlgorithm_broyden:
    return "broyden";
  case NWChemNwpwScfAlgorithm_diis:
    return "diis";
  case NWChemNwpwScfAlgorithm_anderson:
    return "anderson";
  case NWChemNwpwScfAlgorithm_unspecified:
  default:
    return NULL;
  }
}

static const char *
nwpw_precondition_keyword(enum NWChemNwpwToggle precondition) {
  switch (precondition) {
  case NWChemNwpwToggle_enabled:
    return "precondition";
  case NWChemNwpwToggle_disabled:
    return "noprecondition";
  case NWChemNwpwToggle_unspecified:
  default:
    return NULL;
  }
}

static const char *
nwpw_efield_type_keyword(enum NWChemNwpwEfieldType efield_type) {
  switch (efield_type) {
  case NWChemNwpwEfieldType_periodic:
    return "periodic";
  case NWChemNwpwEfieldType_apc:
    return "APC";
  case NWChemNwpwEfieldType_rgrid:
    return "rgrid";
  case NWChemNwpwEfieldType_unspecified:
  default:
    return NULL;
  }
}

static const char *
nwpw_mapping_alias_keyword(enum NWChemNwpwMappingAlias mapping_alias) {
  switch (mapping_alias) {
  case NWChemNwpwMappingAlias_slab1d:
    return "1d-slab";
  case NWChemNwpwMappingAlias_hilbert2d:
    return "2d-hilbert";
  case NWChemNwpwMappingAlias_hcurve2d:
    return "2d-hcurve";
  case NWChemNwpwMappingAlias_unspecified:
  default:
    return NULL;
  }
}

static int
nwpw_mapping_alias_value(enum NWChemNwpwMappingAlias mapping_alias) {
  switch (mapping_alias) {
  case NWChemNwpwMappingAlias_slab1d:
    return 1;
  case NWChemNwpwMappingAlias_hilbert2d:
    return 2;
  case NWChemNwpwMappingAlias_hcurve2d:
    return 3;
  case NWChemNwpwMappingAlias_unspecified:
  default:
    return 0;
  }
}

static int render_set_logical_directive(const char *key,
                                        enum NWChemToggle toggle, char *dst,
                                        size_t dst_size) {
  const char *value = nwchem_toggle_logical_keyword(toggle);
  if (!value)
    return 0;
  if (dst[0] != '\0') {
    size_t used = strlen(dst);
    if (used > 0 && dst[used - 1] != '\n' &&
        append_format(dst, dst_size, "\n") != 0)
      return -1;
  }
  return append_format(dst, dst_size, "set %s logical %s\n", key, value);
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
  const char *print_level = print_level_keyword(ccsd.printLevel);
  int nprint_items = pointer_list_len(&ccsd.printItems);
  int nnoprint_items = pointer_list_len(&ccsd.noPrintItems);
  if (nprint_items < 0 || nnoprint_items < 0)
    return -1;
  int ndoa = list32_len(ccsd.doa);
  int ndob = list32_len(ccsd.dob);
  int ndog = list32_len(ccsd.dog);
  int ndoh = list32_len(ccsd.doh);
  int ndojk = list32_len(ccsd.dojk);
  int ndos = list32_len(ccsd.dos);
  int ndod = list32_len(ccsd.dod);
  if (ndoa < 0 || ndob < 0 || ndog < 0 || ndoh < 0 || ndojk < 0 ||
      ndos < 0 || ndod < 0)
    return -1;
  int has_promoted =
      ccsd.maxiter > 0 || ccsd.thresh > 0.0 || ccsd.tol2e > 0.0 ||
      ccsd.iprt > 0 || ccsd.maxDiis > 0 || ccsd.frozenCore > 0 ||
      ccsd.frozenVirtual > 0 || ccsd.useDisk != NWChemToggle_unspecified ||
      ccsd.sameSpinScale > 0.0 || ccsd.oppositeSpinScale > 0.0;
  int has_print = print_level || nprint_items > 0 || nnoprint_items > 0;
  int has_switches = ndoa > 0 || ndob > 0 || ndog > 0 || ndoh > 0 ||
                     ndojk > 0 || ndos > 0 || ndod > 0;
  int has_direct_toggles =
      ccsd.useTriplesDriverNonblocking != NWChemToggle_unspecified ||
      ccsd.useCcsdOpenmp != NWChemToggle_unspecified ||
      ccsd.useTriplesDriverOpenmp != NWChemToggle_unspecified ||
      ccsd.useTriplesDriverOffload != NWChemToggle_unspecified;
  int has_block_content = has_directives || has_print || has_switches ||
                          (include_direct_promoted && has_promoted);
  int has_full_deck_sets = include_direct_promoted && has_direct_toggles;
  if (!has_block_content && !has_full_deck_sets)
    return 0;
  if (has_block_content) {
    if (append_format(block, sizeof(block), "ccsd\n") != 0)
      return -1;
    if (include_direct_promoted && ccsd.maxiter > 0 &&
        append_format(block, sizeof(block), "  maxiter %d\n", ccsd.maxiter) !=
            0)
      return -1;
    if (include_direct_promoted && ccsd.thresh > 0.0 &&
        append_format(block, sizeof(block), "  thresh %.12g\n", ccsd.thresh) !=
            0)
      return -1;
    if (include_direct_promoted && ccsd.tol2e > 0.0 &&
        append_format(block, sizeof(block), "  tol2e %.12g\n", ccsd.tol2e) !=
            0)
      return -1;
    if (include_direct_promoted && ccsd.iprt > 0 &&
        append_format(block, sizeof(block), "  iprt %d\n", ccsd.iprt) != 0)
      return -1;
    if (include_direct_promoted && ccsd.maxDiis > 0 &&
        append_format(block, sizeof(block), "  diisbas %d\n", ccsd.maxDiis) !=
            0)
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
    if (print_level || nprint_items > 0) {
      if (append_format(block, sizeof(block), "  print") != 0)
        return -1;
      if (print_level &&
          append_format(block, sizeof(block), " %s", print_level) != 0)
        return -1;
      if (append_text_args(ccsd.printItems, block, sizeof(block)) != 0 ||
          append_format(block, sizeof(block), "\n") != 0)
        return -1;
    }
    if (nnoprint_items > 0) {
      if (append_format(block, sizeof(block), "  noprint") != 0 ||
          append_text_args(ccsd.noPrintItems, block, sizeof(block)) != 0 ||
          append_format(block, sizeof(block), "\n") != 0)
        return -1;
    }
    if (render_int32_list_directive(ccsd.doa, "doa", block, sizeof(block)) !=
            0 ||
        render_int32_list_directive(ccsd.dob, "dob", block, sizeof(block)) !=
            0 ||
        render_int32_list_directive(ccsd.dog, "dog", block, sizeof(block)) !=
            0 ||
        render_int32_list_directive(ccsd.doh, "doh", block, sizeof(block)) !=
            0 ||
        render_int32_list_directive(ccsd.dojk, "dojk", block, sizeof(block)) !=
            0 ||
        render_int32_list_directive(ccsd.dos, "dos", block, sizeof(block)) !=
            0 ||
        render_int32_list_directive(ccsd.dod, "dod", block, sizeof(block)) !=
            0)
      return -1;
    if (render_directives(ccsd.directives, block, sizeof(block), "  ") != 0 ||
        append_format(block, sizeof(block), "end") != 0)
      return -1;
  }
  if (has_full_deck_sets &&
      (render_set_logical_directive("ccsd:use_trpdrv_nb",
                                    ccsd.useTriplesDriverNonblocking, block,
                                    sizeof(block)) != 0 ||
       render_set_logical_directive("ccsd:use_ccsd_omp", ccsd.useCcsdOpenmp,
                                    block, sizeof(block)) != 0 ||
       render_set_logical_directive("ccsd:use_trpdrv_omp",
                                    ccsd.useTriplesDriverOpenmp, block,
                                    sizeof(block)) != 0 ||
       render_set_logical_directive("ccsd:use_trpdrv_offload",
                                    ccsd.useTriplesDriverOffload, block,
                                    sizeof(block)) != 0))
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

static const char *print_level_keyword(enum NWChemPrintLevel print_level) {
  switch (print_level) {
  case NWChemPrintLevel_none:
    return "none";
  case NWChemPrintLevel_low:
    return "low";
  case NWChemPrintLevel_medium:
    return "medium";
  case NWChemPrintLevel_high:
    return "high";
  case NWChemPrintLevel_debug:
    return "debug";
  case NWChemPrintLevel_unspecified:
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
  const char *print_level = print_level_keyword(tce.printLevel);
  int has_tce_property = tce.dipole || tce.quadrupole || tce.octupole;
  int nprint_items = pointer_list_len(&tce.printItems);
  if (nprint_items < 0)
    return -1;
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
      !(include_direct_promoted && has_tce_property) &&
      !print_level && nprint_items == 0 &&
      !(include_direct_promoted && has_renderable_promoted))
    return 0;
  if (append_format(block, sizeof(block), "tce\n") != 0)
    return -1;
  if (freeze_mode &&
      append_format(block, sizeof(block), "  %s\n", freeze_mode) != 0)
    return -1;
  if (include_direct_promoted && tce.dipole &&
      append_format(block, sizeof(block), "  dipole\n") != 0)
    return -1;
  if (include_direct_promoted && tce.quadrupole &&
      append_format(block, sizeof(block), "  quadrupole\n") != 0)
    return -1;
  if (include_direct_promoted && tce.octupole &&
      append_format(block, sizeof(block), "  octupole\n") != 0)
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
  if (print_level || nprint_items > 0) {
    if (append_format(block, sizeof(block), "  print") != 0)
      return -1;
    if (print_level &&
        append_format(block, sizeof(block), " %s", print_level) != 0)
      return -1;
    for (int i = 0; i < nprint_items; ++i) {
      capn_text item = capn_get_text(tce.printItems, i, empty_text);
      if (item.len <= 0)
        continue;
      if (append_format(block, sizeof(block), " ") != 0 ||
          append_text(block, sizeof(block), item) != 0)
        return -1;
    }
    if (append_format(block, sizeof(block), "\n") != 0)
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
  if (include_direct_promoted && nwpw.cutoffWavefunction > 0.0) {
    if (append_format(block, sizeof(block), "  cutoff %.15g",
                      nwpw.cutoffWavefunction) != 0)
      return -1;
    if (nwpw.cutoffEnergy > 0.0 &&
        append_format(block, sizeof(block), " %.15g", nwpw.cutoffEnergy) != 0)
      return -1;
    if (append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
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
      (nwpw.tolerancesSet || nwpw.toleranceEnergy > 0.0 ||
       nwpw.toleranceDensity > 0.0 ||
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
  if (include_direct_promoted && nwpw.boStepStart > 0) {
    int bo_step_end = nwpw.boStepEnd > 0 ? nwpw.boStepEnd : 100;
    if (append_format(block, sizeof(block), "  bo_steps %d %d\n",
                      nwpw.boStepStart, bo_step_end) != 0)
      return -1;
  }
  if (include_direct_promoted && nwpw.mcStepStart > 0) {
    int mc_step_end = nwpw.mcStepEnd > 0 ? nwpw.mcStepEnd : 100;
    if (append_format(block, sizeof(block), "  mc_steps %d %d\n",
                      nwpw.mcStepStart, mc_step_end) != 0)
      return -1;
  }
  if (include_direct_promoted &&
      (nwpw.boTimeStepSet || nwpw.boTimeStep > 0.0)) {
    double bo_time_step = nwpw.boTimeStep > 0.0 ? nwpw.boTimeStep : 15.0;
    if (append_format(block, sizeof(block), "  bo_time_step %.15g\n",
                      bo_time_step) != 0)
      return -1;
  }
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
  if (include_direct_promoted &&
      (nwpw.boFakeMassSet || nwpw.boFakeMass > 0.0)) {
    double bo_fake_mass = nwpw.boFakeMass > 0.0 ? nwpw.boFakeMass : 500.0;
    if (append_format(block, sizeof(block), "  bo_fake_mass %.15g\n",
                      bo_fake_mass) != 0)
      return -1;
  }
  capn_list32 scaling_atoms = nwpw.scalingAtomIndices;
  capn_resolve(&scaling_atoms.p);
  int n_scaling_atoms = list32_len(scaling_atoms);
  if (n_scaling_atoms < 0)
    return -1;
  if (include_direct_promoted &&
      (nwpw.scalingSet || nwpw.scalingFirst > 0.0 ||
       nwpw.scalingSecond > 0.0 || n_scaling_atoms > 0)) {
    double scaling_first = nwpw.scalingFirst > 0.0 ? nwpw.scalingFirst : 1.0;
    double scaling_second =
        nwpw.scalingSecond > 0.0 ? nwpw.scalingSecond : scaling_first;
    if (append_format(block, sizeof(block), "  scaling %.15g %.15g",
                      scaling_first, scaling_second) != 0)
      return -1;
    for (int i = 0; i < n_scaling_atoms; ++i) {
      int atom_index = (int)(int32_t)capn_get32(scaling_atoms, i);
      if (append_format(block, sizeof(block), " %d", atom_index) != 0)
        return -1;
    }
    if (append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (include_direct_promoted &&
      (nwpw.npDimensionsSet || nwpw.npFftProcesses > 0 ||
       nwpw.npOrbitalProcesses > 0 ||
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
  if (include_direct_promoted && nwpw.fractionalOrbitalsStart > 0) {
    int fractional_end = nwpw.fractionalOrbitalsEnd > 0
                             ? nwpw.fractionalOrbitalsEnd
                             : nwpw.fractionalOrbitalsStart;
    if (append_format(block, sizeof(block), "  fractional_orbitals %d %d\n",
                      nwpw.fractionalOrbitalsStart, fractional_end) != 0)
      return -1;
  }
  capn_list64 occupation_values = nwpw.occupations;
  capn_list32 occupation_states = nwpw.occupationStates;
  capn_resolve(&occupation_values.p);
  capn_resolve(&occupation_states.p);
  int n_occupation_values = list64_len(occupation_values);
  int n_occupation_states = list32_len(occupation_states);
  if (n_occupation_values < 0 || n_occupation_states < 0)
    return -1;
  if (include_direct_promoted && n_occupation_values > 0) {
    if (append_format(block, sizeof(block), "  occupations\n") != 0)
      return -1;
    for (int i = 0; i < n_occupation_values; ++i) {
      double occupation = capn_to_f64(capn_get64(occupation_values, i));
      int state = 1;
      if (i < n_occupation_states) {
        int candidate = (int)(int32_t)capn_get32(occupation_states, i);
        if (candidate > 0)
          state = candidate;
      }
      if (append_format(block, sizeof(block), "    occupation %.15g %d\n",
                        occupation, state) != 0)
        return -1;
    }
    if (nwpw.extraOrbitals > 0 &&
        append_format(block, sizeof(block), "    extra_orbitals %d\n",
                      nwpw.extraOrbitals) != 0)
      return -1;
    if (append_format(block, sizeof(block), "    end\n") != 0)
      return -1;
  }
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
    if (nwpw.fractionalOrbitalsStart > 0) {
      int fractional_end = nwpw.fractionalOrbitalsEnd > 0
                               ? nwpw.fractionalOrbitalsEnd
                               : nwpw.fractionalOrbitalsStart;
      if (append_format(block, sizeof(block), " orbitals %d %d",
                        nwpw.fractionalOrbitalsStart, fractional_end) != 0)
        return -1;
    }
    if (append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  const int has_virtual_alias =
      nwpw.virtualAliasSet || nwpw.virtualAliasStart > 0 ||
      nwpw.virtualAliasEnd > 0;
  if (include_direct_promoted && has_virtual_alias) {
    int virtual_start =
        nwpw.virtualAliasStart > 0 ? nwpw.virtualAliasStart : 3;
    int virtual_end =
        nwpw.virtualAliasEnd > 0 ? nwpw.virtualAliasEnd : virtual_start;
    if (append_format(block, sizeof(block), "  virtual %d %d\n",
                      virtual_start, virtual_end) != 0)
      return -1;
  }
  if (include_direct_promoted && nwpw.virtualOrbitalsStart > 0) {
    int virtual_end = nwpw.virtualOrbitalsEnd > 0
                          ? nwpw.virtualOrbitalsEnd
                          : nwpw.virtualOrbitalsStart;
    if (append_format(block, sizeof(block), "  virtual %d %d\n",
                      nwpw.virtualOrbitalsStart, virtual_end) != 0)
      return -1;
  }
  if (include_direct_promoted &&
      nwpw.lcaoMode == NWChemNwpwLcaoMode_skip &&
      append_format(block, sizeof(block), "  lcao_skip\n") != 0)
    return -1;
  if (include_direct_promoted &&
      nwpw.lcaoMode == NWChemNwpwLcaoMode_lcao &&
      append_format(block, sizeof(block), "  lcao\n") != 0)
    return -1;
  capn_list32 lcao_mask_up_orbitals = nwpw.lcaoMaskUpOrbitals;
  capn_list32 lcao_mask_down_orbitals = nwpw.lcaoMaskDownOrbitals;
  capn_resolve(&lcao_mask_up_orbitals.p);
  capn_resolve(&lcao_mask_down_orbitals.p);
  int n_lcao_mask_up = list32_len(lcao_mask_up_orbitals);
  int n_lcao_mask_down = list32_len(lcao_mask_down_orbitals);
  if (n_lcao_mask_up < 0 || n_lcao_mask_down < 0)
    return -1;
  if (include_direct_promoted &&
      nwpw.lcaoMask == NWChemNwpwToggle_disabled &&
      append_format(block, sizeof(block), "  lcao_mask off\n") != 0)
    return -1;
  if (include_direct_promoted &&
      nwpw.lcaoMask != NWChemNwpwToggle_disabled && n_lcao_mask_up > 0) {
    if (append_format(block, sizeof(block), "  lcao_mask up") != 0 ||
        append_int32_args(lcao_mask_up_orbitals, block, sizeof(block)) != 0 ||
        append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (include_direct_promoted &&
      nwpw.lcaoMask != NWChemNwpwToggle_disabled && n_lcao_mask_down > 0) {
    if (append_format(block, sizeof(block), "  lcao_mask down") != 0 ||
        append_int32_args(lcao_mask_down_orbitals, block, sizeof(block)) !=
            0 ||
        append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (include_direct_promoted && nwpw.ewaldGridX > 0 &&
      append_format(block, sizeof(block), "  ewald_ngrid %d %d %d\n",
                    nwpw.ewaldGridX,
                    nwpw.ewaldGridY > 0 ? nwpw.ewaldGridY : nwpw.ewaldGridX,
                    nwpw.ewaldGridZ > 0 ? nwpw.ewaldGridZ
                                        : (nwpw.ewaldGridY > 0
                                               ? nwpw.ewaldGridY
                                               : nwpw.ewaldGridX)) != 0)
    return -1;
  const int has_temperature =
      nwpw.temperatureIon > 0.0 || nwpw.temperatureIonPeriod > 0.0 ||
      nwpw.temperatureElectron > 0.0 ||
      nwpw.temperatureElectronPeriod > 0.0 ||
      nwpw.temperatureRestart != NWChemNwpwToggle_unspecified ||
      nwpw.temperatureIonChainLength > 0 ||
      nwpw.temperatureElectronChainLength > 0;
  if (include_direct_promoted && has_temperature) {
    double ion_temperature =
        nwpw.temperatureIon > 0.0 ? nwpw.temperatureIon : 298.15;
    double ion_period =
        nwpw.temperatureIonPeriod > 0.0 ? nwpw.temperatureIonPeriod : 1200.0;
    double electron_temperature = nwpw.temperatureElectron > 0.0
                                      ? nwpw.temperatureElectron
                                      : ion_temperature;
    double electron_period = nwpw.temperatureElectronPeriod > 0.0
                                 ? nwpw.temperatureElectronPeriod
                                 : ion_period;
    int ion_chain_length = nwpw.temperatureIonChainLength > 0
                               ? nwpw.temperatureIonChainLength
                               : 1;
    int electron_chain_length =
        nwpw.temperatureElectronChainLength > 0
            ? nwpw.temperatureElectronChainLength
            : ion_chain_length;
    const char *restart = nwpw.temperatureRestart == NWChemNwpwToggle_disabled
                              ? "start"
                              : "restart";
    if (append_format(block, sizeof(block),
                      "  temperature %.15g %.15g %.15g %.15g %s %d %d\n",
                      ion_temperature, ion_period, electron_temperature,
                      electron_period, restart, ion_chain_length,
                      electron_chain_length) != 0)
      return -1;
  }
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
  if (include_direct_promoted &&
      nwpw.atomEfield == NWChemNwpwToggle_enabled &&
      append_format(block, sizeof(block), "  atom_efield\n") != 0)
    return -1;
  if (include_direct_promoted &&
      nwpw.atomEfieldGradient == NWChemNwpwToggle_enabled &&
      append_format(block, sizeof(block), "  atom_efield_grad\n") != 0)
    return -1;
  if (include_direct_promoted &&
      nwpw.mulliken == NWChemNwpwToggle_disabled &&
      append_format(block, sizeof(block), "  mulliken off\n") != 0)
    return -1;
  if (include_direct_promoted &&
      nwpw.mulliken != NWChemNwpwToggle_disabled &&
      nwpw.mullikenKawai == NWChemNwpwToggle_enabled &&
      append_format(block, sizeof(block), "  mulliken kawai\n") != 0)
    return -1;
  if (include_direct_promoted &&
      nwpw.mulliken == NWChemNwpwToggle_enabled &&
      nwpw.mullikenKawai != NWChemNwpwToggle_enabled &&
      append_format(block, sizeof(block), "  mulliken\n") != 0)
    return -1;
  const char *periodic_dipole_logical =
      nwpw_toggle_logical_keyword(nwpw.periodicDipole);
  if (include_direct_promoted && periodic_dipole_logical &&
      append_format(block, sizeof(block), "  periodic_dipole %s\n",
                    periodic_dipole_logical) != 0)
    return -1;
  const int has_efield =
      nwpw.electricField != NWChemNwpwToggle_unspecified ||
      nwpw.electricFieldX != 0.0 || nwpw.electricFieldY != 0.0 ||
      nwpw.electricFieldZ != 0.0 || nwpw.electricFieldCenterSet ||
      nwpw.electricFieldType != NWChemNwpwEfieldType_unspecified;
  if (include_direct_promoted && has_efield) {
    const int efield_enabled =
        nwpw.electricField != NWChemNwpwToggle_disabled;
    if (!efield_enabled) {
      if (append_format(block, sizeof(block), "  efield false\n") != 0)
        return -1;
    } else {
      if (append_format(block, sizeof(block), "  efield true %.15g %.15g %.15g",
                        nwpw.electricFieldX, nwpw.electricFieldY,
                        nwpw.electricFieldZ) != 0)
        return -1;
      if (nwpw.electricFieldCenterSet &&
          append_format(block, sizeof(block), " center %.15g %.15g %.15g",
                        nwpw.electricFieldCenterX, nwpw.electricFieldCenterY,
                        nwpw.electricFieldCenterZ) != 0)
        return -1;
      const char *efield_type =
          nwpw_efield_type_keyword(nwpw.electricFieldType);
      if (efield_type &&
          append_format(block, sizeof(block), " %s", efield_type) != 0)
        return -1;
      if (append_format(block, sizeof(block), "\n") != 0)
        return -1;
    }
  }
  const int has_smooth_cutoff =
      nwpw.smoothCutoff != NWChemNwpwToggle_unspecified ||
      nwpw.smoothCutoffAfac > 0.0 || nwpw.smoothCutoffSigma > 0.0;
  if (include_direct_promoted && has_smooth_cutoff) {
    if (nwpw.smoothCutoff == NWChemNwpwToggle_disabled) {
      if (append_format(block, sizeof(block), "  smooth_cutoff false\n") != 0)
        return -1;
    } else {
      double afac =
          nwpw.smoothCutoffAfac > 0.0 ? nwpw.smoothCutoffAfac : 2.0;
      double sigma =
          nwpw.smoothCutoffSigma > 0.0 ? nwpw.smoothCutoffSigma : 4.0;
      if (append_format(block, sizeof(block),
                        "  smooth_cutoff true %.15g %.15g\n", afac,
                        sigma) != 0)
        return -1;
    }
  }
  const char *cutoff_boot_wavefunction_logical =
      nwpw_toggle_logical_keyword(nwpw.cutoffBootWavefunction);
  if (include_direct_promoted && cutoff_boot_wavefunction_logical &&
      append_format(block, sizeof(block), "  cutoff_boot_wavefunction %s\n",
                    cutoff_boot_wavefunction_logical) != 0)
    return -1;
  const char *fast_erf_logical = nwpw_toggle_logical_keyword(nwpw.fastErf);
  if (include_direct_promoted && fast_erf_logical &&
      append_format(block, sizeof(block), "  fast_erf %s\n",
                    fast_erf_logical) != 0)
    return -1;
  if (include_direct_promoted &&
      (nwpw.dipoleMotion == NWChemNwpwToggle_enabled ||
       nwpw.dipoleMotionFilename.len > 0)) {
    if (append_format(block, sizeof(block), "  dipole_motion") != 0)
      return -1;
    if (nwpw.dipoleMotionFilename.len > 0 &&
        (append_format(block, sizeof(block), " ") != 0 ||
         append_text(block, sizeof(block), nwpw.dipoleMotionFilename) != 0))
      return -1;
    if (append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  const char *rho_use_symmetry_logical =
      nwpw_toggle_logical_keyword(nwpw.rhoUseSymmetry);
  if (include_direct_promoted && rho_use_symmetry_logical &&
      append_format(block, sizeof(block), "  symmetry %s\n",
                    rho_use_symmetry_logical) != 0)
    return -1;
  if (include_direct_promoted && nwpw.oneElectronGuessSet) {
    int it_in =
        nwpw.oneElectronGuessItIn > 0 ? nwpw.oneElectronGuessItIn : 50;
    int it_out =
        nwpw.oneElectronGuessItOut > 0 ? nwpw.oneElectronGuessItOut : 1;
    int it_ortho = nwpw.oneElectronGuessItOrtho > 0
                       ? nwpw.oneElectronGuessItOrtho
                       : 1;
    if (append_format(block, sizeof(block),
                      "  one_electron_guess %d %d %d\n", it_in, it_out,
                      it_ortho) != 0)
      return -1;
  }
  const int has_fmm = nwpw.fmm != NWChemNwpwToggle_unspecified ||
                      nwpw.fmmLmax > 0 || nwpw.fmmLongRange > 0;
  if (include_direct_promoted && has_fmm) {
    const char *fmm_logical =
        nwpw.fmm == NWChemNwpwToggle_disabled ? "false" : "true";
    const int fmm_lmax = nwpw.fmmLmax > 0 ? nwpw.fmmLmax : 10;
    const int fmm_long_range =
        nwpw.fmmLongRange > 0 ? nwpw.fmmLongRange : 1;
    if (append_format(block, sizeof(block), "  fmm %s %d %d\n", fmm_logical,
                      fmm_lmax, fmm_long_range) != 0)
      return -1;
  }
  capn_list64 born_vradii_angstrom = nwpw.bornVRadiiAngstrom;
  capn_resolve(&born_vradii_angstrom.p);
  int nborn_vradii = 0;
  if (born_vradii_angstrom.p.type != CAPN_NULL) {
    if (born_vradii_angstrom.p.type != CAPN_LIST ||
        born_vradii_angstrom.p.datasz != 8)
      return -1;
    nborn_vradii = born_vradii_angstrom.p.len;
  }
  const int has_born = nwpw.born != NWChemNwpwToggle_unspecified ||
                       nwpw.bornDielectric > 0.0 ||
                       nwpw.bornRelax != NWChemNwpwToggle_unspecified ||
                       nborn_vradii > 0;
  if (include_direct_promoted && has_born) {
    const int born_enabled = nwpw.born != NWChemNwpwToggle_disabled;
    if (!born_enabled) {
      if (append_format(block, sizeof(block), "  born false\n") != 0)
        return -1;
    } else {
      if (append_format(block, sizeof(block), "  born") != 0)
        return -1;
      if (nwpw.bornDielectric > 0.0 &&
          append_format(block, sizeof(block), " %.15g",
                        nwpw.bornDielectric) != 0)
        return -1;
      if (nwpw.bornRelax != NWChemNwpwToggle_unspecified) {
        const char *relax =
            nwpw.bornRelax == NWChemNwpwToggle_enabled ? "true" : "false";
        if (append_format(block, sizeof(block), " relax %s", relax) != 0)
          return -1;
      }
      for (int i = 0; i < nborn_vradii; ++i) {
        double radius =
            capn_to_f64(capn_get64(born_vradii_angstrom, i));
        if (append_format(block, sizeof(block), " %.15g", radius) != 0)
          return -1;
      }
      if (append_format(block, sizeof(block), "\n") != 0)
        return -1;
    }
  }
  int nvfield_filenames = pointer_list_len(&nwpw.vfieldFilenames);
  if (nvfield_filenames < 0)
    return -1;
  if (include_direct_promoted && nvfield_filenames > 0) {
    if (append_format(block, sizeof(block), "  vfield") != 0)
      return -1;
    for (int i = 0; i < nvfield_filenames; ++i) {
      capn_text filename = capn_get_text(nwpw.vfieldFilenames, i, empty_text);
      if (filename.len <= 0)
        continue;
      if (append_format(block, sizeof(block), " ") != 0 ||
          append_text(block, sizeof(block), filename) != 0)
        return -1;
    }
    if (append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (include_direct_promoted && nwpw.singlePrecisionHfx &&
      append_format(block, sizeof(block), "  single_precision_hfx\n") != 0)
    return -1;
  if (include_direct_promoted && nwpw.geometryOptimize &&
      append_format(block, sizeof(block), "  geometry_optimize\n") != 0)
    return -1;
  if (include_direct_promoted && nwpw.auxiliaryPotentials &&
      append_format(block, sizeof(block), "  auxiliary_potentials\n") != 0)
    return -1;
  if (include_direct_promoted && nwpw.allowTranslation &&
      append_format(block, sizeof(block), "  allow_translation\n") != 0)
    return -1;
  if (include_direct_promoted && nwpw.etMovecsA.len > 0) {
    if (append_format(block, sizeof(block), "  et ") != 0 ||
        append_text(block, sizeof(block), nwpw.etMovecsA) != 0)
      return -1;
    if (nwpw.etMovecsB.len > 0 &&
        (append_format(block, sizeof(block), " ") != 0 ||
         append_text(block, sizeof(block), nwpw.etMovecsB) != 0))
      return -1;
    if (nwpw.etIonA.len > 0 &&
        (append_format(block, sizeof(block), " ") != 0 ||
         append_text(block, sizeof(block), nwpw.etIonA) != 0))
      return -1;
    if (nwpw.etIonB.len > 0 &&
        (append_format(block, sizeof(block), " ") != 0 ||
         append_text(block, sizeof(block), nwpw.etIonB) != 0))
      return -1;
    if (append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (include_direct_promoted && nwpw.multiplicity > 0 &&
      append_format(block, sizeof(block), "  mult %d\n", nwpw.multiplicity) !=
          0)
    return -1;
  const char *spin_mode = nwpw_spin_mode_keyword(nwpw.spinMode);
  if (include_direct_promoted && spin_mode &&
      append_format(block, sizeof(block), "  %s\n", spin_mode) != 0)
    return -1;
  const int has_dos_scalars = nwpw.dosSet || nwpw.dosAlphaSet ||
                              nwpw.dosNpointsSet || nwpw.dosEminSet ||
                              nwpw.dosEmaxSet;
  if (include_direct_promoted && has_dos_scalars) {
    double alpha =
        nwpw.dosAlphaSet ? nwpw.dosAlpha : nwpw_dos_default_alpha();
    if (append_format(block, sizeof(block), "  dos %.15g", alpha) != 0)
      return -1;
    if (nwpw.dosNpointsSet &&
        append_format(block, sizeof(block), " %d", nwpw.dosNpoints) != 0)
      return -1;
    if (nwpw.dosEminSet &&
        append_format(block, sizeof(block), " %.15g", nwpw.dosEmin) != 0)
      return -1;
    if (nwpw.dosEmaxSet &&
        append_format(block, sizeof(block), " %.15g", nwpw.dosEmax) != 0)
      return -1;
    if (append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (include_direct_promoted && nwpw.dosFilename.len > 0 &&
      (append_format(block, sizeof(block), "  dos_filename ") != 0 ||
       append_text(block, sizeof(block), nwpw.dosFilename) != 0 ||
       append_format(block, sizeof(block), "\n") != 0))
    return -1;
  const char *cpmd_properties_logical =
      nwpw_toggle_logical_keyword(nwpw.cpmdProperties);
  if (include_direct_promoted && cpmd_properties_logical &&
      append_format(block, sizeof(block), "  cpmd_properties %s\n",
                    cpmd_properties_logical) != 0)
    return -1;
  const char *use_grid_comparison_logical =
      nwpw_toggle_logical_keyword(nwpw.useGridComparison);
  if (include_direct_promoted && use_grid_comparison_logical &&
      append_format(block, sizeof(block), "  use_grid_cmp %s\n",
                    use_grid_comparison_logical) != 0)
    return -1;
  if (include_direct_promoted &&
      (nwpw.director != NWChemNwpwToggle_unspecified ||
       nwpw.directorFilename.len > 0)) {
    if (nwpw.director == NWChemNwpwToggle_disabled) {
      if (append_format(block, sizeof(block), "  director false\n") != 0)
        return -1;
    } else {
      if (append_format(block, sizeof(block), "  director") != 0)
        return -1;
      if (nwpw.directorFilename.len > 0 &&
          (append_format(block, sizeof(block), " ") != 0 ||
           append_text(block, sizeof(block), nwpw.directorFilename) != 0))
        return -1;
      if (append_format(block, sizeof(block), "\n") != 0)
        return -1;
    }
  }
  if (include_direct_promoted &&
      (nwpw.cellExpandX > 0 || nwpw.cellExpandY > 0 ||
       nwpw.cellExpandZ > 0) &&
      append_format(block, sizeof(block), "  expand_cell %d %d %d\n",
                    nwpw.cellExpandX > 0 ? nwpw.cellExpandX : 1,
                    nwpw.cellExpandY > 0 ? nwpw.cellExpandY : 1,
                    nwpw.cellExpandZ > 0 ? nwpw.cellExpandZ : 1) != 0)
    return -1;
  const char *mapping_alias =
      nwpw_mapping_alias_keyword(nwpw.mappingAlias);
  if (include_direct_promoted && mapping_alias &&
      append_format(block, sizeof(block), "  %s\n", mapping_alias) != 0)
    return -1;
  if (include_direct_promoted && (nwpw.mappingSet || nwpw.mapping > 0)) {
    int mapping = nwpw.mapping > 0 ? nwpw.mapping : 1;
    if (append_format(block, sizeof(block), "  mapping %d\n", mapping) != 0)
      return -1;
  }
  const char *rotation_logical = nwpw_toggle_logical_keyword(nwpw.rotation);
  if (include_direct_promoted && rotation_logical &&
      append_format(block, sizeof(block), "  rotation %s\n",
                    rotation_logical) != 0)
    return -1;
  if (include_direct_promoted && nwpw.lmaxMultipole >= 0 &&
      append_format(block, sizeof(block), "  integrate_mult_l %d\n",
                    nwpw.lmaxMultipole) != 0)
    return -1;
  if (include_direct_promoted && (nwpw.fei || nwpw.feiFilename.len > 0)) {
    if (append_format(block, sizeof(block), "  Fei") != 0)
      return -1;
    if (nwpw.feiFilename.len > 0 &&
        (append_format(block, sizeof(block), " ") != 0 ||
         append_text(block, sizeof(block), nwpw.feiFilename) != 0))
      return -1;
    if (append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (include_direct_promoted && nwpw.initialVelocitiesTemperature > 0.0) {
    const int seed =
        nwpw.initialVelocitiesSeed >= 0 ? nwpw.initialVelocitiesSeed : 494;
    if (append_format(block, sizeof(block), "  initial_velocities %.15g %d\n",
                      nwpw.initialVelocitiesTemperature, seed) != 0)
      return -1;
  }
  const char *make_hmass2_logical =
      nwpw_toggle_logical_keyword(nwpw.makeHmass2);
  if (include_direct_promoted && make_hmass2_logical &&
      append_format(block, sizeof(block), "  makehmass2 %s\n",
                    make_hmass2_logical) != 0)
    return -1;
  if (include_direct_promoted && nwpw.translateVectorSet) {
    double translate_y =
        nwpw.translateVectorY != 0.0 ? nwpw.translateVectorY
                                     : nwpw.translateVectorX;
    double translate_z =
        nwpw.translateVectorZ != 0.0 ? nwpw.translateVectorZ : translate_y;
    if (append_format(block, sizeof(block), "  translate_vector %.15g %.15g %.15g",
                      nwpw.translateVectorX, translate_y, translate_z) != 0)
      return -1;
    if (nwpw.translateGeometryName.len > 0 &&
        (append_format(block, sizeof(block), " ") != 0 ||
         append_text(block, sizeof(block), nwpw.translateGeometryName) != 0))
      return -1;
    if (nwpw.translateReorder != NWChemNwpwToggle_unspecified) {
      const char *reorder =
          nwpw.translateReorder == NWChemNwpwToggle_enabled ? "reorder"
                                                            : "noreorder";
      if (append_format(block, sizeof(block), " %s", reorder) != 0)
        return -1;
    }
    if (append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  if (include_direct_promoted && nwpw.socketType.len > 0) {
    if (append_format(block, sizeof(block), "  socket ") != 0 ||
        append_text(block, sizeof(block), nwpw.socketType) != 0)
      return -1;
    if (nwpw.socketIp.len > 0 &&
        (append_format(block, sizeof(block), " ") != 0 ||
         append_text(block, sizeof(block), nwpw.socketIp) != 0))
      return -1;
    if (append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  capn_list64 apc_gamma = nwpw.apcGamma;
  capn_resolve(&apc_gamma.p);
  int napc_gamma = 0;
  if (apc_gamma.p.type != CAPN_NULL) {
    if (apc_gamma.p.type != CAPN_LIST || apc_gamma.p.datasz != 8)
      return -1;
    napc_gamma = apc_gamma.p.len;
  }
  if (include_direct_promoted && nwpw.apcSet && napc_gamma > 0) {
    if (append_format(block, sizeof(block), "  apc %.15g", nwpw.apcGc) != 0)
      return -1;
    for (int i = 0; i < napc_gamma; ++i) {
      double gamma = capn_to_f64(capn_get64(apc_gamma, i));
      if (append_format(block, sizeof(block), " %.15g", gamma) != 0)
        return -1;
    }
    if (append_format(block, sizeof(block), "\n") != 0)
      return -1;
  }
  const char *translation_logical =
      nwpw_toggle_logical_keyword(nwpw.translation);
  if (include_direct_promoted && translation_logical &&
      append_format(block, sizeof(block), "  translation %s\n",
                    translation_logical) != 0)
    return -1;
  const char *minimizer_command =
      nwpw_minimizer_command(nwpw.minimizer);
  const char *minimizer_variant =
      nwpw_minimizer_variant(nwpw.minimizer);
  const char *ks_algorithm = nwpw_ks_algorithm_keyword(nwpw.ksAlgorithm);
  const char *scf_algorithm = nwpw_scf_algorithm_keyword(nwpw.scfAlgorithm);
  const char *precondition = nwpw_precondition_keyword(nwpw.precondition);
  const int has_scf_numeric =
      nwpw.kerkerG0Set || nwpw.ksAlphaSet || nwpw.ksMaxitOrbSet ||
      nwpw.ksMaxitOrbsSet || nwpw.diisHistoriesSet;
  const int has_scf_options =
      ks_algorithm || scf_algorithm || precondition ||
      has_scf_numeric ||
      nwpw_minimizer_is_scf(nwpw.minimizer);
  if (include_direct_promoted && has_scf_options) {
    const char *scf_variant = nwpw_minimizer_is_scf(nwpw.minimizer)
                                  ? minimizer_variant
                                  : "density";
    if (append_format(block, sizeof(block), "  scf %s", scf_variant) != 0)
      return -1;
    if (ks_algorithm &&
        append_format(block, sizeof(block), " %s", ks_algorithm) != 0)
      return -1;
    if (scf_algorithm &&
        append_format(block, sizeof(block), " %s", scf_algorithm) != 0)
      return -1;
    if (precondition &&
        append_format(block, sizeof(block), " %s", precondition) != 0)
      return -1;
    if (nwpw.kerkerG0Set &&
        append_format(block, sizeof(block), " kerker %.15g",
                      nwpw.kerkerG0) != 0)
      return -1;
    if (nwpw.ksAlphaSet &&
        append_format(block, sizeof(block), " alpha %.15g",
                      nwpw.ksAlpha) != 0)
      return -1;
    if (nwpw.ksMaxitOrbSet &&
        append_format(block, sizeof(block), " iterations %d",
                      nwpw.ksMaxitOrb) != 0)
      return -1;
    if (nwpw.ksMaxitOrbsSet &&
        append_format(block, sizeof(block), " outer_iterations %d",
                      nwpw.ksMaxitOrbs) != 0)
      return -1;
    if (nwpw.diisHistoriesSet &&
        append_format(block, sizeof(block), " diis_histories %d",
                      nwpw.diisHistories) != 0)
      return -1;
    if (append_format(block, sizeof(block), "\n") != 0)
      return -1;
  } else if (include_direct_promoted && minimizer_command &&
             minimizer_variant &&
             append_format(block, sizeof(block), "  %s %s\n",
                           minimizer_command, minimizer_variant) != 0) {
    return -1;
  }
  if (render_directives(nwpw.directives, block, sizeof(block), "  ") != 0)
    return -1;
  if (!include_direct_promoted && strcmp(block, "nwpw\n") == 0)
    return 0;
  if (append_format(block, sizeof(block), "end") != 0)
    return -1;
  const char *atom_efield_logical =
      nwpw_toggle_logical_keyword(nwpw.atomEfield);
  const char *atom_efield_grad_logical =
      nwpw_toggle_logical_keyword(nwpw.atomEfieldGradient);
  if (include_direct_promoted &&
      nwpw.atomEfield == NWChemNwpwToggle_disabled && atom_efield_logical &&
      append_format(block, sizeof(block),
                    "\nset nwpw:atom_efield logical %s",
                    atom_efield_logical) != 0)
    return -1;
  if (include_direct_promoted &&
      nwpw.atomEfieldGradient == NWChemNwpwToggle_disabled &&
      atom_efield_grad_logical &&
      append_format(block, sizeof(block),
                    "\nset nwpw:atom_efield_grad logical %s",
                    atom_efield_grad_logical) != 0)
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
    case NWChemInputStanza_Kind_brillouinZone:
      if (render_brillouin_zone_stanza(stanza.brillouinZone, dst, dst_size,
                                       include_direct_promoted_nwpw) != 0)
        return -1;
      break;
    case NWChemInputStanza_Kind_simulationCell:
      if (render_simulation_cell_stanza(stanza.simulationCell, dst, dst_size,
                                        include_direct_promoted_nwpw) != 0)
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
    double *opposite_spin_scale, int *use_trpdrv_nb, int *use_ccsd_omp,
    int *use_trpdrv_omp, int *use_trpdrv_offload) {
  if (params.p.type == CAPN_NULL || !has_options || !maxiter || !thresh ||
      !tol2e || !iprt || !max_diis || !frozen_core || !frozen_virtual ||
      !use_disk || !same_spin_scale || !opposite_spin_scale ||
      !use_trpdrv_nb || !use_ccsd_omp || !use_trpdrv_omp ||
      !use_trpdrv_offload)
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
  *use_trpdrv_nb = NWChemToggle_unspecified;
  *use_ccsd_omp = NWChemToggle_unspecified;
  *use_trpdrv_omp = NWChemToggle_unspecified;
  *use_trpdrv_offload = NWChemToggle_unspecified;

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
    if (ccsd.useTriplesDriverNonblocking != NWChemToggle_unspecified) {
      *has_options = 1;
      *use_trpdrv_nb = ccsd.useTriplesDriverNonblocking;
    }
    if (ccsd.useCcsdOpenmp != NWChemToggle_unspecified) {
      *has_options = 1;
      *use_ccsd_omp = ccsd.useCcsdOpenmp;
    }
    if (ccsd.useTriplesDriverOpenmp != NWChemToggle_unspecified) {
      *has_options = 1;
      *use_trpdrv_omp = ccsd.useTriplesDriverOpenmp;
    }
    if (ccsd.useTriplesDriverOffload != NWChemToggle_unspecified) {
      *has_options = 1;
      *use_trpdrv_offload = ccsd.useTriplesDriverOffload;
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
    if (nwpw.cutoffWavefunction > 0.0) {
      *has_options = 1;
      *wavefunction_cutoff = nwpw.cutoffWavefunction;
      *energy_cutoff =
          nwpw.cutoffEnergy > 0.0 ? nwpw.cutoffEnergy
                                  : 2.0 * nwpw.cutoffWavefunction;
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
    if (nwpw.tolerancesSet || nwpw.toleranceEnergy > 0.0 ||
        nwpw.toleranceDensity > 0.0 ||
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

int nwchemc_params_extract_direct_nwpw_bo_with_scaling_atoms(
    NWChemParams_ptr params, int *has_options, int *balance_mode,
    int *bo_step_start, int *bo_step_end, double *bo_time_step,
    int *bo_algorithm, double *bo_fake_mass, int *has_scaling,
    double *scaling_first, double *scaling_second, int *scaling_atoms,
    size_t scaling_atoms_capacity, size_t *scaling_atom_count) {
  if (params.p.type == CAPN_NULL || !has_options || !balance_mode ||
      !bo_step_start || !bo_step_end || !bo_time_step || !bo_algorithm ||
      !bo_fake_mass || !has_scaling || !scaling_first || !scaling_second ||
      !scaling_atom_count)
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
  *scaling_atom_count = 0;

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
    if (nwpw.boStepStart > 0) {
      *has_options = 1;
      *bo_step_start = nwpw.boStepStart;
      *bo_step_end = nwpw.boStepEnd > 0 ? nwpw.boStepEnd : 100;
    }
    if (nwpw.mcStepStart > 0) {
      *has_options = 1;
      *bo_step_start = nwpw.mcStepStart;
      *bo_step_end = nwpw.mcStepEnd > 0 ? nwpw.mcStepEnd : 100;
    }
    if (nwpw.boTimeStepSet || nwpw.boTimeStep > 0.0) {
      *has_options = 1;
      *bo_time_step = nwpw.boTimeStep > 0.0 ? nwpw.boTimeStep : 15.0;
    }
    if (nwpw.boAlgorithm != NWChemNwpwBoAlgorithm_unspecified) {
      *has_options = 1;
      *bo_algorithm = nwpw.boAlgorithm;
    }
    if (nwpw.boFakeMassSet || nwpw.boFakeMass > 0.0) {
      *has_options = 1;
      *bo_fake_mass = nwpw.boFakeMass > 0.0 ? nwpw.boFakeMass : 500.0;
    }
    capn_list32 scaling_atom_indices = nwpw.scalingAtomIndices;
    capn_resolve(&scaling_atom_indices.p);
    int n_scaling_atom_indices = list32_len(scaling_atom_indices);
    if (n_scaling_atom_indices < 0)
      return -1;
    if (nwpw.scalingSet || nwpw.scalingFirst > 0.0 ||
        nwpw.scalingSecond > 0.0 || n_scaling_atom_indices > 0) {
      *has_options = 1;
      *has_scaling = 1;
      *scaling_first = nwpw.scalingFirst > 0.0 ? nwpw.scalingFirst : 1.0;
      *scaling_second =
          nwpw.scalingSecond > 0.0 ? nwpw.scalingSecond : *scaling_first;
      if (n_scaling_atom_indices > 0) {
        *scaling_atom_count = (size_t)n_scaling_atom_indices;
        if (scaling_atoms) {
          if ((size_t)n_scaling_atom_indices > scaling_atoms_capacity)
            return -1;
          for (int j = 0; j < n_scaling_atom_indices; ++j)
            scaling_atoms[j] =
                (int)(int32_t)capn_get32(scaling_atom_indices, j);
        }
      }
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_bo(
    NWChemParams_ptr params, int *has_options, int *balance_mode,
    int *bo_step_start, int *bo_step_end, double *bo_time_step,
    int *bo_algorithm, double *bo_fake_mass, int *has_scaling,
    double *scaling_first, double *scaling_second) {
  size_t scaling_atom_count = 0;
  return nwchemc_params_extract_direct_nwpw_bo_with_scaling_atoms(
      params, has_options, balance_mode, bo_step_start, bo_step_end,
      bo_time_step, bo_algorithm, bo_fake_mass, has_scaling, scaling_first,
      scaling_second, NULL, 0, &scaling_atom_count);
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
    if (nwpw.npDimensionsSet || nwpw.npFftProcesses > 0 ||
        nwpw.npOrbitalProcesses > 0 ||
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
    if (nwpw.fractionalOrbitalsStart > 0) {
      *has_fractional = 1;
      *fractional_orbitals_start = nwpw.fractionalOrbitalsStart;
      *fractional_orbitals_end = nwpw.fractionalOrbitalsEnd > 0
                                     ? nwpw.fractionalOrbitalsEnd
                                     : nwpw.fractionalOrbitalsStart;
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
    if (nwpw.virtualAliasSet || nwpw.virtualAliasStart > 0 ||
        nwpw.virtualAliasEnd > 0) {
      *has_options = 1;
      *virtual_orbitals_start =
          nwpw.virtualAliasStart > 0 ? nwpw.virtualAliasStart : 3;
      *virtual_orbitals_end =
          nwpw.virtualAliasEnd > 0 ? nwpw.virtualAliasEnd
                                   : *virtual_orbitals_start;
    }
    if (nwpw.virtualOrbitalsStart > 0) {
      *has_options = 1;
      *virtual_orbitals_start = nwpw.virtualOrbitalsStart;
      *virtual_orbitals_end = nwpw.virtualOrbitalsEnd > 0
                                   ? nwpw.virtualOrbitalsEnd
                                   : nwpw.virtualOrbitalsStart;
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

int nwchemc_params_extract_direct_nwpw_lcao_mask(
    NWChemParams_ptr params, int *has_options, int *lcao_mask,
    int *up_orbitals, size_t up_capacity, size_t *up_count,
    int *down_orbitals, size_t down_capacity, size_t *down_count) {
  if (params.p.type == CAPN_NULL || !has_options || !lcao_mask ||
      !up_count || !down_count)
    return -1;

  *has_options = 0;
  *lcao_mask = NWChemNwpwToggle_unspecified;
  *up_count = 0;
  *down_count = 0;

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
    capn_list32 lcao_mask_up_orbitals = nwpw.lcaoMaskUpOrbitals;
    capn_list32 lcao_mask_down_orbitals = nwpw.lcaoMaskDownOrbitals;
    capn_resolve(&lcao_mask_up_orbitals.p);
    capn_resolve(&lcao_mask_down_orbitals.p);
    int nup = list32_len(lcao_mask_up_orbitals);
    int ndown = list32_len(lcao_mask_down_orbitals);
    if (nup < 0 || ndown < 0)
      return -1;

    if (nwpw.lcaoMask != NWChemNwpwToggle_unspecified) {
      *has_options = 1;
      *lcao_mask = nwpw.lcaoMask;
    }

    if (nup > 0) {
      if (!up_orbitals || (size_t)nup > up_capacity)
        return -1;
      *has_options = 1;
      *up_count = (size_t)nup;
      if (*lcao_mask == NWChemNwpwToggle_unspecified)
        *lcao_mask = NWChemNwpwToggle_enabled;
      for (int j = 0; j < nup; ++j)
        up_orbitals[j] = (int)(int32_t)capn_get32(lcao_mask_up_orbitals, j);
    }

    if (ndown > 0) {
      if (!down_orbitals || (size_t)ndown > down_capacity)
        return -1;
      *has_options = 1;
      *down_count = (size_t)ndown;
      if (*lcao_mask == NWChemNwpwToggle_unspecified)
        *lcao_mask = NWChemNwpwToggle_enabled;
      for (int j = 0; j < ndown; ++j)
        down_orbitals[j] =
            (int)(int32_t)capn_get32(lcao_mask_down_orbitals, j);
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_occupations(
    NWChemParams_ptr params, int *has_options, double *occupations,
    int *states, size_t capacity, size_t *count, int *extra_orbitals) {
  if (params.p.type == CAPN_NULL || !has_options || !count ||
      !extra_orbitals)
    return -1;

  *has_options = 0;
  *count = 0;
  *extra_orbitals = 0;

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
    capn_list64 occupation_values = nwpw.occupations;
    capn_list32 occupation_states = nwpw.occupationStates;
    capn_resolve(&occupation_values.p);
    capn_resolve(&occupation_states.p);
    int nvalues = list64_len(occupation_values);
    int nstates = list32_len(occupation_states);
    if (nvalues < 0 || nstates < 0)
      return -1;

    if (nwpw.extraOrbitals > 0) {
      *has_options = 1;
      *extra_orbitals = nwpw.extraOrbitals;
    }
    if (nvalues <= 0)
      continue;
    if (!occupations || !states || (size_t)nvalues > capacity)
      return -1;

    *has_options = 1;
    *count = (size_t)nvalues;
    for (int j = 0; j < nvalues; ++j) {
      int state = 1;
      if (j < nstates) {
        int candidate = (int)(int32_t)capn_get32(occupation_states, j);
        if (candidate > 0)
          state = candidate;
      }
      occupations[j] = capn_to_f64(capn_get64(occupation_values, j));
      states[j] = state;
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
    int has_nose = nwpw.noseHoover != NWChemNwpwToggle_unspecified ||
                   nwpw.noseRestart != NWChemNwpwToggle_unspecified ||
                   nwpw.noseElectronPeriod > 0.0 ||
                   nwpw.noseElectronTemperature > 0.0 ||
                   nwpw.noseIonPeriod > 0.0 ||
                   nwpw.noseIonTemperature > 0.0 ||
                   nwpw.noseElectronChainLength > 0 ||
                   nwpw.noseIonChainLength > 0;
    int has_temperature =
        nwpw.temperatureIon > 0.0 || nwpw.temperatureIonPeriod > 0.0 ||
        nwpw.temperatureElectron > 0.0 ||
        nwpw.temperatureElectronPeriod > 0.0 ||
        nwpw.temperatureRestart != NWChemNwpwToggle_unspecified ||
        nwpw.temperatureIonChainLength > 0 ||
        nwpw.temperatureElectronChainLength > 0;
    if (!has_nose && !has_temperature)
      continue;

    *has_options = 1;
    if (has_temperature) {
      double ion_temp =
          nwpw.temperatureIon > 0.0 ? nwpw.temperatureIon : 298.15;
      double ion_period_value =
          nwpw.temperatureIonPeriod > 0.0 ? nwpw.temperatureIonPeriod : 1200.0;
      int ion_chain = nwpw.temperatureIonChainLength > 0
                          ? nwpw.temperatureIonChainLength
                          : 1;
      *nose_hoover = NWChemNwpwToggle_enabled;
      *nose_restart =
          nwpw.temperatureRestart == NWChemNwpwToggle_unspecified
              ? NWChemNwpwToggle_enabled
              : nwpw.temperatureRestart;
      *ion_temperature = ion_temp;
      *ion_period = ion_period_value;
      *electron_temperature =
          nwpw.temperatureElectron > 0.0 ? nwpw.temperatureElectron : ion_temp;
      *electron_period = nwpw.temperatureElectronPeriod > 0.0
                             ? nwpw.temperatureElectronPeriod
                             : ion_period_value;
      *ion_chain_length = ion_chain;
      *electron_chain_length = nwpw.temperatureElectronChainLength > 0
                                   ? nwpw.temperatureElectronChainLength
                                   : ion_chain;
      continue;
    }

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

int nwchemc_params_extract_direct_nwpw_electric_field(
    NWChemParams_ptr params, int *has_options, int *atom_efield,
    int *atom_efield_gradient) {
  if (params.p.type == CAPN_NULL || !has_options || !atom_efield ||
      !atom_efield_gradient)
    return -1;

  *has_options = 0;
  *atom_efield = NWChemNwpwToggle_unspecified;
  *atom_efield_gradient = NWChemNwpwToggle_unspecified;

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
    if (nwpw.atomEfield != NWChemNwpwToggle_unspecified) {
      *has_options = 1;
      *atom_efield = nwpw.atomEfield;
    }
    if (nwpw.atomEfieldGradient != NWChemNwpwToggle_unspecified) {
      *has_options = 1;
      *atom_efield_gradient = nwpw.atomEfieldGradient;
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_mulliken(
    NWChemParams_ptr params, int *has_options, int *mulliken,
    int *mulliken_kawai) {
  if (params.p.type == CAPN_NULL || !has_options || !mulliken ||
      !mulliken_kawai)
    return -1;

  *has_options = 0;
  *mulliken = NWChemNwpwToggle_unspecified;
  *mulliken_kawai = NWChemNwpwToggle_unspecified;

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
    if (nwpw.mulliken != NWChemNwpwToggle_unspecified) {
      *has_options = 1;
      *mulliken = nwpw.mulliken;
    }
    if (nwpw.mullikenKawai != NWChemNwpwToggle_unspecified) {
      *has_options = 1;
      *mulliken_kawai = nwpw.mullikenKawai;
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_periodic_dipole(
    NWChemParams_ptr params, int *has_options, int *periodic_dipole) {
  if (params.p.type == CAPN_NULL || !has_options || !periodic_dipole)
    return -1;

  *has_options = 0;
  *periodic_dipole = NWChemNwpwToggle_unspecified;

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
    if (nwpw.periodicDipole != NWChemNwpwToggle_unspecified) {
      *has_options = 1;
      *periodic_dipole = nwpw.periodicDipole;
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_efield(
    NWChemParams_ptr params, int *has_options, int *efield,
    double efield_vector[3], int *has_center, double efield_center[3],
    int *efield_type) {
  if (params.p.type == CAPN_NULL || !has_options || !efield ||
      !efield_vector || !has_center || !efield_center || !efield_type)
    return -1;

  *has_options = 0;
  *efield = NWChemNwpwToggle_unspecified;
  efield_vector[0] = 0.0;
  efield_vector[1] = 0.0;
  efield_vector[2] = 0.0;
  *has_center = 0;
  efield_center[0] = 0.0;
  efield_center[1] = 0.0;
  efield_center[2] = 0.0;
  *efield_type = NWChemNwpwEfieldType_unspecified;

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
    const int has_efield =
        nwpw.electricField != NWChemNwpwToggle_unspecified ||
        nwpw.electricFieldX != 0.0 || nwpw.electricFieldY != 0.0 ||
        nwpw.electricFieldZ != 0.0 || nwpw.electricFieldCenterSet ||
        nwpw.electricFieldType != NWChemNwpwEfieldType_unspecified;
    if (!has_efield)
      continue;

    *has_options = 1;
    *efield = nwpw.electricField == NWChemNwpwToggle_unspecified
                  ? NWChemNwpwToggle_enabled
                  : nwpw.electricField;
    efield_vector[0] = nwpw.electricFieldX;
    efield_vector[1] = nwpw.electricFieldY;
    efield_vector[2] = nwpw.electricFieldZ;
    *has_center = nwpw.electricFieldCenterSet ? 1 : 0;
    if (*has_center) {
      efield_center[0] = nwpw.electricFieldCenterX;
      efield_center[1] = nwpw.electricFieldCenterY;
      efield_center[2] = nwpw.electricFieldCenterZ;
    }
    *efield_type = nwpw.electricFieldType;
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_smooth_cutoff(
    NWChemParams_ptr params, int *has_options, int *smooth_cutoff,
    double values[2]) {
  if (params.p.type == CAPN_NULL || !has_options || !smooth_cutoff || !values)
    return -1;

  *has_options = 0;
  *smooth_cutoff = NWChemNwpwToggle_unspecified;
  values[0] = 0.0;
  values[1] = 0.0;

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
    const int has_smooth_cutoff =
        nwpw.smoothCutoff != NWChemNwpwToggle_unspecified ||
        nwpw.smoothCutoffAfac > 0.0 || nwpw.smoothCutoffSigma > 0.0;
    if (!has_smooth_cutoff)
      continue;

    *has_options = 1;
    *smooth_cutoff = nwpw.smoothCutoff == NWChemNwpwToggle_unspecified
                         ? NWChemNwpwToggle_enabled
                         : nwpw.smoothCutoff;
    values[0] = nwpw.smoothCutoffAfac > 0.0 ? nwpw.smoothCutoffAfac : 2.0;
    values[1] =
        nwpw.smoothCutoffSigma > 0.0 ? nwpw.smoothCutoffSigma : 4.0;
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_cutoff_boot_wavefunction(
    NWChemParams_ptr params, int *has_options,
    int *cutoff_boot_wavefunction) {
  if (params.p.type == CAPN_NULL || !has_options ||
      !cutoff_boot_wavefunction)
    return -1;

  *has_options = 0;
  *cutoff_boot_wavefunction = NWChemNwpwToggle_unspecified;

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
    if (nwpw.cutoffBootWavefunction != NWChemNwpwToggle_unspecified) {
      *has_options = 1;
      *cutoff_boot_wavefunction = nwpw.cutoffBootWavefunction;
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_fast_erf(NWChemParams_ptr params,
                                                int *has_options,
                                                int *fast_erf) {
  if (params.p.type == CAPN_NULL || !has_options || !fast_erf)
    return -1;

  *has_options = 0;
  *fast_erf = NWChemNwpwToggle_unspecified;

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
    if (nwpw.fastErf != NWChemNwpwToggle_unspecified) {
      *has_options = 1;
      *fast_erf = nwpw.fastErf;
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_dipole_motion(
    NWChemParams_ptr params, int *has_options, int *dipole_motion,
    capn_text *filename) {
  if (params.p.type == CAPN_NULL || !has_options || !dipole_motion ||
      !filename)
    return -1;

  *has_options = 0;
  *dipole_motion = NWChemNwpwToggle_unspecified;
  filename->str = NULL;
  filename->len = 0;
  filename->seg = NULL;

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
    const int has_dipole_motion =
        nwpw.dipoleMotion != NWChemNwpwToggle_unspecified ||
        nwpw.dipoleMotionFilename.len > 0;
    if (!has_dipole_motion)
      continue;

    *has_options = 1;
    *dipole_motion = nwpw.dipoleMotion == NWChemNwpwToggle_unspecified
                         ? NWChemNwpwToggle_enabled
                         : nwpw.dipoleMotion;
    *filename = nwpw.dipoleMotionFilename;
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_rho_use_symmetry(
    NWChemParams_ptr params, int *has_options, int *rho_use_symmetry) {
  if (params.p.type == CAPN_NULL || !has_options || !rho_use_symmetry)
    return -1;

  *has_options = 0;
  *rho_use_symmetry = NWChemNwpwToggle_unspecified;

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
    if (nwpw.rhoUseSymmetry != NWChemNwpwToggle_unspecified) {
      *has_options = 1;
      *rho_use_symmetry = nwpw.rhoUseSymmetry;
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_one_electron_guess(
    NWChemParams_ptr params, int *has_options, int *it_in, int *it_out,
    int *it_ortho) {
  if (params.p.type == CAPN_NULL || !has_options || !it_in || !it_out ||
      !it_ortho)
    return -1;

  *has_options = 0;
  *it_in = 0;
  *it_out = 0;
  *it_ortho = 0;

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
    if (!nwpw.oneElectronGuessSet)
      continue;

    *has_options = 1;
    *it_in = nwpw.oneElectronGuessItIn > 0 ? nwpw.oneElectronGuessItIn : 50;
    *it_out = nwpw.oneElectronGuessItOut > 0 ? nwpw.oneElectronGuessItOut : 1;
    *it_ortho =
        nwpw.oneElectronGuessItOrtho > 0 ? nwpw.oneElectronGuessItOrtho : 1;
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_fmm(NWChemParams_ptr params,
                                           int *has_options, int *fmm,
                                           int *fmm_lmax,
                                           int *fmm_long_range) {
  if (params.p.type == CAPN_NULL || !has_options || !fmm || !fmm_lmax ||
      !fmm_long_range)
    return -1;

  *has_options = 0;
  *fmm = NWChemNwpwToggle_unspecified;
  *fmm_lmax = 0;
  *fmm_long_range = 0;

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
    const int has_fmm = nwpw.fmm != NWChemNwpwToggle_unspecified ||
                        nwpw.fmmLmax > 0 || nwpw.fmmLongRange > 0;
    if (!has_fmm)
      continue;

    *has_options = 1;
    *fmm = nwpw.fmm == NWChemNwpwToggle_unspecified ? NWChemNwpwToggle_enabled
                                                    : nwpw.fmm;
    *fmm_lmax = nwpw.fmmLmax > 0 ? nwpw.fmmLmax : 10;
    *fmm_long_range = nwpw.fmmLongRange > 0 ? nwpw.fmmLongRange : 1;
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_born(
    NWChemParams_ptr params, int *has_options, int *born, double *dielectric,
    int *relax, double *vradii_angstrom, size_t vradii_capacity,
    size_t *vradii_count) {
  if (params.p.type == CAPN_NULL || !has_options || !born || !dielectric ||
      !relax || !vradii_count)
    return -1;

  *has_options = 0;
  *born = NWChemNwpwToggle_unspecified;
  *dielectric = 0.0;
  *relax = NWChemNwpwToggle_unspecified;
  *vradii_count = 0;

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
    capn_list64 born_vradii = nwpw.bornVRadiiAngstrom;
    capn_resolve(&born_vradii.p);
    int nradii = 0;
    if (born_vradii.p.type != CAPN_NULL) {
      if (born_vradii.p.type != CAPN_LIST || born_vradii.p.datasz != 8)
        return -1;
      nradii = born_vradii.p.len;
    }
    const int has_born = nwpw.born != NWChemNwpwToggle_unspecified ||
                         nwpw.bornDielectric > 0.0 ||
                         nwpw.bornRelax != NWChemNwpwToggle_unspecified ||
                         nradii > 0;
    if (!has_born)
      continue;

    *has_options = 1;
    *born = nwpw.born == NWChemNwpwToggle_unspecified
                ? NWChemNwpwToggle_enabled
                : nwpw.born;
    *dielectric = nwpw.bornDielectric;
    *relax = nwpw.bornRelax;
    *vradii_count = (size_t)nradii;
    if (nradii > 0 && vradii_angstrom) {
      if ((size_t)nradii > vradii_capacity)
        return -1;
      for (int j = 0; j < nradii; ++j)
        vradii_angstrom[j] = capn_to_f64(capn_get64(born_vradii, j));
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_vfield(
    NWChemParams_ptr params, int *has_options, capn_text *filenames,
    size_t filename_capacity, size_t *filename_count) {
  if (params.p.type == CAPN_NULL || !has_options || !filename_count)
    return -1;

  *has_options = 0;
  *filename_count = 0;

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
    int nfilenames = pointer_list_len(&nwpw.vfieldFilenames);
    if (nfilenames < 0)
      return -1;
    if (nfilenames == 0)
      continue;

    *has_options = 1;
    *filename_count = (size_t)nfilenames;
    if (filenames) {
      if ((size_t)nfilenames > filename_capacity)
        return -1;
      for (int j = 0; j < nfilenames; ++j)
        filenames[j] = capn_get_text(nwpw.vfieldFilenames, j, empty_text);
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_single_precision_hfx(
    NWChemParams_ptr params, int *has_options, int *single_precision_hfx) {
  if (params.p.type == CAPN_NULL || !has_options || !single_precision_hfx)
    return -1;

  *has_options = 0;
  *single_precision_hfx = 0;

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
    if (!nwpw.singlePrecisionHfx)
      continue;

    *has_options = 1;
    *single_precision_hfx = 1;
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_geometry_optimize(
    NWChemParams_ptr params, int *has_options, int *geometry_optimize) {
  if (params.p.type == CAPN_NULL || !has_options || !geometry_optimize)
    return -1;

  *has_options = 0;
  *geometry_optimize = 0;

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
    if (!nwpw.geometryOptimize)
      continue;

    *has_options = 1;
    *geometry_optimize = 1;
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_auxiliary_potentials(
    NWChemParams_ptr params, int *has_options, int *auxiliary_potentials) {
  if (params.p.type == CAPN_NULL || !has_options || !auxiliary_potentials)
    return -1;

  *has_options = 0;
  *auxiliary_potentials = 0;

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
    if (!nwpw.auxiliaryPotentials)
      continue;

    *has_options = 1;
    *auxiliary_potentials = 1;
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_multiplicity(
    NWChemParams_ptr params, int *has_options, int *multiplicity, int *ispin) {
  if (params.p.type == CAPN_NULL || !has_options || !multiplicity || !ispin)
    return -1;

  *has_options = 0;
  *multiplicity = 0;
  *ispin = 0;

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
    if (nwpw.multiplicity <= 0)
      continue;

    *has_options = 1;
    *multiplicity = nwpw.multiplicity;
    *ispin = nwpw.multiplicity > 1 ? 2 : 1;
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_allow_translation(
    NWChemParams_ptr params, int *has_options, int *allow_translation) {
  if (params.p.type == CAPN_NULL || !has_options || !allow_translation)
    return -1;

  *has_options = 0;
  *allow_translation = 0;

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
    if (!nwpw.allowTranslation)
      continue;

    *has_options = 1;
    *allow_translation = 1;
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_spin_mode(NWChemParams_ptr params,
                                                 int *has_options,
                                                 int *spin_mode, int *ispin) {
  if (params.p.type == CAPN_NULL || !has_options || !spin_mode || !ispin)
    return -1;

  *has_options = 0;
  *spin_mode = NWChemNwpwSpinMode_unspecified;
  *ispin = 0;

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
    int current_ispin = nwpw_spin_mode_ispin(nwpw.spinMode);
    if (current_ispin == 0)
      continue;

    *has_options = 1;
    *spin_mode = nwpw.spinMode;
    *ispin = current_ispin;
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_spin_ispins(
    NWChemParams_ptr params, int *ispins, size_t capacity, size_t *count) {
  if (params.p.type == CAPN_NULL || !ispins || !count)
    return -1;

  *count = 0;

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
    int current_ispin = nwpw_spin_mode_ispin(nwpw.spinMode);
    if (current_ispin == 0)
      continue;
    if (*count >= capacity)
      return -1;
    ispins[*count] = current_ispin;
    ++(*count);
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_dos(
    NWChemParams_ptr params, int *has_options, int *dos_alpha_set,
    double *dos_alpha, int *dos_npoints_set, int *dos_npoints,
    int *dos_emin_set, double *dos_emin, int *dos_emax_set, double *dos_emax,
    capn_text *dos_filename) {
  if (params.p.type == CAPN_NULL || !has_options || !dos_alpha_set ||
      !dos_alpha || !dos_npoints_set || !dos_npoints || !dos_emin_set ||
      !dos_emin || !dos_emax_set || !dos_emax || !dos_filename)
    return -1;

  *has_options = 0;
  *dos_alpha_set = 0;
  *dos_alpha = 0.0;
  *dos_npoints_set = 0;
  *dos_npoints = 0;
  *dos_emin_set = 0;
  *dos_emin = 0.0;
  *dos_emax_set = 0;
  *dos_emax = 0.0;
  *dos_filename = (capn_text){0};

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
    const int has_dos_scalars = nwpw.dosSet || nwpw.dosAlphaSet ||
                                nwpw.dosNpointsSet || nwpw.dosEminSet ||
                                nwpw.dosEmaxSet;
    const int has_dos = has_dos_scalars || nwpw.dosFilename.len > 0;
    if (!has_dos)
      continue;

    *has_options = 1;
    if (has_dos_scalars) {
      *dos_alpha_set = 1;
      *dos_alpha =
          nwpw.dosAlphaSet ? nwpw.dosAlpha : nwpw_dos_default_alpha();
      *dos_npoints_set = nwpw.dosNpointsSet;
      *dos_npoints = nwpw.dosNpoints;
      *dos_emin_set = nwpw.dosEminSet;
      *dos_emin = nwpw.dosEmin;
      *dos_emax_set = nwpw.dosEmaxSet;
      *dos_emax = nwpw.dosEmax;
    }
    if (nwpw.dosFilename.len > 0)
      *dos_filename = nwpw.dosFilename;
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_cpmd_grid(
    NWChemParams_ptr params, int *has_options, int *cpmd_properties,
    int *use_grid_comparison) {
  if (params.p.type == CAPN_NULL || !has_options || !cpmd_properties ||
      !use_grid_comparison)
    return -1;

  *has_options = 0;
  *cpmd_properties = NWChemNwpwToggle_unspecified;
  *use_grid_comparison = NWChemNwpwToggle_unspecified;

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
    if (nwpw.cpmdProperties != NWChemNwpwToggle_unspecified) {
      *has_options = 1;
      *cpmd_properties = nwpw.cpmdProperties;
    }
    if (nwpw.useGridComparison != NWChemNwpwToggle_unspecified) {
      *has_options = 1;
      *use_grid_comparison = nwpw.useGridComparison;
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_director(
    NWChemParams_ptr params, int *has_options, int *director,
    capn_text *filename) {
  if (params.p.type == CAPN_NULL || !has_options || !director || !filename)
    return -1;

  *has_options = 0;
  *director = NWChemNwpwToggle_unspecified;
  *filename = (capn_text){0};

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
    const int has_director =
        nwpw.director != NWChemNwpwToggle_unspecified ||
        nwpw.directorFilename.len > 0;
    if (!has_director)
      continue;

    *has_options = 1;
    *director = nwpw.director == NWChemNwpwToggle_unspecified
                    ? NWChemNwpwToggle_enabled
                    : nwpw.director;
    *filename = nwpw.directorFilename;
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_cell_mapping(
    NWChemParams_ptr params, int *has_options, int cell_expand[3],
    int *mapping) {
  if (params.p.type == CAPN_NULL || !has_options || !cell_expand || !mapping)
    return -1;

  *has_options = 0;
  cell_expand[0] = 0;
  cell_expand[1] = 0;
  cell_expand[2] = 0;
  *mapping = 0;

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
    if (nwpw.cellExpandX > 0 || nwpw.cellExpandY > 0 ||
        nwpw.cellExpandZ > 0) {
      *has_options = 1;
      cell_expand[0] = nwpw.cellExpandX > 0 ? nwpw.cellExpandX : 1;
      cell_expand[1] = nwpw.cellExpandY > 0 ? nwpw.cellExpandY : 1;
      cell_expand[2] = nwpw.cellExpandZ > 0 ? nwpw.cellExpandZ : 1;
    }
    int mapping_alias = nwpw_mapping_alias_value(nwpw.mappingAlias);
    if (mapping_alias > 0) {
      *has_options = 1;
      *mapping = mapping_alias;
    }
    if (nwpw.mappingSet || nwpw.mapping > 0) {
      *has_options = 1;
      *mapping = nwpw.mapping > 0 ? nwpw.mapping : 1;
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_rotation_multipole(
    NWChemParams_ptr params, int *has_options, int *rotation,
    int *lmax_multipole) {
  if (params.p.type == CAPN_NULL || !has_options || !rotation ||
      !lmax_multipole)
    return -1;

  *has_options = 0;
  *rotation = NWChemNwpwToggle_unspecified;
  *lmax_multipole = -1;

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
    if (nwpw.rotation != NWChemNwpwToggle_unspecified) {
      *has_options = 1;
      *rotation = nwpw.rotation;
    }
    if (nwpw.lmaxMultipole >= 0) {
      *has_options = 1;
      *lmax_multipole = nwpw.lmaxMultipole;
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_fei(
    NWChemParams_ptr params, int *has_options, int *fei, capn_text *filename) {
  if (params.p.type == CAPN_NULL || !has_options || !fei || !filename)
    return -1;

  *has_options = 0;
  *fei = 0;
  *filename = (capn_text){0};

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
    if (!nwpw.fei && nwpw.feiFilename.len <= 0)
      continue;

    *has_options = 1;
    *fei = 1;
    *filename = nwpw.feiFilename;
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_et(
    NWChemParams_ptr params, int *has_options, capn_text *movecs_a,
    capn_text *movecs_b, capn_text *ion_a, capn_text *ion_b) {
  if (params.p.type == CAPN_NULL || !has_options || !movecs_a || !movecs_b ||
      !ion_a || !ion_b)
    return -1;

  *has_options = 0;
  *movecs_a = (capn_text){0};
  *movecs_b = (capn_text){0};
  *ion_a = (capn_text){0};
  *ion_b = (capn_text){0};

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
    if (nwpw.etMovecsA.len <= 0 && nwpw.etMovecsB.len <= 0 &&
        nwpw.etIonA.len <= 0 && nwpw.etIonB.len <= 0)
      continue;

    *has_options = 1;
    *movecs_a = nwpw.etMovecsA;
    *movecs_b = nwpw.etMovecsB;
    *ion_a = nwpw.etIonA;
    *ion_b = nwpw.etIonB;
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_initial_velocities(
    NWChemParams_ptr params, int *has_options, double *temperature, int *seed) {
  if (params.p.type == CAPN_NULL || !has_options || !temperature || !seed)
    return -1;

  *has_options = 0;
  *temperature = 0.0;
  *seed = 494;

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
    if (nwpw.initialVelocitiesTemperature <= 0.0)
      continue;

    *has_options = 1;
    *temperature = nwpw.initialVelocitiesTemperature;
    *seed =
        nwpw.initialVelocitiesSeed >= 0 ? nwpw.initialVelocitiesSeed : 494;
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_make_hmass2(
    NWChemParams_ptr params, int *has_options, int *make_hmass2) {
  if (params.p.type == CAPN_NULL || !has_options || !make_hmass2)
    return -1;

  *has_options = 0;
  *make_hmass2 = NWChemNwpwToggle_unspecified;

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
    if (nwpw.makeHmass2 != NWChemNwpwToggle_unspecified) {
      *has_options = 1;
      *make_hmass2 = nwpw.makeHmass2;
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_translate_vector(
    NWChemParams_ptr params, int *has_options, double vector[3],
    capn_text *geometry_name, int *reorder) {
  if (params.p.type == CAPN_NULL || !has_options || !vector ||
      !geometry_name || !reorder)
    return -1;

  *has_options = 0;
  vector[0] = 0.0;
  vector[1] = 0.0;
  vector[2] = 0.0;
  *geometry_name = (capn_text){0};
  *reorder = NWChemNwpwToggle_unspecified;

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
    if (!nwpw.translateVectorSet)
      continue;

    *has_options = 1;
    vector[0] = nwpw.translateVectorX;
    vector[1] =
        nwpw.translateVectorY != 0.0 ? nwpw.translateVectorY : vector[0];
    vector[2] =
        nwpw.translateVectorZ != 0.0 ? nwpw.translateVectorZ : vector[1];
    *geometry_name = nwpw.translateGeometryName;
    *reorder = nwpw.translateReorder;
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_socket(
    NWChemParams_ptr params, int *has_options, capn_text *socket_type,
    capn_text *socket_ip) {
  if (params.p.type == CAPN_NULL || !has_options || !socket_type ||
      !socket_ip)
    return -1;

  *has_options = 0;
  *socket_type = (capn_text){0};
  *socket_ip = (capn_text){0};

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
    if (nwpw.socketType.len <= 0)
      continue;

    *has_options = 1;
    *socket_type = nwpw.socketType;
    *socket_ip = nwpw.socketIp;
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_apc(
    NWChemParams_ptr params, int *has_options, double *gc, double *gamma,
    size_t gamma_capacity, size_t *gamma_count) {
  if (params.p.type == CAPN_NULL || !has_options || !gc || !gamma_count)
    return -1;

  *has_options = 0;
  *gc = 0.0;
  *gamma_count = 0;

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
    capn_list64 apc_gamma = nwpw.apcGamma;
    capn_resolve(&apc_gamma.p);
    int ngamma = 0;
    if (apc_gamma.p.type != CAPN_NULL) {
      if (apc_gamma.p.type != CAPN_LIST || apc_gamma.p.datasz != 8)
        return -1;
      ngamma = apc_gamma.p.len;
    }
    if (!nwpw.apcSet || ngamma <= 0)
      continue;

    *has_options = 1;
    *gc = nwpw.apcGc;
    *gamma_count = (size_t)ngamma;
    if (gamma) {
      if ((size_t)ngamma > gamma_capacity)
        return -1;
      for (int j = 0; j < ngamma; ++j)
        gamma[j] = capn_to_f64(capn_get64(apc_gamma, j));
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_translation(
    NWChemParams_ptr params, int *has_options, int *translation) {
  if (params.p.type == CAPN_NULL || !has_options || !translation)
    return -1;

  *has_options = 0;
  *translation = NWChemNwpwToggle_unspecified;

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
    if (nwpw.translation != NWChemNwpwToggle_unspecified) {
      *has_options = 1;
      *translation = nwpw.translation;
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_minimizer(
    NWChemParams_ptr params, int *has_options, int *minimizer) {
  if (params.p.type == CAPN_NULL || !has_options || !minimizer)
    return -1;

  *has_options = 0;
  *minimizer = NWChemNwpwMinimizer_unspecified;

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
    if (nwpw.minimizer != NWChemNwpwMinimizer_unspecified) {
      *has_options = 1;
      *minimizer = nwpw.minimizer;
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_scf_algorithms(
    NWChemParams_ptr params, int *has_options, int *ks_algorithm,
    int *scf_algorithm, int *precondition) {
  if (params.p.type == CAPN_NULL || !has_options || !ks_algorithm ||
      !scf_algorithm || !precondition)
    return -1;

  *has_options = 0;
  *ks_algorithm = NWChemNwpwKsAlgorithm_unspecified;
  *scf_algorithm = NWChemNwpwScfAlgorithm_unspecified;
  *precondition = NWChemNwpwToggle_unspecified;

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
    if (nwpw.ksAlgorithm != NWChemNwpwKsAlgorithm_unspecified) {
      *has_options = 1;
      *ks_algorithm = nwpw.ksAlgorithm;
    }
    if (nwpw.scfAlgorithm != NWChemNwpwScfAlgorithm_unspecified) {
      *has_options = 1;
      *scf_algorithm = nwpw.scfAlgorithm;
    }
    if (nwpw.precondition != NWChemNwpwToggle_unspecified) {
      *has_options = 1;
      *precondition = nwpw.precondition;
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_nwpw_scf_numeric(
    NWChemParams_ptr params, int *has_options,
    NWChemNwpwScfNumericControls *controls) {
  if (params.p.type == CAPN_NULL || !has_options || !controls)
    return -1;

  *has_options = 0;
  memset(controls, 0, sizeof(*controls));

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
    if (nwpw.kerkerG0Set) {
      *has_options = 1;
      controls->kerker_g0_set = 1;
      controls->kerker_g0 = nwpw.kerkerG0;
    }
    if (nwpw.ksAlphaSet) {
      *has_options = 1;
      controls->ks_alpha_set = 1;
      controls->ks_alpha = nwpw.ksAlpha;
    }
    if (nwpw.ksMaxitOrbSet) {
      *has_options = 1;
      controls->ks_maxit_orb_set = 1;
      controls->ks_maxit_orb = nwpw.ksMaxitOrb;
    }
    if (nwpw.ksMaxitOrbsSet) {
      *has_options = 1;
      controls->ks_maxit_orbs_set = 1;
      controls->ks_maxit_orbs = nwpw.ksMaxitOrbs;
    }
    if (nwpw.diisHistoriesSet) {
      *has_options = 1;
      controls->diis_histories_set = 1;
      controls->diis_histories = nwpw.diisHistories;
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_brillouin_zone(
    NWChemParams_ptr params, int *has_options, capn_text *zone_name,
    int monkhorst_pack[3], int *max_kpoints_print, double *kvectors,
    size_t kvector_capacity, size_t *kvector_count) {
  if (params.p.type == CAPN_NULL || !has_options || !zone_name ||
      !monkhorst_pack || !max_kpoints_print || !kvector_count)
    return -1;

  *has_options = 0;
  *zone_name = (capn_text){0};
  monkhorst_pack[0] = 0;
  monkhorst_pack[1] = 0;
  monkhorst_pack[2] = 0;
  *max_kpoints_print = 0;
  *kvector_count = 0;

  struct NWChemParams view;
  read_NWChemParams(&view, params);
  int n = struct_list_len(&view.inputStanzas.p);
  if (n < 0)
    return -1;

  for (int i = 0; i < n; ++i) {
    struct NWChemInputStanza stanza;
    get_NWChemInputStanza(&stanza, view.inputStanzas, i);
    if (stanza.kind != NWChemInputStanza_Kind_brillouinZone ||
        stanza.brillouinZone.p.type == CAPN_NULL)
      continue;

    struct NWChemBrillouinZoneStanza zone;
    read_NWChemBrillouinZoneStanza(&zone, stanza.brillouinZone);
    int nk = struct_list_len(&zone.kVectors.p);
    if (nk < 0)
      return -1;
    if (zone.zoneName.len > 0) {
      *has_options = 1;
      *zone_name = zone.zoneName;
    }
    if (zone.monkhorstPackX != 0 || zone.monkhorstPackY != 0 ||
        zone.monkhorstPackZ != 0) {
      *has_options = 1;
      monkhorst_pack[0] = zone.monkhorstPackX != 0 ? zone.monkhorstPackX : 1;
      monkhorst_pack[1] = zone.monkhorstPackY != 0 ? zone.monkhorstPackY : 1;
      monkhorst_pack[2] = zone.monkhorstPackZ != 0 ? zone.monkhorstPackZ : 1;
    }
    if (zone.maxKpointsPrint > 0) {
      *has_options = 1;
      *max_kpoints_print = zone.maxKpointsPrint;
    }
    if (nk > 0) {
      *has_options = 1;
      *kvector_count = (size_t)nk;
      if (kvectors) {
        if ((size_t)nk > kvector_capacity)
          return -1;
        for (int j = 0; j < nk; ++j) {
          struct NWChemKVector kvector;
          get_NWChemKVector(&kvector, zone.kVectors, j);
          kvectors[4 * (size_t)j] = kvector.x;
          kvectors[4 * (size_t)j + 1] = kvector.y;
          kvectors[4 * (size_t)j + 2] = kvector.z;
          kvectors[4 * (size_t)j + 3] =
              kvector.weight != 0.0 ? kvector.weight : -1.0;
        }
      }
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_brillouin_tetrahedron(
    NWChemParams_ptr params, int *has_options, int tetrahedron_grid[3]) {
  if (params.p.type == CAPN_NULL || !has_options || !tetrahedron_grid)
    return -1;

  *has_options = 0;
  tetrahedron_grid[0] = 0;
  tetrahedron_grid[1] = 0;
  tetrahedron_grid[2] = 0;

  struct NWChemParams view;
  read_NWChemParams(&view, params);
  int n = struct_list_len(&view.inputStanzas.p);
  if (n < 0)
    return -1;

  for (int i = 0; i < n; ++i) {
    struct NWChemInputStanza stanza;
    get_NWChemInputStanza(&stanza, view.inputStanzas, i);
    if (stanza.kind != NWChemInputStanza_Kind_brillouinZone ||
        stanza.brillouinZone.p.type == CAPN_NULL)
      continue;

    struct NWChemBrillouinZoneStanza zone;
    read_NWChemBrillouinZoneStanza(&zone, stanza.brillouinZone);
    if (zone.tetrahedronGridX > 0 || zone.tetrahedronGridY > 0 ||
        zone.tetrahedronGridZ > 0) {
      *has_options = 1;
      tetrahedron_grid[0] =
          zone.tetrahedronGridX > 0 ? zone.tetrahedronGridX : 2;
      tetrahedron_grid[1] =
          zone.tetrahedronGridY > 0 ? zone.tetrahedronGridY : 2;
      tetrahedron_grid[2] =
          zone.tetrahedronGridZ > 0 ? zone.tetrahedronGridZ : 2;
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_brillouin_dos_grid(
    NWChemParams_ptr params, int *has_options, int dos_grid[3]) {
  if (params.p.type == CAPN_NULL || !has_options || !dos_grid)
    return -1;

  *has_options = 0;
  dos_grid[0] = 0;
  dos_grid[1] = 0;
  dos_grid[2] = 0;

  struct NWChemParams view;
  read_NWChemParams(&view, params);
  int n = struct_list_len(&view.inputStanzas.p);
  if (n < 0)
    return -1;

  for (int i = 0; i < n; ++i) {
    struct NWChemInputStanza stanza;
    get_NWChemInputStanza(&stanza, view.inputStanzas, i);
    if (stanza.kind != NWChemInputStanza_Kind_brillouinZone ||
        stanza.brillouinZone.p.type == CAPN_NULL)
      continue;

    struct NWChemBrillouinZoneStanza zone;
    read_NWChemBrillouinZoneStanza(&zone, stanza.brillouinZone);
    if (zone.dosGridX > 0 || zone.dosGridY > 0 || zone.dosGridZ > 0) {
      *has_options = 1;
      dos_grid[0] = zone.dosGridX > 0 ? zone.dosGridX : 2;
      dos_grid[1] = zone.dosGridY > 0 ? zone.dosGridY : 2;
      dos_grid[2] = zone.dosGridZ > 0 ? zone.dosGridZ : 2;
    }
  }

  return 0;
}

static capn_text params_literal_text(const char *value) {
  capn_text text;
  text.seg = 0;
  text.str = value;
  text.len = value ? strlen(value) : 0;
  return text;
}

static int append_brillouin_dos_zone(capn_text *zone_names, int *zone_grids,
                                     size_t capacity, size_t *count,
                                     capn_text zone_name,
                                     const char *default_zone_name, int x,
                                     int y, int z) {
  if (*count >= capacity)
    return -1;
  zone_names[*count] =
      zone_name.len > 0 ? zone_name : params_literal_text(default_zone_name);
  zone_grids[3 * *count] = x > 0 ? x : 2;
  zone_grids[3 * *count + 1] = y > 0 ? y : 2;
  zone_grids[3 * *count + 2] = z > 0 ? z : 2;
  ++*count;
  return 0;
}

int nwchemc_params_extract_direct_brillouin_dos_zones(
    NWChemParams_ptr params, capn_text *zone_names, int *zone_grids,
    size_t capacity, size_t *count) {
  if (params.p.type == CAPN_NULL || !count)
    return -1;

  *count = 0;
  if (capacity > 0 && (!zone_names || !zone_grids))
    return -1;

  struct NWChemParams view;
  read_NWChemParams(&view, params);
  int n = struct_list_len(&view.inputStanzas.p);
  if (n < 0)
    return -1;

  for (int i = 0; i < n; ++i) {
    struct NWChemInputStanza stanza;
    get_NWChemInputStanza(&stanza, view.inputStanzas, i);
    if (stanza.kind != NWChemInputStanza_Kind_brillouinZone ||
        stanza.brillouinZone.p.type == CAPN_NULL)
      continue;

    struct NWChemBrillouinZoneStanza zone;
    read_NWChemBrillouinZoneStanza(&zone, stanza.brillouinZone);
    if ((zone.dosGridX > 0 || zone.dosGridY > 0 || zone.dosGridZ > 0) &&
        append_brillouin_dos_zone(zone_names, zone_grids, capacity, count,
                                  zone.dosGridZoneName, "structure_default",
                                  zone.dosGridX, zone.dosGridY,
                                  zone.dosGridZ) != 0)
      return -1;
    if ((zone.tetrahedronGridX > 0 || zone.tetrahedronGridY > 0 ||
         zone.tetrahedronGridZ > 0) &&
        append_brillouin_dos_zone(zone_names, zone_grids, capacity, count,
                                  zone.tetrahedronZoneName, "zone_default",
                                  zone.tetrahedronGridX,
                                  zone.tetrahedronGridY,
                                  zone.tetrahedronGridZ) != 0)
      return -1;
    if ((zone.dosFftGridX > 0 || zone.dosFftGridY > 0 ||
         zone.dosFftGridZ > 0) &&
        append_brillouin_dos_zone(zone_names, zone_grids, capacity, count,
                                  zone.dosFftGridZoneName,
                                  "zone_fft_default", zone.dosFftGridX,
                                  zone.dosFftGridY, zone.dosFftGridZ) != 0)
      return -1;
  }

  return 0;
}

int nwchemc_params_for_each_direct_pseudopotential(
    NWChemParams_ptr params, nwchemc_params_direct_pseudopotential_fn callback,
    void *user_data, size_t *count) {
  if (params.p.type == CAPN_NULL || !count)
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
      if (callback && callback(user_data, target, &entry) != 0)
        return -1;
      ++*count;
    }
  }
  return 0;
}

int nwchemc_params_for_each_direct_pseudopotential_spin_rule(
    NWChemParams_ptr params,
    nwchemc_params_pseudopotential_spin_rule_fn callback, void *user_data,
    size_t *count) {
  if (params.p.type == CAPN_NULL || !count)
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
    if (pseudopotential.pspSpin ==
        NWChemPseudopotentialSpinMode_disabled)
      continue;
    int nrules = struct_list_len(&pseudopotential.spinRules.p);
    if (nrules < 0)
      return -1;
    for (int j = 0; j < nrules; ++j) {
      struct NWChemPseudopotentialSpinRule rule;
      get_NWChemPseudopotentialSpinRule(&rule, pseudopotential.spinRules, j);
      int nions = list32_len(rule.ionIndices);
      if (nions < 0)
        return -1;
      if (nions == 0)
        continue;
      size_t rule_index = *count + 1;
      if (callback && callback(user_data, rule_index, &rule) != 0)
        return -1;
      ++*count;
    }
  }
  return 0;
}

int nwchemc_params_for_each_direct_pseudopotential_uterm_rule(
    NWChemParams_ptr params,
    nwchemc_params_pseudopotential_uterm_rule_fn callback, void *user_data,
    size_t *count) {
  if (params.p.type == CAPN_NULL || !count)
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
    if (pseudopotential.uterm == NWChemToggle_disabled)
      continue;
    int nrules = struct_list_len(&pseudopotential.utermRules.p);
    if (nrules < 0)
      return -1;
    for (int j = 0; j < nrules; ++j) {
      struct NWChemPseudopotentialUtermRule rule;
      get_NWChemPseudopotentialUtermRule(&rule, pseudopotential.utermRules, j);
      int nions = list32_len(rule.ionIndices);
      if (nions < 0)
        return -1;
      if (nions == 0)
        continue;
      size_t rule_index = *count + 1;
      if (callback && callback(user_data, rule_index, &rule) != 0)
        return -1;
      ++*count;
    }
  }
  return 0;
}

struct direct_pseudopotential_extract_state {
  capn_text *elements;
  int *library_types;
  capn_text *library_names;
  size_t capacity;
  size_t count;
};

static int extract_direct_pseudopotential_entry(
    void *user_data, capn_text target,
    const struct NWChemPseudopotentialEntry *entry) {
  struct direct_pseudopotential_extract_state *state = user_data;
  if (!state || !entry || state->count >= state->capacity)
    return -1;
  state->elements[state->count] = target;
  state->library_types[state->count] = entry->libraryType;
  state->library_names[state->count] = entry->libraryName;
  ++state->count;
  return 0;
}

int nwchemc_params_extract_direct_pseudopotentials(
    NWChemParams_ptr params, capn_text *elements, int *library_types,
    capn_text *library_names, size_t capacity, size_t *count) {
  if (params.p.type == CAPN_NULL || !elements || !library_types ||
      !library_names || !count)
    return -1;

  struct direct_pseudopotential_extract_state state = {
      .elements = elements,
      .library_types = library_types,
      .library_names = library_names,
      .capacity = capacity,
      .count = 0,
  };
  size_t walked = 0;
  int rc = nwchemc_params_for_each_direct_pseudopotential(
      params, extract_direct_pseudopotential_entry, &state, &walked);
  *count = state.count;
  if (rc != 0 || walked != state.count)
    return -1;
  return 0;
}

int nwchemc_params_extract_direct_pseudopotential_spin(
    NWChemParams_ptr params, int *has_options, int *pspspin_enabled,
    int *pspspin_count, int *semicore_small) {
  if (params.p.type == CAPN_NULL || !has_options || !pspspin_enabled ||
      !pspspin_count || !semicore_small)
    return -1;

  *has_options = 0;
  *pspspin_enabled = 0;
  *pspspin_count = 0;
  *semicore_small = NWChemToggle_unspecified;

  struct NWChemParams view;
  read_NWChemParams(&view, params);
  int n = struct_list_len(&view.inputStanzas.p);
  if (n < 0)
    return -1;

  for (int i = 0; i < n; ++i) {
    struct NWChemInputStanza stanza;
    get_NWChemInputStanza(&stanza, view.inputStanzas, i);
    if (stanza.kind != NWChemInputStanza_Kind_pseudopotential ||
        stanza.pseudopotential.p.type == CAPN_NULL)
      continue;

    struct NWChemPseudopotentialStanza pseudopotential;
    read_NWChemPseudopotentialStanza(&pseudopotential,
                                     stanza.pseudopotential);
    if (pseudopotential.semicoreSmall != NWChemToggle_unspecified) {
      *has_options = 1;
      *semicore_small = pseudopotential.semicoreSmall;
    }
    if (pseudopotential.pspSpin ==
        NWChemPseudopotentialSpinMode_disabled) {
      *has_options = 1;
      *pspspin_enabled = 0;
      *pspspin_count = 0;
      continue;
    }
    if (pseudopotential.pspSpin == NWChemPseudopotentialSpinMode_enabled) {
      *has_options = 1;
      *pspspin_enabled = 1;
    }
    int nrules = struct_list_len(&pseudopotential.spinRules.p);
    if (nrules < 0)
      return -1;
    for (int j = 0; j < nrules; ++j) {
      struct NWChemPseudopotentialSpinRule rule;
      get_NWChemPseudopotentialSpinRule(&rule, pseudopotential.spinRules, j);
      int nions = list32_len(rule.ionIndices);
      if (nions < 0)
        return -1;
      if (nions == 0)
        continue;
      *has_options = 1;
      *pspspin_enabled = 1;
      ++*pspspin_count;
    }
  }

  return 0;
}

int nwchemc_params_extract_direct_pseudopotential_uterm(
    NWChemParams_ptr params, int *has_options, int *uterm_enabled,
    int *uterm_count) {
  if (params.p.type == CAPN_NULL || !has_options || !uterm_enabled ||
      !uterm_count)
    return -1;

  *has_options = 0;
  *uterm_enabled = 0;
  *uterm_count = 0;

  struct NWChemParams view;
  read_NWChemParams(&view, params);
  int n = struct_list_len(&view.inputStanzas.p);
  if (n < 0)
    return -1;

  for (int i = 0; i < n; ++i) {
    struct NWChemInputStanza stanza;
    get_NWChemInputStanza(&stanza, view.inputStanzas, i);
    if (stanza.kind != NWChemInputStanza_Kind_pseudopotential ||
        stanza.pseudopotential.p.type == CAPN_NULL)
      continue;

    struct NWChemPseudopotentialStanza pseudopotential;
    read_NWChemPseudopotentialStanza(&pseudopotential,
                                     stanza.pseudopotential);
    if (pseudopotential.uterm == NWChemToggle_disabled) {
      *has_options = 1;
      *uterm_enabled = 0;
      *uterm_count = 0;
      continue;
    }
    if (pseudopotential.uterm == NWChemToggle_enabled) {
      *has_options = 1;
      *uterm_enabled = 1;
    }
    int nrules = struct_list_len(&pseudopotential.utermRules.p);
    if (nrules < 0)
      return -1;
    for (int j = 0; j < nrules; ++j) {
      struct NWChemPseudopotentialUtermRule rule;
      get_NWChemPseudopotentialUtermRule(&rule, pseudopotential.utermRules, j);
      int nions = list32_len(rule.ionIndices);
      if (nions < 0)
        return -1;
      if (nions == 0)
        continue;
      *has_options = 1;
      *uterm_enabled = 1;
      ++*uterm_count;
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

int nwchemc_force_input_hessian_result_factors(ForceInput_ptr force_input,
                                               double *energy_factor,
                                               double *hessian_factor) {
  if (force_input.p.type == CAPN_NULL || !energy_factor || !hessian_factor)
    return -1;

  struct ForceInput view;
  read_ForceInput(&view, force_input);
  double length_factor = 1.0;
  double energy = 1.0;
  if (force_input_length_factor(view.lengthUnit, &length_factor) != 0 ||
      force_input_energy_factor(view.energyUnit, &energy) != 0)
    return -1;

  double length_scale = length_factor / NWCHEMC_BOHR_TO_ANGSTROM;
  *energy_factor = energy;
  *hessian_factor = energy * length_scale * length_scale;
  return 0;
}

int nwchemc_force_input_stress_result_factors(ForceInput_ptr force_input,
                                              double *energy_factor,
                                              double *stress_factor) {
  if (force_input.p.type == CAPN_NULL || !energy_factor || !stress_factor)
    return -1;

  struct ForceInput view;
  read_ForceInput(&view, force_input);
  double length_factor = 1.0;
  double energy = 1.0;
  if (force_input_length_factor(view.lengthUnit, &length_factor) != 0 ||
      force_input_energy_factor(view.energyUnit, &energy) != 0)
    return -1;

  double length_scale = length_factor / NWCHEMC_BOHR_TO_ANGSTROM;
  *energy_factor = energy;
  *stress_factor = energy * length_scale * length_scale * length_scale;
  return 0;
}

int nwchemc_force_input_position_result_factors(ForceInput_ptr force_input,
                                                double *energy_factor,
                                                double *position_factor) {
  if (force_input.p.type == CAPN_NULL || !energy_factor || !position_factor)
    return -1;

  struct ForceInput view;
  read_ForceInput(&view, force_input);
  double length_factor = 1.0;
  double energy = 1.0;
  if (force_input_length_factor(view.lengthUnit, &length_factor) != 0 ||
      force_input_energy_factor(view.energyUnit, &energy) != 0 ||
      length_factor == 0.0)
    return -1;

  *energy_factor = energy;
  *position_factor = 1.0 / length_factor;
  return 0;
}

size_t nwchemc_potential_result_flat_size(size_t force_count) {
  if (force_count > (SIZE_MAX - NWCHEMC_POTENTIAL_RESULT_BASE_SIZE) / 8u)
    return 0;
  return NWCHEMC_POTENTIAL_RESULT_BASE_SIZE + force_count * 8u;
}

size_t nwchemc_gradient_result_flat_size(size_t gradient_count) {
  if (gradient_count > (SIZE_MAX - NWCHEMC_POTENTIAL_RESULT_BASE_SIZE) / 8u)
    return 0;
  return NWCHEMC_POTENTIAL_RESULT_BASE_SIZE + gradient_count * 8u;
}

size_t nwchemc_hessian_result_flat_size(size_t hessian_count) {
  if (hessian_count > (SIZE_MAX - NWCHEMC_POTENTIAL_RESULT_BASE_SIZE) / 8u)
    return 0;
  return NWCHEMC_POTENTIAL_RESULT_BASE_SIZE + hessian_count * 8u;
}

size_t nwchemc_dipole_result_flat_size(void) {
  return NWCHEMC_POTENTIAL_RESULT_BASE_SIZE + 3u * 8u;
}

size_t nwchemc_quadrupole_result_flat_size(void) {
  return NWCHEMC_POTENTIAL_RESULT_BASE_SIZE + 6u * 8u;
}

size_t nwchemc_stress_result_flat_size(void) {
  return NWCHEMC_POTENTIAL_RESULT_BASE_SIZE + 9u * 8u;
}

size_t nwchemc_polarizability_result_flat_size(void) {
  return NWCHEMC_POTENTIAL_RESULT_BASE_SIZE + 12u * 8u;
}

size_t nwchemc_optimize_result_flat_size(size_t position_count) {
  if (position_count >
      (SIZE_MAX - NWCHEMC_POTENTIAL_RESULT_BASE_SIZE) / 8u)
    return 0;
  return NWCHEMC_POTENTIAL_RESULT_BASE_SIZE + position_count * 8u;
}

size_t nwchemc_frequencies_result_flat_size(size_t frequency_count) {
  size_t required = NWCHEMC_POTENTIAL_RESULT_BASE_SIZE;
  if (frequency_count > (size_t)INT_MAX)
    return 0;
  if (frequency_count > (SIZE_MAX - required) / 32u)
    return 0;
  required += frequency_count * 32u;
  if (frequency_count > 0 && frequency_count > SIZE_MAX / frequency_count)
    return 0;
  size_t normal_mode_count = frequency_count * frequency_count;
  if (normal_mode_count > (size_t)INT_MAX)
    return 0;
  if (normal_mode_count > (SIZE_MAX - required) / 8u)
    return 0;
  return required + normal_mode_count * 8u;
}

static int nwchemc_potential_result_write_lists(
    double energy, const double *forces, size_t force_count,
    const double *gradient, size_t gradient_count, const double *hessian,
    size_t hessian_count, const double *dipole, size_t dipole_count,
    const double *quadrupole, size_t quadrupole_count,
    const double *stress, size_t stress_count,
    const double *polarizability, size_t polarizability_count,
    const double *optimized_positions, size_t position_count,
    const double *frequencies, size_t frequency_count,
    const double *intensities, size_t intensity_count,
    const double *normal_modes, size_t normal_mode_count,
    const double *projected_frequencies, size_t projected_frequency_count,
    const double *projected_intensities, size_t projected_intensity_count,
    double zero_point_energy, double thermal_energy,
    double thermal_enthalpy, double entropy, double heat_capacity_cv,
    void *potential_result_capnp, size_t potential_result_capacity_bytes,
    size_t *potential_result_size_bytes) {
  if (!potential_result_capnp || !potential_result_size_bytes ||
      force_count > (size_t)INT_MAX || gradient_count > (size_t)INT_MAX ||
      hessian_count > (size_t)INT_MAX ||
      dipole_count > (size_t)INT_MAX || quadrupole_count > (size_t)INT_MAX ||
      stress_count > (size_t)INT_MAX ||
      polarizability_count > (size_t)INT_MAX ||
      position_count > (size_t)INT_MAX ||
      frequency_count > (size_t)INT_MAX || intensity_count > (size_t)INT_MAX ||
      normal_mode_count > (size_t)INT_MAX ||
      projected_frequency_count > (size_t)INT_MAX ||
      projected_intensity_count > (size_t)INT_MAX ||
      (force_count > 0 && !forces) ||
      (gradient_count > 0 && !gradient) ||
      (hessian_count > 0 && !hessian) ||
      (dipole_count > 0 && !dipole) ||
      (quadrupole_count > 0 && !quadrupole) ||
      (stress_count > 0 && !stress) ||
      (polarizability_count > 0 && !polarizability) ||
      (position_count > 0 && !optimized_positions) ||
      (frequency_count > 0 && !frequencies) ||
      (intensity_count > 0 && !intensities) ||
      (normal_mode_count > 0 && !normal_modes) ||
      (projected_frequency_count > 0 && !projected_frequencies) ||
      (projected_intensity_count > 0 && !projected_intensities))
    return -1;

  size_t required = NWCHEMC_POTENTIAL_RESULT_BASE_SIZE;
  if (force_count > (SIZE_MAX - required) / 8u)
    return -1;
  required += force_count * 8u;
  if (gradient_count > (SIZE_MAX - required) / 8u)
    return -1;
  required += gradient_count * 8u;
  if (hessian_count > (SIZE_MAX - required) / 8u)
    return -1;
  required += hessian_count * 8u;
  if (dipole_count > (SIZE_MAX - required) / 8u)
    return -1;
  required += dipole_count * 8u;
  if (quadrupole_count > (SIZE_MAX - required) / 8u)
    return -1;
  required += quadrupole_count * 8u;
  if (stress_count > (SIZE_MAX - required) / 8u)
    return -1;
  required += stress_count * 8u;
  if (polarizability_count > (SIZE_MAX - required) / 8u)
    return -1;
  required += polarizability_count * 8u;
  if (position_count > (SIZE_MAX - required) / 8u)
    return -1;
  required += position_count * 8u;
  if (frequency_count > (SIZE_MAX - required) / 8u)
    return -1;
  required += frequency_count * 8u;
  if (intensity_count > (SIZE_MAX - required) / 8u)
    return -1;
  required += intensity_count * 8u;
  if (normal_mode_count > (SIZE_MAX - required) / 8u)
    return -1;
  required += normal_mode_count * 8u;
  if (projected_frequency_count > (SIZE_MAX - required) / 8u)
    return -1;
  required += projected_frequency_count * 8u;
  if (projected_intensity_count > (SIZE_MAX - required) / 8u)
    return -1;
  required += projected_intensity_count * 8u;
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
  capn_list64 force_list = {0};
  capn_list64 gradient_list = {0};
  capn_list64 hessian_list = {0};
  capn_list64 dipole_list = {0};
  capn_list64 quadrupole_list = {0};
  capn_list64 stress_list = {0};
  capn_list64 polarizability_list = {0};
  capn_list64 optimized_list = {0};
  capn_list64 frequency_list = {0};
  capn_list64 intensity_list = {0};
  capn_list64 normal_mode_list = {0};
  capn_list64 projected_frequency_list = {0};
  capn_list64 projected_intensity_list = {0};
  if (force_count > 0)
    force_list = capn_new_list64(root.seg, (int)force_count);
  if (gradient_count > 0)
    gradient_list = capn_new_list64(root.seg, (int)gradient_count);
  if (hessian_count > 0)
    hessian_list = capn_new_list64(root.seg, (int)hessian_count);
  if (dipole_count > 0)
    dipole_list = capn_new_list64(root.seg, (int)dipole_count);
  if (quadrupole_count > 0)
    quadrupole_list = capn_new_list64(root.seg, (int)quadrupole_count);
  if (stress_count > 0)
    stress_list = capn_new_list64(root.seg, (int)stress_count);
  if (polarizability_count > 0)
    polarizability_list = capn_new_list64(root.seg,
                                          (int)polarizability_count);
  if (position_count > 0)
    optimized_list = capn_new_list64(root.seg, (int)position_count);
  if (frequency_count > 0)
    frequency_list = capn_new_list64(root.seg, (int)frequency_count);
  if (intensity_count > 0)
    intensity_list = capn_new_list64(root.seg, (int)intensity_count);
  if (normal_mode_count > 0)
    normal_mode_list = capn_new_list64(root.seg, (int)normal_mode_count);
  if (projected_frequency_count > 0)
    projected_frequency_list = capn_new_list64(
        root.seg, (int)projected_frequency_count);
  if (projected_intensity_count > 0)
    projected_intensity_list = capn_new_list64(
        root.seg, (int)projected_intensity_count);
  if (result.p.type == CAPN_NULL ||
      (force_count > 0 && force_list.p.type == CAPN_NULL) ||
      (gradient_count > 0 && gradient_list.p.type == CAPN_NULL) ||
      (hessian_count > 0 && hessian_list.p.type == CAPN_NULL) ||
      (dipole_count > 0 && dipole_list.p.type == CAPN_NULL) ||
      (quadrupole_count > 0 && quadrupole_list.p.type == CAPN_NULL) ||
      (stress_count > 0 && stress_list.p.type == CAPN_NULL) ||
      (polarizability_count > 0 &&
       polarizability_list.p.type == CAPN_NULL) ||
      (position_count > 0 && optimized_list.p.type == CAPN_NULL) ||
      (frequency_count > 0 && frequency_list.p.type == CAPN_NULL) ||
      (intensity_count > 0 && intensity_list.p.type == CAPN_NULL) ||
      (normal_mode_count > 0 && normal_mode_list.p.type == CAPN_NULL) ||
      (projected_frequency_count > 0 &&
       projected_frequency_list.p.type == CAPN_NULL) ||
      (projected_intensity_count > 0 &&
       projected_intensity_list.p.type == CAPN_NULL)) {
    capn_free(&arena);
    return -1;
  }
  for (size_t i = 0; i < force_count; ++i)
    capn_set64(force_list, (int)i, capn_from_f64(forces[i]));
  for (size_t i = 0; i < gradient_count; ++i)
    capn_set64(gradient_list, (int)i, capn_from_f64(gradient[i]));
  for (size_t i = 0; i < hessian_count; ++i)
    capn_set64(hessian_list, (int)i, capn_from_f64(hessian[i]));
  for (size_t i = 0; i < dipole_count; ++i)
    capn_set64(dipole_list, (int)i, capn_from_f64(dipole[i]));
  for (size_t i = 0; i < quadrupole_count; ++i)
    capn_set64(quadrupole_list, (int)i, capn_from_f64(quadrupole[i]));
  for (size_t i = 0; i < stress_count; ++i)
    capn_set64(stress_list, (int)i, capn_from_f64(stress[i]));
  for (size_t i = 0; i < polarizability_count; ++i)
    capn_set64(polarizability_list, (int)i,
               capn_from_f64(polarizability[i]));
  for (size_t i = 0; i < position_count; ++i)
    capn_set64(optimized_list, (int)i, capn_from_f64(optimized_positions[i]));
  for (size_t i = 0; i < frequency_count; ++i)
    capn_set64(frequency_list, (int)i, capn_from_f64(frequencies[i]));
  for (size_t i = 0; i < intensity_count; ++i)
    capn_set64(intensity_list, (int)i, capn_from_f64(intensities[i]));
  for (size_t i = 0; i < normal_mode_count; ++i)
    capn_set64(normal_mode_list, (int)i, capn_from_f64(normal_modes[i]));
  for (size_t i = 0; i < projected_frequency_count; ++i)
    capn_set64(projected_frequency_list, (int)i,
               capn_from_f64(projected_frequencies[i]));
  for (size_t i = 0; i < projected_intensity_count; ++i)
    capn_set64(projected_intensity_list, (int)i,
               capn_from_f64(projected_intensities[i]));

  struct PotentialResult view;
  memset(&view, 0, sizeof(view));
  view.energy = energy;
  view.zeroPointEnergy = zero_point_energy;
  view.thermalEnergy = thermal_energy;
  view.thermalEnthalpy = thermal_enthalpy;
  view.entropy = entropy;
  view.heatCapacityCv = heat_capacity_cv;
  view.forces = force_list;
  view.gradient = gradient_list;
  view.hessian = hessian_list;
  view.dipole = dipole_list;
  view.quadrupole = quadrupole_list;
  view.stress = stress_list;
  view.polarizability = polarizability_list;
  view.optimizedPos = optimized_list;
  view.frequencies = frequency_list;
  view.intensities = intensity_list;
  view.normalModes = normal_mode_list;
  view.projectedFrequencies = projected_frequency_list;
  view.projectedIntensities = projected_intensity_list;
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

int nwchemc_potential_result_write(double energy, const double *forces,
                                   size_t force_count,
                                   void *potential_result_capnp,
                                   size_t potential_result_capacity_bytes,
                                   size_t *potential_result_size_bytes) {
  return nwchemc_potential_result_write_lists(
      energy, forces, force_count, NULL, 0, NULL, 0, NULL, 0, NULL, 0,
      NULL, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0,
      NULL, 0, 0.0, 0.0, 0.0, 0.0, 0.0, potential_result_capnp,
      potential_result_capacity_bytes, potential_result_size_bytes);
}

int nwchemc_potential_result_write_gradient(
    double energy, const double *gradient, size_t gradient_count,
    void *potential_result_capnp, size_t potential_result_capacity_bytes,
    size_t *potential_result_size_bytes) {
  return nwchemc_potential_result_write_lists(
      energy, NULL, 0, gradient, gradient_count, NULL, 0, NULL, 0, NULL, 0,
      NULL, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0,
      NULL, 0, 0.0, 0.0, 0.0, 0.0, 0.0, potential_result_capnp,
      potential_result_capacity_bytes, potential_result_size_bytes);
}

int nwchemc_potential_result_write_hessian(
    double energy, const double *hessian, size_t hessian_count,
    void *potential_result_capnp, size_t potential_result_capacity_bytes,
    size_t *potential_result_size_bytes) {
  return nwchemc_potential_result_write_lists(
      energy, NULL, 0, NULL, 0, hessian, hessian_count, NULL, 0, NULL, 0,
      NULL, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0,
      NULL, 0, 0.0, 0.0, 0.0, 0.0, 0.0, potential_result_capnp,
      potential_result_capacity_bytes, potential_result_size_bytes);
}

int nwchemc_potential_result_write_dipole(
    double energy, const double *dipole, void *potential_result_capnp,
    size_t potential_result_capacity_bytes,
    size_t *potential_result_size_bytes) {
  return nwchemc_potential_result_write_lists(
      energy, NULL, 0, NULL, 0, NULL, 0, dipole, 3, NULL, 0, NULL, 0,
      NULL, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0,
      0.0, 0.0, 0.0, 0.0, 0.0, potential_result_capnp,
      potential_result_capacity_bytes,
      potential_result_size_bytes);
}

int nwchemc_potential_result_write_polarizability(
    double energy, const double *polarizability,
    void *potential_result_capnp, size_t potential_result_capacity_bytes,
    size_t *potential_result_size_bytes) {
  return nwchemc_potential_result_write_lists(
      energy, NULL, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0,
      NULL, 0, polarizability, 12, NULL, 0, NULL, 0, NULL, 0,
      NULL, 0, NULL, 0, NULL, 0, 0.0, 0.0, 0.0, 0.0, 0.0,
      potential_result_capnp,
      potential_result_capacity_bytes, potential_result_size_bytes);
}

int nwchemc_potential_result_write_quadrupole(
    double energy, const double *quadrupole, void *potential_result_capnp,
    size_t potential_result_capacity_bytes,
    size_t *potential_result_size_bytes) {
  return nwchemc_potential_result_write_lists(
      energy, NULL, 0, NULL, 0, NULL, 0, NULL, 0, quadrupole, 6,
      NULL, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0,
      NULL, 0, NULL, 0, NULL, 0, 0.0, 0.0, 0.0, 0.0, 0.0,
      potential_result_capnp,
      potential_result_capacity_bytes, potential_result_size_bytes);
}

int nwchemc_potential_result_write_stress(
    double energy, const double *stress, void *potential_result_capnp,
    size_t potential_result_capacity_bytes,
    size_t *potential_result_size_bytes) {
  return nwchemc_potential_result_write_lists(
      energy, NULL, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0,
      stress, 9, NULL, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0,
      NULL, 0, NULL, 0, 0.0, 0.0, 0.0, 0.0, 0.0,
      potential_result_capnp,
      potential_result_capacity_bytes,
      potential_result_size_bytes);
}

int nwchemc_potential_result_write_optimized(
    double energy, const double *optimized_positions, size_t position_count,
    void *potential_result_capnp, size_t potential_result_capacity_bytes,
    size_t *potential_result_size_bytes) {
  return nwchemc_potential_result_write_lists(
      energy, NULL, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0,
      NULL, 0, NULL, 0, optimized_positions, position_count, NULL, 0,
      NULL, 0, NULL, 0, NULL, 0, NULL, 0,
      0.0, 0.0, 0.0, 0.0, 0.0, potential_result_capnp,
      potential_result_capacity_bytes,
      potential_result_size_bytes);
}

int nwchemc_potential_result_write_frequencies(
    double energy, const double *frequencies, const double *intensities,
    const double *normal_modes, const double *thermochemistry,
    const double *projected_frequencies,
    const double *projected_intensities, size_t frequency_count,
    void *potential_result_capnp, size_t potential_result_capacity_bytes,
    size_t *potential_result_size_bytes) {
  if (frequency_count > 0 && frequency_count > SIZE_MAX / frequency_count)
    return -1;
  size_t normal_mode_count = frequency_count * frequency_count;
  double zero_point_energy = thermochemistry ? thermochemistry[0] : 0.0;
  double thermal_energy = thermochemistry ? thermochemistry[1] : 0.0;
  double thermal_enthalpy = thermochemistry ? thermochemistry[2] : 0.0;
  double entropy = thermochemistry ? thermochemistry[3] : 0.0;
  double heat_capacity_cv = thermochemistry ? thermochemistry[4] : 0.0;
  return nwchemc_potential_result_write_lists(
      energy, NULL, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0,
      NULL, 0, NULL, 0, frequencies, frequency_count, intensities,
      frequency_count, normal_modes, normal_mode_count, projected_frequencies,
      frequency_count, projected_intensities, frequency_count,
      zero_point_energy, thermal_energy, thermal_enthalpy, entropy,
      heat_capacity_cv,
      potential_result_capnp, potential_result_capacity_bytes,
      potential_result_size_bytes);
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
