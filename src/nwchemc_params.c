#include "nwchemc_params.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

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
      append_text(block, sizeof(block), set.key) != 0 ||
      append_format(block, sizeof(block), " ") != 0 ||
      append_text(block, sizeof(block), set.value) != 0)
    return -1;
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

static int render_pseudopotential_entries(
    NWChemPseudopotentialEntry_list entries, char *dst, size_t dst_size) {
  int n = struct_list_len(&entries.p);
  if (n < 0)
    return -1;
  for (int i = 0; i < n; ++i) {
    struct NWChemPseudopotentialEntry entry;
    get_NWChemPseudopotentialEntry(&entry, entries, i);
    if (entry.element.len <= 0 || entry.libraryName.len <= 0)
      continue;
    if (append_format(dst, dst_size, "    ") != 0 ||
        append_text(dst, dst_size, entry.element) != 0 ||
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
                                         char *dst, size_t dst_size) {
  if (ptr.p.type == CAPN_NULL)
    return 0;

  struct NWChemPseudopotentialStanza pseudopotential;
  char block[4096];
  block[0] = '\0';
  read_NWChemPseudopotentialStanza(&pseudopotential, ptr);
  if (append_format(block, sizeof(block), "nwpw\n  pseudopotentials\n") != 0 ||
      render_pseudopotential_entries(pseudopotential.entries, block,
                                     sizeof(block)) != 0 ||
      append_format(block, sizeof(block), "  end\n") != 0 ||
      render_directives(pseudopotential.directives, block, sizeof(block),
                        "  ") != 0 ||
      append_format(block, sizeof(block), "end") != 0)
    return -1;
  return append_block(dst, dst_size, block);
}

static int render_scf_stanza(NWChemScfStanza_ptr ptr, char *dst,
                             size_t dst_size) {
  if (ptr.p.type == CAPN_NULL)
    return 0;

  struct NWChemScfStanza scf;
  char block[4096];
  block[0] = '\0';
  read_NWChemScfStanza(&scf, ptr);
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
  if (scf.maxiter > 0 &&
      append_format(block, sizeof(block), "  maxiter %d\n", scf.maxiter) != 0)
    return -1;
  if (scf.thresh > 0.0 &&
      append_format(block, sizeof(block), "  thresh %.12g\n", scf.thresh) != 0)
    return -1;
  if (scf.tol2e > 0.0 &&
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
                                size_t dst_size) {
  if (ptr.p.type == CAPN_NULL)
    return 0;

  struct NWChemDriverStanza driver;
  char block[4096];
  block[0] = '\0';
  read_NWChemDriverStanza(&driver, ptr);
  if (append_format(block, sizeof(block), "driver\n") != 0)
    return -1;
  if (driver.maxiter > 0 &&
      append_format(block, sizeof(block), "  maxiter %d\n", driver.maxiter) != 0)
    return -1;
  if (driver.tight && append_format(block, sizeof(block), "  tight\n") != 0)
    return -1;
  if (driver.loose && append_format(block, sizeof(block), "  loose\n") != 0)
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

static int render_input_stanzas(NWChemInputStanza_list stanzas, char *dst,
                                size_t dst_size,
                                int include_direct_promoted_dft) {
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
      if (render_set_stanza(stanza.set, dst, dst_size) != 0)
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
                                        dst_size) != 0)
        return -1;
      break;
    case NWChemInputStanza_Kind_scf:
      if (render_scf_stanza(stanza.scf, dst, dst_size) != 0)
        return -1;
      break;
    case NWChemInputStanza_Kind_task:
      if (render_task_stanza(stanza.taskStanza, dst, dst_size) != 0)
        return -1;
      break;
    case NWChemInputStanza_Kind_driver:
      if (render_driver_stanza(stanza.driver, dst, dst_size) != 0)
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
  if (render_input_stanzas(view.inputStanzas, dst, dst_size, 1) != 0)
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
  if (render_input_stanzas(view.inputStanzas, dst, dst_size, 0) != 0)
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
