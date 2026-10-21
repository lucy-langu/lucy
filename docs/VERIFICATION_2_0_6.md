# Lucy 2.0.6 — Release verification

## Scope

This release consolidates fixes for string slicing/indexing, string iteration, and full-expression string interpolation, synchronizes the documentation with those behaviors, and updates current build/runtime version metadata to `2.0.6`.

## Regression coverage

- `tests/34_string_range_indexing.lucy`: inclusive/exclusive ranges, negative indexes, bounds behavior, and Unicode code points.
- `tests/35_string_iteration.lucy`: iteration over strings and zero-based code-point indexes, including Unicode.
- `tests/36_expression_interpolation.lucy`: shorthand interpolation, arithmetic, method calls, module function calls, and map/member expressions.
- Existing runtime hardening and language regression tests remain enabled.

## Validation status

- Source archive integrity: run `unzip -t Lucy-2.0.6.zip` after packaging.
- Linux Debug build: passed with CMake using the current release source tree.
- Linux CTest: **40/40 passed** (`ctest --test-dir /tmp/lucy206-build --output-on-failure`).
- CLI version check: `lucy --version` returned `2.0.6`.
- Windows/MSVC GitHub Actions: pending; no native Windows result is claimed.
- Windows/MSVC GitHub Actions: pending. The prior `NOMINMAX` and conservative stack-depth changes are retained, but a successful Windows run must not be inferred from Linux results.

## Release checklist

- [x] Confirm `VERSION`, CMake project version, runtime `VERSION`, CLI output, installer message, standard-library manifest, and core API contract report `2.0.6`.
- [x] Run the complete CTest suite from a clean Debug build directory (40/40 passed).
- [ ] Run Windows/MSVC GitHub Actions and retain the complete log.
- [ ] Verify installed docs include `LANGUAGE_REFERENCE.md`, `CHANGELOG.md`, and this release record.
