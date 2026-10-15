#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
app="${1:-build/remix-linux/hyperframes-cpp-native}"
project=examples/json-showcase-v2/showcase-v2.json
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
"$app" --project validate "$project" | grep -q '4 escenas, 24s'
"$app" --project frame "$project" "$tmp/a.png" 15 > /dev/null
"$app" --project frame "$project" "$tmp/b.png" 15 > /dev/null
cmp "$tmp/a.png" "$tmp/b.png"
"$app" --project frame "$project" "$tmp/c.png" 21 > /dev/null
if cmp -s "$tmp/a.png" "$tmp/c.png"; then echo "No hay diferencia entre escenas";exit 1;fi
mkdir -p "$tmp/assets"
cp examples/json-showcase-v2/assets/* "$tmp/assets/"
sh scripts/test-fixtures.sh json "$project" "$tmp" showcase
for broken in invalid-radius.json invalid-color.json; do
 if "$app" --project validate "$tmp/$broken" >/dev/null 2>&1;then echo "Validación falló: $broken";exit 1;fi
done
echo "Showcase v2: geometrías, diseño editable, fotogramas deterministas y validación OK"
