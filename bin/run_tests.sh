#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT}/test/build"

mkdir -p "${BUILD_DIR}"
cmake -S "${ROOT}/test" -B "${BUILD_DIR}"
cmake --build "${BUILD_DIR}"
"${BUILD_DIR}/beacon_unit_tests"
