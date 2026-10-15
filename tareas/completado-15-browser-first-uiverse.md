# Experimento Browser-first y UIverse
Fecha: 2026-10-15
Objetivo: probar escenas HTML/CSS animadas por Chromium CDP con fotogramas deterministas y codificacion nativa H.264/AAC.
Estado: en proceso.
Restricciones: reutilizar CDP ya existente; no Node/Python, ni ffmpeg como dependencia de produccion, ni servicios externos.
Seguridad: ejecutar solo fixtures HTML confiables con CSP sin JS ni red; no dar soporte a snippets arbitrarios en MCP hasta aislar Chromium sin privilegios.
Pruebas: fotogramas 0, 1, 2 y reproducibilidad por seek; preview de MP4 y ctest.
Pendientes: sesiones de browser reutilizables, alpha, sincronizar cues de voz, decodificar clips, endurecer sandbox para importaciones.

## Resultado comprobado
Se desarrollaron --browser-widget-frame y --browser-widget-render con
control de animaciones CSS por tiempo. Tres fotogramas visuales difieren;
el MP4 de 2s H.264/AAC 24fps se decodificó completamente. El test
browser-css-widget pasó (34 s).
Prototipo CLI local exclusivamente, no publicado por MCP ni listo para
HTML aportado por usuarios. Siguientes tareas 16 y 17.
