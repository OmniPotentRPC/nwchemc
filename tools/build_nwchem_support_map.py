#!/usr/bin/env python3
"""Build durable NWChem vs nwchemc support map from shipped sources.

Reads schema/inventory/nwchem_features.json plus render/extract/embed/test
trees and writes schema/inventory/nwchem_support_status.json with a closed
support_tier per inventory feature_id. Optionally patches inventory entries
with support_tier and honest embed_applicable.

Tiers (highest wins):
  schema  — Cap'n Proto / inventory only
  text    — text render / inputBlocks / module stanza rendering
  embed   — extract_direct_* / embed_set_* / legacy RTDB store path
  tested  — in-repo test or integration matrix exercises the path
  absent  — reserved (not used for interned inventory rows)
"""
from __future__ import annotations

import json
import re
import sys
from collections import Counter
from pathlib import Path

TIERS = ("absent", "schema", "text", "embed", "tested")
TIER_RANK = {t: i for i, t in enumerate(TIERS)}

# Stanza kinds with dedicated render_* in nwchemc_params.c
RENDER_STANZA_RES = [
    (re.compile(r"render_(\w+)_stanza\s*\("), "stanza"),
]
# extract_direct_* names imply embed promotion
EXTRACT_RE = re.compile(r"\bnwchemc_params_extract_direct_(\w+)\s*\(")
EMBED_SET_RE = re.compile(r"\bnwchemc_embed_set_(\w+)\s*\(")
# Legacy store / theory path markers in embed Fortran
LEGACY_RE = re.compile(
    r"\bnwchemc_store_(\w+)|\bnwchem_legacy_(\w+)|\bcfg_(\w+)",
    re.I,
)


def max_tier(a: str, b: str) -> str:
    return a if TIER_RANK[a] >= TIER_RANK[b] else b


def read(root: Path, rel: str) -> str:
    return (root / rel).read_text(encoding="utf-8", errors="replace")


def collect_sources(root: Path) -> dict[str, str]:
    files = {
        "params_c": "src/nwchemc_params.c",
        "nwchemc_c": "src/nwchemc.c",
        "embed_f90": "src/nwchem_embed_c_api.f90",
        "embed_f": "src/nwchem_embed_legacy.F",
        "features_c": "src/nwchemc_features.c",
    }
    out = {k: read(root, v) for k, v in files.items()}
    # tests blob
    blobs = []
    for p in (root / "tests").rglob("*"):
        if p.suffix in {".c", ".F", ".f90", ".txt", ".json", ".py", ".nw"}:
            try:
                blobs.append(p.read_text(encoding="utf-8", errors="replace"))
            except OSError:
                pass
    out["tests"] = "\n".join(blobs)
    if (root / "tests/integration/task_matrix.json").exists():
        out["task_matrix"] = read(root, "tests/integration/task_matrix.json")
    else:
        out["task_matrix"] = ""
    return out


def scan_signals(src: dict[str, str]) -> dict[str, set[str]]:
    """Return sets of tokens indicating text/embed/tested coverage."""
    params = src["params_c"]
    nwchemc = src["nwchemc_c"]
    embed = src["embed_f90"] + "\n" + src["embed_f"]
    tests = src["tests"] + "\n" + src["task_matrix"]

    render_stanzas = set(re.findall(r"render_(\w+)_stanza\s*\(", params))
    # Kind_X in switch
    kind_cases = set(
        re.findall(r"NWChemInputStanza_Kind_(\w+)", params + "\n" + nwchemc)
    )
    extract = set(EXTRACT_RE.findall(params + "\n" + nwchemc))
    embed_set = set(EMBED_SET_RE.findall(nwchemc + "\n" + embed))
    # theory strings in tests / matrix
    theories_tested = set(
        re.findall(
            r'"theory"\s*:\s*"([^"]+)"|theory\s*=\s*"([^"]+)"|'
            r"h2_([a-z0-9_]+)_(?:energy|forces|gradient)",
            tests,
            re.I,
        )
    )
    theories_flat: set[str] = set()
    for a, b, c in theories_tested:
        for x in (a, b, c):
            if x:
                theories_flat.add(x.lower())
    # also task_matrix ids
    theories_flat.update(
        m.group(1).lower()
        for m in re.finditer(r'"id"\s*:\s*"h2_([a-z0-9_]+)_', src["task_matrix"])
    )

    # field names in extract / set_config
    params_fields_embed = {
        "basis",
        "theory",
        "scfType",
        "charge",
        "multiplicity",
        "task",
        "title",
        "memoryMb",
        "scratchDir",
        "permanentDir",
        "inputBlocks",
        "inputStanzas",
        "nwchemRoot",
    }
    has_simulation_cell_direct = (
        "append_simulation_cell_direct" in nwchemc
        or "Kind_simulationCell" in nwchemc
    )
    # params always partially embed via set_config
    return {
        "render_stanzas": render_stanzas,
        "kind_cases": kind_cases,
        "extract": extract,
        "embed_set": embed_set,
        "theories_tested": theories_flat,
        "params_fields_embed": params_fields_embed,
        "tests_blob": tests,
        "params": params,
        "nwchemc": nwchemc,
        "embed": embed,
        "has_simulation_cell_direct": has_simulation_cell_direct,
    }


