# GPTFrames OAuth · PIN configurable en .env
Fecha: 2026-10-15. Rama: feature/env-oauth-pin, creada desde feature/voice-fonetica-30fps-no-brand; main sin modificar.

## Solicitud
Permitir que el usuario defina personalmente el PIN con GPTFRAMES_OAUTH_PIN en el archivo .env de la raíz de GPTFrames. El gateway OAuth de Go debe usar ese valor en lugar del PIN autogenerado.

## Implementado
- .env.example contiene GPTFRAMES_OAUTH_PIN= vacío a propósito (obliga a elegir un PIN).
- tunnel/cmd/gptframes-tunnel/pin_config.go carga la variable del entorno del proceso (primera prioridad); si no está, carga clave del archivo root/.env (segunda); si tampoco está, mantiene fallback al antiguo .gptframes-private/oauth-pin.txt (tercera).
- Validación con pinauth.Validate: PIN entre 6 y 128 caracteres; no vacíos, espacios externos ni saltos de línea; permite comillas y comentarios; rechaza claves duplicadas, symlink, archivos mayores de 64KB; en Linux requiere chmod 600.
- Valor del PIN personalizado nunca aparece en stdout/logs, nunca se copia a oauth-pin.txt ni se sube a Git. .env ya estaba en gitignore.
- main.go usa configuredPIN antes de crear OAuth y muestra solo el origen del PIN, sin contenido.
- README.md y docs/GO_GRADIO_MCP.md actualizados.
- Compatible Linux y Windows (en Windows se emplean ACL en vez de chmod).
- Binarios Linux y Windows actualizados en build/ para el próximo reinicio; túnel público previo queda ejecutándose sin interrupción.

## Pruebas
Go test ./..., go vet ./... y crossbuild GOOS=windows GOARCH=amd64 pasaron, incluyendo precedencia .env sobre PIN generado, shell sobre .env, PIN inválido, duplicados, permisos inseguros, symlinks, fallback sin .env y variables vacías. Suite Python 17/17 OK. No se imprimieron credenciales.
El proceso existente NO fue reiniciado para no cambiar el dominio Gradio ni interrumpir ChatGPT. Para aplicar un PIN personalizado el usuario debe crear/editar .env y reiniciar manualmente el gateway de forma controlada.
Los OAuth tokens ya emitidos podrían seguir siendo válidos tras cambiar PIN; el cambio solo afecta nuevas autorizaciones hasta revocación adicional.
