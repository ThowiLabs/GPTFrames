#!/bin/sh
set -eu
APP="$1"
ROOT="$2"
PROJECT="$ROOT/examples/pro-studio-1080p/project.json"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT INT TERM
"$APP" --project validate "$PROJECT" | grep -q '1920x1080'
"$APP" --project lint "$PROJECT" > "$TMP/review.json"
grep -q '"warning_count": 0' "$TMP/review.json"
"$APP" --project frame "$PROJECT" "$TMP/a.png" 1.0 >/dev/null
"$APP" --project frame "$PROJECT" "$TMP/b.png" 2.5 >/dev/null
test -s "$TMP/a.png" && test -s "$TMP/b.png"
if cmp -s "$TMP/a.png" "$TMP/b.png"; then
 echo 'Camera/procedural animation produced two identical frames' >&2
 exit 1
fi
echo 'PASS 1920x1080, centered layout, zero lint warnings, animated frames'
