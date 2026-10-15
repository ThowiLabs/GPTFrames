#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
# Install system packages: espeak-ng, libespeak-ng-dev, libssl-dev,
# libjson-c-dev, libzstd-dev, cmake, g++, curl, openssl, pkg-config.
if [[ ! -f third_party/neo-tts/CMakeLists.txt ]]; then
  git submodule update --init third_party/neo-tts
fi
ROOT="$PWD/third_party/onnxruntime-dist"
ORT_SHA256=67db4dc1561f1e3fd42e619575c82c601ef89849afc7ea85a003abbac1a1a105
if [[ ! -f "$ROOT/lib/libonnxruntime.so" ]]; then
  mkdir -p "$ROOT"
  tempfile="$(mktemp)"
  trap 'rm -f "$tempfile"' EXIT
  curl -fL --retry 3 --max-time 150 \
    "https://github.com/microsoft/onnxruntime/releases/download/v1.20.1/onnxruntime-linux-x64-1.20.1.tgz" -o "$tempfile"
  echo "$ORT_SHA256  $tempfile" | sha256sum -c -
  tar -xzf "$tempfile" --strip-components=1 -C "$ROOT"
fi
cmake -S third_party/neo-tts -B build/neo-lite -DNEO_LITE_ONLY=ON \
  -DNEO_ORT_ROOT="$ROOT" -DCMAKE_BUILD_TYPE=Release
cmake --build build/neo-lite -j2
./scripts/neo-lite-local.sh --help
