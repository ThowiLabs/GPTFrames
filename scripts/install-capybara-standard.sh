#!/usr/bin/env bash
# Capybara es-MX CKPT => ONNX => .neo STANDARD.
# Fetch via lilith-hf wrapper; never expose credentials in CLI args/logs.
set -Eeuo pipefail
umask 077
project="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
repo='HirCoir/piper-checkpoint-es-mx-capybara'
filename='epoch=268-step=110398.ckpt'
src_dir="$project/build/runtime/capybara-source"
staging="$project/build/runtime/capybara-build"
dest="$project/models/Capybara_es_MX_epoch268_step110398_standard.neo"
exporter='/workspace/neo-tts/build-infer/neo-tts'
compiler="$project/build/neo-lite/neo-compiler"
synth="$project/build/neo-lite/neo-lite"

for binary in "$exporter" "$compiler" "$synth"; do
  test -x "$binary" || { echo "MISSING_EXECUTABLE: $binary" >&2; exit 3; }
done
mkdir -p "$src_dir" "$staging"
test ! -e "$dest" || { echo "TARGET_EXISTS: $dest" >&2; exit 4; }

if test ! -s "$src_dir/$filename" || test ! -s "$src_dir/config.json"; then
  command -v lilith-hf >/dev/null || { echo "lilith-hf unavailable" >&2; exit 2; }
  echo "FETCHING via lilith-hf hf download..."
  lilith-hf hf download "$repo" "$filename" config.json --local-dir "$src_dir" --max-workers 2
fi
test -s "$src_dir/$filename"
test -s "$src_dir/config.json"
echo "INSPECTING checkpoint..."
"$exporter" vits-ckpt-inspect --checkpoint "$src_dir/$filename" --config "$src_dir/config.json"

# Do not leave partial output visible to the MCP models scanner.
echo "EXPORTING ONNX via native exporter..."
(cd /workspace/neo-tts && "$exporter" export-onnx --checkpoint "$src_dir/$filename" --config "$src_dir/config.json" --out "$staging/capybara.onnx" --progress)
test -s "$staging/capybara.onnx"
test -s "$staging/capybara.onnx.json"

echo "COMPILING .neo with --profile standard..."
"$compiler" build \
  --source "$staging/capybara.onnx" \
  --config "$staging/capybara.onnx.json" \
  --name 'Capybara es-MX (epoch 268, step 110398)' \
  --out "$staging/capybara-standard.neo" \
  --profile standard \
  --author 'HirCoir (checkpoint source)' \
  --license 'CC-BY-NC-4.0' \
  --description 'Piper Spanish (Mexico) Capybara, checkpoint epoch=268 step=110398, standard package'
echo "VERIFYING signature/profile..."
"$compiler" verify --model "$staging/capybara-standard.neo" | tee "$staging/verification.txt"
grep -q 'profile=standard' "$staging/verification.txt"

echo "SMOKE TESTING speech synthesis..."
"$synth" infer --model "$staging/capybara-standard.neo" --text 'Hola, esta es una prueba de la voz Capybara en español.' --out "$staging/capybara-test.wav" --device cpu
test -s "$staging/capybara-test.wav"

# Atomic, exclusive publish: newly introduced models become visible to MCP watcher.
ln -- "$staging/capybara-standard.neo" "$dest"
chmod 0644 "$dest"
echo "MODEL_INSTALLED=$dest"
ls -lh "$dest"
