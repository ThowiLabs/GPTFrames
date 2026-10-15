# GPTFrames native C++ MCP

gptframes-mcp is a C++17 executable. It implements JSON-RPC 2.0 with protocol version 2025-06-18 through newline-delimited stdio or stateless Streamable HTTP JSON responses.

## Build and run

    cmake -S . -B build/native-linux -DHFC_MCP=ON
    cmake --build build/native-linux --target gptframes-mcp -j2
    build/native-linux/gptframes-mcp --transport stdio

The video engine must also be built for validate/preview/render/voice tools. Build it with HFC_NATIVE_MP4=ON to export MP4. HFC_MCP=OFF produces the renderer without the MCP service.

The root defaults to the source checkout used during development, or a packaged root containing examples/json-showcase-v2 beside bin/. Override it with --root or GPTFRAMES_ROOT when relocating the application.

## Preserved tools

| Tool | Behavior |
| --- | --- |
| list_projects | List project slugs. |
| create_project | Copy the editable showcase and its assets to a new project. |
| validate_project | Validate via the native C++ renderer. |
| preview_frame | Render a PNG at a requested second. |
| render_video | Render native MP4, preserving the existing 60-second MCP limit. |
| synthesize_voice | Run native Neo Lite through the renderer and atomically attach validated mono 24kHz WAV. |
| search_images | Discover Wikimedia Commons or Openverse resources with attribution metadata. |
| download_commons_image | Import a Commons PNG/JPEG and its attribution sidecar. |

The argument names and response fields used by the previous service are retained. Wrong types, unknown arguments, booleans used as numeric arguments, invalid slugs and out-of-range values are rejected. The native service serializes tool mutations to avoid lost project updates.

## Configuration

| Variable | Meaning |
| --- | --- |
| GPTFRAMES_ROOT | Root containing the packaged templates. |
| GPTFRAMES_PROJECTS | Projects directory; default ROOT/projects. |
| GPTFRAMES_MODELS | Model directory; default ROOT/models. |
| GPTFRAMES_ENGINE | Native engine; discovered beside gptframes-mcp by default. |
| GPTFRAMES_NEO_LITE | Native Neo Lite executable or native launcher. |
| GPTFRAMES_MCP_TOKEN | HTTP bearer token, at least 24 characters. |
| GPTFRAMES_ALLOWED_ORIGINS | Comma-separated exact additional HTTP Origin values. |

The service invokes the renderer with separate argv entries and captured output, never through a command shell. Linux uses posix_spawn and process groups; Windows uses CreateProcessW and a kill-on-close job. Timeouts terminate the child group. Inference failure removes partial outputs and leaves the prior project intact.

## HTTP transport

    build/native-linux/gptframes-mcp --transport http --port 8769

The service binds to 127.0.0.1, requires bearer authentication on POST /mcp, bounds message bodies to 1,000,000 bytes, validates supplied origins and suppresses request logging. GET /health returns minimal public status. MCP notifications receive HTTP 202; unsupported GET /mcp receives 405. JSON-RPC errors and tool execution errors remain distinct.

Use a protected HTTPS reverse proxy for remote clients. OAuth, session persistence, SSE subscriptions, queued render jobs and per-user ACLs are not introduced here.

## Networking and delivery

Linux provider requests use libcurl with certificate verification; Windows requests use WinHTTP. Queries and downloads use HTTPS and exact host allowlists. Redirects are validated before the next request, response sizes are bounded, and Commons downloads accept PNG/JPEG magic signatures and preserve metadata. Openverse is discovery-only.

Returned paths are internal filesystem paths, as in the previous interface. A client needs an authenticated artifact channel to retrieve those outputs remotely.

## Native tests

    ctest --test-dir build/native-linux --output-on-failure -R mcp

Tests exercise real stdio/HTTP transport, bearer/origin handling, message limits, the native renderer, filesystem confinement, mocked provider responses, download attribution, process timeout and voice rollback with a native test fixture. Fixture inference is not a voice-quality test.
