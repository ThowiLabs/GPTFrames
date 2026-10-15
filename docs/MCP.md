# GPTFrames MCP

mcp/server.py uses Python's standard library and provides JSON-RPC 2.0 MCP version 2025-06-18 via newline-delimited stdio or sessionless Streamable HTTP JSON responses.

## Reglas de producción obligatorias: movimiento y pronunciación

La skill y la interfaz MCP comparten reglas en mcp/production.py. **Antes de crear un video**, llama a get_production_guidelines.

- **No producir diapositivas estáticas por defecto.** Animar al menos una capa real por escena (ideal 2+ capas escalonadas con entradas, movimiento, opacidad, escala, máscaras, tablas progresivas). analyze_motion reporta por escena el número de capas que CAMBIAN efectivamente de pose y lista las escenas estáticas. validate_project conserva la validación nativa y añade ese informe; render_video rechaza escenas inmóviles salvo allow_static=true solicitado de forma explícita por el usuario. La auditoría es sobre keyframes JSON, no sobre composiciones HTML/GSAP externas; para ellas inspecciona fotogramas.
- **Escribir para TTS como se pronuncia.** Dos textos independientes: display_text con el nombre/sintaxis correctos, y text hablado adaptado a la voz. Ejemplos: Go = «gou», Rust = «rast», C++ = «ce más más», HirCoir = «Jírcoir». prepare_narration propone adaptaciones **sin cambiar el texto visible**; NO sintetiza ni garantiza sonido correcto. Pasar el texto fonético revisado a synthesize_voice.text y el escrito correctamente a synthesize_voice.display_text. El servidor conserva los textos por separado en archivos .txt y .display.txt; no crea subtítulos. Escuchar el WAV y medir su duración antes de exportar.
- **Respetar el encargo.** Resolución/FPS y relación de aspecto solicitados (para documental horizontal reciente: 1920×1080 a 30 FPS). Marca, subtítulos, música, CTA y contador de páginas según usuario; no insertar elementos no solicitados. Usar recursos con licencias y créditos verificados.
- **Control final:** captura de inicio/mitad/final; prueba de pronunciación con la voz seleccionada; análisis de transiciones y movimiento; ffprobe/decodificación y comprobación de salida real.

Ejemplo en MCP JSON-RPC: llamar tools/call con name=prepare_narration, arguments={"display_text":"Go, Rust y C++. Suscríbete a HirCoir"} devuelve display_text intacto y suggested_spoken_text para revisión auditiva. Llamar analyze_motion con project=nombre informa static_scenes. Para renderizar: render_video requiere project y opcional seconds/allow_static (false por defecto).

Documentación de skills: .claude/skills/gptframes-video/SKILL.md (específica) y .claude/skills/hyperframes-cpp/SKILL.md (motor clásico).

## Tools

get_production_guidelines, prepare_narration, analyze_motion, list_projects, create_project, validate_project, preview_frame, render_video, synthesize_voice, search_images, download_commons_image.

The project directory is controlled with GPTFRAMES_PROJECTS, default ./projects. Project names are validated slugs. TTS models are limited to GPTFRAMES_MODELS, default ./models. The compositor executable is GPTFRAMES_ENGINE and Neo Lite launcher is GPTFRAMES_NEO_LITE.

Run locally:

    python3 mcp/server.py --transport stdio

For a private HTTP reverse proxy, configure GPTFRAMES_MCP_TOKEN with a long random bearer token and run:

    python3 mcp/server.py --transport http --port 8769

The HTTP server binds to 127.0.0.1 and requires authorization for POST /mcp; GET /health is a minimal status check. Only enable HTTPS access through a protected tunnel or reverse proxy. Basic bearer auth is implemented; OAuth, session persistence, SSE subscriptions, per-user ACLs and queued jobs are not. Registration with a compatible MCP client is a separate user-controlled process. Do not publish tokens.

Wikimedia downloads allow only raster PNG/JPEG from Wikimedia's CDN, enforce 18MB max, block untrusted redirects and produce .attribution.json with artist, license and source metadata. Openverse is search-only. Search results are not a grant of rights to reuse images commercially.

Server outputs are informational paths within the sandbox. For remote video delivery, provide a separate authenticated artifact channel or signed URLs.
