# Video Go vs. Rust vs. C++ — voz fonética, sin marca, 30 FPS

Fecha: 2026-10-15. Rama: `feature/voice-fonetica-30fps-no-brand`.

## Cambios del usuario
- Quitar del **video y su nueva página de reproducción** cualquier marca visible GPTFrames; no renombrar ni borrar el código/repositorio interno.
- Quitar subtítulos incrustados tipo ASS/captions, sin eliminar títulos informativos propios de las composiciones.
- Regenerar **todas las 13 narraciones** usando el mismo modelo Capybara compilado del checkpoint exacto `epoch=268-step=110398.ckpt`, sin sustituirlo por la exportación de origen indeterminado.
- Mantener dos versiones de texto: guion editorial bien escrito en `scene.narration` y versión fonética para Neo Lite en `scene.speech_text`, escrita en `text/narracion-NN.txt`. Términos: Go → «gou», Rust → «rast», C más más → «ce más más», HirCoir → «Jírcoir», entre otros. La fonética es aproximada y se usa exclusivamente para el sintetizador.
- Incorporar invitación a suscribirse al canal **HirCoir** en la última narración y una tarjeta final animada «SUSCRÍBETE A HIRCOIR».
- Cambiar la exportación de 24 a **30 fotogramas por segundo**; mantener 1920 × 1080, 243 segundos y 13 escenas. Total esperado: 7290 frames.
- Conservar voz sola, sin música procedural ni música no autorizada de YouTube Studio.

## Construcción
`python3 examples/go-rust-cpp-1080p/storyboard.py` actualiza 13 guiones con fonética y fps=30.
`python3 .../make_slides.py` y `python3 .../make_motion_layers.py` regeneran 13 fondos y capas sin marca.
Neo Lite infiere cada `text/narracion-XX.txt` con `voice/latest/capybara-epoch268-step110398.neo`, depositando los WAV en `voice/phonetic-30fps`.
`python3 .../mix_voice_latest.py --voice-dir examples/go-rust-cpp-1080p/voice/phonetic-30fps` crea `output/capybara-fonetica-30fps.wav` PCM mono 24k.
`python3 .../render_motion.py` crea segmentos 30 FPS, anima elementos separados y multiplica H.264 + AAC en `output/motion-30fps/Go-Rust-Cpp-Capybara-30fps-SinSubtitulos.mp4`. No usa ASS.
`python3 tests/test_video_pronunciation_30fps.py` valida metadatos, guion, marca y subtítulos.

## Salida y seguridad
No se incluyen en Git checkpoints ONNX/NEO, credenciales HF, PIN OAuth ni audios/imágenes derivadas, solo código y documentación. La página pública v3 es exclusivamente de reproducción: no expone MCP. Los URLs públicos temporales pueden expirar.

## Limitaciones
La pronunciación aproximada no equivale a un diccionario fonético de eSpeak/IPA; para matices de voz se requieren revisiones auditivas. La nueva codificación a 30fps incrementa el número de fotogramas de 5832 a 7290 y su costo de render.
