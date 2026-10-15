# Compositor JSON nativo — HyperFrames C++

El modo `--project` convierte un archivo JSON en video vertical MP4 o en un fotograma PNG. Usa el mismo ejecutable que los modos clásicos de HyperFrames. **Cambiar una escena no requiere recompilar C++.**

## Ejecución

```bash
APP=./build/remix-linux/hyperframes-cpp-native
$APP --project validate examples/json-compositor/showcase.json
$APP --project frame examples/json-compositor/showcase.json escena.png 8
$APP --project render examples/json-compositor/showcase.json video.mp4
$APP --project render examples/json-compositor/showcase.json clip.mp4 2
```

`frame` toma un segundo preciso (puede contener decimales). El argumento opcional al final de `render` recorta la duración de salida. Windows utiliza las mismas opciones con `hyperframes-cpp-native.exe`.

## Esquema v1

```json
{
  "version": 1,
  "fps": 24,
  "canvas": {"width": 720, "height": 1280},
  "scenes": [
    {
      "start": 0, "duration": 5,
      "background": "#081629", "gradient": "#173044",
      "layers": [
        {
          "type": "text",
          "text": "VIDEO NUEVO",
          "x": 50, "y": 240, "width": 620, "fontSize": 64,
          "color": "#EFFFFA", "bold": true,
          "keyframes": [
            {"time": 0, "opacity": 0, "y": 360},
            {"time": 0.8, "opacity": 1, "y": 240, "easing": "back"}
          ]
        },
        {
          "type": "image", "src": "assets/logotipo.png",
          "x": 130, "y": 580, "width": 460, "height": 460,
          "keyframes": [
            {"time": 0, "rotation": -30, "scale": 0.6},
            {"time": 2, "rotation": 0, "scale": 1, "easing": "easeOut"}
          ]
        }
      ]
    },
    {
      "start": 5, "duration": 5,
      "background": "#28143E",
      "transition": {"type": "iris", "duration": 0.6},
      "layers": []
    }
  ],
  "audio": [{"src": "assets/musica.wav", "start": 0, "gain": 0.7}]
}
```

### Capas disponibles

- `text`: fuente DejaVu incrustada, UTF-8, saltos de línea, `fontSize`, `bold`, `underline`, `color`, anchura máxima, `opacity` y `scale`.
- `image`: PNG/JPEG con canal alfa, escalado bilineal, rotación, opacidad, animación y composición por orden de capas.
- `rect`, `ellipse`, `line`: formas rasterizadas con cobertura antialias en los bordes, rotación, escala, dimensiones y opacidad.
- Orden de profundidad: las capas posteriores del arreglo se dibujan encima de las anteriores.
- `start` y `duration` opcionales por capa; tiempo relativo a su escena.

### Keyframes

Cada capa puede tener un arreglo `keyframes` (máximo 128, tiempos estrictamente ascendentes). Las propiedades numéricas disponibles son: `x`, `y`, `width`, `height`, `scale`, `rotation`, `opacity`. Los keyframes sin propiedades nuevas conservan las del anterior. `time` es relativo al comienzo de la **capa**.

Easings: `linear`, `easeIn`, `easeOut`, `easeInOut`, `back`, `step`. El easing se declara en el keyframe de destino.

### Escenas y transiciones

Las escenas tienen duración y deben estar ordenadas y ser contiguas. `background` puede combinarse con `gradient` para un degradado vertical. Transiciones soportadas entre escenas: `cut`, `fade`, `wipe`, `iris`, `slide`. La duración de transición no puede superar la duración de ninguna de las dos escenas.

### Audio

Permite hasta 12 pistas PCM WAV mono, 24 kHz, 16 bits, desplazamiento temporal `start` y ganancia `gain` de 0 a 2. Las pistas se mezclan en memoria con limitación de amplitud y se exportan como AAC. No hay importación directa de MP3/M4A en este modo todavía.

### Límites y seguridad

- Diseño interno fijo **720×1280**; la opción `canvas` modifica el tamaño de salida mediante escalado bilineal. Por ahora se exige formato 9:16 y dimensiones múltiplos de 16 para el encoder H.264 incluido. Ejemplos: 288×512, 576×1024, 720×1280 y 864×1536.
- Duración máxima 240 segundos, 64 escenas, 100 capas por escena, 128 keyframes por capa, JSON de máximo 1 MB.
- Solo se permiten rutas relativas para medios, dentro del directorio del proyecto; no se permiten `..` ni enlaces simbólicos que salgan de él. PNG/JPEG de hasta 25 MB y tamaño máximo de decodificación limitado.
- El nuevo compositor es CPU y no incluye editor visual ni máscaras personalizadas, shaders GPU, Lottie, Rive, SVG o motor Three.js.
- La rotación se aplica a imágenes y geometría, **no al texto**; esta limitación se valida explícitamente.
- La salida MP4 integrada requiere `HFC_NATIVE_MP4=ON`. Para el modo `browser`, sigue siendo necesario un navegador.
- Se utiliza `nlohmann/json` (MIT) y `stb_image` (dominio público/MIT) que HyperFrames ya incluía; **no añadimos dependencias externas de ejecución**.

## Mejoras de gráficos v2

La nueva demostración editorial está en `examples/json-showcase-v2/showcase-v2.json` (cuatro escenas de seis segundos). El motor incluye además los tipos **glow**, **ring** y **grid**, además de `radius` y `strokeWidth` en tarjetas `rect` y propiedades de sombra tipográfica `shadowColor`, `shadowOpacity`, `shadowX` y `shadowY`. Los cambios no requieren nuevas bibliotecas. Véase [GRAPHICS_V2.md](GRAPHICS_V2.md).

## Validación y compilación cruzada

```bash
bash tests/json-compositor.sh
bash tests/remix-smoke.sh
bash tests/smoke-linux.sh
bash scripts/build-remix-offline.sh
```

Al distribuir binarios con H.264/AAC, revisar las obligaciones de patentes y las licencias en `THIRD_PARTY_NOTICES.md`. Los binarios Windows se compilan cruzados, pero su ejecución real se debe verificar en Windows.
