# Fecha
2026-10-15 UTC

# Objetivo
Hacer reutilizable el motor HyperFrames C++ Remix mediante un formato JSON declarativo de escenas, capas, animaciones y audio, sin editar ni recompilar código C++ para cada video.

# Decisiones tomadas
Implementar en rama feature/compositor-json-nativo creada desde feature/remix-hyperframes-remotion; no tocar main. Reusar la copia ya incluida de nlohmann/json v3.11.3 en vendor/browser/json.hpp y stb_image en vendor/browser/stb_image.h. No introducir ThorVG/Blend2D todavía porque la necesidad prioritaria es disponer de proyectos configurables que permitan utilizar cualquier efecto en videos nuevos sin programarlos a mano. Mantener canvas interno 720×1280 y salida opcional escalada en 9:16, alineada a múltiplos de 16 por el codec H264.

# Arquitectura actual
main.cpp añade --project validate|frame|render. compositor/json_project.hpp integra parser validado, carga de imágenes relativas segura, scene graph de capas, keyframes/easings, transiciones, composición con alfa y mezcla de WAV PCM mono 24k. No rompe CLI original HTML/AVI/browser/remix/fusion. examples/json-compositor/showcase.json y sus assets PNG/WAV permiten cambiar texto, colores, animaciones y posiciones sin recompilar.

# Librerías usadas
nlohmann/json MIT y stb_image MIT/public-domain, ambas YA disponibles en vendor/browser; stb_truetype, minih264, minimp4, vo-aacenc como previamente. Ninguna nueva dependencia externa en ejecución.

# Archivos importantes modificados
main.cpp, compositor/json_project.hpp, docs/JSON_COMPOSITOR.md, examples/json-compositor/showcase.json y assets/{orbit.png,ambient.wav,chime.wav}, tests/json-compositor.sh, README.md.

# Problemas encontrados
Primer intento agregó json.hpp y stb_image.h duplicados. Se detectó choque de implementación stb_image, y se eliminó la duplicación reutilizando las bibliotecas actuales. La macro near() de cabeceras Windows chocó con un nombre de función del compositor: se renombró nearTime(). Demo de recursos se generó con bibliotecas estándar de Python porque el contenedor no tenía Pillow instalado.

# Soluciones implementadas
Tres escenas JSON nuevas, movimientos de keyframes, imágenes PNG con canal alfa, formas, texto, 4 curvas de easing más step/back, transiciones cut/fade/wipe/iris/slide y dos pistas WAV mezcladas; muestra MP4 exportada de 18 segundos, 432 frames 720x1280 a 24fps H264/AAC, decodificación completa FFmpeg sin errores. Tests/json-compositor.sh valida que editar el JSON cambia la imagen, determinismo, seguridad de rutas, tiempo y MP4. Windows cross-compilation exitosa antes y después de cambiar rutas; ejecución en Windows real pendiente.

# Pendientes
Prueba manual Windows real. Agregar soporte MP3/M4A a nivel JSON; SVG/Lottie y composición GPU aún pendientes. Mejorar antialias/shadows, rotación de texto y tamaño del canvas real en lugar de escalado. Crear editor de timeline visual y publicar en hosting permanente solo si se solicita.

# Próximos pasos
Verificar regresiones HTML/browser/remix y empaquetar ZIP completo con fuentes y assets (sin incluir TTF crudas), Linux/Windows y demo descargable; dejar contexto actualizado.
