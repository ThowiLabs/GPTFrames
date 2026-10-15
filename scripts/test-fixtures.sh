#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")/.."
compiler="${HFC_HOST_CXX:-c++}"
tool="build/host-tools/gptframes-test-fixtures"
mkdir -p build/host-tools
if [ ! -x "$tool" ] || [ tests/fixtures.cpp -nt "$tool" ] || [ mcp/common.hpp -nt "$tool" ] || [ voice/neo_bridge.hpp -nt "$tool" ]; then
  "$compiler" -std=c++17 -O2 tests/fixtures.cpp -o "$tool"
fi
exec "$tool" "$@"
