#!/bin/sh
set -eu
bin="$1"
root="$2"
if ! "$bin" --browser-check >/dev/null 2>&1; then
 echo "SKIP: Chromium unavailable"
 exit 0
fi
dir="$(mktemp -d)"
trap 'rm -rf "$dir"' EXIT HUP INT TERM
html="$root/animaciones/catalogo.html"
# One-shot keyframes must be freshly seeked from their local chapter origin.
# Each pair belongs to the SAME chapter: any difference proves actual motion.
for pair in '2.60 3.60 checkmark' '5.07 6.07 padlock' '7.55 8.45 notification'
do
 set -- $pair
 "$bin" --browser-widget-frame "$html" "$1" "$dir/$3-a.png" 848 480 >/dev/null 2>"$dir/stderr.log"
 "$bin" --browser-widget-frame "$html" "$2" "$dir/$3-b.png" 848 480 >/dev/null 2>>"$dir/stderr.log"
 if cmp -s "$dir/$3-a.png" "$dir/$3-b.png";then
  echo "FAIL: no animation movement in $3" >&2
  exit 1
 fi
done
echo "PASS: checkmark, padlock and notification animate at local chapter time"
