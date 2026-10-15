#pragma once
#include "oauth_crypto.hpp"
#include "http_client.hpp"
#include "../vendor/httplib/httplib.h"
#include <functional>
#include <map>
#include <set>

namespace gptframes::mcp {
inline std::string private_text(const fs::path& path, std::size_t maximum) {
    if (fs::is_symlink(path) || !fs::is_regular_file(path)) throw std::runtime_error("Private configuration missing");
#ifndef _WIN32
    const auto permissions = fs::status(path).permissions();
    if ((permissions & (fs::perms::group_all | fs::perms::others_all)) != fs::perms::none)
        throw std::runtime_error("Private configuration requires mode 600");
#endif
    return trim(read_file(path, maximum));
}
inline void private_directory(const fs::path& path) {
    if (fs::is_symlink(path)) throw std::runtime_error("Private directory cannot be a symlink");
    if (!fs::exists(path)) {
        fs::create_directories(path);
        fs::permissions(path, fs::perms::owner_all, fs::perm_options::replace);
    }
#ifndef _WIN32
    if ((fs::status(path).permissions() & (fs::perms::group_all | fs::perms::others_all)) != fs::perms::none)
        throw std::runtime_error("Private directory requires mode 700");
#endif
}
inline std::string public_origin(std::string value) {
    if (!value.empty() && value.back() == '/') value.pop_back();
    if (value.compare(0, 8, "https://") || value.size() > 250) throw std::invalid_argument("Public URL requires an HTTPS origin");
    const auto host = value.substr(8);
    if (host.empty() || !std::all_of(host.begin(), host.end(), [](unsigned char c) {
            return ascii_alnum(c) || c == '-' || c == '.'; }) || host.front() == '.' || host.back() == '.')
        throw std::invalid_argument("Public URL requires an HTTPS origin without path or credentials");
    return value;
}
inline bool chatgpt_redirect(const std::string& value) {
    if (value == "https://chatgpt.com/connector_platform_oauth_redirect") return true;
    const std::string prefix = "https://chatgpt.com/connector/oauth/";
    return value.compare(0, prefix.size(), prefix) == 0 && value.size() > prefix.size() &&
        value.size() < 300 && std::all_of(value.begin() + static_cast<std::ptrdiff_t>(prefix.size()), value.end(),
            [](unsigned char c) { return ascii_alnum(c) || c == '-' || c == '_'; });
}
inline std::string html_escape(const std::string& value) {
    std::string result;
    for (char c : value) {
        if (c == '&') result += "&amp;";
        else if (c == '<') result += "&lt;";
        else if (c == '>') result += "&gt;";
        else if (c == '"') result += "&quot;";
        else if (c == '\'') result += "&#39;";
        else result += c;
    }
    return result;
}
struct OAuthConfig {
    std::string pin, origin;
    fs::path origin_file, clients_file;
    static OAuthConfig from_environment(const fs::path& root) {
        OAuthConfig c;
        c.pin = environment("GPTFRAMES_OAUTH_PIN");
        if (c.pin.empty()) {
            const auto file = environment("GPTFRAMES_OAUTH_PIN_FILE");
            if (!file.empty()) c.pin = private_text(fs::u8path(file), 256);
        }
        c.origin = environment("GPTFRAMES_PUBLIC_URL");
        const auto origin_file = environment("GPTFRAMES_PUBLIC_URL_FILE");
        if (!origin_file.empty()) c.origin_file = fs::u8path(origin_file);
        c.clients_file = fs::u8path(environment("GPTFRAMES_OAUTH_STATE",
                         (root / "build/runtime/oauth-clients.json").u8string()));
        return c;
    }
};
class OAuth {
    using Clock = std::function<std::int64_t()>;
    struct Client { std::string name; std::set<std::string> redirects; std::int64_t expires = 0; };
    struct Grant {
        std::string client, redirect, challenge, state, scope, resource;
        std::int64_t expires = 0;
        std::string csrf_hash;
    };
    struct Token {
        std::string client, scope, resource, family;
        std::int64_t expires = 0;
        bool used = false;
    };
    OAuthConfig config_;
    Clock clock_;
    std::string salt_, pin_hash_, origin_;
    std::mutex mutex_;
    std::map<std::string, Client> clients_;
    std::map<std::string, Grant> pending_, codes_;
    std::map<std::string, Token> access_, refresh_;
    std::int64_t pin_window_ = 0, register_window_ = 0, authorize_window_ = 0;
    int pin_attempts_ = 0, registrations_ = 0, authorizations_ = 0;
    std::string origin_locked() {
        if (origin_.empty()) {
            if (!config_.origin.empty()) origin_ = public_origin(config_.origin);
            else if (!config_.origin_file.empty() && fs::exists(config_.origin_file))
                origin_ = public_origin(private_text(config_.origin_file, 512));
            else throw std::runtime_error("Public URL initializing");
        }
        return origin_;
    }
    std::string resource_locked() { return origin_locked() + "/mcp"; }
    static std::string param(const httplib::Request& request, const std::string& name, bool required = false) {
        if (request.params.count(name) > 1) throw std::invalid_argument("Duplicate OAuth parameter");
        const auto value = request.get_param_value(name);
        if (value.size() > 2048 || value.find_first_of("\r\n") != std::string::npos ||
            value.find('\0') != std::string::npos || (required && value.empty()))
            throw std::invalid_argument("Invalid OAuth parameter");
        return value;
    }
    static void json(httplib::Response& response, int status, const Json& value) {
        response.status = status;
        response.set_header("Cache-Control", "no-store");
        response.set_header("Pragma", "no-cache");
        response.set_content(value.dump(), "application/json");
    }
    static void error(httplib::Response& response, const std::string& code, int status = 400) {
        json(response, status, {{"error", code}});
    }
    static std::string csrf_cookie(const httplib::Request& request) {
        std::string value;
        for (const auto& header : request.headers) {
            if (lower(header.first) != "cookie") continue;
            std::istringstream cookies(header.second); std::string item;
            while (std::getline(cookies, item, ';')) {
                item = trim(item);
                const std::string prefix = "__Secure-gptframes-csrf=";
                if (item.compare(0, prefix.size(), prefix) == 0) {
                    if (!value.empty()) return {};
                    value = item.substr(prefix.size());
                }
            }
        }
        return value;
    }
    static void authorization_error(httplib::Response& response, const std::string& message, int status = 403) {
        response.status = status;
        response.set_content("<!doctype html><html lang=\"es\"><meta charset=\"utf-8\"><title>Reconectar GPTFrames</title>"
            "<main><h1>No se pudo completar la conexión</h1><p>" + html_escape(message) +
            "</p><p>Cierra esta ventana e inicia de nuevo la conexión desde ChatGPT.</p></main></html>", "text/html; charset=utf-8");
    }
    static void security(httplib::Response& response) {
        response.set_header("Cache-Control", "no-store");
        response.set_header("Referrer-Policy", "no-referrer");
        response.set_header("X-Content-Type-Options", "nosniff");
        response.set_header("Content-Security-Policy",
            "default-src 'none'; style-src 'unsafe-inline'; form-action 'self'; frame-ancestors 'none'; base-uri 'none'");
        response.set_header("X-Frame-Options", "DENY");
    }
    template<class Map> void prune(Map& map) {
        const auto now = clock_();
        for (auto it = map.begin(); it != map.end();) {
            if (it->second.expires <= now) it = map.erase(it); else ++it;
        }
    }
    void prune_all() {
        prune(pending_); prune(codes_); prune(access_); prune(refresh_);
        for (auto it = clients_.begin(); it != clients_.end();) {
            if (it->second.expires && it->second.expires <= clock_()) it = clients_.erase(it); else ++it;
        }
    }
    bool rate(std::int64_t& window, int& count, int limit) {
        const auto now = clock_();
        if (now - window >= 60 || now < window) { window = now; count = 0; }
        return ++count <= limit;
    }
    void save_clients() {
        if (config_.clients_file.empty()) return;
        Json records = Json::array();
        for (const auto& [id, c] : clients_) if (!c.expires)
            records.push_back({{"id", id}, {"name", c.name}, {"redirects", c.redirects}});
        atomic_write(config_.clients_file, Json{{"version", 1}, {"clients", records}}.dump(2) + "\n");
        fs::permissions(config_.clients_file, fs::perms::owner_read | fs::perms::owner_write, fs::perm_options::replace);
    }
    std::string scopes(const std::string& text) {
        if (text.empty()) return "gptframes";
        std::istringstream input(text); std::string item; std::set<std::string> requested;
        while (input >> item) {
            if (item != "gptframes" && item != "offline_access") throw std::invalid_argument("Unsupported OAuth scope");
            requested.insert(item);
        }
        if (!requested.count("gptframes")) throw std::invalid_argument("Missing tool scope");
        return requested.count("offline_access") ? "gptframes offline_access" : "gptframes";
    }
    void render_pin(httplib::Response& response, const std::string& transaction, const Grant& grant,
                    const std::string& csrf, const std::string& message = {}) {
        security(response);
        if (response.status < 0) response.status = 200;
        const auto name = clients_.at(grant.client).name;
        const std::string html = std::string(R"HTML(<!doctype html><html lang="es"><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1"><title>Conectar GPTFrames</title>
<style>body{margin:0;background:#0d1422;color:#edf3fc;font:17px system-ui,sans-serif;display:grid;place-items:center;min-height:100vh}
main{max-width:430px;margin:24px;padding:34px;background:#172238;border-radius:20px;box-shadow:0 24px 80px #0005}
small{color:#9cc8ff;letter-spacing:2px}h1{font-size:30px;margin:16px 0}p{line-height:1.5;color:#c5d2e4}
input{box-sizing:border-box;width:100%;padding:15px;font-size:23px;letter-spacing:3px;border-radius:10px;border:1px solid #526784;background:#0d1422;color:white;margin:12px 0}
button{width:100%;padding:14px;border:0;border-radius:10px;font:inherit;font-weight:650;background:#94c6ff;color:#102139;cursor:pointer;margin-top:10px}
button[value=deny]{background:transparent;color:#b9c8dc}.error{color:#ffb5b5}code{overflow-wrap:anywhere;font-size:12px}</style>
<main><small>GPTFRAMES</small><h1>Conecta con ChatGPT</h1>
<p>Introduce tu PIN privado para autorizar a ChatGPT a crear proyectos, buscar imágenes y generar voz y videos.</p>
)HTML") + (message.empty() ? "" : "<p class=\"error\">" + html_escape(message) + "</p>") +
            "<p>Cliente: " + html_escape(name) + "<br><code>" + html_escape(grant.redirect) + "</code></p>" +
            "<form method=\"post\" action=\"/oauth/authorize\"><input type=\"hidden\" name=\"transaction\" value=\"" +
            transaction + "\"><input type=\"hidden\" name=\"csrf\" value=\"" + html_escape(csrf) + "\"><label for=\"pin\">PIN de conexión</label><input id=\"pin\" name=\"pin\" type=\"password\" " +
            "autocomplete=\"one-time-code\" maxlength=\"128\" autofocus required>" +
            "<button type=\"submit\" name=\"decision\" value=\"approve\">Conectar GPTFrames</button>" +
            "<button type=\"submit\" name=\"decision\" value=\"deny\" formnovalidate>Cancelar</button></form></main></html>";
        response.set_content(html, "text/html; charset=utf-8");
    }
    void redirect(httplib::Response& response, const Grant& grant, const std::string& key, const std::string& value) {
        std::string location = grant.redirect + "?" + key + "=" + encode_query(value) +
                               "&iss=" + encode_query(origin_locked());
        if (!grant.state.empty()) location += "&state=" + encode_query(grant.state);
        // Match kaggle-mcp's HTML handoff. A cross-origin HTTP redirect after a
        // form POST is blocked by the PIN document's form-action 'self' CSP.
        security(response);
        response.headers.erase("Content-Security-Policy");
        response.set_header("Content-Security-Policy",
            "default-src 'none'; style-src 'unsafe-inline'; frame-ancestors 'none'; base-uri 'none'");
        response.set_header("Set-Cookie", "__Secure-gptframes-csrf=; Path=/oauth/authorize; Secure; HttpOnly; SameSite=Lax; Max-Age=0");
        response.status = 200;
        const auto target = html_escape(location);
        response.set_content("<!doctype html><html lang=\"es\"><head><meta charset=\"utf-8\">"
            "<meta http-equiv=\"refresh\" content=\"0; url=" + target + "\"><title>Volviendo a ChatGPT</title></head>"
            "<body><main><h1>Volviendo a ChatGPT</h1><p>Si no continúa automáticamente, pulsa el enlace.</p>"
            "<a href=\"" + target + "\" rel=\"noreferrer\">Continuar en ChatGPT</a></main></body></html>", "text/html; charset=utf-8");
    }
    Json issue(const Grant& grant, std::string family = {}, std::int64_t refresh_expiry = 0) {
        prune_all();
        if (access_.size() >= 4096 || refresh_.size() >= 4096) throw std::runtime_error("OAuth capacity");
        const auto token = oauth_random_token();
        if (family.empty()) family = oauth_random_token();
        access_[oauth_digest(token)] = {grant.client, grant.scope, grant.resource, family, clock_() + 3600, false};
        Json result{{"access_token", token}, {"token_type", "Bearer"}, {"expires_in", 3600},
                    {"scope", grant.scope}, {"resource", grant.resource}};
        if (grant.scope.find("offline_access") != std::string::npos) {
            const auto refresh = oauth_random_token();
            refresh_[oauth_digest(refresh)] = {grant.client, grant.scope, grant.resource, family,
                                              refresh_expiry ? refresh_expiry : clock_() + 86400, false};
            result["refresh_token"] = refresh;
        }
        return result;
    }
    void revoke_family(const std::string& family) {
        for (auto it = access_.begin(); it != access_.end();) {
            if (it->second.family == family) it = access_.erase(it); else ++it;
        }
        for (auto it = refresh_.begin(); it != refresh_.end();) {
            if (it->second.family == family) it = refresh_.erase(it); else ++it;
        }
    }
public:
    explicit OAuth(OAuthConfig config, Clock clock = [] {
        return std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }) : config_(std::move(config)), clock_(std::move(clock)), salt_(oauth_random_bytes(32)) {
        if (config_.pin.size() < 6 || config_.pin.size() > 128 || config_.pin != trim(config_.pin) ||
            config_.pin.find_first_of("\r\n") != std::string::npos || config_.pin.find('\0') != std::string::npos)
            throw std::invalid_argument("HTTP requires GPTFRAMES_OAUTH_PIN_FILE or GPTFRAMES_OAUTH_PIN (6-128 characters)");
        if (config_.origin.empty() && config_.origin_file.empty())
            throw std::invalid_argument("HTTP requires GPTFRAMES_PUBLIC_URL or GPTFRAMES_PUBLIC_URL_FILE");
        if (!config_.origin.empty()) origin_ = public_origin(config_.origin);
        pin_hash_ = oauth_sha256(salt_ + config_.pin);
        std::fill(config_.pin.begin(), config_.pin.end(), '\0'); config_.pin.clear();
        if (!config_.clients_file.empty()) {
            private_directory(config_.clients_file.parent_path());
            if (fs::exists(config_.clients_file) || fs::is_symlink(config_.clients_file)) {
                const auto document = Json::parse(private_text(config_.clients_file, max_message_bytes));
                if (document.value("version", 0) != 1 || !document.contains("clients") ||
                    !document["clients"].is_array() || document["clients"].size() > 128)
                    throw std::runtime_error("Invalid OAuth registration state");
                for (const auto& record : document["clients"]) {
                    Client client{record.at("name").get<std::string>(), record.at("redirects").get<std::set<std::string>>()};
                    const auto id = record.at("id").get<std::string>();
                    if (id.size() != 43 || client.name.size() > 80 || client.redirects.empty() ||
                        client.redirects.size() > 8 || !std::all_of(client.redirects.begin(), client.redirects.end(), chatgpt_redirect))
                        throw std::runtime_error("Invalid OAuth registration state");
                    clients_[id] = std::move(client);
                }
            }
        }
    }
    bool owns_origin(const std::string& origin) {
        std::lock_guard<std::mutex> lock(mutex_);
        return !origin.empty() && origin == origin_locked();
    }
    std::string challenge() {
        std::lock_guard<std::mutex> lock(mutex_);
        return "Bearer resource_metadata=\"" + origin_locked() +
            "/.well-known/oauth-protected-resource\", scope=\"gptframes\", error=\"invalid_token\", "
            "error_description=\"Connect GPTFrames with ChatGPT and your PIN\"";
    }
    bool authenticated(const httplib::Request& request) {
        if (request.get_header_value_count("Authorization") != 1) return false;
        const auto header = request.get_header_value("Authorization");
        if (header.compare(0, 7, "Bearer ") || header.size() != 50) return false;
        std::lock_guard<std::mutex> lock(mutex_);
        prune(access_);
        const auto found = access_.find(oauth_digest(header.substr(7)));
        return found != access_.end() && found->second.expires > clock_() &&
               found->second.resource == resource_locked() &&
               found->second.scope.find("gptframes") == 0;
    }
    void handle(const std::string& route, const httplib::Request& request, httplib::Response& response) {
        security(response);
        std::lock_guard<std::mutex> lock(mutex_);
        try {
            const auto issuer = origin_locked(), resource = resource_locked();
            if (route == "resource") {
                json(response, 200, {{"resource", resource}, {"authorization_servers", Json::array({issuer})},
                    {"scopes_supported", Json::array({"gptframes"})}, {"bearer_methods_supported", Json::array({"header"})},
                    {"resource_name", "GPTFrames"}});
                return;
            }
            if (route == "metadata") {
                json(response, 200, {{"issuer", issuer}, {"authorization_endpoint", issuer + "/oauth/authorize"},
                    {"token_endpoint", issuer + "/oauth/token"}, {"registration_endpoint", issuer + "/oauth/register"},
                    {"authorization_response_iss_parameter_supported", true}, {"response_types_supported", Json::array({"code"})},
                    {"grant_types_supported", Json::array({"authorization_code", "refresh_token"})},
                    {"token_endpoint_auth_methods_supported", Json::array({"none"})},
                    {"code_challenge_methods_supported", Json::array({"S256"})},
                    {"scopes_supported", Json::array({"gptframes", "offline_access"})}});
                return;
            }
            if (request.method == "POST") {
                if (request.body.empty() || request.body.size() > 16384) { error(response, "invalid_request", 413); return; }
                if (route != "authorize" && (request.get_header_value_count("Origin") > 1 ||
                    (!request.get_header_value("Origin").empty() && request.get_header_value("Origin") != issuer))) {
                    error(response, "invalid_request", 403); return;
                }
            }
            prune_all();
            if (route == "register") {
                if (!rate(register_window_, registrations_, 20) || clients_.size() >= 1024) {
                    response.set_header("Retry-After", "60"); error(response, "temporarily_unavailable", 429); return;
                }
                const auto media = lower(trim(request.get_header_value("Content-Type")));
                if (request.get_header_value_count("Content-Type") != 1 ||
                    (media != "application/json" && media.compare(0, 17, "application/json;") != 0)) {
                    error(response, "invalid_client_metadata"); return;
                }
                const auto data = Json::parse(request.body);
                if (!data.is_object() || !data.contains("redirect_uris") || !data["redirect_uris"].is_array() ||
                    data["redirect_uris"].empty() || data["redirect_uris"].size() > 8 ||
                    data.value("token_endpoint_auth_method", "none") != "none") {
                    error(response, "invalid_client_metadata"); return;
                }
                Client client{data.value("client_name", "ChatGPT"), {}};
                if (client.name.empty() || client.name.size() > 80) { error(response, "invalid_client_metadata"); return; }
                for (const auto& uri : data["redirect_uris"]) {
                    if (!uri.is_string() || !chatgpt_redirect(uri.get<std::string>())) { error(response, "invalid_redirect_uri"); return; }
                    client.redirects.insert(uri.get<std::string>());
                }
                if (data.contains("grant_types"))
                    for (const auto& grant : data.at("grant_types"))
                        if (grant != "authorization_code" && grant != "refresh_token") { error(response, "invalid_client_metadata"); return; }
                if (data.contains("response_types") && data["response_types"] != Json::array({"code"})) {
                    error(response, "invalid_client_metadata"); return;
                }
                const auto id = oauth_random_token();
                client.expires = clock_() + 1800;
                clients_[id] = client;
                json(response, 201, {{"client_id", id}, {"client_id_issued_at", clock_()},
                    {"redirect_uris", client.redirects}, {"client_name", client.name}, {"token_endpoint_auth_method", "none"},
                    {"grant_types", Json::array({"authorization_code", "refresh_token"})}, {"response_types", Json::array({"code"})}});
                return;
            }
            if (route == "authorize" && request.method == "GET") {
                if (!rate(authorize_window_, authorizations_, 60) || pending_.size() >= 256) {
                    error(response, "temporarily_unavailable", 429); return;
                }
                Grant grant{param(request, "client_id", true), param(request, "redirect_uri", true),
                    param(request, "code_challenge", true), param(request, "state"),
                    scopes(param(request, "scope")), param(request, "resource", true), clock_() + 300};
                const auto client = clients_.find(grant.client);
                if (client == clients_.end() || !client->second.redirects.count(grant.redirect) ||
                    param(request, "response_type") != "code" || param(request, "code_challenge_method") != "S256" ||
                    grant.resource != resource || grant.challenge.size() != 43 ||
                    !std::all_of(grant.challenge.begin(), grant.challenge.end(), [](unsigned char c) {
                        return ascii_alnum(c) || c == '-' || c == '_'; })) {
                    error(response, "invalid_request"); return;
                }
                const auto transaction = oauth_random_token(), csrf = oauth_random_token();
                grant.csrf_hash = oauth_digest(csrf);
                pending_[oauth_digest(transaction)] = grant;
                response.set_header("Set-Cookie", "__Secure-gptframes-csrf=" + csrf +
                    "; Path=/oauth/authorize; Secure; HttpOnly; SameSite=Lax; Max-Age=300");
                render_pin(response, transaction, grant, csrf);
                return;
            }
            if (request.get_header_value("Content-Type").find("application/x-www-form-urlencoded") != 0) {
                error(response, "invalid_request"); return;
            }
            if (route == "authorize") {
                const auto transaction = param(request, "transaction");
                const auto csrf = param(request, "csrf"), cookie = csrf_cookie(request);
                if (transaction.empty() || csrf.size() != 43 || cookie.size() != 43 ||
                    !constant_time_equal(csrf, cookie)) {
                    authorization_error(response, "La sesión de seguridad no es válida. Permite las cookies de esta ventana."); return;
                }
                const auto found = pending_.find(oauth_digest(transaction));
                if (found == pending_.end()) {
                    authorization_error(response, "La ventana de conexión caducó o ya fue utilizada."); return;
                }
                const auto grant = found->second;
                if (!constant_time_equal(oauth_digest(csrf), grant.csrf_hash)) {
                    authorization_error(response, "La sesión no corresponde a esta ventana."); return;
                }
                const auto action = param(request, "decision", true);
                if (action == "deny") { pending_.erase(found); redirect(response, grant, "error", "access_denied"); return; }
                if (action != "approve") { error(response, "invalid_request"); return; }
                if (!rate(pin_window_, pin_attempts_, 8)) {
                    response.set_header("Retry-After", "60"); error(response, "temporarily_unavailable", 429); return;
                }
                const auto pin = param(request, "pin", true);
                if (pin.size() > 128 || !constant_time_equal(oauth_sha256(salt_ + pin), pin_hash_)) {
                    response.status = 403; render_pin(response, transaction, grant, csrf, "PIN incorrecto. Intenta nuevamente."); return;
                }
                if (!clients_.count(grant.client)) { error(response, "invalid_request", 403); return; }
                auto& registered = clients_.at(grant.client);
                if (registered.expires) {
                    const auto approved = std::count_if(clients_.begin(), clients_.end(), [](const auto& entry) { return !entry.second.expires; });
                    if (approved >= 128) { error(response, "temporarily_unavailable", 503); return; }
                    const auto previous = registered.expires;
                    registered.expires = 0;
                    try { save_clients(); } catch (...) { registered.expires = previous; throw; }
                }
                if (codes_.size() >= 256) { error(response, "temporarily_unavailable", 503); return; }
                const auto code = oauth_random_token();
                pending_.erase(found);
                auto authorized = grant; authorized.expires = clock_() + 90;
                codes_[oauth_digest(code)] = authorized;
                redirect(response, grant, "code", code);
                return;
            }
            if (route == "token") {
                if (request.get_header_value_count("Authorization") || !param(request, "client_secret").empty()) {
                    error(response, "invalid_client"); return;
                }
                const auto client = param(request, "client_id", true);
                if (!clients_.count(client)) { error(response, "invalid_client"); return; }
                const auto grant_type = param(request, "grant_type", true);
                if (grant_type == "authorization_code") {
                    const auto found = codes_.find(oauth_digest(param(request, "code", true)));
                    if (found == codes_.end()) { error(response, "invalid_grant"); return; }
                    const auto grant = found->second;
                    const auto verifier = param(request, "code_verifier", true);
                    if (grant.client != client || grant.redirect != param(request, "redirect_uri", true) ||
                        grant.resource != param(request, "resource", true) || verifier.size() < 43 || verifier.size() > 128 ||
                        !std::all_of(verifier.begin(), verifier.end(), [](unsigned char c) {
                            return ascii_alnum(c) || c == '-' || c == '_' || c == '.' || c == '~'; }) ||
                        !constant_time_equal(oauth_digest(verifier), grant.challenge)) {
                        error(response, "invalid_grant"); return;
                    }
                    codes_.erase(found);
                    json(response, 200, issue(grant)); return;
                }
                if (grant_type == "refresh_token") {
                    const auto found = refresh_.find(oauth_digest(param(request, "refresh_token", true)));
                    if (found == refresh_.end() || found->second.client != client) { error(response, "invalid_grant"); return; }
                    const auto old = found->second;
                    if (old.used) { revoke_family(old.family); error(response, "invalid_grant"); return; }
                    const auto requested = param(request, "resource");
                    if ((!requested.empty() && old.resource != requested) || old.resource != resource ||
                        (!param(request, "scope").empty() && scopes(param(request, "scope")) != old.scope)) {
                        error(response, "invalid_grant"); return;
                    }
                    found->second.used = true;
                    json(response, 200, issue({old.client, {}, {}, {}, old.scope, old.resource, 0}, old.family, old.expires));
                    return;
                }
                error(response, "unsupported_grant_type"); return;
            }
            error(response, "invalid_request");
        } catch (const Json::exception&) { error(response, "invalid_request"); }
        catch (const std::invalid_argument&) { error(response, "invalid_request"); }
        catch (const std::exception&) { error(response, "temporarily_unavailable", 503); }
    }
};
} // namespace gptframes::mcp
