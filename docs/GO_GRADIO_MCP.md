# GPTFrames MCP: túnel Go nativo de KagSSH-Go con OAuth

## Procedencia
GPTFrames/tunnel utiliza componentes del repositorio del mismo propietario https://github.com/ThowiLabs/kagssh-go, commit inspeccionado bba20575d85473b6032f0c525ffaf877a2fc7c56. Paquetes traídos: internal/gradiolive, frpmsg, oauth, pinauth y securefs. Se adaptaron rutas de importación a GPTFrames. Ver tunnel/UPSTREAM.md.

El cliente del túnel Gradio está implementado en Go puro con TLS, yamux y protocolo FRP, sin ejecutar frpc ni usar Gradio Python. El código OAuth incluye registro dinámico de clientes, PKCE S256, estados persistentes, PIN y bearer tokens con TTL.

## Construcción
Desde GPTFrames:

    cd tunnel
    go test ./...
    go build -o ../build/gptframes-tunnel-go ./cmd/gptframes-tunnel
    cd ..

Go necesita dependencias públicas hashicorp/yamux y fatedier/golib. El motor de render C++ y la aplicación MCP Python conservan sus dependencias originales.

## Uso
El ejecutable lanza el servidor MCP privado en Python y luego obtiene un dominio HTTPS temporal de Gradio. Solo abre el servicio OAuth cuando una prueba real por la URL pública confirma que el túnel apunta al proceso correcto.

    ./build/gptframes-tunnel-go --port 18885 --backend-port 18886 --project-dir "$PWD" --data-dir "$PWD/.gptframes-private"

Muestra la dirección pública del tipo https://<random>.gradio.live/mcp. El PIN es generado una sola vez y almacenado en .gptframes-private/oauth-pin.txt, modo 0600, dentro del directorio 0700. Nunca se coloca el PIN en la URL ni en el código fuente. El cliente que desee acceder a las herramientas MCP debe pasar la autorización OAuth mediante navegador con ese PIN. El contenido del archivo es una credencial: se debe leer de forma privada y **no pegarse en mensajes, logs o repositorios**.

La URL es temporal; si Gradio cambia el subdominio, el recurso OAuth también cambia y podría requerir reconectar el cliente. Este túnel no debe considerarse hosting permanente.

## Pruebas
- Go: go test ./... pasó para gradiolive, OAuth, pinauth y FRP.
- Modo local --local: GET /health devuelve 200, POST /mcp sin bearer devuelve 401, metadata OAuth anuncia S256 y registro dinámico, estado OAuth guardado con permisos 0600.
- Modo público Gradio: comprobado GET /health 200, POST /mcp sin bearer 401, metadata OAuth publicada con issuer HTTPS, PKCE S256 y registration_endpoint.
- El gateway evita conectarse a un puerto backend ocupado previamente, y usa un token secreto efímero para comunicar Go con el MCP privado.
- No se ha ejecutado todavía un recorrido completo de autorización desde la interfaz de ChatGPT; la conexión requiere que el usuario dé el consentimiento y registro en Settings -> Plugins o en la interfaz aplicable.

## Seguridad y límites
El transporte Gradio **no agrega autenticación por sí solo**: la capa OAuth Go es obligatoria y está delante de /mcp. El proceso interno Python escucha 127.0.0.1 con bearer secreto diferente del OAuth de usuarios. Los servicios solo se exponen después de verificar el subdominio con un nonce aleatorio.

No publicar el archivo PIN ni el estado OAuth. No habilitar una ruta pública para esos archivos. Los resultados de render exponen rutas locales, no enlaces firmados: el sitio estático del documental se sirve mediante otro túnel separado de las herramientas MCP.

El protocolo MCP remoto puede cambiar y la compatibilidad de la experiencia de conectar en ChatGPT debe comprobarse con el flujo real de autorización de la plataforma. El software no se registra por sí solo como un plugin ni conecta la cuenta sin consentimiento.
