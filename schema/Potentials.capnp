# @brief RPC schema for distributed potential evaluations.
#
# This schema defines the binary communication contract between the light client
# and the RPC server components. The design derives from the C-style structures
# in the eOn [1] project (at the v4 writeup).
#
# # References
# [1] eOn Development Team. eOn Documentation. https://eondocs.org.

@0xbd1f89fa17369103;

# @struct ForceInput
# @brief Input configuration for a potential energy evaluation.
# @field lengthUnit Unit expression for positions/box (default "angstrom").
# @field energyUnit Unit expression for energy output (default "eV").
# Unit strings are parsed by rgpot::units::unit_conversion_factor().
# Examples: "angstrom", "bohr", "eV", "hartree", "kJ/mol", "kcal/mol".
struct ForceInput {
  pos        @0 :List(Float64); # @brief Flat array of atomic coordinates [natoms * 3].
  atmnrs     @1 :List(Int32);   # @brief Array of atomic numbers [natoms].
  box        @2 :List(Float64); # @brief Simulation cell vectors [9] (row-major 3x3).
  lengthUnit @3 :Text = "angstrom"; # @brief Unit for positions and box vectors.
  energyUnit @4 :Text = "eV";       # @brief Unit for energy and forces output.
}

# @struct PotentialResult
# @brief Results returned from a potential energy evaluation.
struct PotentialResult {
  energy @0 :Float64;       # @brief The calculated potential energy.
  forces @1 :List(Float64); # @brief Flat array of atomic forces [natoms * 3].
}

# @struct NWChemParams
# @brief NWChem-specific knobs (one backend arm inside PotentialConfig / rgpot params).
#
# Not a standalone "rgpot config language" - only used when the active potential
# is NWChem (or configure targets that backend). Same fields in/out via Cap'n Proto.
struct NWChemDirective {
  keyword @0 :Text;       # Directive keyword inside a block, e.g. "convergence".
  args    @1 :List(Text); # Tokenized directive arguments.
}

struct NWChemGenericStanza {
  name       @0 :Text;                   # NWChem block name, e.g. "driver".
  directives @1 :List(NWChemDirective);  # Structured block body.
}

struct NWChemSetDirective {
  key       @0 :Text; # RTDB key, e.g. "dft:avg_fon".
  value     @1 :Text; # Backward-compatible single NWChem input literal.
  valueType @2 :ValueType = auto;
  values    @3 :List(Text); # Preferred structured value list.

  enum ValueType {
    auto    @0; # Infer through NWChem text parser in full decks; direct embed treats as text.
    text    @1; # NWChem "string" set value.
    double  @2;
    integer @3;
    logical @4;
  }
}

struct NWChemDftSmearing {
  sigmaHartree @0 :Float64 = 0.0; # Molecular DFT smear sigma in Hartree.
  mode         @1 :Mode = fixsz;

  enum Mode {
    fixsz   @0;
    nofixsz @1;
  }
}

struct NWChemDftStanza {
  xc         @0 :Text = "";                  # Exchange-correlation keyword.
  direct     @1 :Bool = false;               # Emit "direct".
  smearing   @2 :NWChemDftSmearing;          # Emit "smear ...".
  directives @3 :List(NWChemDirective);      # Extra structured DFT directives.
}

