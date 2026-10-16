# Motion explicativo sincronizado con la voz (obligatorio)

Aplicar a todo video narrado nuevo o revisado, salvo que el usuario pida expresamente una pieza estática. El nivel de referencia es una explicación construida ante el espectador: objetos, relaciones y resultados se revelan al oír el concepto. Un preset animado detrás de una diapositiva completa no cumple.

## 1. Diseñar estados, no diapositivas

- Escribir la narración final por escena y sintetizarla antes de fijar el montaje. Medir los WAV reales.
- Preparar un mapa de beats por frase/concepto: qué se oye, qué aparece, qué cambia, qué permanece y qué sale. En una escena explicativa de 10–15 s buscar normalmente 3–6 cambios con sentido; es una guía de ritmo, no una cuota de efectos. Conservar pausas útiles para comprender.
- Mantener un solo foco principal. Usar etiquetas breves, tipografía legible en móvil y espacio suficiente para objetos grandes. Mantener consistentes color, escala y dirección espacial.
- Dejar visible desde el arranque un contexto útil: pregunta, silueta, estructura o estado inicial. No dejar segundos vacíos esperando el primer cue.
- Distinguir contexto de respuesta: una estructura vacía puede ser necesaria si representa una matriz, chip o grafo; una tarjeta decorativa vacía no lo es. Ocultar los operandos, etiquetas o resultados que aún no se explican.
- Animar la explicación: desplazar paquetes hacia sus destinos, dividir una tarea, recorrer aristas, resaltar fila/columna, construir una geometría y actualizar un resultado. Entrar títulos con fade y repetir un loop no reemplaza estos cambios.
- Mantener los resultados el tiempo suficiente para leerlos. Retirar o atenuar el foco anterior antes de introducir texto que ocupe su posición.

## 2. Fijar los cues al audio real

Conservar un manifiesto editable `motion_events.json` (metadatos del proyecto, NO campos nuevos del esquema del motor). Registrar cada evento significativo con escena, elemento estable, frase/palabra de anclaje, inicio/fin local, tipo de acción y procedencia del tiempo. Ejemplo:

```json
{
  "scene": 5,
  "element": "result-19",
  "anchor": "diecinueve",
  "start": 3.16,
  "end": 12.4,
  "action": "reveal-result",
  "timing_source": "word-estimate-reviewed"
}
```

El ejemplo ilustra el formato; calcular sus tiempos con el audio actual. Añadir a los metadatos FPS, duración y huella/versión del WAV para detectar marcas obsoletas.

- Usar el alineador disponible como estimación inicial. La alineación por energía PCM no es reconocimiento fonético ni garantiza sincronización exacta. Ajustar escuchando las frases difíciles y mirando el render.
- Para operaciones críticas, sintetizar frases/cláusulas separadas y registrar sus offsets medidos si el alineador no resulta fiable. No insertar pausas artificiales excesivas para acomodar la animación.
- Usar segundos locales para la animación de cada escena. Convertir explícitamente: tiempo global = inicio de escena + inicio de pista dentro de escena + tiempo de palabra + ajuste editorial. No sumar dos veces el offset.
- Elegir un criterio comprobable: por defecto, comenzar la entrada en la palabra ancla o inmediatamente después; revisar diferencias de más de ~0.15 s. No confundir esa tolerancia editorial con precisión del alineador. Distinguir inicio de transición y estado ya legible.
- Cambiar voz, texto, recorte o velocidad del audio obliga a recalcular inicios acumulados, cues, duración y subtítulos. No reutilizar silenciosamente las marcas anteriores.
- No afirmar sincronización exacta sin una medición que lo demuestre.

## 3. Elegir e implementar recursos

1. Consultar inventario y buscar por concepto; inspeccionar IDs, movimientos y propiedades con `describe_animation`.
2. Reutilizar un recurso pertinente si permite los estados y tiempos requeridos. Un texto editable no cambia necesariamente la simulación ni el reloj interno. Comprobarlo.
3. Si el preset no permite sincronizar las fases, adaptarlo mediante código fuente autorizado y crear una variante revisada y registrada, conservando el original. No aceptar el loop genérico como solución final.
4. Usar CLI/HTML/CSS/SVG para tener control del montaje cuando el entorno lo permita. En MCP nativo JSON insertar únicamente `html_widget` registrados; `save_project` no registra HTML arbitrario. Documentar la raíz de recursos de las variantes privadas.
5. Si falta una animación esencial y no es viable crearla con los medios autorizados, pedir SOLO ese recurso: describir entrada/salida, estados, etiquetas, dimensiones, tiempos y restricciones. Continuar el trabajo independiente. No pedir a Gemini por rutina ni bloquear todo el montaje.
6. Mantener archivos fuente, generador, manifiesto, catálogo y audio en el proyecto editable. No exigir un servicio MCP para reproducir el render si hay motor CLI compatible.

