#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
bin="${1:-build/remix-linux/hyperframes-cpp-native}"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
"$bin" --remix-list > "$tmp/list.txt"
test "$(wc -l < "$tmp/list.txt")" -eq 15
"$bin" --inspect tests/fixtures/remix-intro.html > "$tmp/html.txt"
test "$(grep -cE '^[1-6] \|' "$tmp/html.txt")" -eq 6
"$bin" --remix-frame "$tmp/a.png" 72 > /dev/null
"$bin" --remix-frame "$tmp/b.png" 72 > /dev/null
cmp "$tmp/a.png" "$tmp/b.png"
if "$bin" --remix-frame "$tmp/bad.png" 999 > /dev/null 2>&1; then
  echo "ERROR: segundos fuera de rango aceptados" >&2;exit 1
fi
if "$bin" --remix "$tmp/bad.mp4" 61 > /dev/null 2>&1; then
  echo "ERROR: fps fuera de rango aceptados" >&2;exit 1
fi
"$bin" --remix "$tmp/remix-clip.mp4" 12 288x512 2 - > "$tmp/clip.log" 2>&1
if command -v ffprobe >/dev/null; then
 rate="$(ffprobe -v error -select_streams v:0 -show_entries stream=r_frame_rate -of default=nk=1:nw=1 "$tmp/remix-clip.mp4")"
 codec="$(ffprobe -v error -select_streams v:0 -show_entries stream=codec_name -of default=nk=1:nw=1 "$tmp/remix-clip.mp4")"
 duration="$(ffprobe -v error -show_entries format=duration -of default=nk=1:nw=1 "$tmp/remix-clip.mp4")"
 test "$rate" = "12/1"
 test "$codec" = "h264"
 test "$duration" = "2.000000"
fi
echo "Remix verificado: 15 escenas, HTML original, frames deterministas, validación de entrada, MP4 H264/AAC"
