# CONTEXTO DE CONTINUIDAD — GPTFrames / Neo Lite / Studio V5

**Actualizado:** 2026-10-15 19:50 UTC.
**Propósito:** El usuario va a abrir un chat nuevo. LEER ESTE ARCHIVO ANTES DE SEGUIR. Es un resumen operativo del trabajo realizado en el proyecto Yah.
**Conector obligatorio para operaciones:** **[$slack](app://asdk_app_6ad0ee0d906081918e820a4050541f25)**, NO $PLS/$Pl/$Pl2. Aunque sus herramientas `mcp__Slack___*` se anuncien internamente como Lilith, el usuario lo identifica como $slack. Usar exclusivamente ese conector para acceder al sandbox.

## 1. Entorno y repositorios

- **Sandbox GPTFrames existente:** `ly5QOxFKvGjynGz6r3Cy60jf`, 3 vCPU, 6144 MB RAM, almacenamiento persistente, /workspace.
- **Repositorio privado:** `ThowiLabs/GPTFrames` clonado en **`/workspace/GPTFrames`**. Rama actual: **`feature/professional-studio-native`**. El usuario pidió mantener `main` intacto hasta verificar. Numerosos cambios están **sin commit y sin push**. **NO realizar reset, checkout forzado, clean ni borrar nada.** No asumir que el trabajo está subido a GitHub.
- Código y biblioteca Neo TTS: `/workspace/neo-tts`; consultas/otros clones: `/workspace/neo-tts-inspect`, `/workspace/Neo-TTS-App-inspect` y `/workspace/Piper-Neo-inspect`.
- Motor C++17: `/workspace/GPTFrames/build/native-full/hyperframes-cpp-native`; binario del MCP `build/native-full/gptframes-mcp`. `build/neo-lite/neo-lite` y `build/neo-lite/neo-compiler`. Voz nativa `scripts/neo-lite-local.sh`.
- **El servidor MCP de GPTFrames está DETENIDO por petición previa. No reiniciarlo sin consentimiento.** Puerto anterior 18769; las comprobaciones HTTPS del MCP no deben darse por activas. Hay un servicio **separado** de descargas HTTP en puerto 8765 y un túnel Gradio temporal; NO es el MCP.
- El modelo de voz se instaló en `/workspace/GPTFrames/models/lilith_epoch9804.neo` y existen alias (hardlinks) `lilith.neo`, `Lilith.neo`. Modelo Piper de Lilith en español latinoamericano, extraído del repo HF `HirCoir/piper-checkpoint-es-mx-lilith`, checkpoint `epoch=9804-step=4115380.ckpt`, de ~30 MB en formato .neo. Voz de un solo hablante; Neo Lite sintetiza sin Python ni LibTorch. LibTorch se necesitó solo para exportar el CKPT a ONNX en C++.
- El MCP conectado previamente como **[$pr](app://asdk_app_6ad0fd17b6848191896491fbc89b226b)** se probó con éxito: listar/crear proyectos, sintetizar Lilith, vista previa y render MP4. No es necesario para ejecutar render local.

## 2. Objetivo del usuario

GPTFrames debe crear vídeos profesionales de manera **nativa C++**, sin depender de Python, Node, ejecutables externos de renderizado ni servicios de render terceros (salvo navegador opcional para HTML y modelo local de voz). Se requiere diseño visual profesional, imágenes abundantes, autoalineación de elementos, formas y bordes, animación bien anclada, narración y gráficos sincronizados por voz, proyecto editable, control de calidad y servidor futuro con trabajo de render y visor de progreso.

Video de prueba: **«¿Por qué la IA aún no reemplaza a los programadores? — El caso de Telcel y el registro de líneas en México»**. Formato horizontal **1920×1080**, 24 FPS, **SIN música**, narrado por Lilith, despedida con invitación a suscribirse. En el guion NO afirmar que la IA causó la vulnerabilidad; separar denuncias de exposición en respuesta JSON y postura de Telcel que negó evidencia de filtración. Nunca incluir datos personales auténticos en recreaciones.

## 3. Trabajo completado previamente — Studio y V1–V4

- Producidos V1–V4. V1 básico con textos pequeños/mal centrados. V2 añadía animaciones y cámara pero se deslizaba toda la pantalla; el zoom/pulsación desplazaba círculos y flechas. V3 corrigió escala con pivote centrado y eliminó paneos globales. V4 se rediseñó como documental de **12 capítulos, duración 236.888 segundos (~3:57)** con guion original, imágenes de archivo Wikimedia Commons acreditadas, simulaciones técnicas, Lilith y subtítulos.
- **V4 final renderizado y validado:** `/workspace/GPTFrames/projects/ia-programadores-telcel-v4/IA_Programadores_Telcel_Editorial_V4_1080p_Optimizado.mp4` (~14 MB). Recursos del proyecto: `project.json`, `editorial_script.json`, `assets/audio/01.wav` hasta `12.wav`, `assets/photos/*.jpg`, `image_credits.json`, `FUENTES_Y_CREDITOS.md`, `DESCRIPCION_YOUTUBE.md`, `SUBTITULOS_ES.srt` (estos subtítulos antiguos son ESTIMADOS).
- Enlace externo temporal para V4: `https://771757373c9f1dfc2a.gradio.live/IA_Programadores_Telcel_Editorial_V4_FullHD.mp4`. **Puede expirar**; comprobar túnel antes de afirmar que funciona.
- Código Studio ya incorporado en la rama experimental: `compositor/auto_layout.hpp` (grupos overlay,row,column,grid, padding/gap/align/justify), `geometry_transform.hpp` (centro de escala y cámara), `quality_checks.hpp` (lint visual), `main.cpp` (render FullHD y paralelismo), `native_mp4.hpp` (1920x1080), herramientas MCP en `mcp/render_jobs.hpp`, `mcp/render_http_routes.hpp`, `mcp/render_viewer.hpp` y modificaciones en los contratos MCP. Documentación: `docs/PROFESSIONAL_STUDIO.md`. Cola de renders privados, URL de seguimiento, cancelación, vista previa/reproductor. Aún faltan mejoras de editor web drag-and-drop y validación exhaustiva.
- Repositorios Neo TTS App y Piper revisados. **Neo-TTS-App NO sabe segundos exactos por letra/fonema**: `src/read_along.hpp` estima duración por palabra desde duraciones de fragmentos y valles de energía. No afirmar precisión fonética real. Un export de duraciones fonéticas del modelo Piper es trabajo FUTURO, no completado.

## 4. Trabajo en CURSO — Studio V5 (prioridad absoluta)

Última petición: corregir anticipaciones de gráficos frente a la narración, evitar formas/tarjetas sin texto, **reorganizar layout dinámicamente** cuando aparezcan elementos, y **añadir recorte de imágenes con formas y bordes**. Mantener la calidad narrativa y audios de V4. Producir video nuevo V5 directamente con C++ sin iniciar MCP.

**Implementado y compilado**:
1. `compositor/json_project.hpp`: para `image` acepta `shape: "rect"|"rounded"|"circle"|"ellipse"`, `borderWidth`, `borderColor`, `fit: "cover"|"stretch"`, `radius`; recorte + borde nativos en píxeles. No requiere navegador/FFmpeg para render.
2. `tools/native_word_align.cpp`: alineador nativo C++17 de tiempos APROXIMADOS por palabra a partir del guion original y WAV PCM16 mono, basados en pesos textuales y pausas del audio. Compilado manualmente a `build/native-full/gptframes-word-align`. Resultado `projects/ia-programadores-telcel-v4/word_timings_v5.json`: **12 escenas, 669 palabras**. No se modificó el modelo Lilith ni export fonético Piper; no afirmar precisión exacta. Documento `docs/VOICE_SYNC_V5.md`.
3. `compositor/reactive_layout.hpp`: `cueIndex` ligado a palabra hablada (estimada), aparición atómica de grupos con hijos (tarjeta+texto), actualización de posiciones por frame y transición de layout ~280ms. Integrado en `compositor/json_project.hpp` con `speechTiming`, `scene.reactive=true`. Escenas antiguas siguen sin modo reactivo.
4. `projects/ia-programadores-telcel-v4/build_v5.py`: pequeño script de *autoría offline*, no dependencia del motor; transforma V4 en V5, asigna cues semánticos de 12 escenas, tarjetas/grupos y estilos de imagen. La salida es **`projects/ia-programadores-telcel-v4/project_v5.json`** (101 KB). Este proyecto **VALIDA** como 12 escenas, 236.888s, 1920x1080, 24 FPS, 12 WAV; se generaron previews `v5-preview-8.png`, `v5-preview-36.png`, `v5-preview-57.png`, `v5-preview-76.png`, `v5-preview-85.png`, `v5-preview-92.png`, `v5-preview-108.png`, `v5-preview-127.png`, `v5-preview-144.png`, `v5-preview-163.png`, `v5-preview-185.png`, `v5-preview-209.png`, `v5-preview-232.png`. **Todavía no hay MP4 final V5** ni se han evaluado esas imágenes visualmente.
5. `docs/VOICE_SYNC_V5.md` describe técnica, arquitectura y límites. `tests/reactive_layout_test.cpp` y casos de regresión adicionales agregados. Última ejecución de `ctest --test-dir build/native-full --output-on-failure` fue **11/11 PASS** (19:50 UTC). Conviene añadir test de pixeles/geom de image masks.

**Problemas abiertos concretos**:
- Revisión visual pendiente de previews V5. Buscar grupos mal dimensionados, layout dinámico que se salta/cuadra los elementos, ocultación de decoraciones del grupo, tipografías pequeñas y fotos mal recortadas. Generar previews adicionales en segundos alrededor de cada `cueIndex`, no solo escenas medias.
- `projects/ia-programadores-telcel-v4/v5-quality.json` indica **14 avisos**: 12 `small_box` (principalmente escena 10) + 1 `small_text` (escena 5) + 1 `low_contrast`. Corregir con el autor V5, **pero recordar que el lint estático podría producir falsos positivos al evaluar estados reactivos**. No ocultar avisos sin inspección.
- El script `build_v5.py` ajustó `inset` y alturas para validar, pero los textos largos quizá no tengan espacio; verificar.
- Sincronización por palabra es aproximada. Revisar visualmente que objetos aparezcan cuando Lilith los mencione. No prometer timing por fonema.
- El compositor actual calcula objetos activos por frame; rendimiento Full HD y seguridad/compatibilidad deben revisarse.
- Falta renderizar **V5 MP4**, comprobar audio/video H.264/AAC, que no haya música, duración, decodificación total, luego publicar por el servidor de descarga temporal activo SIN reiniciar MCP.

## 5. Comandos seguros para reanudar en nuevo chat

Mediante `mcp__Slack___exec` indicando `sandbox_id: "ly5QOxFKvGjynGz6r3Cy60jf"`, `cwd:"/workspace/GPTFrames"`:

```bash
cat /workspace/contexto.md
git branch --show-current && git status --short
ctest --test-dir build/native-full --output-on-failure
build/native-full/hyperframes-cpp-native --project validate projects/ia-programadores-telcel-v4/project_v5.json
build/native-full/hyperframes-cpp-native --project lint projects/ia-programadores-telcel-v4/project_v5.json
build/native-full/hyperframes-cpp-native --project frame projects/ia-programadores-telcel-v4/project_v5.json /workspace/GPTFrames/projects/ia-programadores-telcel-v4/v5-inspeccion.png 92
# Al terminar QA y correcciones:
build/native-full/hyperframes-cpp-native --project render projects/ia-programadores-telcel-v4/project_v5.json projects/ia-programadores-telcel-v4/IA_Programadores_Telcel_Studio_V5_1080p.mp4
# Chequear MP4:
ffprobe -v error -show_entries format=duration,size:stream=codec_name,codec_type,width,height,avg_frame_rate -of json projects/ia-programadores-telcel-v4/IA_Programadores_Telcel_Studio_V5_1080p.mp4
```

El render 236.9 s @24FPS tiene unos 5685 fotogramas; ejecutar en segundo plano si tarda y vigilar log/exit antes de publicar, sin interrumpir otros servicios ni procesos. Se puede publicar descarga usando el servicio temporal independiente `/workspace/gptframes-neo-download` (enlace mediante túnel público verificado) sin activar GPTFrames MCP. Verificar la ruta y el túnel mediante `mcp__Slack___tunnel_list` y `curl -I`; **no inventar URL**. Enlazar el nuevo video cuando esté comprobado.

## 6. Preferencias y decisiones del usuario

- Usar el **conector $slack exclusivamente** para estas tareas, no PLS. Crear/modificar en sandbox, mantener otros servicios ajenos intactos.
- No hacer commit/push a `main` sin decisión explícita; rama actual experimental. No resetear archivos pendientes.
- No inventar resultados: distinguir lo implementado/probado de lo proyectado.
- No reemplazar motor C++ por Python o dependencias de render externas; el navegador y voz local son excepciones permitidas. Los scripts Python de autoría offline no son dependencia de producción.
- Audio **Lilith**, sin música, Full HD horizontal, invitación a suscribirse, imágenes y narración de calidad. No imágenes de datos reales de terceros ni afirmaciones falsas sobre Telcel.
- Las fotos de V4 son de Wikimedia Commons, créditos en `image_credits.json` y `FUENTES_Y_CREDITOS.md`; mantener atribuciones.
- La URL del MCP antiguo y el PIN no se incluyen intencionalmente en este contexto por seguridad. No recuperar o exponer secretos en documentación.

## 7. Estado preciso a fecha de creación

- Servidor MCP de GPTFrames: **detenido** (proceso zombie registrado no representa servicio operativo).
- Render final V4: **completado**.
- Código V5: **compilado**, proyecto V5 **validado**, 13 previews generadas, pruebas **11/11 OK**.
- Render final V5: **PENDIENTE**.
- Fuente/creditos/subtitulos V4 disponibles.
- Cambios Git: **SIN PUSH** a GitHub. Esta documentación es para transferencia a otro chat; continuar verificando desde el estado real del sandbox.

**Primera instrucción útil en el nuevo chat:** «Conéctate mediante $slack al sandbox `ly5QOxFKvGjynGz6r3Cy60jf`, lee `/workspace/contexto.md`, revisa visualmente los previews de V5, corrige avisos reales, renderiza y verifica el MP4 V5 y dame el enlace de descarga sin reiniciar el MCP.»