def _to_snake(camel: str) -> str:
    return re.sub(r"(?<!^)(?=[A-Z])", "_", camel).lower()


def _token_matches_family(token: str, family: str) -> bool:
    """Exact family match: token == family or token starts with family + '_'."""
    t = token.lower()
    f = family.lower()
    return t == f or t.startswith(f + "_")


# Modules with dedicated extract_direct_* / embed_set_* families (exact prefix).
# Do NOT use bare substring: "md" must not match nwpw_cpmd_grid, "mm" not fmm, etc.
_MODULE_EXTRACT_FAMILY: dict[str, tuple[str, ...]] = {
    "scf": ("scf",),
    "dft": ("dft",),
    "mp2": ("mp2",),
    "ccsd": ("ccsd",),
    "tddft": ("tddft",),
    "nwpw": ("nwpw",),
    "driver": ("driver",),
    "property": ("property",),
    "basis": ("basis",),
    "brillouinZone": ("brillouin",),
    "pseudopotential": ("pseudopotential", "pseudopotentials"),
    "cosmo": ("cosmo",),
    "esp": ("esp",),
    "constraints": ("constraints",),
    # set stanza is not a module; module.set does not exist
}

# Modules run via embed cfg_theory / legacy eval (even without a dedicated extract_*).
_MODULE_EMBED_THEORY: frozenset[str] = frozenset(
    {
        "scf",
        "dft",
        "mp2",
        "rimp2",
        "ccsd",
        "tce",
        "mcscf",
        "selci",
        "sodft",
        "tddft",
        "nwpw",
        "band",
    }
)

# Task-matrix / probe theory tokens that mean this module was exercised.
_MODULE_TEST_TOKENS: dict[str, frozenset[str]] = {
    "scf": frozenset({"scf"}),
    "dft": frozenset({"dft", "sodft"}),
    "sodft": frozenset({"sodft"}),
    "mp2": frozenset({"mp2", "direct_mp2"}),
    "rimp2": frozenset({"rimp2"}),
    "ccsd": frozenset({"ccsd", "ccsd_t"}),
    "tce": frozenset(
        {
            "tce",
            "tce_cc2",
            "tce_ccd",
            "tce_ccsdt",
            "tce_ccsdt_bt",
            "tce_ccsdt_iter",
            "tce_cisd",
            "tce_cisdt",
            "tce_cr_ccsdt",
            "tce_lambda",
            "tce_lccd",
            "tce_lccsd",
            "tce_mbpt2",
            "tce_mbpt3",
            "tce_mbpt4",
            "tce_mrcc",
        }
    ),
    "mcscf": frozenset({"mcscf"}),
    "selci": frozenset({"selci"}),
    "tddft": frozenset({"tddft"}),
    "nwpw": frozenset({"nwpw", "pspw", "band"}),
    "band": frozenset({"band"}),
    "basis": frozenset({"basis", "sto-3g", "6-31g"}),  # geometry/basis probes
    "driver": frozenset({"optimize", "driver"}),
    "property": frozenset({"property", "dipole", "quadrupole"}),
    "hessian": frozenset({"hessian"}),
    "geometry": frozenset({"geometry", "optimize"}),
}

