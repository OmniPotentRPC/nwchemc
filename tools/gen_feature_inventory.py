#!/usr/bin/env python3
"""Generate schema/inventory/nwchem_features.json and C intern tables from schema."""
from __future__ import annotations

import json
import re
import sys
from pathlib import Path


def cstr(s: str) -> str:
    return '"' + s.replace("\\", "\\\\").replace('"', '\\"') + '"'


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


def parse_struct_fields(body: str) -> list[dict[str, object]]:
    fields = []
    for match in re.finditer(r"^\s*(\w+)\s+@(\d+)\s*:\s*([^;]+);", body, re.M):
        fields.append(
            {
                "name": match.group(1),
                "id": int(match.group(2)),
                "type": match.group(3).strip(),
            }
        )
    return fields


def parse_interface_methods(body: str) -> list[dict[str, object]]:
    methods = []
    for match in re.finditer(
        r"^\s*(\w+)\s+@(\d+)\s*\((.*?)\)\s*->\s*\((.*?)\);",
        body,
        re.M | re.S,
    ):
        methods.append(
            {
                "name": match.group(1),
                "id": int(match.group(2)),
                "params": " ".join(match.group(3).split()),
                "results": " ".join(match.group(4).split()),
            }
        )
    return methods


def main() -> int:
    root = Path(sys.argv[1] if len(sys.argv) > 1 else ".")
    schema = (root / "schema/Potentials.capnp").read_text(encoding="utf-8")
    schema_clean = strip_capnp_comments(schema)
    c_src = (root / "src/nwchemc_params.c").read_text(encoding="utf-8")

    mod_block = find_named_block(schema_clean, "enum", "NWChemModuleName")
    if not mod_block:
        print("NWChemModuleName not found", file=sys.stderr)
        return 1
    modules = [
        {"camel": m.group(1), "id": int(m.group(2))}
        for m in re.finditer(r"(\w+)\s+@(\d+);", mod_block)
    ]

    lit_map = {
        m.group(1): m.group(2)
        for m in re.finditer(
            r"case NWChemModuleName_(\w+):\s*return \"([^\"]+)\"", c_src
        )
    }

    params_block = find_named_block(schema_clean, "struct", "NWChemParams")
    if not params_block:
        print("NWChemParams not found", file=sys.stderr)
        return 1
    fields = parse_struct_fields(params_block)

    input_stanza_block = find_named_block(schema_clean, "struct", "NWChemInputStanza")
    stanza_enum = (
        find_named_block(input_stanza_block, "enum", "Kind")
        if input_stanza_block
        else None
    )
    stanza_kinds = re.findall(r"(\w+)\s+@\d+;", stanza_enum) if stanza_enum else []

    schema_fields = []
    for struct_name, body in iter_named_blocks(schema_clean, "struct"):
        for field in parse_struct_fields(body):
            schema_fields.append(
                {
                    "feature_id": f"field.{struct_name}.{field['name']}",
                    "schema_path": f"{struct_name}.{field['name']}",
                    "struct": struct_name,
                    "name": field["name"],
                    "field_id": field["id"],
                    "type": field["type"],
                    "role": f"{struct_name}.{field['name']} Cap'n Proto field",
                    "stub_applicable": True,
                    "embed_applicable": True,
                    "driver_class": "schema_field",
                }
            )

    schema_methods = []
    for interface_name, body in iter_named_blocks(schema_clean, "interface"):
        for method in parse_interface_methods(body):
            schema_methods.append(
                {
                    "feature_id": f"method.{interface_name}.{method['name']}",
                    "schema_path": f"{interface_name}.{method['name']}",
                    "interface": interface_name,
                    "name": method["name"],
                    "method_id": method["id"],
                    "params": method["params"],
                    "results": method["results"],
                    "role": (
                        f"{interface_name}.{method['name']} Cap'n Proto "
                        "interface method"
                    ),
                    "stub_applicable": True,
                    "embed_applicable": True,
                    "driver_class": "schema_method",
                }
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
        "mrccData": (
            "NWChemInputStanza.mrccData",
            "NWChemMrccDataStanza",
            "TCE MRCC data block",
        ),
        "brillouinZone": (
            "NWChemInputStanza.brillouinZone",
            "NWChemBrillouinZoneStanza",
            "NWPW Brillouin-zone k-point controls",
        ),
        "simulationCell": (
            "NWChemInputStanza.simulationCell",
            "NWChemSimulationCellStanza",
            "NWPW simulation-cell RTDB controls",
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
        "schema_fields": schema_fields,
        "schema_methods": schema_methods,
        "abi_entrypoints": [
            {
                "name": "nwchemc_set_params",
                "stub": "fails non-zero",
                "embed": "applies Cap'n Proto params",
            },
            {
                "name": "nwchemc_configure",
                "stub": "fails non-zero",
                "embed": "applies PotentialConfig.nwchem params",
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
                "name": "nwchemc_dipole",
                "stub": "fails ok==0",
                "embed": "runs total dipole",
            },
            {
                "name": "nwchemc_polarizability",
                "stub": "fails ok==0",
                "embed": "runs electric polarizability response",
            },
            {
                "name": "nwchemc_quadrupole",
                "stub": "fails ok==0",
                "embed": "runs total traceless quadrupole",
            },
            {
                "name": "nwchemc_stress",
                "stub": "fails ok==0",
                "embed": "runs stress tensor",
            },
            {
                "name": "nwchemc_optimize",
                "stub": "fails ok==0",
                "embed": "runs geometry optimization",
            },
            {
                "name": "nwchemc_frequencies",
                "stub": "fails ok==0",
                "embed": "runs harmonic frequencies",
            },
            {
                "name": "nwchemc_energy_gradient_from_config",
                "stub": "fails ok==0",
                "embed": "runs PotentialConfig energy/gradient",
            },
            {
                "name": "nwchemc_energy_from_config",
                "stub": "fails ok==0",
                "embed": "runs PotentialConfig energy-only",
            },
            {
                "name": "nwchemc_energy_forces_from_config",
                "stub": "fails ok==0",
                "embed": "runs PotentialConfig energy/forces",
            },
            {
                "name": "nwchemc_hessian_from_config",
                "stub": "fails ok==0",
                "embed": "runs PotentialConfig Cartesian Hessian",
            },
            {
                "name": "nwchemc_dipole_from_config",
                "stub": "fails ok==0",
                "embed": "runs PotentialConfig total dipole",
            },
            {
                "name": "nwchemc_polarizability_from_config",
                "stub": "fails ok==0",
                "embed": "runs PotentialConfig electric polarizability response",
            },
            {
                "name": "nwchemc_quadrupole_from_config",
                "stub": "fails ok==0",
                "embed": "runs PotentialConfig total traceless quadrupole",
            },
            {
                "name": "nwchemc_stress_from_config",
                "stub": "fails ok==0",
                "embed": "runs PotentialConfig stress tensor",
            },
            {
                "name": "nwchemc_optimize_from_config",
                "stub": "fails ok==0",
                "embed": "runs PotentialConfig geometry optimization",
            },
            {
                "name": "nwchemc_frequencies_from_config",
                "stub": "fails ok==0",
                "embed": "runs PotentialConfig harmonic frequencies",
            },
            {
                "name": "nwchemc_session_create",
                "stub": "returns NULL",
                "embed": "creates persistent Cap'n Proto session",
            },
            {
                "name": "nwchemc_session_create_from_config",
                "stub": "returns NULL",
                "embed": "creates persistent PotentialConfig session",
            },
            {
                "name": "nwchemc_session_set_params",
                "stub": "fails non-zero",
                "embed": "replaces params before topology",
            },
            {
                "name": "nwchemc_session_configure",
                "stub": "fails non-zero",
                "embed": "configures PotentialConfig before topology",
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
                "name": "nwchemc_session_dipole",
                "stub": "fails ok==0",
                "embed": "runs session total dipole",
            },
            {
                "name": "nwchemc_session_polarizability",
                "stub": "fails ok==0",
                "embed": "runs session electric polarizability response",
            },
            {
                "name": "nwchemc_session_quadrupole",
                "stub": "fails ok==0",
                "embed": "runs session total traceless quadrupole",
            },
            {
                "name": "nwchemc_session_stress",
                "stub": "fails ok==0",
                "embed": "runs session stress tensor",
            },
            {
                "name": "nwchemc_session_optimize",
                "stub": "fails ok==0",
                "embed": "runs session geometry optimization",
            },
            {
                "name": "nwchemc_session_frequencies",
                "stub": "fails ok==0",
                "embed": "runs session harmonic frequencies",
            },
            {
                "name": "nwchemc_session_calculate_forces",
                "stub": "fails ok==0",
                "embed": "runs session ForceInput energy/forces",
            },
            {
                "name": "nwchemc_session_calculate_energy",
                "stub": "fails ok==0",
                "embed": "runs session ForceInput energy-only",
            },
            {
                "name": "nwchemc_calculate_forces",
                "stub": "fails ok==0",
                "embed": "runs one-shot ForceInput energy/forces",
            },
            {
                "name": "nwchemc_calculate_forces_from_config",
                "stub": "fails ok==0",
                "embed": "runs one-shot PotentialConfig energy/forces",
            },
            {
                "name": "nwchemc_calculate_energy",
                "stub": "fails ok==0",
                "embed": "runs one-shot ForceInput energy-only",
            },
            {
                "name": "nwchemc_calculate_energy_from_config",
                "stub": "fails ok==0",
                "embed": "runs one-shot PotentialConfig energy-only",
            },
            {
                "name": "nwchemc_energy_result_size_for_force_input",
                "stub": "returns 0",
                "embed": "sizes ForceInput energy-only PotentialResult",
            },
            {
                "name": "nwchemc_session_calculate_energy_result",
                "stub": "fails ok==0",
                "embed": "runs session ForceInput energy-only into PotentialResult",
            },
            {
                "name": "nwchemc_calculate_energy_result",
                "stub": "fails ok==0",
                "embed": "runs one-shot ForceInput energy-only into PotentialResult",
            },
            {
                "name": "nwchemc_calculate_energy_result_from_config",
                "stub": "fails ok==0",
                "embed": "runs one-shot PotentialConfig energy-only into PotentialResult",
            },
            {
                "name": "nwchemc_session_calculate_result",
                "stub": "fails ok==0",
                "embed": "runs session ForceInput energy/forces into PotentialResult",
            },
            {
                "name": "nwchemc_calculate_result",
                "stub": "fails ok==0",
                "embed": "runs one-shot ForceInput energy/forces into PotentialResult",
            },
            {
                "name": "nwchemc_calculate_result_from_config",
                "stub": "fails ok==0",
                "embed": "runs one-shot PotentialConfig energy/forces into PotentialResult",
            },
            {
                "name": "nwchemc_forces_result_size_for_force_input",
                "stub": "returns 0",
                "embed": "sizes ForceInput forces PotentialResult",
            },
            {
                "name": "nwchemc_session_calculate_forces_result",
                "stub": "fails ok==0",
                "embed": "runs session ForceInput forces into PotentialResult",
            },
            {
                "name": "nwchemc_calculate_forces_result",
                "stub": "fails ok==0",
                "embed": "runs one-shot ForceInput forces into PotentialResult",
            },
            {
                "name": "nwchemc_calculate_forces_result_from_config",
                "stub": "fails ok==0",
                "embed": "runs one-shot PotentialConfig forces into PotentialResult",
            },
            {
                "name": "nwchemc_calculate_hessian",
                "stub": "fails ok==0",
                "embed": "runs one-shot ForceInput Hessian",
            },
            {
                "name": "nwchemc_calculate_hessian_from_config",
                "stub": "fails ok==0",
                "embed": "runs one-shot PotentialConfig Hessian",
            },
            {
                "name": "nwchemc_hessian_result_size_for_force_input",
                "stub": "returns 0",
                "embed": "sizes ForceInput Hessian PotentialResult",
            },
            {
                "name": "nwchemc_session_calculate_hessian_result",
                "stub": "fails ok==0",
                "embed": "runs session ForceInput Hessian into PotentialResult",
            },
            {
                "name": "nwchemc_calculate_hessian_result",
                "stub": "fails ok==0",
                "embed": "runs one-shot ForceInput Hessian into PotentialResult",
            },
            {
                "name": "nwchemc_calculate_hessian_result_from_config",
                "stub": "fails ok==0",
                "embed": "runs one-shot PotentialConfig Hessian into PotentialResult",
            },
            {
                "name": "nwchemc_calculate_dipole",
                "stub": "fails ok==0",
                "embed": "runs one-shot ForceInput dipole",
            },
            {
                "name": "nwchemc_calculate_dipole_from_config",
                "stub": "fails ok==0",
                "embed": "runs one-shot PotentialConfig dipole",
            },
            {
                "name": "nwchemc_dipole_result_size_for_force_input",
                "stub": "returns 0",
                "embed": "sizes ForceInput dipole PotentialResult",
            },
            {
                "name": "nwchemc_session_calculate_dipole_result",
                "stub": "fails ok==0",
                "embed": "runs session ForceInput dipole into PotentialResult",
            },
            {
                "name": "nwchemc_calculate_dipole_result",
                "stub": "fails ok==0",
                "embed": "runs one-shot ForceInput dipole into PotentialResult",
            },
            {
                "name": "nwchemc_calculate_dipole_result_from_config",
                "stub": "fails ok==0",
                "embed": "runs one-shot PotentialConfig dipole into PotentialResult",
            },
            {
                "name": "nwchemc_calculate_polarizability",
                "stub": "fails ok==0",
                "embed": "runs one-shot ForceInput polarizability",
            },
            {
                "name": "nwchemc_calculate_polarizability_from_config",
                "stub": "fails ok==0",
                "embed": "runs one-shot PotentialConfig polarizability",
            },
            {
                "name": "nwchemc_polarizability_result_size_for_force_input",
                "stub": "returns 0",
                "embed": "sizes ForceInput polarizability PotentialResult",
            },
            {
                "name": "nwchemc_session_calculate_polarizability_result",
                "stub": "fails ok==0",
                "embed": "runs session ForceInput polarizability into PotentialResult",
            },
            {
                "name": "nwchemc_calculate_polarizability_result",
                "stub": "fails ok==0",
                "embed": "runs one-shot ForceInput polarizability into PotentialResult",
            },
            {
                "name": "nwchemc_calculate_polarizability_result_from_config",
                "stub": "fails ok==0",
                "embed": "runs one-shot PotentialConfig polarizability into PotentialResult",
            },
            {
                "name": "nwchemc_calculate_quadrupole",
                "stub": "fails ok==0",
                "embed": "runs one-shot ForceInput quadrupole",
            },
            {
                "name": "nwchemc_calculate_quadrupole_from_config",
                "stub": "fails ok==0",
                "embed": "runs one-shot PotentialConfig quadrupole",
            },
            {
                "name": "nwchemc_quadrupole_result_size_for_force_input",
                "stub": "returns 0",
                "embed": "sizes ForceInput quadrupole PotentialResult",
            },
            {
                "name": "nwchemc_session_calculate_quadrupole_result",
                "stub": "fails ok==0",
                "embed": "runs session ForceInput quadrupole into PotentialResult",
            },
            {
                "name": "nwchemc_calculate_quadrupole_result",
                "stub": "fails ok==0",
                "embed": "runs one-shot ForceInput quadrupole into PotentialResult",
            },
            {
                "name": "nwchemc_calculate_quadrupole_result_from_config",
                "stub": "fails ok==0",
                "embed": "runs one-shot PotentialConfig quadrupole into PotentialResult",
            },
            {
                "name": "nwchemc_calculate_stress",
                "stub": "fails ok==0",
                "embed": "runs one-shot ForceInput stress",
            },
            {
                "name": "nwchemc_calculate_stress_from_config",
                "stub": "fails ok==0",
                "embed": "runs one-shot PotentialConfig stress",
            },
            {
                "name": "nwchemc_stress_result_size_for_force_input",
                "stub": "returns 0",
                "embed": "sizes ForceInput stress PotentialResult",
            },
            {
                "name": "nwchemc_session_calculate_stress_result",
                "stub": "fails ok==0",
                "embed": "runs session ForceInput stress into PotentialResult",
            },
            {
                "name": "nwchemc_calculate_stress_result",
                "stub": "fails ok==0",
                "embed": "runs one-shot ForceInput stress into PotentialResult",
            },
            {
                "name": "nwchemc_calculate_stress_result_from_config",
                "stub": "fails ok==0",
                "embed": "runs one-shot PotentialConfig stress into PotentialResult",
            },
            {
                "name": "nwchemc_calculate_optimize",
                "stub": "fails ok==0",
                "embed": "runs one-shot ForceInput geometry optimization",
            },
            {
                "name": "nwchemc_calculate_optimize_from_config",
                "stub": "fails ok==0",
                "embed": "runs one-shot PotentialConfig geometry optimization",
            },
            {
                "name": "nwchemc_optimize_result_size_for_force_input",
                "stub": "returns 0",
                "embed": "sizes ForceInput optimization PotentialResult",
            },
            {
                "name": "nwchemc_session_calculate_optimize_result",
                "stub": "fails ok==0",
                "embed": "runs session ForceInput optimization into PotentialResult",
            },
            {
                "name": "nwchemc_calculate_optimize_result",
                "stub": "fails ok==0",
                "embed": "runs one-shot ForceInput optimization into PotentialResult",
            },
            {
                "name": "nwchemc_calculate_optimize_result_from_config",
                "stub": "fails ok==0",
                "embed": "runs one-shot PotentialConfig optimization into PotentialResult",
            },
            {
                "name": "nwchemc_calculate_frequencies",
                "stub": "fails ok==0",
                "embed": "runs one-shot ForceInput harmonic frequencies",
            },
            {
                "name": "nwchemc_calculate_frequencies_from_config",
                "stub": "fails ok==0",
                "embed": "runs one-shot PotentialConfig harmonic frequencies",
            },
            {
                "name": "nwchemc_frequencies_result_size_for_force_input",
                "stub": "returns 0",
                "embed": "sizes ForceInput frequencies PotentialResult",
            },
            {
                "name": "nwchemc_session_calculate_frequencies_result",
                "stub": "fails ok==0",
                "embed": "runs session ForceInput frequencies into PotentialResult",
            },
            {
                "name": "nwchemc_calculate_frequencies_result",
                "stub": "fails ok==0",
                "embed": "runs one-shot ForceInput frequencies into PotentialResult",
            },
            {
                "name": "nwchemc_calculate_frequencies_result_from_config",
                "stub": "fails ok==0",
                "embed": "runs one-shot PotentialConfig frequencies into PotentialResult",
            },
            {
                "name": "nwchemc_potential_result_size_for_force_input",
                "stub": "returns 0",
                "embed": "sizes session PotentialResult output from ForceInput",
            },
            {
                "name": "nwchemc_session_calculate_hessian",
                "stub": "fails ok==0",
                "embed": "runs session ForceInput Hessian",
            },
            {
                "name": "nwchemc_session_calculate_dipole",
                "stub": "fails ok==0",
                "embed": "runs session ForceInput dipole",
            },
            {
                "name": "nwchemc_session_calculate_polarizability",
                "stub": "fails ok==0",
                "embed": "runs session ForceInput polarizability",
            },
            {
                "name": "nwchemc_session_calculate_quadrupole",
                "stub": "fails ok==0",
                "embed": "runs session ForceInput quadrupole",
            },
            {
                "name": "nwchemc_session_calculate_stress",
                "stub": "fails ok==0",
                "embed": "runs session ForceInput stress",
            },
            {
                "name": "nwchemc_session_calculate_optimize",
                "stub": "fails ok==0",
                "embed": "runs session ForceInput geometry optimization",
            },
            {
                "name": "nwchemc_session_calculate_frequencies",
                "stub": "fails ok==0",
                "embed": "runs session ForceInput harmonic frequencies",
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
    for f in inventory["schema_fields"]:
        entries.append(
            {
                "id": f["feature_id"],
                "path": f["schema_path"],
                "role": f["role"],
                "klass": "NWCHEMC_FEATURE_SCHEMA_FIELD",
                "eid": f["field_id"],
                "stub": 1 if f["stub_applicable"] else 0,
                "embed": 1 if f["embed_applicable"] else 0,
            }
        )
    for m in inventory["schema_methods"]:
        entries.append(
            {
                "id": m["feature_id"],
                "path": m["schema_path"],
                "role": m["role"],
                "klass": "NWCHEMC_FEATURE_SCHEMA_METHOD",
                "eid": m["method_id"],
                "stub": 1 if m["stub_applicable"] else 0,
                "embed": 1 if m["embed_applicable"] else 0,
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
 * @brief Machine-readable intern table for NWChemParams, schema, and ABI features.
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
  NWCHEMC_FEATURE_SCHEMA_FIELD = 4,
  NWCHEMC_FEATURE_SCHEMA_METHOD = 5,
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
        f"schema_fields={len(inventory['schema_fields'])} "
        f"schema_methods={len(inventory['schema_methods'])} stanzas={len(stanzas)} "
        f"intern_rows={len(entries)}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
