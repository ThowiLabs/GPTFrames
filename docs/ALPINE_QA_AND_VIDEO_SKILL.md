# GPTFrames — Auditoría visual de vídeo Alpine/Docker

Fecha: 2026-10-16. Archivo suministrado por el usuario: vídeo vertical
Alpine Linux / Docker (720x1280, 24fps). Proyecto en sandbox:
`projects/alpine-linux-contenedores-short/project.json`.

## Hallazgos

- El compositor usaba `image.fit=cover` como predeterminado: rellena las
  cajas recortando imágenes con relación de aspecto distinta. Afectaba al
  logo de Docker (682x163) situado en un recuadro 260x112 y a las fotografías.
- En la escena 3 se dibujaba un rectángulo opaco sobre una foto del rack:
  tapaba más del 55% de su superficie. El texto y la foto compartían área.
- En la escena 6 una foto de servidores quedaba confinada a 385x145 en
  un lienzo vertical de 720x1280, con valor visual marginal.
- En la escena 9 una tarjeta superpuesta ocultaba el número central y
  el texto explicativo sobresalía por un borde.
- Los dos titulares de la mayoría de las escenas se superponían.
- Muchas etiquetas tenían solo 18–25 px de fuente en el original.

El informe de layout inicial mostró **77 advertencias**: 60 `small_text`,
15 `text_collision`, una `image_occluded` y una `outside_canvas`.

## Cambios al motor

1. `fit=contain` es el nuevo predeterminado para imágenes nativas. Conserva
   proporción y todos los bordes, con espacios transparentes cuando no coinciden.
2. `fit=cover` y `fit=stretch` siguen disponibles de forma explícita.
3. `image_fit.hpp` encapsula el cálculo y está cubierto por
   `image-fit-preserve` (CTest).
4. `--project lint` avisa `image_cropped` si cover pierde más del 20%
   del original, `image_distorted` si stretch modifica demasiado la relación
   de aspecto y `image_occluded` si una forma opaca posterior tapa más del 55%.
5. `tools/qa_video_project.py` crea informe JSON, tres fotogramas
   por escena y mosaico. `--strict` falla si persisten alertas críticas.

## Proyecto de ejemplo corregido (sin modificar el original)

El proyecto del usuario sigue intacto. La variante se guardó en:

`projects/alpine-linux-contenedores-short/project_visual_v2.json`

- `fit=contain` explícito en todos los recursos.
- Se corrigió la distancia entre títulos de las doce escenas.
- Escena 3: rack separado verticalmente de tarjeta + explicación.
- Escena 6: fotografía de datacenter ampliada.
- Escena 9: dato `~8 MB` y panel explicativo separados.
- Escenas 1 y 4: logotipo Docker más grande, sin corte.
- Tamaño mínimo de fuente de 27 px para etiquetas legibles.

Después, la auditoría completa a **36 fotogramas (3 × 12)** detectó
**cero advertencias**; no se presume que la estética esté aprobada ni que
cada pixel de cada fotograma haya sido inspeccionado. El mosaico final
debe ser revisado por el propietario.

Repetición de QA:

```bash
build/native-full/hyperframes-cpp-native --project lint \
  projects/alpine-linux-contenedores-short/project_visual_v2.json

python3 tools/qa_video_project.py \
  projects/alpine-linux-contenedores-short/project_visual_v2.json \
  --engine build/native-full/hyperframes-cpp-native \
  --out /workspace/alpine-visual-v2-all-qa \
  --strict
```

El proyecto del usuario y las imágenes de revisión se mantienen fuera
del control de versiones del motor. Los cambios de C++ y de las
instrucciones de la skill sí se versionan para futuros trabajos.

## Skill mejorada

La skill canónica `.claude/skills/hyperframes-cpp/SKILL.md` contiene
inventario de herramientas MCP, fuentes disponibles, TTS, browser/CDP,
presets, reglas de recorte, guion sincronizado, pruebas de layout y
criterios de aceptación editorial. Referencias nuevas:
`references/QUALITY_GATE.md` y `references/ASSET_INVENTORY.md`.

La skill se copió a `/root/.claude/skills/hyperframes-cpp/` y
`/root/.agents/skills/hyperframes-cpp/` para futuras sesiones.
El MCP en ejecución no se reinició; los cambios en las descripciones
compiladas de herramientas se verán cuando se despliegue su siguiente
versión.