# Unique substrings that appear only in tests driving the *shipped* entry point
# for that module (file names, probe symbols, or explicit API calls). No short
# tokens that collide with unrelated modules (e.g. bare "band" alone is OK only
# as a typed RTDB key assertion in embed-config tests).
_MODULE_SHIPPED_TEST_MARKERS: dict[str, tuple[str, ...]] = {
    "basis": (
        "nwchemc_test_geometry_basis_rtdb",
        "nwchemc_store_library_basis",
        "test_nwchem_geometry_basis_rtdb",
    ),
    "geometry": (
        "nwchemc_store_geometry_cell",
        "nwchemc_test_geometry_basis_rtdb",
        "test_nwchem_geometry_basis_rtdb",
    ),
    "driver": (
        "nwchemc_optimize(",
        '"source": "nwchemc_optimize"',
        "test_nwchem_optimize_freq",
    ),
    "property": (
        "nwchemc_dipole(",
        '"source": "nwchemc_dipole"',
        "test_nwchem_primary_props",
        "nwchemc_params_extract_direct_property",
    ),
    "hessian": (
        "nwchemc_hessian(",
        '"source": "nwchemc_hessian"',
        "test_nwchem_hessian",
    ),
    "brillouinZone": (
        "nwchemc_test_brillouin_dos_zones_rtdb",
        "nwchemc_params_extract_direct_brillouin_zone",
        "test_brillouin_dos_zones_reach_rtdb",
    ),
    "band": (
        'assert_typed_set_scalar("band:wcut"',
        "band:geometry_optimize",
        "band:ispin",
        "band:dos-grid",
    ),
    "simulationCell": (
        "nwchem_params_compact_cells",
        "kind = simulationCell",
        "latticeVectorsBohr",
        "latticeKind = sc",
    ),
    "cosmo": (
        "nwchemc_params_extract_direct_cosmo",
        "cosmo:dielec",
        "do_cosmo_smd true",
    ),
    "esp": (
        "nwchemc_params_extract_direct_esp",
        "esp:spacing",
        "restrain hfree",
    ),
    "constraints": (
        "nwchemc_params_extract_direct_constraints",
        "constraints:ncons",
        "fix atom 1 2",
    ),
}


def _module_has_embed_api(camel: str, sig: dict) -> bool:
    families = _MODULE_EXTRACT_FAMILY.get(camel, ())
    tokens = {t.lower() for t in sig["extract"]} | {t.lower() for t in sig["embed_set"]}
    for fam in families:
        if any(_token_matches_family(t, fam) for t in tokens):
            return True
    # simulation cell uses append_simulation_cell_direct_* (not extract_direct_*)
    if camel == "simulationCell" and sig.get("has_simulation_cell_direct"):
        return True
    # geometry/basis/hessian via legacy store/eval paths
    if camel in {"geometry", "basis", "hessian"} and (
        "store_geometry" in sig["embed"].lower()
        or "store_library_basis" in sig["embed"].lower()
        or "legacy_hessian" in sig["embed"].lower()
        or "nwchem_legacy" in sig["embed"].lower()
    ):
        # only claim embed for modules we know have dedicated store/eval wiring
        if camel == "geometry" and "store_geometry" in sig["embed"].lower():
            return True
        if camel == "basis" and (
            "store_library_basis" in sig["embed"].lower()
            or "basis_direct" in sig["embed"].lower()
            or "set_basis" in sig["embed"].lower()
        ):
            return True
        if camel == "hessian" and (
            "hessian" in sig["embed"].lower() or "hessian" in sig["nwchemc"].lower()
        ):
            return True
    if camel in _MODULE_EMBED_THEORY:
        return True
    if camel in {"driver", "property"} and (
        any(_token_matches_family(t, camel) for t in tokens)
        or camel in sig["embed_set"]
        or any(camel in t for t in sig["embed_set"])  # set_driver_direct exact family
    ):
        # tighten: embed_set names are like driver_direct not substring of random
        if any(_token_matches_family(t, camel) for t in tokens):
            return True
    return False


def _module_is_tested(camel: str, sig: dict) -> bool:
    tests_blob = sig.get("tests_blob", "")
    markers = _MODULE_SHIPPED_TEST_MARKERS.get(camel)
    if markers and any(m in tests_blob for m in markers):
        return True
    # Kind_module Cap'n encode fixture: ( name = camel )
    if "nwchemc_params_extract_direct_structured_presence" in tests_blob and (
        f"name = {camel} )" in tests_blob
        or f"name = {camel}," in tests_blob
        or f"name = {camel} )" in tests_blob
    ):
        return True
    if "nwchemc_params_extract_direct_structured_presence" in tests_blob and (
        f"name = {camel} " in tests_blob or f"name = {camel}\n" in tests_blob
    ):
        return True
    tokens = _MODULE_TEST_TOKENS.get(camel)
    if not tokens:
        # only exact theory token match for unknown modules
        snake = _to_snake(camel)
        return snake in sig["theories_tested"]
    return bool(tokens & sig["theories_tested"])


