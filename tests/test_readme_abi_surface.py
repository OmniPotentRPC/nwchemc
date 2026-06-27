#!/usr/bin/env python3
import os
import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
HEADER = Path(os.environ.get("NWCHEMC_HEADER_PATH", ROOT / "include" / "nwchemc.h"))
README = Path(os.environ.get("NWCHEMC_README_PATH", ROOT / "README.md"))

DECL_RE = re.compile(
    r"""
    (?:^|\n)\s*
    (?:
      NWChemCResult\s+|
      NWChemCSession\s+\*\s*|
      const\s+char\s+\*\s*|
      size_t\s+|
      int\s+|
      void\s+
    )
    (nwchemc_[A-Za-z0-9_]+)\s*\(
    """,
    re.VERBOSE,
)


def unique_in_order(names):
    seen = set()
    result = []
    for name in names:
        if name in seen:
            continue
        seen.add(name)
        result.append(name)
    return result


def declared_abi_names(text):
    return unique_in_order(DECL_RE.findall(text))


def readme_abi_block(text):
    match = re.search(r"```c\n(?P<body>.*?)\n```", text, re.S)
    if not match:
        raise AssertionError("README.md does not contain a C ABI code block")
    return match.group("body")


class ReadmeAbiSurfaceTest(unittest.TestCase):
    maxDiff = None

    def test_readme_c_block_matches_header_exports(self):
        header_names = declared_abi_names(HEADER.read_text(encoding="utf-8"))
        readme_names = declared_abi_names(
            readme_abi_block(README.read_text(encoding="utf-8"))
        )

        missing = sorted(set(header_names) - set(readme_names))
        extra = sorted(set(readme_names) - set(header_names))
        self.assertEqual(missing, [])
        self.assertEqual(extra, [])
        self.assertEqual(readme_names, header_names)


if __name__ == "__main__":
    raise SystemExit(unittest.main())
