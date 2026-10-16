# GPTFrames — instrucciones editoriales para generar vídeo profesional

Utilizar estas instrucciones al autorar y revisar proyectos nuevos con herramientas MCP.
No equivale a sincronización fonética exacta ni sustituye a la revisión del vídeo.

## Prompt de dirección y control de calidad

Crea un documental tecnológico profesional, narrativo y legible, con composición
de aspecto editorial, transiciones visuales discretas, iconografía semántica y
sin movimientos globales innecesarios de cámara. El guion se narrará con WAV
generados por el modelo de voz local seleccionado.

1. Escribe primero el texto final de narración por escena y genera los audios.
   Después estima sus marcas por palabra con el alineador C++ y adjunta
   speechTiming, scene.reactive y cueIndex por concepto realmente mencionado.
   Aplica voiceOffset de la pista y un cueOffset breve para no anticipar contenido.
   La estimación por palabra NO garantiza precisión fonémica.
2. Cada tarjeta debe ser un grupo semántico atómico: fondo, borde/acento,
   icono y texto aparecen juntos en el mismo cue. Jamás muestres barras o
   paneles vacíos antes de que se pronuncie su contenido.
3. Conserva coordenadas LOCALES de los adornos: al convertir desde
   coordenadas absolutas al overlay, ancla los acentos a su borde mediante
   offsetX/offsetY. No centres una barra estrecha que era lateral.
4. Cuando aparezca otro elemento narrado, el grupo reactivo recalcula el
   layout row/column/grid y suaviza su redistribución; no muevas la cámara
   global ni tapes el texto. Evita superposiciones durante la transición.
5. Para motion graphics usa fade, slide-in corto (12–25 unidades de diseño)
   y entrada escalonada por concepto. Emplea transiciones entre capítulos
   de ~0.3 a 0.5 s. No animes acentos aislados ni uses iconos como ruido visual.
6. Iconos de Bootstrap Icons u otra fuente con licencia compatible deben
   conservar atribución. Los SVG pueden prepararse offline como PNG;
   durante el render final se cargan imágenes con el motor C++.
7. Mantén textos legibles en móvil y pantalla grande; comprueba contraste
   sobre la tarjeta efectiva y no solamente sobre el fondo de la escena.
   No reduzcas la fuente para ocultar desbordamientos.
8. Antes de entregar: validate, review_layout y previews al inicio y
   después de cada cue importante. Verifica que forma, texto e icono entren
   juntos, que no existan barras atravesando títulos, ni tarjetas vacías.
   Renderiza MP4 y verifica FPS, tamaño, H.264/AAC y decodificación.
9. Respeta la separación entre hechos confirmados, alegaciones y
   recreaciones. Nunca muestres datos personales auténticos sin autorización.

## Ejemplo de tarjeta reactiva

{
  "type": "group", "x": 900, "y": 260, "width": 700, "height": 240,
  "direction": "overlay", "align": "center", "cueIndex": 18,
  "animation": {"type": "slide-in", "amplitude": 20, "speed": 3.8},
  "children": [
    {"type": "rect", "width": 700, "height": 240, "color": "#19344F"},
    {"type": "rect", "width": 8, "height": 240, "color": "#61E3CA",
     "offsetX": -346},
    {"type": "text", "width": 590, "height": 110,
     "text": "CONTROL DE ACCESO", "fontSize": 44, "color": "#FFFFFF"}
  ]
}

El offset del acento es (0 - (700-8)/2) = -346. Los elementos de una misma
unidad se temporizan juntos, sin comenzar antes de cueIndex.

## Preview rápida reproducible

Desde la raíz del repositorio, con el motor ya compilado:

build/native-full/hyperframes-cpp-native --project render   projects/ia-programadores-telcel-v4/project_v5.json   projects/ia-programadores-telcel-v4/preview.mp4 --size 848x480

La vista previa mantiene el diseño 1920x1080 editable y aplica un escalado
uniforme, con salida 848x480 a 24 fps; esta anchura es compatible con
el codificador H.264 embebido.

## Motion Studio: nuevos recursos nativos

El compositor permite `symbol` (phone, check, lock, angry-person),
`image.chromaKey` y, con HFC_THORVG=ON, `lottie` con fuente JSON,
velocidad y repetición. Para objetos narrados, agrúpalos con el texto y
fondo en una unidad `reactive`, activada por `cueIndex` aproximado.
No insertes iconos genéricos si el guion requiere una representación
expresiva: utiliza un activo Lottie autorizado con movimiento real,
preferentemente compatible con ThorVG. Inspecciona distintos fotogramas
para comprobar que el activo se mueve y no invade los textos.

Los clips de vídeo MP4/WebM como capas y el croma aplicado a vídeo no
están disponibles todavía; jamás ofrecerlos como funcionalidades reales
hasta incorporar y verificar el decodificador C++ de medios.
Ver guía de uso, configuración y límites en docs/MOTION_STUDIO.md.

## Política de imágenes y entrega verificable

- Las imágenes nativas usan `fit: contain` por defecto. Para logotipos, terminales,
  pantallas y fotografías con sujetos importantes no uses `cover`, pues recorta
  contenido, ni `stretch`, que distorsiona proporciones.
- Una tarjeta opaca encima de una fotografía puede ocultar el recurso entero.
  Coloca la tarjeta junto o debajo, en una región espacial diferente.
- Antes de renderizar, usa `review_layout` o `--project lint` y ejecuta:
  `python3 tools/qa_video_project.py proyecto.json --engine build/native-full/hyperframes-cpp-native --out /tmp/gptframes-qa --strict`.
  Revisa tres capturas de cada escena y una mosaico, incluyendo las transiciones
  y los momentos en que aparecen textos narrados.
- Los indicadores de calidad no verifican automáticamente la estética, la
  exactitud de cada código o el contenido real de una imagen. Hace falta una
  inspección editorial del MP4 final.
- La skill canónica es `.claude/skills/hyperframes-cpp/SKILL.md`;
  contiene el inventario actual de 23 herramientas MCP, selección de presets,
  recursos CSS/HTML, modelos Neo Lite, límites y checklist de calidad.