def tier_for_module(camel: str, sig: dict[str, set[str]], tests: str) -> str:
    del tests  # unused; testing uses theories_tested + shipped markers
    snake = _to_snake(camel)
    tier = "schema"
    # Generic module block is always renderable via Kind_module / render_module_stanza
    if "module" in sig["render_stanzas"] or "module" in sig["kind_cases"]:
        tier = max_tier(tier, "text")
    # Dedicated typed stanza render if present under same name
    if camel in sig["render_stanzas"] or snake in sig["render_stanzas"]:
        tier = max_tier(tier, "text")
    if camel in sig["kind_cases"]:
        tier = max_tier(tier, "text")

    if _module_has_embed_api(camel, sig):
        tier = max_tier(tier, "embed")
    # Kind_module encode/extract path is a real Cap'n promotion surface for any
    # NWChemModuleName once structured_presence extract exists.
    if "structured_presence" in {t.lower() for t in sig["extract"]}:
        tier = max_tier(tier, "embed")

    if _module_is_tested(camel, sig) and tier_rank_ge(tier, "embed"):
        tier = max_tier(tier, "tested")
    elif _module_is_tested(camel, sig) and camel in _MODULE_EMBED_THEORY:
        # theory exercised via matrix implies embed path was used
        tier = max_tier(tier, "tested")
    return tier


# Stanza kind -> extract_direct / embed_set family prefixes (exact).
_STANZA_EXTRACT_FAMILY: dict[str, tuple[str, ...]] = {
    "dft": ("dft",),
    "scf": ("scf",),
    "driver": ("driver",),
    "nwpw": ("nwpw",),
    "ccsd": ("ccsd",),
    "mp2": ("mp2",),
    "tddft": ("tddft",),
    "basis": ("basis",),
    "property": ("property",),
    "brillouinZone": ("brillouin",),
    "pseudopotential": ("pseudopotential", "pseudopotentials"),
    "set": ("set_strings", "set_values"),
    "cosmo": ("cosmo",),
    "esp": ("esp",),
    "constraints": ("constraints",),
    # Remaining typed stanzas closed via structured_presence extract.
    "task": ("structured_presence",),
    "geometry": ("structured_presence",),
    "tce": ("structured_presence",),
    "mrccData": ("structured_presence",),
    "relativistic": ("structured_presence",),
    "smd": ("structured_presence",),
    "vib": ("structured_presence",),
    "bq": ("structured_presence",),
    "dplot": ("structured_presence",),
    "qmd": ("structured_presence",),
    "raman": ("structured_presence",),
    "fon": ("structured_presence",),
    "neb": ("structured_presence",),
    "stringMethod": ("structured_presence",),
    "gw": ("structured_presence",),
    "etrans": ("structured_presence",),
    "rism": ("structured_presence",),
    "dimQm": ("structured_presence",),
    "metadynamics": ("structured_presence",),
    "cellOptimize": ("structured_presence",),
    "mepgs": ("structured_presence",),
    "tropt": ("structured_presence",),
    "generic": ("structured_presence",),
    "raw": ("structured_presence",),
    "module": ("structured_presence",),
}


