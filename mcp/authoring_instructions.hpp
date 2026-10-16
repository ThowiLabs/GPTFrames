#pragma once
#include <string>

namespace gptframes::mcp {

// Instructions advertised as server instructions at MCP initialization and
// modern server discovery. They guide the model, not a security policy or a
// substitute for trusted-catalog verification inside the engine.
inline const std::string& authoring_instructions() {
    static const std::string text = R"GUIDE(
GPTFrames MCP - PRODUCCION VISUAL BROWSER-FIRST (HTML/CSS/SVG).
OBJETIVO: crear videos educativos profesionales, sincronizados con voz, usando primero las animaciones HTML/CSS/SVG ya instaladas. NO transformar explicaciones de programacion, Linux, Docker, arquitectura, redes o lenguajes en diapositivas basicas si existen presets adecuados.

SECUENCIA OBLIGATORIA PARA NUEVOS VIDEOS:
1) Consultar resource_status y list_animation_facets. Consultar list_voice_models si el video necesita voz. Usar rescan_resources cuando el catalogo cambie.
2) Escribir guion y tabla de escenas. PARA CADA escena: buscar presets mediante search_animations, con sinonimos pertinentes; comprobar al menos un candidato con describe_animation; consultar list_asset_presets y describe_asset_preset cuando corresponda. No inventar IDs ni propiedades editables.
3) PRIORIZAR los presets HTML/CSS/SVG revisados y renderizables. Insertarlos REALMENTE como capas JSON type=html_widget, collection=animaciones (o originales si pertenece a esa coleccion), preset=<ID verificado>, properties=<solo campos declarados en editable>. Darles suficiente area visual para que las animaciones, codigos y diagramas sean legibles.
4) Usar escenas C++ nativas para la composicion, voz, titulos complementarios, fondos y transiciones. Si existe un preset pertinente, no sustituirlo por rectangulos genericos, emojis o una foto diminuta. Si no existe, explicar la carencia y elegir una alternativa compatible; nunca usar un preset no verificado.
5) Sincronizar las entradas visuales con la narracion real: animar y revelar el concepto cuando se diga. Inspeccionar duraciones reales de WAV y no truncar frases. Un texto visible editable no implica que se recalcule la simulacion interna de un preset; verificar sus limites.
6) Para imagenes importantes usar fit=contain. Usar cover solo para fondos con recorte intencional; evitar stretch. No tapar fotografias con tarjetas opacas. No reducir texto hasta ser ilegible ni colocar formas vacias.
7) Antes de render_video o start_render: validate_project, review_layout y preview_frame en varias fases de CADA escena (inicio, mitad, final; antes y despues de los cues). Corregir image_cropped, image_occluded, image_distorted, text_collision, text_overflow y outside_canvas. La validacion automatica NO reemplaza la revision visual.
8) Usar start_render para trabajos largos, consultar render_status hasta finalizar y verificar enlace y audio antes de entregar. Informar IDs de presets realmente usados, escenas sin preset, modelo de voz, controles realizados y limitaciones. Nunca afirmar que se renderizo con HTML sin una capa html_widget valida.

SEGURIDAD Y LIMITACIONES: el servidor SOLO permite presets registrados y revisados; HTML detectado como detected-unverified NO se renderiza. El agente no puede inyectar HTML arbitrario por save_project. Respetar esquemas de herramientas MCP y restricciones reales de los presets. Esta instruccion orienta la produccion; no implica que el servidor cree automaticamente codigo HTML nuevo.
)GUIDE";
    return text;
}
} // namespace gptframes::mcp
