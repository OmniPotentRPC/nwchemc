#include "nwchemc_features.h"

#include <string.h>

static const NWChemCFeatureEntry k_features[] = {
    {"module.custom", "NWChemModuleName.custom", "(customName Text)", NWCHEMC_FEATURE_MODULE, 0, 1, 1},
    {"module.basis", "NWChemModuleName.basis", "basis", NWCHEMC_FEATURE_MODULE, 1, 1, 1},
    {"module.bq", "NWChemModuleName.bq", "bq", NWCHEMC_FEATURE_MODULE, 2, 1, 1},
    {"module.ccsd", "NWChemModuleName.ccsd", "ccsd", NWCHEMC_FEATURE_MODULE, 3, 1, 1},
    {"module.cosmo", "NWChemModuleName.cosmo", "cosmo", NWCHEMC_FEATURE_MODULE, 4, 1, 1},
    {"module.dft", "NWChemModuleName.dft", "dft", NWCHEMC_FEATURE_MODULE, 5, 1, 1},
    {"module.dplot", "NWChemModuleName.dplot", "dplot", NWCHEMC_FEATURE_MODULE, 6, 1, 1},
    {"module.drdy", "NWChemModuleName.drdy", "drdy", NWCHEMC_FEATURE_MODULE, 7, 1, 1},
    {"module.driver", "NWChemModuleName.driver", "driver", NWCHEMC_FEATURE_MODULE, 8, 1, 1},
    {"module.esp", "NWChemModuleName.esp", "esp", NWCHEMC_FEATURE_MODULE, 9, 1, 1},
    {"module.etrans", "NWChemModuleName.etrans", "etrans", NWCHEMC_FEATURE_MODULE, 10, 1, 1},
    {"module.geometry", "NWChemModuleName.geometry", "geometry", NWCHEMC_FEATURE_MODULE, 11, 1, 1},
    {"module.gw", "NWChemModuleName.gw", "gw", NWCHEMC_FEATURE_MODULE, 12, 1, 1},
    {"module.hessian", "NWChemModuleName.hessian", "hessian", NWCHEMC_FEATURE_MODULE, 13, 1, 1},
    {"module.mcscf", "NWChemModuleName.mcscf", "mcscf", NWCHEMC_FEATURE_MODULE, 14, 1, 1},
    {"module.md", "NWChemModuleName.md", "md", NWCHEMC_FEATURE_MODULE, 15, 1, 1},
    {"module.mm", "NWChemModuleName.mm", "mm", NWCHEMC_FEATURE_MODULE, 16, 1, 1},
    {"module.mp2", "NWChemModuleName.mp2", "mp2", NWCHEMC_FEATURE_MODULE, 17, 1, 1},
    {"module.ncc", "NWChemModuleName.ncc", "ncc", NWCHEMC_FEATURE_MODULE, 18, 1, 1},
    {"module.nwpw", "NWChemModuleName.nwpw", "nwpw", NWCHEMC_FEATURE_MODULE, 19, 1, 1},
    {"module.property", "NWChemModuleName.property", "property", NWCHEMC_FEATURE_MODULE, 20, 1, 1},
    {"module.python", "NWChemModuleName.python", "python", NWCHEMC_FEATURE_MODULE, 21, 1, 1},
    {"module.qmd", "NWChemModuleName.qmd", "qmd", NWCHEMC_FEATURE_MODULE, 22, 1, 1},
    {"module.qmmm", "NWChemModuleName.qmmm", "qmmm", NWCHEMC_FEATURE_MODULE, 23, 1, 1},
    {"module.rimp2", "NWChemModuleName.rimp2", "rimp2", NWCHEMC_FEATURE_MODULE, 24, 1, 1},
    {"module.rism", "NWChemModuleName.rism", "rism", NWCHEMC_FEATURE_MODULE, 25, 1, 1},
    {"module.scf", "NWChemModuleName.scf", "scf", NWCHEMC_FEATURE_MODULE, 26, 1, 1},
    {"module.selci", "NWChemModuleName.selci", "selci", NWCHEMC_FEATURE_MODULE, 27, 1, 1},
    {"module.smd", "NWChemModuleName.smd", "smd", NWCHEMC_FEATURE_MODULE, 28, 1, 1},
    {"module.tce", "NWChemModuleName.tce", "tce", NWCHEMC_FEATURE_MODULE, 29, 1, 1},
    {"module.vib", "NWChemModuleName.vib", "vib", NWCHEMC_FEATURE_MODULE, 30, 1, 1},
    {"module.vscf", "NWChemModuleName.vscf", "vscf", NWCHEMC_FEATURE_MODULE, 31, 1, 1},
    {"module.xtb", "NWChemModuleName.xtb", "xtb", NWCHEMC_FEATURE_MODULE, 32, 1, 1},
    {"module.analysis", "NWChemModuleName.analysis", "analysis", NWCHEMC_FEATURE_MODULE, 33, 1, 1},
    {"module.argos", "NWChemModuleName.argos", "argos", NWCHEMC_FEATURE_MODULE, 34, 1, 1},
    {"module.argosDiana", "NWChemModuleName.argosDiana", "argos_diana", NWCHEMC_FEATURE_MODULE, 35, 1, 1},
    {"module.argosPrep", "NWChemModuleName.argosPrep", "argos_prep", NWCHEMC_FEATURE_MODULE, 36, 1, 1},
    {"module.argosPrepare", "NWChemModuleName.argosPrepare", "argos_prepare", NWCHEMC_FEATURE_MODULE, 37, 1, 1},
    {"module.band", "NWChemModuleName.band", "band", NWCHEMC_FEATURE_MODULE, 38, 1, 1},
    {"module.bandDplot", "NWChemModuleName.bandDplot", "band_dplot", NWCHEMC_FEATURE_MODULE, 39, 1, 1},
    {"module.brillouinZone", "NWChemModuleName.brillouinZone", "brillouin_zone", NWCHEMC_FEATURE_MODULE, 40, 1, 1},
    {"module.bsemol", "NWChemModuleName.bsemol", "bsemol", NWCHEMC_FEATURE_MODULE, 41, 1, 1},
    {"module.cckohn", "NWChemModuleName.cckohn", "cckohn", NWCHEMC_FEATURE_MODULE, 42, 1, 1},
    {"module.cellOptimize", "NWChemModuleName.cellOptimize", "cell_optimize", NWCHEMC_FEATURE_MODULE, 43, 1, 1},
    {"module.cgsd", "NWChemModuleName.cgsd", "cgsd", NWCHEMC_FEATURE_MODULE, 44, 1, 1},
    {"module.constraints", "NWChemModuleName.constraints", "constraints", NWCHEMC_FEATURE_MODULE, 45, 1, 1},
    {"module.cpmd", "NWChemModuleName.cpmd", "cpmd", NWCHEMC_FEATURE_MODULE, 46, 1, 1},
    {"module.cpsd", "NWChemModuleName.cpsd", "cpsd", NWCHEMC_FEATURE_MODULE, 47, 1, 1},
    {"module.ddscf", "NWChemModuleName.ddscf", "ddscf", NWCHEMC_FEATURE_MODULE, 48, 1, 1},
    {"module.diana", "NWChemModuleName.diana", "diana", NWCHEMC_FEATURE_MODULE, 49, 1, 1},
    {"module.dimpar", "NWChemModuleName.dimpar", "dimpar", NWCHEMC_FEATURE_MODULE, 50, 1, 1},
    {"module.dimqm", "NWChemModuleName.dimqm", "dimqm", NWCHEMC_FEATURE_MODULE, 51, 1, 1},
    {"module.dk", "NWChemModuleName.dk", "dk", NWCHEMC_FEATURE_MODULE, 52, 1, 1},
    {"module.dmd", "NWChemModuleName.dmd", "dmd", NWCHEMC_FEATURE_MODULE, 53, 1, 1},
    {"module.dntmc", "NWChemModuleName.dntmc", "dntmc", NWCHEMC_FEATURE_MODULE, 54, 1, 1},
    {"module.fractionalOccupations", "NWChemModuleName.fractionalOccupations", "frac_occ", NWCHEMC_FEATURE_MODULE, 55, 1, 1},
    {"module.freeze", "NWChemModuleName.freeze", "freeze", NWCHEMC_FEATURE_MODULE, 56, 1, 1},
    {"module.intgrl", "NWChemModuleName.intgrl", "intgrl", NWCHEMC_FEATURE_MODULE, 57, 1, 1},
    {"module.mdXs", "NWChemModuleName.mdXs", "md_xs", NWCHEMC_FEATURE_MODULE, 58, 1, 1},
    {"module.mepgs", "NWChemModuleName.mepgs", "mepgs", NWCHEMC_FEATURE_MODULE, 59, 1, 1},
    {"module.metadynamics", "NWChemModuleName.metadynamics", "metadynamics", NWCHEMC_FEATURE_MODULE, 60, 1, 1},
    {"module.modelpotential", "NWChemModuleName.modelpotential", "modelpotential", NWCHEMC_FEATURE_MODULE, 61, 1, 1},
    {"module.neb", "NWChemModuleName.neb", "neb", NWCHEMC_FEATURE_MODULE, 62, 1, 1},
    {"module.occup", "NWChemModuleName.occup", "occup", NWCHEMC_FEATURE_MODULE, 63, 1, 1},
    {"module.prepare", "NWChemModuleName.prepare", "prepare", NWCHEMC_FEATURE_MODULE, 64, 1, 1},
    {"module.pspFormatter", "NWChemModuleName.pspFormatter", "psp_formatter", NWCHEMC_FEATURE_MODULE, 65, 1, 1},
    {"module.pspGenerator", "NWChemModuleName.pspGenerator", "psp_generator", NWCHEMC_FEATURE_MODULE, 66, 1, 1},
    {"module.pspw", "NWChemModuleName.pspw", "pspw", NWCHEMC_FEATURE_MODULE, 67, 1, 1},
    {"module.pspwDplot", "NWChemModuleName.pspwDplot", "pspw_dplot", NWCHEMC_FEATURE_MODULE, 68, 1, 1},
    {"module.pspwQmmm", "NWChemModuleName.pspwQmmm", "pspw_qmmm", NWCHEMC_FEATURE_MODULE, 69, 1, 1},
    {"module.pspwWannier", "NWChemModuleName.pspwWannier", "pspw_wannier", NWCHEMC_FEATURE_MODULE, 70, 1, 1},
    {"module.qmdNamd", "NWChemModuleName.qmdNamd", "qmd_namd", NWCHEMC_FEATURE_MODULE, 71, 1, 1},
    {"module.raman", "NWChemModuleName.raman", "raman", NWCHEMC_FEATURE_MODULE, 72, 1, 1},
    {"module.rel", "NWChemModuleName.rel", "rel", NWCHEMC_FEATURE_MODULE, 73, 1, 1},
    {"module.rtTddft", "NWChemModuleName.rtTddft", "rt_tddft", NWCHEMC_FEATURE_MODULE, 74, 1, 1},
    {"module.simulationCell", "NWChemModuleName.simulationCell", "simulation_cell", NWCHEMC_FEATURE_MODULE, 75, 1, 1},
    {"module.string", "NWChemModuleName.string", "string", NWCHEMC_FEATURE_MODULE, 76, 1, 1},
    {"module.tamd", "NWChemModuleName.tamd", "tamd", NWCHEMC_FEATURE_MODULE, 77, 1, 1},
    {"module.task", "NWChemModuleName.task", "task", NWCHEMC_FEATURE_MODULE, 78, 1, 1},
    {"module.taskShell", "NWChemModuleName.taskShell", "task_shell", NWCHEMC_FEATURE_MODULE, 79, 1, 1},
    {"module.tceMrcc", "NWChemModuleName.tceMrcc", "tce_mrcc", NWCHEMC_FEATURE_MODULE, 80, 1, 1},
    {"module.tddft", "NWChemModuleName.tddft", "tddft", NWCHEMC_FEATURE_MODULE, 81, 1, 1},
    {"module.tddftGradient", "NWChemModuleName.tddftGradient", "tddft_grad", NWCHEMC_FEATURE_MODULE, 82, 1, 1},
    {"module.tropt", "NWChemModuleName.tropt", "tropt", NWCHEMC_FEATURE_MODULE, 83, 1, 1},
    {"module.vibZone", "NWChemModuleName.vibZone", "vib_zone", NWCHEMC_FEATURE_MODULE, 84, 1, 1},
    {"module.waterPseudopotential", "NWChemModuleName.waterPseudopotential", "water_pseudopotential", NWCHEMC_FEATURE_MODULE, 85, 1, 1},
    {"module.x2c", "NWChemModuleName.x2c", "x2c", NWCHEMC_FEATURE_MODULE, 86, 1, 1},
    {"module.zora", "NWChemModuleName.zora", "zora", NWCHEMC_FEATURE_MODULE, 87, 1, 1},
    {"stanza.generic", "NWChemInputStanza.generic", "Named block with directives then end", NWCHEMC_FEATURE_STANZA, -1, 1, 1},
    {"stanza.dft", "NWChemInputStanza.dft", "Typed dft block with xc/direct/smear/directives", NWCHEMC_FEATURE_STANZA, -1, 1, 1},
    {"stanza.set", "NWChemInputStanza.set", "set key value RTDB directive", NWCHEMC_FEATURE_STANZA, -1, 1, 1},
    {"stanza.raw", "NWChemInputStanza.raw", "Literal NWChem input fragment", NWCHEMC_FEATURE_STANZA, -1, 1, 1},
    {"stanza.module", "NWChemInputStanza.module", "Typed module block from NWChemModuleName", NWCHEMC_FEATURE_STANZA, -1, 1, 1},
    {"stanza.pseudopotential", "NWChemInputStanza.pseudopotential", "Pseudopotential library/file entries block", NWCHEMC_FEATURE_STANZA, -1, 1, 1},
    {"stanza.scf", "NWChemInputStanza.scf", "Typed scf block (vectors/maxiter/thresh/directives)", NWCHEMC_FEATURE_STANZA, -1, 1, 1},
    {"stanza.task", "NWChemInputStanza.taskStanza", "Explicit task theory operation [ignore] line", NWCHEMC_FEATURE_STANZA, -1, 1, 1},
    {"stanza.driver", "NWChemInputStanza.driver", "Geometry optimization driver block", NWCHEMC_FEATURE_STANZA, -1, 1, 1},
    {"stanza.property", "NWChemInputStanza.property", "Property block (dipole/mulliken/quadrupole)", NWCHEMC_FEATURE_STANZA, -1, 1, 1},
    {"stanza.basis", "NWChemInputStanza.basisStanza", "Structured basis/ECP block (complements top-level basis)", NWCHEMC_FEATURE_STANZA, -1, 1, 1},
    {"stanza.geometry", "NWChemInputStanza.geometry", "Geometry block metadata (units/symmetry; coords via ABI)", NWCHEMC_FEATURE_STANZA, -1, 1, 1},
    {"params.basis", "NWChemParams.basis", "Gaussian basis set name", NWCHEMC_FEATURE_PARAMS_FIELD, 0, 1, 1},
    {"params.theory", "NWChemParams.theory", "Main theory/task family (scf/dft/...)", NWCHEMC_FEATURE_PARAMS_FIELD, 1, 1, 1},
    {"params.scfType", "NWChemParams.scfType", "SCF type or DFT XC keyword", NWCHEMC_FEATURE_PARAMS_FIELD, 2, 1, 1},
    {"params.charge", "NWChemParams.charge", "Molecular charge", NWCHEMC_FEATURE_PARAMS_FIELD, 3, 1, 1},
    {"params.multiplicity", "NWChemParams.multiplicity", "Spin multiplicity 2S+1", NWCHEMC_FEATURE_PARAMS_FIELD, 4, 1, 1},
    {"params.enginePath", "NWChemParams.enginePath", "dlopen path for engine; empty probes env", NWCHEMC_FEATURE_PARAMS_FIELD, 5, 1, 0},
    {"params.nwchemRoot", "NWChemParams.nwchemRoot", "NWCHEM_TOP embed root hint", NWCHEMC_FEATURE_PARAMS_FIELD, 6, 1, 1},
    {"params.task", "NWChemParams.task", "Task label energy|gradient|hessian|property", NWCHEMC_FEATURE_PARAMS_FIELD, 7, 1, 1},
    {"params.title", "NWChemParams.title", "Optional title/start prefix", NWCHEMC_FEATURE_PARAMS_FIELD, 8, 1, 1},
    {"params.memoryMb", "NWChemParams.memoryMb", "Memory MiB; 0 keeps defaults", NWCHEMC_FEATURE_PARAMS_FIELD, 9, 1, 1},
    {"params.scratchDir", "NWChemParams.scratchDir", "NWCHEM_SCRATCH_DIR", NWCHEMC_FEATURE_PARAMS_FIELD, 10, 1, 1},
    {"params.permanentDir", "NWChemParams.permanentDir", "NWCHEM_PERMANENT_DIR", NWCHEMC_FEATURE_PARAMS_FIELD, 11, 1, 1},
    {"params.inputBlocks", "NWChemParams.inputBlocks", "Raw directive blocks before task", NWCHEMC_FEATURE_PARAMS_FIELD, 12, 1, 1},
    {"params.inputStanzas", "NWChemParams.inputStanzas", "Structured input stanzas", NWCHEMC_FEATURE_PARAMS_FIELD, 13, 1, 1},
    {"abi.nwchemc_set_params", "include/nwchemc.h::nwchemc_set_params", "stub=fails non-zero; embed=applies Cap'n Proto params", NWCHEMC_FEATURE_ABI, -1, 1, 1},
    {"abi.nwchemc_energy_gradient", "include/nwchemc.h::nwchemc_energy_gradient", "stub=fails ok==0; embed=runs energy/gradient", NWCHEMC_FEATURE_ABI, -1, 1, 1},
    {"abi.nwchemc_energy", "include/nwchemc.h::nwchemc_energy", "stub=fails ok==0; embed=runs energy-only (no grad output)", NWCHEMC_FEATURE_ABI, -1, 1, 1},
    {"abi.nwchemc_energy_forces", "include/nwchemc.h::nwchemc_energy_forces", "stub=fails ok==0; embed=runs energy/forces (negated gradient)", NWCHEMC_FEATURE_ABI, -1, 1, 1},
    {"abi.nwchemc_hessian", "include/nwchemc.h::nwchemc_hessian", "stub=fails ok==0; embed=runs Cartesian Hessian", NWCHEMC_FEATURE_ABI, -1, 1, 1},
    {"abi.nwchemc_session_create", "include/nwchemc.h::nwchemc_session_create", "stub=returns NULL; embed=creates persistent Cap'n Proto session", NWCHEMC_FEATURE_ABI, -1, 1, 1},
    {"abi.nwchemc_session_set_params", "include/nwchemc.h::nwchemc_session_set_params", "stub=fails non-zero; embed=replaces persistent Cap'n Proto params", NWCHEMC_FEATURE_ABI, -1, 1, 1},
    {"abi.nwchemc_session_destroy", "include/nwchemc.h::nwchemc_session_destroy", "stub=no-op; embed=releases persistent session", NWCHEMC_FEATURE_ABI, -1, 1, 1},
    {"abi.nwchemc_session_energy_gradient", "include/nwchemc.h::nwchemc_session_energy_gradient", "stub=fails ok==0; embed=runs session energy/gradient", NWCHEMC_FEATURE_ABI, -1, 1, 1},
    {"abi.nwchemc_session_energy", "include/nwchemc.h::nwchemc_session_energy", "stub=fails ok==0; embed=runs session energy-only", NWCHEMC_FEATURE_ABI, -1, 1, 1},
    {"abi.nwchemc_session_energy_forces", "include/nwchemc.h::nwchemc_session_energy_forces", "stub=fails ok==0; embed=runs session energy/forces", NWCHEMC_FEATURE_ABI, -1, 1, 1},
    {"abi.nwchemc_session_calculate_forces", "include/nwchemc.h::nwchemc_session_calculate_forces", "stub=fails ok==0; embed=runs session ForceInput energy/forces", NWCHEMC_FEATURE_ABI, -1, 1, 1},
    {"abi.nwchemc_session_calculate_hessian", "include/nwchemc.h::nwchemc_session_calculate_hessian", "stub=fails ok==0; embed=runs session ForceInput Hessian", NWCHEMC_FEATURE_ABI, -1, 1, 1},
    {"abi.nwchemc_session_hessian", "include/nwchemc.h::nwchemc_session_hessian", "stub=fails ok==0; embed=runs session Cartesian Hessian", NWCHEMC_FEATURE_ABI, -1, 1, 1},
    {"abi.nwchemc_available", "include/nwchemc.h::nwchemc_available", "stub=returns 0; embed=returns 1", NWCHEMC_FEATURE_ABI, -1, 1, 1},
    {"abi.nwchemc_version", "include/nwchemc.h::nwchemc_version", "stub=contains stub; embed=library version", NWCHEMC_FEATURE_ABI, -1, 1, 1},
    {"abi.nwchemc_finalize", "include/nwchemc.h::nwchemc_finalize", "stub=no-op; embed=finalize owned runtime", NWCHEMC_FEATURE_ABI, -1, 1, 1},
};