enum NWChemModuleName {
  custom   @0;
  basis    @1;
  bq       @2;
  ccsd     @3;
  cosmo    @4;
  dft      @5;
  dplot    @6;
  drdy     @7;
  driver   @8;
  esp      @9;
  etrans   @10;
  geometry @11;
  gw       @12;
  hessian  @13;
  mcscf    @14;
  md       @15;
  mm       @16;
  mp2      @17;
  ncc      @18;
  nwpw     @19;
  property @20;
  python   @21;
  qmd      @22;
  qmmm     @23;
  rimp2    @24;
  rism     @25;
  scf      @26;
  selci    @27;
  smd      @28;
  tce      @29;
  vib      @30;
  vscf     @31;
  xtb      @32;
  analysis @33;
  argos @34;
  argosDiana @35;
  argosPrep @36;
  argosPrepare @37;
  band @38;
  bandDplot @39;
  brillouinZone @40;
  bsemol @41;
  cckohn @42;
  cellOptimize @43;
  cgsd @44;
  constraints @45;
  cpmd @46;
  cpsd @47;
  ddscf @48;
  diana @49;
  dimpar @50;
  dimqm @51;
  dk @52;
  dmd @53;
  dntmc @54;
  fractionalOccupations @55;
  freeze @56;
  intgrl @57;
  mdXs @58;
  mepgs @59;
  metadynamics @60;
  modelpotential @61;
  neb @62;
  occup @63;
  prepare @64;
  pspFormatter @65;
  pspGenerator @66;
  pspw @67;
  pspwDplot @68;
  pspwQmmm @69;
  pspwWannier @70;
  qmdNamd @71;
  raman @72;
  rel @73;
  rtTddft @74;
  simulationCell @75;
  string @76;
  tamd @77;
  task @78;
  taskShell @79;
  tceMrcc @80;
  tddft @81;
  tddftGradient @82;
  tropt @83;
  vibZone @84;
  waterPseudopotential @85;
  x2c @86;
  zora @87;
}

struct NWChemModuleStanza {
  name       @0 :NWChemModuleName = custom;  # Known NWChem block name.
  customName @1 :Text = "";                  # Block name when name == custom.
  directives @2 :List(NWChemDirective);      # Structured block body.
}

struct NWChemPseudopotentialEntry {
  element     @0 :Text;             # Element symbol, e.g. "Si"; ignored when allElements is true.
  libraryType @1 :LibraryType = library;
  libraryName @2 :Text;             # NWPW library name or file token.
  allElements @3 :Bool = false;     # Use NWChem's "*" default entry for every element.

  enum LibraryType {
    library     @0; # NWChem "library" / PSPW library entry.
    pspwLibrary @1; # Explicit "pspw_library" entry.
    pawLibrary  @2; # PAW library entry.
    cpi         @3; # CPI pseudopotential file entry.
    teter       @4; # Teter pseudopotential file entry.
  }
}

struct NWChemPseudopotentialStanza {
  entries    @0 :List(NWChemPseudopotentialEntry);
  directives @1 :List(NWChemDirective); # Extra nwpw directives near the block.
}

enum NWChemNwpwBalanceMode {
  unspecified @0; # Do not emit balance/nobalance.
  balance     @1; # Emit/promote NWChem "balance".
  nobalance   @2; # Emit/promote NWChem "nobalance".
}

enum NWChemNwpwBoAlgorithm {
  unspecified    @0; # Do not emit bo_algorithm.
  verlet         @1; # NWChem bo_algorithm verlet.
  velocityVerlet @2; # NWChem bo_algorithm velocity-verlet.
  leapFrog       @3; # NWChem bo_algorithm leap-frog.
}

enum NWChemNwpwToggle {
  unspecified @0; # Do not emit the option.
  enabled     @1; # Emit/promote the option as enabled.
  disabled    @2; # Emit/promote the option as disabled.
}

enum NWChemNwpwSmearType {
  unspecified        @0; # Do not emit fractional_smeartype.
  fixed              @1; # NWChem smear fixed.
  step               @2; # NWChem smear step.
  fermi              @3; # NWChem smear fermi.
  gaussian           @4; # NWChem smear gaussian.
  marzariVanderbilt  @5; # NWChem smear marzari-vanderbilt.
}

