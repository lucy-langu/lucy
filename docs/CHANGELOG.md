# Lucy changelog

## 2.0.6 — String semantics, interpolation, and documentation consistency

- Fixed string range indexing: `text[0..4]` returns five UTF-8 code points, while `text[0...4]` returns four; negative endpoints and out-of-bounds clamping follow the documented range rules.
- Fixed direct string indexing to operate on UTF-8 code-point boundaries, including negative indexes, rather than raw bytes.
- Added string iteration for `for`/`foreach`; each iteration yields one UTF-8 code point and the optional index is zero-based.
- Added full expression interpolation using `${...}`, including function calls such as `${system.platform()}` and expressions such as `${10 + 20}`. `$name` and dotted member-path shorthand remain supported.
- Corrected the language reference, which previously claimed expression interpolation was unsupported despite its implementation and tests. Synchronized getting-started examples, changelog, runtime/API version metadata, and release verification notes.
- Preserved the Windows `NOMINMAX` build fix and conservative call/nesting limits from the 2.0.5 hardening work.
- Added regression tests for string range indexing, Unicode-aware string iteration, and expression interpolation.
- Validation recorded for this source bundle: Linux build and CTest run. Windows/MSVC must be confirmed by CI before claiming Windows verification.


## 2.0.2 — REPL usability, introspection, and documentation synchronization

- Removed ANSI color styling from the REPL banner and prompts; retained cursor-control sequences required for editing.
- Updated Tab completion to expand a common prefix, show candidates on the next Tab, and cycle on subsequent Tabs.
- Added `:vars`, `:vars --all`, `:functions`, `:modules`, `:where <name>`, and `:methods <Type>` commands.
- Added source-file tracking for user-defined functions and module-path inspection; interactive definitions correctly report that line metadata is unavailable.
- Restricted executable-derived module discovery to recognizable Lucy install roots so a build-tree executable does not scan arbitrary parent directories.
- Included built-in type extension methods in completion and added runnable examples/tests for type extensions and module discovery.
- Fixed `len(string)` to count UTF-8 code points, made `float(string)` reject trailing junk/non-finite values, and implemented the documented `Map.length()`/`Map.size()` methods.
- Added a consolidated native built-in reference and an executable documentation-example checker; updated the language reference, REPL manual, and source-indexed standard-library pages to version 2.0.2.

## 2.0.1 — reliability and tooling release

- Prevented `String.replace` from looping forever when the search string is empty; it now raises `ValueError`.
- Made `String.length`, `char_at`, `slice`, and `reverse` operate on UTF-8 code points instead of raw bytes. This does not promise grapheme-cluster segmentation.
- Made `String.to_int` and `String.to_float` reject trailing junk; non-finite float spellings are rejected by these conversion methods.
- Reworked CSV parsing to handle empty input and final newlines without synthetic records, and to reject malformed quoting.
- Added correct JSON `\uXXXX` surrogate-pair handling and rejection of isolated surrogates.
- Expanded edge-case regression coverage and documentation for the behaviors above.

## 2.0.0

Initial Lucy 2.0 language release. See [history and migration](HISTORY_AND_MIGRATION.md).

## 2.0.3 — Ruby-inspired core filesystem API

- Added import-free `Dir`, `File`, and `Path` global objects with a conventional object-oriented API surface.
- Kept `fs` and `system` compatibility APIs; the new facades delegate to them rather than duplicating filesystem logic.
- Bumped runtime, CLI, and CMake project version to 2.0.3.
- Added regression coverage for directory access, file metadata, and path operations.
