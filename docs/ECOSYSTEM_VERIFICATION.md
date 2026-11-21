# Lucy 2.0.7 + Paw — ecosystem verification

**Verification environment:** Linux, GCC 14.2, CMake, SQLite disabled.  
**Scope:** current source snapshot after Paw consolidation and additive standard-library helper APIs.

## Automated results

| Check | Result |
|---|---:|
| Clean CMake build (`lucy`, `paw`, runtime, test executables) | Passed |
| CTest | 64 / 64 passed |
| Lucy-native test runner | 49 passed, 0 failed, 1 optional SQLite test skipped |
| Documentation examples | 15 passed, 2 skipped, 0 failed |
| Standard-library manifest | 16 module records verified |
| Paw CLI help/version | Passed |
| Paw project creation | Passed |
| Paw project `run start` / `run test` | Passed |
| Installed Paw executable from a separate prefix | Passed |
| Installed Paw creating and running a project outside source tree | Passed |
| Existing language/runtime regression suite | Passed |

The two documentation skips are the network-dependent HTTP example and optional SQLite example (SQLite was disabled). CTest's 64 tests include the four new standard-library helper suites and Paw integration tests.

## Paw architecture verified

- `paw` is built as a separate executable target.
- It shares the Lucy runtime library.
- Its dispatcher is `tools/paw/main.lucy`.
- Command implementations are `.lucy` files under `tools/paw/commands/`.
- CMake installs `paw` beside `lucy`, and installs Paw source modules plus stdlib under the Lucy data directory on Unix-like systems.
- Paw locates its sibling Lucy runtime and installed standard library without relying on the caller's working directory.
- No per-command global Binstubs are generated for Lucy's own ecosystem tools.

## Additive standard-library API

Twenty-two Lucy-level helper functions were added to each of these modules:

- `fs`: project-oriented file/path operations.
- `system`: environment parsing and process execution helpers.
- `data`: collection shaping and map/array transformations.
- `text`: normalization and formatting.

The helpers are composed from existing public APIs; no new parser syntax or native runtime primitive was introduced for these additions. Each module has an API reference update and a dedicated executable test file.

## Not verified

- Windows and macOS builds and runtime installation have not been executed in this environment.
- SQLite-enabled build and SQLite-specific tests were not run.
- Remote package registry, remote publishing/download, transitive dependency resolution, production LSP/DAP, formatter, and linter are not implemented by this change.
- Local package management remains local-source based.

Do not infer cross-platform success from the Linux result. CI is configured to exercise Paw on Linux, Windows, and macOS, but those jobs must pass before making a cross-platform release claim.
