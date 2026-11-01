# Lucy 2.0.7 — Release Verification

## Scope

Lucy 2.0.7 is an ecosystem architecture, diagnostics, and stabilization release. It synchronizes release metadata and adds specifications for the package registry, dependency resolver, and LSP. It does not claim those three systems are implemented.

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

## Notes\n\nThe first documentation-example run used a SQLite-disabled build and correctly failed the SQLite example because the optional feature was not compiled in. The successful 16/1 result was obtained with a separate SQLite-enabled executable. The SQLite-enabled build compiled the runtime and CLI; its auxiliary REPL completion test target was not completed within the available build window, so no SQLite-enabled full CTest result is claimed.\n\n## Required commands

```sh
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug --parallel
ctest --test-dir build-debug --output-on-failure
./build-debug/lucy --version
./build-debug/lucy tools/stdlib_manager.lucy verify
./build-debug/lucy tools/lucy_test.lucy ./build-debug/lucy
./build-debug/lucy tools/verify_doc_examples.lucy
unzip -t Lucy-2.0.7-source.zip
```

Adjust executable paths for Windows/macOS and the selected generator. Do not mark an item passed without retaining its output.

## Release decision

This archive is a prepared 2.0.7 source/documentation candidate, not a verified binary release. Publish only after all required platform checks have passed and the release evidence has been attached.
