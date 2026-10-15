# Fecha
2026-10-15

# Objetivo
Ensayar nueva arquitectura browser-first, capaz de reproducir widgets
HTML/CSS de estilo UIverse sin requerir Node/Python en tiempo de render.

# Decisiones tomadas
- Antes de experimentar, guardar V5 y Motion Studio en commit 9f20cef,
  rama feature/professional-studio-native.
- Crear rama experiment/browser-first-uiverse, separada de main.
- No sustituir completamente el compositor nativo: mantener motor C++
  para pistas, sincronía por voz, vídeo, máscara, PCM y MP4.
- Utilizar el navegador ya detectado y CDP, no instalar Playwright.
- CSS animations con document.getAnimations(), pause/currentTime
  deterministas por tiempo de escena; PNG capturado por Chromium.
- Limitar nuevas entradas al CLI experimental con HTML local y CSP,
  NO exponer a los endpoints MCP mientras Chromium no esté aislado.

# Arquitectura actual
- hyperframes-cpp-native: composiciones, Neo Lite CLI, carga voz .neo,
  lector de escenas, codec H.264/AAC nativo, CDP existente.
- --browser-widget-frame: busca animaciones CSS y captura tiempo dado.
- --browser-widget-render: convierte secuencia de capturas a MP4 nativo.
- browser_capture.hpp usa perfil Chromium local temporal y CDP loopback.

# Librerías usadas
C++17, librerías ya integradas de codec; Chromium del sistema. ThorVG
continúa siendo opcional. La demo usa CSS puro. Ningún paquete NPM/PIP.

# Archivos importantes modificados
browser_capture.hpp, main.cpp, CMakeLists.txt, docs/BROWSER_FIRST_EXPERIMENT.md,
examples/browser-first-uiverse/loader.html,
tests/browser-widget-smoke.sh, tareas/completado-15-browser-first-uiverse.md.

# Problemas encontrados
- La versión previa de CDP requería GSAP window.__timelines.main,
  y por tanto no procesaba componentes CSS autónomos.
- La captura PNG implica sobrecoste por fotograma.
- El Chromium heredado usa --no-sandbox cuando corre como root:
  prohibido exponer HTML de terceros a este modo sin aislamiento.
- La comprobación simple de CSP no equivale a un sanitizer ni sandbox.

# Soluciones implementadas
- Añadido modo CSS alternativo en el mismo capturador CDP.
- Fotogramas distintos en t=0, 0.7 y 1.4 segundos, inspeccionados.
- Render MP4 848x480, 24fps, 2s verificado mediante ffprobe y decode.
- Test browser-css-widget pasó en 34s.
- Enlace público de preview:
  https://771757373c9f1dfc2a.gradio.live/GPTFrames_BrowserFirst_CSS_480p_24fps.mp4

# Pendientes
- Aislar Chromium con UID sin privilegios + namespace/seccomp/red bloqueada.
- Permitir importación segura de HTML/CSS de terceros.
- PNG con canal alfa y mezcla con C++ para capas overlay.
- Sincronización global por voz y paneles reactivos híbridos.
- Vídeo de entrada MP4/WebM y sincronía multimedia.
- Reducir tiempos de inicio de Chromium y medir velocidad de captura.

# Próximos pasos
Desarrollar pipeline de proyectos backend=native|browser|hybrid,
sin alterar proyectos v1 ni requerir servicios de Node o Python. Añadir
catálogo curado MIT de UIverse con atribución por activo.

# Control de Git
Commit base: 9f20cef.
Rama actual: experiment/browser-first-uiverse.
No se ha reiniciado MCP, no se ha hecho push ni merge a main.
Los proyectos privados, modelos y fotogramas diagnósticos siguen locales.
