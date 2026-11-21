# Lucy Ecosystem Tooling

Paw is the official command-line interface for Lucy's first-party ecosystem. Its command dispatcher and command implementations are Lucy programs under `tools/paw/`.

See [PAW.md](PAW.md) for installation and usage, and [ECOSYSTEM_VERIFICATION.md](ECOSYSTEM_VERIFICATION.md) for the tested feature set and platform boundaries.

The low-level project command metadata and `lucy run` support remain for backward compatibility. Repository tooling is no longer exposed as multiple global Binstubs.

Remote registry, transitive dependency resolution, and LSP documents in this repository remain specifications rather than production services.
