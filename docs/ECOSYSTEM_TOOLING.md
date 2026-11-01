# Lucy ecosystem tooling

Lucy ships its developer-facing project tools as Lucy programs. Python is not required to scaffold a project, manage local source packages, or run a project's Lucy tests.

## Project scaffold

```sh
lucy tools/lucy_new.lucy my_app
cd my_app
lucy src/main.lucy
```

The scaffold creates `src/`, `tests/`, `packages/`, `extensions/`, and `docs/`, along with a `lucy.toml`, README, executable entry point, and smoke test. The manifest is descriptive metadata; it does not yet implement a remote dependency solver.

## Local package management

```sh
lucy tools/lucy_pkg.lucy init
lucy tools/lucy_pkg.lucy add csv_tools ../csv_tools.lucy 0.1.0
lucy tools/lucy_pkg.lucy update csv_tools ./csv_tools_v2.lucy 0.2.0
lucy tools/lucy_pkg.lucy list
lucy tools/lucy_pkg.lucy verify
lucy tools/lucy_pkg.lucy remove csv_tools
```

The current package manager deliberately supports local `.lucy` source files only. It installs them under `packages/<name>/src/init.lucy`, writes package metadata, and records the installed source and semantic version in `lucy.lock`. Installation stages the source before replacing the destination. The lockfile is a reproducibility and integrity record, not a cryptographic signature.

Not implemented yet: remote registries, package downloads, dependency graph solving, version-range resolution, checksums/signatures, and native extension binary distribution. The CLI reports these boundaries rather than pretending that a local file copy is a full registry client.

## Test runner

```sh
lucy tools/lucy_test.lucy ./build/lucy
```

The runner executes `.lucy` tests under `tests/`, applies the repository's expected-failure list, prints `PASS`, `XFAIL`, `XPASS`, `FAIL`, and `SKIP`, and applies a 30-second timeout to each test. SQLite tests are skipped by default; set `LUCY_TEST_SQLITE=1` only when Lucy was built with SQLite support.

CTest remains the authoritative full native test suite because it also includes C++ tests and CMake diagnostic checks:

```sh
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Process API requirement

Tooling uses `system.exec(argv, options)` instead of shell command construction. It captures `stdout` and `stderr` separately, returns an exit status and PID, supports a working directory, and enforces a process timeout. `system.run(command)` remains for compatibility but should not receive untrusted interpolated input.

## Ecosystem roadmap and implementation boundaries

1. Remote package registry and signed package metadata.
2. Dependency graph resolver with semver constraints and deterministic lockfile generation.
3. Native extension manifest, ABI compatibility checks, and platform artifact selection.
4. `lucy build` and `lucy package` frontends over the same manifest and process APIs.
5. Editor language-server protocol (LSP), formatter, linter, and documentation generation as first-party Lucy tools.


## 2.0.7 ecosystem architecture

Lucy 2.0.7 formalizes three ecosystem components:

- **Package Registry:** remote package discovery, immutable releases, publisher ownership, and verified downloads. See [Package Registry Specification](PACKAGE_REGISTRY_SPEC.md).
- **Dependency Resolver:** deterministic SemVer constraint solving and versioned lockfile generation. See [Dependency Resolver Specification](DEPENDENCY_RESOLVER_SPEC.md).
- **Language Server Protocol:** shared-parser diagnostics and editor intelligence over JSON-RPC stdio. See [LSP Specification](LSP_SPEC.md).

These three components are **specifications, not shipped implementations** in this archive. The existing local package manager remains the only supported package installation workflow. Proposed future command names in the specifications must not be used in user documentation as if they already exist.

For 2.0.7 stabilization work, use [Debugging and Diagnostics](DEBUGGING_AND_DIAGNOSTICS.md), [Regression and Release Process](REGRESSION_AND_RELEASE_PROCESS.md), and the [Ecosystem Status Matrix](ECOSYSTEM_2_0_7_STATUS.md).