struct NWChemNwpwStanza {
  energyCutoff       @0 :Float64 = 0.0; # Emit/promote NWPW energy_cutoff.
  wavefunctionCutoff @1 :Float64 = 0.0; # Emit/promote NWPW wavefunction_cutoff.
  ewaldRcut          @2 :Float64 = 0.0; # Emit/promote NWPW ewald_rcut.
  ewaldNcut          @3 :Int32 = 0;     # Emit/promote NWPW ewald_ncut.
  directives         @4 :List(NWChemDirective);
  cellName                    @5  :Text = ""; # Emit/promote NWPW cell_name.
  inputWavefunctionFilename   @6  :Text = ""; # Emit/promote NWPW input_wavefunction_filename.
  outputWavefunctionFilename  @7  :Text = ""; # Emit/promote NWPW output_wavefunction_filename.
  fakeMass                    @8  :Float64 = 0.0; # Emit/promote NWPW fake_mass.
  timeStep                    @9  :Float64 = 0.0; # Emit/promote NWPW time_step.
  loopStart                   @10 :Int32 = 0;     # First value in NWPW loop.
  loopEnd                     @11 :Int32 = 0;     # Second value in NWPW loop.
  toleranceEnergy             @12 :Float64 = 0.0; # First NWPW tolerances value.
  toleranceDensity            @13 :Float64 = 0.0; # Second NWPW tolerances value.
  toleranceGradient           @14 :Float64 = 0.0; # Third NWPW tolerances value.
  exchangeCorrelation         @15 :Text = "";      # Emit/promote NWPW exchange_correlation.
  balanceMode                 @16 :NWChemNwpwBalanceMode = unspecified; # Emit/promote balance/nobalance.
  boStepStart                 @17 :Int32 = 0;       # First NWPW bo_steps value.
  boStepEnd                   @18 :Int32 = 0;       # Second NWPW bo_steps value.
  boTimeStep                  @19 :Float64 = 0.0;   # Emit/promote NWPW bo_time_step.
  boAlgorithm                 @20 :NWChemNwpwBoAlgorithm = unspecified; # Emit/promote NWPW bo_algorithm.
  boFakeMass                  @21 :Float64 = 0.0;   # Emit/promote NWPW bo_fake_mass.
  scalingFirst                @22 :Float64 = 0.0;   # First NWPW scaling value.
  scalingSecond               @23 :Float64 = 0.0;   # Second NWPW scaling value.
  npFftProcesses              @24 :Int32 = 0;       # First NWPW np_dimensions value.
  npOrbitalProcesses          @25 :Int32 = 0;       # Second NWPW np_dimensions value.
  npKspaceProcesses           @26 :Int32 = 0;       # Third NWPW np_dimensions value.
  spinOrbit                   @27 :NWChemNwpwToggle = unspecified; # Emit/promote NWPW spin_orbit.
  parallelIo                  @28 :NWChemNwpwToggle = unspecified; # Emit/promote NWPW parallel_io.
  xyzFilename                 @29 :Text = "";       # Emit/promote NWPW xyz_filename.
  ionMotionFilename           @30 :Text = "";       # Emit/promote NWPW ion_motion_filename.
  electronMotionFilename      @31 :Text = "";       # Emit/promote NWPW emotion_filename.
  hamiltonianMotionFilename   @32 :Text = "";       # Emit/promote NWPW hmotion_filename.
  orbitalMotionFilename       @33 :Text = "";       # Emit/promote NWPW omotion_filename.
  eigenvalueMotionFilename    @34 :Text = "";       # Emit/promote NWPW eigmotion_filename.
  fractionalOrbitalsStart     @35 :Int32 = 0;       # First NWPW fractional_orbitals value.
  fractionalOrbitalsEnd       @36 :Int32 = 0;       # Second NWPW fractional_orbitals value.
  smearTemperature            @37 :Float64 = 0.0;   # Emit/promote NWPW smear temperature RTDB value.
  smearAlpha                  @38 :Float64 = 0.0;   # Emit/promote NWPW smear alpha.
  smearType                   @39 :NWChemNwpwSmearType = unspecified; # Emit/promote NWPW smear type.
}

