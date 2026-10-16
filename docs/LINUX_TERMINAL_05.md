# Terminales Linux — Colección 05

Este paquete adapta los cuatro componentes del archivo
`animaciones/source_terminal_05.html` aportado por el usuario:

1. **Bash / nginx:** escritura de `systemctl status nginx.service` y
   aparición escalonada de su salida, con desplazamiento de la terminal.
2. **Kernel / systemd:** las 23 líneas del ejemplo de arranque aparecen
   siguiendo un reloj reproducible y el registro se desplaza.
3. **Fastfetch:** entrada del comando, logotipo textual de Arch y 13
   líneas de hardware/SO reveladas secuencialmente.
4. **Vim:** modo INSERT, edición de `daemon.py`, `:wq`,
   mensaje de guardado y regreso a la consola.

La plantilla original usa `sleep`, `setTimeout` y
`Math.random`. Esa lógica se reemplazó por **HTML y CSS puros**:
`animation-delay`, `steps()`, fotogramas discretos y desplazamientos.
El reproductor Chromium de GPTFrames sitúa cada animación exactamente
en el segundo solicitado, por lo que el vídeo funciona de forma
reproducible sin esperar tiempos reales y sin ejecutar scripts JS del
usuario.

Los cuatro presets están en `animaciones/presets/` y registrados
como números **62–65** en `animaciones/catalogo.json` bajo
`Terminales Linux`; se indexan tags, movimientos y propiedades
editables mediante `indexar_tags.py`.

## Edición de campos

Ejemplo de capa nativa:

```json
{
  "type":"html_widget",
  "collection":"animaciones",
  "preset":"terminal-bash-nginx",
  "x":110,"y":90,"width":950,"height":550,
  "properties":{
    "user":"admin",
    "host":"servidor-web",
    "command":"systemctl status nginx.service"
  }
}
```

Los presets declaran campos editables en JSON y el renderer cambia
únicamente los nodos `data-field` por `textContent` a través de
Chromium/CDP. Los campos no ejecutan las órdenes que muestran.

**Nota:** en el preset Bash, la duración del efecto de escritura fue
calibrada para la longitud del comando del ejemplo; una cadena de
otra longitud conserva la animación de revelado pero no necesariamente
la misma velocidad por carácter. Las salidas de Bash, boot y fastfetch
representan las líneas provistas en la plantilla: son datos de
demostración, no la telemetría real del equipo.

## Reproducción y pruebas

```bash
python3 animaciones/build_terminal_05.py
python3 animaciones/indexar_tags.py
python3 animaciones/build_gallery_terminal_05.py

build/native-full/hyperframes-cpp-native --browser-widget-render \
  animaciones/catalogo-terminal-05.html \
  animaciones/Terminales_Linux_4_Presets_480p24.mp4 \
  32 24 848 480
```

El catálogo muestra 4 módulos a razón de 8 segundos cada uno
a 848×480/24 FPS, con nombres y descripciones. El vídeo se genera
desde el binario nativo y Chromium; los scripts de Python son solamente
herramientas para crear y mantener los presets dentro del repositorio.

Los modelos de voz, media y nuevas animaciones siguen detectándose
en caliente mediante el `ResourceMonitor` implementado en la colección
anterior. No es necesario reiniciar el MCP cada vez que se añade un
asset; los cambios al ejecutable C++ requieren desplegar una versión
nueva del binario en el siguiente ciclo de actualización.
