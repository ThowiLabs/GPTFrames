# Fecha
2026-10-15 UTC
# Objetivo
Recrear el video Go/Rust/C++ de 243 segundos y 1920x1080 a 30 FPS con voz Capybara del checkpoint 268/110398, sin marcas GPTFrames ni subtítulos ASS, y cierre de suscripción a HirCoir.
# Decisiones
Rama independiente feature/voice-fonetica-30fps-no-brand, parent feature/capybara-latest-motion-youtube, main intacto. Mantener guion editorial `narration` legible y generar por separado `speech_text` fonético para Neo Lite (Go -> gou, Rust -> rast, C más más -> ce más más, HirCoir -> Jírcoir, también backend/ownership/goroutines). Conservar escritura correcta de nombres de lenguajes y canal en la imagen. Duración exactamente 243s, 30fps=7290 fotogramas, 13 escenas.
# Implementación
storyboard.py produce texto para TTS separado, fps30. make_slides.py elimina las marcas de cabecera y pie y añade tarjeta de CTA «SUSCRÍBETE A HIRCOIR»; make_motion_layers.py añade la tarjeta como capa animada. render_motion.py usa 30fps, evalua movimiento por frame y elimina por completo filtro ASS, output motion-30fps con nombre neutral. mix_voice_latest.py mezcla los 13 WAV fonéticos sin música en capybara-fonetica-30fps.wav. Nueva página v3 sin branding visible y con enlaces MP4/voz/licencias. Pruebas tests/test_video_pronunciation_30fps.py.
# Dependencias y seguridad
Mismo modelo .neo compilado de checkpoint HirCoir epoch268-step110398, no se publica ni se copia a GitHub; tampoco HF tokens, PIN OAuth ni archivos de video grandes. Todo ejecutado por integración Slack en sandbox VNqmLEZSl5l2WvJCy-0Aa0RO. No se conecta ni modifica MCP.
# Hechos verificados
La reconstrucción produjo 13 archivos de voz fonética mediante Neo Lite y mezcla WAV de 243s, PCM 24 kHz mono, -17.6dB medio / -0.4dB pico. Primer segmento visual 1920x1080 30fps, captura comprobada sin marcas, sin subtítulos. Primera vista previa 34s H264/AAC 30fps 1020 cuadros, URL temporal https://9409cf3846cd3d9be6.gradio.live/v3/adelanto-Capybara-30fps.mp4 . Pantalla CTA inspeccionada; HTML de las 13 escenas sin GPTFRAMES, 13 fondos y capas reconstruidos. Test fonético sin subtítulos pasó.
# Pendientes
Finalizar render completo de 13 segmentos, ffprobe/decodificar completa, copiar MP4 a public/v3, compartir descarga temporal y documentar commit/push. Probar pronunciación con escucha humana, diccionario IPA de phonemizer si procede.

# Verificación final
13 segmentos exportados a 30fps. MP4 final output/motion-30fps/Go-Rust-Cpp-Capybara-30fps-SinSubtitulos.mp4 41,698,059 bytes, H.264 (7290 cuadros), AAC, 1920x1080, 243s. Reproductor HTTPS /v3/, audio WAV y video verificados HTTP 200, enlace temporal. FFmpeg decodificación de los 7290 cuadros y audio exit0. Pistas subtítulos ninguna. Captura del frame t=240 s mostró CTA correcto. Página inicial redirige automáticamente a /v3/.
