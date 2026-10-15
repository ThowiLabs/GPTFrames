# GPTFrames — Colección 04 y descubrimiento automático

## Linux Motion (29 presets)

La colección de 30 módulos recibida en `source_linux_04.html` se adaptó como
**29 recursos independientes**. Se descartó intencionalmente **26 — Debian Red Swirl**
porque el usuario indicó que está mal diseñado; **no existe** un preset Debian
renderizable en la biblioteca.

- **Kernel y arranque:** Kernel Panic, systemd Boot, advertencia root,
  Fastfetch, Vim y SSH Randomart.
- **Procesos y memoria:** htop, RAM/Swap, OOM Killer, árbol de procesos,
  load average, pipeline de Unix.
- **Paquetes y red:** Pacman, RAID, ICMP ping, cron, firewall DROP,
  enlace simbólico.
- **Seguridad y archivos:** chmod, sudo, SELinux, chroot, firma GPG,
  árbol de directorios.
- **Distros y hardware:** Tux, Arch, Ubuntu, GRUB y Raspberry GPIO.

`build_linux_04.py` extrae una sola tarjeta y su CSS por preset,
elimina los controles del HTML original y produce HTML offline sin scripts
ni dependencias CDN en `animaciones/presets/<id>.html`. Conserva metadata,
categoría, tags y movimientos CSS; `indexar_tags.py` actualiza el buscador.
La biblioteca pasó de 32 a 61 presets.

**Importante:** Las órdenes peligrosas que aparecen en algunos elementos
(p. ej., el rechazo visual a una orden root) son solo texto de una
ilustración. Los presets nunca ejecutan esas órdenes.

### Correcciones funcionales

Las transiciones originales de Vim y chmod intentaban animar `content` de
elementos HTML reales, lo cual no altera su texto. Se reemplazaron con
estados de texto mutuamente visibles controlados por keyframes, de modo
que se ve INSERT → :wq → written, y permisos 777 → 700.

### Catálogo de vídeo

`build_gallery_linux_04.py` genera `catalogo-linux-04.html` que
muestra 29 escenas con 2 segundos por recurso y reloj CSS por escena.

```bash
python3 animaciones/build_linux_04.py
python3 animaciones/indexar_tags.py
python3 animaciones/build_gallery_linux_04.py
build/native-full/hyperframes-cpp-native --browser-widget-render \
  animaciones/catalogo-linux-04.html \
  animaciones/Linux_Motion_29_Presets_480p24.mp4 58 24 848 480
```

## Recursos en caliente — sin reiniciar por cada asset nuevo

El binario MCP incorpora `ResourceMonitor`, un servicio de escaneo local
en segundo plano que revisa aproximadamente cada **2 segundos**:
- `animaciones/presets/*.html`;
- `animaciones/collections/*.json` (descubrimiento de manifests);
- `GPTFRAMES_MODELS/*.neo`, `GPTFRAMES_MODELS/*.onnx`;
- `assets/`: imágenes, vídeos, sonido y Lottie.

Consultas disponibles:

- `resource_status`: último inventario detectado automáticamente, con
  fecha en milisegundos Unix y conteos por tipo.
- `rescan_resources`: actualización **inmediata** bajo demanda, sin reinicio.
- `list_voice_models`: consulta el sistema de archivos en cada llamada;
  los nuevos modelos locales se pueden usar directamente.
- `list_animations`, `search_animations`, `describe_animation` y
  `list_animation_facets`: cargan el catálogo actualizado en cada solicitud;
  no guardan una copia obsoleta en memoria.

Los archivos HTML que alguien deposita en `animaciones/presets/` sin
registrarlos en `catalogo.json` aparecen con
`status=detected-unverified` y `renderable=false`, por seguridad.
Para permitir usarlos en renders es necesaria una entrada validada en
el catálogo con `id`, `preset_file` y metadata. Después de registrar
un asset, **no se necesita reiniciar el MCP**.

**Distinción de despliegue:** los cambios al **código compilado** del MCP
requieren desplegar la versión compilada en la siguiente ejecución o
actualizar el proceso por el método habitual. Una vez ejecutando esa
versión, incorporar, modificar o eliminar recursos locales no requiere
reiniciar el MCP. No se reinició la instancia anterior durante este cambio.
La revisión automática no concede permisos para ejecutar HTML arbitrario.
