# GPTFrames — Colecciones 06, 07 y 08 (30 animaciones)

Se importaron **tres HTML completos** del usuario, con diez componentes por archivo:

| Colección | Archivo original guardado | Categoría |
|---|---|---|
| 06 | `animaciones/source_architecture_06.html` | Arquitectura de software |
| 07 | `animaciones/source_runtime_07.html` | Programación y ejecución |
| 08 | `animaciones/source_languages_08.html` | Lenguajes de programación |

Se conservan las **65 animaciones anteriores** y se agregan **30 presets nuevos**,
numerados del **66 al 95**, para un total de 95 recursos registrados.

## Índice

### Arquitectura de software (66–75)

1. Arquitectura Monolítica
2. Arquitectura Modular
3. Monolito Modular
4. Microservicios
5. Arquitectura Cliente-Servidor
6. Arquitectura MVC
7. Arquitectura en Capas (N-Tier)
8. Arquitectura Hexagonal
9. Arquitectura Orientada a Eventos
10. Refactorización Limpia

### Programación y ejecución (76–85)

1. Editor y Tokens
2. Pipeline de Compilación
3. Intérprete Línea por Línea
4. Debugger y Breakpoints
5. Call Stack (Pila de Llamadas)
6. Recursividad y Desbobinado
7. Memoria Stack vs Heap
8. Variables y Referencias
9. Bucles e Iteraciones
10. Manejo de Excepciones

### Lenguajes de programación (86–95)

1. Python
2. JavaScript
3. TypeScript
4. Go
5. Rust
6. C
7. C++
8. Java
9. C#
10. SQL

## Cómo funciona

Los tres originales son aplicaciones JavaScript con actualizaciones de SVG mediante
`requestAnimationFrame`, controles de avance y estado editable. Para integrarlas
sin ejecutar código JS arbitrario al renderizar, el importador utiliza:

1. `extract_snapshot_sources_06_08.cjs`: parser Acorn para localizar solamente
   la declaración del array de módulos; evaluación restringida con timeout en
   un VM local de Node, durante la **generación**, para materializar estados SVG.
2. `snapshot_sources_06_08.json`: artefacto temporal de 30 especificaciones
   y **158 estados visuales**; no es necesario en tiempo de ejecución.
3. `build_collections_06_08.py`: genera presets HTML/CSS/SVG totalmente
   autocontenidos y 3 galerías por categoría, sin scripts externos o incrustados.
4. `indexar_tags.py`: actualiza los tags y facetas de búsqueda del MCP.

Los archivos finales se pueden usar con el binario compilado de GPTFrames
y Chromium. Los pasos de generación utilizan Node y Python como herramientas
de **autoría**, no como dependencias del renderizador en producción.

Cada transición CSS se activa por un timestamp determinista que GPTFrames
sitúa desde su compositor browser-first. No espera temporizadores de la
página ni requiere un `requestAnimationFrame` en tiempo real.

## Propiedades editables

En cada recurso se pueden editar el nombre (`title`), descripción
(`description`) y nombre de archivo (`file`). Los identificadores y
valores de los códigos de ejemplo que sean texto visible se exponen con
`data-field` y etiquetas de tipo `text` en el catálogo. Cambiar esos
textos altera las etiquetas visibles en los snapshots, **no recalcula
la geometría ni la lógica semántica** de los estados exportados. Para
crear una simulación nueva con otras variables u otras condiciones,
se cambia la configuración del archivo fuente y se regeneran los estados.

Ejemplo de recurso:
```json
{
  "type": "html_widget",
  "collection": "animaciones",
  "preset": "language-python",
  "x": 130, "y": 120, "width": 800, "height": 460,
  "properties": {"title": "Python", "classname": "DataPipeline"}
}
```

## Vista previa

Cada galería tiene 10 escenas de 4 segundos y un total de **40 segundos**.

```bash
node animaciones/extract_snapshot_sources_06_08.cjs
python3 animaciones/build_collections_06_08.py
python3 animaciones/indexar_tags.py

build/native-full/hyperframes-cpp-native --browser-widget-render \
 animaciones/catalogo_architecture_06_08.html \
 animaciones/Arquitectura_Software_10_480p24.mp4 40 24 848 480
build/native-full/hyperframes-cpp-native --browser-widget-render \
 animaciones/catalogo_runtime_06_08.html \
 animaciones/Programacion_Ejecucion_10_480p24.mp4 40 24 848 480
build/native-full/hyperframes-cpp-native --browser-widget-render \
 animaciones/catalogo_languages_06_08.html \
 animaciones/Lenguajes_Programacion_10_480p24.mp4 40 24 848 480
```

Los tres MP4 pueden concatenarse sin recomprimir mediante FFmpeg para
obtener un **catálogo único de 120 segundos**. Formato: 848×480, 24fps.

## Controles de calidad y seguridad

- Ningún preset generado carga CDN o bibliotecas remotas.
- CSP local: `default-src 'none'`, `script-src 'none'`, `connect-src 'none'`.
- El source JS de los HTML originales se mantiene únicamente como referencia
  y nunca se ejecuta dentro de las plantillas distribuidas.
- Títulos largos se truncan dentro del inspector sin superponer tarjetas.
- La visibilidad del **módulo padre** se anima con opacidad y visibilidad:
  evita que los hijos SVG de otras escenas reaparezcan.
- Los diagramas son ilustraciones educativas; no ejecutan código ni comandos
  reales ni realizan transacciones de red.
- Las pruebas del MCP verifican 95 recursos, tres categorías de diez,
  identificación única y campos de texto editables.
- No hace falta reiniciar el MCP para descubrir archivos y registros nuevos
  una vez activado el binario de resource-monitor, siempre que solo cambien
  los assets y no el programa compilado.

Los cambios se conservan en `experiment/browser-first-uiverse`; no se
reemplazan versiones anteriores.
