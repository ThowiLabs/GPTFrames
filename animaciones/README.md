# Animaciones — biblioteca de motion graphics

Repositorio: rama `experiment/browser-first-uiverse` de GPTFrames.

## Colección 01 + 02

La biblioteca contiene **23 animaciones HTML/CSS únicas** agrupadas por
categoría y consultables mediante las herramientas MCP `list_animations`
y `describe_animation`. Todo funciona offline en Chromium/CDP sin CDN,
Node, React ni scripts definidos por los presets.

**Colección 01 (01–06):** teléfono vibrando, palomita de éxito, candado
cerrándose, notificación emergente, ecualizador de audio y escáner biométrico.

**Colección 02 (07–23):** avatar neutral, feliz, pensando, enojado, gritando,
durmiendo; servidor Linux, automóvil, reloj analógico, tacómetro; campana;
cortina wipe, split diagonal, revelado iris; gráfica de barras,
loader radial neón y progreso shimmer.

El segundo archivo aportado incluía también variantes de teléfono,
palomita, candado y ecualizador. Fueron identificadas como funciones
repetidas y **no** se agregaron por segunda vez.

## Archivos

- `catalogo.json`: metadatos de 23 diseños numerados y sus categorías
- `presets/<id>.html`: un HTML autónomo por animación
- `catalogo.html`: demostración numerada de 57.5 segundos
- `build_collection.py`: genera los primeros seis diseños
- `source_curated_02.json`: versión depurada del código facilitado para
  los 17 diseños nuevos
- `build_additions.py`: reconstruye el catálogo de 23. Si no está el
  HTML temporal original, usa el JSON depurado del repositorio
- `ejemplo-capa-hibrida.json`: ejemplo de widget CSS sobre escena nativa

## Reproducción determinista

Cada animación CSS se calcula con su **tiempo local**:
`t_local = t_global - tiempo_inicio_del_módulo`.

Anteriormente el renderer adelantaba todas las animaciones desde el segundo
cero del catálogo, dejando la palomita, candado y notificación estáticos
cuando finalmente llegaba su pantalla. Se corrigió la temporización en
`browser_capture.hpp`, buscando el ancestro `.module` y leyendo
`main[data-segment-seconds]`; la propia animación de visibilidad sigue
controlada por el reloj global. La prueba debe comparar fotogramas dentro
del **mismo** segmento, no solo de segmentos distintos.

## Capa HTML sobre C++

```json
{
  "type": "html_widget",
  "collection": "animaciones",
  "preset": "padlock-closing",
  "x": 600, "y": 120,
  "width": 360, "height": 360,
  "start": 0, "duration": 2.5
}
```

Se seleccionan assets exclusivamente de un catálogo local confiable.
Las imágenes HTML se capturan como RGBA por CDP, y C++ las compone sobre
las otras capas usando alfa. En la versión experimental las capas HTML
no soportan rotaciones y el render emplea un navegador por widget activo.

## Comandos

```bash
python3 animaciones/build_additions.py
build/native-full/hyperframes-cpp-native --browser-widget-frame \
  animaciones/catalogo.html 3.4 animaciones/ejemplo.png 848 480

build/native-full/hyperframes-cpp-native --browser-widget-render \
  animaciones/catalogo.html animaciones/catalogo-480p.mp4 \
  57.5 24 848 480
```

**Importante:** el navegador antiguo en ejecución root aún utiliza
`--no-sandbox`; hasta aislarlo con un usuario dedicado no se deben
renderizar snippets HTML arbitrarios enviados por terceros. Los presets
de esta colección son HTML/CSS local previamente revisado y sin JS.

Código de la Colección 02 basado en el archivo entregado por el usuario,
no en código obtenido de UIverse u otros catálogos externos.

## Colección 04 — Linux Motion Pack (29 módulos)

Se añadió `build_linux_04.py`, que transforma el HTML de Linux facilitado por
el usuario en 29 presets CSS/HTML independientes. Se **omite** el diseño 26,
Debian Red Swirl. El catálogo suma 61 presets con categorías, tags y
movimientos indexados.

`build_gallery_linux_04.py` produce `catalogo-linux-04.html`; el vídeo
de evaluación `Linux_Motion_29_Presets_480p24.mp4` dura 58 s, con
resolución 848×480 y 24 FPS.

`../docs/LINUX_MOTION_HOT_RELOAD.md` documenta tanto el catálogo como
el sistema nuevo de actualización en caliente de recursos del MCP.
