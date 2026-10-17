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
LUCY_PATH="$ROOT/stdlib" "$BUILD_DIR/lucy" "$ROOT/projects/config_validator.lucy"
LUCY_PATH="$ROOT/stdlib" "$BUILD_DIR/lucy" "$ROOT/projects/sales_report.lucy"
LUCY_PATH="$ROOT/stdlib" "$BUILD_DIR/lucy" "$ROOT/projects/sqlite_task_store.lucy"
for example in "$ROOT"/examples/*.lucy; do
  (cd "$EXAMPLE_TMP" && LUCY_PATH="$ROOT/stdlib" "$BUILD_DIR/lucy" "$example" >/dev/null)
done
python3 "$ROOT/tools/verify_doc_examples.py" "$BUILD_DIR/lucy"
printf '\nLucy release verification passed (tests, projects, and documentation examples).\n'
