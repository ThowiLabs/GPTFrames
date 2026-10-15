# Código Go reutilizado desde kagssh-go

Fuente: https://github.com/ThowiLabs/kagssh-go
Revisión inspeccionada: bba20575d85473b6032f0c525ffaf877a2fc7c56
Titular: misma cuenta del proyecto GPTFrames (ThowiLabs).

Paquetes reutilizados: `internal/gradiolive`, `internal/frpmsg`, `internal/oauth`, `internal/pinauth`, `internal/securefs`.
El módulo Go renombra únicamente las rutas de importación de `internal/`.
El cliente Gradio implementa túnel FRP 0.44/TLS/yamux de forma nativa sin Python Gradio ni frpc.
La seguridad OAuth reutiliza el diseño probado en kagssh-go con autorización por PIN, PKCE, registro de clientes y validación de bearer.
NOTA: kagssh-go no contiene archivo LICENSE en el commit inspeccionado; esta copia se hace bajo autorización del propietario de ambos repositorios. Añadir licencia explícita antes de redistribución externa.
