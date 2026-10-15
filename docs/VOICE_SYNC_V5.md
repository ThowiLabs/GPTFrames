# GPTFrames Studio V5 — composición vinculada a la narración

## Compatibilidad y arquitectura

El compositor sigue siendo C++17; no necesita Python ni Node.js para cargar, diseñar, sintetizar o renderizar. El audio lo proporciona Neo Lite y un modelo local `.neo`. El módulo `gptframes-word-align`, también C++17, toma la transcripción original y un WAV por capítulo para generar el archivo de sincronización. Las fotografías, formas y textos se rasterizan dentro del motor nativo.

Las escenas antiguas continúan usando capas con posiciones absolutas: el modo reactivo es optativo por escena.

## Sincronización con Lilith

1. Generar el audio de cada capítulo a partir del **texto exacto** que se va a narrar.
2. Exportar tiempos aproximados de palabras con el ejecutable C++:

```bash
./gptframes-word-align editorial_script.json assets/audio word_timings_v5.json
```

El archivo de entrada es `{"parts":[{"index":1,"text":"..."},...]}`; los WAV se numeran `01.wav`, `02.wav`, etc. El alineador acepta WAV PCM16 mono a 8–192 kHz.

El JSON de salida marca palabras (`start`, `end`) en segundos **relativos a cada clip**. La duración del WAV es exacta, pero los tiempos por palabra son **estimados** con el número de caracteres y los valles de energía del audio (ventanas de unos 10 ms). No es reconocimiento automático, *forced alignment* ni tiempo exacto de una letra. Una traducción o edición del guion exige regenerar el archivo. La mejora fonética requiere una exportación compatible de las duraciones del modelo, aún pendiente.

En el JSON del proyecto, añadir `"speechTiming":"word_timings_v5.json"`, y a cada escena que utilice eventos de voz `"reactive":true`. Un grupo con `"cueIndex":17` aparecerá cuando se alcance la palabra número 18 de la escena correspondiente; `"cueOffset":0.05` retrasa la entrada cinco centésimas adicionales. El motor aplica el desplazamiento de inicio de audio por defecto de 100 ms que utiliza la plantilla V5.

```json
{
  "reactive": true,
  "layers": [
    {
      "type": "group",
      "x": 120, "y": 300, "width": 620, "height": 220,
      "direction": "overlay",
      "align": "center",
      "cueIndex": 17,
      "children": [
        {"type":"rect","width":620,"height":220,"color":"#163249","radius":22},
        {"type":"text","width":540,"height":90,"fontSize":45,
         "text":"VALIDACIÓN DE IDENTIDAD","color":"#FFFFFF"}
      ]
    }
  ]
}
```

El texto y la forma comparten un único evento. Un grupo completo que todavía no se menciona **no participa del layout**, evitando paneles vacíos. La posición de los hermanos activos se vuelve a calcular en cada fotograma, y si aparece un hermano, los anteriores se redistribuyen durante 280 ms, sin mover toda la cámara.

Los eventos reactivados pueden componerse con grupos `row`, `column`, `grid` u `overlay`. Las capas fijas y sus keyframes siguen siendo compatibles. Por ahora los marcadores temporales por palabra deben inspeccionarse en escenas críticas; la identificación semántica de conceptos complejos no es totalmente automática.

## Imágenes con recortes y bordes

El elemento `image` admite ahora:

```json
{"type":"image","src":"assets/photos/archivo.jpg",
 "x":760,"y":180,"width":640,"height":430,
 "shape":"rounded","radius":28,
 "borderWidth":4,"borderColor":"#69DCCF","fit":"cover"}
```

- `shape`: `rect`, `rounded`, `circle`, `ellipse`.
- `radius`: radio de esquina para el recorte redondeado.
- `borderWidth`: grosor del borde en píxeles de diseño (0–70).
- `borderColor`: `#RRGGBB`.
- `fit`: `cover` (recorta al centro sin distorsionar la proporción) o `stretch` (se ajusta a la caja).
- Recorte, suavizado de bordes y rotación se calculan en C++, sin navegador.

## Proyecto de demostración

`projects/ia-programadores-telcel-v4/project_v5.json` reutiliza las doce escenas, fotografías y voces de la V4. El editor offline `build_v5.py` **solo prepara el JSON una vez**; no es dependencia de GPTFrames para ejecutar o renderizar. V5 incluye un checklist que se va llenando al pronunciar cada prueba, tarjetas de riesgo que se redistribuyen, paneles que aparecen junto con su texto, un ejemplo de JSON guiado por la narración y fotografías con recortes redondeados, elípticos y bordes.

## Pruebas

```bash
ctest --test-dir build/native-full --output-on-failure
./hyperframes-cpp-native --project validate projects/ia-programadores-telcel-v4/project_v5.json
./hyperframes-cpp-native --project lint projects/ia-programadores-telcel-v4/project_v5.json
```

La prueba `reactive-voice-layout` comprueba que los cuadros y sus textos se muestran juntos, que no se anticipan al evento y que el reacomodo termina suavemente.

## Trabajo posterior

- Exportación de duraciones por fonema desde un nuevo modelo Piper/Neo compatible, sin modificar el contenedor firmado de los modelos ya instalados.
- Ajustador manual de marcas de palabra y editor visual en vivo; por el momento los cambios se previsualizan por fotograma.
- Mejor análisis de colisiones visuales durante las fases de reacomodo.
- Tests automatizados de percepción/claridad de la narración y reconciliación entre guion y WAV.
