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

## Elegir tu propio PIN mediante .env

El gateway Go ahora admite **GPTFRAMES_OAUTH_PIN** en el archivo .env de la raíz del proyecto. No requiere bibliotecas nuevas.

    cd /workspace/GPTFrames
    cp .env.example .env
    chmod 600 .env
    nano .env

En el archivo privado coloca, por ejemplo:

    GPTFRAMES_OAUTH_PIN=TuPinPropioLargoYUnico2026

Después inicia o reinicia el gateway de forma controlada:

    ./build/gptframes-tunnel-go --port 18885 --backend-port 18886 --project-dir "$PWD" --data-dir "$PWD/.gptframes-private"

**Prioridad exacta** (de mayor a menor):
1. Variable de entorno del proceso GPTFRAMES_OAUTH_PIN (por ejemplo, desde un gestor de secretos).
2. Clave GPTFRAMES_OAUTH_PIN dentro de PROJECT_DIR/.env, usando --project-dir para definir el directorio.
3. PIN autogenerado persistente en .gptframes-private/oauth-pin.txt, solo si no se configuró el anterior.

El PIN personalizado sustituye **para nuevas autorizaciones** al PIN autogenerado anterior, aunque exista oauth-pin.txt. No se escribe ni se copia a ese archivo, y jamás aparece en los registros ni en la URL. Los PIN definidos manualmente admiten entre 6 y 128 caracteres, sin saltos de línea ni espacios en extremos. Se permiten comillas simples/dobles y comentarios en .env. El .env debe ser un archivo regular, no symlink, con permisos privados (600) en Linux/macOS; en Windows protege el archivo mediante ACL del sistema; si está mal protegido o el valor es inválido, el inicio falla de forma segura.

**Importante:** cambiar el PIN afecta **nuevas solicitudes OAuth** cuando reinicies el gateway. Los tokens OAuth ya emitidos pueden permanecer vigentes; cambiar el PIN **no es lo mismo que revocar una sesión autorizada**. Para revocación, gestionar el estado OAuth/tokens por separado. El gateway en ejecución no recarga .env dinámicamente; planificar el reinicio porque el túnel Gradio puede cambiar de URL y requerir volver a conectar ChatGPT. No subas .env al repositorio (.gitignore lo excluye). La plantilla .env.example no contiene credenciales reales.

## Uso
El ejecutable lanza el servidor MCP privado en Python y luego obtiene un dominio HTTPS temporal de Gradio. Solo abre el servicio OAuth cuando una prueba real por la URL pública confirma que el túnel apunta al proceso correcto.

    ./build/gptframes-tunnel-go --port 18885 --backend-port 18886 --project-dir "$PWD" --data-dir "$PWD/.gptframes-private"

Muestra la dirección pública del tipo https://<random>.gradio.live/mcp. **Si no has definido GPTFRAMES_OAUTH_PIN** por variable de entorno ni por .env, el PIN se genera y conserva en .gptframes-private/oauth-pin.txt (modo 0600, dentro del directorio 0700). Nunca se coloca el PIN en la URL ni en el código fuente. El cliente que desee acceder a las herramientas MCP debe pasar la autorización OAuth mediante navegador con ese PIN. El contenido del archivo es una credencial: se debe leer de forma privada y **no pegarse en mensajes, logs o repositorios**.

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

## Servidor de video independiente
Se añadió cmd/gptframes-media, un servidor HTTP Go con net/http.FileServer que admite Range 206 Partial Content en loopback para streaming y descarga eficiente de videos. Nunca comparte el puerto MCP ni expone el PIN. Se probó Range local con solicitudes bytes=0-99 y bytes=100-199 que obtuvieron HTTP 206. El túnel Gradio público devuelve HTTP 200 a Range, probablemente por el intermediario; la descarga MP4 y reproducción progresiva funcionan, pero saltar a capítulos puede requerir buffering. Para búsqueda de segmentos/seek estricto en producción usar un hosting que preserve cabeceras Range.
