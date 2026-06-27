#!/usr/bin/env python3

import os
import sys

project = "nwchemc"
copyright = "2026-present, nwchemc developers"
author = "Rohit Goswami"
release = "0.1.0"
version = "0.1.0"

doxyrest_prefix = os.environ.get("CONDA_PREFIX")
if doxyrest_prefix:
    sys.path.insert(0, os.path.join(doxyrest_prefix, "share", "doxyrest", "sphinx"))

extensions = [
    "doxyrest",
    "cpplexer",
    "myst_parser",
    "sphinx.ext.intersphinx",
    "sphinx_sitemap",
]

templates_path = ["_templates"]
exclude_patterns = [
    "_build",
    "Thumbs.db",
    ".DS_Store",
    "api/interface_nwchem_embed_c_api_nwchem_legacy_set_config.rst",
]
source_suffix = {
    ".rst": "restructuredtext",
    ".md": "markdown",
}

intersphinx_mapping = {
    "python": ("https://docs.python.org/3", None),
}

html_theme = "shibuya"
html_static_path = ["_static"]
html_baseurl = "https://nwchemc.rgoswami.me/"

html_theme_options = {
    "github_url": "https://github.com/OmniPotentRPC/nwchemc",
    "accent_color": "teal",
    "dark_code": True,
    "globaltoc_expand_depth": 1,
}

html_context = {
    "source_type": "github",
    "source_user": "OmniPotentRPC",
    "source_repo": "nwchemc",
    "source_version": "main",
    "source_docs_path": "/docs/source/",
}

html_css_files = ["custom.css"]
