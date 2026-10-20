# Lucy 2.0.3 — verification record

## Scope of this update

Lucy 2.0.3 begins a compatibility-focused move toward a more Ruby-like object-oriented standard library. The new global core facades `Dir`, `File`, and `Path` are available without `import` and delegate to the existing canonical `system` and `fs` implementations. This avoids maintaining duplicate implementations and keeps older lowercase module APIs working.

Examples:

```lucy
println Dir.cwd()
println Dir.entries(".")
println File.read("README.md")
println Path.basename("src/runtime.cpp")
```

## API contract

- `Dir.cwd()` and `Dir.pwd()` return the current working directory.
- `Dir.entries`, `files`, `dirs`, `exist`, `mkdir`, `rmdir`, `empty`, `glob`, and `walk` forward to the established `fs` APIs.
- `File.read`, `write`, `append`, `read_lines`, `write_lines`, `exists`/`exist`, `size`, `delete`/`remove`, `copy`, `move`, `touch`, and `open` forward to `fs`.
- `Path.join`, `absolute`, `expand`, `basename`, `dirname`, `extension`, and `stem` forward to `fs`.
- Existing `fs.*` and `system.cwd()` APIs remain supported.

## Verification status

This file describes the intended 2.0.3 change. Build and test results must be recorded from the target platform before publishing a release. Windows and macOS native validation must not be inferred from a Linux build.

## Follow-up validation pass

The core facade implementation was revised so `File`, `Dir`, and `Path` are registered as core classes, not mutable-looking facade instances. Their methods are populated from the public `fs` and `system` module functions, and normal `class File { ... }` declarations extend the same class method table. A regression test verifies `File.read`/`File.write`, class extension, and import compatibility.

The standard library now has independent semantic-version metadata in `stdlib/manifest.json`. `tools/stdlib_manager.py verify` validates all 16 module records and their entry files. This is not a remote package registry or auto-updater.

A separate out-of-tree Release build with SQLite enabled passed all 34 CTest tests after correcting the `name_error_suggestion` test's missing `LUCY_PATH` environment. The release script also passed all three integration projects and all standalone examples. The documentation-example checker reports 16 passing examples and one skipped network-dependent example. Lexical/parser diagnostics now print the source line and caret when a reliable line/column is available; runtime locations are now tracked at statement/expression roots; per-token spans for every AST node remain a known limitation.


## Object model and diagnostic follow-up

The 2.0.3 source now supports explicit `super.method(...)`, `static func`, and method-level `public`/`protected`/`private` modifiers. The behavior and deliberate compatibility boundaries are documented in `OBJECT_MODEL_AND_DIAGNOSTICS.md`. Runtime errors crossing parsed expression/statement roots carry a 1-based source line and column; the command-line renderer prints the corresponding source excerpt and caret. User-function errors append source-aware frames during unwinding. This is source-location tracking for AST roots, not a full span for every AST node; native stack frames and Unicode display-cell column alignment remain unsupported.

The standard-library manager now supports `install-source MODULE VERSION FILE`: it replaces one module entry and updates its manifest version transactionally, refuses downgrades unless explicitly allowed, and rolls back on manifest/constraint verification failure. It does not yet provide a remote registry or prove semantic API compatibility of third-party source.

Local Linux Release validation after these changes: 34/34 CTest tests pass with SQLite enabled. Added tests cover static methods, inheritance/super dispatch, private/protected access, and runtime source excerpts. `.github/workflows/ci.yml` configures a Debug/Release matrix on Ubuntu, Windows, and macOS; Windows/macOS are pending actual CI execution and are not claimed as locally verified.
