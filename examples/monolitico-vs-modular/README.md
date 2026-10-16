# GPTFrames — Ejemplo educativo: monolítico vs modular

Este ejemplo produce un video de **56 segundos, 848×480, 24 FPS**
que compara código concentrado en una misma función con responsabilidades
separadas por módulos. Incluye narración en español sintetizada con
el modelo local Lilith/Neo Lite.

## Matiz técnico importante

**Monolítico** suele describir una unidad de despliegue. **Modular**
describe cómo se divide el código en responsabilidades.
No son alternativas mutuamente excluyentes: **un monolito puede estar
bien modularizado**. El ejemplo deliberadamente presenta un monolito
poco modular para ilustrar el problema del acoplamiento, y aclara la
distinción en las escenas 1, 3 y 6.

## Contenido

Siete escenas de ocho segundos cada una:
1. Arquitectura monolítica frente a organización modular.
2. Una función Python que concentra cobro, stock y avisos.
3. Riesgos del acoplamiento al modificar una regla.
4. Refactorización en pagos.py, inventario.py y avisos.py.
5. main.py importa y coordina las funciones de cada módulo.
6. Monolito modular: módulos separados y un solo despliegue.
7. Conclusiones sobre separación de responsabilidades.

El HTML usa CSS determinista y el reloj CDP de Chromium.
No contiene scripts ejecutables ni recursos CDN. La voz se genera
con el binario compilado Neo Lite; los scripts Python se usan
únicamente para autoría y ensamblado de la vista previa.

## Producción

```bash
python3 examples/monolitico-vs-modular/build_narration.py
cd examples/monolitico-vs-modular
for i in 0 1 2 3 4 5 6; do
  n=$(printf '%02d' "$i")
  ../../build/neo-lite/neo-lite infer --model ../../models/Lilith.neo \
    --text-file "voice-$n.txt" --out "voice-$n.wav" --device cpu
done
cd ../..
python3 examples/monolitico-vs-modular/build_video.py
python3 examples/monolitico-vs-modular/prepare_audio.py
build/native-full/hyperframes-cpp-native --browser-widget-render \
  examples/monolitico-vs-modular/monolitico-modular.html \
  examples/monolitico-vs-modular/Monolitico_vs_Modular_SILENT_480p24.mp4 \
  56 24 848 480
ffmpeg -i examples/monolitico-vs-modular/Monolitico_vs_Modular_SILENT_480p24.mp4 \
  -i examples/monolitico-vs-modular/narracion-56s.wav \
  -i examples/monolitico-vs-modular/Monolitico_vs_Modular_subtitulos.srt \
  -map 0:v -map 1:a -map 2 -c:v copy -c:a aac -b:a 128k \
  -c:s mov_text -movflags +faststart \
  examples/monolitico-vs-modular/Monolitico_vs_Modular_480p24.mp4
```

El ajuste de tempo de cada narración deja 360 ms de margen para
la entrada visual de la escena sin cortar la voz. El archivo de
subtítulos es opcional y acompaña al MP4 final.

Las secuencias de código son **ilustrativas**: su objetivo es mostrar
la organización del software, no implementar un sistema de pagos real.
