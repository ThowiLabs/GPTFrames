# GPTFrames: connect to ChatGPT

GPTFrames is a C++17 MCP server. Its public connection method is an HTTPS URL and a PIN window opened during ChatGPT authorization.

## User connection

1. In ChatGPT, add a custom MCP server using the public URL ending in /mcp.
2. Choose OAuth with automatic client registration (DCR). Leave manual client credentials empty.
3. When the GPTFrames window opens, enter your private PIN and select Conectar GPTFrames.

After confirmation, ChatGPT receives connection credentials automatically. The user does not copy a bearer token or API key. A fixed GPTFRAMES_MCP_TOKEN is no longer accepted by the HTTP server.

## Build

    cmake -S . -B build/native-linux -DHFC_MCP=ON
    cmake --build build/native-linux --target gptframes-mcp -j2

Linux requires libcurl and OpenSSL libcrypto development packages for building. Windows uses system WinHTTP and BCrypt. The native video engine must also be built to validate, preview, synthesize voice or render MP4.

## Operator configuration

| Variable | Meaning |
| --- | --- |
| GPTFRAMES_OAUTH_PIN_FILE | Private file containing the PIN. Requires a regular file, no symlink, mode 600 on Linux. |
| GPTFRAMES_OAUTH_PIN | Optional PIN supplied by the operator environment instead of a file. Never printed by the server. |
| GPTFRAMES_PUBLIC_URL | Exact public HTTPS origin, without a path; endpoints are derived from it. |
| GPTFRAMES_PUBLIC_URL_FILE | Private file supplying the origin after a managed tunnel starts. Discovery stays unavailable until configured. The origin is locked after first successful loading. |
| GPTFRAMES_OAUTH_STATE | Private file preserving DCR client registrations across server restarts. |
| GPTFRAMES_ROOT | Root containing templates. |
| GPTFRAMES_PROJECTS | Projects directory; defaults to ROOT/projects. |
| GPTFRAMES_MODELS | Local voice model directory; defaults to ROOT/models. |
| GPTFRAMES_ENGINE | Native engine, discovered beside the MCP executable by default. |
| GPTFRAMES_NEO_LITE | Installed native Neo Lite executable or native launcher. |
| GPTFRAMES_ALLOWED_ORIGINS | Exact extra allowed origins, comma-separated. |

The PIN accepts 6-128 characters. Generate a long, unpredictable PIN; a temporary PIN can be provided privately to the owner. Never commit it or place it in a URL. The state directory requires mode 700 on Linux. Windows operators must protect private files with their account ACL.

    export GPTFRAMES_OAUTH_PIN_FILE=/private/path/gptframes-pin
    export GPTFRAMES_PUBLIC_URL=https://your-public-host.example
    export GPTFRAMES_OAUTH_STATE=/private/path/oauth-clients.json
    build/native-linux/gptframes-mcp --port 18769 --root "$PWD"

HTTP listens only on 127.0.0.1. Place the authorized HTTPS tunnel or reverse proxy in front. HTTP is the default executable mode. New deployments fail closed if the PIN or public URL configuration is missing.

## Authorization behavior

The C++ server publishes protected-resource and OAuth metadata. It supports public-client dynamic registration, authorization codes with PKCE S256, exact registered ChatGPT callbacks, issuer identification and resource-bound opaque access tokens.

The supported callbacks are the stable https://chatgpt.com/connector_platform_oauth_redirect and callback-specific https://chatgpt.com/connector/oauth/CALLBACK_ID forms. The server does not advertise CIMD. Select DCR in the connection settings.

The consent page displays the requesting client and callback. Its one-use transaction expires after five minutes; codes expire after 90 seconds. PIN guesses are rate limited. The PIN form requires a session-bound CSRF value matching its Secure, HttpOnly, SameSite=Lax cookie; it cannot be embedded in frames. Like kaggle-mcp, this browser form does not rely on Origin, which can be null under no-referrer or rewritten by a proxy. MCP API requests retain strict Origin checks. After approval, an HTML refresh and a fallback link return to the exact registered ChatGPT callback, avoiding form-action redirect restrictions. Registration and authorization state have capacity limits.

Access expires after one hour. When offline_access is requested, refresh tokens rotate, expire after at most 24 hours and revoke their family if reused. Registered clients persist; authorization transactions and access/refresh credentials remain in memory. A server restart revokes active credentials, so ChatGPT may ask to connect again. A changed temporary tunnel URL also requires a new connection.

GET /health and GET / show only minimal public status. POST /mcp requires a valid OAuth-issued credential; there is no static-token bypass. Missing or expired credentials return a discovery challenge. Bodies are bounded to 1,000,000 bytes, supplied Origins are checked and request bodies/credentials are not logged.

This is a single-owner PIN authorization service. Per-user accounts, enterprise identity, multi-user ACLs and permanent hosting are separate production work.

## ChatGPT action discovery

The connection follows the behavior reviewed in ThowiLabs/kaggle-mcp at 8ed2d2b, implemented here in C++.

The HTTP server supports legacy initialize (2025-03-26, 2025-06-18, 2025-11-25) and server/discover for 2026-07-28. Modern requests validate Mcp-Method, Mcp-Name for tool calls, and protocol/client-capabilities metadata. Discovery and tool responses use the complete result format and private cache scope. Unsupported versions return -32022 with available versions.

## Tools

| Tool | Behavior |
| --- | --- |
| list_projects | List project slugs. |
| create_project | Copy the editable showcase and assets to a new project. |
| validate_project | Validate with the native renderer. |
| preview_frame | Render a PNG at the requested second. |
| render_video | Render native MP4; current MCP limit is 60 seconds. |
| synthesize_voice | Run Neo Lite and attach validated mono 24 kHz WAV atomically. |
| search_images | Wikimedia Commons or Openverse discovery with attribution metadata. |
| download_commons_image | Import a Commons PNG/JPEG and attribution sidecar. |

All tools declare the gptframes OAuth scope. Wrong types, unknown arguments, invalid slugs and out-of-range values are rejected. Tool mutations are serialized. Process arguments are passed directly without a shell. Timeouts terminate the subprocess group; failed inference leaves the prior project intact.

Provider requests use verified HTTPS and exact host allowlists. Redirects and response sizes are checked. Openverse remains discovery-only. Returned media paths are internal filesystem paths; authenticated remote artifact delivery is pending.

## Regression tests

    ctest --test-dir build/native-linux --output-on-failure

Compiled C++ tests exercise the PIN page, registration, PKCE, callback and audience validation, issuer/state, cancellation, expiry, replay prevention, refresh rotation, PIN throttling, HTTP behavior and the native tools. They also preserve an internal stdio test mode, selected explicitly with --transport stdio; this is not the public ChatGPT connection flow.

Reference for interoperability: https://developers.openai.com/plugins/build/auth
