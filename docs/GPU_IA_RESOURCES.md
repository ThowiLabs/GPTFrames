# Recursos GPU e IA — colección 09

Se integran tres HTML/SVG proporcionados por el usuario como recursos locales de la colección `animaciones`. No se requieren dependencias nuevas ni recompilar el motor. La búsqueda vuelve a leer el catálogo en cada consulta.

| Número | ID | Archivo de origen | Campos editables |
| --- | --- | --- | --- |
| 96 | cpu-vs-gpu-parallel | cpu-vs-gpu-parallel.html | 14 |
| 97 | gpu-pixels | gpu-pixels.html | 7 |
| 98 | gpu-matrix-multiply | ai_studio_code (2).html | 21 |

Duración: 12 segundos por recurso, sin bucle. HTML/CSS/SVG offline, sin JavaScript ni descargas, fondo transparente y animaciones compatibles con `document.getAnimations()` y el reloj del motor. Los textos `data-field` usan guion bajo para ajustarse al contrato de propiedades del capturador. El catálogo declara cada texto, tipo y valor predeterminado. Las propiedades no exponen edición de números matemáticos ni geometría.

## Adaptaciones de integración

- CPU/GPU: rótulos más breves para evitar desbordamientos, estado de procesamiento separado del estado final y barras completas al terminar. Los bloques representan un ejemplo conceptual, no cantidades reales de núcleos ni mediciones de rendimiento.
- Matrices: se eliminaron desplazamientos CSS que reemplazaban las traslaciones SVG y recortaban el título. La aparición de números escala respecto de su propio centro. Los textos explican que el reparto real depende del hardware y que algunas GPU incorporan unidades especializadas.
- Los tres recursos se ajustan al viewport conservando las proporciones del SVG 848x480. Se mantienen los colores, los gráficos y la secuencia originales.
- El ejemplo de matrices conserva A=[[1,2],[3,4]], B=[[5,6],[7,8]] y C=[[19,22],[43,50]]. Los productos mostrados son correctos; es una visualización didáctica, no un programa de ejecución GPU.

## Uso

Desde la raíz del repositorio:

```bash
build/native-full/hyperframes-cpp-native --project validate animaciones/ejemplo-gpu-ia.json
build/native-full/hyperframes-cpp-native --project lint animaciones/ejemplo-gpu-ia.json
build/native-full/hyperframes-cpp-native --project frame animaciones/ejemplo-gpu-ia.json /tmp/gpu-ia.png 30
build/native-full/hyperframes-cpp-native --project render animaciones/ejemplo-gpu-ia.json /tmp/gpu-ia.mp4 --size 848x480
```

El ejemplo contiene tres escenas de 12 segundos sobre fondo oscuro, con un lienzo editable de 1280x720 y sin narración. Su función es demostrar el uso de los recursos; no es el video final del canal.

```json
{"type":"html_widget","collection":"animaciones","preset":"gpu-matrix-multiply","x":0,"y":0,"width":1280,"height":720,"start":0,"duration":12,"properties":{"title":"Multiplicación de matrices"}}
```

Cada campo `properties` debe existir en `editable` del recurso. El motor admite hasta 16 cambios de texto por capa; el recurso de matrices expone 21 campos disponibles, de los que se puede seleccionar un máximo de 16 por uso. El render requiere Chromium compatible con CDP, el ejecutable con captura de navegador y la raíz del repositorio como directorio actual o `GPTFRAMES_ROOT`.

## Verificación de integración

Proyecto JSON validado y sin advertencias del linter. Se revisaron nueve fotogramas (tres por recurso), se comprobó que las fases producen imágenes distintas y se probó la aplicación de textos editables. Las búsquedas C++ resuelven los tres IDs. Los 95 registros anteriores del catálogo no se modificaron.
