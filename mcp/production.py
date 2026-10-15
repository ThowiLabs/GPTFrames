"""Shared production requirements exposed by GPTFrames MCP.

Animation is checked against the actual JSON keyframe schema. Suggested TTS
spellings are opt-in previews; users must listen to their selected voice.
"""
from __future__ import annotations

import re
from typing import Any

GUIDELINES = {
    "default_mode": "motion_graphics_no_slideshow",
    "tts": {
        "required": "Separar SIEMPRE el texto visible del texto hablado; al TTS enviar términos escritos COMO SE PRONUNCIAN en el idioma y voz elegidos.",
        "on_screen": "Ortografía correcta: Go, Rust, C++, HirCoir; conservar sintaxis literal de comandos y código.",
        "spoken_examples": {"Go": "gou", "Rust": "rast", "C++": "ce más más", "HirCoir": "Jírcoir", "HTTP": "hache te te pe", "2026": "dos mil veintiséis"},
        "qa": "Generar muestras con el modelo exacto (Neo Lite/Piper u otro), escuchar términos complejos, acrónimos, números, fechas, unidades y nombres; corregir fonética y regenerar WAV antes del MP4. No afirmar que la pronunciación es perfecta sin escuchar.",
        "timing": "Medir la duración real de cada WAV y mantener pausas naturales; no cortar palabras ni acelerar excesivamente.",
    },
    "animation": {
        "required": "NO crear un pase de diapositivas estáticas. Cada escena debe presentar movimiento real de elementos, no solo cortes o zoom global.",
        "minimum": "Al menos una capa con keyframes que CAMBIEN propiedades (ideal 2+ capas con animaciones escalonadas) por escena; el MCP analiza x, y, opacity, scale, rotation, width y height.",
        "techniques": ["entrada/salida de títulos por capas", "movimiento y parallax de imágenes", "tarjetas escalonadas", "gráficas o tablas que aparezcan progresivamente", "transiciones limpias", "resaltados de texto", "keyframes con easing", "cambios de opacidad, escala y posición"],
        "qa": "Vista previa al inicio, mitad y final; comprobar que los componentes cambian realmente entre fotogramas. Evitar una sola captura fija por escena.",
    },
    "visual": {
        "format": "Respetar el formato solicitado: 16:9 (p.ej. 1920x1080), 9:16 o cuadrado. Para la serie actual usar 30 FPS salvo instrucción contraria.",
        "branding": "No añadir marcas de GPTFrames, contadores página/total ni marcas ajenas si el usuario no las pidió.",
        "subtitles": "Los subtítulos de la narración son OPCIONALES: no incrustarlos cuando el usuario pide no tenerlos. Los títulos informativos sí pueden aparecer como motion graphics.",
        "credits": "Usar imágenes con derechos verificados, guardar autor, URL, licencia y atribución. No inventar ni extraer música de bibliotecas privadas sin acceso autorizado.",
    },
    "delivery": [
        "Storyboard de cada escena con voz visible/hablada separadas y animación prevista.",
        "Exportar muestra de voz, escuchar pronunciaciones y medir duración antes de render.",
        "Previsualizar escenas y auditar keyframes (MCP: analyze_motion).",
        "Solo cuando esté listo renderizar y comprobar MP4, FPS, resolución, códecs, duración, subtítulos y decodificación.",
        "Entregar enlace real y archivos editables si procede; no modificar main sin autorización.",
    ],
}

# Suggestions are NOT directly applied to titles, command lines or code.
SPOKEN_REPLACEMENTS = (
    (r"(?<!\w)C\+\+(?!\w)", "ce más más"),
    (r"\bHirCoir\b", "Jírcoir"),
    (r"\bRust\b", "rast"),
    (r"\bGo\b", "gou"),
    (r"\bHTTP\b", "hache te te pe"),
    (r"\bgoroutines\b", "gou rutinas"),
    (r"\bbackend\b", "bac end"),
    (r"\bownership\b", "óunership"),
    (r"\bborrowing\b", "bórouin"),
)


