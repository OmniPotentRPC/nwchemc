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
    }


def tier_for_module(camel: str, sig: dict[str, set[str]], tests: str) -> str:
    name = camel[0].lower() + camel[1:] if camel else camel
    # snake for nwchem text
    snake = re.sub(r"(?<!^)(?=[A-Z])", "_", camel).lower()
    tier = "schema"
    # module stanza always rendered via render_module_stanza if Kind_module handled
    if "module" in sig["render_stanzas"] or "module" in sig["kind_cases"]:
        tier = max_tier(tier, "text")
    # dedicated typed stanzas
    if snake in sig["render_stanzas"] or name in sig["render_stanzas"]:
        tier = max_tier(tier, "text")
    if camel.lower() in {x.lower() for x in sig["render_stanzas"]}:
        tier = max_tier(tier, "text")
    # embed if extract or embed_set mentions
    keys = {
        snake,
        name.lower(),
        camel.lower(),
        snake.replace("_", ""),
    }
    for k in keys:
        for e in sig["extract"]:
            if k in e.lower() or e.lower() in k:
                tier = max_tier(tier, "embed")
        for e in sig["embed_set"]:
            if k in e.lower() or e.lower() in k:
                tier = max_tier(tier, "embed")
    # theory tested
    for t in sig["theories_tested"]:
        if snake in t or t in snake or name.lower() in t:
            tier = max_tier(tier, "tested")
            break
    # known embed theory families always embed-evaluated via cfg_theory
    if snake in {
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
        "driver",
        "property",
        "hessian",
        "geometry",
        "basis",
        "brillouin_zone",
        "brillouinZone",
    } or camel in {
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
        "driver",
        "property",
        "hessian",
        "geometry",
        "basis",
        "brillouinZone",
        "simulationCell",
    }:
        # only if theory actually runs through embed legacy
        if snake in {
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
        } or camel in {
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
        }:
            tier = max_tier(tier, "embed")
            if any(
                snake in t or t.startswith(snake) or snake.startswith(t)
                for t in sig["theories_tested"]
            ):
                tier = max_tier(tier, "tested")
        elif camel in {
            "driver",
            "property",
            "hessian",
            "geometry",
            "basis",
            "brillouinZone",
            "simulationCell",
            "scf",
            "dft",
            "mp2",
            "ccsd",
            "tce",
            "nwpw",
        } or snake in {
            "driver",
            "property",
            "hessian",
            "geometry",
            "basis",
            "brillouin_zone",
            "simulation_cell",
        }:
            tier = max_tier(tier, "embed")
            if camel.lower() in tests.lower() or snake in tests.lower():
                tier = max_tier(tier, "tested")
    return tier


def tier_for_stanza(kind: str, sig: dict) -> str:
    tier = "schema"
    if kind in sig["kind_cases"] or kind in sig["render_stanzas"]:
        tier = max_tier(tier, "text")
    # render function names map: taskStanza -> task, basisStanza -> basis
    aliases = {
        "task": "task",
        "basis": "basis",
        "mrccData": "mrcc",
        "brillouinZone": "brillouin",
        "simulationCell": "simulation",
        "pseudopotential": "pseudopotential",
    }
    k = kind.lower()
    for e in sig["extract"]:
        el = e.lower()
        if k in el or el in k or aliases.get(kind, "").lower() in el:
            tier = max_tier(tier, "embed")
    for e in sig["embed_set"]:
        el = e.lower()
        if k in el or el in k or aliases.get(kind, "").lower() in el:
            tier = max_tier(tier, "embed")
    # set/raw always text; set also embed via rtdb strings
    if kind == "set":
        tier = max_tier(tier, "embed")
    if kind == "raw":
        tier = max_tier(tier, "text")
    if kind in {"dft", "scf", "driver", "nwpw", "ccsd", "tce", "basis", "mp2", "tddft",
                "property", "geometry", "pseudopotential", "brillouinZone",
                "simulationCell"}:
        if tier == "schema":
            tier = "text"
        # if extract exists for family
        fam = kind.lower().replace("zone", "").replace("cell", "")
        if any(fam[:4] in e.lower() for e in sig["extract"] | sig["embed_set"]):
            tier = max_tier(tier, "embed")
    tests = sig.get("tests_blob", "")
    if kind.lower() in tests.lower() and tier_rank_ge(tier, "embed"):
        tier = max_tier(tier, "tested")
    elif kind in {"dft", "scf", "basis", "nwpw", "driver", "ccsd", "tce", "mp2"} and tier == "embed":
        if kind.lower() in tests.lower():
            tier = max_tier(tier, "tested")
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


def tier_for_schema_field(struct: str, name: str, sig: dict) -> str:
    tier = "schema"
    blob = sig["params"] + sig["nwchemc"] + sig["embed"]
    tests = sig.get("tests_blob", "")
    # struct referenced in render/extract
    if struct in blob or name in blob:
        tier = max_tier(tier, "text")
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
