#!/bin/sh
set -eu
engine="$1"
project="$2/examples/motion-studio-native/project.json"
temp_dir="$(mktemp -d)"
trap 'rm -rf "$temp_dir"' EXIT INT TERM
"$engine" --project validate "$project" >/dev/null
"$engine" --project frame "$project" "$temp_dir/early.png" 13 >/dev/null
"$engine" --project frame "$project" "$temp_dir/late.png" 14 >/dev/null
if cmp -s "$temp_dir/early.png" "$temp_dir/late.png"; then
 echo "Lottie frames must change with timeline" >&2
 exit 1
fi
echo "PASS Lottie plays on native C++ timeline"
