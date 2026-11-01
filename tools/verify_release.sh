#!/usr/bin/env sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD_DIR=${BUILD_DIR:-"$ROOT/build-release-check"}
EXAMPLE_TMP=$(mktemp -d)
cleanup() {
  rm -f "$ROOT/projects/fixtures/config.normalized.json" \
        "$ROOT/projects/fixtures/sales-report.csv" \
        "$ROOT/projects/fixtures/tasks.db"
  rm -rf "$EXAMPLE_TMP"
}
trap cleanup EXIT
cd "$ROOT"
cmake -S "$ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release -DLUCY_ENABLE_SQLITE=ON
cmake --build "$BUILD_DIR" --parallel
ctest --test-dir "$BUILD_DIR" --output-on-failure
LUCY_TEST_SQLITE=1 LUCY_PATH="$ROOT/stdlib" "$BUILD_DIR/lucy" "$ROOT/tools/lucy_test.lucy" "$BUILD_DIR/lucy" "$ROOT/tests"

# Exercise the Lucy-native ecosystem commands in an isolated temporary tree.
LUCY_PATH="$ROOT/stdlib" "$BUILD_DIR/lucy" "$ROOT/tools/lucy_new.lucy" ecosystem_smoke "$EXAMPLE_TMP/ecosystem_smoke"
(cd "$EXAMPLE_TMP/ecosystem_smoke" && \
  LUCY_PATH="$ROOT/stdlib" "$BUILD_DIR/lucy" src/main.lucy && \
  LUCY_PATH="$ROOT/stdlib" "$BUILD_DIR/lucy" tests/smoke.lucy && \
  LUCY_PATH="$ROOT/stdlib" "$BUILD_DIR/lucy" "$ROOT/tools/lucy_pkg.lucy" init && \
  printf 'func answer() { return 42 }\n' > package_source.lucy && \
  LUCY_PATH="$ROOT/stdlib" "$BUILD_DIR/lucy" "$ROOT/tools/lucy_pkg.lucy" add smoke package_source.lucy 0.1.0 && \
  printf 'import smoke\nassert smoke.answer() == 42\n' > use_package.lucy && \
  LUCY_PATH="$ROOT/stdlib" "$BUILD_DIR/lucy" use_package.lucy && \
  LUCY_PATH="$ROOT/stdlib" "$BUILD_DIR/lucy" "$ROOT/tools/lucy_pkg.lucy" update smoke tests/smoke.lucy 0.2.0 && \
  LUCY_PATH="$ROOT/stdlib" "$BUILD_DIR/lucy" "$ROOT/tools/lucy_pkg.lucy" verify && \
  LUCY_PATH="$ROOT/stdlib" "$BUILD_DIR/lucy" "$ROOT/tools/lucy_pkg.lucy" remove smoke)
LUCY_PATH="$ROOT/stdlib" "$BUILD_DIR/lucy" "$ROOT/tools/lucy_ext_new.lucy" smoke_ext "$EXAMPLE_TMP/smoke_ext"
cmake -S "$EXAMPLE_TMP/smoke_ext" -B "$EXAMPLE_TMP/smoke_ext/build" \
  -DLUCY_SOURCE_DIR="$ROOT" -DLUCY_RUNTIME_DIR="$BUILD_DIR"
cmake --build "$EXAMPLE_TMP/smoke_ext/build" --parallel
LUCY_PATH="$ROOT/stdlib" "$BUILD_DIR/lucy" "$ROOT/projects/config_validator.lucy"
LUCY_PATH="$ROOT/stdlib" "$BUILD_DIR/lucy" "$ROOT/projects/sales_report.lucy"
LUCY_PATH="$ROOT/stdlib" "$BUILD_DIR/lucy" "$ROOT/projects/sqlite_task_store.lucy"
for example in "$ROOT"/examples/*.lucy; do
  (cd "$EXAMPLE_TMP" && LUCY_PATH="$ROOT/stdlib" "$BUILD_DIR/lucy" "$example" >/dev/null)
done
LUCY_PATH="$ROOT/stdlib" "$BUILD_DIR/lucy" "$ROOT/tools/verify_doc_examples.lucy" "$BUILD_DIR/lucy"
printf '\nLucy release verification passed (tests, projects, and documentation examples).\n'
