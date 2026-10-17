# Lucy 2.0.2 verification record

This record describes checks performed for the 2.0.2 source package. It is evidence of tested paths, not a claim that the language is defect-free.

## Automated checks

- Linux/GCC C++17 Debug build completed with SQLite enabled.
- CTest regression suite: 28/28 tests passed, including built-in type extension, module-path discovery, and live completion of user-added methods.
- All shipped `examples/*.lucy` programs executed successfully in the current Linux environment.
- Per-module standard-library quick examples: 15 passed; the `http` quick example was explicitly skipped because it requires network access. The consolidated built-in reference example also passed.
- REPL pipe smoke test exercised `:vars`, `:where`, `:methods`, `:modules`, and exit handling; banner/prompts contain no ANSI color styling.
- `tools/generate_stdlib_docs.py` regenerates per-module API pages from the current `VERSION` file.

## Scope and limits

- This environment did not run a native Windows or macOS build. Removing color styling reduces Windows color-rendering issues, but terminal cursor-control behavior still depends on the console host.
- `:where` reports exact paths for discovered modules and source-file-defined functions/classes. Interactive definitions do not retain source-line metadata.
- `:modules` reports `.lucy` modules found in configured search roots; it is not a whole-disk search.
- UTF-8 string operations remain code-point-aware rather than grapheme-cluster-aware; full Unicode case conversion is not implemented.
- No claim is made that every possible program or external integration has been fuzzed or exhaustively tested.
