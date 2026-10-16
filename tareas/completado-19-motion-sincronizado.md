# Edición narrada con motion sincronizado

Solicitud: adoptar como estándar la edición aprobada del video GPU/IA revisado, con elementos que aparecen y se transforman con la narración.

## Cambios

- Sustituir el prompt anunciado por MCP en initialize y server/discover por un flujo obligatorio de audio real, mapa de beats, eventos por elemento, adaptación de presets y gate audiovisual.
- Reforzar seis herramientas de autoría/preview/render y el tip inicial de proyecto.
- Actualizar la skill canónica y sus copias instaladas en los runtimes Claude/Agents.
- Añadir references/NARRATED_MOTION.md: gramática de edición, manifiesto de cues, offsets, alineación aproximada, patrones reutilizables, seek determinista, legibilidad, rendimiento y seguridad de exportaciones canceladas.
- Conectar los requisitos con WORKFLOWS.md, QUALITY_GATE.md y los prompts documentados.
- Mantener explícita la diferencia entre obligaciones editoriales del agente y validación técnica del motor. No prometer un evaluador estético automático.

## Activación y comprobaciones

Skill validada con quick_validate.py y enlaces/copias instaladas comprobados. Servidor recompilado y recargado con entorno mínimo explícito y los mismos archivos privados de configuración, puerto y origen público. Health correcto y metadatos públicos idénticos; no se leyó el entorno del proceso ni se expusieron credenciales. El cliente puede necesitar reconectar al renovarse las credenciales OAuth temporales.

Pruebas MCP comprueban que ambos protocolos publican el estándar y que los recordatorios están presentes en las herramientas. Se corrigen expectativas antiguas: 95 animaciones frente a 98 ya integradas, y 15 herramientas frente a las 23 actuales. El transporte HTTP comprueba el esquema OAuth de todas las herramientas y la igualdad de instrucciones en ambos protocolos.

La política es aplicable a nuevos videos narrados y revisiones, salvo petición explícita del usuario de tratamiento estático. Los medios de proyectos y pesos de voz permanecen fuera de los commits de código.
