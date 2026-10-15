# Fecha
2026-10-15 UTC
# Objetivo
Rehacer documental Go/Rust/C++ Full HD con verdadera animación, voz del checkpoint Capybara de más epochs/steps y música licenciada de YouTube Studio si está disponible.
# Decisiones
Rama feature/capybara-latest-motion-youtube creada desde feature/capybara-kagssh-mcp-1080p; main y video anterior intactos. El repositorio checkpoint HF lista epoch=268-step=110398.ckpt como el mayor step/epoch, con licencia CC BY-NC 4.0; no se infiere calidad de métricas inexistentes. Se usa sesión hf auth sin imprimir token; no hay acceso a YouTube Studio, por lo que el video v2 se produce sin música, evitando infracciones y sin inventar una pista.
# Arquitectura
LibTorch CPU 2.5.1 para compilar NEO_INFER_ONLY, export-onnx C++ nativo usando resources en third_party/neo-tts/assets. Neo Compiler convierte el ONNX exacto a capybara-epoch268-step110398.neo, verificado, y Neo Lite sintetiza 13 segmentos reales. mix_voice_latest.py produce WAV PCM mono 24 kHz de 243 segundos sin música procedural. Opción --youtube-audio-library permite usar MP3/WAV descargado legítimamente por el usuario.
# Animaciones
make_motion_layers.py separa las imágenes de fondo y los componentes HTML/CSS a PNG RGBA 1920x1080 (títulos, logotipos, tarjetas, fotografías, filas de tabla); render_motion.py anima x/y/fade por frame con FFmpeg. make_captions.py genera subtítulos ASS con fade por capítulo. Timeline 13 escenas de 243 s. 1920x1080 a 24 FPS y H264/AAC.
# Recursos
Checkpoint y ONNX/.neo en examples/go-rust-cpp-1080p/voice/latest, ignorados en Git. LibTorch en third_party/libtorch-cpu, ignorado. motion-layers y motion-captions generadas e ignoradas. Codigo: make_motion_layers.py, make_captions.py, render_motion.py y mix_voice_latest.py, docs/CAPYBARA_LATEST_MOTION.md.
# Problemas/soluciones
Primera compilación faltaba libminizip-dev, zlib1g-dev y libcurl4-openssl-dev, instaladas. Para ejecutar export-onnx es imprescindible cwd third_party/neo-tts donde están assets/piper-medium.graph.pb.gz y assets/piper-medium.weights.tsv. Export ONNX correcto 63,516,077 bytes/350 tensores. Neo Compiler generó .neo de 31,365,501 bytes con firma estructural válida. La síntesis de una frase desde último .neo pasó.
# Pendientes
Finalizar inferencia de todos capítulos, mezclar voz, terminar render v2, ffprobe/ffmpeg QA, publicar sitio temporal con descarga, subir commit GitHub de rama feature. Esperar pista autorizada desde YouTube Studio si se solicita con música; no entrar a cookies de Google ajenas.
