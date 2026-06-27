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


def strip_capnp_comments(text: str) -> str:
    return re.sub(r"#.*", "", text)


def iter_named_blocks(text: str, keyword: str):
    pattern = re.compile(rf"\b{keyword}\s+(\w+)\s*\{{")
    for match in pattern.finditer(text):
        name = match.group(1)
        start = match.end() - 1
        depth = 0
        for idx in range(start, len(text)):
            if text[idx] == "{":
                depth += 1
            elif text[idx] == "}":
                depth -= 1
                if depth == 0:
                    yield name, text[start + 1 : idx]
                    break


def find_named_block(text: str, keyword: str, name: str) -> str | None:
    for block_name, body in iter_named_blocks(text, keyword):
        if block_name == name:
            return body
    return None


def parse_struct_fields(body: str) -> set[str]:
    return set(re.findall(r"^\s*(\w+)\s+@\d+\s*:", body, re.M))


def parse_module_enums(text: str) -> set[str]:
    block = find_named_block(text, "enum", "NWChemModuleName")
    if not block:
        raise SystemExit("NWChemModuleName not found in schema")
    return set(re.findall(r"(\w+)\s+@\d+;", block))


def parse_params_fields(text: str) -> set[str]:
    block = find_named_block(text, "struct", "NWChemParams")
    if not block:
        raise SystemExit("NWChemParams not found in schema")
    return parse_struct_fields(block)


def parse_schema_fields(text: str) -> set[str]:
    fields: set[str] = set()
    for struct_name, body in iter_named_blocks(text, "struct"):
        for field_name in parse_struct_fields(body):
            fields.add(f"{struct_name}.{field_name}")
    return fields


def parse_stanza_kinds(text: str) -> set[str]:
    stanza_block = find_named_block(text, "struct", "NWChemInputStanza")
    block = find_named_block(stanza_block, "enum", "Kind") if stanza_block else None
    if not block:
        return set()
    return set(re.findall(r"(\w+)\s+@\d+;", block))


def main() -> int:
    schema = strip_capnp_comments(SCHEMA.read_text(encoding="utf-8"))
    inv = json.loads(INVENTORY.read_text(encoding="utf-8"))
    features_c = FEATURES_C.read_text(encoding="utf-8")
    options_doc = OPTIONS_DOC.read_text(encoding="utf-8") if OPTIONS_DOC.exists() else ""

    schema_mods = parse_module_enums(schema)
    inv_mods = {m["camel"] for m in inv["modules"]}
    schema_fields = parse_params_fields(schema)
    inv_fields = {f["name"] for f in inv["params_fields"]}
    schema_all_fields = parse_schema_fields(schema)
    inv_schema_fields = {f"{f['struct']}.{f['name']}" for f in inv["schema_fields"]}
    schema_stanzas = parse_stanza_kinds(schema)
    inv_stanzas = {s["kind"] for s in inv["stanzas"]}

    errors: list[str] = []
    for label, missing, extra in (
        ("modules", schema_mods - inv_mods, inv_mods - schema_mods),
        ("params_fields", schema_fields - inv_fields, inv_fields - schema_fields),
        (
            "schema_fields",
            schema_all_fields - inv_schema_fields,
            inv_schema_fields - schema_all_fields,
        ),
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
    for name in sorted(schema_all_fields):
        if f'"field.{name}"' not in features_c:
            errors.append(f"C intern table missing field.{name}")
    for abi in (
        "nwchemc_set_params",
        "nwchemc_energy_gradient",
        "nwchemc_energy",
        "nwchemc_energy_forces",
        "nwchemc_hessian",
        "nwchemc_dipole",
        "nwchemc_quadrupole",
        "nwchemc_stress",
        "nwchemc_optimize",
        "nwchemc_frequencies",
        "nwchemc_session_create",
        "nwchemc_session_set_params",
        "nwchemc_session_destroy",
        "nwchemc_session_energy_gradient",
        "nwchemc_session_energy",
        "nwchemc_session_energy_forces",
        "nwchemc_session_dipole",
        "nwchemc_session_quadrupole",
        "nwchemc_session_stress",
        "nwchemc_session_optimize",
        "nwchemc_session_frequencies",
        "nwchemc_session_calculate_forces",
        "nwchemc_session_calculate_result",
        "nwchemc_calculate_result",
        "nwchemc_calculate_hessian",
        "nwchemc_hessian_result_size_for_force_input",
        "nwchemc_session_calculate_hessian_result",
        "nwchemc_calculate_hessian_result",
        "nwchemc_calculate_dipole",
        "nwchemc_dipole_result_size_for_force_input",
        "nwchemc_session_calculate_dipole_result",
        "nwchemc_calculate_dipole_result",
        "nwchemc_calculate_quadrupole",
        "nwchemc_quadrupole_result_size_for_force_input",
        "nwchemc_session_calculate_quadrupole_result",
        "nwchemc_calculate_quadrupole_result",
        "nwchemc_calculate_stress",
        "nwchemc_stress_result_size_for_force_input",
        "nwchemc_session_calculate_stress_result",
        "nwchemc_calculate_stress_result",
        "nwchemc_calculate_optimize",
        "nwchemc_optimize_result_size_for_force_input",
        "nwchemc_session_calculate_optimize_result",
        "nwchemc_calculate_optimize_result",
        "nwchemc_calculate_frequencies",
        "nwchemc_frequencies_result_size_for_force_input",
        "nwchemc_session_calculate_frequencies_result",
        "nwchemc_calculate_frequencies_result",
        "nwchemc_potential_result_size_for_force_input",
        "nwchemc_session_calculate_hessian",
        "nwchemc_session_calculate_dipole",
        "nwchemc_session_calculate_quadrupole",
        "nwchemc_session_calculate_stress",
        "nwchemc_session_calculate_optimize",
        "nwchemc_session_calculate_frequencies",
        "nwchemc_session_hessian",
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
        if "schema fields" not in options_doc:
            errors.append("options.org missing schema field inventory coverage")
        if "field.NWChemTceStanza.quadrupole" not in options_doc:
            errors.append("options.org missing TCE schema field inventory example")
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
        f"schema_fields={len(schema_all_fields)} stanzas={len(schema_stanzas)} intern_ok_rows="
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
