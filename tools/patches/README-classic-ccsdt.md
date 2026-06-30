# Classic CCSD(T) embed patches for linked NWChem

Apply to the NWChem tree used as `nwchem_root` (rg.terra: rgpot/third_party/nwchem)
before linking embed consumers, then rebuild `libmoints.a` / `libccsd.a` objects:

- `moints_aux2_classic_ccsdt.patch` — guard empty `ga_distribution` on `g_exch` in `moints_Ktrf34`
- `moints_trp_classic_ccsdt.patch` — same guard in `moints_trp` / K transform path

Symptom without patches: embed `theory=ccsd(t)` on H2/STO-3G aborts in classic
triples with `cannot locate region: X oper [1:0 ,1:1]` while CLI succeeds.
Root cause: under embed GA ownership for the X oper array can report an empty
row range on access; patches force `rlo/rhi` to `1 … nbf*nvir` when empty.

Embed also uses `ga_initialize_ltd` + CLI-scale MA minima + `ma_set_numalign(6)`
in `nwchem_embed_legacy.F` (matches `nwchem.F`).
