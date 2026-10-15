#pragma once
#include "service.hpp"
#include "oauth.hpp"
#include <regex>

namespace gptframes::mcp {
inline bool allowed_origin(const std::string& origin) {
    if (origin.empty()) return true;
    static const std::regex loopback(R"(^https?://(localhost|127\.0\.0\.1|\[::1\])(:[0-9]+)?$)");
    if (std::regex_match(origin, loopback) || origin == "https://chatgpt.com" || origin == "https://chat.openai.com" || origin == "https://www.chatgpt.com") return true;
    std::istringstream configured(environment("GPTFRAMES_ALLOWED_ORIGINS"));
    std::string item;
    while (std::getline(configured, item, ',')) if (origin == trim(item)) return true;
    return false;
}
inline void configure_http(httplib::Server& server, Service& service, const std::shared_ptr<OAuth>& oauth) {
    if (!oauth) throw std::invalid_argument("HTTP requires PIN/OAuth");
    server.set_payload_max_length(max_message_bytes);
    server.set_read_timeout(10);
    server.set_write_timeout(10);
    server.new_task_queue = [] { return new httplib::ThreadPool(4); };
    server.Get("/health", [](const httplib::Request&, httplib::Response& response) {
        response.set_content(R"({"status":"ok"})", "application/json");
        response.set_header("Cache-Control", "no-store");
    });
    server.Get("/", [](const httplib::Request&, httplib::Response& response) {
        response.set_content(R"({"name":"GPTFrames","connection":"ChatGPT OAuth","mcp":"/mcp"})", "application/json");
        response.set_header("Cache-Control", "no-store");
    });
    for (const auto* route : {"/.well-known/oauth-protected-resource", "/.well-known/oauth-protected-resource/mcp"})
        server.Get(route, [oauth](const httplib::Request& request, httplib::Response& response) { oauth->handle("resource", request, response); });
    server.Get("/.well-known/oauth-authorization-server",
        [oauth](const httplib::Request& request, httplib::Response& response) { oauth->handle("metadata", request, response); });
    server.Post("/oauth/register",
        [oauth](const httplib::Request& request, httplib::Response& response) { oauth->handle("register", request, response); });
    server.Get("/oauth/authorize",
        [oauth](const httplib::Request& request, httplib::Response& response) { oauth->handle("authorize", request, response); });
    server.Post("/oauth/authorize",
        [oauth](const httplib::Request& request, httplib::Response& response) { oauth->handle("authorize", request, response); });
    server.Post("/oauth/token",
        [oauth](const httplib::Request& request, httplib::Response& response) { oauth->handle("token", request, response); });
    const auto authenticated = [oauth](const httplib::Request& request, httplib::Response& response) {
        response.set_header("Cache-Control", "no-store");
        if (oauth->authenticated(request)) return true;
        try {
            const auto challenge = oauth->challenge();
            response.status = 401;
            response.set_header("WWW-Authenticate", challenge);
            Json error{{"error", "authentication_required"}, {"connection", "ChatGPT OAuth"}};
            if (request.method == "POST" && request.body.size() <= max_message_bytes) {
                const auto call = Json::parse(request.body, nullptr, false);
                if (call.is_object() && call.contains("id") && call.value("method", Json{}) == "tools/call")
                    error = {{"jsonrpc", "2.0"}, {"id", call["id"]}, {"result", {
                        {"content", Json::array({{{"type", "text"}, {"text", "Connect GPTFrames with ChatGPT and your PIN."}}})},
                        {"_meta", {{"mcp/www_authenticate", Json::array({challenge})}}}, {"isError", true}}}};
            }
            response.set_content(error.dump(), "application/json");
        } catch (const std::exception&) {
            response.status = 503; response.set_content(R"({"error":"initializing"})", "application/json");
        }
        return false;
    };
    server.Get("/mcp", [authenticated](const httplib::Request& request, httplib::Response& response) {
        if (authenticated(request, response)) response.status = 405;
    });
    server.Delete("/mcp", [authenticated](const httplib::Request& request, httplib::Response& response) {
        if (authenticated(request, response)) response.status = 405;
    });
    server.Post("/mcp", [&service, authenticated, oauth](const httplib::Request& request, httplib::Response& response) {
        if (!authenticated(request, response)) return;
        if (request.get_header_value_count("Origin") > 1 || (!allowed_origin(request.get_header_value("Origin")) && !oauth->owns_origin(request.get_header_value("Origin")))) {
            response.status = 403; return;
        }
        if (request.body.empty() || request.body.size() > max_message_bytes) { response.status = 413; return; }
        const auto type = lower(trim(request.get_header_value("Content-Type")));
        if (type != "application/json" && type.compare(0, 17, "application/json;") != 0) { response.status = 415; return; }
        try {
            const auto call = Json::parse(request.body);
            const auto version = request.get_header_value("MCP-Protocol-Version");
            const Json supported = Json::array({"2026-07-28", "2025-11-25", "2025-06-18", "2025-03-26"});
            auto reject = [&](int code, const std::string& message, const Json& data = Json{}) {
                response.status = 400;
                Json error{{"code", code}, {"message", message}};
                if (!data.is_null()) error["data"] = data;
                response.set_content(Json{{"jsonrpc", "2.0"}, {"id", call.is_object() ? call.value("id", Json{}) : Json{}},
                    {"error", error}}.dump(), "application/json");
            };
            if (request.get_header_value_count("MCP-Protocol-Version") > 1 ||
                (!version.empty() && std::find(supported.begin(), supported.end(), version) == supported.end())) {
                reject(-32022, "Unsupported protocol version", {{"supported", supported}, {"requested", version}}); return;
            }
            if (version == "2026-07-28") {
                const auto method = call.is_object() ? call.value("method", Json{}) : Json{};
                const auto params = call.is_object() ? call.value("params", Json{}) : Json{};
                const auto meta = params.is_object() ? params.value("_meta", Json{}) : Json{};
                if (!method.is_string() || request.get_header_value_count("Mcp-Method") != 1 ||
                    request.get_header_value("Mcp-Method") != method.get<std::string>() ||
                    !meta.is_object() || meta.value("io.modelcontextprotocol/protocolVersion", Json{}) != version ||
                    !meta.value("io.modelcontextprotocol/clientCapabilities", Json{}).is_object()) {
                    reject(-32020, "Invalid protocol headers or metadata"); return;
                }
                if (method == "tools/call" &&
                    (request.get_header_value_count("Mcp-Name") != 1 ||
                     params.value("name", Json{}) != request.get_header_value("Mcp-Name"))) {
                    reject(-32020, "Tool name header does not match request"); return;
                }
            }
            const auto result = service.rpc(call, version.empty() ? "2025-06-18" : version);
            if (!result) { response.status = 202; return; }
            response.set_content(result->dump(), "application/json");
        } catch (const std::exception&) { response.set_content(parse_error().dump(), "application/json"); }
    });
}
} // namespace gptframes::mcp
