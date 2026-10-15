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
python3 - "$project" "$tmp" <<'PY'
import json,sys,pathlib
src=json.loads(pathlib.Path(sys.argv[1]).read_text())
assert {l["type"] for s in src["scenes"] for l in s["layers"]} >= {"glow","ring","grid","rect","text","line","ellipse"}
p=pathlib.Path(sys.argv[2])
s=json.loads(json.dumps(src))
for l in s["scenes"][0]["layers"]:
 if l.get("type")=="rect" and "radius" in l: l["radius"]=99999;break
(p/"invalid-radius.json").write_text(json.dumps(s))
s=json.loads(json.dumps(src))
s["scenes"][0]["background"]="#GGGGGG"
(p/"invalid-color.json").write_text(json.dumps(s))
PY
for broken in invalid-radius.json invalid-color.json; do
 if "$app" --project validate "$tmp/$broken" >/dev/null 2>&1;then echo "Validación falló: $broken";exit 1;fi
done
echo "Showcase v2: geometrías, diseño editable, fotogramas deterministas y validación OK"