### Reglas de implementación

- Hacer cada animación determinista y seekable al tiempo solicitado. Usar CSS/WAAPI pausadas o la ruta de timeline soportada por el motor. Evitar Date.now, temporizadores de tiempo real, azar sin semilla y dependencias CDN.
- Comprobar saltos hacia delante, hacia atrás y el mismo tiempo repetido; no depender de haber reproducido fotogramas previos.
- Revelar como grupo atómico tarjeta + etiqueta + icono + acento cuando constituyan una misma unidad. Separar operandos/pasos que se explican en momentos diferentes.
- Para paquetes móviles, reservar rutas fuera de títulos y leyendas. Mostrar los destinos y la relación causal. No ocultar números pequeños bajo partículas.
- Actualizar contadores discretos sin crossfade si genera dos cifras superpuestas. Sustituir captions secuenciales sin solaparlas.
- Usar slide corto (aprox. 12–25 unidades de diseño), fade breve o trazado de línea con propósito. Preferir cambios locales; evitar temblores y zoom global repetido.
- Para widgets de lienzo completo, preferir fondo/gradiente nativo y HTML transparente si el aspecto lo permite. Medir una muestra representativa antes del render completo; una captura SVG opaca de toda la pantalla puede costar mucho más. Mantener la composición final.
- Conservar veracidad: las comparaciones conceptuales no son benchmarks. No inventar velocidades o proporciones de hardware.

## 4. Patrones transferibles

| Explicación | Montaje esperado |
| --- | --- |
| CPU y GPU | Aparecen núcleos/unidades cuando se nombran; tareas viajan hacia ellos y se agrupan según el reparto explicado. |
| Multiplicar matrices | Mostrar operandos por frase; resaltar fila y columna; añadir cada producto y suma; revelar el resultado al pronunciarlo; poblar otras celdas después. |
| Geometría a imagen | Construir malla/triángulos; introducir fragmentos y colores en su frase; presentar el resultado final al hablar de la imagen. |
| Flujo de red o programa | Hacer avanzar el paquete/control por los nodos mientras se narra; resaltar la instrucción o estado activo. |
| Comparación o conclusión | Revelar cada alternativa al nombrarla; construir la relación final y mantenerla para lectura. |

Los ejemplos enseñan una gramática de edición, no obligan a copiar la paleta ni a usar chips en todos los temas.

## 5. Gate editorial y técnico antes de entregar

- Validar esquema y layout. Revisar inicio, mitad y final de TODAS las escenas, y muestras antes, durante y después de cada beat importante.
- En DOM/SVG comprobar opacidad efectiva incluyendo ancestros, transformaciones y clip; no basta con el estilo del nodo. Verificar que un elemento oculto antes del cue aparece después del tiempo de entrada.
- Comprobar con píxeles reales que estados de un mismo objeto cambian. Una imagen distinta por un cursor parpadeante no prueba motion explicativo. El lint nativo puede desconocer el layout interno de un widget.
- Revisar sincronización escuchando y viendo una muestra representativa con varios beats; después revisar el MP4 final, no solo PNGs. Una hoja de contacto verifica composición, no sincronía audiovisual.
- Bloquear entrega si el diagrama está completo desde el principio mientras la voz enumera sus partes, si el resultado se adelanta, si solo hay loops de adorno, texto solapado, cues sin eventos visibles o largos tramos muertos sin propósito.
- Esperar salida cero del render. Usar una ruta temporal ÚNICA por intento; comprobar que no queda otro proceso hijo escribiendo antes de reutilizar una salida cancelada. Publicar/reemplazar el MP4 final solo después de validarlo.
- Verificar duración, FPS, tamaño, número esperado de frames, audio y decodificación completa. Con FFmpeg usar `-v error -xerror` y comprobar salida/errores; guardar logs acotados. No asumir que código cero con errores de decodificación significa un archivo sano.
- Guardar evidencia de cues, muestras y QA. Entregar MP4 y fuente editable verificados; explicar limitaciones reales sin proclamar calidad estética por pasar tests.
