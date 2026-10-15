#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
app="${1:-build/remix-linux/hyperframes-cpp-native}"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
fixture="examples/json-compositor/showcase.json"
"$app" --project validate "$fixture" | grep -q '3 escenas, 18s'
"$app" --project frame "$fixture" "$tmp/frame-a.png" 2 > /dev/null
"$app" --project frame "$fixture" "$tmp/frame-b.png" 2 > /dev/null
cmp "$tmp/frame-a.png" "$tmp/frame-b.png"
"$app" --project frame "$fixture" "$tmp/frame-c.png" 8 > /dev/null
! cmp -s "$tmp/frame-a.png" "$tmp/frame-c.png"
sh scripts/test-fixtures.sh json "$fixture" "$tmp" compositor
mkdir -p "$tmp/assets"
cp examples/json-compositor/assets/* "$tmp/assets/"
"$app" --project frame "$tmp/modified.json" "$tmp/changed.png" 2 > /dev/null
! cmp -s "$tmp/frame-a.png" "$tmp/changed.png"
if "$app" --project validate "$tmp/wrong-order.json" >/dev/null 2>&1; then echo "Falló validación del orden" >&2;exit 1;fi
if "$app" --project validate "$tmp/traversal.json" >/dev/null 2>&1; then echo "Falló validación de rutas" >&2;exit 1;fi
if "$app" --project frame "$fixture" "$tmp/invalid.png" 19 >/dev/null 2>&1; then echo "Aceptó tiempo fuera de rango" >&2;exit 1;fi
"$app" --project render "$fixture" "$tmp/clip.mp4" 2 >"$tmp/render.log" 2>&1
if command -v ffprobe >/dev/null 2>&1;then
 video="$(ffprobe -v error -select_streams v:0 -show_entries stream=codec_name -of default=noprint_wrappers=1:nokey=1 "$tmp/clip.mp4")"
 audio="$(ffprobe -v error -select_streams a:0 -show_entries stream=codec_name -of default=noprint_wrappers=1:nokey=1 "$tmp/clip.mp4")"
 frames="$(ffprobe -v error -select_streams v:0 -show_entries stream=nb_frames -of default=noprint_wrappers=1:nokey=1 "$tmp/clip.mp4")"
 test "$video" = "h264";test "$audio" = "aac";test "$frames" = "48"
fi
printf 'JSON compositor OK: capas, determinismo, datos editables, validación, MP4 y AAC.\n'
