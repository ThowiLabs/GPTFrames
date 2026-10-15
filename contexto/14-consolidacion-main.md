# Consolidación en main
Fecha: 2026-10-15 UTC

El usuario confirmó que ChatGPT conecta con el PIN y pidió concentrar el repositorio remoto y local en main, eliminando las demás ramas.

Se conserva como árbol activo la implementación C++ del motor/MCP/utilidades y el flujo PIN nativo basado en kaggle-mcp. El historial de las ramas anteriores se incorpora como ascendencia de main antes de eliminar sus referencias; sus prototipos Go/Python y scripts de demostración quedan en ese historial, sin reincorporarse al código activo. No se reescribe la historia publicada.

Los proyectos privados, modelos, medios generados y el ejemplo local no versionado permanecen en el entorno. No se suben a GitHub. El worktree operativo del MCP puede permanecer separado, con HEAD desligado y el mismo commit de main, para conservar la conexión en uso.

## Voz Lilith

A petición del usuario se instaló el archivo local existente lilith.neo en el directorio de modelos configurado del servicio. La firma del contenedor NEO fue validada (identidad de autor no verificada externamente). La herramienta synthesize_voice del MCP conectado generó una narración real de 3.65079 segundos y la adjuntó como WAV PCM mono de 24 kHz al proyecto de prueba prueba-voz-lilith.

Para sintetizar, el parámetro model es lilith.neo. El modelo permanece privado, fuera del repositorio. No fue necesario reiniciar el MCP para reconocerlo.
