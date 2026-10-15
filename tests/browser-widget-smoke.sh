#!/usr/bin/env sh
set -eu
binary="$1"
root="$2"
if ! "$binary" --browser-check >/dev/null 2>&1; then
  echo "SKIP: browser is not installed"
  exit 0
fi
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT HUP INT TERM
source="$root/examples/browser-first-uiverse/loader.html"
"$binary" --browser-widget-frame "$source" 0 "$tmp/zero.png" 848 480 >/dev/null 2>"$tmp/browser.log"
"$binary" --browser-widget-frame "$source" 0.7 "$tmp/mid.png" 848 480 >/dev/null 2>>"$tmp/browser.log"
test -s "$tmp/zero.png"
test -s "$tmp/mid.png"
if cmp -s "$tmp/zero.png" "$tmp/mid.png"; then
  echo "CSS keyframes were not advanced by CDP" >&2
  exit 1
fi
"$binary" --browser-widget-render "$source" "$tmp/widget.mp4" 0.5 8 848 480 >/dev/null 2>>"$tmp/browser.log"
test -s "$tmp/widget.mp4"
echo "PASS: CSS timeline seek and H.264/AAC preview"
