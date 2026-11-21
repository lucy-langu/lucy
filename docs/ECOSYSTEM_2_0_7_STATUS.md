# Lucy 2.0.7 Ecosystem Status

This matrix describes the shipped source tree, not roadmap specifications.

| Capability | Status | Implementation |
|---|---|---|
| Project scaffolding | Implemented | `tools/paw/commands/new.lucy` |
| Project command execution | Implemented | Paw delegates to the retained `lucy run` compatibility interface |
| Build frontend | Implemented | `tools/paw/commands/build.lucy` |
| Test runner | Implemented | `tools/paw/commands/test.lucy` |
| Local source package manager | Implemented | `tools/paw/commands/package.lucy` |
| Standard-library manifest manager | Implemented | `tools/paw/commands/stdlib.lucy` |
| Documentation generation/checking | Implemented | `tools/paw/commands/docs*.lucy` |
| Native extension scaffold | Implemented | `tools/paw/commands/extension.lucy` |
| Standalone Paw executable | Implemented | `paw` target plus `tools/paw/main.lucy` |
| Remote package registry | Specification only | `PACKAGE_REGISTRY_SPEC.md` |
| Transitive dependency resolver | Specification only | `DEPENDENCY_RESOLVER_SPEC.md` |
| Production LSP | Specification only | `LSP_SPEC.md` |
| Formatter / linter / DAP | Not implemented | Future work |

Paw is the single first-party command-line interface. Legacy project `[commands]` metadata and `lucy run` remain supported for compatibility, but repository tools are no longer installed as separate global Binstubs.

Linux verification for the Paw-consolidated snapshot is recorded in [ECOSYSTEM_VERIFICATION.md](ECOSYSTEM_VERIFICATION.md). Windows and macOS remain unverified until their CI jobs pass.
