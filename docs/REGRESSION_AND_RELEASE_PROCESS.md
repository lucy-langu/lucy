# Lucy Regression, Audit, and Release Process

## 1. Release philosophy

Prefer a narrow, reproducible fix over a broad refactor. A release candidate should not mix unrelated language semantics changes with package tooling or documentation work. For 2.0.7, changes should be limited to diagnostics, ecosystem contracts, reliability, documentation, and version consistency.

## 2. Required test layers

1. **Unit/native tests:** C++ helpers, source spans, parser/runtime edge cases.
2. **Language regressions:** `.lucy` programs for valid and expected-error behavior.
3. **Tool tests:** package manager, project scaffold, standard-library manifest, documentation generator, test runner.
4. **Integration tests:** project-local package import, native extension build/load, optional SQLite.
5. **Documentation examples:** execute every example marked runnable.
6. **Platform builds:** Linux, Windows/MSVC, Windows/MinGW where supported, and macOS.

## 3. Bug-to-test workflow

```text
issue
  -> classify subsystem
  -> reproduce on baseline
  -> minimize input
  -> add failing regression
  -> implement smallest fix
  -> run focused test
  -> run complete native + language suite
  -> update docs/API inventory
  -> review compatibility
  -> record platform evidence
```

## 4. Required release metadata audit

The following values must agree:

- `VERSION`;
- `project(... VERSION ...)` in `CMakeLists.txt`;
- CLI `--version` output;
- runtime global `VERSION`;
- REPL banner;
- `stdlib/manifest.json` language version;
- `docs/contracts/core_api.json` version field;
- installer/package metadata, if included.

Historical verification files must retain their historical version numbers. Do not globally replace historical records.

## 5. Documentation review

For every public behavior change, update all affected references:

- language reference;
- programming/getting-started guide;
- builtin and standard-library references;
- per-module API page;
- REPL/tooling docs;
- examples and tests;
- changelog and release verification.

Mark examples as runnable, illustrative, expected-error, or environment-dependent. Do not present a proposed CLI command as implemented.

## 6. Cross-platform evidence

For each platform record OS version, architecture, compiler/toolchain, CMake version, build type, feature flags, command, test totals, and CI run URL or log artifact. Use `pending` when no evidence exists.

## 7. Release archive

- Start from a clean checkout/export.
- Exclude build directories, caches, temporary logs, and personal environment files.
- Include source, tests, docs, editor assets, licenses, and release scripts.
- Run `unzip -t` on the final archive.
- Build from the extracted archive, not only the working tree.
- Compare archive contents with the intended tracked source.
- Generate checksums and publish them with the release.

## 8. Stop-ship conditions

Do not mark the release ready if any of these remain:

- version metadata mismatch;
- failing regression test;
- undocumented behavior change;
- broken clean build;
- lockfile corruption or unsafe archive extraction;
- unverified claim of Windows/macOS support;
- secrets or local paths in release artifacts;
- release notes claim a registry, resolver, LSP, debugger, or formatter that is not actually implemented.
