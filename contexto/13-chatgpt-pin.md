# Conexión ChatGPT con ventana PIN en C++
Fecha: 2026-10-15 UTC
Rama: refactor/cpp-sin-python

La conexión pública es HTTPS /mcp, con registro automático del cliente y una ventana donde el propietario introduce su PIN. El servicio HTTP ya no acepta GPTFRAMES_MCP_TOKEN ni credenciales estáticas. El PIN, los clientes autorizados y la URL temporal se configuran fuera de Git.

## Referencia solicitada

Se revisó ThowiLabs/kaggle-mcp en 8ed2d2b. Se adaptó su comportamiento a C++17, sin introducir el backend Go ni Python: autorización PIN, DCR, PKCE S256, callbacks ChatGPT exactos y descubrimiento de herramientas después de autorizar.

En especial, se incorporó server/discover para el protocolo 2026-07-28, el formato completo de respuesta y los encabezados Mcp-Method/Mcp-Name junto a metadatos de versión/capacidades. Se conserva initialize para clientes 2025-03-26, 2025-06-18 y 2025-11-25. Versiones no admitidas devuelven -32022 con versiones disponibles. Los Origins aceptan ChatGPT y el origen público configurado.

## Autorización nativa

- OpenSSL en Linux y BCrypt en Windows para aleatoriedad y SHA-256.
- PIN privado, comparación constante y límite de intentos; formulario en español sin revelar el PIN.
- Corregido el rechazo del formulario con Origin null: como kaggle-mcp, se exige token CSRF ligado a la sesión y a una cookie Secure/HttpOnly/SameSite=Lax. Sin cookie, con token falso o con otra sesión se rechaza; el Origin de la API MCP sigue validándose.
- Retorno HTML con meta refresh y enlace de respaldo al callback ChatGPT, como kaggle-mcp, para evitar el bloqueo CSP de redirecciones tras POST. El formulario usa decision, sin colisionar con su atributo action.
- Transacciones y códigos de un uso, PKCE obligatorio, recurso y redirección verificados.
- Identificación de emisor en respuestas de autorización.
- Acceso de una hora; refresh rotatorio hasta 24 horas con revocación ante reutilización.
- Registros pendientes acotados y temporales; solo se persisten clientes aprobados.
- Inicio HTTP en loopback; ningún acceso a herramientas sin autorización. La raíz y health son solo estado mínimo.
- Los tokens viven en memoria. Reiniciar el servidor o cambiar la URL temporal puede requerir reconectar ChatGPT.

## Comprobaciones

Las pruebas C++ cubren autorización completa, PIN incorrecto, callback/recurso/origen, vectores RFC7636, expiración, replay, refresh, rechazos sin autenticación, descubrimiento moderno y herramientas. Se compila también el ejecutable MCP para Windows mediante MinGW; la ejecución real en Windows queda pendiente.

El usuario confirmó que la conexión con PIN y la prueba desde ChatGPT funcionaron. Después se comprobó también synthesize_voice usando el conector VL conectado a este MCP.
