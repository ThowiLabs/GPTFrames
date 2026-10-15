# GPTFrames 15 de octubre de 2026: Capybara NEO, documental Full HD, túnel OAuth de KagSSH-Go

## Objetivo
Con acceso HF del usuario, compilar Capybara ONNX (exportación de checkpoint Piper) a archivo NEO y usarlo para narrar documental de Go/Rust/C++ mínimo 3 minutos horizontal 1080p, con tabla comparativa y sin contador de páginas. Reutilizar Gradio nativo Go y OAuth de KagSSH-Go para enlazar MCP a ChatGPT.

## Hechos
Rama de trabajo feature/capybara-kagssh-mcp-1080p, original GPTFrames main intacto hasta integración. El CLI HF estaba autenticado en sesión privada como HirCoir, HF_TOKEN no existía en entorno; se descargaron archivos de repos gated/public sin imprimir credenciales. `neo-compiler build --profile lite` compiló es_MX-capybara.onnx a capybara.neo 31,261,591 bytes y verify confirmó estructura/firma. Se sintetizaron desde capybara.neo 13 WAV reales mediante Neo Lite CPU, se ajustaron por capítulo y mezclaron con música procedural original.

Storyboards de 13 escenas/243s; HTML a PNG 1920×1080 con Chromium, clips animados mediante zoompan y fade FFmpeg, MP4 H.264/AAC final 1920×1080 24fps 5832 frames, duración 243.000000s, tamaño 32,972,611 bytes. FFmpeg decodificó el video completo sin errores. Tabla editorial compara 7 dimensiones; no contiene paginación. Recursos con fuentes de Wikimedia Commons, incluyendo fotos atribuidas con originales de Unsplash, más búsqueda Openverse ya disponible en MCP. Video público temporal https://9409cf3846cd3d9be6.gradio.live/ y descarga MP4 en esa raíz.

Túnel Go: se clonó https://github.com/ThowiLabs/kagssh-go HEAD bba20575d85473b6032f0c525ffaf877a2fc7c56 y se copiaron internal/gradiolive, frpmsg, oauth, pinauth, securefs a módulo go en tunnel, renombrando las rutas internas. Se implementó cmd/gptframes-tunnel: FRP nativo Gradio TLS/yamux, prueba aleatoria HTTPS antes de anunciar servicio, puente OAuth PKCE/S256 con registro dinámico y PIN privado 0600; proxy autenticado hacia MCP Python local usando token efímero y comprobación de puerto ocupado. Go tests originales de OAuth/Gradio pasaron. Túnel público https://89ee9bbcec8e846a8f.gradio.live/mcp comprobado GET /health 200, POST /mcp sin bearer 401, metadata OAuth HTTPS PKCE/DCR 200. Registro real desde UI ChatGPT pendiente de acción del usuario.

## Decisiones
NO subir HF token, modelo ONNX/.neo, PIN ni estado OAuth a Git. .gitignore dedicado. Licencias imágenes y voz conservar. No sustituir voz por otra: Capybara verdadero. Mantener CGI y C++ vertical anterior; vídeo 16:9 usa pipeline Chromium+FFmpeg documentada como tal.

## Archivos
README.md, docs/CAPYBARA_1080P_VIDEO.md, docs/GO_GRADIO_MCP.md, tunnel/*, examples/go-rust-cpp-1080p/{storyboard.py,make_slides.py,mix_audio.py,render_video.py,assets-sources.json,assets/}, modelos bajo voice/hf ignorados, output/ ignorado.

## Pendiente
Hacer commit/push GitHub de feature. Probar autenticación OAuth desde cliente ChatGPT real con PIN privado. Publicación Gradio temporal. Probar Windows en máquina real. Para soporte horizontal 16:9 nativo, refactorizar framebuffer del compositor JSON C++.

### Mejoras de entrega
El sitio de video 1080p se movió de Python http.server a cmd/gptframes-media Go; rango local 206 comprobado pero túnel Gradio del reproductor elimina Range (HTTP 200), documento de límite. Tests Go nuevos: archivo PIN privado y estable, puente fail-closed antes de OAuth, rechazo de puerto ocupado. Video de 243s por URL temporal distinta del MCP OAuth.
