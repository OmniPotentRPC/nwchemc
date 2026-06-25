#!/usr/bin/env python3
"""Generate schema/inventory/nwchem_features.json and C intern tables from schema."""
from __future__ import annotations

import json
import re
import sys
from pathlib import Path


def cstr(s: str) -> str:
    return '"' + s.replace("\\", "\\\\").replace('"', '\\"') + '"'


def main() -> int:
    root = Path(sys.argv[1] if len(sys.argv) > 1 else ".")
    schema = (root / "schema/Potentials.capnp").read_text(encoding="utf-8")
    c_src = (root / "src/nwchemc_params.c").read_text(encoding="utf-8")

    mod_block = re.search(r"enum NWChemModuleName \{(.*?)\}", schema, re.S)
    if not mod_block:
        print("NWChemModuleName not found", file=sys.stderr)
        return 1
    modules = [
        {"camel": m.group(1), "id": int(m.group(2))}
        for m in re.finditer(r"(\w+)\s+@(\d+);", mod_block.group(1))
    ]

    lit_map = {
        m.group(1): m.group(2)
        for m in re.finditer(
            r"case NWChemModuleName_(\w+):\s*return \"([^\"]+)\"", c_src
        )
    }

    params_block = re.search(r"struct NWChemParams \{(.*?)\}", schema, re.S)
    if not params_block:
        print("NWChemParams not found", file=sys.stderr)
        return 1
    fields = [
        {
            "name": m.group(1),
            "id": int(m.group(2)),
            "type": m.group(3).strip().split("#")[0].strip(),
        }
        for m in re.finditer(r"(\w+)\s+@(\d+)\s*:\s*([^;]+);", params_block.group(1))
    ]

    stanza_enum = re.search(
        r"struct NWChemInputStanza.*?enum Kind \{(.*?)\}", schema, re.S
    )
    stanza_kinds = (
        re.findall(r"(\w+)\s+@\d+;", stanza_enum.group(1)) if stanza_enum else []
    )

    stanza_meta = {
        "generic": (
            "NWChemInputStanza.generic",
            "NWChemGenericStanza",
            "Named block with directives then end",
        ),
        "dft": (
            "NWChemInputStanza.dft",
            "NWChemDftStanza",
            "Typed dft block with xc/direct/smear/directives",
        ),
        "set": (
            "NWChemInputStanza.set",
            "NWChemSetDirective",
            "set key value RTDB directive",
        ),
        "raw": ("NWChemInputStanza.raw", "Text", "Literal NWChem input fragment"),
        "module": (
            "NWChemInputStanza.module",
            "NWChemModuleStanza",
            "Typed module block from NWChemModuleName",
        ),
        "scf": (
            "NWChemInputStanza.scf",
            "NWChemScfStanza",
            "Typed scf block (vectors/maxiter/thresh/directives)",
        ),
        "task": (
            "NWChemInputStanza.taskStanza",
            "NWChemTaskStanza",
            "Explicit task theory operation [ignore] line",
        ),
        "driver": (
            "NWChemInputStanza.driver",
            "NWChemDriverStanza",
            "Geometry optimization driver block",
        ),
        "property": (
            "NWChemInputStanza.property",
            "NWChemPropertyStanza",
            "Property block (dipole/mulliken/quadrupole)",
        ),
        "basis": (
            "NWChemInputStanza.basisStanza",
            "NWChemBasisStanza",
            "Structured basis/ECP block (complements top-level basis)",
        ),
        "geometry": (
            "NWChemInputStanza.geometry",
            "NWChemGeometryStanza",
            "Geometry block metadata (units/symmetry; coords via ABI)",
        ),
        "pseudopotential": (
            "NWChemInputStanza.pseudopotential",
            "NWChemPseudopotentialStanza",
            "Pseudopotential library/file entries block",
        ),
    }

    stanzas = []
    for kind in stanza_kinds:
        path, typ, role = stanza_meta.get(
            kind, (f"NWChemInputStanza.{kind}", kind, f"{kind} stanza")
        )
        stanzas.append(
            {
                "kind": kind,
                "schema_path": path,
                "capnp_type": typ,
                "nwchem_role": role,
                "stub": True,
                "embed": True,
            }
        )

    field_roles = {
        "basis": ("Gaussian basis set name", True, True),
        "theory": ("Main theory/task family (scf/dft/...)", True, True),
        "scfType": ("SCF type or DFT XC keyword", True, True),
        "charge": ("Molecular charge", True, True),
        "multiplicity": ("Spin multiplicity 2S+1", True, True),
        "enginePath": ("dlopen path for engine; empty probes env", True, False),
        "nwchemRoot": ("NWCHEM_TOP embed root hint", True, True),
        "task": ("Task label energy|gradient|hessian|property", True, True),
        "title": ("Optional title/start prefix", True, True),
        "memoryMb": ("Memory MiB; 0 keeps defaults", True, True),
        "scratchDir": ("NWCHEM_SCRATCH_DIR", True, True),
        "permanentDir": ("NWCHEM_PERMANENT_DIR", True, True),
        "inputBlocks": ("Raw directive blocks before task", True, True),
        "inputStanzas": ("Structured input stanzas", True, True),
    }

    inventory = {
        "version": 1,
        "schema": "schema/Potentials.capnp",
        "modules": [],
        "stanzas": stanzas,
        "params_fields": [],
        "abi_entrypoints": [
            {
                "name": "nwchemc_set_params",
                "stub": "fails non-zero",
                "embed": "applies Cap'n Proto params",
            },
            {
                "name": "nwchemc_energy_gradient",
                "stub": "fails ok==0",
                "embed": "runs energy/gradient",
            },
            {
                "name": "nwchemc_energy",
                "stub": "fails ok==0",
                "embed": "runs energy-only (no grad output)",
            },
            {
                "name": "nwchemc_energy_forces",
                "stub": "fails ok==0",
                "embed": "runs energy/forces (negated gradient)",
            },
            {
                "name": "nwchemc_hessian",
                "stub": "fails ok==0",
                "embed": "runs Cartesian Hessian",
            },
            {
                "name": "nwchemc_session_create",
                "stub": "returns NULL",
                "embed": "creates persistent Cap'n Proto session",
            },
            {
                "name": "nwchemc_session_set_params",
                "stub": "fails non-zero",
                "embed": "replaces persistent Cap'n Proto params",
            },
            {
                "name": "nwchemc_session_destroy",
                "stub": "no-op",
                "embed": "releases persistent session",
            },
            {
                "name": "nwchemc_session_energy_gradient",
                "stub": "fails ok==0",
                "embed": "runs session energy/gradient",
            },
            {
                "name": "nwchemc_session_energy",
                "stub": "fails ok==0",
                "embed": "runs session energy-only",
            },
            {
                "name": "nwchemc_session_energy_forces",
                "stub": "fails ok==0",
                "embed": "runs session energy/forces",
            },
            {
                "name": "nwchemc_session_calculate_forces",
                "stub": "fails ok==0",
                "embed": "runs session ForceInput energy/forces",
            },
            {
                "name": "nwchemc_session_hessian",
                "stub": "fails ok==0",
                "embed": "runs session Cartesian Hessian",
            },
            {
                "name": "nwchemc_available",
                "stub": "returns 0",
                "embed": "returns 1",
            },
            {
                "name": "nwchemc_version",
                "stub": "contains stub",
                "embed": "library version",
            },
            {
                "name": "nwchemc_finalize",
                "stub": "no-op",
                "embed": "finalize owned runtime",
            },
        ],
    }

    for mod in modules:
        nw = lit_map.get(mod["camel"])
        if mod["camel"] == "custom":
            nw = "(customName Text)"
            role = "Caller-supplied block name via customName"
        else:
            role = f"NWChem input handler block `{nw}`"
        inventory["modules"].append(
            {
                "feature_id": f"module.{mod['camel']}",
                "schema_path": f"NWChemModuleName.{mod['camel']}",
                "enum_id": mod["id"],
                "camel": mod["camel"],
                "nwchem_text": nw,
                "role": role,
                "stub_applicable": True,
                "embed_applicable": True,
                "driver_class": "module_stanza",
            }
        )

    for f in fields:
        role, stub, embed = field_roles.get(
            f["name"], ("NWChemParams field", True, True)
        )
        inventory["params_fields"].append(
            {
                "feature_id": f"params.{f['name']}",
                "schema_path": f"NWChemParams.{f['name']}",
                "field_id": f["id"],
                "name": f["name"],
                "type": f["type"],
                "role": role,
                "stub_applicable": stub,
                "embed_applicable": embed,
                "driver_class": "params_field",
            }
        )

    out_dir = root / "schema/inventory"
    out_dir.mkdir(parents=True, exist_ok=True)
    (out_dir / "nwchem_features.json").write_text(
        json.dumps(inventory, indent=2) + "\n", encoding="utf-8"
    )

    entries = []
    for m in inventory["modules"]:
        entries.append(
            {
                "id": m["feature_id"],
                "path": m["schema_path"],
                "role": m["nwchem_text"] or m["role"],
                "klass": "NWCHEMC_FEATURE_MODULE",
                "eid": m["enum_id"],
                "stub": 1,
                "embed": 1,
            }
        )
    for s in inventory["stanzas"]:
        entries.append(
            {
                "id": f"stanza.{s['kind']}",
                "path": s["schema_path"],
                "role": s["nwchem_role"],
                "klass": "NWCHEMC_FEATURE_STANZA",
                "eid": -1,
                "stub": 1,
                "embed": 1,
            }
        )
    for f in inventory["params_fields"]:
        entries.append(
            {
                "id": f["feature_id"],
                "path": f["schema_path"],
                "role": f["role"],
                "klass": "NWCHEMC_FEATURE_PARAMS_FIELD",
                "eid": f["field_id"],
                "stub": 1 if f["stub_applicable"] else 0,
                "embed": 1 if f["embed_applicable"] else 0,
            }
        )
    for a in inventory["abi_entrypoints"]:
        entries.append(
            {
                "id": f"abi.{a['name']}",
                "path": f"include/nwchemc.h::{a['name']}",
                "role": f"stub={a['stub']}; embed={a['embed']}",
                "klass": "NWCHEMC_FEATURE_ABI",
                "eid": -1,
                "stub": 1,
                "embed": 1,
            }
        )

    h = """#pragma once
/**
 * @file nwchemc_features.h
 * @brief Machine-readable intern table for NWChemParams / module / stanza features.
 *
 * Kept in sync with schema/inventory/nwchem_features.json and schema/Potentials.capnp.
 * Regenerate via tools/gen_feature_inventory.py when schema changes.
 */

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NWChemCFeatureClass {
  NWCHEMC_FEATURE_MODULE = 0,
  NWCHEMC_FEATURE_STANZA = 1,
  NWCHEMC_FEATURE_PARAMS_FIELD = 2,
  NWCHEMC_FEATURE_ABI = 3,
} NWChemCFeatureClass;

typedef struct NWChemCFeatureEntry {
  const char *feature_id;
  const char *schema_path;
  const char *nwchem_text_or_role;
  NWChemCFeatureClass klass;
  int enum_or_field_id;
  int stub_applicable;
  int embed_applicable;
} NWChemCFeatureEntry;

size_t nwchemc_feature_count(void);
const NWChemCFeatureEntry *nwchemc_feature_table(void);
const NWChemCFeatureEntry *nwchemc_feature_find(const char *feature_id);
size_t nwchemc_feature_count_class(NWChemCFeatureClass klass);
const char *nwchemc_module_nwchem_name(int module_enum_id);
size_t nwchemc_module_feature_count(void);

#ifdef __cplusplus
}
#endif
"""

    c_lines = [
        '#include "nwchemc_features.h"',
        "",
        "#include <string.h>",
        "",
        "static const NWChemCFeatureEntry k_features[] = {",
    ]
    for e in entries:
        c_lines.append(
            f'    {{{cstr(e["id"])}, {cstr(e["path"])}, {cstr(e["role"])}, '
            f'{e["klass"]}, {e["eid"]}, {e["stub"]}, {e["embed"]}}},'
        )
    c_lines.extend(
        [
            "};",
            "",
            f"static const size_t k_feature_count = {len(entries)};",
            "",
            "size_t nwchemc_feature_count(void) { return k_feature_count; }",
            "",
            "const NWChemCFeatureEntry *nwchemc_feature_table(void) { return k_features; }",
            "",
            "const NWChemCFeatureEntry *nwchemc_feature_find(const char *feature_id) {",
            "  if (!feature_id)",
            "    return NULL;",
            "  for (size_t i = 0; i < k_feature_count; ++i) {",
            "    if (strcmp(k_features[i].feature_id, feature_id) == 0)",
            "      return &k_features[i];",
            "  }",
            "  return NULL;",
            "}",
            "",
            "size_t nwchemc_feature_count_class(NWChemCFeatureClass klass) {",
            "  size_t n = 0;",
            "  for (size_t i = 0; i < k_feature_count; ++i) {",
            "    if (k_features[i].klass == klass)",
            "      ++n;",
            "  }",
            "  return n;",
            "}",
            "",
            "size_t nwchemc_module_feature_count(void) {",
            "  return nwchemc_feature_count_class(NWCHEMC_FEATURE_MODULE);",
            "}",
            "",
            "const char *nwchemc_module_nwchem_name(int module_enum_id) {",
            "  for (size_t i = 0; i < k_feature_count; ++i) {",
            "    if (k_features[i].klass == NWCHEMC_FEATURE_MODULE &&",
            "        k_features[i].enum_or_field_id == module_enum_id) {",
            "      const char *t = k_features[i].nwchem_text_or_role;",
            "      if (t && t[0] == '(')",
            "        return NULL;",
            "      return t;",
            "    }",
            "  }",
            "  return NULL;",
            "}",
            "",
        ]
    )

    (root / "include/nwchemc_features.h").write_text(h, encoding="utf-8")
    (root / "src/nwchemc_features.c").write_text("\n".join(c_lines) + "\n", encoding="utf-8")

    # Copy self into tools/ if invoked externally
    tools_script = root / "tools/gen_feature_inventory.py"
    if not tools_script.exists() or tools_script.resolve() != Path(__file__).resolve():
        pass  # installed separately

    print(
        f"modules={len(inventory['modules'])} fields={len(inventory['params_fields'])} "
        f"stanzas={len(stanzas)} intern_rows={len(entries)}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
