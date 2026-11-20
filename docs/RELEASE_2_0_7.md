# Lucy 2.0.7 — Ecosystem and Stability Release

## Release intent

Lucy 2.0.7 is an ecosystem, diagnostics, and stabilization release. It does not intentionally introduce new language syntax, change expression semantics, or expand the core object model. The priorities are reproducible package workflows, clear contracts for future ecosystem services, debugging, regression prevention, and documentation accuracy.

## Scope

- Establish the versioned design contracts for the Lucy Package Registry.
- Specify deterministic dependency resolution and lockfile semantics.
- Specify the Language Server Protocol (LSP) boundary and editor capabilities.
- Audit existing local package tooling and native extension boundaries.
- Implement declarative project command metadata and cross-platform Binstub generation.
- Consolidate repository maintenance/build/test/package workflows behind the official Paw CLI, with command implementations in Lucy.
- Improve the maintainer workflow for diagnostics, regression triage, and release verification.
- Synchronize version metadata and documentation navigation.

## Important implementation status

This source archive contains a **local-source package manager** (`tools/paw/commands/package.lucy`), project-local package discovery, native extension scaffolding, test tooling, standard-library manifest tooling, project `[commands]` metadata, and cross-platform Binstub generation.

The following are specified in this release but are **not implemented as production services/clients in this source tree**:

- A hosted or self-hostable remote package registry and its authentication/publishing service.
- A transitive semantic-version dependency solver.
- An LSP server process speaking JSON-RPC over stdio.

The design documents define boundaries and acceptance criteria; they must not be read as evidence that remote publishing, remote installation, dependency solving, or editor LSP features already work. Existing local package commands remain the supported package workflow.

## Compatibility policy

- No intentional language grammar or runtime semantic change is part of this release.
- `VERSION`, CMake project version, CLI version output, runtime `VERSION`, standard-library manifest language version, and core API contract version must agree.
- `lucy.lock` remains the local package manager's lockfile. Any future lockfile format change requires an explicit schema version and migration behavior.
- Registry metadata and resolver behavior must be versioned independently from the language runtime.
- The LSP must consume the same lexer/parser/diagnostic contracts as the CLI; it must not maintain a divergent parser.

## Verification gate

Do not publish a release as fully verified until the release checklist in `VERIFICATION_2_0_7.md` has been run on clean Linux, Windows, and macOS environments. A platform is verified only by its own successful build and test result.

## Documents added

- `PACKAGE_REGISTRY_SPEC.md`
- `DEPENDENCY_RESOLVER_SPEC.md`
- `LSP_SPEC.md`
- `DEBUGGING_AND_DIAGNOSTICS.md`
- `REGRESSION_AND_RELEASE_PROCESS.md`
- `ECOSYSTEM_2_0_7_STATUS.md`
- `COMMAND_METADATA_AND_BINSTUBS.md`


## Paw consolidation addendum

The repository's first-party tool entry point is `paw`. It is built and installed beside `lucy`, while its dispatcher and subcommands are Lucy source files. The old per-tool scripts have been consolidated under `tools/paw/commands/`. Existing project `[commands]` and `lucy run` remain supported for backwards compatibility; they are not the repository's toolchain interface.

The latest Linux snapshot passed 64/64 CTest tests, including Paw integration and four standard-library helper API suites. See [Ecosystem Verification](ECOSYSTEM_VERIFICATION.md) for the exact platform and feature boundaries.