# @struct NWChemScfStanza
# @brief Typed SCF/HF block controls (vectors, convergence, thresh).
# Extra directives cover the long tail of SCF options via NWChemDirective.
struct NWChemScfStanza {
  vectorsInput  @0 :Text = "";   # Emit "vectors input <path>" when non-empty.
  vectorsOutput @1 :Text = "";   # Emit "vectors output <path>" when non-empty.
  maxiter       @2 :Int32 = 0;   # SCF max iterations; embed writes RTDB directly.
  thresh        @3 :Float64 = 0; # SCF convergence threshold; embed writes RTDB directly.
  tol2e         @4 :Float64 = 0; # Two-electron tolerance; embed writes RTDB directly.
  noprint       @5 :Bool = false;# Emit "noprint".
  directives    @6 :List(NWChemDirective);
}

# @struct NWChemTaskStanza
# @brief Explicit NWChem "task <theory> <operation>" line.
# Prefer top-level theory/task for embed defaults; use this stanza when emitting
# a full input deck with multiple tasks or non-default theory/operation pairs.
struct NWChemTaskStanza {
  theory    @0 :Text = ""; # scf, dft, mp2, tce, ... (empty => caller omits theory token).
  operation @1 :Text = ""; # energy, gradient, hessian, optimize, property, ...
  ignore    @2 :Bool = false; # Emit "ignore" suffix when true.
}

# @struct NWChemDriverStanza
# @brief Geometry optimization / driver block.
struct NWChemDriverStanza {
  maxiter    @0 :Int32 = 0;     # Driver max steps; embed writes RTDB directly.
  tight      @1 :Bool = false;  # Tight convergence; embed writes RTDB directly.
  loose      @2 :Bool = false;  # Loose convergence; embed writes RTDB directly.
  xyz        @3 :Text = "";     # Emit "xyz <path>" when non-empty.
  directives @4 :List(NWChemDirective);
  gmaxTol    @5 :Float64 = 0;   # Emit "gmax"; embed writes driver:gmax_tol.
  grmsTol    @6 :Float64 = 0;   # Emit "grms"; embed writes driver:grms_tol.
  xmaxTol    @7 :Float64 = 0;   # Emit "xmax"; embed writes driver:xmax_tol.
  xrmsTol    @8 :Float64 = 0;   # Emit "xrms"; embed writes driver:xrms_tol.
}

# @struct NWChemPropertyStanza
# @brief Property evaluation block (dipole, mulliken, ...).
struct NWChemPropertyStanza {
  dipole     @0 :Bool = false;
  mulliken   @1 :Bool = false;
  quadrupol  @2 :Bool = false; # NWChem keyword "quadrupole" (typo preserved in field name only).
  directives @3 :List(NWChemDirective);
}

# @struct NWChemBasisStanza
# @brief Structured Gaussian basis / ECP block (complements top-level basis name).
# Use when callers need spherical/cartesian, segment, or per-element library lines.
struct NWChemBasisStanza {
  spherical  @0 :Bool = false; # Emit "* library ... spherical" style when true.
  segment    @1 :Text = "";    # Optional segment label for "* library <segment>".
  ecp        @2 :Text = "";    # Optional ECP library/block name emitted as extra line.
  directives @3 :List(NWChemDirective); # Per-element "H library 6-31g" etc.
}

# @struct NWChemGeometryStanza
# @brief Geometry block metadata (units/symmetry/noautosym). Coordinates normally
# come from the C ABI positions/atomic_numbers arrays, not this stanza.
struct NWChemGeometryStanza {
  units      @0 :Text = "";    # angstrom, bohr, au, nm, ...
  symmetry   @1 :Text = "";    # c1, d2h, ... or empty.
  noautosym  @2 :Bool = false;
  noautoz    @3 :Bool = false;
  center     @4 :Bool = false; # Emit "center".
  directives @5 :List(NWChemDirective); # Extra geometry directives (not atom lines).
}

struct NWChemInputStanza {
  kind            @0 :Kind = generic;
  generic         @1 :NWChemGenericStanza;
  dft             @2 :NWChemDftStanza;
  set             @3 :NWChemSetDirective;
  raw             @4 :Text;
  module          @5 :NWChemModuleStanza;
  pseudopotential @6 :NWChemPseudopotentialStanza;
  scf             @7 :NWChemScfStanza;
  taskStanza      @8 :NWChemTaskStanza;
  driver          @9 :NWChemDriverStanza;
  property        @10 :NWChemPropertyStanza;
  basisStanza     @11 :NWChemBasisStanza;
  geometry        @12 :NWChemGeometryStanza;
  nwpw            @13 :NWChemNwpwStanza;