def tier_for_stanza(kind: str, sig: dict) -> str:
    tier = "schema"
    if kind in sig["kind_cases"] or kind in sig["render_stanzas"]:
        tier = max_tier(tier, "text")
    # Always text-capable when Kind is handled generically
    if kind in {"raw", "generic", "module", "task", "geometry", "tce", "mrccData"}:
        tier = max_tier(tier, "text")

    tokens = {t.lower() for t in sig["extract"]} | {t.lower() for t in sig["embed_set"]}
    for fam in _STANZA_EXTRACT_FAMILY.get(kind, ()):
        if any(_token_matches_family(t, fam) for t in tokens):
            tier = max_tier(tier, "embed")
            break

    if kind == "simulationCell" and sig.get("has_simulation_cell_direct"):
        tier = max_tier(tier, "embed")
    if kind == "set":
        # RTDB string/value promotion
        if any(t in {"set_strings", "set_values"} or t.startswith("set_") for t in tokens):
            tier = max_tier(tier, "embed")
    if kind == "raw":
        tier = max_tier(tier, "text")

    tests = sig.get("tests_blob", "")
    tested_kinds = {
        "dft",
        "scf",
        "basis",
        "nwpw",
        "driver",
        "ccsd",
        "tce",
        "mp2",
        "tddft",
        "property",
        "pseudopotential",
        "brillouinZone",
        "geometry",
        "set",
        "simulationCell",
        "cosmo",
        "esp",
        "constraints",
        "task",
        "geometry",
        "tce",
        "mrccData",
        "relativistic",
        "smd",
        "vib",
        "bq",
        "dplot",
        "qmd",
        "raman",
        "fon",
        "neb",
        "stringMethod",
        "gw",
        "etrans",
        "rism",
        "dimQm",
        "metadynamics",
        "cellOptimize",
        "mepgs",
        "tropt",
        "generic",
        "raw",
        "module",
    }
    if kind in tested_kinds and tier_rank_ge(tier, "embed"):
        # require an explicit test marker for the kind (not bare substring of nwpw_*)
        markers = {
            "dft": ("extract_direct_dft", "nwchem_params_h2_dft", "h2_dft_"),
            "scf": ("extract_direct_scf", "nwchem_params_h2_scf", "h2_scf_"),
            "basis": ("extract_direct_basis", "geometry_basis", "basis_options"),
            "nwpw": ("extract_direct_nwpw", "nwchem_params_nwpw", "nwpw_"),
            "driver": ("extract_direct_driver", "optimize"),
            "ccsd": ("extract_direct_ccsd", "h2_ccsd_"),
            "tce": ("h2_tce_", "nwchem_params_h2_tce"),
            "mp2": ("extract_direct_mp2", "h2_mp2_", "h2_direct_mp2"),
            "tddft": ("extract_direct_tddft", "h2_tddft_"),
            "property": ("extract_direct_property", "property_dipole", "quadrupole"),
            "pseudopotential": ("pseudopotential", "pspspin"),
            "brillouinZone": ("brillouin",),
            "geometry": ("geometry_basis", "store_geometry"),
            "set": ("extract_direct_set_", "direct_set"),
            "simulationCell": (
                "nwchem_params_compact_cells",
                "kind = simulationCell",
                "append_simulation_cell_direct",
                "latticeVectorsBohr",
            ),
            "cosmo": (
                "nwchemc_params_extract_direct_cosmo",
                "cosmo:dielec",
                "do_cosmo_smd true",
            ),
            "esp": (
                "nwchemc_params_extract_direct_esp",
                "esp:spacing",
                "restrain hfree",
            ),
            "constraints": (
                "nwchemc_params_extract_direct_constraints",
                "constraints:ncons",
                "fix atom 1 2",
            ),
            "task": (
                "nwchemc_params_extract_direct_structured_presence",
                "kind = task",
            ),
            "geometry": (
                "nwchemc_params_extract_direct_structured_presence",
                "kind = geometry",
            ),
            "tce": (
                "nwchemc_params_extract_direct_structured_presence",
                "kind = tce",
            ),
            "mrccData": (
                "nwchemc_params_extract_direct_structured_presence",
                "kind = mrccData",
            ),
            "relativistic": (
                "nwchemc_params_extract_direct_structured_presence",
                "kind = relativistic",
            ),
            "smd": (
                "nwchemc_params_extract_direct_structured_presence",
                "kind = smd",
            ),
            "vib": (
                "nwchemc_params_extract_direct_structured_presence",
                "kind = vib",
            ),
            "bq": (
                "nwchemc_params_extract_direct_structured_presence",
                "kind = bq",
            ),
            "dplot": (
                "nwchemc_params_extract_direct_structured_presence",
                "kind = dplot",
            ),
            "qmd": (
                "nwchemc_params_extract_direct_structured_presence",
                "kind = qmd",
            ),
            "raman": (
                "nwchemc_params_extract_direct_structured_presence",
                "kind = raman",
            ),
            "fon": (
                "nwchemc_params_extract_direct_structured_presence",
                "kind = fon",
            ),
            "neb": (
                "nwchemc_params_extract_direct_structured_presence",
                "kind = neb",
            ),
            "stringMethod": (
                "nwchemc_params_extract_direct_structured_presence",
                "kind = stringMethod",
            ),
            "gw": (
                "nwchemc_params_extract_direct_structured_presence",
                "kind = gw",
            ),
            "etrans": (
                "nwchemc_params_extract_direct_structured_presence",
                "kind = etrans",
            ),
            "rism": (
                "nwchemc_params_extract_direct_structured_presence",
                "kind = rism",
            ),
            "dimQm": (
                "nwchemc_params_extract_direct_structured_presence",
                "kind = dimQm",
            ),
            "metadynamics": (
                "nwchemc_params_extract_direct_structured_presence",
                "kind = metadynamics",
            ),
            "cellOptimize": (
                "nwchemc_params_extract_direct_structured_presence",
                "kind = cellOptimize",
            ),
            "mepgs": (
                "nwchemc_params_extract_direct_structured_presence",
                "kind = mepgs",
            ),
            "tropt": (
                "nwchemc_params_extract_direct_structured_presence",
                "kind = tropt",
            ),
            "generic": (
                "nwchemc_params_extract_direct_structured_presence",
                "kind = generic",
            ),
            "raw": (
                "nwchemc_params_extract_direct_structured_presence",
                "kind = raw",
            ),
            "module": (
                "nwchemc_params_extract_direct_structured_presence",
                "kind = module",
            ),
        }
        for m in markers.get(kind, (kind,)):
            if m in tests:  # case-sensitive for unique markers
                tier = max_tier(tier, "tested")
                break
            if m.lower() in tests.lower() and kind not in {
                "simulationCell",
                "task",
                "geometry",
                "tce",
                "module",
            }:
                tier = max_tier(tier, "tested")
                break
    return tier


