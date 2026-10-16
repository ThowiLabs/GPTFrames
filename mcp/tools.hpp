#pragma once
#include "common.hpp"
namespace gptframes::mcp {
inline const Json& tool_definitions() {
    static const Json tools = [] {
        auto result = Json::parse(R"JSON([
 {
  "name": "rescan_resources",
  "description": "Immediately refresh resource discovery and return current resource inventory without restarting MCP.",
  "inputSchema": {
   "type": "object",
   "properties": {},
   "additionalProperties": false
  }
 },
 {
  "name": "resource_status",
  "description": "PASO 1 OBLIGATORIO en la creacion de videos: inspeccionar inventario de animaciones HTML/CSS/SVG, modelos Neo y medios instalados antes de diseñar escenas. Se actualiza en caliente.",
  "inputSchema": {
   "type": "object",
   "properties": {},
   "additionalProperties": false
  }
 },
 {
  "name": "search_animations",
  "description": "PASO OBLIGATORIO ANTES DE DISEÑAR CADA ESCENA: buscar presets HTML/CSS/SVG por concepto narrado y sinonimos. Priorizar presets registrados relevantes sobre rectangulos/tarjetas nativas basicas. Retorna IDs, etiquetas, campos editables y ranking; confirmar detalles con describe_animation antes de componer. No ejecutar HTML arbitrario.",
  "inputSchema": {
   "type": "object",
   "properties": {
    "query": {
     "type": "string",
     "minLength": 2,
     "maxLength": 160
    },
    "category": {
     "type": "string",
     "maxLength": 50
    },
    "type": {
     "type": "string",
     "maxLength": 50
    },
    "tag": {
     "type": "string",
     "maxLength": 60
    },
    "motion": {
     "type": "string",
     "maxLength": 60
    },
    "limit": {
     "type": "integer",
     "minimum": 1,
     "maximum": 30
    }
   },
   "required": [
    "query"
   ],
   "additionalProperties": false
  }
 },
 {
  "name": "list_animation_facets",
  "description": "PASO 2 OBLIGATORIO: identificar categorias, tipos, tags y estilos CSS disponibles antes de elegir efectos para el guion. No suponer una cantidad fija de recursos.",
  "inputSchema": {
   "type": "object",
   "properties": {},
   "additionalProperties": false
  }
 },
 {
  "name": "list_animations",
  "description": "Consultar la biblioteca real de animaciones HTML/CSS/SVG disponibles y filtrar por categoria/tipo/tag/motion. Solo los presets registrados y revisados se pueden renderizar; detected-unverified es exclusivamente descubrimiento, no autorizacion.",
  "inputSchema": {
   "type": "object",
   "properties": {
    "category": {
     "type": "string",
     "maxLength": 50
    },
    "type": {
     "type": "string",
     "maxLength": 50
    },
    "tag": {
     "type": "string",
     "maxLength": 60
    },
    "motion": {
     "type": "string",
     "maxLength": 60
    },
    "limit": {
     "type": "integer",
     "minimum": 1,
     "maximum": 100
    }
   },
   "additionalProperties": false
  }
 },
 {
  "name": "describe_animation",
  "description": "PASO OBLIGATORIO tras search_animations: confirmar que el ID HTML/CSS/SVG esta registrado, que es adecuado para la escena y cuales son sus campos editable, duracion, categoria y movimientos. Solo copiar propiedades existentes a html_widget; no inventarlas.",
  "inputSchema": {
   "type": "object",
   "properties": {
    "id": {
     "type": "string",
     "minLength": 1,
     "maxLength": 64
    }
   },
   "required": [
    "id"
   ],
   "additionalProperties": false
  }
 },
 {
  "name": "list_asset_presets",
  "description": "List available original animation presets and styles by optional category.",
  "inputSchema": {
   "type": "object",
   "properties": {
    "category": {
     "type": "string",
     "maxLength": 48
    }
   },
   "additionalProperties": false
  }
 },
 {
  "name": "describe_asset_preset",
  "description": "Read information about one original animation preset.",
  "inputSchema": {
   "type": "object",
   "properties": {
    "id": {
     "type": "string",
     "minLength": 1,
     "maxLength": 64
    }
   },
   "required": [
    "id"
   ],
   "additionalProperties": false
  }
 },
 {
  "name": "list_projects",
  "description": "List available GPTFrames projects.",
  "inputSchema": {
   "type": "object",
   "properties": {},
   "additionalProperties": false
  }
 },
 {
  "name": "create_project",
  "description": "BROWSER-FIRST OBLIGATORIO: crea el proyecto EDITABLE, pero la plantilla inicial C++ solo es un punto de partida. ANTES de diseñar sus escenas llama resource_status, list_animation_facets, search_animations y describe_animation; sustituye visuales basicos por capas html_widget con presets registrados de animaciones. Para videos narrados sincroniza TTS y animaciones. Imagenes importantes fit=contain. Tras componer llama validate_project, review_layout, preview_frame para cada escena y SOLO entonces render. Consulta instrucciones MCP en initialize. MOTION SINCRONIZADO OBLIGATORIO: mapa de beats y motion_events.json por elemento/frase; no aceptar diagrama completo anticipado ni loops decorativos. Adaptar variantes registradas si los tiempos del preset no encajan. Verificar cues y MP4 final con audio antes de entregar.",
  "inputSchema": {
   "type": "object",
   "properties": {
    "name": {
     "type": "string",
     "minLength": 1,
     "maxLength": 40
    }
   },
   "required": [
    "name"
   ],
   "additionalProperties": false
  }
 },
 {
  "name": "validate_project",
  "description": "PASO DE CONTROL antes del render: validar JSON de todas las escenas, incluyendo capas html_widget referidas a presets registrados y campos editables legitimos. Validar no significa aprobar la composicion visual: ejecutar review_layout y preview_frame despues.",
  "inputSchema": {
   "type": "object",
   "properties": {
    "project": {
     "type": "string"
    }
   },
   "required": [
    "project"
   ],
   "additionalProperties": false
  }
 },
 {
  "name": "preview_frame",
  "description": "VISUAL QA OBLIGATORIO PARA html_widget: previsualizar el preset HTML/SVG realmente insertado y su secuencia a inicio/medio/final de cada escena, incluyendo cambios de voz; verificar imagen completa, tipografia, animacion y ausencias de capas que tapen contenido. No aceptar solo el JSON. MOTION SINCRONIZADO OBLIGATORIO: mapa de beats y motion_events.json por elemento/frase; no aceptar diagrama completo anticipado ni loops decorativos. Adaptar variantes registradas si los tiempos del preset no encajan. Verificar cues y MP4 final con audio antes de entregar.",
  "inputSchema": {
   "type": "object",
   "properties": {
    "project": {
     "type": "string"
    },
    "second": {
     "type": "number",
     "minimum": 0,
     "maximum": 240
    }
   },
   "required": [
    "project",
    "second"
   ],
   "additionalProperties": false
  }
 },
 {
  "name": "render_video",
  "description": "RENDER SOLO TRAS BROWSER-FIRST: busca y selecciona las animaciones existentes por escena (search_animations, describe_animation), inserta html_widget registrados para el contenido pertinente y utiliza capas nativas solo como complemento. Antes del render ejecuta validate_project, review_layout y preview_frame en todas las escenas, verifica tiempos de narracion y evita recortes, oclusiones, deformaciones y textos truncados. Salida H.264/AAC sin garantia automatica de calidad visual. MOTION SINCRONIZADO OBLIGATORIO: mapa de beats y motion_events.json por elemento/frase; no aceptar diagrama completo anticipado ni loops decorativos. Adaptar variantes registradas si los tiempos del preset no encajan. Verificar cues y MP4 final con audio antes de entregar.",
  "inputSchema": {
   "type": "object",
   "properties": {
    "project": {
     "type": "string"
    },
    "seconds": {
     "type": "number",
     "minimum": 0.1,
     "maximum": 60
    }
   },
   "required": [
    "project"
   ],
   "additionalProperties": false
  }
 },
 {
  "name": "synthesize_voice",
  "description": "Synthesize audio with installed C++ Neo Lite and a local ONNX/NEO model, then attach 24kHz mono WAV to the project.",
  "inputSchema": {
   "type": "object",
   "properties": {
    "project": {
     "type": "string"
    },
    "model": {
     "type": "string"
    },
    "text": {
     "type": "string",
     "minLength": 1,
     "maxLength": 4000
    },
    "start": {
     "type": "number",
     "minimum": 0,
     "maximum": 240
    },
    "track_name": {
     "type": "string"
    }
   },
   "required": [
    "project",
    "model",
    "text"
   ],
   "additionalProperties": false
  }
 },
 {
  "name": "search_images",
  "description": "Search Wikimedia Commons or Openverse with author, source and license metadata. Results do not automatically grant reuse rights.",
  "inputSchema": {
   "type": "object",
   "properties": {
    "query": {
     "type": "string",
     "minLength": 2,
     "maxLength": 120
    },
    "provider": {
     "type": "string",
     "enum": [
      "wikimedia",
      "openverse"
     ]
    },
    "limit": {
     "type": "integer",
     "minimum": 1,
     "maximum": 15
    }
   },
   "required": [
    "query"
   ],
   "additionalProperties": false
  }
 },
 {
  "name": "download_commons_image",
  "description": "Download a PNG/JPEG from Wikimedia Commons into project assets and preserve attribution in JSON.",
  "inputSchema": {
   "type": "object",
   "properties": {
    "project": {
     "type": "string"
    },
    "file_title": {
     "type": "string",
     "maxLength": 240
    },
    "asset_name": {
     "type": "string"
    }
   },
   "required": [
    "project",
    "file_title",
    "asset_name"
   ],
   "additionalProperties": false
  }
 },
 {
  "name": "list_voice_models",
  "description": "List installed local .neo and .onnx voices with live discovery on every request, no MCP restart.",
  "inputSchema": {
   "type": "object",
   "properties": {},
   "additionalProperties": false
  }
 },
 {
  "name": "review_layout",
  "description": "GATE BROWSER-FIRST: audita composicion antes de renderizar y comprueba que las escenas didacticas usen html_widget registrados cuando haya presets relevantes; revisar image_cropped/image_distorted/image_occluded, texto legible, contraste, colisiones y limites. Lint no sustituye preview_frame en principio/mitad/final de todas las escenas. MOTION SINCRONIZADO OBLIGATORIO: mapa de beats y motion_events.json por elemento/frase; no aceptar diagrama completo anticipado ni loops decorativos. Adaptar variantes registradas si los tiempos del preset no encajan. Verificar cues y MP4 final con audio antes de entregar.",
  "inputSchema": {
   "type": "object",
   "properties": {
    "project": {
     "type": "string"
    }
   },
   "required": [
    "project"
   ],
   "additionalProperties": false
  }
 },
 {
  "name": "start_render",
  "description": "RENDER SOLO DESPUES DEL GATE BROWSER-FIRST: documentar los presets HTML/CSS/SVG que se han seleccionado y estan efectivamente insertados como capas html_widget; validar el JSON y revisar fotogramas de cada escena con review_layout/preview_frame; sincronizar con narracion y no tapar fotos o textos. Trabajos largos se revisan con render_status. No afirmar calidad automatica solo por exportar. MOTION SINCRONIZADO OBLIGATORIO: mapa de beats y motion_events.json por elemento/frase; no aceptar diagrama completo anticipado ni loops decorativos. Adaptar variantes registradas si los tiempos del preset no encajan. Verificar cues y MP4 final con audio antes de entregar.",
  "inputSchema": {
   "type": "object",
   "properties": {
    "project": {
     "type": "string"
    },
    "seconds": {
     "type": "number",
     "minimum": 0.1,
     "maximum": 240
    }
   },
   "required": [
    "project"
   ],
   "additionalProperties": false
  }
 },
 {
  "name": "render_status",
  "description": "Read current render progress and result by job ID.",
  "inputSchema": {
   "type": "object",
   "properties": {
    "job_id": {
     "type": "string"
    }
   },
   "required": [
    "job_id"
   ],
   "additionalProperties": false
  }
 },
 {
  "name": "cancel_render",
  "description": "Cancel an active or queued native render by job ID.",
  "inputSchema": {
   "type": "object",
   "properties": {
    "job_id": {
     "type": "string"
    }
   },
   "required": [
    "job_id"
   ],
   "additionalProperties": false
  }
 },
 {
  "name": "read_project",
  "description": "Read an editable composition JSON and its optimistic concurrency ETag.",
  "inputSchema": {
   "type": "object",
   "properties": {
    "project": {
     "type": "string"
    }
   },
   "required": [
    "project"
   ],
   "additionalProperties": false
  }
 },
 {
  "name": "save_project",
  "description": "Guardar JSON editable por ETag. BROWSER-FIRST: insertar capas html_widget reales con collection=animaciones (o originales), preset=ID verificado por describe_animation y properties SOLO de editable; priorizar la biblioteca existente sobre tarjetas C++ vacias. Mantener grupos/cueIndex sincronizados con la narracion; no se admite HTML remoto o no registrado. MOTION SINCRONIZADO OBLIGATORIO: mapa de beats y motion_events.json por elemento/frase; no aceptar diagrama completo anticipado ni loops decorativos. Adaptar variantes registradas si los tiempos del preset no encajan. Verificar cues y MP4 final con audio antes de entregar.",
  "inputSchema": {
   "type": "object",
   "properties": {
    "project": {
     "type": "string"
    },
    "etag": {
     "type": "string",
     "minLength": 43,
     "maxLength": 43
    },
    "document": {
     "type": "object"
    }
   },
   "required": [
    "project",
    "etag",
    "document"
   ],
   "additionalProperties": false
  }
 }
])JSON");
        for (auto& tool : result) {
            const auto scheme = Json::array({{{"type", "oauth2"}, {"scopes", Json::array({"gptframes"})}}});
            tool["securitySchemes"] = scheme;
            tool["_meta"] = {{"securitySchemes", scheme}};
        }
        return result;
    }();
    return tools;
}
inline void validate_arguments(const std::string& name, const Json& arguments) {
    if (!arguments.is_object()) throw std::invalid_argument("arguments debe ser un objeto JSON");
    const Json* schema = nullptr;
    for (const auto& tool : tool_definitions()) if (tool["name"] == name) { schema = &tool["inputSchema"]; break; }
    if (!schema) throw std::invalid_argument("Herramienta no reconocida");
    if (schema->contains("required"))
        for (const auto& key : (*schema)["required"]) if (!arguments.contains(key.get<std::string>()))
            throw std::invalid_argument("Falta argumento: " + key.get<std::string>());
    const auto& properties = (*schema)["properties"];
    for (const auto& item : arguments.items()) {
        if (!properties.contains(item.key())) throw std::invalid_argument("Argumento no reconocido: " + item.key());
        const auto& spec = properties[item.key()];
        const auto& value = item.value();
        const auto type = spec["type"].get<std::string>();
        if ((type == "string" && !value.is_string()) || (type == "integer" && !value.is_number_integer()) ||
            (type == "number" && !value.is_number()) || (type == "object" && !value.is_object()) || (type == "array" && !value.is_array()))
            throw std::invalid_argument("Tipo incorrecto: " + item.key());
        if (value.is_number()) {
            const double number = value.get<double>();
            if (!std::isfinite(number) || (spec.contains("minimum") && number < spec["minimum"].get<double>()) ||
                (spec.contains("maximum") && number > spec["maximum"].get<double>()))
                throw std::invalid_argument("Numero fuera de rango: " + item.key());
        }
        if (value.is_string()) {
            const auto text = value.get<std::string>();
            const auto length = utf8_length(text);
            if (text.find('\0') != std::string::npos || (spec.contains("minLength") && length < spec["minLength"].get<std::size_t>()) ||
                (spec.contains("maxLength") && length > spec["maxLength"].get<std::size_t>()))
                throw std::invalid_argument("Texto invalido o demasiado largo: " + item.key());
        }
        if (spec.contains("enum") && std::find(spec["enum"].begin(), spec["enum"].end(), value) == spec["enum"].end())
            throw std::invalid_argument("Valor no permitido: " + item.key());
    }
}
} // namespace gptframes::mcp
