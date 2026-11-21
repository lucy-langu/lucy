# Lucy 2.0.7 — Release Verification

## Scope

Lucy 2.0.7 is an ecosystem architecture, diagnostics, and stabilization release. It synchronizes release metadata and adds specifications for the package registry, dependency resolver, and LSP. It does not claim those three systems are implemented.

## Original release-candidate verification (historical)

The earlier 44-test / 45-runner figures below describe a previous source snapshot and are superseded by the dated ecosystem verification record at the end of this document.

## Automated verification

The supplied source archive was inspected and edited for documentation/version consistency. A clean Linux Debug configuration and build completed with SQLite disabled. CTest passed all 44 tests. The Lucy-native runner reported 37 passed, 0 failed, and 1 skipped (optional SQLite test). Standard-library manifest validation passed. Documentation examples passed 16, with 1 network-dependent HTTP example skipped, using a separate SQLite-enabled executable. Windows and macOS were not tested.

| Check | Status | Evidence |
|---|---|---|
| Archive integrity (`unzip -t`) | Passed | Final archive integrity check |
| Clean Linux Debug build | Passed | GNU C++ 14.2.0, CMake Debug, SQLite disabled |
| Linux CTest | Passed | 44/44 passed, SQLite disabled |
| Lucy-native test runner | Passed | 37 passed, 0 failed, 1 skipped (optional SQLite) |
| Documentation example verification | Passed | 16 passed, 1 network-dependent skipped, 0 failed (SQLite-enabled executable) |
| `lucy --version` | Passed | Printed `2.0.7` |
| Runtime `VERSION` | Passed | Executed `println VERSION`; printed `2.0.7` |
| Standard-library manifest validation | Passed | 16 module records verified |
| Project-local package integration | Passed | CTest fixture passed |
| Native extension scaffold/build | Pending | Not exercised in this pass |
| Windows/MSVC build and tests | Pending | CI evidence required |
| Windows/MinGW build and tests | Pending | If supported by release matrix |
| macOS build and tests | Pending | CI evidence required |

## Metadata consistency checklist

- [x] `VERSION` set to `2.0.7`.
- [x] CMake project version set to `2.0.7`.
- [x] CLI version/banner strings updated to `2.0.7`.
- [x] Runtime `VERSION` and REPL banner strings updated to `2.0.7`.
- [x] `stdlib/manifest.json` language version updated to `2.0.7`.
- [x] Core API contract version updated to `2.0.7`.
- [ ] Build and execute the version checks from a clean build.

## Notes

The first documentation-example run used a SQLite-disabled build and correctly failed the SQLite example because the optional feature was not compiled in. The successful 16/1 result was obtained with a separate SQLite-enabled executable. The SQLite-enabled build compiled the runtime and CLI; its auxiliary REPL completion test target was not completed within the available build window, so no SQLite-enabled full CTest result is claimed.

## Required commands

```sh
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug --parallel
ctest --test-dir build-debug --output-on-failure
./build-debug/lucy --version
./build-debug/paw stdlib verify
./build-debug/paw test ./build-debug/lucy
./build-debug/paw docs-check ./build-debug/lucy
unzip -t Lucy-2.0.7-source.zip
```

Adjust executable paths for Windows/macOS and the selected generator. Do not mark an item passed without retaining its output.

## Release decision

This archive is a prepared 2.0.7 source/documentation candidate, not a verified binary release. Publish only after all required platform checks have passed and the release evidence has been attached.


## Additive object/API verification — 2026-11-12

This verification supplements the earlier release-candidate record above. The source tree now includes the additive built-in object protocol and native value APIs documented in `BUILTIN_REFERENCE.md`. Version metadata remains `2.0.7`; no grammar changes were made.

| Check | Status | Evidence |
|---|---|---|
| Linux Release build | Passed | GCC 14.2.0, CMake 3.31.6, SQLite disabled |
| Linux CTest | Passed | 53/53 passed, including four expected-failure contract tests |
| Object protocol tests | Passed | `47_object_protocols.lucy` |
| Collection API tests | Passed | `48_collection_methods.lucy` |
| Scalar API tests | Passed | `49_scalar_methods.lucy` |
| Invalid-input contracts | Passed | Array sort, Map fetch, numeric parity, and reduce arity tests |
| REPL completion | Passed | Built-in, object, function, class, and collection completion checks |
| Standard-library manifest | Passed | 16 module records verified |
| Lucy-native test runner | Passed | 45 passed, 0 failed, 1 optional SQLite test skipped |
| Documentation examples | Passed | 15 passed, 2 skipped (SQLite disabled and network-dependent HTTP), 0 failed |
| Windows MinGW 9.2 | Pending | Must be run on the user's Windows toolchain |
| Windows/MSVC and macOS | Pending | CI evidence required |

The Linux result does not establish Windows compatibility. The Windows process execution test and native resource/icon compilation still require the Windows CI matrix or a local MinGW/MSVC run.



## Ecosystem command and binstub verification — 2026-11-12

The source tree was extended with declarative `[commands]` metadata, `lucy run`, command listing, and project-bound binstub generation. The Linux build was repeated after these changes.

| Check | Status | Evidence |
|---|---|---|
| Clean Linux Release build | Passed | GCC 14.2.0, CMake, SQLite disabled |
| CTest | Passed | 60/60 tests, including command dispatch, Binstubs, project scaffolding, global scaffolder Binstub, and local package workflow |
| Lucy-native test runner | Passed | 45 passed, 0 failed, 1 optional SQLite test skipped |
| Standard-library manifest | Passed | 16 module records verified |
| Documentation examples | Passed | 15 passed, 2 skipped (optional SQLite and network-dependent HTTP), 0 failed |
| Project command argument forwarding | Passed | Multi-word argument preserved by `lucy run` and POSIX Binstub |
| Unsafe command path rejection | Passed | Duplicate entries, missing files, and project path traversal covered |
| Windows / macOS | Pending | Platform-specific CI evidence is still required |

The Binstub integration test executed on Linux. The Windows `.cmd` branch is implemented but was not executed in this environment. The release is not claimed to be cross-platform verified until Windows and macOS CI results are available.


## Paw consolidation and standard-library helper APIs — 2026-11-13

The later Paw source snapshot supersedes the historical command/Binstub architecture described above.

| Check | Result |
|---|---|
| Linux CMake build | Passed |
| CTest | 64/64 passed |
| Paw installed-prefix smoke test | Passed |
| Paw project create/run/test | Passed |
| Documentation examples | 15 passed, 2 skipped, 0 failed |
| Standard-library manifest | 16 modules verified |

The official repository CLI is now `paw`. First-party commands are implemented as Lucy modules under `tools/paw/commands/`; legacy individual tool files and root-project per-command Binstubs have been removed. Low-level `[commands]` support remains in the Lucy executable for existing user projects.

Twenty-two additive Lucy-level helper APIs were added to each of `fs`, `system`, `data`, and `text`. Dedicated tests and module documentation were added. These are standard-library compositions, not new C++ native primitives.

Only Linux was tested directly. Windows/macOS and SQLite-enabled configurations remain unverified for this snapshot.
