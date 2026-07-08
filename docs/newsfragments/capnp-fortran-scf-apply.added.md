Wire public **capnp-fortran** into the embed apply path: Fortran decodes
serialized `NWChemParams` for config, SCF, DFT, driver, NWPW cutoffs, basis,
Brillouin, pseudopotential, and set stanzas via `nwchemc_embed_apply_params`.
Exploded `set_*_direct` setters remain for CommonMethodSpec overlay only.
C still renders `input_blocks` and expands NWPW/etc. into typed RTDB packs.
