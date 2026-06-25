#!/usr/bin/env python3
"""Cross-check intern inventory vs schema, C table, and options doc."""
from __future__ import annotations

import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SCHEMA = ROOT / "schema" / "Potentials.capnp"
INVENTORY = ROOT / "schema" / "inventory" / "nwchem_features.json"
FEATURES_C = ROOT / "src" / "nwchemc_features.c"
OPTIONS_DOC = ROOT / "docs" / "orgmode" / "reference" / "nwchem-options.org"


def parse_module_enums(text: str) -> set[str]:
    block = re.search(r"enum NWChemModuleName \{(.*?)\}", text, re.S)
    if not block:
        raise SystemExit("NWChemModuleName not found in schema")
    return set(re.findall(r"(\w+)\s+@\d+;", block.group(1)))


def parse_params_fields(text: str) -> set[str]:
    block = re.search(r"struct NWChemParams \{(.*?)\}", text, re.S)
    if not block:
        raise SystemExit("NWChemParams not found in schema")
    return set(re.findall(r"(\w+)\s+@\d+\s*:", block.group(1)))


def parse_stanza_kinds(text: str) -> set[str]:
    block = re.search(r"struct NWChemInputStanza.*?enum Kind \{(.*?)\}", text, re.S)
    if not block:
        return set()
    return set(re.findall(r"(\w+)\s+@\d+;", block.group(1)))


def main() -> int:
    schema = SCHEMA.read_text(encoding="utf-8")
    inv = json.loads(INVENTORY.read_text(encoding="utf-8"))
    features_c = FEATURES_C.read_text(encoding="utf-8")
    options_doc = OPTIONS_DOC.read_text(encoding="utf-8") if OPTIONS_DOC.exists() else ""

    schema_mods = parse_module_enums(schema)
    inv_mods = {m["camel"] for m in inv["modules"]}
    schema_fields = parse_params_fields(schema)
    inv_fields = {f["name"] for f in inv["params_fields"]}
    schema_stanzas = parse_stanza_kinds(schema)
    inv_stanzas = {s["kind"] for s in inv["stanzas"]}

    errors: list[str] = []
    for label, missing, extra in (
        ("modules", schema_mods - inv_mods, inv_mods - schema_mods),
        ("params_fields", schema_fields - inv_fields, inv_fields - schema_fields),
        ("stanzas", schema_stanzas - inv_stanzas, inv_stanzas - schema_stanzas),
    ):
        if missing:
            errors.append(f"inventory missing {label}: {sorted(missing)}")
        if extra:
            errors.append(f"inventory extra {label}: {sorted(extra)}")

    for mod in sorted(schema_mods):
        if f'"module.{mod}"' not in features_c:
            errors.append(f"C intern table missing module.{mod}")
    for kind in sorted(schema_stanzas):
        if f'"stanza.{kind}"' not in features_c:
            errors.append(f"C intern table missing stanza.{kind}")
    for name in sorted(schema_fields):
        if f'"params.{name}"' not in features_c:
            errors.append(f"C intern table missing params.{name}")
    for abi in (
        "nwchemc_set_params",
        "nwchemc_energy_gradient",
        "nwchemc_energy_forces",
        "nwchemc_hessian",
        "nwchemc_available",
        "nwchemc_version",
        "nwchemc_finalize",
    ):
        if f'"abi.{abi}"' not in features_c:
            errors.append(f"C intern table missing abi.{abi}")

    # Options doc must document intern/inventory and each top-level field.
    if OPTIONS_DOC.exists():
        if "schema/inventory/nwchem_features.json" not in options_doc:
            errors.append("options.org missing feature inventory path")
        if "nwchemc_features" not in options_doc and "Feature Inventory" not in options_doc:
            errors.append("options.org missing feature inventory section/reference")
        for name in sorted(schema_fields):
            if f"~{name}~" not in options_doc and f"| ~{name}~" not in options_doc:
                # enginePath may be embed-only; still require intern row (checked above)
                if name == "enginePath":
                    continue
                errors.append(f"options.org missing documented field ~{name}~")
        for kind in sorted(schema_stanzas):
            # stanza kinds appear as section titles or intern ids
            if kind not in options_doc and f"stanza.{kind}" not in options_doc:
                if kind == "pseudopotential" and "Pseudopotential" in options_doc:
                    continue
                errors.append(f"options.org missing stanza coverage for {kind}")

    print(
        f"modules={len(schema_mods)} fields={len(schema_fields)} "
        f"stanzas={len(schema_stanzas)} intern_ok_rows="
        f"{features_c.count('NWCHEMC_FEATURE_')}"
    )
    if errors:
        for e in errors:
            print(f"ERROR: {e}", file=sys.stderr)
        return 1
    print("inventory_ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
