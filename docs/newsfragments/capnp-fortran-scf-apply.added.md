Wire public **capnp-fortran** into the embed apply path: Fortran decodes
serialized `NWChemParams` for the SCF family (maxiter/thresh/tol2e) so the
exploded `nwchemc_embed_set_scf_direct` setter is no longer used on that
path. C ABI sizing/results still use c-capnproto.