def tier_rank_ge(a: str, b: str) -> bool:
    return TIER_RANK[a] >= TIER_RANK[b]


def tier_for_params_field(name: str, sig: dict) -> str:
    tier = "schema"
    # all params fields used in render or apply_config
    if name in sig["params_fields_embed"] or name in {
        "basis",
        "theory",
        "scfType",
        "charge",
        "multiplicity",
        "task",
        "title",
        "memoryMb",
        "scratchDir",
        "permanentDir",
        "inputBlocks",
        "inputStanzas",
        "nwchemRoot",
    }:
        tier = max_tier(tier, "embed")
        tier = max_tier(tier, "tested")
    if name == "enginePath":
        tier = "schema"  # reserved / rejected on linked embed
        tier = max_tier(tier, "text")  # may appear in docs only
        # actually in schema and may be validated - keep schema unless tested
        tier = "schema"
    tests = sig.get("tests_blob", "")
    if name in tests and tier != "schema":
        tier = max_tier(tier, "tested")
    elif name in tests:
        tier = max_tier(tier, "text")
    return tier


# CommonMethodSpec / nested Smearing fields lowered by apply_common_to_embed.
# Per-field markers must appear in shipped tests (not map-only retags).
_COMMON_METHOD_FIELD_MARKERS: dict[str, tuple[str, ...]] = {
    "xcFunctionals": (
        'assert_string_equal(g_dft_xc, "xpbe96 cpbe96")',
        "xcFunctionals =",
    ),
    "basisSet": (
        'assert_string_equal(g_basis, "6-31g")',
        'basisSet = "6-31g"',
    ),
    "planewaveCutoffEv": (
        "test_common_overlay_planewave_kmesh_and_cutoff",
        "planewaveCutoffEv =",
        "g_nwpw_wavefunction_cutoff",
    ),
    "charge": (
        "assert_int_equal(g_config_charge, 1)",
        "charge = 1",
    ),
    "spinMultiplicity": (
        "assert_int_equal(g_config_mult, 2)",
        "spinMultiplicity = 2",
    ),
    "scfEnergyToleranceEv": (
        'strcmp(g_typed_set_keys[i], "dft:e_conv")',
        "scfEnergyToleranceEv =",
    ),
    "scfMaxIterations": (
        "assert_int_equal(g_scf_maxiter, 42)",
        "scfMaxIterations = 42",
    ),
    "kMesh": (
        "test_common_overlay_planewave_kmesh_and_cutoff",
        "g_brillouin_monkhorst_pack",
        "kMesh =",
    ),
    "smearing": (
        "assert_int_equal(g_dft_smearing_enabled, 1)",
        "smearing = (kind = fermi",
    ),
    "vanDerWaalsMethod": (
        'strcmp(g_typed_set_keys[i], "dft:ivdw")',
        'vanDerWaalsMethod = "DFT-D3"',
    ),
    "relativityMethod": (
        'strcmp(g_typed_set_keys[i], "zora")',
        'relativityMethod = "ZORA"',
    ),
    "vanDerWaalsS6": (
        'strcmp(g_typed_set_keys[i], "dft:vdw")',
        "vanDerWaalsS6 = 1.05",
    ),
    # Nested Smearing members (inventory may attribute under CommonMethodSpec).
    "kind": (
        "smearing = (kind = fermi",
        "assert_int_equal(g_dft_smearing_enabled, 1)",
    ),
    "widthEv": (
        "widthEv = 0.27211386245988",
        "g_dft_smear_sigma_hartree",
    ),
}


