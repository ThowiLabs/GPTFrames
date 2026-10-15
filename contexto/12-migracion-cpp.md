# Migración del MCP y utilidades a C++17
Fecha: 2026-10-15 UTC
Rama: refactor/cpp-sin-python
Base: 7db4a6eb457d753e79613770ef7a975985bb9494

## Objetivo y decisión de arquitectura

Mantener el motor y las herramientas propias en C++ y conservar el navegador instalado para composiciones HTML/CSS/JavaScript. El servidor MCP y sus clientes de proveedores se compilan como un ejecutable nativo. CMake y los scripts de shell coordinan compilación, ejecución y pruebas.

El submódulo Neo-TTS conserva su revisión fijada; se usa su ruta de inferencia nativa NEO_LITE_ONLY. Sus herramientas de entrenamiento de upstream quedan fuera del flujo de GPTFrames. Los modelos y binarios externos se instalan por separado.

## Implementado

- Sustituidos mcp/server.py y mcp/media.py por mcp/server.cpp y módulos C++: contratos, servicio, HTTP, medios, procesos y utilidades.
- Conservadas las ocho herramientas y sus campos de respuesta: proyectos, validación, preview, MP4, voz, búsqueda Wikimedia/Openverse e importación Commons.
- Stdio con mensajes acotados y recuperación tras errores; HTTP en loopback con bearer, validación de Origin y límites de cuerpo.
- Procesos mediante argv, sin shell; timeouts y grupos de procesos en Linux, CreateProcessW y Job Object en Windows. La salida capturada está acotada y la salida continua no evita el timeout.
- HTTPS con libcurl en Linux y WinHTTP en Windows. Hosts y redirecciones verificados, tamaños acotados y atribución junto a cada imagen.
- Escrituras de proyecto atómicas, rollback tras errores de voz y rechazo de rutas fuera del proyecto, incluidos directorios enlazados.
- Generador de fuentes y fixtures de audio/HTML/JSON reescritos en C++; eliminados los seis archivos Python propios del repositorio y sus invocaciones.
- CMake genera las fuentes en el directorio de build usando un compilador anfitrión. Las compilaciones configuradas utilizan ese encabezado incluso si existe uno antiguo en la raíz.
- Paquetes Linux/Windows incluyen renderer, MCP, plantilla editable, assets, documentación, licencia de cpp-httplib y skill del agente.
- README, docs/MCP.md, skill existente y workflows actualizados. docs/ROADMAP.md registra prioridades concretas para shorts y videos generales.

## Verificación

| Comprobación | Resultado |
| --- | --- |
| Build Release Linux con HTML, MP4, browser y MCP | Completado |
| CTest | 5 suites aprobadas: 22 casos MCP, 16 checks HTTP/stdio y pruebas de timeline, CSS y escala |
| Fixtures y compositor JSON v1 / showcase v2 | Aprobados |
| Smoke Linux: AVI, WAV, codecs, temas, Unicode, escala y cambios CSS | Aprobado |
| Chromium: fotogramas de distintos tiempos y MP4 nativo con AAC | Aprobado |
| Neo Lite real mediante MCP C++ | Narración PCM16 mono 24 kHz de 4.5395 s; adjuntada al proyecto |
| MP4 mediante MCP con la voz real | 2 s, 720x1280, 48 fotogramas H.264 y audio AAC |
| Wikimedia / Openverse en vivo | Búsqueda aprobada; Commons descargó JPEG y sidecar con autor/fuente/CC BY-SA 4.0 |
| Windows x64 con MinGW | Renderer y MCP compilados desde Linux; ejecución real en Windows pendiente |
| Configuración con HFC_MCP=OFF | Configurada sin buscar libcurl |
| Paquetes | Checksums y contenido Linux/Windows aprobados; Linux creó, validó y previsualizó un proyecto tras reubicar el paquete, sin --root |

Las pruebas de regresión de red usan respuestas simuladas. Las comprobaciones en vivo dependen de la disponibilidad del proveedor. La inferencia simulada en las pruebas unitarias verifica el contrato y rollback; la comprobación real anterior se hizo aparte con un modelo local no redistribuido.

## Límites que se conservan

El MCP exporta hasta 60 segundos por llamada; el compositor JSON admite hasta 240 segundos. JSON acepta hasta 12 pistas WAV con inicio y ganancia. Openverse solo descubre; Commons permite importar PNG/JPEG. Linux MCP requiere libcurl/TLS/CA del sistema; el renderer conserva enlace estático. Browser mode requiere Chromium/Chrome/Edge instalado.

## Trabajo siguiente

Seguir docs/ROADMAP.md: subtítulos y alineación, presets visuales y comprobaciones de composición, más proveedores y filtros, acabado de audio, duración configurable con trabajos cancelables y entrega autenticada de archivos. Falta QA real en Windows y un transporte remoto de producción con HTTPS/autenticación apropiados.

## Entorno de trabajo

Se usó un worktree independiente /workspace/GPTFrames-cpp-sin-python. /workspace/GPTFrames conserva su rama feature/remotion-lilith-security y el proyecto de usuario no versionado. El enlace local third_party/_sources solo reutiliza caché de dependencias y está ignorado por Git. Los paquetes y medios de prueba no se incorporan al repositorio.
