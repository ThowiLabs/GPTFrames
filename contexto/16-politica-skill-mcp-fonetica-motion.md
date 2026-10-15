# GPTFrames 2026-10-15 · Política de producción en skill y MCP

## Petición
Que tanto la skill como las herramientas MCP indiquen que el texto para el TTS debe escribirse **como se pronuncia** y que las escenas deben tener **animaciones reales, no imágenes estáticas**, más controles vistos en documental 30 FPS.

## Implementación
- Nueva skill `.claude/skills/gptframes-video/SKILL.md`, específica para GPTFrames/Neo Lite/MCP.
- Actualizada `.claude/skills/hyperframes-cpp/SKILL.md` y `.claude/skills/hyperframes-cpp/references/WORKFLOWS.md` con las mismas reglas.
- Nuevo módulo `mcp/production.py` centraliza `GUIDELINES`, `suggest_pronunciation` y `analyze_motion`.
- `mcp/server.py` anuncia `get_production_guidelines`, `prepare_narration`, `analyze_motion`; actualiza descripciones de crear/validar/renderizar/sintetizar.
- `prepare_narration` separa visible Go/Rust/C++/HirCoir de hablado gou/rast/ce más más/Jírcoir; TTS `synthesize_voice` añade campo opcional `display_text` para guardar por separado el texto correcto de pantalla.
- `analyze_motion` inspecciona capas JSON realmente variables por propiedad a través de keyframes C++, ignora falsos movimientos por salto a t=0; `validate_project` lo devuelve y `render_video` bloquea escenas inmóviles salvo `allow_static=true` solicitado expresamente.
- Docs `docs/MCP.md` y `README.md`, pruebas MCP ampliadas.

## Pruebas
Python 17 pruebas `tests/test_gptframes.py` OK. `tests/test_video_pronunciation_30fps.py` OK. MCP real `--transport stdio`, methods tools/list y tools/call para prepare_narration/analyze_motion OK. JSON de demostración tiene 7/10/8/7 capas animadas por escena. Python py_compile y git diff --check OK.

## Alcance real
El detector de movimiento se aplica al JSON del compositor y no inspecciona código GSAP/HTML ni demuestra movimiento visible entre píxeles: la skill pide inspección adicional de fotogramas para HTML. Los cambios de código del servidor MCP requieren **reiniciar el proceso MCP vigente** antes de aparecer en un túnel ya publicado; no modificar túneles OAuth ni secretos o puerto en caliente.
