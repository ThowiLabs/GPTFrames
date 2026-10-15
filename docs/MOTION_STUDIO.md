# GPTFrames — Motion Studio: ampliación de recursos animados

## Disponible y probado (C++17)

- `type:"symbol"` para objetos procedurales `phone`, `check`, `lock` y
  `angry-person`. Se calcula por fotograma; no requiere navegador,
  Python, Node ni ejecutables externos de render.
- Los símbolos admiten `x`, `y`, `width`, `height`, `color`,
  `animateSeconds`, `animation` y `keyframes`. Se pueden colocar dentro
  de un grupo `reactive` con `cueIndex` para activación por voz aproximada.
- `type:"image"` permite `chromaKey` para recortar fondos uniformes.
  Los bordes suaves se calculan en el motor nativo; el verde residual
  se atenúa en píxeles semitransparentes.

## Configuración

```json
{
  "type": "symbol",
  "symbol": "phone",
  "x": 180, "y": 210, "width": 300, "height": 340,
  "color": "#61E3CA",
  "animateSeconds": 1.3
}
```

La palomita del teléfono se dibuja progresivamente. El candado
tiene una transición entre arco abierto y cerrado.

```json
{
  "type": "image", "src": "assets/personaje-green.png",
  "x": 800, "y": 200, "width": 400, "height": 400,
  "chromaKey": {
    "enabled": true, "color": "#00FF00",
    "tolerance": 0.18, "softness": 0.08
  }
}
```

La distancia RGB normalizada a `color` determina la transparencia:
`tolerance` establece el umbral y `softness` suaviza el recorte.
Para escenas profesionales usar fuentes con un croma uniforme,
sin reflejos verdes sobre la piel ni el cabello. No equivale a un
recorte profesional basado en segmentación semántica.

Demostración autónoma en
`examples/motion-studio-native/project.json`:
tres escenas con símbolos, transiciones y una imagen recortada.
El proyecto está disponible a 1024×576 de diseño y se puede
renderizar a 848×480 y 24 FPS con `--size 848x480`.

## Estado y próximas extensiones

**Etapa B1 — implementada y validada: Lottie vectorial.**
La opción `HFC_THORVG=ON` enlaza ThorVG 1.2.0 por C++ estático.
El motor acepta una capa `type:"lottie"` con `src` JSON,
`x/y/width/height`, `rate` (0.1–5) y `loop` (booleano). El clip
se reproduce desde `start` o `cueIndex` cuando la escena es reactiva.
Los fotogramas se renderizan con ThorVG CPU y conservan alfa;
el sistema usa un caché pequeño por hilo y un límite de 1.9 MP/capa.

```json
{"type":"lottie","src":"assets/moving-orb.json",
 "x":100,"y":120,"width":480,"height":360,"rate":1,"loop":true}
```

Build opcional reproducible (desde la raíz, con Meson instalado):

```bash
git clone --depth=1 https://github.com/thorvg/thorvg.git third_party/_sources/thorvg
meson setup build/thorvg-native third_party/_sources/thorvg   --prefix="$PWD/third_party/_install/thorvg"   -Dengines=cpu -Dloaders=lottie,svg   -Dtests=false -Ddefault_library=static -Dstatic=true -Dextra=''
meson compile -C build/thorvg-native -j3
meson install -C build/thorvg-native
cmake -S . -B build/native-full -DHFC_THORVG=ON
cmake --build build/native-full --target hyperframes-cpp-native -j2
```

La biblioteca compilada y las fuentes quedan en carpetas locales
`third_party/_install/` y `third_party/_sources/` excluidas del
Git público. Sin ThorVG el motor básico sigue funcionando, pero
rechaza con error claro las capas `lottie`.
Los Lottie están restringidos a JSON menor o igual a 1 MiB dentro
del proyecto y no admiten imágenes externas enlazadas.

**Etapa B2 — pendiente:** SVG como capa independiente, presets de
animación, control de color y catálogo de personajes profesionales
con licencia de redistribución comprobada. Una animación Lottie
puede contener expresiones no compatibles con el motor; no garantizar
paridad total con After Effects.

**Etapa C: vídeo multicapa.** Integrar una edición LGPL de
`libavformat`, `libavcodec` y `libswscale` vía su API de C,
NO invocando el programa `ffmpeg` durante el render. Esto abre
MP4/WebM según decodificadores compilados. Cada clip necesitará
`start`, `trimIn`, `trimOut`, `loop`, `speed`, `gain`, `mute`,
`fit`, `chromaKey`, `alpha`, filtros y posición en la pila de capas.
Acotar resolución, duración, fps, bytes, memoria y número de decoders;
usar decoder caches por clip para el render paralelo. La instalación
actual NO incluye los encabezados de desarrollo necesarios.
Enlazar bibliotecas no equivale a añadir ejecutables externos.

**Etapa D: personajes articulados.** Evaluar Rive y personajes
importados con animaciones etiquetadas por estados
`happy`, `angry`, `speaking`, `thinking`; no representar una
expresión estática como animación articulada.

**Etapa E: autoría visual.** Editor de clips en pistas, recorte verde
en vivo con picker, curva de movimiento, biblioteca de recursos
indexada, validación de licencias, previsualización de cues y pruebas
automatizadas de sincronización.

## Precauciones

- El tiempo de cada palabra de Neo Lite sigue estimado; no existe
  todavía alineación fonética exacta.
- La salida profesional debe conservar atribución de iconos y material
  de terceros, y respetar los permisos de imagen y vídeo.
- El códec H.264/AAC nativo mantiene sus condiciones de patente.
- FFmpeg cambia los compromisos legales según la configuración
  LGPL/GPL; antes de distribuir, revisar el enlazado y obligaciones.
- En la V5 original el MCP permanece detenido. Este trabajo no cambia
  `main`, no limpia el árbol Git ni publica cambios en remoto.
