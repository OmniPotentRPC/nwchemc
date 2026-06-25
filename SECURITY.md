# Security Policy

## Supported Versions

Security fixes are released through normal semantic-version tags.

| Version | Supported |
| ------- | --------- |
| 0.1.x   | yes       |

## Reporting a Vulnerability

Email security-sensitive reports to the maintainer address in the repository
metadata. Include the affected commit or tag, platform, compiler, NWChem build
details, and a minimal reproducer when one is available.

Do not include credentials, proprietary molecular data, or machine-local paths
that are not needed to reproduce the issue.

## Security Boundary

`nwchemc` accepts Cap'n Proto messages from callers and converts them to an
NWChem input/configuration for an in-process NWChem run. Treat untrusted input
as untrusted code for the NWChem engine: a caller that can set arbitrary NWChem
input stanzas can request file paths, scratch locations, memory settings, and
module-specific operations supported by the linked NWChem build.