static const size_t k_feature_count = 131;

size_t nwchemc_feature_count(void) { return k_feature_count; }

const NWChemCFeatureEntry *nwchemc_feature_table(void) { return k_features; }

const NWChemCFeatureEntry *nwchemc_feature_find(const char *feature_id) {
  if (!feature_id)
    return NULL;
  for (size_t i = 0; i < k_feature_count; ++i) {
    if (strcmp(k_features[i].feature_id, feature_id) == 0)
      return &k_features[i];
  }
  return NULL;
}

size_t nwchemc_feature_count_class(NWChemCFeatureClass klass) {
  size_t n = 0;
  for (size_t i = 0; i < k_feature_count; ++i) {
    if (k_features[i].klass == klass)
      ++n;
  }
  return n;
}

size_t nwchemc_module_feature_count(void) {
  return nwchemc_feature_count_class(NWCHEMC_FEATURE_MODULE);
}

const char *nwchemc_module_nwchem_name(int module_enum_id) {
  for (size_t i = 0; i < k_feature_count; ++i) {
    if (k_features[i].klass == NWCHEMC_FEATURE_MODULE &&
        k_features[i].enum_or_field_id == module_enum_id) {
      const char *t = k_features[i].nwchem_text_or_role;
      if (t && t[0] == '(')
        return NULL;
      return t;
    }
  }
  return NULL;
}

