# GPTFrames — Roadmap de 80 animaciones educativas para Gemini

Este roadmap organiza **8 lotes de 10 recursos** solicitados al generador
externo de HTML/CSS. Cada lote debe llegar en un HTML autocontenido,
con módulos numerados, independientes, reproducibles por tiempo y
sin CDN ni scripts asíncronos.

| Lote | Categoría | Contenido |
|---|---|---|
| 01 | Arquitectura de software | Monolito, modular, monolito modular, microservicios, cliente-servidor, MVC, capas, hexagonal, eventos, refactorización |
| 02 | Programación y ejecución | Editor con typing, compilación, intérprete, debugger, call stack, recursión, stack/heap, referencias, bucles, excepciones |
| 03 | Lenguajes de programación | Python, JavaScript, TypeScript, Go, Rust, C, C++, Java, C#, SQL |
| 04 | Bases de datos | JOIN, B-Tree, ACID, normalización, relaciones, replicación, sharding, caché Redis, query optimizer, migración |
| 05 | Git y DevOps | Commit, branches, merge/rebase, PR, CI/CD, Docker, Compose, Kubernetes, blue-green, rollback |
| 06 | Redes y seguridad | OSI, TCP handshake, DNS, HTTP/HTTPS, firewall, NAT, VPN, proxy, SSH, load balancer |
| 07 | Algoritmos | Arrays/listas, stack/queue, búsqueda binaria, sorting, BST, hash table, BFS/DFS, DP, Big O, GC |
| 08 | Frontend web | DOM, box model, Flex/Grid, responsive, rendering pipeline, React tree, SSR/hydration, REST fetch, WebSocket, autenticación |

## Contrato de aceptación de cada HTML

- Diez componentes visuales con IDs únicos y nombre/descripción.
- Campos de texto, código, ejemplos, colores y etiquetas editables.
- No CDN, webfonts, librerías externas ni JavaScript con temporizadores
  para la reproducción final. El motor de GPTFrames controla cada tiempo.
- No reutilizar HTML sujeto a atribución de terceras fuentes sin permiso.
- El texto no puede desbordarse ni introducirse antes del relato.
- Conservar código original en `animaciones/source_*.html`, adaptar cada
  recurso a `animaciones/presets/` y registrar tags, tipo, categoría,
  movimientos CSS y los campos editables.
- Generar vídeo numerado a **848×480/24fps**, verificar movimiento y
  entregar al usuario para aceptar/modificar/rechazar cada diseño.
- Hacer commit de código **solo** en rama experimental; dejar fuera
  renders, capturas de QA y archivos WAV intermedios.
- No reiniciar el MCP para incorporar assets; catálogos y modelos se
  descubren desde filesystem. Cambios a un ejecutable requieren build
  y despliegue separado.

## Ejemplo de vídeo

`examples/monolitico-vs-modular/` contiene una demostración terminada:
monolito vs organización modular, siete escenas de ocho segundos,
narración española Neo Lite/Lilith y subtítulos. Recordatorio:
monolítico (despliegue) y modular (estructura interna) no se excluyen.
