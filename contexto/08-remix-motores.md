# Fecha

2026-10-15 UTC

# Objetivo

Fusionar las capacidades HyperFrames C++ y el compositor de efectos C++ en un solo ejecutable, conservando interfaces y funcionalidades.

# Decisiones tomadas

Se trabajó sobre feature/remix-hyperframes-remotion en /workspace/hyperframes-cpp-public, sin tocar main. El compositor de efectos se adaptó como namespace remix que comparte framebuffer, tipografía TrueType y codec MP4, sin proceso externo. Se agregaron --remix-list, --remix-frame, --remix y --fusion. --fusion permite elegir tema classic/news/html/browser para la primera composición HTML, y agrega 9 escenas del showcase (81 s). Son recursos específicos de demostración y no una implementación de toda la API de Remotion.

# Arquitectura actual

main.cpp conserva importación HTML y salida AVI/MP4, selector de temas; remix/remix_renderer.hpp contiene efectos, 15 escenas y audio; remix/showcase.hpp y remix/audio_score.hpp implementan las nuevas animaciones y música. Las seis voces originales y el HTML están en proyecto-remix como demo local. Todo el video fusionado usa H.264/AAC codificado en C++.

# Librerías usadas

STL C++17, stb TrueType, litehtml, Chromium CDP opcional, minih264, minimp4, vo-aacenc. DejaVu Sans incrustada en ejecutable durante compilación; sin distribuir raw .ttf ni font_embedded.hpp.

# Archivos importantes modificados

main.cpp, remix/remix_renderer.hpp, remix/showcase.hpp, remix/audio_score.hpp, docs/REMIX.md, tests/remix-smoke.sh, README.md (adenda); proyecto-remix/index.html y audio/ usados como fixture.

# Problemas encontrados

Conflicto inicial de include relativa en módulos y overload rect entre los dos motores; resueltos. Prueba inicial MP4 360x640 no alineada a 16px exigida por H264; cambiado a 288x512. Windows ejecutable compilado cruzado pero no ejecutado en Windows real.

# Soluciones implementadas

Linux con todos los backends habilitados compiló; Windows cross-build con litehtml y captura browser también compiló. --remix-list 15 escenas, --remix-frame exporta PNG reproducible y --inspect HTML recupera 6 escenas. Fusion produjo video de 148 s, 3552 frames, H264/AAC 720x1280 a 24 FPS, 9 903 028 bytes. FFmpeg decodificó 148 s sin errores. tests/remix-smoke.sh pasó, tests/smoke-linux.sh original pasó integralmente. Video visual revisado en segundos 5 y 87.

# Pendientes

Probar .exe en Windows real; comprobar modo browser con composiciones JS externas; validar licencias/patentes H264/AAC para distribución. El showcase posterior sigue siendo un template predeterminado; no hay editor JSON universal de efectos.

# Próximos pasos

Empaquetar fuente con Git y ejemplo audiovisual privado sin fuentes TTF crudas, copiar entregables a servidor temporal de túnel y publicar enlaces.