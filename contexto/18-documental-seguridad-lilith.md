# GPTFrames · Lilith y seguridad web · 2026-10-15

## Solicitud
Descargar modelo Lilith de un ZIP privado YahirHub/piper-voices, convertir a .neo e ilustrar vulnerabilidades web, cómo funcionan y defensas del servidor, aclarando que el tema es complejo y solo se ofrece una introducción.

## Acceso al modelo
Repo YahirHub/piper-voices no accesible anónimamente: 404. La URL original con parámetro token no pudo procesarse por controles de seguridad del entorno; no se replicó ni guardó esa credencial. Alternativa explícita y autorizada: cuenta HF HirCoir conectada, https://huggingface.co/HirCoir/es_MX-lilith2, revisión 8d7ca9584a4019a13f4a661d27951b93c9fdea36, dos archivos descargados: es_MX-lilith.onnx (63,122,309 bytes) y es_MX-lilith.onnx.json. Se creó voice/lilith.neo (31,273,799 bytes) usando Neo Compiler perfil lite; neo-compiler verify pasó con trust=unverified-author; Neo Lite hizo inferencia real de 13 WAV. No se puede asegurar que sea el mismo modelo binario del ZIP no accesible. Licencia declarada HF Apache-2.0; modelo gated permanece privado.

## Resultado
Documental educativo OWASP Top 10:2025, 13 escenas x 280 segundos a 30fps = 8400 fotogramas H264, AAC, 1920x1080 de salida, 45,681,715 bytes. Voz Lilith mono 24 kHz de 280s, sin música. Guion incluye control de acceso, IDOR, SQL injection, XSS, autenticación, mala configuración, cadena de suministro, SSRF, CSRF y archivos, criptografía TLS, monitoreo/disponibilidad, hardening y breve introducción (complejidad del tema); pruebas solo autorizadas. Fuentes OWASP y Cheat Sheets oficiales. Los 13 fondos y cuatro elementos por escena se generaron con Chromium/CSS a 1920x1080, compositing intermedio 1280x720 y upscale al final a 1920x1080 por límite 4GB RAM (honestidad sobre raster efectivo). Grupos de títulos, diagramas/protecciones y recomendaciones se animan con posiciones, opacidad y paneo por frame FFmpeg.

Archivo final privado sandbox: examples/seguridad-web-lilith/output/Vulnerabilidades-Web-Lilith-1080p-30fps.mp4; audio: output/lilith-voiceonly.wav; .neo: voice/lilith.neo.

## QA y publicación
ffprobe confirma 280.000000 s, 1920x1080, 30/1 fps, 8400 frames, H264/AAC; FFmpeg decodificó todo MP4 con salida de errores vacía. Inspección visual de inicio, inyección, configuración y cierre (texto legible, sin subtítulos). MP4 público temporal HTTPS 200: https://9409cf3846cd3d9be6.gradio.live/seguridad-web-lilith/Vulnerabilidades-Web-Lilith-1080p-30fps.mp4 ; reproductor https://9409cf3846cd3d9be6.gradio.live/seguridad-web-lilith/ ; WAV y fuentes también HTTP200. No publicar modelo, tokens ni OAuth PIN, solamente MP4/audio/créditos. Scripts en examples/seguridad-web-lilith y README.md.

## Limitaciones
La URL Gradio es temporal. La voz y la pronunciación no se auditaron con escucha humana; mezcla atempo máximo ~1.14. No confundir salida 1080p con raster intermedio. El ZIP exacto del usuario no fue verificado: se debe pedir adjuntar o habilitar acceso al ZIP para comparar los SHA-256 si el usuario necesita identidad exacta. Modo de despliegue sin autorización de ChatGPT MCP modificada.
