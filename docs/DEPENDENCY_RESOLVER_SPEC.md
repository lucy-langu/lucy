# Lucy Dependency Resolver — Deterministic Resolution Specification

**Status:** design specification; transitive dependency solving is not implemented in this source archive.

## 1. Responsibilities

The resolver converts declared version constraints into one deterministic dependency graph and a lockfile. It must be a pure planning stage: resolution does not download, extract, or execute packages. Acquisition and installation happen only after a complete valid plan exists.

## 2. Version constraints

Use SemVer 2.0.0 precedence. The manifest parser must distinguish exact versions, comparator sets, caret ranges, tilde ranges, wildcard ranges, and prerelease identifiers. The accepted grammar must be documented and tested; unsupported syntax must fail rather than be approximated.

Examples:

```toml
[dependencies]
exact_tool = "=1.2.3"
patch_updates = "~1.2.0"
compatible_minor = "^1.2.0"
bounded = ">=1.2.0 <2.0.0"
```

Prerelease versions are not selected by a stable range unless the constraint explicitly includes a prerelease or an opt-in flag is supplied.

## 3. Resolution algorithm

A practical first implementation can use deterministic backtracking:

1. Load the root manifest and normalize direct dependency names.
2. Build candidate sets from the configured registry and local overrides.
3. Filter candidates by all accumulated constraints, Lucy compatibility, platform constraints, yanked state, and prerelease policy.
4. Select the unresolved package with the smallest remaining candidate set (fail-first heuristic).
5. Try candidates in descending SemVer order, with deterministic tie-breaking by registry priority and immutable release ID.
6. Add transitive constraints and continue.
7. Backtrack on an empty candidate set or a conflict.
8. Detect cycles and report the complete dependency path.
9. Emit a canonical graph and lockfile only after all constraints are satisfied.

The resolver must not silently choose a version that violates a constraint. A conflict report should identify the package, each requesting parent, and the incompatible ranges.

## 4. Determinism

Given the same root manifest, registry snapshot, platform target, resolver policy, and overrides, resolution must produce byte-identical normalized lock data. Network response order and filesystem enumeration order must not affect the result.

## 5. Lockfile

The future lockfile format must be versioned. Suggested structure:

```toml
lock_version = 1
lucy_version = "2.0.7"
resolver_version = "0.1.0"

[[package]]
name = "text_helpers"
version = "1.4.2"
source = "registry+https://registry.example.invalid"
release_id = "immutable-release-id"
sha256 = "..."
dependencies = []

[roots]
app = ["text_helpers"]
```

This is a format example only; `registry.example.invalid` is deliberately non-operational. Do not copy it as a real service endpoint.

Each package record must pin an exact version and immutable source identity, digest, and transitive dependencies. The lockfile must never contain credentials. A `--locked` mode must fail if the manifest and lockfile disagree; it must not rewrite the lockfile.

## 6. Overrides and sources

Support project-local path overrides for development, but mark them clearly in lock data and avoid pretending they are portable registry artifacts. Source priority must be explicit. Two different sources claiming the same package/version must not be merged.

## 7. Commands (proposed interface)

```sh
lucy package resolve
lucy package install
lucy package update
lucy package tree
lucy package why text_helpers
lucy package check
```

These are proposed future commands, not commands guaranteed by the current binary. The current supported local manager remains `lucy tools/lucy_pkg.lucy ...`.

## 8. Failure model

Required diagnostic classes:

- no candidate satisfies a constraint;
- incompatible transitive constraints;
- dependency cycle;
- unavailable registry/source;
- invalid or yanked release;
- unsupported manifest/lock schema;
- digest mismatch;
- Lucy runtime compatibility mismatch;
- platform/architecture incompatibility.

Errors must include a minimal reproducible explanation and suggest the narrowest corrective action. Never silently discard a dependency.

## 9. Acceptance tests

Cover exact pins, caret/tilde/comparator ranges, prereleases, yanked versions, transitive conflicts, cycles, platform filters, local overrides, deterministic output, stale lockfiles, missing registries, digest mismatch, and offline mode.
