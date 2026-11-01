# Lucy-native repository and ecosystem tooling

Lucy now provides first-party developer workflows as Lucy programs. These tools use the runtime's argv-based process API rather than assembling shell command strings.

## Repository maintenance

Run from the Lucy repository root after building the interpreter:

```sh
./build/lucy tools/generate_stdlib_docs.lucy
./build/lucy tools/stdlib_manager.lucy list
./build/lucy tools/stdlib_manager.lucy verify
./build/lucy tools/verify_doc_examples.lucy ./build/lucy
```

The standard-library manager validates module metadata and source files. Documentation generation indexes the shipped pure-Lucy APIs. Documentation examples are executed by Lucy and have an explicit environment-dependent/network policy.

## Project and extension scaffolding

```sh
./build/lucy tools/lucy_new.lucy my_app
./build/lucy tools/lucy_ext_new.lucy telemetry
```

`lucy_new` creates a Lucy application layout with `src`, `tests`, `packages`, `extensions`, `docs`, a `lucy.toml` project descriptor, and a smoke test. `lucy_ext_new` creates a C++ extension skeleton using the public `include/lucy/extension.hpp` API and a CMake build file.

## Local package workflow

```sh
./build/lucy tools/lucy_pkg.lucy init
./build/lucy tools/lucy_pkg.lucy add csv_tools ./csv_tools.lucy 0.1.0
./build/lucy tools/lucy_pkg.lucy update csv_tools ./csv_tools_v2.lucy 0.2.0
./build/lucy tools/lucy_pkg.lucy list
./build/lucy tools/lucy_pkg.lucy verify
./build/lucy tools/lucy_pkg.lucy remove csv_tools
```

This is a local-source package manager, not a remote registry. It stages `.lucy` files into `packages/<name>/src/init.lucy`, writes package metadata, and records package versions and source provenance in `lucy.lock`. It validates names, semantic versions, package entry paths, and consistency between lockfile and package metadata. Updates stage replacement files and roll back installed source/metadata if lockfile persistence fails.

Remote registries, dependency graph solving, version-range resolution, cryptographic checksums/signatures, native binary distribution, and registry authentication remain future work. The current tool does not claim to implement them.

## Test workflow

```sh
./build/lucy tools/lucy_test.lucy ./build/lucy
```

The runner executes Lucy test files, recognizes expected-failure cases, skips SQLite tests unless `LUCY_TEST_SQLITE=1`, and applies a 30-second process timeout. It reports `PASS`, `XFAIL`, `XPASS`, `FAIL`, and `SKIP`. CTest remains authoritative because it also includes native C++ tests and diagnostic-output checks.

## Build workflow

```sh
./build/lucy tools/lucy_build.lucy all
./build/lucy tools/lucy_build.lucy configure --build-dir build-sqlite --sqlite
./build/lucy tools/lucy_build.lucy build --build-dir build-sqlite
./build/lucy tools/lucy_build.lucy test --build-dir build-sqlite
```

The build frontend calls CMake and CTest with argv arrays. It supports build-directory selection, CMake generator selection, and the optional SQLite build flag. It does not replace CMake; it provides a Lucy-native, cross-platform orchestration entry point.

## Process contract

`system.exec(argv, options)` launches a child process without a shell and returns independent `stdout`, `stderr`, `status`, `success`, `timed_out`, and `pid` fields. Supported options include `cwd` and `timeout_ms`. A timeout terminates the child and reports status `124`. Prefer this API for developer tooling. `system.run(command)` remains for compatibility when shell semantics are explicitly needed.

## CI and release checks

CI and release automation should build Lucy first, run CTest, run the Lucy-native test runner, regenerate and inspect standard-library documentation, and verify example projects. Native extension templates should be compiled against the exact Lucy runtime build they target.
