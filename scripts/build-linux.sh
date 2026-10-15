#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
if [[ ! -f font_embedded.hpp ]]; then
 sh scripts/embed-fonts.sh
fi
g++ -std=c++17 -O2 -Wall -Wextra -Werror -pedantic -static -s main.cpp -o hyperframes-linux
printf 'Compilado Linux: %s\n' "$PWD/hyperframes-linux"
