# GPTFrames

GPTFrames is a C++17 video compositor extended with native Neo Lite speech inference and an MCP tool server for controlled image search, narration and video rendering.

## Components

- Renderer: inherits HyperFrames Visual v2, JSON scene composition, keyframes, text, PNG/JPEG, shapes, transitions, optional Chromium HTML capture, AVI and H.264/AAC MP4.
- Voice: Neo Lite from the pinned MIT-licensed Neo-TTS Git submodule. Supports ONNX/NEO single-speaker Piper VITS models; creates 24kHz mono WAV for the renderer.
- MCP service: Python standard library, stdio and authenticated HTTP, with create/validate/render, voice synthesis, Wikimedia Commons and Openverse search and safe Commons download with attribution.
- Video engine is buildable on Linux and Windows x64 from Linux. Neo Lite runtime is tested on Linux; a Windows Neo Lite distribution is not yet included.

## Build the video engine

Clone with submodules:

    git clone --recurse-submodules https://github.com/ThowiLabs/GPTFrames.git
    cd GPTFrames

Install C++17, CMake, Python3 and MinGW-w64 for Windows builds, then:

    bash scripts/fetch-html-deps.sh
    bash scripts/fetch-codecs.sh
    python3 scripts/embed_fonts.py
    cmake -S . -B build/gptframes-linux -DCMAKE_BUILD_TYPE=Release -DHFC_HTML_LAYOUT=ON -DHFC_NATIVE_MP4=ON -DHFC_BROWSER_CAPTURE=ON
    cmake --build build/gptframes-linux -j3
    build/gptframes-linux/hyperframes-cpp-native --project render examples/json-showcase-v2/showcase-v2.json video.mp4

Windows: cross-compile with CMAKE_SYSTEM_NAME=Windows and x86_64-w64-mingw32-gcc/g++.

## Optional Neo Lite speech

On Debian/Ubuntu install espeak-ng, libespeak-ng-dev, libssl-dev, libjson-c-dev, libzstd-dev, cmake, C++17 and curl.

    bash scripts/build-neo-lite-linux.sh

Download a properly licensed Piper Spanish ONNX single-speaker voice with matching JSON into models/ (models are excluded from Git). Example model: es_ES-davefx-medium, listed at https://huggingface.co/rhasspy/piper-voices/tree/main/es/es_ES/davefx/medium

    build/gptframes-linux/hyperframes-cpp-native --voice synth --model models/es_ES-davefx-medium.onnx --text-file examples/gptframes/narracion.txt --out voz-24000.wav --neo-lite "$PWD/scripts/neo-lite-local.sh"

The C++ voice bridge invokes Neo Lite directly without a shell and checks/resamples WAV to 24000 Hz 16-bit mono. Model licenses, audio rights and voice consent must be checked separately. See docs/NEO_LITE.md.

## MCP server

Local MCP (stdio, no Python packages required):

    export GPTFRAMES_ENGINE="$PWD/build/gptframes-linux/hyperframes-cpp-native"
    export GPTFRAMES_NEO_LITE="$PWD/scripts/neo-lite-local.sh"
    python3 mcp/server.py --transport stdio

Remote MCP is an optional authenticated HTTP transport bound to 127.0.0.1. It requires GPTFRAMES_MCP_TOKEN (a long secret) and HTTPS reverse proxy/tunnel. A separate registration/authentication step is required by the external MCP client. OAuth and GET SSE are not yet implemented; compatibility with client-specific connection flows is not guaranteed. Never expose the write tools without authentication. See docs/MCP.md.

Image sources: Wikimedia Commons metadata-driven downloads with attribution sidecars; Openverse search results are discovery only, not automatically imported. Review third-party media licenses. See docs/IMAGE_SOURCES.md.

## Video Full HD con voz Capybara (243 segundos)

Ejemplo educativo Go vs. Rust vs. C++: 13 capítulos, 1920×1080 a 24 fps, fotos/logotipos atribuidos de Commons, tabla de comparación y narraciones reales de Capybara compilada a archivo NEO mediante Neo Compiler y sintetizada con Neo Lite. Se exporta mediante la pipeline HTML/Chromium/FFmpeg 16:9 que extiende el compositor vertical original. Ver [docs/CAPYBARA_1080P_VIDEO.md](docs/CAPYBARA_1080P_VIDEO.md) para reproducción y metodología. El modelo y los archivos generados se guardan localmente, fuera de Git.

## Túnel Gradio nativo Go con OAuth para ChatGPT

Se adaptaron componentes del propio [kagssh-go](https://github.com/ThowiLabs/kagssh-go) al módulo Go en `tunnel/`, que usa Gradio FRP nativo TLS/yamux y autorización OAuth con PIN/PKCE antes de exponer el MCP:

    cd tunnel && go test ./... && go build -o ../build/gptframes-tunnel-go ./cmd/gptframes-tunnel
    cd .. && ./build/gptframes-tunnel-go --port 18885 --backend-port 18886

La descarga del documental Full HD también tiene servidor nativo Go opcional con soporte HTTP Range local:

    cd tunnel && go build -o ../build/gptframes-media-go ./cmd/gptframes-media
    cd .. && ./build/gptframes-media-go --port 18989 --root examples/go-rust-cpp-1080p/public

El servidor de video es **separado** del gateway MCP OAuth. Algunos proxies Gradio no preservan las cabeceras Range, lo cual puede limitar los saltos de reproducción hasta que se descargue suficiente video.

La URL temporal tiene formato https://subdominio.gradio.live/mcp. Se requiere registrar el MCP desde ChatGPT y completar la autorización OAuth; no se enlaza automáticamente la cuenta. El archivo de PIN es privado (0600). [Documentación y limitaciones](docs/GO_GRADIO_MCP.md).

## Documentary v2: Capybara epoch 268 / step 110398 and live motion

The Full HD Go/Rust/C++ documentary has a second version based on the **latest numeric checkpoint** from HirCoir/piper-checkpoint-es-mx-capybara (epoch=268-step=110398.ckpt). Neo TTS native C++ exports the exact checkpoint to ONNX, then Neo Compiler packages a Lite-compatible .neo; Neo Lite synthesizes every Spanish narration chapter. The checkpoint/model binaries are downloaded by the authenticated owner and are **not committed**. More steps do not objectively guarantee better quality; this is simply the most-trained checkpoint available.

The updated 243s 1920x1080 pipeline animates independent transparent layers (image cards, titles, feature boxes, individual comparison-table rows), plus live subtitles and parallax. It is no longer a static slideshow. See [docs/CAPYBARA_LATEST_MOTION.md](docs/CAPYBARA_LATEST_MOTION.md) for rebuild details and voice/music licensing.

**Music:** the prior generated soundtrack was removed. The official YouTube Audio Library requires a logged-in YouTube Studio session, unavailable in this sandbox. We do not rip tracks or substitute an unverified track. The new mix is narration-only until the owner supplies a permitted music file downloaded in Studio; the mixer accepts `--youtube-audio-library /path/to/track.mp3` for that case.

## Tests

    python3 -m unittest discover -s tests -p test_gptframes.py -v
    bash tests/json-compositor.sh
    bash tests/json-showcase-v2.sh
    bash tests/remix-smoke.sh

## Licensing

Core source is Apache-2.0 (LICENSE); Neo-TTS is an MIT submodule. Codec dependencies, ONNX Runtime, voice models and image licenses have their own obligations. This remains a prototype, not a complete video editor or an implementation of the React Remotion API.
