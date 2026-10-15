#!/bin/sh
set -eu
engine="$1"
root="$2"
if ! "$engine" --browser-check >/dev/null 2>&1; then
  echo "SKIP: Chromium not available"
  exit 0
fi
if ! command -v ffmpeg >/dev/null 2>&1; then
  echo "SKIP: ffmpeg not installed"
  exit 0
fi
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT HUP INT TERM
html="$root/animaciones/presets/breaking-news-ticker.html"
"$engine" --browser-widget-frame "$html" 0.6 "$tmp/early.png" 848 480 >"$tmp/log" 2>&1
"$engine" --browser-widget-frame "$html" 2.5 "$tmp/later.png" 848 480 >>"$tmp/log" 2>&1
# Compare only the cyan news strip, not the blinking red live indicator.
for t in early later; do
  ffmpeg -nostdin -v error -y -i "$tmp/$t.png" -vf "crop=795:32:25:252,format=gray" -f rawvideo "$tmp/$t.bin"
done
if cmp -s "$tmp/early.bin" "$tmp/later.bin"; then
  echo "FAIL: news ticker did not move inside video time range" >&2
  exit 1
fi
echo "PASS: scrolling ticker strip moves, independent of blinking status icon"
