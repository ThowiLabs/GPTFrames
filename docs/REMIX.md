# HyperFrames C++ Remix — integración de los dos motores

## Características conservadas
HyperFrames C++ conserva importación HTML de escenas, temas classic/news, render html con litehtml, captura browser con Chromium, AVI RGB24/RLE8/MJPEG/PCM y MP4 H.264/AAC. El motor sigue compilando en Linux y Windows x64 desde Linux.

## Nuevo compositor integrado
Un único binario contiene además 15 escenas y efectos: tipografía cinética, spring, resaltado, parallax, gráficos, máscaras, partículas, trazados, 3D simulado, visualizador musical y 7 transiciones. El código está en remix/remix_renderer.hpp, remix/showcase.hpp y remix/audio_score.hpp. Comparte framebuffer y codificador con HyperFrames, sin invocar ejecutables adicionales.

## Modo Fusion
El argumento --fusion recibe un HTML de HyperFrames y sus WAV originales, renderiza esas escenas en classic, news, html o browser y añade al final 81 segundos de 9 capítulos de efectos procedurales con música sintetizada. La duración total es duración_del_HTML + 81 s. Los efectos posteriores aún son una demostración predefinida, no un lenguaje configurable general.

## Comandos de ejemplo
Linux: ./build/remix-linux/hyperframes-cpp-native --fusion proyecto-remix/index.html proyecto-remix/audio salida.mp4 24 classic 720x1280
Remix: ./build/remix-linux/hyperframes-cpp-native --remix salida.mp4 24 720x1280 148 proyecto-remix/audio
Frame: ./build/remix-linux/hyperframes-cpp-native --remix-frame escena.png 72
Capítulos: ./build/remix-linux/hyperframes-cpp-native --remix-list
Elegir HTML/JS fiel: sustituir classic por browser en --fusion (requiere Chromium instalado).

## Construcción y pruebas
./scripts/build-native-mp4.sh para Linux, ./scripts/build-native-windows.sh para Windows con MinGW-w64, tests/remix-smoke.sh para las funciones nuevas. La rama principal no se modificó.

## Límites y obligaciones
No ofrece paridad de API con Remotion; no incorpora React, Lottie, Rive, Three.js ni Studio. Windows cross-build requiere prueba de ejecución real. Revisa licencias DejaVu, minih264, minimp4, vo-aacenc y obligaciones de patentes de H.264/AAC. Los WAV de demostración deben respetar derechos originales. URL Gradio temporal.