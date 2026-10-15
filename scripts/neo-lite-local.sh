#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
export LD_LIBRARY_PATH="$PWD/third_party/onnxruntime-dist/lib:$(printenv LD_LIBRARY_PATH || true)"
exec "$PWD/build/neo-lite/neo-lite" "$@"
