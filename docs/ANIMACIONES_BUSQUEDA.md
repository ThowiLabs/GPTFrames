# Búsqueda inteligente y presets editables — GPTFrames

## Índice de recursos

`animaciones/catalogo.json` contiene 32 recursos, clasificados por:
- `category`: área del catálogo (Hardware, Radiofrecuencia, Noticias...)
- `type`: objeto representado (servidor, antena, procesador, cintillo...)
- `tags`: términos en español, inglés y sinónimos de uso
- `motion`: efectos detectados en CSS y revisados (rotación, parpadeo, ondas...)
- `css_keyframes`: nombres reales de animaciones `@keyframes` del archivo
- `usage`: contextos editoriales donde recomendar un asset
- `editable`: propiedades disponibles y tipo de dato, por preset

El archivo `animaciones/indexar_tags.py` analiza el CSS guardado, combina
movimientos identificados por expresiones regulares con términos semánticos
revisados a partir de los componentes y escribe el índice en JSON.

No implica reconocimiento perfecto del propósito del código: la detección
de movimientos es heurística. Los términos semánticos se revisan al crear
el preset. No se cargan snippets externos por medio del buscador.

## Herramientas MCP

- `list_animations`: lista recursos por `category`, `type`, `tag`,
  `motion` y `limit`.
- `search_animations`: búsqueda textual combinada, ranking reproducible,
  filtro por facetas, `limit` y resultados con metadata y campos editables.
  Las palabras de consulta deben coincidir con el mismo asset para que este
  sea devuelto. Búsquedas con tildes o sin ellas se normalizan.
- `list_animation_facets`: enumera valores de categoría, tipo, tags y
  movimientos, con cantidades para construir filtros de UI.
- `describe_animation`: devuelve el preset exacto por ID.

### Ejemplos de argumentos

```json
{"query":"servidor linux","type":"servidor","limit":6}
```

```json
{"query":"ondas rf","category":"Radiofrecuencia"}
```

```json
{"query":"titular noticias","tag":"cintillo"}
```

```json
{"type":"procesador","limit":10}
```

Las etiquetas son datos, no componentes ejecutables, y no tienen permisos
para enviar HTML/JS arbitrario a Chromium.

## Configuración visual editable

Los nuevos componentes extraídos del HTML del usuario son:
24 RF transmisión, 25 RF recepción, 26 navegador, 27 teléfono con imagen,
28 Linux torre, 29 Windows Server, 30 CPU, 31 laptop, 32 cintillo.

La declaración del preset incluye su esquema `editable`; el proyecto
solicita valores con `properties`.

Ejemplo:

```json
{
  "type":"html_widget",
  "collection":"animaciones",
  "preset":"editable-cpu",
  "x":560,"y":140,"width":410,"height":360,
  "properties":{
    "cpu_name":"Intel Core i5",
    "cpu_model":"14600K",
    "cpu_socket":"LGA1700"
  }
}
```

- Los campos de texto se aplican al DOM con `textContent`, no
  `innerHTML`; no permiten código de usuario.
- Los campos de imagen se especifican mediante ruta local
  `assets/archivo.png`, `.jpg`, `.jpeg` o `.webp` dentro de la carpeta
  del proyecto. El compositor valida la ruta y la codifica como URL
  `data:` para Chromium sin acceso a internet.
- Los campos de color admiten exactamente `#RRGGBB`; el preset RF
  expone `signal_color`.
- El motor verifica que cada campo esté declarado en `editable`
  y limita la longitud del texto. Los assets sin campos editables
  mantienen sus animaciones propias.
- La línea de tiempo CSS sigue controlada desde Chromium mediante CDP,
  mientras el compositor nativo dibuja las demás capas y genera MP4.

### Reconstrucción del catálogo (herramientas solo de autoría)

```bash
python3 animaciones/build_collection.py
python3 animaciones/build_additions.py
python3 animaciones/build_editable_03.py
python3 animaciones/indexar_tags.py
python3 animaciones/build_gallery_03.py
```

El vídeo generado no requiere Python ni Node: utiliza el binario de
GPTFrames y el navegador instalado.

### Vídeo de inspección de la Colección 03

`animaciones/catalogo-editable-03.html` presenta los nueve diseños
con nombre, descripción, tags y los campos editables que exponen.
Se produce una vista previa de 31.5 s en 848x480 a 24 fps mediante:

```bash
build/native-full/hyperframes-cpp-native --browser-widget-render \
  animaciones/catalogo-editable-03.html \
  animaciones/Animaciones_Editable_03_480p24.mp4 31.5 24 848 480
```

## Seguridad

Actualmente el entorno de pruebas puede ejecutar Chromium como root
sin sandbox del navegador. El modo de HTML arbitrario de terceros
**no está habilitado en el MCP**: se utilizan únicamente presets locales
revisados y listas blancas de propiedades. La importación de plantillas
desconocidas requiere un proceso de aislamiento separado antes de
considerarse segura para el público.

## Corrección del cintillo — movimiento visible

El módulo `breaking-news-ticker` se corrigió para evitar que una
marquesina de 18 segundos quede fuera de cámara durante su aparición
de 3.5 segundos en el catálogo. Ahora:

- Las noticias comienzan a verse desde el primer fotograma y avanzan de
  derecha a izquierda con el `@keyframes newsCrawl` de 12 segundos.
- Dos copias idénticas del texto se desplazan `-50%` de la anchura total,
  para obtener un bucle continuo sin saltos.
- Ambos elementos exponen `data-field="ticker"`. El motor Chromium/CDP
  actualiza **todas** las coincidencias de ese campo (nunca `innerHTML`),
  preservando la edición de noticias desde JSON.
- La última escena de `catalogo-editable-03.html` tiene una presentación
  horizontal más amplia para resultar legible a 480p.
- La vista previa independiente `demo-cintillo-corregido.html` presenta el
  cintillo durante ocho segundos a 24 FPS.

`ticker-motion-css` es un test opcional para CTest que captura dos tiempos
de un mismo preset, compara solo los píxeles de la franja inferior y falla
si el contenido no cambia de posición. Además, `mcp-native` sigue pasando
para las 32 animaciones y sus herramientas de búsqueda.