  enum Kind {
    generic         @0;
    dft             @1;
    set             @2;
    raw             @3;
    module          @4;
    pseudopotential @5;
    scf             @6;
    task            @7; # NWChemInputStanza.taskStanza
    driver          @8;
    property        @9;
    basis           @10; # NWChemInputStanza.basisStanza
    geometry        @11;
    nwpw            @12;
  }
}

struct NWChemParams {
  basis        @0 :Text = "sto-3g";  # Gaussian basis (sto-3g, 6-31g*, ...).
  theory       @1 :Text = "scf";     # scf | dft | blyp | b3lyp | ...
  scfType      @2 :Text = "rhf";     # HF: rhf/uhf; DFT: xc (blyp, b3lyp, ...).
  charge       @3 :Int32 = 0;
  multiplicity @4 :Int32 = 1;        # 2S+1.
  enginePath   @5 :Text = "";        # libnwchem_engine.so (dlopen); empty => probe/env.
  nwchemRoot   @6 :Text = "";        # NWCHEM_TOP for embed; empty => env.
  task         @7 :Text = "gradient"; # energy | gradient | property; frontend usually calls gradient.
  title        @8 :Text = "";         # Optional NWChem title/start prefix.
  memoryMb     @9 :UInt32 = 0;        # 0 => NWChem defaults / environment.
  scratchDir   @10 :Text = "";        # Optional NWChem scratch directory.
  permanentDir @11 :Text = "";        # Optional NWChem permanent directory.
  inputBlocks  @12 :List(Text);       # Raw NWChem directive blocks applied before task.
  inputStanzas @13 :List(NWChemInputStanza); # Structured NWChem input stanzas.
  # Long-tail / method-specific NWChem options not yet typed above: use
  # NWChemInputStanza.raw, inputBlocks, NWChemSetDirective, or NWChemModuleStanza.custom.
}

# Future backend option structs (extend here, then add a PotentialConfig union arm):
#   struct XTBParams { method @0 :Text = "GFN2-xTB"; ... }
#   struct TBLiteParams { method @0 :Text = "GFN2-xTB"; ... }
#   struct MetatomicParams { modelPath @0 :Text; device @1 :Text = "cpu"; ... }

# @struct PotentialConfig
# @brief **rgpot user parameters (extensible, in/out via Cap'n Proto only).**
#
# This is the single user-facing options carrier for rgpot: pass in to configure
# a live Potential (RPC `configure` or in-process apply), and/or round-trip out
# when a backend supports get. One schema for wire + embed - no parallel
# TOML/JSON/YAML option files for backends.
#
# Tagged union: exactly one backend's options (or none). Add new arms as new
# potentials gain runtime knobs (e.g. metatomic @2 :MetatomicParams).
# `calculate` geometry stays on ForceInput; this struct is method/backend setup only.
struct PotentialConfig {
  union {
    none      @0 :Void;         # No backend-specific options (or no-op configure).
    nwchem    @1 :NWChemParams; # NWChemPot / potserv ... NWChem
    # metatomic @2 :MetatomicParams;  # reserved pattern for later
    # xtb       @3 :XTBParams;
    # tblite    @4 :TBLiteParams;
  }
}

# @interface Potential
# @brief The RPC interface for remote calculations.
interface Potential {
  # @brief Executes the potential and force calculation.
  # @param fip The input atomic configuration.
  # @return The resulting energy and force vector.
  calculate @0 (fip :ForceInput) -> (result :PotentialResult);

  # @brief Apply rgpot user parameters (PotentialConfig) before calculate().
  # @param config Backend-tagged options (nwchem, future metatomic/xtb/...).
  # @return ok=false if the arm does not match the server backend or apply failed.
  configure @1 (config :PotentialConfig) -> (ok :Bool, message :Text);
}
