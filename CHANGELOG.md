# Changelog

All notable user-facing changes to this project are documented in this file.
This project follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

<!-- towncrier release notes start -->

## [1.3.0](https://github.com/OmniPotentRPC/nwchemc/tree/v1.3.0) - 2026-08-10

### Added

- Expose session and one-shot ForceInput energy-only C ABI wrappers. ([#calculate-energy-abi](https://github.com/OmniPotentRPC/nwchemc/issues/calculate-energy-abi))
- Expose a one-shot `nwchemc_calculate_forces` ForceInput wrapper for native force buffers. ([#calculate-forces-abi](https://github.com/OmniPotentRPC/nwchemc/issues/calculate-forces-abi))
- Wire public **capnp-fortran** into the embed apply path: Fortran decodes
  serialized `NWChemParams` for config, SCF, DFT, driver, NWPW cutoffs, basis,
  Brillouin, pseudopotential, and set stanzas via `nwchemc_embed_apply_params`.
  Exploded `set_*_direct` setters remain for CommonMethodSpec overlay only.
  C still renders `input_blocks` and expands NWPW/etc. into typed RTDB packs. ([#capnp-fortran-scf-apply](https://github.com/OmniPotentRPC/nwchemc/issues/capnp-fortran-scf-apply))
- Classic CCSD input now models `print` and `noprint` controls as structured fields rendered through the CCSD stanza. ([#ccsd-print-controls](https://github.com/OmniPotentRPC/nwchemc/issues/ccsd-print-controls))
- Classic CCSD input now models the alternate triples-driver RTDB toggles as structured fields and promotes them directly in embed builds. ([#ccsd-rtdb-toggles](https://github.com/OmniPotentRPC/nwchemc/issues/ccsd-rtdb-toggles))
- Classic CCSD input now models the documented `doa`, `dob`, `dog`, `doh`, `dojk`, `dos`, and `dod` switch arrays as structured integer lists. ([#ccsd-switch-arrays](https://github.com/OmniPotentRPC/nwchemc/issues/ccsd-switch-arrays))
- Compact NWPW simulation-cell lattice forms (`sc`, `fcc`, and `bcc`) now promote
  directly to `<cell>:unita` RTDB vectors in embed builds. ([#compact-cell-direct](https://github.com/OmniPotentRPC/nwchemc/issues/compact-cell-direct))
- Embed-config cmocka asserts cosmo and DFT-dispersion RTDB promo keys from the
  new-stanzas fixture on the real energy_gradient path. ([#cosmo-disp-embed-promo](https://github.com/OmniPotentRPC/nwchemc/issues/cosmo-disp-embed-promo))
- Added a real-NWChem ForceInput.box regression that verifies parsed periodic cell vectors reach the geometry RTDB in Bohr units. ([#forceinput-cell-rtdb](https://github.com/OmniPotentRPC/nwchemc/issues/forceinput-cell-rtdb))
- Expose ForceInput gradient raw-buffer and PotentialResult result-carrier C ABI entry points. ([#forceinput-gradient-result-abi](https://github.com/OmniPotentRPC/nwchemc/issues/forceinput-gradient-result-abi))
- `ForceInput` can override session charge and spin multiplicity for serialized
  step evaluations while retaining `NWChemParams` defaults for unset fields and
  raw C-array session calls. ([#forceinput-state](https://github.com/OmniPotentRPC/nwchemc/issues/forceinput-state))
- Extended the real-NWChem ForceInput RTDB regression to verify per-step charge
  and spin multiplicity storage alongside periodic cell vectors. ([#forceinput-state-rtdb](https://github.com/OmniPotentRPC/nwchemc/issues/forceinput-state-rtdb))
- Added raw ForceInput frequency-detail ABI calls for normal modes, projected vibration lists, and thermochemistry buffers. ([#frequency-detail-raw-abi](https://github.com/OmniPotentRPC/nwchemc/issues/frequency-detail-raw-abi))
- Expose Hessian calculations through the Cap'n Proto `PotentialResult` result
  carrier with session and one-shot C ABI entry points. ([#hessian-result-abi](https://github.com/OmniPotentRPC/nwchemc/issues/hessian-result-abi))
- Real-NWChem Hessian coverage now checks Bohr/eV `PotentialResult.hessian`
  conversion for one-shot, `PotentialConfig`, and session C ABI calls. ([#hessian-unit-result](https://github.com/OmniPotentRPC/nwchemc/issues/hessian-unit-result))
- Installed CMake and pkg-config consumer smokes now run invalid-input ABI
  checks across the rgpot result-carrier, raw ForceInput, session, and
  coordinate entry points. ([#installed-consumer-invalid-input](https://github.com/OmniPotentRPC/nwchemc/issues/installed-consumer-invalid-input))
- Installed CMake and pkg-config consumers now compile-link the rgpot-facing
  `PotentialResult` sizing and calculation ABI. ([#installed-result-abi-smoke](https://github.com/OmniPotentRPC/nwchemc/issues/installed-result-abi-smoke))
- Add `tools/nwchemc_mpi_force_timer` and `tools/nwchemc_strong_scaling_sweep.sh` for
  multi-rank timing of the public `nwchemc_energy_gradient` path under `mpirun`. ([#mpi-force-timer](https://github.com/OmniPotentRPC/nwchemc/issues/mpi-force-timer))
- TCE MRCCDATA input now has a structured Cap'n Proto stanza for roots, active spaces, reference occupation strings, subgroup tiling, and USS correction cards. ([#mrccdata-stanza](https://github.com/OmniPotentRPC/nwchemc/issues/mrccdata-stanza))
- Structured `neb` stanza fields (nbeads, kbeads, steps, stepsize, trust, nhist,
  algorithm, reset, convergence presets, and gmax/grms/xmax/xrms) now promote to
  the matching `neb:*` RTDB keys from `neb_input.F` on the embed set_params path. ([#neb-rtdb-promo](https://github.com/OmniPotentRPC/nwchemc/issues/neb-rtdb-promo))
- Promote structured NWPW `allowTranslation` to direct CGSD/BAND `allow_translation` RTDB keys in embed builds. ([#nwpw-allow-translation](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-allow-translation))
- Promote NWPW APC Gc and gamma controls through typed Cap'n Proto fields and direct RTDB writes. ([#nwpw-apc](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-apc))
- Expose NWPW `atomEfield` and `atomEfieldGradient` toggles as typed Cap'n
  Proto fields and promote them to `nwpw:atom_efield` and
  `nwpw:atom_efield_grad` RTDB keys in embed mode. ([#nwpw-atom-efield](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-atom-efield))
- Promote structured NWPW `auxiliaryPotentials` to the direct `pspw_qmmm_auxon` RTDB flag in embed builds. ([#nwpw-auxiliary-potentials](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-auxiliary-potentials))
- Expose NWPW Born solvation controls as typed Cap'n Proto fields and promote
  them to `nwpw:born`, `nwpw:born_dielec`, `nwpw:born_relax`, and
  `nwpw:born_vradii` RTDB values in embed builds. ([#nwpw-born](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-born))
- Promote NWPW Brillouin-zone structure and FFT zone aliases through typed Cap'n Proto fields and direct RTDB string writes. ([#nwpw-brillouin-aliases](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-brillouin-aliases))
- Expose NWPW `cellExpandX`, `cellExpandY`, `cellExpandZ`, and `mapping` as
  typed Cap'n Proto fields and promote them to `nwpw:cell_expand` and
  `nwpw:mapping` RTDB values in embed builds. ([#nwpw-cell-mapping](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-cell-mapping))
- The real-NWChem configured NWPW RTDB probe now covers auxiliary potentials,
  NWPW multiplicity, DOS scalar, director, grid-comparison, and FEI controls,
  and the Fortran bridge accepts the same 256 direct RTDB set entries as the C
  ABI. ([#nwpw-configured-control-rtdb-probe](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-configured-control-rtdb-probe))
- Added a real NWChem configured RTDB smoke for NWPW controls used by PSPW-style rgpot integrations. ([#nwpw-configured-rtdb](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-configured-rtdb))
- Configured NWPW RTDB coverage now checks spin-orbit, parallel I/O, Nose-Hoover, electric-field, Mulliken, dipole-motion, FMM, and Born-relax controls against a real embedded NWChem RTDB. ([#nwpw-configured-rtdb-controls](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-configured-rtdb-controls))
- Expose NWPW `cpmdProperties` and `useGridComparison` as typed Cap'n Proto
  fields and promote them to `nwpw:cpmd_properties` and `nwpw:use_grid_cmp`
  RTDB values in embed builds. ([#nwpw-cpmd-grid](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-cpmd-grid))
- Promote structured NWPW `cutoff` alias controls to direct PSPW/BAND/CPSD/CPMD cutoff RTDB values in embed builds. ([#nwpw-cutoff-alias](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-cutoff-alias))
- Expose NWPW `cutoffBootWavefunction` as a typed Cap'n Proto field and
  promote it to the `nwpw:cutoff_boot_psi` RTDB logical key in embed mode. ([#nwpw-cutoff-boot](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-cutoff-boot))
- Expose NWPW `dipoleMotion` and `dipoleMotionFilename` as typed Cap'n
  Proto fields and promote them to `nwpw:dipole_motion` and
  `nwpw:dipole_motion_filename` RTDB keys in embed mode. ([#nwpw-dipole-motion](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-dipole-motion))
- Expose NWPW `director` and `directorFilename` as typed Cap'n Proto fields and
  promote them to `nwpw:use_director` and `nwpw:director_filename` RTDB values
  in embed builds. ([#nwpw-director](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-director))
- Promote structured NWPW DOS scalar controls and `dosFilename` to direct `dos:*` and `nwpw:dos:filename` RTDB keys in embed builds. ([#nwpw-dos-controls](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-dos-controls))
- Add structured NWPW `dos-fft-grid` controls and promote generated DOS-style Brillouin zones in embed builds. ([#nwpw-dos-fft-grid](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-dos-fft-grid))
- Add structured NWPW `dos-grid` controls and promote `band:dos-grid` in embed builds. ([#nwpw-dos-grid](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-dos-grid))
- Expose NWPW external electric-field controls as typed Cap'n Proto fields and
  promote them to `nwpw:efield`, `nwpw:efield_vector`,
  `nwpw:efield_center`, and `nwpw:efield_type` RTDB keys in embed mode. ([#nwpw-efield](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-efield))
- Add structured NWPW `et` movecs and ion string controls with direct `pspw:et:*` promotion. ([#nwpw-et-strings](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-et-strings))
- Expose NWPW `fastErf` as a typed Cap'n Proto field and promote it to
  the `nwpw:fast_erf` RTDB logical key in embed mode. ([#nwpw-fast-erf](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-fast-erf))
- Promote NWPW FEI controls through typed Cap'n Proto fields and direct RTDB writes. ([#nwpw-fei](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-fei))
- The real-NWChem configured NWPW RTDB probe now covers structured vfield and DOS
  filename controls. ([#nwpw-filename-rtdb-probe](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-filename-rtdb-probe))
- Expose NWPW `fmm`, `fmmLmax`, and `fmmLongRange` as typed Cap'n Proto
  fields and promote them to `nwpw:fmm`, `nwpw:fmm_lmax`, and `nwpw:fmm_lr`
  RTDB keys in embed mode. ([#nwpw-fmm](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-fmm))
- Promote structured NWPW fractional occupation controls through typed Cap'n Proto fields and direct RTDB writes. ([#nwpw-fractional-occupations](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-fractional-occupations))
- Promote structured NWPW `geometryOptimize` to the direct CGSD/CPSD/BAND geometry optimization RTDB flags in embed builds. ([#nwpw-geometry-optimize](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-geometry-optimize))
- Promote NWPW initial-velocity temperature and seed controls through typed Cap'n Proto fields and direct RTDB writes. ([#nwpw-initial-velocities](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-initial-velocities))
- Add structured NWPW mapping alias controls for `1d-slab`, `2d-hilbert`, and `2d-hcurve`. ([#nwpw-mapping-alias](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-mapping-alias))
- Add structured NWPW `mc_steps` alias controls and promote them to the same `nwpw:bo_steps` RTDB values used by NWChem. ([#nwpw-mc-steps-alias](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-mc-steps-alias))
- Promote NWPW CG, LMBFGS, and SCF minimizer modes through a typed Cap'n Proto enum and direct RTDB write. ([#nwpw-minimizer](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-minimizer))
- Expose NWPW `mulliken` and `mullikenKawai` toggles as typed Cap'n Proto
  fields and promote them to the matching Mulliken RTDB logical keys in embed
  mode. ([#nwpw-mulliken-toggle](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-mulliken-toggle))
- Promote structured NWPW `multiplicity` to direct CGSD/BAND/CPSD `ispin` and `mult` RTDB keys in embed builds. ([#nwpw-multiplicity](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-multiplicity))
- Promote NWPW one-electron guess iteration controls through typed Cap'n Proto fields and direct RTDB writes. ([#nwpw-one-electron-guess](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-one-electron-guess))
- Expose NWPW `periodicDipole` as a typed Cap'n Proto field and promote it to
  the `nwpw:periodic_dipole` RTDB logical key in embed mode. ([#nwpw-periodic-dipole](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-periodic-dipole))
- Expose NWPW `rhoUseSymmetry` as a typed Cap'n Proto field and promote it
  to the `nwpw:rho_use_symmetry` RTDB logical key in embed mode. ([#nwpw-rho-symmetry](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-rho-symmetry))
- Expose NWPW `rotation` and `lmaxMultipole` as typed Cap'n Proto fields and
  promote them to `nwpw:rotation` and `nwpw:lmax_multipole` RTDB values in
  embed builds. ([#nwpw-rotation-multipole](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-rotation-multipole))
- Added a real NWChem RTDB regression test for promoted NWPW direct controls
  covering BO steps, minimizer/SCF controls, motion, APC, Born, mapping, and
  cell-expansion keys. ([#nwpw-rtdb-regression](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-rtdb-regression))
- Promote structured NWPW scaling atom indexes through typed Cap'n Proto fields and direct RTDB writes. ([#nwpw-scaling-atoms](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-scaling-atoms))
- Promote NWPW SCF KS algorithm, mixing algorithm, and precondition controls through typed Cap'n Proto fields and direct RTDB writes. ([#nwpw-scf-algorithms](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-scf-algorithms))
- Promote NWPW SCF kerker, alpha, iteration, and DIIS history controls through typed Cap'n Proto fields and direct RTDB writes. ([#nwpw-scf-numeric](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-scf-numeric))
- Promote structured NWPW `singlePrecisionHfx` to the direct `pspw:HFX_single_precision` RTDB key in embed builds. ([#nwpw-single-precision-hfx](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-single-precision-hfx))
- Expose NWPW `smoothCutoff` controls as typed Cap'n Proto fields and
  promote enabled values to the `nwpw:smooth_cutoff` RTDB double pair in
  embed mode. ([#nwpw-smooth-cutoff](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-smooth-cutoff))
- Promote NWPW socket type and address controls through typed Cap'n Proto fields and direct RTDB writes. ([#nwpw-socket](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-socket))
- Promote structured NWPW `spinMode` aliases to direct CGSD/BAND/CPSD `ispin` RTDB keys in embed builds. ([#nwpw-spin-mode](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-spin-mode))
- Add structured NWPW `temperature` alias controls and promote them through the paired Nose-Hoover RTDB keys. ([#nwpw-temperature-alias](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-temperature-alias))
- Add structured NWPW `tetrahedron` grid controls and promote `band:tetrahedron-grid` in embed builds. ([#nwpw-tetrahedron-grid](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-tetrahedron-grid))
- Promote NWPW translate-vector controls through typed Cap'n Proto fields and direct RTDB writes. ([#nwpw-translate-vector](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-translate-vector))
- Promote NWPW translation controls through a typed Cap'n Proto toggle and direct RTDB writes. ([#nwpw-translation](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-translation))
- Promote NWPW uterm potential rules through typed Cap'n Proto pseudopotential fields and direct indexed RTDB writes. ([#nwpw-uterm](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-uterm))
- Promote structured NWPW `vfieldFilenames` values to the direct `nwpw:vfield_filenames` RTDB key in embed builds. ([#nwpw-vfield](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-vfield))
- Add structured NWPW `virtual` alias controls and promote them through `nwpw:excited_ne`. ([#nwpw-virtual-alias](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-virtual-alias))
- Added C ABI and `PotentialResult` result-carrier entry points for NWChem polarizability. ([#polarizability-abi](https://github.com/OmniPotentRPC/nwchemc/issues/polarizability-abi))
- Added a real-NWChem `PotentialConfig.nwchem` pseudopotential integration test
  that drives the public config path and verifies the promoted RTDB keys. ([#potential-config-pseudopotential-rtdb](https://github.com/OmniPotentRPC/nwchemc/issues/potential-config-pseudopotential-rtdb))
- CLI-vs-embed integration matrix now compares primary observables beyond
  energy and gradient: optimize positions, frequency magnitudes, dipole,
  quadrupole, and Hessian (via NWChem ``.hess``), driven by shipped C ABI
  probes and ``tests/integration/task_matrix.json``. Polarizability remains
  embed-proven with a non-trivial magnitude gate without inventing CLI tensors. ([#primary-cli-matrix](https://github.com/OmniPotentRPC/nwchemc/issues/primary-cli-matrix))
- Expose dipole and quadrupole calculations through the Cap'n Proto
  `PotentialResult` result carrier with session and one-shot C ABI entry points. ([#property-result-abi](https://github.com/OmniPotentRPC/nwchemc/issues/property-result-abi))
- Real-NWChem rgpot smoke coverage now checks eV `PotentialResult.energy`
  conversion for dipole, polarizability, and quadrupole result carriers while
  leaving their property values in atomic units. ([#property-result-energy-unit](https://github.com/OmniPotentRPC/nwchemc/issues/property-result-energy-unit))
- Added a c-capnproto pseudopotential entry walker so embed setup can consume generated entry values directly before the final C/Fortran packing step. ([#pseudopotential-capnp-walk](https://github.com/OmniPotentRPC/nwchemc/issues/pseudopotential-capnp-walk))
- Structured pseudopotential stanzas now expose NWChem's `pseudopotential_libraries` block alias. ([#pseudopotential-library-alias](https://github.com/OmniPotentRPC/nwchemc/issues/pseudopotential-library-alias))
- Added a real NWChem RTDB regression for direct pseudopotential library, PAW, CPI, Teter, reset, all-elements, pspspin, semicore, and uterm key writes. ([#pseudopotential-rtdb-regression](https://github.com/OmniPotentRPC/nwchemc/issues/pseudopotential-rtdb-regression))
- Expose the NWPW pseudopotential `semicoreSmall` toggle as a typed Cap'n Proto
  field and promote it to the `nwpw:psp:semicore_small` RTDB key in embed mode. ([#pseudopotential-semicore-toggle](https://github.com/OmniPotentRPC/nwchemc/issues/pseudopotential-semicore-toggle))
- Real-NWChem pseudopotential coverage now verifies that a session created from a
  structured pseudopotential config can be reconfigured with a reset config before
  topology is accepted. ([#pseudopotential-session-reset](https://github.com/OmniPotentRPC/nwchemc/issues/pseudopotential-session-reset))
- Promote `pspSpin = disabled` to direct NWPW RTDB reset keys in embed builds. ([#pspspin-direct](https://github.com/OmniPotentRPC/nwchemc/issues/pspspin-direct))
- Structured pseudopotential `spinRules` now render indexed `pspspin` input lines and promote to typed NWPW RTDB keys in embed mode. ([#pspspin-rules](https://github.com/OmniPotentRPC/nwchemc/issues/pspspin-rules))
- Real-NWChem PSPW pseudopotential force coverage now checks Bohr/eV
  `PotentialResult` conversion for one-shot and session C ABI calls. ([#pspw-pseudopotential-force-units](https://github.com/OmniPotentRPC/nwchemc/issues/pspw-pseudopotential-force-units))
- Added a real NWChem PSPW pseudopotential force smoke for the rgpot `PotentialResult` path. ([#pspw-pseudopotential-forces](https://github.com/OmniPotentRPC/nwchemc/issues/pspw-pseudopotential-forces))
- The real-NWChem PSPW pseudopotential force smoke now covers session result
  carriers as well as the one-shot `PotentialConfig.nwchem + ForceInput` path. ([#pspw-pseudopotential-session-forces](https://github.com/OmniPotentRPC/nwchemc/issues/pspw-pseudopotential-session-forces))
- The real-NWChem PSPW stress smoke now validates one-shot and session stress
  buffers and `PotentialResult.stress` serialization. ([#pspw-stress-session-result](https://github.com/OmniPotentRPC/nwchemc/issues/pspw-stress-session-result))
- The C ABI now exposes total traceless quadrupole calculations through direct, session, and ForceInput entry points backed by NWChem's property multipole routine. ([#quadrupole-abi](https://github.com/OmniPotentRPC/nwchemc/issues/quadrupole-abi))
- Real-NWChem rgpot smoke coverage now checks `ForceInput.energyUnit = "eV"`
  conversion for `PotentialResult` energy, forces, and Hessian against native raw
  buffers. ([#rgpot-forceinput-unit-result](https://github.com/OmniPotentRPC/nwchemc/issues/rgpot-forceinput-unit-result))
- Add a focused rgpot integration guide covering the result-carrier call path, merge gate, and real-NWChem verification probes. ([#rgpot-integration-guide](https://github.com/OmniPotentRPC/nwchemc/issues/rgpot-integration-guide))
- Real-NWChem rgpot smoke coverage now checks optimized coordinate result
  conversion for a Bohr/eV `ForceInput` fixture. ([#rgpot-optimize-unit-result](https://github.com/OmniPotentRPC/nwchemc/issues/rgpot-optimize-unit-result))
- Expose `nwchemc_session_reset_topology()` for reusing a configured session after an explicit atom-count or species-order change. ([#session-reset-abi](https://github.com/OmniPotentRPC/nwchemc/issues/session-reset-abi))
- Single-point and multi-reference embed/CLI matrix coverage includes TCE
  CCD/LCCD/LCCSD/CC2/MBPT(2–4)/CISD/CISDT/CR-CCSD(T)/lambda-CCSD(T)/CCSD[T],
  iterative TCE CCSDT, TCE MRCC/CISD/CCD/MBPT(2) forces (H2 STO-3G on rg.terra).
  Classic CCSD(T) remains CLI-only on this fixture (embed triples GA abort). ([#sp-mr-tce-methods](https://github.com/OmniPotentRPC/nwchemc/issues/sp-mr-tce-methods))
- Expose stress tensor calculations through direct, session, ForceInput, and
  `PotentialResult.stress` C ABI entry points backed by NWChem NWPW stress RTDB
  arrays. ([#stress-abi](https://github.com/OmniPotentRPC/nwchemc/issues/stress-abi))
- Real-NWChem PSPW stress coverage now checks one-shot and session
  `PotentialResult.stress` conversion for a Bohr/eV periodic `ForceInput`. ([#stress-unit-result](https://github.com/OmniPotentRPC/nwchemc/issues/stress-unit-result))
- Expose optimization and harmonic-frequency calculations through Cap'n Proto
  `PotentialResult` result carriers, including size preflight helpers plus
  session and one-shot C ABI entry points. ([#structural-result-abi](https://github.com/OmniPotentRPC/nwchemc/issues/structural-result-abi))
- TCE input now models the `dipole` block keyword as a structured Cap'n Proto field rendered through the TCE stanza. ([#tce-dipole](https://github.com/OmniPotentRPC/nwchemc/issues/tce-dipole))
- TCE coupled-cluster controls now have a structured Cap'n Proto stanza with full-deck rendering and direct typed `tce:*` RTDB writes for embed builds. ([#tce-direct-controls](https://github.com/OmniPotentRPC/nwchemc/issues/tce-direct-controls))
- TCE input now models symbolic atomic freeze modes with a structured `freezeMode` field while keeping numeric freeze counts on the direct `tce:frozen core` and `tce:frozen virtual` RTDB paths. ([#tce-freeze-modes](https://github.com/OmniPotentRPC/nwchemc/issues/tce-freeze-modes))
- TCE input now models the `print` keyword as structured `printLevel` and `printItems` fields rendered through the TCE stanza. ([#tce-print-controls](https://github.com/OmniPotentRPC/nwchemc/issues/tce-print-controls))
- Promote the NWPW makehmass2 control through a typed Cap'n Proto toggle and direct RTDB write. ([#nwpw-makehmass2](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-makehmass2))

### Fixed

- Default omitted structured Brillouin `dos-grid` zone tokens to `structure_default`. ([#brillouin-dos-grid-default](https://github.com/OmniPotentRPC/nwchemc/issues/brillouin-dos-grid-default))
- Default omitted structured Brillouin `monkhorst-pack` grid values to `1`. ([#brillouin-monkhorst-default](https://github.com/OmniPotentRPC/nwchemc/issues/brillouin-monkhorst-default))
- Frequency `PotentialResult` carriers now convert `energy` to
  `ForceInput.energyUnit` while leaving frequencies in cm^-1 and intensities in
  atomic units. ([#frequency-result-energy-unit](https://github.com/OmniPotentRPC/nwchemc/issues/frequency-result-energy-unit))
- Support structured NWPW `bo_fake_mass` directives with omitted values by defaulting to `500`. ([#nwpw-bo-fake-mass-default](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-bo-fake-mass-default))
- Default the second structured NWPW `bo_steps` value to `100` when it is omitted. ([#nwpw-bo-step-default](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-bo-step-default))
- Support structured NWPW `bo_time_step` directives with omitted values by defaulting to `15`. ([#nwpw-bo-time-step-default](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-bo-time-step-default))
- Render structured NWPW `expand_cell` directives when any axis is set and default omitted axes to `1`. ([#nwpw-cell-expand-default](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-cell-expand-default))
- Model NWPW `dos` with omitted scalar arguments by promoting NWChem's default
  DOS alpha value. ([#nwpw-dos-default](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-dos-default))
- Default the second structured NWPW `fractional_orbitals` value to the first when it is omitted. ([#nwpw-fractional-orbitals-default](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-fractional-orbitals-default))
- Model NWPW `mapping` with an omitted integer by promoting NWChem's default
  mapping value. ([#nwpw-mapping-default](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-mapping-default))
- Default the second structured NWPW `mc_steps` value to `100` when it is omitted. ([#nwpw-mc-step-default](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-mc-step-default))
- Support structured NWPW `np_dimensions` directives with omitted values by defaulting dimensions to `-1`. ([#nwpw-np-dimensions-default](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-np-dimensions-default))
- Apply NWChem's `one_electron_guess` defaults when the structured control omits iteration counts. ([#nwpw-one-electron-guess-defaults](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-one-electron-guess-defaults))
- Support structured NWPW `scaling` directives with omitted values by defaulting to `1 1`. ([#nwpw-scaling-default](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-scaling-default))
- Default the second NWPW `smear orbitals` value to the first when rendering structured fractional orbital fields. ([#nwpw-smear-orbitals-default](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-smear-orbitals-default))
- Model NWPW `tolerances` with omitted arguments by promoting NWChem's default
  values for energy, density, and gradient tolerances. ([#nwpw-tolerances-default](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-tolerances-default))
- Default omitted structured NWPW `translate_vector` Y/Z values from the preceding vector component. ([#nwpw-translate-vector-default](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-translate-vector-default))
- Render structured NWPW virtual orbital counts with NWChem's `virtual` keyword. ([#nwpw-virtual-keyword](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-virtual-keyword))
- Default the second structured NWPW `virtual` orbital value to the first when it is omitted. ([#nwpw-virtual-orbitals-default](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-virtual-orbitals-default))
- Rebuild NWChem archives with `-fPIC` (and GA with `CFLAGS_FORGA`/`FFLAGS_FORGA`)
  so meson can produce a shared `libnwchemc.so`. Add `tools/rebuild_nwchem_pic.sh`
  and keep applied_* config integers as `int64` so they match the NWChem
  `-fdefault-integer-8` embed ABI under gfortran 14+. ([#pic-nwchem-shared-link](https://github.com/OmniPotentRPC/nwchemc/issues/pic-nwchem-shared-link))
- Structured pseudopotential `spinRules` can now promote ion lists up to the embed atom cap through typed RTDB integer arrays. ([#pspspin-large-ion-lists](https://github.com/OmniPotentRPC/nwchemc/issues/pspspin-large-ion-lists))
- Hessian and frequency `PotentialResult` carriers now preserve the NWChem task
  energy through the C ABI, including `PotentialConfig` entry points used by
  rgpot integrations. ([#rgpot-result-carrier-energy](https://github.com/OmniPotentRPC/nwchemc/issues/rgpot-result-carrier-energy))


## [1.2.0](https://github.com/OmniPotentRPC/nwchemc/tree/v1.2.0) - 2026-06-26

### Added

- Classic CCSD scalar controls now have a structured Cap'n Proto stanza with full-deck rendering and direct `ccsd:*` RTDB writes in embed builds. ([#ccsd-direct-controls](https://github.com/OmniPotentRPC/nwchemc/issues/ccsd-direct-controls))
- Added one-shot, session, and ForceInput C ABI entry points for total dipole extraction through NWChem RTDB state. ([#dipole-abi](https://github.com/OmniPotentRPC/nwchemc/issues/dipole-abi))
- Added structured NWPW Born-Oppenheimer controls for balance, BO steps, BO time
  step, BO algorithm, BO fake mass, and scaling, with embed builds writing the
  matching NWPW/CPMD RTDB values directly. ([#nwpw-bo-controls](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-bo-controls))
- Added structured NWPW cutoff controls and promoted them to direct RTDB writes for embed calculations. ([#nwpw-cutoff-controls](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-cutoff-controls))
- Added structured NWPW `exchangeCorrelation` support for simple named
  functionals, with embed builds writing the matching PSPW/BAND/CPSD/CPMD RTDB
  keys directly. ([#nwpw-exchange-correlation](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-exchange-correlation))
- Added structured NWPW execution controls for processor dimensions, spin-orbit,
  and parallel I/O, with embed builds writing the matching RTDB values directly. ([#nwpw-execution-controls](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-execution-controls))
- Added structured NWPW fractional orbital and smear controls, with embed builds
  writing the matching `nwpw:fractional_*` RTDB values directly. ([#nwpw-fractional-smearing](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-fractional-smearing))
- Added structured NWPW output and motion filename fields, with embed builds
  writing the matching CPMD and NWPW RTDB string values directly. ([#nwpw-motion-filenames](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-motion-filenames))
- Structured NWPW Nose-Hoover thermostat fields now render full-deck syntax and write the paired CPMD/NWPW RTDB keys directly in embed builds. ([#nwpw-nose-controls](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-nose-controls))
- Added structured NWPW virtual orbital, LCAO, and Ewald grid controls, with
  embed builds writing the matching `nwpw:*` RTDB values directly. ([#nwpw-orbital-grid-controls](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-orbital-grid-controls))
- Added structured NWPW state controls for cell names, wavefunction filenames, fake mass, time step, loop, and tolerance RTDB values. ([#nwpw-state-controls](https://github.com/OmniPotentRPC/nwchemc/issues/nwpw-state-controls))
- Added one-shot ForceInput Hessian and dipole C ABI wrappers that reuse the session validation path without requiring callers to manage a session. ([#oneshot-forceinput-wrappers](https://github.com/OmniPotentRPC/nwchemc/issues/oneshot-forceinput-wrappers))
- Added the `nwchemc_hessian()` C ABI, cmocka-backed C tests, and structured Cap'n Proto pseudopotential stanzas for NWPW library selection. ([#v2-hessian-pseudopotential](https://github.com/OmniPotentRPC/nwchemc/issues/v2-hessian-pseudopotential))

### Fixed

- Embed rendering omits explicit task stanzas and relies on the C ABI entry point to write NWChem task RTDB keys directly. ([#embed-task-direct](https://github.com/OmniPotentRPC/nwchemc/issues/embed-task-direct))
- Persistent sessions now reject parameter replacement after the first accepted topology; create a separate session for a new post-step configuration. ([#session-param-replacement](https://github.com/OmniPotentRPC/nwchemc/issues/session-param-replacement))
- Persistent sessions reject later force/result/Hessian steps whose atom count or species ordering differs from the session's first accepted topology. ([#session-topology](https://github.com/OmniPotentRPC/nwchemc/issues/session-topology))


## [1.1.0](https://github.com/OmniPotentRPC/nwchemc/tree/v1.1.0) - 2026-06-25

### Added

- Expand typed NWChem module stanzas to cover the curated local NWChem
  input-handler set and add a schema coverage test. ([#module-coverage](https://github.com/OmniPotentRPC/nwchemc/issues/module-coverage))


## [1.0.0](https://github.com/OmniPotentRPC/nwchemc/tree/v1.0.0) - 2026-06-25

### Added

- Added CMake support for the stub/parser build and the real NWChem C/Fortran
  shared library path, including CTest coverage for serial and MPI gradient
  calculations. ([#cmake](https://github.com/OmniPotentRPC/nwchemc/issues/cmake))
- Add typed NWChem module stanzas for common NWChem input blocks while preserving
  tokenized directives and raw input fallback paths. ([#module-stanzas](https://github.com/OmniPotentRPC/nwchemc/issues/module-stanzas))

### Fixed

- Clarify and validate that NWChem embed builds require a source/build tree rather
  than an executable runtime prefix. ([#embed-root-validation](https://github.com/OmniPotentRPC/nwchemc/issues/embed-root-validation))


## [0.1.0](https://github.com/OmniPotentRPC/nwchemc/tree/v0.1.0) - 2026-06-25

### Added

- Stable C ABI for passing flat Cap'n Proto `NWChemParams` messages into an
  embedded NWChem gradient calculation.
- Vendored `capnp-c` reader generation for the C runtime path.
- Meson build with stub tests, parser tests, serial NWChem gradient tests,
  structured stanza tests, raw input-block tests, and an MPI test.
