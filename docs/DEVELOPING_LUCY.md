# Developing Lucy 2.0.0

## 1. Repository structure

The repository is intentionally split by responsibility:

```text
include/lucy/    public and runtime headers
src/             C++ implementation
stdlib/          Pure-Lucy standard library
tests/           regression programs and REPL tests
examples/        user-facing examples
docs/            documentation
extensions/      native extension examples/resources
editors/         editor integrations
assets/          project assets
```

## 2. Build before editing behavior

Use an out-of-tree build:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Keep the build tree outside source directories when possible.

## 3. Regression tests

Lucy uses small `.lucy` programs for language behavior. Each feature should have a focused test. Recent 2.0 regression areas include:

- language 1.1.1 compatibility coverage;
- SQLite optional behavior;
- typed values and integer literals;
- type errors;
- 2.0 language features;
- struct field contracts;
- language regressions;
- malformed hex/binary/decimal separators;
- standard-library availability;
- built-in type extensions;
- array ranges and indexed loops;
- numeric base conversions.

The source tree currently contains the numeric conversion regression test as `tests/18_numeric_base_conversions.lucy`.

## 4. Adding a language feature

A typical language feature crosses several layers:

1. Token definition if new syntax is required.
2. Lexer recognition.
3. Parser/AST representation.
4. Runtime evaluation.
5. Completion/help support if the feature is user-visible.
6. Regression tests.
7. Documentation.

Do not stop after the parser accepts syntax. A feature is incomplete until runtime semantics, diagnostics, tests, and docs agree.

## 5. Adding a standard-library feature

Prefer Pure Lucy when possible:

```text
stdlib/<module>.lucy
```

If a platform/external-library primitive is necessary, add the narrowest native bridge and keep the public wrapper readable.

Document:

- function signature;
- accepted values;
- return value;
- errors;
- platform limitations;
- one realistic example.

## 6. API consistency

Before adding a new API, search the whole runtime and standard library for equivalent behavior. If an operation already exists under several names, choose one public spelling and document compatibility aliases together.

For example, numeric integer formatting now has one conceptual family:

```lucy
bin(value, width)
hex(value, width)
oct(value, width)

value.to_binary(width)
value.to_hex(width)
value.to_octal(width)
```

The numeric `hex` family must not be confused with `text.hex_encode`, which performs data/text encoding.

## 7. Documentation synchronization

Every user-visible change should update the appropriate current documentation page. Removed behavior belongs in `HISTORY_AND_MIGRATION.md`, not in current examples.

Documentation should be treated as a release artifact alongside the code. Version-aware docs and explicit migration guidance reduce ambiguity when behavior changes. citeturn0search0turn0search3

## 8. Testing documentation examples

Examples should be chosen so they can become regression tests. If an example claims:

```lucy
println 255.to_hex()
```

the corresponding runtime test should verify the actual result.

## 9. C++ style

Keep implementation readable and split by responsibility. Avoid growing `main.cpp` into a second runtime implementation. Public extension contracts belong in headers under `include/lucy/`.

Comments should explain why a non-obvious implementation exists. Do not write comments that merely repeat the code.

## 10. Release checklist

Before a Lucy release:

- build from a clean tree;
- run the complete test suite;
- verify optional SQLite behavior in both configurations when possible;
- check REPL completion;
- check standard-library resolution from a different working directory;
- verify installation layout;
- regenerate/update docs;
- check examples against current syntax;
- update changelog/history;
- verify that removed APIs are not presented as current.
