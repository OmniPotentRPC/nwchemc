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
  hasCharge @5 :Bool = false;       # @brief Override NWChemParams.charge for this step.
  charge @6 :Int32 = 0;             # @brief Per-step charge when hasCharge is true.
  hasMultiplicity @7 :Bool = false; # @brief Override NWChemParams.multiplicity for this step.
  multiplicity @8 :Int32 = 1;       # @brief Per-step spin multiplicity when hasMultiplicity is true.
}

# @struct PotentialResult
# @brief Results returned from a potential energy evaluation.
struct PotentialResult {
  energy  @0 :Float64;       # @brief The calculated potential energy.
  forces  @1 :List(Float64); # @brief Flat array of atomic forces [natoms * 3].
  hessian @2 :List(Float64); # @brief Dense Cartesian Hessian [natoms * 3]^2.
  dipole  @3 :List(Float64); # @brief Total dipole vector [3] in atomic units.
  quadrupole @4 :List(Float64); # @brief Traceless quadrupole [6] in atomic units.
  optimizedPos @5 :List(Float64); # @brief Optimized Cartesian coordinates [natoms * 3].
  frequencies @6 :List(Float64); # @brief Harmonic vibrational frequencies [natoms * 3] in cm^-1.
  intensities @7 :List(Float64); # @brief Harmonic IR intensities [natoms * 3] in atomic units.
  stress @8 :List(Float64); # @brief Stress tensor [9] in energy/length^3 units.
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

enum NWChemPseudopotentialSpinMode {
  unspecified @0; # Do not emit pspspin.
  enabled     @1; # Emit pspspin on.
  disabled    @2; # Emit pspspin off.
}

struct NWChemPseudopotentialSpinRule {
  channel                  @0 :Channel = up;
  angularMomentum          @1 :AngularMomentum = s;
  scale                    @2 :Float64 = 1.0;
  ionIndices               @3 :List(Int32);
  hasMagneticQuantumNumber @4 :Bool = false;
  magneticQuantumNumber    @5 :Int32 = 0;

  enum Channel {
    up   @0;
    down @1;
  }

  enum AngularMomentum {
    s @0;
    p @1;
    d @2;
    f @3;
  }
}

struct NWChemPseudopotentialUtermRule {
  angularMomentum @0 :AngularMomentum = s;
  uScale          @1 :Float64 = 0.0;
  jScale          @2 :Float64 = 0.0;
  ionIndices      @3 :List(Int32);

