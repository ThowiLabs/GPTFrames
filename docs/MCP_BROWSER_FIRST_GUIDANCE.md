# GPTFrames MCP: instrucciones Browser-First para el agente

## Comportamiento anunciado por el servidor

Los clientes MCP que se conectan a GPTFrames ahora reciben el texto de
`mcp/authoring_instructions.hpp` en la propiedad `instructions` de:

- `initialize` (protocolo MCP tradicional).
- `server/discover` (descubrimiento moderno de 2026-07-28).

El texto explica la prioridad de las animaciones HTML/CSS/SVG revisadas
sobre los bloques nativos genéricos, sus límites y el proceso correcto
de producción (inventario, guion, búsqueda, descripción, integración
`html_widget`, sincronización, validación, previsualización y render).

Las descripciones de herramientas exponen la misma orientación para
`resource_status`, `list_animation_facets`, `search_animations`,
`list_animations`, `describe_animation`, `create_project`,
`save_project`, `validate_project`, `preview_frame`,
`review_layout`, `render_video` y `start_render`.

`create_project` conserva una plantilla nativa compatible por
retrocompatibilidad; devuelve un `tip` que advierte al agente que debe
reemplazar los visuales de ejemplo por presets HTML elegidos. No es
correcto afirmar que la plantilla de ejemplo tenga automáticamente
animaciones HTML. El agente debe añadir las capas válidas al
`project.json` con `save_project` y verificar fotogramas reales.

## Limitación de las instrucciones

Esto es una **guía de trabajo para agentes**, no una condición técnica
que obligue a cada render a contener HTML. El servidor valida que los
presets se encuentren registrados y que sus propiedades sean permitidas,
pero un vídeo puramente nativo todavía se admite: sigue siendo necesario
para proyectos legítimos sin presets relevantes.

En una próxima fase se podría agregar una validación editorial optativa,
como `require_browser_first=true` por proyecto, con un informe de los
presets usados, sin bloquear contenidos que no requieran HTML.

## Activación

El cambio modifica código C++ compilado del servidor. Para que una
instancia MCP ya iniciada publique el nuevo mensaje, debe ejecutarse
la nueva versión binaria. Reiniciarla invalida los tokens OAuth de
acceso que solo viven en memoria; **el cliente puede necesitar reconectar**.

No cambies la URL pública, el PIN ni el estado de registros OAuth,
ni reinicies el túnel HTTPS. Preserva los archivos privados del directorio
`build/runtime/gptframes-auth/`.

Los tests en `tests/mcp_test.cpp` comprueban que el campo
`instructions` aparece en ambos protocolos y que los metadatos de
las herramientas principales anuncian el flujo Browser-First.