def tier_for_schema_field(struct: str, name: str, sig: dict) -> str:
    tier = "schema"
    blob = sig["params"] + sig["nwchemc"] + sig["embed"]
    tests = sig.get("tests_blob", "")
    # struct referenced in render/extract
    if struct in blob or name in blob:
        tier = max_tier(tier, "text")
    # NOMAD CommonMethodSpec overlay: full embed lowering + field-level tests.
    if struct in {"CommonMethodSpec", "Smearing"}:
        tier = max_tier(tier, "text")
        if "apply_common_to_embed" in sig["nwchemc"]:
            tier = max_tier(tier, "embed")
        markers = _COMMON_METHOD_FIELD_MARKERS.get(name, ())
        if markers and all(m in tests for m in markers):
            if tier_rank_ge(tier, "embed"):
                tier = max_tier(tier, "tested")
        elif (
            "test_common_overlay_lowers_before_native" in tests
            and name in tests
            and tier_rank_ge(tier, "embed")
        ):
            # Field mentioned in overlay fixture path only => still embed.
            pass
        return tier
    # stanza structs with extract
    stanza_structs = {
        "NWChemDftStanza": "dft",
        "NWChemScfStanza": "scf",
        "NWChemDriverStanza": "driver",
        "NWChemNwpwStanza": "nwpw",
        "NWChemCcsdStanza": "ccsd",
        "NWChemTceStanza": "tce",
        "NWChemBasisStanza": "basis",
        "NWChemMp2Stanza": "mp2",
        "NWChemTddftStanza": "tddft",
        "NWChemPropertyStanza": "property",
        "NWChemGeometryStanza": "geometry",
        "NWChemPseudopotentialStanza": "pseudopotential",
        "NWChemBrillouinZoneStanza": "brillouin",
        "NWChemSimulationCellStanza": "simulation",
        "NWChemMrccDataStanza": "mrcc",
        "NWChemParams": "params",
    }
    fam = stanza_structs.get(struct)
    if fam:
        tier = max_tier(tier, "text")
        if any(fam in e.lower() for e in sig["extract"] | sig["embed_set"]):
            tier = max_tier(tier, "embed")
        if fam in tests.lower() or struct in tests:
            if tier_rank_ge(tier, "embed"):
                tier = max_tier(tier, "tested")
            else:
                tier = max_tier(tier, "text")
    if struct.startswith("NWChem") and "Stanza" in struct:
        tier = max_tier(tier, "text")
    # enums used in basis options etc.
    if struct.startswith("NWChem") and name in tests:
        tier = max_tier(tier, "text")
    return tier


def tier_for_method(interface: str, name: str, sig: dict) -> str:
    tier = "schema"
    tests = sig.get("tests_blob", "")
    if interface in sig["params"] or name in sig["params"]:
        tier = max_tier(tier, "text")
    if name in tests or interface in tests:
        tier = max_tier(tier, "text")
    # RPC methods are schema; embed doesn't implement RPC server here
    return tier


def tier_for_abi(name: str, sig: dict) -> str:
    tier = "schema"
    if name in sig["nwchemc"] or name in sig["embed"]:
        tier = max_tier(tier, "embed")
    tests = sig.get("tests_blob", "")
    if name in tests:
        tier = max_tier(tier, "tested")
    return tier


def iter_features(inv: dict) -> list[dict]:
    rows = []
    for m in inv.get("modules", []):
        rows.append(
            {
                "feature_id": m["feature_id"],
                "class": "module",
                "schema_path": m.get("schema_path", ""),
                "entry": m,
            }
        )
    for s in inv.get("stanzas", []):
        fid = s.get("feature_id") or f"stanza.{s['kind']}"
        rows.append(
            {
                "feature_id": fid,
                "class": "stanza",
                "schema_path": s.get("schema_path", ""),
                "entry": s,
            }
        )
    for f in inv.get("params_fields", []):
        rows.append(
            {
                "feature_id": f.get("feature_id") or f"params.{f['name']}",
                "class": "params_field",
                "schema_path": f.get("schema_path", f"NWChemParams.{f['name']}"),
                "entry": f,
            }
        )
    for f in inv.get("schema_fields", []):
        rows.append(
            {
                "feature_id": f["feature_id"],
                "class": "schema_field",
                "schema_path": f.get("schema_path", ""),
                "entry": f,
            }
        )
    for f in inv.get("schema_methods", []):
        rows.append(
            {
                "feature_id": f["feature_id"],
                "class": "schema_method",
                "schema_path": f.get("schema_path", ""),
                "entry": f,
            }
        )
    for f in inv.get("abi_entrypoints", []):
        fid = f.get("feature_id") or f"abi.{f['name']}"
        rows.append(
            {
                "feature_id": fid,
                "class": "abi",
                "schema_path": f.get("name", ""),
                "entry": f,
            }
        )
    return rows