def suggest_pronunciation(display_text: str) -> dict[str, Any]:
    if not isinstance(display_text, str) or not 1 <= len(display_text.strip()) <= 4000:
        raise ValueError("Texto original requerido (1–4000 caracteres)")
    spoken = display_text
    changes = []
    for pattern, replacement in SPOKEN_REPLACEMENTS:
        updated, hits = re.subn(pattern, replacement, spoken, flags=re.IGNORECASE)
        if hits:
            changes.append({"spelling": pattern, "suggested_pronunciation": replacement, "occurrences": hits})
            spoken = updated
    return {
        "display_text": display_text,
        "suggested_spoken_text": spoken,
        "changes": changes,
        "requires_listening": True,
        "instructions": "Solo propuesta editable. Escuchar la voz real, corregir y pasar el texto fonético elegido en synthesize_voice.text; mantener display_text correctamente escrito. No alterar comandos ni código en pantalla.",
    }


POSE_FIELDS = ("x", "y", "opacity", "scale", "rotation", "width", "height")
POSE_DEFAULTS = {"x": 0, "y": 0, "opacity": 1, "scale": 1, "rotation": 0, "width": 200, "height": 100}


def analyze_motion(doc: dict[str, Any]) -> dict[str, Any]:
    if not isinstance(doc, dict) or not isinstance(doc.get("scenes"), list):
        raise ValueError("El proyecto requiere escenas (scenes) en un arreglo")
    reports: list[dict[str, Any]] = []
    for i, scene in enumerate(doc["scenes"], 1):
        if not isinstance(scene, dict):
            raise ValueError(f"Escena {i} no es un objeto")
        layers = scene.get("layers", [])
        if not isinstance(layers, list):
            raise ValueError(f"Capas de escena {i} deben ser arreglo")
        animated = []
        for j, layer in enumerate(layers, 1):
            if not isinstance(layer, dict):
                continue
            keys = layer.get("keyframes") or []
            if not isinstance(keys, list) or not keys:
                continue
            # An animation must have at least one changed property over time,
            # counting the base pose and the actual sequential keyframes.
            props = []
            for field in POSE_FIELDS:
                # Match C++ project::sample: a key at t=0 supersedes the base
                # pose immediately; a base-to-zero jump is NOT an animation.
                initial = layer.get(field, POSE_DEFAULTS[field])
                points = [(0, initial)]
                current = initial
                for key in keys:
                    if not isinstance(key, dict):
                        continue
                    current = key.get(field, current)
                    instant = key.get("time", 0)
                    if instant == 0:
                        points = [(0, current)]
                    else:
                        points.append((instant, current))
                if any(t1 > t0 and repr(v1) != repr(v0)
                       for (t0, v0), (t1, v1) in zip(points, points[1:])):
                    props.append(field)
            if props:
                animated.append({"layer": j, "type": layer.get("type", "unknown"), "properties": props, "keyframe_count": len(keys)})
        reports.append({
            "scene": i,
            "duration": scene.get("duration"),
            "layers": len(layers),
            "animated_layers": len(animated),
            "animated": bool(animated),
            "animated_details": animated,
            "recommendation": ("Añade 2+ capas con keyframes escalonados, cambios visibles de posición, opacidad o escala. Evita diapositivas estáticas." if not animated else
                               "Animación mínima detectada; ideal usar capas múltiples y movimiento escalonado." if len(animated) < 2 else
                               "Movimiento por capas detectado."),
        })
    static = [item["scene"] for item in reports if not item["animated"]]
    return {
        "scene_count": len(reports),
        "total_animated_layers": sum(item["animated_layers"] for item in reports),
        "static_scenes": static,
        "meets_motion_requirement": bool(reports) and not static,
        "scenes": reports,
        "note": "Auditoría estructural de keyframes, no sustituye la inspección visual de fotogramas. Las animaciones HTML/GSAP externas no son analizadas por este inspector JSON.",
    }
