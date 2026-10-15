# Gráficos v2 — calidad visual del compositor JSON

La demo JSON original era una prueba mínima de funcionamiento, no una composición editorial de referencia. Se sustituyó por `examples/json-showcase-v2/showcase-v2.json`, una pieza de cuatro escenas (24s) diseñada para poner a prueba las nuevas primitivas del rasterizador.

## Implementación C++17

Se añadieron tipos JSON:

- `glow`: luz radial suave procedimental, ajustable con `opacity` y `color`.
- `ring`: anillo antialias con `strokeWidth`, escalable y rotatorio.
- `grid`: retícula configurable mediante `strokeWidth` como separación en píxeles.
- `rect`: soporta `radius` y `strokeWidth` para tarjetas redondeadas y contornos.
- `text`: admite `shadowColor`, `shadowOpacity`, `shadowX`, `shadowY`.

Se conserva toda la sintaxis original; propiedades nuevas son opcionales. El compositor sigue siendo nativo C++ sin navegador ni dependencias nuevas. También se corrigió un error en la interpolación bilineal de PNG/JPEG.

El nuevo ejemplo muestra color grading oscuro, tarjetas y paneles, texto editorial, iluminación y retículas. Las figuras animadas ahora pueden combinarse con curvas `easing` y el sistema de transiciones existente.

## Pruebas

```bash
cmake --build build/remix-linux -j2
bash tests/json-compositor.sh
bash tests/json-showcase-v2.sh
./build/remix-linux/hyperframes-cpp-native --project render examples/json-showcase-v2/showcase-v2.json video-v2.mp4
```

## Límites

Esta mejora es **rasterización 2D procedural**, no un motor GPU ni una implementación completa de SVG/Lottie/Rive. Las tarjetas tienen bordes antialias aproximados y las sombras tipográficas son desplazadas, no un blur gaussiano real. Para un salto de fidelidad aún mayor es necesario integrar un motor gráfico profesional y una edición visual con previsualización en vivo. La prueba Windows ejecutable está pendiente; cross-build se realiza desde Linux.

## Compatibilidad

Se trabajó en `feature/showcase-json-v2`, rama derivada de `feature/compositor-json-nativo`, sin alterar `main`. Los comandos `--project`, `--remix`, `--fusion`, AVI/MP4, html, browser y news mantienen su interfaz.
