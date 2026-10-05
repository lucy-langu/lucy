#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DEFAULT_PREFIX="/usr/local"
PREFIX="${LUCY_PREFIX:-$DEFAULT_PREFIX}"
BUILD_DIR="${LUCY_BUILD_DIR:-${ROOT}/build-install}"

if ! command -v cmake >/dev/null 2>&1; then
  echo "CMake is required. Install CMake and a C++17 compiler first." >&2
  exit 1
fi

# If the default system prefix is not writable, install as the current user.
if [[ "$PREFIX" == "$DEFAULT_PREFIX" && ! -w "$DEFAULT_PREFIX" ]]; then
  PREFIX="${HOME}/.local"
  echo "No write access to /usr/local; using $PREFIX instead."
fi

cmake -S "$ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX"
cmake --build "$BUILD_DIR" --config Release
ctest --test-dir "$BUILD_DIR" --output-on-failure
cmake --install "$BUILD_DIR" --config Release

cat <<EOF

Lucy 2.0.0 installed successfully.
Binary: $PREFIX/bin/lucy
Stdlib: $PREFIX/share/lucy/stdlib
Editors: $PREFIX/share/lucy/editors

Lucy normally discovers its installation root automatically. For a custom or
portable environment, use one variable for all Lucy resources:

  export LUCY_PATH="$PREFIX"

LUCY_PATH controls stdlib, packages, extensions, and other Lucy resources.
EOF
