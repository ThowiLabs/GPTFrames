# Vulnerabilidades web: cómo funcionan y cómo proteger tu servidor

Documental educativo de 13 escenas, 280 segundos, salida H.264/AAC de 1920×1080 a 30 FPS. Explica controles preventivos sin procedimientos de explotación, sin música y sin subtítulos. El tema es complejo y se presenta como una breve introducción.

## Fuentes técnicas

- OWASP Top 10:2025: https://top10.owasp.org/2025/
- Control de acceso: https://top10.owasp.org/2025/A01_2025-Broken_Access_Control/
- SQL Injection Prevention: https://cheatsheetseries.owasp.org/cheatsheets/SQL_Injection_Prevention_Cheat_Sheet.html
- Cross-Site Scripting Prevention: https://cheatsheetseries.owasp.org/cheatsheets/Cross_Site_Scripting_Prevention_Cheat_Sheet.html
- Database Security: https://cheatsheetseries.owasp.org/cheatsheets/Database_Security_Cheat_Sheet.html
- Secure Code Review: https://cheatsheetseries.owasp.org/cheatsheets/Secure_Code_Review_Cheat_Sheet.html

## Voz Lilith y procedencia

No fue posible confirmar ni descargar el ZIP privado de YahirHub desde el sandbox, por lo que **no se afirma que este modelo sea binariamente idéntico al solicitado por ese enlace**. Para cumplir el objetivo se utilizó la distribución Lilith del propio autor HirCoir en Hugging Face, con acceso autenticado y licencia declarada Apache-2.0:

- Repositorio: https://huggingface.co/HirCoir/es_MX-lilith2
- Revisión fijada: 8d7ca9584a4019a13f4a661d27951b93c9fdea36
- Se descargaron únicamente es_MX-lilith.onnx (63,122,309 bytes) y es_MX-lilith.onnx.json.
- Neo Compiler compiló voice/lilith.neo (31,273,799 bytes), verificación estructural completada; key_trust=unverified-author (firmante no reconocido por la lista de confianza).
- Neo Lite CPU realizó las 13 inferencias realmente a partir de ese archivo .neo, 22,050 Hz originales y mezcla de 24 kHz mono, 280s.
- Archivo .neo y fuente ONNX permanecen **solo en el sandbox** y no se distribuyen desde el reproductor público ni se suben al repositorio.

## Código y reconstrucción

Requisitos: HF CLI autorizado, FFmpeg, Chromium y binarios Neo Compiler y Neo Lite; asegurarse de que la ruta a ONNX Runtime esté en LD_LIBRARY_PATH al usar la CLI de inferencia.

1. hf download HirCoir/es_MX-lilith2 es_MX-lilith.onnx es_MX-lilith.onnx.json --revision 8d7ca9584a4019a13f4a661d27951b93c9fdea36 --local-dir examples/seguridad-web-lilith/voice
2. build/neo-lite/neo-compiler build --source examples/seguridad-web-lilith/voice/es_MX-lilith.onnx --config examples/seguridad-web-lilith/voice/es_MX-lilith.onnx.json --name Lilith --author HirCoir --license Apache-2.0 --profile lite --out examples/seguridad-web-lilith/voice/lilith.neo
3. python3 examples/seguridad-web-lilith/storyboard.py
4. python3 examples/seguridad-web-lilith/synthesize.py
5. python3 examples/seguridad-web-lilith/make_visuals.py
6. python3 examples/seguridad-web-lilith/prepare_small_layers.py
7. python3 examples/seguridad-web-lilith/render_fast.py
8. python3 examples/seguridad-web-lilith/publish.py

make_visuals.py crea 13 fondos y cuatro capas transparentes por escena mediante Chromium a 1920×1080. prepare_small_layers.py reduce las capas a 1280×720 y combina dos capas para evitar falta de memoria. render_fast.py anima tres grupos por escena (título, diagrama+protección, tarjetas) además de paneo de fondo, y escala la salida H.264 a 1920×1080. Se entrega **Full HD de salida con composición intermedia de 720p**, no rasterización 1080p nativa del compositor JSON C++. No ejecutar render y Chromium simultáneamente en sandbox de 4 GB.

El tutorial no incluye payloads de explotación. Cubre control de acceso/IDOR, inyección SQL, XSS, autenticación/sesiones, configuración insegura, dependencias, SSRF, CSRF, archivos, TLS/cifrado, monitoreo, disponibilidad y hardening. Reafirma que la seguridad web es un tema complejo y que el video es una introducción.

## Legalidad y privacidad

Los diagramas son originales y las fuentes son las guías públicas citadas. La música no se incluye. No publicar tokens, claves, PIN OAuth, checkpoints, ONNX, .neo ni otros archivos privados. Un enlace temporal de Gradio servirá únicamente MP4, WAV, poster y fuentes.
