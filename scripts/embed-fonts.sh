#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")/.."
compiler="${HFC_HOST_CXX:-c++}"
tool="build/host-tools/gptframes-embed-fonts"
mkdir -p build/host-tools
if [ ! -x "$tool" ] || [ tools/embed_fonts.cpp -nt "$tool" ]; then
  "$compiler" -std=c++17 -O2 tools/embed_fonts.cpp -o "$tool"
fi
exec "$tool" "${HFC_FONT_DIR:-/usr/share/fonts/truetype/dejavu}" "${1:-font_embedded.hpp}"
