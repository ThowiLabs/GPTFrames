# GPTFrames

GPTFrames is a C++17 video compositor with a native C++ MCP service, controlled installed-browser capture, Wikimedia/Openverse media discovery and optional native Neo Lite speech inference.

## Components

- Renderer: JSON scene composition, keyframes, text, PNG/JPEG, shapes, transitions, optional Chromium HTML/CSS/JavaScript capture, AVI and native H.264/AAC MP4.
- MCP: the compiled gptframes-mcp executable provides stdio and authenticated loopback HTTP. Its eight existing tool names and environment variables are preserved.
- Media clients: C++ with libcurl HTTPS on Linux and system WinHTTP on Windows. Commons downloads retain author, source and license sidecars; Openverse remains discovery-only.
- Voice: the pinned Neo-TTS submodule builds its native NEO_LITE_ONLY inference path. No interpreter is required.
- Build utilities and regression fixtures: C++ executables. CMake automatically embeds local fonts using a host C++ compiler, including during cross-compilation.

The engine and service use C/C++ libraries. Browser compositions remain HTML/CSS/JavaScript executed inside the installed browser under the C++ controller. Shell files only orchestrate builds and test commands. There are no project-owned .py files or interpreter calls in build/test scripts.

## Build

Clone with submodules:

    git clone --recurse-submodules https://github.com/ThowiLabs/GPTFrames.git
    cd GPTFrames

On Debian/Ubuntu install C/C++17, CMake, Git, curl, DejaVu fonts and libcurl development files:

    sudo apt install build-essential cmake git curl fonts-dejavu-core libcurl4-openssl-dev

Fetch optional native renderer dependencies, then build:

    sh scripts/fetch-html-deps.sh
    sh scripts/fetch-codecs.sh
    cmake -S . -B build/native-linux -DCMAKE_BUILD_TYPE=Release -DHFC_HTML_LAYOUT=ON -DHFC_NATIVE_MP4=ON -DHFC_BROWSER_CAPTURE=ON -DHFC_MCP=ON
    cmake --build build/native-linux -j2

CMake generates its font header under the build directory. Set HFC_FONT_DIR if DejaVuSans.ttf and DejaVuSans-Bold.ttf are stored elsewhere. Cross builds use HFC_HOST_CXX to select the host compiler for the font generator.

The renderer keeps its installed-browser mode. Chromium/Chrome/Edge must be installed to use that mode. JSON and raster modes do not require a browser. The native renderer does not invoke an external encoder executable.

## MCP service

Local MCP:

    build/native-linux/gptframes-mcp --transport stdio

Example client configuration:

    {
      "mcpServers": {
        "gptframes": {
          "command": "/absolute/path/GPTFrames/build/native-linux/gptframes-mcp",
          "args": ["--transport", "stdio", "--root", "/absolute/path/GPTFrames"]
        }
      }
    }

The engine is discovered beside the MCP executable. GPTFRAMES_ENGINE can select a different native engine. GPTFRAMES_ROOT, GPTFRAMES_PROJECTS, GPTFRAMES_MODELS and GPTFRAMES_NEO_LITE remain supported.

Authenticated HTTP:

    export GPTFRAMES_MCP_TOKEN='use-a-long-private-token-at-least-24-characters'
    build/native-linux/gptframes-mcp --transport http --port 8769

HTTP binds to 127.0.0.1. POST /mcp requires the bearer token. GET /health returns minimal status. HTTP clients that send an Origin must use a loopback origin or an exact origin configured in GPTFRAMES_ALLOWED_ORIGINS. Remote transport still requires a protected HTTPS reverse proxy. OAuth and SSE sessions are not added by this migration. See docs/MCP.md.

## Native voice inference

Install eSpeak NG, OpenSSL, json-c, zstd and their development headers, then:

    bash scripts/build-neo-lite-linux.sh

Place a licensed single-speaker Piper ONNX/NEO model and its matching configuration in models/. Native command:

    build/native-linux/hyperframes-cpp-native --voice synth --model models/voz.onnx --text-file narracion.txt --out voz.wav --neo-lite "$PWD/scripts/neo-lite-local.sh"

The bridge executes Neo Lite directly and validates/resamples its PCM output to mono 24kHz WAV. Voice models are not redistributed. See docs/NEO_LITE.md.

## Tests

    ctest --test-dir build/native-linux --output-on-failure
    bash tests/json-compositor.sh build/native-linux/hyperframes-cpp-native
    bash tests/json-showcase-v2.sh build/native-linux/hyperframes-cpp-native
    bash tests/smoke-linux.sh
    sh tests/browser-integration.sh build/native-linux/hyperframes-cpp-native

Unit and transport tests are compiled C++. The fixture generator is also C++. Network access is mocked in regression tests; live provider checks can be performed separately. Browser tests report a skip if no compatible browser is installed. ffprobe, when present, is an optional test probe.

## Windows

Install MinGW-w64 for Linux cross-compilation, then run sh scripts/build-native-windows.sh. The Windows MCP uses system WinHTTP; no libcurl DLL is required. Cross-compilation is not a substitute for runtime testing on Windows. Native speech dependencies have a separate distribution; see docs/NEO_LITE.md.

After building both platforms, `bash scripts/package-release.sh v0.2.0` creates renderer + MCP packages with the editable template and agent skill. Linux MCP requires system libcurl and its TLS/CA support; the static renderer remains independent of that library. Chromium and voice models are installed separately. The packaging command creates local archives and does not publish a GitHub release.

## Scope and licensing

This change replaces the interpreter-based service and utilities with C++; it preserves the existing rendering scope. JSON project limits, native audio formats, browser timing support and other renderer limitations are documented in docs/ARCHITECTURE.md and docs/ROADMAP.md.

Core source is Apache-2.0; Neo-TTS is an MIT submodule. Third-party C/C++ libraries, models, fonts, browser assets and media retain their own terms. See THIRD_PARTY_NOTICES.md.
