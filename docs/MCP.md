# GPTFrames MCP

mcp/server.py uses Python's standard library and provides JSON-RPC 2.0 MCP version 2025-06-18 via newline-delimited stdio or sessionless Streamable HTTP JSON responses.

## Tools

list_projects, create_project, validate_project, preview_frame, render_video, synthesize_voice, search_images, download_commons_image.

The project directory is controlled with GPTFRAMES_PROJECTS, default ./projects. Project names are validated slugs. TTS models are limited to GPTFRAMES_MODELS, default ./models. The compositor executable is GPTFRAMES_ENGINE and Neo Lite launcher is GPTFRAMES_NEO_LITE.

Run locally:

    python3 mcp/server.py --transport stdio

For a private HTTP reverse proxy, configure GPTFRAMES_MCP_TOKEN with a long random bearer token and run:

    python3 mcp/server.py --transport http --port 8769

The HTTP server binds to 127.0.0.1 and requires authorization for POST /mcp; GET /health is a minimal status check. Only enable HTTPS access through a protected tunnel or reverse proxy. Basic bearer auth is implemented; OAuth, session persistence, SSE subscriptions, per-user ACLs and queued jobs are not. Registration with a compatible MCP client is a separate user-controlled process. Do not publish tokens.

Wikimedia downloads allow only raster PNG/JPEG from Wikimedia's CDN, enforce 18MB max, block untrusted redirects and produce .attribution.json with artist, license and source metadata. Openverse is search-only. Search results are not a grant of rights to reuse images commercially.

Server outputs are informational paths within the sandbox. For remote video delivery, provide a separate authenticated artifact channel or signed URLs.
