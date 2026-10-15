#pragma once
#include "common.hpp"
namespace gptframes::mcp {
inline const Json& tool_definitions() {
    static const Json tools = [] {
        auto result = Json::parse(R"JSON([
 {"name":"list_projects","description":"List available GPTFrames projects.","inputSchema":{"type":"object","properties":{},"additionalProperties":false}},
 {"name":"create_project","description":"Create an editable GPTFrames JSON video project from the included showcase.","inputSchema":{"type":"object","properties":{"name":{"type":"string","minLength":1,"maxLength":40}},"required":["name"],"additionalProperties":false}},
 {"name":"validate_project","description":"Validate scenes, layers, paths and timing using the native C++ engine.","inputSchema":{"type":"object","properties":{"project":{"type":"string"}},"required":["project"],"additionalProperties":false}},
 {"name":"preview_frame","description":"Render a PNG preview at the requested second.","inputSchema":{"type":"object","properties":{"project":{"type":"string"},"second":{"type":"number","minimum":0,"maximum":240}},"required":["project","second"],"additionalProperties":false}},
 {"name":"render_video","description":"Render MP4 with native H.264/AAC encoding.","inputSchema":{"type":"object","properties":{"project":{"type":"string"},"seconds":{"type":"number","minimum":0.1,"maximum":60}},"required":["project"],"additionalProperties":false}},
 {"name":"synthesize_voice","description":"Synthesize audio with installed C++ Neo Lite and a local ONNX/NEO model, then attach 24kHz mono WAV to the project.","inputSchema":{"type":"object","properties":{"project":{"type":"string"},"model":{"type":"string"},"text":{"type":"string","minLength":1,"maxLength":4000},"start":{"type":"number","minimum":0,"maximum":240},"track_name":{"type":"string"}},"required":["project","model","text"],"additionalProperties":false}},
 {"name":"search_images","description":"Search Wikimedia Commons or Openverse with author, source and license metadata. Results do not automatically grant reuse rights.","inputSchema":{"type":"object","properties":{"query":{"type":"string","minLength":2,"maxLength":120},"provider":{"type":"string","enum":["wikimedia","openverse"]},"limit":{"type":"integer","minimum":1,"maximum":15}},"required":["query"],"additionalProperties":false}},
 {"name":"download_commons_image","description":"Download a PNG/JPEG from Wikimedia Commons into project assets and preserve attribution in JSON.","inputSchema":{"type":"object","properties":{"project":{"type":"string"},"file_title":{"type":"string","maxLength":240},"asset_name":{"type":"string"}},"required":["project","file_title","asset_name"],"additionalProperties":false}}
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
            (type == "number" && !value.is_number()))
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