  enum AngularMomentum {
    s @0;
    p @1;
    d @2;
    f @3;
  }
}

struct NWChemPseudopotentialStanza {
  entries    @0 :List(NWChemPseudopotentialEntry);
  directives @1 :List(NWChemDirective); # Extra nwpw directives near the block.
  pspSpin    @2 :NWChemPseudopotentialSpinMode = unspecified; # Emit pspspin on/off.
  spinRules  @3 :List(NWChemPseudopotentialSpinRule); # Emit/promote pspspin up/down scaling rules.
  semicoreSmall @4 :NWChemToggle = unspecified; # Promote nwpw:psp:semicore_small.
  uterm      @5 :NWChemToggle = unspecified; # Emit/promote NWPW uterm on/off.
  utermRules @6 :List(NWChemPseudopotentialUtermRule); # Emit/promote indexed uterm rules.
}

struct NWChemKVector {
  x      @0 :Float64 = 0.0; # First reduced Brillouin-zone coordinate.
  y      @1 :Float64 = 0.0; # Second reduced Brillouin-zone coordinate.
  z      @2 :Float64 = 0.0; # Third reduced Brillouin-zone coordinate.
  weight @3 :Float64 = 0.0; # Optional NWChem kvector weight.
}

struct NWChemBrillouinZoneStanza {
  zoneName          @0 :Text = ""; # NWChem zone_name; empty uses zone_default.
  kVectors          @1 :List(NWChemKVector); # Explicit kvector entries.
  maxKpointsPrint   @2 :Int32 = 0; # max_kpoints_print value.
  monkhorstPackX    @3 :Int32 = 0; # First monkhorst-pack grid value.
  monkhorstPackY    @4 :Int32 = 0; # Second monkhorst-pack grid value.
  monkhorstPackZ    @5 :Int32 = 0; # Third monkhorst-pack grid value.
  directives        @6 :List(NWChemDirective);
  zoneStructureName @7 :Text = ""; # Emit/promote band_structure:zone_name.
  zoneFftName       @8 :Text = ""; # Emit/promote band_fft:zone_name.
  tetrahedronGridX  @9 :Int32 = 0; # First NWPW tetrahedron grid value.
  tetrahedronGridY  @10 :Int32 = 0; # Second NWPW tetrahedron grid value.
  tetrahedronGridZ  @11 :Int32 = 0; # Third NWPW tetrahedron grid value.
  tetrahedronZoneName @12 :Text = ""; # Optional trailing NWPW tetrahedron zone token.
  dosGridX          @13 :Int32 = 0; # First NWPW dos-grid value.
  dosGridY          @14 :Int32 = 0; # Second NWPW dos-grid value.
  dosGridZ          @15 :Int32 = 0; # Third NWPW dos-grid value.
  dosGridZoneName   @16 :Text = ""; # Optional trailing NWPW dos-grid zone token.
}

enum NWChemSimulationCellLatticeKind {
  unspecified @0; # Do not emit a compact cubic lattice directive.
  sc          @1; # Emit/promote NWChem "sc".
  fcc         @2; # Emit/promote NWChem "fcc".
  bcc         @3; # Emit/promote NWChem "bcc".
}

struct NWChemSimulationCellStanza {
  cellName            @0  :Text = ""; # NWChem cell_name; empty uses cell_default.
  boundaryConditions  @1  :Text = ""; # periodic, surface, free-space, ...
  latticeVectorsBohr  @2  :List(Float64); # Flat 3x3 simulation-cell vectors in Bohr.
  ngridX              @3  :Int32 = 0; # First ngrid value.
  ngridY              @4  :Int32 = 0; # Second ngrid value.
  ngridZ              @5  :Int32 = 0; # Third ngrid value.
  ngridSmallX         @6  :Int32 = 0; # First ngrid_small value.
  ngridSmallY         @7  :Int32 = 0; # Second ngrid_small value.
  ngridSmallZ         @8  :Int32 = 0; # Third ngrid_small value.
  boxDeltaBohr        @9  :Float64 = 0.0; # box_delta in Bohr.
  boxOrient           @10 :Bool = false; # Emit/promote box_orient.
  boxDifferentLengths @11 :Bool = false; # Emit/promote box_different_lengths.
  latticeKind         @12 :NWChemSimulationCellLatticeKind = unspecified;
  latticeLengthBohr   @13 :Float64 = 0.0; # sc/fcc/bcc compact lattice length in Bohr.
  directives          @14 :List(NWChemDirective);
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

enum NWChemNwpwSpinMode {
  unspecified  @0; # Do not emit/promote dft/odft.
  restricted   @1; # Emit dft; promote CGSD/BAND/CPSD ispin = 1.
  unrestricted @2; # Emit odft; promote CGSD/BAND/CPSD ispin = 2.
}

enum NWChemNwpwMinimizer {
  unspecified   @0; # Do not emit/promote nwpw:minimizer.
  cgGrassman    @1; # Emit cg grassman; promote nwpw:minimizer = 1.
  cgStiefel     @2; # Emit cg stiefel; promote nwpw:minimizer = 4.
  cgStich       @3; # Emit cg stich; promote nwpw:minimizer = 9.
  lmbfgsGrassman @4; # Emit lmbfgs grassman; promote nwpw:minimizer = 2.
  lmbfgsStiefel @5; # Emit lmbfgs stiefel; promote nwpw:minimizer = 7.
  lmbfgsStich   @6; # Emit lmbfgs stich; promote nwpw:minimizer = 10.
  scfDensity    @7; # Emit scf density; promote nwpw:minimizer = 8.
  scfPotential  @8; # Emit scf potential; promote nwpw:minimizer = 5.
}

enum NWChemNwpwKsAlgorithm {
  unspecified @0; # Do not emit/promote nwpw:ks_algorithm.
  blockCg     @1; # Emit block-cg; promote nwpw:ks_algorithm = -1.
  cg          @2; # Emit cg; promote nwpw:ks_algorithm = 0.
  rmmDiis     @3; # Emit rmm-diis; promote nwpw:ks_algorithm = 1.
}

enum NWChemNwpwScfAlgorithm {
  unspecified @0; # Do not emit/promote nwpw:scf_algorithm.
  simple      @1; # Emit simple; promote nwpw:scf_algorithm = 0.
  broyden     @2; # Emit broyden; promote nwpw:scf_algorithm = 1.
  diis        @3; # Emit diis; promote nwpw:scf_algorithm = 2.
  anderson    @4; # Emit anderson; promote nwpw:scf_algorithm = 3.
}

enum NWChemNwpwToggle {
  unspecified @0; # Do not emit the option.
  enabled     @1; # Emit/promote the option as enabled.
  disabled    @2; # Emit/promote the option as disabled.
}

enum NWChemToggle {
  unspecified @0; # Do not emit the option.
  enabled     @1; # Emit/promote the option as enabled.
  disabled    @2; # Emit/promote the option as disabled.
}

enum NWChemPrintLevel {
  unspecified @0; # Do not emit a print directive.
  none        @1; # Emit "print none".
  low         @2; # Emit "print low".
  medium      @3; # Emit "print medium".
  high        @4; # Emit "print high".
  debug       @5; # Emit "print debug".
}

enum NWChemTceReference {
  unspecified @0; # Do not emit tce reference.
  dft         @1; # Emit DFT; promote tce:reference = 0.
  hf          @2; # Emit HF; promote tce:reference = 1.
  scf         @3; # Emit SCF; promote tce:reference = 1.
}

enum NWChemTceTwoElectronStorage {
  unspecified @0; # Do not emit/promote tce:model2e.
  default     @1; # Emit/promote tce:model2e = default.
  orbital     @2; # Emit 2eorb; promote tce:model2e = 2eorb.
  spin        @3; # Emit 2espin; promote tce:model2e = 2espin.
}

enum NWChemTceIoAlgorithm {
  unspecified @0; # Do not emit/promote tce:ioalg.
  fortran     @1; # Emit io fortran; promote tce:ioalg = 0.
  eaf         @2; # Emit io eaf; promote tce:ioalg = 1.
  ga          @3; # Emit io ga; promote tce:ioalg = 2.
  sf          @4; # Emit io sf; promote tce:ioalg = 3.
  replicated  @5; # Emit io replicated; promote tce:ioalg = 4.
  dra         @6; # Emit io dra; promote tce:ioalg = 5.
  gaEaf       @7; # Emit io ga_eaf; promote tce:ioalg = 6.
}

enum NWChemTceFreezeMode {
  unspecified @0; # Do not emit a symbolic TCE freeze directive.
  atomic      @1; # Emit "freeze atomic".
  core        @2; # Emit "freeze core".
  coreAtomic  @3; # Emit "freeze core atomic".
}

enum NWChemNwpwSmearType {
  unspecified        @0; # Do not emit fractional_smeartype.
  fixed              @1; # NWChem smear fixed.
  step               @2; # NWChem smear step.
  fermi              @3; # NWChem smear fermi.
  gaussian           @4; # NWChem smear gaussian.
  marzariVanderbilt  @5; # NWChem smear marzari-vanderbilt.
}

enum NWChemNwpwLcaoMode {
  unspecified @0; # Do not emit lcao/lcao_skip.
  lcao        @1; # Emit/promote NWChem "lcao".
  skip        @2; # Emit/promote NWChem "lcao_skip".
}

enum NWChemNwpwEfieldType {
  unspecified @0; # Do not emit/promote nwpw:efield_type.
  periodic    @1; # Emit/promote periodic electric-field mode.
  apc         @2; # Emit/promote APC electric-field mode.
  rgrid       @3; # Emit/promote rgrid electric-field mode.
}

enum NWChemNwpwMappingAlias {
  unspecified @0; # Do not emit a named NWPW mapping alias.
  slab1d      @1; # Emit/promote NWChem "1d-slab" mapping alias.
  hilbert2d   @2; # Emit/promote NWChem "2d-hilbert" mapping alias.
  hcurve2d    @3; # Emit/promote NWChem "2d-hcurve" mapping alias.
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
  virtualOrbitalsStart        @40 :Int32 = 0;       # First NWPW virtual_orbitals value.
  virtualOrbitalsEnd          @41 :Int32 = 0;       # Second NWPW virtual_orbitals value.
  lcaoMode                    @42 :NWChemNwpwLcaoMode = unspecified; # Emit/promote lcao/lcao_skip.
  ewaldGridX                  @43 :Int32 = 0;       # First NWPW ewald_ngrid value.
  ewaldGridY                  @44 :Int32 = 0;       # Second NWPW ewald_ngrid value.
  ewaldGridZ                  @45 :Int32 = 0;       # Third NWPW ewald_ngrid value.
  noseHoover                  @46 :NWChemNwpwToggle = unspecified; # Emit/promote Nose-Hoover or energy nose toggle.
  noseRestart                 @47 :NWChemNwpwToggle = unspecified; # Emit/promote Nose-Hoover restart/start.
  noseElectronPeriod          @48 :Float64 = 0.0;   # Nose-Hoover electron period Pe.
  noseElectronTemperature     @49 :Float64 = 0.0;   # Nose-Hoover electron temperature Te.
  noseIonPeriod               @50 :Float64 = 0.0;   # Nose-Hoover ion period Pr.
  noseIonTemperature          @51 :Float64 = 0.0;   # Nose-Hoover ion temperature Tr.
  noseElectronChainLength     @52 :Int32 = 0;       # Nose-Hoover Mchain value.
  noseIonChainLength          @53 :Int32 = 0;       # Nose-Hoover Nchain value.
  atomEfield                  @54 :NWChemNwpwToggle = unspecified; # Emit/promote atom_efield.
  atomEfieldGradient          @55 :NWChemNwpwToggle = unspecified; # Emit/promote atom_efield_grad.
  mulliken                    @56 :NWChemNwpwToggle = unspecified; # Emit/promote NWPW mulliken/off.
  mullikenKawai               @57 :NWChemNwpwToggle = unspecified; # Emit/promote NWPW mulliken kawai.
  periodicDipole              @58 :NWChemNwpwToggle = unspecified; # Emit/promote NWPW periodic_dipole.
  electricField               @59 :NWChemNwpwToggle = unspecified; # Emit/promote NWPW efield.
  electricFieldX              @60 :Float64 = 0.0;    # First NWPW efield vector value.
  electricFieldY              @61 :Float64 = 0.0;    # Second NWPW efield vector value.
  electricFieldZ              @62 :Float64 = 0.0;    # Third NWPW efield vector value.
  electricFieldCenterSet      @63 :Bool = false;     # Emit/promote NWPW efield center.
  electricFieldCenterX        @64 :Float64 = 0.0;    # First NWPW efield center value.
  electricFieldCenterY        @65 :Float64 = 0.0;    # Second NWPW efield center value.
  electricFieldCenterZ        @66 :Float64 = 0.0;    # Third NWPW efield center value.
  electricFieldType           @67 :NWChemNwpwEfieldType = unspecified; # Emit/promote NWPW efield type.
  smoothCutoff                @68 :NWChemNwpwToggle = unspecified; # Emit/promote NWPW smooth_cutoff.
  smoothCutoffAfac            @69 :Float64 = 0.0;    # First NWPW smooth_cutoff value.
  smoothCutoffSigma           @70 :Float64 = 0.0;    # Second NWPW smooth_cutoff value.
  cutoffBootWavefunction      @71 :NWChemNwpwToggle = unspecified; # Emit/promote NWPW cutoff_boot_wavefunction.
  fastErf                     @72 :NWChemNwpwToggle = unspecified; # Emit/promote NWPW fast_erf.
  dipoleMotion                @73 :NWChemNwpwToggle = unspecified; # Emit/promote NWPW dipole_motion.
  dipoleMotionFilename        @74 :Text = "";       # Emit/promote NWPW dipole_motion filename.
  rhoUseSymmetry              @75 :NWChemNwpwToggle = unspecified; # Emit/promote NWPW symmetry.
  fmm                         @76 :NWChemNwpwToggle = unspecified; # Emit/promote NWPW fmm.
  fmmLmax                     @77 :Int32 = 0;       # NWPW fmm lmax value.
  fmmLongRange                @78 :Int32 = 0;       # NWPW fmm lr value.
  born                        @79 :NWChemNwpwToggle = unspecified; # Emit/promote NWPW Born solvation.
  bornDielectric              @80 :Float64 = 0.0;   # NWPW born dielectric value.
  bornRelax                   @81 :NWChemNwpwToggle = unspecified; # NWPW born relax flag.
  bornVRadiiAngstrom          @82 :List(Float64);    # NWPW born input radii in Angstrom.
  cpmdProperties              @83 :NWChemNwpwToggle = unspecified; # Emit/promote NWPW cpmd_properties.
  useGridComparison           @84 :NWChemNwpwToggle = unspecified; # Emit/promote NWPW use_grid_cmp.
  director                    @85 :NWChemNwpwToggle = unspecified; # Emit/promote NWPW director.
  directorFilename            @86 :Text = "";       # Emit/promote NWPW director filename.
  cellExpandX                 @87 :Int32 = 0;       # First NWPW expand_cell value.
  cellExpandY                 @88 :Int32 = 0;       # Second NWPW expand_cell value.
  cellExpandZ                 @89 :Int32 = 0;       # Third NWPW expand_cell value.
  mapping                     @90 :Int32 = 0;       # NWPW mapping value.
  rotation                    @91 :NWChemNwpwToggle = unspecified; # Emit/promote NWPW rotation.
  lmaxMultipole               @92 :Int32 = -1;      # NWPW integrate_mult_l / lmax_multipole value.
  fei                         @93 :Bool = false;    # Emit/promote NWPW Fei.
  feiFilename                 @94 :Text = "";       # Filename token in NWPW Fei.
  initialVelocitiesTemperature @95 :Float64 = 0.0;  # NWPW initial_velocities temperature in K.
  initialVelocitiesSeed       @96 :Int32 = -1;      # NWPW initial_velocities seed; negative uses NWChem default.
  makeHmass2                  @97 :NWChemNwpwToggle = unspecified; # Emit/promote NWPW makehmass2.
  translateVectorSet          @98 :Bool = false;    # Emit/promote NWPW translate_vector.
  translateVectorX            @99 :Float64 = 0.0;   # First NWPW translate_vector value.
  translateVectorY            @100 :Float64 = 0.0;  # Second NWPW translate_vector value.
  translateVectorZ            @101 :Float64 = 0.0;  # Third NWPW translate_vector value.
  translateGeometryName       @102 :Text = "";      # Optional NWPW translate_vector geometry name.
  translateReorder            @103 :NWChemNwpwToggle = unspecified; # Optional NWPW translate_vector reorder flag.
  socketType                  @104 :Text = "";      # NWPW socket type token.
  socketIp                    @105 :Text = "";      # Optional NWPW socket IP or socket path token.
  apcSet                      @106 :Bool = false;   # Emit/promote NWPW APC values.
  apcGc                       @107 :Float64 = 0.0;  # NWPW APC Gc value.
  apcGamma                    @108 :List(Float64);  # NWPW APC gamma values.
  translation                 @109 :NWChemNwpwToggle = unspecified; # Emit/promote NWPW translation.
  minimizer                   @110 :NWChemNwpwMinimizer = unspecified; # Emit/promote NWPW minimizer.
  ksAlgorithm                 @111 :NWChemNwpwKsAlgorithm = unspecified; # Emit/promote NWPW KS algorithm.
  scfAlgorithm                @112 :NWChemNwpwScfAlgorithm = unspecified; # Emit/promote NWPW SCF algorithm.
  precondition                @113 :NWChemNwpwToggle = unspecified; # Emit/promote NWPW SCF precondition flag.
  kerkerG0Set                 @114 :Bool = false;   # Emit/promote NWPW SCF kerker g0.
  kerkerG0                    @115 :Float64 = 0.0;  # NWPW SCF kerker g0 value.
  ksAlphaSet                  @116 :Bool = false;   # Emit/promote NWPW SCF alpha.
  ksAlpha                     @117 :Float64 = 0.0;  # NWPW SCF alpha value.
  ksMaxitOrbSet               @118 :Bool = false;   # Emit/promote NWPW SCF inner iterations.
  ksMaxitOrb                  @119 :Int32 = 0;      # NWPW SCF inner iteration count.
  ksMaxitOrbsSet              @120 :Bool = false;   # Emit/promote NWPW SCF outer iterations.
  ksMaxitOrbs                 @121 :Int32 = 0;      # NWPW SCF outer iteration count.
  diisHistoriesSet            @122 :Bool = false;   # Emit/promote NWPW SCF DIIS histories.
  diisHistories               @123 :Int32 = 0;      # NWPW SCF DIIS history count.
  oneElectronGuessSet         @124 :Bool = false;   # Emit/promote NWPW one_electron_guess.
  oneElectronGuessItIn        @125 :Int32 = 0;      # NWPW one-electron guess inner iterations.
  oneElectronGuessItOut       @126 :Int32 = 0;      # NWPW one-electron guess outer iterations.
  oneElectronGuessItOrtho     @127 :Int32 = 0;      # NWPW one-electron guess orthogonalization iterations.
  vfieldFilenames             @128 :List(Text);     # Emit/promote NWPW vfield filenames.
  singlePrecisionHfx          @129 :Bool = false;   # Emit/promote NWPW single_precision_hfx.
  dosAlphaSet                 @130 :Bool = false;   # Emit/promote NWPW DOS alpha.
  dosAlpha                    @131 :Float64 = 0.0;  # NWPW DOS alpha value.
  dosNpointsSet               @132 :Bool = false;   # Emit/promote NWPW DOS point count.
  dosNpoints                  @133 :Int32 = 0;      # NWPW DOS point count.
  dosEminSet                  @134 :Bool = false;   # Emit/promote NWPW DOS minimum energy.
  dosEmin                     @135 :Float64 = 0.0;  # NWPW DOS minimum energy.
  dosEmaxSet                  @136 :Bool = false;   # Emit/promote NWPW DOS maximum energy.
  dosEmax                     @137 :Float64 = 0.0;  # NWPW DOS maximum energy.
  dosFilename                 @138 :Text = "";      # Emit/promote NWPW dos_filename.
  geometryOptimize            @139 :Bool = false;   # Emit/promote NWPW geometry_optimize.
  auxiliaryPotentials         @140 :Bool = false;   # Emit/promote NWPW auxiliary_potentials.
  multiplicity                @141 :Int32 = 0;      # Emit/promote NWPW mult.
  spinMode                    @142 :NWChemNwpwSpinMode = unspecified; # Emit/promote NWPW dft/odft.
  allowTranslation            @143 :Bool = false;   # Emit/promote NWPW allow_translation alias.
  cutoffWavefunction          @144 :Float64 = 0.0;  # Wavefunction value in NWPW cutoff alias.
  cutoffEnergy                @145 :Float64 = 0.0;  # Optional energy value in NWPW cutoff alias.
  mcStepStart                 @146 :Int32 = 0;      # First NWPW mc_steps alias value.
  mcStepEnd                   @147 :Int32 = 0;      # Second NWPW mc_steps alias value.
  etMovecsA                   @148 :Text = "";      # First movecs token in NWPW et.
  etMovecsB                   @149 :Text = "";      # Second movecs token in NWPW et.
  etIonA                      @150 :Text = "";      # First ion token in NWPW et.
  etIonB                      @151 :Text = "";      # Second ion token in NWPW et.
  temperatureIon              @152 :Float64 = 0.0;  # First NWPW temperature argument, Tion.
  temperatureIonPeriod        @153 :Float64 = 0.0;  # Second NWPW temperature argument, Pion.
  temperatureElectron         @154 :Float64 = 0.0;  # Third NWPW temperature argument, Telc.
  temperatureElectronPeriod   @155 :Float64 = 0.0;  # Fourth NWPW temperature argument, Pelc.
  temperatureRestart          @156 :NWChemNwpwToggle = unspecified; # restart/start token in NWPW temperature.
  temperatureIonChainLength   @157 :Int32 = 0;      # First chain argument in NWPW temperature.
  temperatureElectronChainLength @158 :Int32 = 0;   # Second chain argument in NWPW temperature.
  mappingAlias                @159 :NWChemNwpwMappingAlias = unspecified; # Named NWPW mapping alias.
  virtualAliasSet             @160 :Bool = false;   # Emit/promote NWPW virtual alias with defaults.
  virtualAliasStart           @161 :Int32 = 0;      # First NWPW virtual alias value.
  virtualAliasEnd             @162 :Int32 = 0;      # Second NWPW virtual alias value.
  boTimeStepSet               @163 :Bool = false;   # Emit/promote NWPW bo_time_step with the default value when boTimeStep is unset.
  boFakeMassSet               @164 :Bool = false;   # Emit/promote NWPW bo_fake_mass with the default value when boFakeMass is unset.
  scalingSet                  @165 :Bool = false;   # Emit/promote NWPW scaling with default values when scalingFirst is unset.
  npDimensionsSet             @166 :Bool = false;   # Emit/promote NWPW np_dimensions with default values when all dimensions are unset.
  tolerancesSet               @167 :Bool = false;   # Emit/promote NWPW tolerances with default values when all tolerance fields are unset.
  mappingSet                  @168 :Bool = false;   # Emit/promote NWPW mapping with the default value when mapping is unset.
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

# @struct NWChemCcsdStanza
# @brief Classic CCSD block controls backed by stable ccsd:* RTDB keys.
struct NWChemCcsdStanza {
  maxiter           @0 :Int32 = 0; # Emit/promote ccsd:maxiter.
  thresh            @1 :Float64 = 0.0; # Emit/promote ccsd:thresh.
  tol2e             @2 :Float64 = 0.0; # Emit/promote ccsd:tol2e.
  iprt              @3 :Int32 = 0; # Emit/promote ccsd:iprt.
  maxDiis           @4 :Int32 = 0; # Emit/promote ccsd:maxdiis / diisbas.
  frozenCore        @5 :Int32 = 0; # Emit/promote ccsd:frozen core.
  frozenVirtual     @6 :Int32 = 0; # Emit/promote ccsd:frozen virtual.
  useDisk           @7 :NWChemToggle = unspecified; # disabled emits/promotes nodisk.
  sameSpinScale     @8 :Float64 = 0.0; # Emit/promote ccsd:fss.
  oppositeSpinScale @9 :Float64 = 0.0; # Emit/promote ccsd:fos.
  directives        @10 :List(NWChemDirective);
  printLevel        @11 :NWChemPrintLevel = unspecified; # Emit CCSD print level.
  printItems        @12 :List(Text); # Extra CCSD print names after the level.
  noPrintItems      @13 :List(Text); # CCSD noprint names.
  doa               @14 :List(Int32); # Emit CCSD DOA debugging switches.
  dob               @15 :List(Int32); # Emit CCSD DOB debugging switches.
  dog               @16 :List(Int32); # Emit CCSD DOG debugging switches.
  doh               @17 :List(Int32); # Emit CCSD DOH debugging switches.
  dojk              @18 :List(Int32); # Emit CCSD DOJK debugging switches.
  dos               @19 :List(Int32); # Emit CCSD DOS debugging switches.
  dod               @20 :List(Int32); # Emit CCSD DOD debugging switches.
  useTriplesDriverNonblocking @21 :NWChemToggle = unspecified; # Emit/promote ccsd:use_trpdrv_nb.
  useCcsdOpenmp               @22 :NWChemToggle = unspecified; # Emit/promote ccsd:use_ccsd_omp.
  useTriplesDriverOpenmp      @23 :NWChemToggle = unspecified; # Emit/promote ccsd:use_trpdrv_omp.
  useTriplesDriverOffload     @24 :NWChemToggle = unspecified; # Emit/promote ccsd:use_trpdrv_offload.
}

# @struct NWChemTceStanza
# @brief Tensor Contraction Engine controls for CC, CI, MBPT, EOM-CC, and CR-EOMCC.
struct NWChemTceStanza {
  method                @0  :Text = ""; # Full-deck method keyword, e.g. ccsd(t), cr-ccsd(t), bwccsd.
  model                 @1  :Text = ""; # Emit/promote tce:model normalized value.
  model2e               @2  :NWChemTceTwoElectronStorage = unspecified; # Emit/promote tce:model2e.
  perturbative          @3  :Text = ""; # Promote tce:perturbative.
  ccsdVariant           @4  :Text = ""; # Promote tce:ccsdvar.
  noTriplesSingles      @5  :NWChemToggle = unspecified; # Promote tce:nts.
  reference             @6  :NWChemTceReference = unspecified; # Emit/promote tce:reference.
  frozenCore            @7  :Int32 = 0; # Emit/promote tce:frozen core.
  frozenVirtual         @8  :Int32 = 0; # Emit/promote tce:frozen virtual.
  thresh                @9  :Float64 = 0.0; # Emit/promote tce:thresh.
  levelShift            @10 :Float64 = 0.0; # Emit/promote tce:zlshift.
  leftLevelShift        @11 :Float64 = 0.0; # Emit/promote tce:zlshiftl.
  levelShift2Alpha      @12 :Float64 = 0.0; # First value in tce:zlshift2.
  levelShift2Beta       @13 :Float64 = 0.0; # Second value in tce:zlshift2.
  levelShift3Alpha      @14 :Float64 = 0.0; # First value in tce:zlshift3.
  levelShift3Beta       @15 :Float64 = 0.0; # Second value in tce:zlshift3.
  maxiter               @16 :Int32 = 0; # Emit/promote tce:maxiter.
  ioAlgorithm           @17 :NWChemTceIoAlgorithm = unspecified; # Emit/promote tce:ioalg.
  diis                  @18 :Int32 = 0; # Emit/promote tce:diis.
  diis2                 @19 :Int32 = 0; # Emit/promote tce:diis2.
  diis3                 @20 :Int32 = 0; # Emit/promote tce:diis3.
  eomSolver             @21 :Int32 = 0; # Emit eomsol; promote tce:eoms.
  hbarDimension         @22 :Int32 = 0; # Emit hbard; promote tce:hbard.
  nroots                @23 :Int32 = 0; # Emit/promote tce:nroots.
  target                @24 :Int32 = 0; # Emit/promote tce:target.
  targetSymmetry        @25 :Text = ""; # Emit targetsym; promote tce:targetsym.
  symmetry              @26 :NWChemToggle = unspecified; # Emit/promote tce:symmetry.
  densityMatrix         @27 :NWChemToggle = unspecified; # Emit densmat when enabled; promote tce:densmat.
  densityMatrixFile     @28 :Text = ""; # Emit/promote tce:file_densmat.
  left                  @29 :NWChemToggle = unspecified; # Promote tce:left.
  multipole             @30 :Int32 = 0; # Emit/promote tce:multipole.
  fragment              @31 :Int32 = -1; # Emit/promote tce:fragment when >= 0.
  recomputeFock         @32 :NWChemToggle = unspecified; # Emit fock/nofock; promote tce:recompf.
  activeOccupiedAlpha   @33 :Int32 = 0; # Emit/promote tce:active_oa.
  activeOccupiedBeta    @34 :Int32 = 0; # Emit/promote tce:active_ob.
  activeVirtualAlpha    @35 :Int32 = 0; # Emit/promote tce:active_va.
  activeVirtualBeta     @36 :Int32 = 0; # Emit/promote tce:active_vb.
  activeOccupied        @37 :Int32 = 0; # Emit oact; promote tce:oact.
  activeUnoccupied      @38 :Int32 = 0; # Emit uact; promote tce:uact.
  activeEnergyMin       @39 :Float64 = 0.0; # Emit emin_act; promote tce:eactmin.
  activeEnergyMax       @40 :Float64 = 0.0; # Emit emax_act; promote tce:eactmax.
  activeExcitationLevel @41 :Int32 = 0; # Emit t3a_lvl; promote tce:act_excit_lvl.
  maxDiff               @42 :Float64 = 0.0; # Emit/promote tce:maxdiff.
  atomicTileSize        @43 :Int32 = 0; # Emit attilesize; promote tce:maxs.
  split                 @44 :Int32 = 0; # Emit split; promote tce:ichopx.
  twoElectronMethod     @45 :Int32 = 0; # Emit 2emet; promote tce:i4im.
  diskBackend           @46 :Int32 = -1; # Emit idiskx; promote tce:idiskx when >= 0.
  tileSize              @47 :Int32 = 0; # Emit/promote tce:tilesize.
  cudaDevices           @48 :Int32 = 0; # Emit/promote tce:cuda.
  mrcc                  @49 :NWChemToggle = unspecified; # Promote tce:mrcc.
  tccSpaces             @50 :NWChemToggle = unspecified; # Emit tcc_spaces; promote tce:ltcc.
  eaCcsd                @51 :NWChemToggle = unspecified; # Emit/promote tce:eaccsd when enabled.
  ipCcsd                @52 :NWChemToggle = unspecified; # Emit/promote tce:ipccsd when enabled.
  directives            @53 :List(NWChemDirective);
  freezeMode            @54 :NWChemTceFreezeMode = unspecified; # Emit symbolic freeze directive; not directly promoted.
  dipole                @55 :Bool = false; # Emit/promote TCE dipole response keyword.
  printLevel            @56 :NWChemPrintLevel = unspecified; # Emit TCE print level.
  printItems            @57 :List(Text); # Extra TCE print names after the level.
  quadrupole            @58 :Bool = false; # Emit/promote TCE quadrupole response keyword.
  octupole              @59 :Bool = false; # Emit/promote TCE octupole response keyword.
}

# @struct NWChemMrccDataStanza
# @brief TCE MRCC data block for reference roots, active spaces, and occupation strings.
struct NWChemMrccDataStanza {
  root           @0 :Int32 = 0; # Emit "root <n>".
  casElectrons   @1 :Int32 = 0; # First value in "cas <electrons> <orbitals>".
  casOrbitals    @2 :Int32 = 0; # Second value in "cas <electrons> <orbitals>".
  nref           @3 :Int32 = 0; # Emit explicit nref; zero derives from references length.
  references     @4 :List(Text); # MRCC occupation strings following nref.
  se4t           @5 :Bool = false; # Emit "se4t".
  noAposteriori  @6 :Bool = false; # Emit "no_aposteriori".
  subgroupSize   @7 :Int32 = 0; # Emit "subgroupsize <n>".
  improveTiling  @8 :Bool = false; # Emit "improvetiling".
  usspt          @9 :Bool = false; # Emit "usspt".
  directives     @10 :List(NWChemDirective);
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
  ccsd            @14 :NWChemCcsdStanza;
  tce             @15 :NWChemTceStanza;
  mrccData        @16 :NWChemMrccDataStanza;
  brillouinZone   @17 :NWChemBrillouinZoneStanza;
  simulationCell  @18 :NWChemSimulationCellStanza;

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
    ccsd            @13;
    tce             @14;
    mrccData        @15;
    brillouinZone   @16;
    simulationCell  @17;
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
