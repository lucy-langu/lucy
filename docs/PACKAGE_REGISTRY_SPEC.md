# Lucy Package Registry — Protocol and Service Specification

**Status:** architecture specification; remote registry client/server are not implemented in this source archive.

## 1. Goals and non-goals

The registry is the distribution and discovery layer for Pure-Lucy packages. It should provide immutable package releases, searchable metadata, publisher ownership, deprecation/yank state, and integrity metadata.

It is not a runtime module loader, a replacement for `LUCY_PATH`, or a mechanism for silently executing install-time scripts. Package installation must not execute package code.

## 2. Package identity

A package is identified by a normalized, case-insensitive registry namespace and package name. The display name may preserve publisher casing; the canonical identity is lowercase ASCII.

Recommended constraints:

- Name: `^[a-z0-9][a-z0-9_-]{0,63}$`.
- Version: strict SemVer 2.0.0.
- A release is immutable after publication.
- Re-publishing an existing `(name, version)` is rejected.
- Prerelease versions are excluded from unconstrained stable resolution unless explicitly requested.

## 3. Package layout

```text
example-package/
  lucy.toml
  README.md
  LICENSE
  src/
    init.lucy
  tests/
  docs/
```

The manifest should declare:

```toml
[package]
name = "example-package"
version = "1.2.0"
description = "Example reusable Lucy package"
license = "MIT"
authors = ["publisher"]
entry = "src/init.lucy"
lucy = ">=2.0.7 <3.0.0"

[dependencies]
text_helpers = "^1.4.0"
```

The exact accepted fields must be enforced by a manifest schema version. Unknown fields should produce a clear diagnostic; forward-compatible extension fields should live under an explicitly namespaced metadata table.

## 4. Registry API shape

The initial HTTP API should be small and versioned:

| Method | Endpoint | Purpose |
|---|---|---|
| GET | `/v1/packages/{name}` | Package metadata and release list |
| GET | `/v1/packages/{name}/{version}` | Immutable release metadata |
| GET | `/v1/packages/{name}/{version}/download` | Archive download |
| GET | `/v1/search?q=...` | Search |
| POST | `/v1/packages/{name}/releases` | Publish a release |
| POST | `/v1/packages/{name}/owners` | Manage package owners |

Responses use JSON and explicit schema versions. Error responses contain stable machine-readable codes, a human message, and optional field-level details. The service must document pagination, rate limits, maximum archive size, and retention behavior.

## 5. Release metadata

A release record should include:

- canonical package name and SemVer version;
- manifest schema version;
- supported Lucy version range;
- entry point;
- dependency constraints;
- archive size and SHA-256 digest;
- publication timestamp;
- publisher identity and immutable release identifier;
- license and source repository URL when provided;
- yanked/deprecated status.

The digest detects accidental or malicious artifact substitution in transit. It is not a publisher signature. A future signing design must define key rotation, revocation, trust roots, and signature verification before claiming authenticated provenance.

## 6. Publishing workflow

1. Validate manifest syntax and required fields.
2. Validate package name/version and reject an existing release.
3. Run package tests in an isolated environment with network access disabled by default.
4. Build a deterministic archive with normalized paths and timestamps.
5. Reject path traversal, symlinks escaping the package root, and forbidden generated files.
6. Compute the archive digest.
7. Authenticate the publisher and verify ownership.
8. Upload the immutable release and metadata atomically.
9. Return the release identifier and digest.

Registry-side validation is not a guarantee that a package is safe. Clients must show publisher, version, permissions/risks, and source metadata before installation.

## 7. Client behavior and trust

A future `lucy package publish` or `lucy pkg publish` command must never imply that the current `tools/lucy_pkg.lucy` supports remote publishing. Credentials must be stored outside project manifests and lockfiles. Tokens must not appear in command history, verbose logs, error messages, or CI output.

Remote downloads must use HTTPS, enforce redirect policy, validate content length, verify SHA-256 before extraction, and extract into a staging directory. Installation must reject archive path traversal and must not run lifecycle scripts.

## 8. Registry configuration

Configuration precedence should be explicit: command-line option, project configuration, user configuration, built-in default. A project may select a registry URL, but credentials remain user/CI scoped. The default registry URL must be a documented product decision, not an implicit hard-coded assumption.

## 9. Acceptance criteria

- Immutable releases and duplicate publication rejection.
- Strict manifest and SemVer validation.
- Search and metadata endpoints with stable schemas.
- Archive digest verification before extraction.
- Safe extraction and atomic installation.
- Authentication and owner authorization tests.
- Offline behavior and actionable network diagnostics.
- Integration tests against a disposable local registry.
- No credential leakage in logs or lockfiles.
