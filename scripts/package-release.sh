#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
version="${1:-v1.0.0}"
if [[ "$version" != v[0-9]* || "$version" == *[!a-zA-Z0-9._-]* ]]; then
  echo "Version must start with v and contain only letters, digits, dots, underscores or hyphens" >&2; exit 2
fi
linux="build/native-linux/hyperframes-cpp-native"
windows="build/native-windows/hyperframes-cpp-native.exe"
linux_mcp="build/native-linux/gptframes-mcp"
windows_mcp="build/native-windows/gptframes-mcp.exe"
if [[ ! -x "$linux" || ! -s "$windows" || ! -x "$linux_mcp" || ! -s "$windows_mcp" ]]; then
  echo "Build both native renderers and C++ MCP targets before packaging" >&2; exit 2
fi
mkdir -p dist
destination="$PWD/dist"
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT
skill=".claude/skills/hyperframes-cpp"
source_copy() {
  local root="$1"
  mkdir -p "$root/skills/hyperframes-cpp" "$root/bin" "$root/docs" \
    "$root/assets/fonts" "$root/examples/json-showcase-v2" "$root/licenses"
  cp -R "$skill/." "$root/skills/hyperframes-cpp/"
  cp LICENSE THIRD_PARTY_NOTICES.md README.md "$root/"
  cp docs/*.md "$root/docs/"
  cp assets/fonts/LICENSE.txt "$root/assets/fonts/LICENSE.txt"
  cp vendor/httplib/LICENSE "$root/licenses/cpp-httplib-MIT.txt"
  cp examples/json-showcase-v2/showcase-v2.json "$root/examples/json-showcase-v2/"
  cp -R examples/json-showcase-v2/assets "$root/examples/json-showcase-v2/"
  cp scripts/install-skill.sh "$root/install-skill.sh"
  chmod +x "$root/install-skill.sh"
}
label="hyperframes-cpp-$version"
linux_root="$work/$label-linux-x64"
source_copy "$linux_root"
cp "$linux" "$linux_root/bin/hyperframes-cpp-native"
cp "$linux_mcp" "$linux_root/bin/gptframes-mcp"
chmod +x "$linux_root/bin/hyperframes-cpp-native" "$linux_root/bin/gptframes-mcp"
tar -C "$work" -czf "$destination/$label-linux-x64.tar.gz" "$label-linux-x64"
win_root="$work/$label-windows-x64"
source_copy "$win_root"
cp "$windows" "$win_root/bin/hyperframes-cpp-native.exe"
cp "$windows_mcp" "$win_root/bin/gptframes-mcp.exe"
(cd "$work" && zip -qr "$destination/$label-windows-x64.zip" "$label-windows-x64")
skill_root="$work/$label-agent-skill"
mkdir -p "$skill_root/hyperframes-cpp"
cp -R "$skill/." "$skill_root/hyperframes-cpp/"
(cd "$work" && zip -qr "$destination/$label-agent-skill.zip" "$label-agent-skill")
(cd dist && sha256sum "$label-linux-x64.tar.gz" "$label-windows-x64.zip" "$label-agent-skill.zip" > SHA256SUMS.txt)
ls -lh dist/
echo "RELEASE_PACKAGES_READY $version"
