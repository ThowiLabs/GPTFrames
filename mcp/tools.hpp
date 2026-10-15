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
  "description": "Show current background filesystem scan for installed voices, animation files, images, audio, video and Lottie resources.",
  "inputSchema": {
   "type": "object",
   "properties": {},
   "additionalProperties": false
  }
 },
 {
  "name": "search_animations",
  "description": "Search indexed animations by meaning, text, category, type and CSS-derived motion tags. Returns ranked safe preset IDs, editable fields and descriptions. No arbitrary HTML execution.",
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
  "description": "Discover available animation categories, asset types, tags and CSS motion effects with counts for filter UI.",
  "inputSchema": {
   "type": "object",
   "properties": {},
   "additionalProperties": false
  }
 },
 {
  "name": "list_animations",
  "description": "Live discover new HTML animations. Curated presets are renderable; newly found unregistered HTML is marked detected-unverified and never executed. Filter by category/type/tags.",
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
  "description": "Describe one curated user-provided HTML/CSS animation by ID.",
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
  "description": "Create an editable native video project. Supports voice-timed cards, native motion symbols (phone/check/lock/angry-person), image chromaKey and optional ThorVG Lottie layers. Group labels, accents, icons and animation in atomic voice cues; preview and lint before render. See docs/VIDEO_AUTHORING_PROMPT.md and docs/MOTION_STUDIO.md.",
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
  "description": "Validate native scenes, timing, speech cues, image chromaKey, motion symbols and optional ThorVG Lottie JSON with safe project paths.",
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
  "description": "Render a PNG preview at the requested second.",
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
  "description": "Render C++ H.264/AAC MP4, including animated symbols, image chromaKey and ThorVG Lottie layers when enabled. Validate cueIndex timing, atomic groups, readable labels and proper anchored accents. Word timing is estimated.",
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
  "description": "Audit text readability, contrast, collisions and bounds; additionally inspect reactive states before and after cueIndex so no decorative bars cross text, cards appear empty or captions precede narration.",
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
  "description": "Start asynchronous native MP4 rendering with private progress/download. First validate/lint and inspect voice-cue frames; ensure all grouped decorations remain anchored, speech and graphics are aligned, and scene transitions are subtle.",
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
  "description": "Validate and atomically save editable JSON, rejecting stale ETags and unsafe media paths. For voice-driven scenes preserve cueIndex and cueOffset; keep card background, icon, accent and caption within one atomic group.",
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
