# Fecha
2026-10-15 UTC
# Objetivo
GPTFrames: compositor C++17 HyperFrames Visual v2 con Neo Lite TTS, búsqueda con atribución y servicio MCP.
# Decisiones tomadas
Repositorio separado /workspace/GPTFrames, main y remote privado ThowiLabs/GPTFrames, original HyperFrames intacto. Neo TTS importado como submódulo Git MIT fijado f41791d. Inferencia como backend C++ opcional, sin duplicar la biblioteca. MCP Python estándar con stdio y HTTP autenticado sin OAuth. No redistribuir modelos.
# Arquitectura actual
main.cpp incorpora CLI --voice synth; voice/neo_bridge.hpp ejecuta Neo Lite con argumentos separados, lee y transforma WAV PCM mono a 24k para el compositor JSON. mcp/server.py expone herramientas de creación/validación/render y síntesis/audio, búsqueda Wikimedia/Openverse y descarga Commons; mcp/media.py valida orígenes, tamaño y conserva atribución.
# Librerías usadas
Neo Lite/Neo-TTS MIT, ONNX Runtime 1.20.1, eSpeak NG, zstd, json-c, OpenSSL como backend opcional. Renderer original Apache-2.0 y dependencias existentes.
# Archivos importantes modificados
README.md, main.cpp, voice/neo_bridge.hpp, mcp/{server.py,media.py}, docs/{MCP.md,NEO_LITE.md,IMAGE_SOURCES.md}, tests/test_gptframes.py, scripts/build-neo-lite-linux.sh, scripts/neo-lite-local.sh, .gitignore, .gitmodules y tareas.
# Problemas encontrados
Neo Lite requiere ONNX Runtime y eSpeak NG: fueron instalados en sandbox y ONNX Runtime se obtuvo de release oficial. Wikimedia sirve miniaturas desde thumb.wikimedia.org, se amplió la lista de hosts permitidos. La primera inspección del MP4 se hizo mientras se renderizaba; se esperó finalización y luego pasó ffmpeg.
# Soluciones implementadas
Compilados GPTFrames C++ video Linux y Windows x64 desde Linux, Neo Lite Linux x64 con ONNX Runtime. Inferencia real español Piper modelo es_ES-davefx-medium produjo WAV; C++ lo convirtió a 24k PCM16 mono. MCP creó proyecto, descargó una foto Commons CC BY-SA 3.0 con atribución, adjuntó voz a la composición, generó preview y exportó MP4 24 segundos, 576 fotogramas H264/AAC, ffmpeg decode completo OK. 11 pruebas unitarias MCP/seguridad pasaron. Acceso HTTP sin bearer devuelve 401. Las fuentes y el modelo se mantendrán separados del repositorio.
# Pendientes
Pruebas Windows reales de engine y Neo Lite; HTTPS/OAuth compatible con el cliente remoto, rate limits y job queue para producción. Repositorio privado GitHub requiere push de cambios y empaquetar. Modelo de voz no redistribuido. Openverse búsqueda solo; Commons descarga permitida.
# Próximos pasos
Regresiones, commit y push a ThowiLabs/GPTFrames; ZIP, video y docs. Registrar nueva versión tras prueba real Windows.
