# Motor híbrido Browser-first — experimento aislado

## Objetivo y decisión

Esta rama prueba que Chromium puede ser el renderizador visual de escenas
HTML/CSS/SVG/Canvas, incluidas animaciones de tipo UIverse, y que GPTFrames
C++ puede conservar la línea de tiempo, la voz Neo Lite y la codificación
MP4 H.264/AAC. **No es una sustitución completa del compositor JSON.**

La implementación actual de `browser::Capture` estaba condicionada a
`window.__timelines.main` (GSAP). La variante experimental permite cargar
un HTML local sin GSAP y solicitar, a través de CDP, el tiempo de todas sus
animaciones CSS mediante `document.getAnimations()`, `pause()` y
`currentTime`. Se captura un PNG en cada tiempo y el encoder H.264/AAC
embebido en GPTFrames escribe el resultado. No requiere Node, Puppeteer,
Playwright, Python ni un ejecutable de FFmpeg en el flujo de producción.

## Requisitos de ejecución

- Binario compilado `hyperframes-cpp-native` (C++17), con
  `HFC_BROWSER_CAPTURE=ON` y `HFC_NATIVE_MP4=ON`.
- Navegador compatible con CDP: Chromium, Chrome o Edge, detectable vía
  `--browser-check` o variable `HFC_BROWSER`.
- Para narración: binario **Neo Lite** compilado (`neo-lite`) y archivo local
  `.neo`/modelo de voz. La llamada de síntesis sigue siendo un **proceso
  aparte**: integrar la biblioteca Neo Lite dentro del ejecutable GPTFrames
  aún no está hecho. WAV intermedio local 24 kHz mono, luego AAC interno.
- HTML, CSS, fuentes e imágenes **locales**, empaquetados con el proyecto.
- ThorVG Lottie opcional integrado en el ejecutable si se necesita
  `HFC_THORVG=ON`. Nada de esto obliga a añadir runtimes de JavaScript
  de servidor.

## Arquitectura recomendada

```text
Proyecto editable JSON + assets + voz/speechTiming
     |
Motor C++: escenas, cues, reloj fotogramas, controles de seguridad
     |
     +-- Chromium headless CDP: HTML/CSS/SVG/Canvas y widgets
     |      timeline CSS -> fotograma PNG/RGBA por instante
     |
     +-- Compositor C++: máscaras, clips, cámara, fondos y capas nativas
     |
     +-- Neo Lite CLI + modelo .neo -> WAV por escena
     |
     +-- Mezclador PCM y H.264/AAC nativos -> MP4
```

Es preferible **híbrido** a navegador exclusivo: usar el navegador para
visual avanzado, y conservar C++ para capas, audio, codecs, seguridad,
vídeo de entrada y trabajos persistentes. No duplicar CSS en el motor C++.

## Prototipo ejecutable

```bash
cmake -S . -B build/native-full -DHFC_BROWSER_CAPTURE=ON -DHFC_NATIVE_MP4=ON
cmake --build build/native-full --target hyperframes-cpp-native -j2
build/native-full/hyperframes-cpp-native --browser-check

# Capturar exactamente t=0.7 segundos
build/native-full/hyperframes-cpp-native --browser-widget-frame \
  examples/browser-first-uiverse/loader.html 0.7 \
  build/loader-t07.png 848 480

# Producir MP4 de 2 segundos a 24 FPS (sin voz, para evaluar visuales)
build/native-full/hyperframes-cpp-native --browser-widget-render \
  examples/browser-first-uiverse/loader.html \
  build/loader-preview.mp4 2 24 848 480
```

El ejemplo es un loader original HTML/CSS inspirado en el estilo de
interfaces de carga modernas: dos anillos, núcleo pulsante y barra móvil.
No corresponde a un elemento copiado de UIverse. Los componentes obtenidos
de UIverse Galaxy se pueden usar respetando su licencia MIT y conservando
autor/atribución cuando corresponda:
https://github.com/uiverse-io/galaxy .

## Límites actuales

- Los dos comandos `--browser-widget-*` son **experimentales y sólo CLI**.
  No están publicados en las herramientas MCP ni admiten proyectos remotos.
- `--browser-widget-frame`: HTML local hasta 1 MiB, viewport acotado,
  tiempo entre 0 y 120 segundos. `--browser-widget-render`: duración
  máxima de 10 segundos, hasta 30 fps, anchura múltiplo de 16. Voz silenciosa.
- Al renderizar CSS se pide `getAnimations` en cada fotograma, por lo que
  funcionan keyframes y pseudo-elementos. Interacciones JS, animaciones
  dependientes de red, timers, estados React, animaciones Canvas y
  frames de vídeo no se pueden considerar deterministas todavía.
- El prototipo registra un requisito de CSP offline en el HTML, pero
  **eso NO es una sandbox de seguridad suficiente**.
- Chromium ejecutado como root utiliza actualmente `--no-sandbox` en el
  código heredado. En producción hay que usar usuario dedicado, sandbox de
  Chromium habilitado, permisos y cuotas de CPU/memoria, directorios y
  orígenes aislados, red denegada y tiempos máximos. No servir esta ruta
  a HTML no confiable hasta terminar ese trabajo.
- Los cuadros PNG se codifican de nuevo en H.264; el costo de screenshot
  por CDP puede ser alto. Optimizar perfil persistente, formato RGBA y
  caché si la medición lo exige.
- Croma de vídeo, audio de clips, Lottie web arbitrario y múltiples
  escenas browser+capas nativas sincronizadas quedan para etapas siguientes.

## Próxima iteración

1. Unificar el esquema `scene.backend = native|browser|hybrid`;
   no alterar proyectos v1 ya existentes.
2. Captura alfa real de PNG RGBA y composición en C++ de layers
   `html_widget` sobre imágenes o clips con el mismo `cueIndex`.
3. Aislamiento fuerte del navegador y sanitizer/empaquetador local
   de HTML/CSS de UIverse, sin JS arbitrario ni descargas en render.
4. Generación de escenas HTML a partir de JSON y temporización precisa
   de narración, sin fondos/tarjetas vacíos.
5. Clips MP4/WebM con decoder enlazado (bibliotecas FFmpeg o alternativa),
   mixer PCM y caché de fotogramas. El `ffmpeg` de consola sólo
   puede seguir siendo una herramienta de diagnóstico opcional.
6. Tests de determinismo, memoria, fuga de procesos, PNG alpha y vídeo
   integrado, incluidos runners Linux/Windows.
