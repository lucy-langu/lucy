# Lucy 2.0.3 Documentation

This directory documents the language, runtime, standard library, tools, extensions, build process, and known implementation limits. Treat the shipped source and regression tests as the executable contract; when prose disagrees with behavior, record the mismatch and correct the prose or implementation.

## Start here

1. [Getting Started](GETTING_STARTED.md) — installation, first programs, and initial workflow.
2. [Language Reference](LANGUAGE_REFERENCE.md) — syntax, lexical rules, types, expressions, control flow, functions, OOP, imports, and known language limitations.
3. [Programming Guide](PROGRAMMING_GUIDE.md) — idioms, common tasks, error handling, data, networking, reflection, and process control.
4. [Standard Library](STANDARD_LIBRARY.md) — public module APIs, examples, caveats, and known defects.
5. [Built-in Reference](BUILTIN_REFERENCE.md) — global runtime functions and native string/array/map/number methods.
6. [REPL and Tooling](REPL_AND_TOOLING.md) — interactive use, completion, CLI, editor integration, and troubleshooting.
7. [Real-world Workflows](REAL_WORLD_WORKFLOWS.md) — end-to-end CSV, JSON, SQLite, CLI, performance and security patterns.
8. [Runnable Projects](../projects/README.md) — three integration examples with fixtures.

## Reference and maintenance

| Document | Coverage |
|---|---|
| [Environment and Installation](ENVIRONMENT_AND_INSTALLATION.md) | Build prerequisites, installation paths, environment variables, and platform notes. |
| [Standard Library API Inventory](STANDARD_LIBRARY_API_INVENTORY.md) | Source-oriented listing of shipped library classes and methods; verify it against the source before release. |
| [Core API Contract](contracts/core_api.json) | Machine-readable signatures, input descriptions, return contracts, errors, platform targets, and examples for the core filesystem facade. |
| [Packages and Extensions](PACKAGES_AND_EXTENSIONS.md) | Import resolution, packages, native extension boundary, and extension examples. |
| [Runtime and Errors](RUNTIME_AND_ERRORS.md) | Runtime behavior, exception handling, and error categories. |
| [Implementation Architecture](IMPLEMENTATION_ARCHITECTURE.md) | Lexer/parser/runtime architecture and value execution model. |
| [Developing Lucy](DEVELOPING_LUCY.md) | Source layout, build, tests, native extension work, and release workflow. |
| [History and Migration](HISTORY_AND_MIGRATION.md) | Compatibility changes and migration notes. |
| [Changelog](CHANGELOG.md) | User-visible version history. |
| [Real-world Workflows](REAL_WORLD_WORKFLOWS.md) | Complete application workflows and operational cautions. |
| [Red-team Audit](RED_TEAM_AUDIT.md) | Historical 2.0.1 findings, dispositions, and remaining boundaries. |
| [2.0.2 Verification Record](VERIFICATION_2_0_2.md) | Exact build/test environment, passing checks, and unverified areas. |

## Per-module API pages

These pages are source-indexed and list module-level functions, exported objects, classes, and methods. The consolidated [Standard Library](STANDARD_LIBRARY.md) remains the narrative reference; the [API Inventory](STANDARD_LIBRARY_API_INVENTORY.md) is the cross-module index.

- [`app`](stdlib/app.md) · [`crypto`](stdlib/crypto.md) · [`data`](stdlib/data.md) · [`flow`](stdlib/flow.md)
- [`fs`](stdlib/fs.md) · [`http`](stdlib/http.md) · [`math`](stdlib/math.md) · [`random`](stdlib/random.md)
- [`repl`](stdlib/repl.md) · [`result`](stdlib/result.md) · [`runtime`](stdlib/runtime.md) · [`set`](stdlib/set.md)
- [`sqlite`](stdlib/sqlite.md) · [`system`](stdlib/system.md) · [`text`](stdlib/text.md) · [`time`](stdlib/time.md)

Regenerate the per-module pages after changing pure-Lucy APIs:

```sh
python3 tools/generate_stdlib_docs.py
```

## Canonicality and snippet policy

The implementation and regression suite are the final authority for behavior in the corresponding release. A documented example should be one of: (1) a standalone executable example, (2) a fragment with its required context clearly identified, (3) an expected-error example, or (4) an environment-dependent example (network, filesystem permissions, external executable). The 2.0.2 project added dedicated integration examples and regression tests for previously observed edge cases; continue to label network- and platform-dependent examples explicitly.

## Design principles stated by the project

- Braces delimit blocks; normal 2.0 syntax does not use `end`.
- Standard-library APIs favor shallow calls such as `fs.read(...)`.
- String indexing and slicing operate on UTF-8 code points in 2.0.2, but grapheme-cluster segmentation and full Unicode case mapping are not implemented.
- Type annotations are runtime contracts, not static compile-time type checking.
- Pure Lucy libraries are preferred where possible; C++ extensions are available for native functionality.
- Cross-platform support must be verified with clean platform-specific builds; a Linux result alone does not establish Windows/macOS correctness.

| [2.0.3 Verification Record](VERIFICATION_2_0_3.md) | Scope and validation status of the Ruby-inspired core filesystem facades. |

- [Object model and diagnostics](OBJECT_MODEL_AND_DIAGNOSTICS.md)
