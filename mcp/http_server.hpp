#pragma once
#include "service.hpp"
#include "../vendor/httplib/httplib.h"
#include <regex>

namespace gptframes::mcp {
inline bool allowed_origin(const std::string& origin) {
    if (origin.empty()) return true;
    static const std::regex loopback(R"(^https?://(localhost|127\.0\.0\.1|\[::1\])(:[0-9]+)?$)");
    if (std::regex_match(origin, loopback)) return true;
    std::istringstream configured(environment("GPTFRAMES_ALLOWED_ORIGINS"));
    std::string item;
    while (std::getline(configured, item, ',')) if (origin == trim(item)) return true;
    return false;
}
inline void configure_http(httplib::Server& server, Service& service, const std::string& token) {
    if (token.size() < 24) throw std::invalid_argument("HTTP MCP requiere GPTFRAMES_MCP_TOKEN de al menos 24 caracteres");
    server.set_payload_max_length(max_message_bytes);
    server.set_read_timeout(10);
    server.set_write_timeout(10);
    server.new_task_queue = [] { return new httplib::ThreadPool(4); };
    server.Get("/health", [](const httplib::Request&, httplib::Response& response) {
        response.set_content(R"({"status":"ok"})", "application/json");
        response.set_header("Cache-Control", "no-store");
    });
    server.Get("/mcp", [](const httplib::Request&, httplib::Response& response) { response.status = 405; });
    server.Post("/mcp", [&service, token](const httplib::Request& request, httplib::Response& response) {
        response.set_header("Cache-Control", "no-store");
        if (request.get_header_value_count("Authorization") != 1 ||
            !constant_time_equal(request.get_header_value("Authorization"), "Bearer " + token)) {
            response.status = 401;
            response.set_header("WWW-Authenticate", "Bearer realm=\"GPTFrames\"");
            return;
        }
        if (!allowed_origin(request.get_header_value("Origin"))) { response.status = 403; return; }
        if (request.body.empty() || request.body.size() > max_message_bytes) { response.status = 413; return; }
        if (request.get_header_value("Content-Type").find("application/json") != 0) { response.status = 415; return; }
        try {
            const auto result = service.rpc(Json::parse(request.body));
            if (!result) { response.status = 202; return; }
            response.set_content(result->dump(), "application/json");
        } catch (const std::exception&) {
            response.set_content(parse_error().dump(), "application/json");
        }
    });
    server.Delete("/mcp", [](const httplib::Request&, httplib::Response& response) { response.status = 405; });
}
} // namespace gptframes::mcp
