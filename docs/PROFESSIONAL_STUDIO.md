# GPTFrames Professional Studio (rama experimental)

Motor nativo **C++17**. El navegador se utiliza para la interfaz web y, cuando se solicite, para capturar contenido HTML. La inferencia de voz usa Neo Lite con modelos locales `.neo`. La composición JSON, auto-layout, animación, mezcla PCM, MP4 H.264/AAC, cola de trabajos y servidor HTTP no requieren Python, Node, FFmpeg ejecutable ni servicios externos de renderizado.

## Composición automática

Los proyectos v1 con capas absolutas siguen siendo válidos. Un objeto `group` puede contener otros grupos y capas. Sus hijos se resuelven a posiciones finales antes de dibujar.

```json
{
  "type": "group",
  "x": 100, "y": 200, "width": 520, "height": 240,
  "direction": "row", "padding": 24, "gap": 16,
  "justify": "center", "align": "center",
  "children": [
    {"type": "ring", "width": 110, "height": 110, "color": "#FFC777", "strokeWidth": 10},
    {"type": "text", "text": "Alerta", "fontSize": 48, "color": "#FFFFFF"}
  ]
}
```

- `direction`: `overlay` (capas superpuestas y centradas), `row` (horizontal), `column` (vertical), `grid` (columnas con `columns`).
- `align`: `start`, `center`, `end`, `stretch`. `justify`: `start`, `center`, `end`, `space-between`.
- `padding`, `gap`, `offsetX`, `offsetY`, `grow` y tamaños explícitos son opcionales; no se requieren coordenadas absolutas en hijos.
- Texto: `textAlign: left|center|right`, `verticalAlign: top|center|bottom`, `wrap: true`. El centrado horizontal se realiza midiendo los glifos de la fuente incorporada, no a ojo.
- Los grupos se pueden anidar hasta 8 niveles; la escena ya resuelta admite hasta 100 capas. No modifica el comportamiento de las capas absolutas anteriores.
- Formatos vertical 9:16 y horizontal 16:9. Probado en **1920 × 1080** con codificación H.264 nativa y tamaño exacto.

### Animación continua

```json
{"type":"text","text":"TÍTULO","x":140,"y":150,"width":700,"fontSize":70,
 "animation":{"type":"float","amplitude":10,"speed":0.45}}
```

`animation.type`: `float`, `pulse`, `orbit`, `sway`, `fade-in`, `slide-in` o `none`. `speed` controla los ciclos por segundo (o la velocidad de entrada); `amplitude` controla píxeles, grados o escala según el tipo. Los `keyframes` clásicos siguen funcionando y pueden combinarse con la animación procedural.

**Anclaje geométrico:** las animaciones de escala (`pulse`, `scale` en keyframes) usan el **centro original de la caja** como pivote, tanto para formas como para textos. Un círculo y su icono concéntrico mantienen exactamente el mismo centro al aumentar o reducir de tamaño. El zoom opcional de la cámara también transforma sus centros de forma coherente alrededor del centro del lienzo. La cámara no se aplica si se omite la propiedad `camera`; para escenas fijas use transiciones `fade` o `cut` en lugar de desplazamientos persistentes.

Cámara por escena:

```json
"camera":{"startX":0,"startY":0,"endX":-25,"endY":10,
          "startZoom":1,"endZoom":1.05,"duration":5,"easing":"easeInOut"}
```

### Auditoría de diseño

```bash
./hyperframes-cpp-native --project lint mi-proyecto/project.json
```

Devuelve JSON con avisos de texto desbordado, tamaño insuficiente, posible superposición de texto, salida del lienzo y bajo contraste. Un resultado sin avisos **no sustituye una revisión visual humana**.

Ejemplo con layout y cámara: `examples/pro-studio-1080p/project.json`.

## Renderizados con URL de progreso

En el MCP autenticado, usar `start_render` con `{"project":"mi-proyecto"}` o `{"project":"mi-proyecto","seconds":15}`. Responde inmediatamente:

```json
{"id":"ID_ALEATORIO","status":"queued","progress":0,
 "viewer_url":"https://dominio/renders/ID_ALEATORIO#access=TOKEN_PRIVADO"}
```

La URL abre la página integrada en el servidor C++. Muestra una barra de progreso real por fotogramas; una vez completado el trabajo, incorpora reproductor de video y descarga MP4. El botón Cancelar interrumpe el proceso nativo y elimina el archivo parcial.

El token se pasa en el fragmento `#access=` (no llega al servidor en el primer GET); el navegador lo intercambia mediante POST por una cookie `Secure`, `HttpOnly`, `SameSite=Strict`. Sin autorización, los endpoints de estado y video devuelven 403. No insertar el token en logs, capturas ni documentación. Duración del permiso: 24 horas.

- `render_status`: obtener estado por `job_id` dentro del MCP autenticado.
- `cancel_render`: cancelar por `job_id` dentro del MCP autenticado.
- `review_layout`: revisar riesgos visuales de un proyecto.
- `list_voice_models`: listar archivos de voz compatibles instalados.

Persistencia: `build/runtime/render-jobs/<id>/`, permisos privados, copia aislada de los recursos del proyecto, sin seguir enlaces simbólicos, máximo 512 MiB por captura de recursos; cola con un render activo y cuatro pendientes. Un reinicio conserva trabajos completados, marca interrumpidos los trabajos activos y revoca su proceso. El progreso procede de los fotogramas codificados, no de una estimación de tiempo.

**Importante:** el dominio público debe configurarse a través de `GPTFRAMES_PUBLIC_URL` o `GPTFRAMES_PUBLIC_URL_FILE`; el proceso HTTP permanece en loopback y su publicación HTTPS es decisión del operador. La URL se genera con el dominio configurado. `render_video` síncrono sigue disponible por compatibilidad.

## Pruebas

```bash
cmake -S . -B build/studio -DHFC_NATIVE_MP4=ON -DHFC_MCP=ON
cmake --build build/studio -j2
ctest --test-dir build/studio --output-on-failure
```

Verifican layout, compatibilidad MCP/OAuth, estado persistente, cancelación real, acceso no autorizado, cookies, reproducción del MP4 y funcionamiento del codificador nativo.

## Pendiente para versión estable

El editor visual drag-and-drop completo, subtitulado con alineación por palabras y efectos avanzados de audio/video todavía requieren trabajo. Esta rama añade los cimientos y los servicios funcionales, pero no equivale a la cobertura de un editor profesional maduro. También quedan las pruebas de Windows, presupuestos de render ampliados y revisión de licencias/patentes H.264/AAC.
