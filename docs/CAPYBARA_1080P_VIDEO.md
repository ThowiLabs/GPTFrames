# GPTFrames: Capybara y documental Go/Rust/C++ a 1080p

## Objetivo y producto
Video documental educativo de 243 segundos (4:03), horizontal 1920×1080 y 24 fotogramas por segundo. Incluye 13 capítulos, imágenes atribuidas de Commons, voz Capybara, animación tipo Ken Burns, música procedural y tabla cualitativa. Sin «página actual / total de páginas».

## Modelo de voz Capybara
Repositorio checkpoint Piper Train: https://huggingface.co/HirCoir/piper-checkpoint-es-mx-capybara (gated).
Exportación ONNX del mismo propietario: https://huggingface.co/HirCoir/piper-voice-es_mx-capybara (ficha del modelo Apache-2.0).
Se aprovechó la conversión de checkpoint a ONNX ya publicada por HirCoir y se compiló esa exportación a capybara.neo mediante Neo Compiler, profile lite. La estructura y firma fueron verificadas por neo-compiler verify. Neo Lite generó todas las 13 narraciones desde el .neo real (no voz sustituta), PCM mono 22 050 Hz. Mix posterior de voz a 24 kHz, ajuste leve de tempo y música matemática original.

Los pesos se descargan utilizando la sesión privada HF y nunca deben incluirse en Git. HF_TOKEN no estaba como variable de entorno; hf auth login sí funcionó. No se almacena ni imprime el token. Revisar términos de acceso y licencia del checkpoint original antes de cualquier redistribución comercial del modelo.

## Arquitectura horizontal
La etapa C++ del compositor JSON original tiene raster interno de 720×1280. El documental horizontal utiliza una **extensión** distinta: plantillas HTML/CSS de 1920×1080 -> capturas Chromium headless -> segmento MP4 por capítulo con movimiento/fades usando FFmpeg -> montaje H.264/AAC con voz Capybara. Por tanto, no se debe presentar como render horizontal nativo del compositor JSON sin esas herramientas.

Archivos principales:
- examples/go-rust-cpp-1080p/storyboard.py: guion y temporización de 13 escenas.
- make_slides.py: composición editorial Full HD con logos, fotografías y tabla.
- mix_audio.py: generador WAV Capybara y música, pacing por capítulo.
- render_video.py: masterización H.264/AAC.
- assets-sources.json: atribuciones de medios originales de Commons.
- voice/hf: pesos Capybara (ignorados en Git).
- slides y output: recursos derivados (ignorados en Git).

## Ejecución
1. Generar storyboard: python3 examples/go-rust-cpp-1080p/storyboard.py
2. Descargar ONNX y configuración Capybara desde el repositorio autorizado en HF.
3. Ejecutar build/neo-lite/neo-compiler build --source ruta.onnx --config ruta.onnx.json --name Capybara --author HirCoir --profile lite --out capybara.neo
4. Generar un WAV por escena con neo-lite infer --model capybara.neo --text-file texto.txt --out narracion.wav --device cpu
5. Generar HTML mediante make_slides.py; capturar con Chromium headless a 1920×1080.
6. Ejecutar mix_audio.py y render_video.py.
7. Validar ffprobe y decodificación completa ffmpeg.

## Elección «rey»
La elección Rust como equilibrio entre rendimiento y seguridad es un juicio técnico cualitativo, no un benchmark universal. Go destaca en productividad/backend; C++ en control profundo e integración con ecosistemas maduros.

## Licencias y atribución
Consultar assets-sources.json: cada medio indica Commons, autor y licencia. Los materiales CC BY requieren crédito. Los logos y fotos mantienen las condiciones de sus fuentes.