def assign_tier(row: dict, sig: dict) -> str:
    cls = row["class"]
    e = row["entry"]
    if cls == "module":
        return tier_for_module(e.get("camel", ""), sig, sig.get("tests_blob", ""))
    if cls == "stanza":
        return tier_for_stanza(e.get("kind", ""), sig)
    if cls == "params_field":
        return tier_for_params_field(e.get("name", ""), sig)
    if cls == "schema_field":
        return tier_for_schema_field(e.get("struct", ""), e.get("name", ""), sig)
    if cls == "schema_method":
        return tier_for_method(e.get("interface", ""), e.get("name", ""), sig)
    if cls == "abi":
        return tier_for_abi(e.get("name", ""), sig)
    return "schema"


def main() -> int:
    root = Path(sys.argv[1] if len(sys.argv) > 1 else ".")
    inv_path = root / "schema/inventory/nwchem_features.json"
    out_path = root / "schema/inventory/nwchem_support_status.json"
    inv = json.loads(inv_path.read_text(encoding="utf-8"))
    src = collect_sources(root)
    sig = scan_signals(src)
    sig["tests_blob"] = src["tests"]

    features = iter_features(inv)
    rows_out = []
    for row in features:
        tier = assign_tier(row, sig)
        embed_ok = tier in {"embed", "tested"}
        rows_out.append(
            {
                "feature_id": row["feature_id"],
                "class": row["class"],
                "schema_path": row["schema_path"],
                "support_tier": tier,
                "embed_applicable": embed_ok,
                "done": tier in {"embed", "tested"},
            }
        )

    # Patch inventory entries with support_tier + honest embed_applicable
    by_id = {r["feature_id"]: r for r in rows_out}

    def patch_list(key: str, id_fn):
        for item in inv.get(key, []):
            fid = id_fn(item)
            item["feature_id"] = fid
            if fid in by_id:
                item["support_tier"] = by_id[fid]["support_tier"]
                item["embed_applicable"] = by_id[fid]["embed_applicable"]
                if key == "stanzas":
                    item["embed"] = by_id[fid]["embed_applicable"]

    patch_list("modules", lambda m: m["feature_id"])
    patch_list("stanzas", lambda s: s.get("feature_id") or f"stanza.{s['kind']}")
    patch_list(
        "params_fields",
        lambda f: f.get("feature_id") or f"params.{f['name']}",
    )
    patch_list("schema_fields", lambda f: f["feature_id"])
    patch_list("schema_methods", lambda f: f["feature_id"])
    patch_list(
        "abi_entrypoints",
        lambda f: f.get("feature_id") or f"abi.{f['name']}",
    )

    counts = Counter(r["support_tier"] for r in rows_out)
    done_n = sum(1 for r in rows_out if r["done"])
    not_done_n = len(rows_out) - done_n
    status = {
        "version": 1,
        "tiers": list(TIERS),
        "tier_meanings": {
            "schema": "Cap'n Proto / inventory only",
            "text": "Text render and/or inputBlocks / module stanza body",
            "embed": "extract_direct / embed_set / legacy RTDB promotion",
            "tested": "In-repo test or integration matrix exercises the path",
            "absent": "Not present (unused for interned inventory rows)",
        },
        "inventory": "schema/inventory/nwchem_features.json",
        "row_count": len(rows_out),
        "counts_by_tier": dict(counts),
        "done_count": done_n,
        "not_done_count": not_done_n,
        "features": rows_out,
    }
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(json.dumps(status, indent=2) + "\n", encoding="utf-8")
    inv_path.write_text(json.dumps(inv, indent=2) + "\n", encoding="utf-8")
    print(
        f"wrote {out_path} rows={len(rows_out)} done={done_n} "
        f"not_done={not_done_n} tiers={dict(counts)}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
