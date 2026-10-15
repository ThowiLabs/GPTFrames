# Capybara v2 — Epoch 268, step 110398 y motion graphics 1080p

## Razón de esta variante
El repositorio Hugging Face HirCoir/piper-checkpoint-es-mx-capybara contiene muchos checkpoints. La lista se ordenó por step numérico (epoch como desempate); el mayor conocido es `epoch=268-step=110398.ckpt` (~846 MB). Más entrenamiento no implica necesariamente mayor calidad audible, pero este es el elegido explícitamente.

## Conversión
La exportación ONNX pública anterior de HirCoir no identifica su checkpoint preciso. Por eso se descargó el checkpoint específico con acceso HF autorizado por el usuario, su config.json, y se construyó el exportador oficial Neo-TTS C++17 usando LibTorch CPU. El recorrido solicitado: checkpoint epoch268 → Neo TTS export-onnx → ONNX → Neo Compiler build --profile lite → .neo → Neo Lite infer → WAV.

Las piezas grandes del modelo, los paquetes LibTorch y los archivos de credenciales se mantienen fuera del repositorio Git. El checkpoint HF declara CC BY-NC 4.0; antes de usos comerciales se deben verificar las condiciones del autor y de la grabación original.

## Música de la Biblioteca de audio de YouTube
El acceso a la Biblioteca de audio de YouTube redirigió desde este sandbox a inicio de sesión Google; no existe aquí un archivo descargado y licenciado oficialmente desde YouTube Studio. Para no usar una canción ajena sin autorización, la nueva demostración se genera **sin la antigua música procedural**. El mezclador admite una pista descargada legalmente por el titular mediante `mix_voice_latest.py --youtube-audio-library ruta/audio.mp3`, respetando las condiciones de la pista. Sin archivo, el mezclador entrega voz limpia. Esto evita inventar una canción de la biblioteca.

## Animación
La versión anterior consistía principalmente en capturas estáticas con acercamiento. Ahora `make_motion_layers.py` separa títulos, imágenes, tarjetas, logos, ventajas, riesgos, decisiones y **filas individuales de la tabla** como PNG de fondo transparente. `render_motion.py` calcula en cada fotograma desplazamiento horizontal y vertical, opacidades/fades y parallax sobre fondo en movimiento; además `make_captions.py` añade subtítulos con aparición y desaparición gradual alineados por capítulo.

Conservar 1920×1080 a 24fps, 243 segundos (4:03), Go/Rust/C++ y el veredicto cualitativo. Son efectos FFmpeg sobre el pipeline HTML/Chromium de GPTFrames, no se afirma haber reformado todavía el framebuffer C++ nativo, que sigue vertical.
