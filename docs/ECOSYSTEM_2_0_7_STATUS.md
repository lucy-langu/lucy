# Lucy 2.0.7 Ecosystem Status Matrix

This matrix distinguishes shipped behavior from specifications and future implementation work.

| Area | Status in supplied source tree | Evidence / boundary |
|---|---|---|
| Project scaffolding | Implemented | `tools/lucy_new.lucy` |
| Local `.lucy` package manager | Implemented | `tools/lucy_pkg.lucy`; local source only |
| Project-local package imports | Implemented | runtime lookup + fixture test |
| `lucy.lock` local integrity record | Implemented | local manager; not a remote provenance signature |
| Standard-library manifest manager | Implemented | `tools/stdlib_manager.lucy` |
| Native C++ extension scaffold | Implemented | `tools/lucy_ext_new.lucy` |
| Project test runner | Implemented | `tools/lucy_test.lucy` |
| Remote package registry | Specification only | `PACKAGE_REGISTRY_SPEC.md` |
| Transitive dependency resolver | Specification only | `DEPENDENCY_RESOLVER_SPEC.md` |
| Production LSP server | Specification only | `LSP_SPEC.md` |
| Source-level debugger / DAP | Not shipped | tracked as a future tooling gap |
| Formatter / linter | Not shipped | not claimed in release scope |

## Release rule

Only rows marked implemented may appear in release notes as shipped features. A specification is a design contract and implementation plan, not executable functionality.
