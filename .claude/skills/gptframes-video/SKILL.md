---
name: gptframes-video
description: Produce GPTFrames C++/Neo Lite/MCP videos with phonetic Spanish TTS, real layered motion graphics, licensed media, 30 FPS when requested, and full video QA. Use for new scenes, narration, animated charts, video rendering and MCP integrations.
---

# GPTFrames — Reglas de producción audiovisual (MCP + skill)

## Reglas que debes seguir SIEMPRE

1. **La voz se escribe COMO SE PRONUNCIA.** No envíes al motor TTS automáticamente el texto escrito para pantalla. Mantén por escena dos campos diferentes:
   - **display_text / title:** ortografía correcta para gráficos, marcas y código: Go, Rust, C++, HirCoir, SSH, comandos literales.
   - **spoken_text / speech_text:** texto separado adaptado al sonido para el TTS: «gou», «rast», «ce más más», «Jírcoir», «ese ese hache». Escribe también números, fechas, siglas, versiones y unidades de manera que suenen natural en el idioma seleccionado.
   - Estas son **aproximaciones**, no reglas fonéticas universales: generar muestras con la voz real (Capybara .neo, Piper u otra), escuchar, corregir los términos problemáticos y **regenerar** los audios antes de exportar el MP4. No afirmar que suena perfecto sin escuchar.
   - En MCP llama **prepare_narration** para propuestas. Llama **synthesize_voice** con **text=fonético corregido** y, si existe, **display_text=ortografía correcta**. Nunca modificar comandos ni código en pantalla por el diccionario fonético.

2. **No hacer diapositivas estáticas.** Cada escena debe animar componentes independientes, no limitarse a cambiar capturas o hacer un zoom global de una imagen. Requisito mínimo: **una capa con propiedades que realmente cambian por keyframes en cada escena**, preferencia 2–3 capas con entradas escalonadas y movimiento visible. Animar títulos, palabras resaltadas, fotos con parallax, tarjetas, gráficos de barras, contadores, diagramas, máscaras, tablas que entran por filas y transiciones. Los keyframes deben tener variación real de posición, opacidad, escala, giro o dimensión, no repetir valores.
   - Para JSON, **analyze_motion** examina cada escena y **validate_project** incluye ese análisis. **render_video** rechaza por defecto escenas estáticas. Solo permitirlas mediante **allow_static=true si el usuario las pidió expresamente**. Este analizador no entiende la animación externa HTML/GSAP, que debes comprobar visualmente por separado.
   - Para composición HTML, Chromium/GSAP o pipeline FFmpeg, animar **capas independientes** mediante timeline seekable (sin depender del reloj real) y verificar frames iniciales, intermedios y finales.

3. **Respetar el estilo solicitado.** Si el usuario pide horizontal 1920×1080 y 30 FPS, producir exactamente ese formato; vertical si pide shorts. En el documental Go/Rust/C++ actual: 30 FPS, sin marcas GPTFrames en el video, sin subtítulos de narración y cierre de suscripción a **HirCoir**. En proyectos nuevos no añadir/retirar marcas, subtítulos o CTA sin considerar la petición; no mostrar contador «página actual/total» salvo solicitud.

4. **Sonido y ritmo.** Medir WAV generado realmente por escena (no inventar tiempos), añadir pausas razonables y alinear títulos/transiciones con el discurso. No cortar frases ni acelerar de manera excesiva. Revisar nivel de audio, silencios, clipping y fonemas técnicos. La voz para GPTFrames/Neo Lite se prepara como mono PCM signed 16-bit 24 kHz, cuando se utilice la integración de audio nativa.

5. **Recursos legales y entrega.** Usar imágenes con autor, URL y licencia verificadas (MCP search_images y download_commons_image para Commons/Openverse según disponibilidad); atribuirlas donde corresponda. Usar música externa solo cuando se obtuvo legítimamente bajo licencia aplicable; nunca afirmar que una pista proviene de YouTube Studio sin descargarla y verificar condiciones. Exportar muestra corta, analizar cuadros y revisar video completo: resolución, FPS, duración, H.264/AAC, existencia o ausencia de subtítulos y errores de decodificación.

## Orden obligatorio en MCP

- get_production_guidelines
- storyboard separado en texto visible, texto hablado fonético y **qué se anima en cada escena**
- prepare_narration → corrección manual → synthesize_voice → escuchar WAV
- crear/editar JSON con keyframes verdaderos y medios con licencia
- analyze_motion + validate_project; resolver advertencias y estáticos
- preview_frame de inicio/medio/final (para HTML, capturas por fotograma)
- render_video y QA completo, entregar un video real, no un enlace inventado.

## Ejemplo de dos textos (no mostrar pronunciación fonética en pantalla)

```json
{
  "display_text": "Go, Rust y C++. Suscríbete a HirCoir",
  "spoken_text": "Gou, rast y ce más más. Suscríbete a Jírcoir"
}
```

## Keyframes JSON: ejemplo de animación real, no captura estática

```json
{
  "type": "text",
  "text": "¿Cuál es el rey del rendimiento?",
  "x": 100, "y": 220, "opacity": 0,
  "keyframes": [
    {"time": 0, "x": 100, "y": 260, "opacity": 0},
    {"time": 0.8, "x": 80, "y": 220, "opacity": 1, "easing": "easeOut"}
  ]
}
```

Complementar con otros elementos animados en la misma escena. No limitarse a una sola transición general.

## Capacidades y límites

El MCP (mcp/server.py) expone JSON/scene keyframes, voz Neo Lite y búsqueda de imágenes; por defecto el render MCP puede limitar clips a 60 segundos por llamada. El renderizador C++ clásico con CLI HTML/browser es una ruta diferente; no confundir formatos y herramientas. El documental 16:9 largo previo usó HTML/Chromium y composición FFmpeg para animar capas; no afirmar que el compositor JSON interno renderiza ese pipeline de manera nativa. Consultar docs/MCP.md y la skill hermana hyperframes-cpp para detalles del ejecutable nativo.
