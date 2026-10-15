#include "../mcp/http_server.hpp"
#include <thread>
using namespace gptframes::mcp;
namespace {
std::string form(const std::map<std::string, std::string>& fields) {
    std::string result;
    for (const auto& [key, value] : fields) {
        if (!result.empty()) result += '&';
        result += encode_query(key) + "=" + encode_query(value);
    }
    return result;
}
std::string between(const std::string& value, const std::string& first, const std::string& last) {
    const auto start = value.find(first);
    if (start == std::string::npos) throw std::runtime_error("Missing response field");
    const auto begin = start + first.size(), end = value.find(last, begin);
    return value.substr(begin, end == std::string::npos ? end : end - begin);
}
}
int main(int argc, char** argv) {
    if (argc != 3) return 2;
    TemporaryDirectory temp(fs::temp_directory_path(), "gptframes-oauth-tests-");
    const auto root = fs::canonical(fs::u8path(argv[2]));
    Config config{root, temp.path / "projects", temp.path / "models", temp.path / "missing-engine", "neo-lite"};
    Service service(config);
    std::atomic<std::int64_t> now{1000000};
    const std::string issuer = "https://gptframes.example.test", resource = issuer + "/mcp";
    const std::string pin = "952713604285";
    OAuthConfig oauth_config{pin, issuer, {}, temp.path / "clients.json"};
    auto oauth = std::make_shared<OAuth>(oauth_config, [&] { return now.load(); });
    httplib::Server server;
    configure_http(server, service, oauth);
    const int port = server.bind_to_any_port("127.0.0.1");
    if (port < 0) return 1;
    std::thread thread([&] { server.listen_after_bind(); });
    struct Stop {
        httplib::Server& server; std::thread& thread;
        ~Stop() { server.stop(); if (thread.joinable()) thread.join(); }
    } stop{server, thread};
    server.wait_until_ready();
    httplib::Client client("127.0.0.1", port);
    client.set_read_timeout(5);
    const std::string callback = "https://chatgpt.com/connector_platform_oauth_redirect";
    const std::string verifier = "dBjftJeZ4CVP-mB92K27uhbUJU1p1r_wW1gFWFOEjXk";
    const std::string challenge = "E9Melhoa2OwvFrEMTJguCHaoeK1t8URWbuGJSstw-cM";
    const auto ping = Json{{"jsonrpc", "2.0"}, {"id", 1}, {"method", "ping"}}.dump();
    const std::string content_type = "application/x-www-form-urlencoded";
    int count = 0;
    auto check = [&](bool condition, const char* name) {
        if (!condition) throw std::runtime_error(name);
        ++count; std::cout << "PASS " << name << '\n';
    };
    try {
        check(oauth_digest(verifier) == challenge, "RFC7636 SHA256/base64url vector");
        check(oauth_base64url("f") == "Zg" && oauth_base64url("fo") == "Zm8" &&
              oauth_base64url("foo") == "Zm9v", "base64url remainder vectors");
        auto health = client.Get("/health");
        check(health && health->status == 200, "health");
        auto metadata = client.Get("/.well-known/oauth-authorization-server");
        auto doc = Json::parse(metadata->body);
        check(metadata->status == 200 && doc["issuer"] == issuer &&
              doc["code_challenge_methods_supported"] == Json::array({"S256"}) &&
              doc["authorization_response_iss_parameter_supported"] == true, "OAuth issuer PKCE and discovery");
        auto protected_resource = client.Get("/.well-known/oauth-protected-resource/mcp");
        check(protected_resource && Json::parse(protected_resource->body)["resource"] == resource, "resource audience discovery");
        auto unauthenticated = client.Post("/mcp", ping, "application/json");
        check(unauthenticated && unauthenticated->status == 401 &&
              unauthenticated->get_header_value("WWW-Authenticate").find(issuer) != std::string::npos, "OAuth challenge without access");
        auto manual = client.Post("/mcp", {{"Authorization", "Bearer manual-static-token"}}, ping, "application/json");
        check(manual && manual->status == 401, "manual static token rejected");
        auto missing = client.Get("/mcp");
        check(missing && missing->status == 401, "GET discovery requires connection");
        auto invalid_registration = client.Post("/oauth/register",
            Json{{"redirect_uris", Json::array({"https://evil.example.org/callback"})}}.dump(), "application/json");
        check(invalid_registration && invalid_registration->status == 400, "external callback rejected");
        auto registered = client.Post("/oauth/register", Json{{"client_name", "ChatGPT"},
            {"redirect_uris", Json::array({callback})}, {"token_endpoint_auth_method", "none"}}.dump(), "application/json");
        check(registered && registered->status == 201, "ChatGPT dynamic registration");
        check(!fs::exists(oauth_config.clients_file), "unconfirmed registrations are not persisted");
        auto registration = Json::parse(registered->body);
        const auto client_id = registration["client_id"].get<std::string>();
        check(!registration.contains("client_secret") && registration["token_endpoint_auth_method"] == "none", "public client without manual secret");
        std::map<std::string, std::string> auth{
            {"client_id", client_id}, {"redirect_uri", callback}, {"response_type", "code"},
            {"code_challenge", challenge}, {"code_challenge_method", "S256"},
            {"state", "state+with&values"}, {"scope", "gptframes offline_access"}, {"resource", resource}};
        auto bad_auth = auth; bad_auth["redirect_uri"] += "/other";
        auto bad = client.Get("/oauth/authorize?" + form(bad_auth));
        check(bad && bad->status == 400 && !bad->has_header("Location"), "exact registered redirect required");
        bad_auth = auth; bad_auth.erase("code_challenge");
        bad = client.Get("/oauth/authorize?" + form(bad_auth));
        check(bad && bad->status == 400, "PKCE required");
        bad_auth = auth; bad_auth["resource"] = "https://other.example/mcp";
        bad = client.Get("/oauth/authorize?" + form(bad_auth));
        check(bad && bad->status == 400, "authorization resource bound");
        std::map<std::string, std::string> session_csrf;
        auto post_authorization = [&](std::map<std::string, std::string> fields, httplib::Headers headers = httplib::Headers{}) {
            const auto saved = session_csrf.find(fields["transaction"]);
            if (saved != session_csrf.end()) {
                fields["csrf"] = saved->second;
                headers.emplace("Cookie", "__Secure-gptframes-csrf=" + saved->second);
            }
            return client.Post("/oauth/authorize", headers, form(fields), content_type);
        };
        auto callback_target = [](const httplib::Result& result) {
            auto value = between(result->body, "href=\"", "\"");
            std::size_t pos = 0;
            while ((pos = value.find("&amp;", pos)) != std::string::npos) value.replace(pos, 5, "&");
            return value;
        };
        auto get_transaction = [&] {
            auto page = client.Get("/oauth/authorize?" + form(auth));
            if (!page || page->status != 200) throw std::runtime_error("PIN page unavailable");
            const auto tx = between(page->body, "name=\"transaction\" value=\"", "\"");
            const auto csrf = between(page->body, "name=\"csrf\" value=\"", "\"");
            session_csrf[tx] = csrf;
            if (page->get_header_value("Set-Cookie").find("__Secure-gptframes-csrf=" + csrf) == std::string::npos ||
                page->get_header_value("Set-Cookie").find("Secure; HttpOnly; SameSite=Lax") == std::string::npos)
                throw std::runtime_error("private browser CSRF cookie missing");
            return tx;
        };
        auto transaction = get_transaction();
        auto page = client.Get("/oauth/authorize?" + form(auth));
        check(page && page->body.find("PIN de conexión") != std::string::npos &&
              page->body.find(pin) == std::string::npos && page->has_header("Content-Security-Policy"), "PIN window without credential disclosure");
        auto wrong_origin = client.Post("/oauth/authorize", {{"Origin", "https://evil.example"}},
            form({{"transaction", transaction}, {"pin", pin}, {"decision", "approve"}}), content_type);
        check(wrong_origin && wrong_origin->status == 403, "cross origin without browser session rejected");
        auto wrong_transaction = post_authorization({{"transaction", "forged"}, {"pin", pin}, {"decision", "approve"}});
        check(wrong_transaction && wrong_transaction->status == 403, "PIN transaction CSRF protection");
        auto wrong_pin = post_authorization({{"transaction", transaction}, {"pin", "wrong-pin"}, {"decision", "approve"}});
        check(wrong_pin && wrong_pin->status == 403 && !wrong_pin->has_header("Location"), "wrong PIN cannot authorize");
        auto canceled = post_authorization({{"transaction", transaction}, {"decision", "deny"}});
        check(canceled && canceled->status == 200 && callback_target(canceled).find("error=access_denied") != std::string::npos &&
              callback_target(canceled).find("iss=" + encode_query(issuer)) != std::string::npos, "cancel handoff includes issuer");
        for (const auto& origin : {"null", "https://gradio.live", "https://proxy.example"}) {
            const auto tx = get_transaction();
            auto browser = post_authorization({{"transaction", tx}, {"pin", pin}, {"decision", "approve"}}, {{"Origin", origin}});
            check(browser && browser->status == 200 &&
                  callback_target(browser).find("?code=") != std::string::npos &&
                  browser->body.find("http-equiv=\"refresh\"") != std::string::npos &&
                  browser->get_header_value("Content-Security-Policy").find("form-action") == std::string::npos,
                  "browser/proxy Origin accepted with bound CSRF cookie and PIN");
        }
        const auto secure_tx = get_transaction();
        const auto csrf = session_csrf.at(secure_tx);
        const auto csrf_body = form({{"transaction", secure_tx}, {"csrf", csrf}, {"pin", pin}, {"decision", "approve"}});
        auto no_cookie = client.Post("/oauth/authorize", {{"Origin", "null"}}, csrf_body, content_type);
        check(no_cookie && no_cookie->status == 403, "null Origin cannot bypass browser cookie");
        auto forged_cookie = client.Post("/oauth/authorize", {{"Cookie", "__Secure-gptframes-csrf=" + std::string(43, 'x')}},
            csrf_body, content_type);
        check(forged_cookie && forged_cookie->status == 403, "CSRF form and cookie must match");
        auto alien_session = client.Post("/oauth/authorize", {{"Cookie", "__Secure-gptframes-csrf=" + csrf}},
            form({{"transaction", get_transaction()}, {"csrf", csrf}, {"pin", pin}, {"decision", "approve"}}), content_type);
        check(alien_session && alien_session->status == 403, "CSRF token bound to authorization session");
        auto duplicate_cookie = client.Post("/oauth/authorize",
            {{"Cookie", "__Secure-gptframes-csrf=" + csrf + "; __Secure-gptframes-csrf=" + csrf}}, csrf_body, content_type);
        check(duplicate_cookie && duplicate_cookie->status == 403, "duplicate CSRF cookies rejected");
        transaction = get_transaction();
        auto approve = [&](const std::string& tx) {
            return post_authorization({{"transaction", tx}, {"pin", pin}, {"decision", "approve"}}, {{"Origin", issuer}});
        };
        auto approved = approve(transaction);
        const auto location = callback_target(approved);
        check(approved && approved->status == 200 && location.find("state=state%2Bwith%26values") != std::string::npos &&
              location.find("iss=" + encode_query(issuer)) != std::string::npos, "PIN confirmation and state issuer callback");
        const auto code = between(location, "?code=", "&");
        auto reused_transaction = approve(transaction);
        check(reused_transaction && reused_transaction->status == 403, "authorization transaction single use");
        std::map<std::string, std::string> exchange{
            {"grant_type", "authorization_code"}, {"code", code}, {"client_id", client_id},
            {"redirect_uri", callback}, {"code_verifier", verifier}, {"resource", resource}};
        auto exchange_bad = exchange; exchange_bad["code_verifier"] = std::string(43, 'x');
        auto wrong_verifier = client.Post("/oauth/token", form(exchange_bad), content_type);
        check(wrong_verifier && wrong_verifier->status == 400, "intercepted code needs PKCE verifier");
        exchange_bad = exchange; exchange_bad["resource"] = "https://other.example/mcp";
        auto wrong_resource = client.Post("/oauth/token", form(exchange_bad), content_type);
        check(wrong_resource && wrong_resource->status == 400, "token exchange audience required");
        auto exchanged = client.Post("/oauth/token", form(exchange), content_type);
        check(exchanged && exchanged->status == 200, "PIN authorization exchange");
        const auto tokens = Json::parse(exchanged->body);
        auto reused = client.Post("/oauth/token", form(exchange), content_type);
        check(reused && reused->status == 400, "authorization code cannot be replayed");
        const httplib::Headers authorization{{"Authorization", "Bearer " + tokens["access_token"].get<std::string>()}};
        auto allowed = client.Post("/mcp", authorization, ping, "application/json");
        check(allowed && allowed->status == 200 && Json::parse(allowed->body)["id"] == 1, "authorized MCP connection");
        const auto initialize = Json{{"jsonrpc", "2.0"}, {"id", 2}, {"method", "initialize"}}.dump();
        auto initialized = client.Post("/mcp", authorization, initialize, "application/json");
        check(initialized && Json::parse(initialized->body)["result"]["serverInfo"]["name"] == "gptframes", "HTTP initialize");
        auto tools = client.Post("/mcp", authorization, R"({"jsonrpc":"2.0","id":3,"method":"tools/list"})", "application/json");
        const auto tool_list = Json::parse(tools->body)["result"]["tools"];
        check(tools && tools->status == 200 && tool_list.size() == 8 &&
              tool_list[0]["securitySchemes"][0]["type"] == "oauth2", "eight tools declare OAuth");
        Json modern_call{{"jsonrpc", "2.0"}, {"id", 4}, {"method", "server/discover"},
            {"params", {{"_meta", {{"io.modelcontextprotocol/protocolVersion", "2026-07-28"},
                {"io.modelcontextprotocol/clientCapabilities", Json::object()}}}}}};
        auto modern_headers = authorization;
        modern_headers.emplace("MCP-Protocol-Version", "2026-07-28");
        modern_headers.emplace("Mcp-Method", "server/discover");
        modern_headers.emplace("Origin", "https://www.chatgpt.com");
        auto discovery = client.Post("/mcp", modern_headers, modern_call.dump(), "application/json");
        check(discovery && discovery->status == 200 &&
              Json::parse(discovery->body)["result"]["resultType"] == "complete" &&
              Json::parse(discovery->body)["result"]["_meta"]["io.modelcontextprotocol/serverInfo"]["name"] == "gptframes",
              "ChatGPT modern action discovery after PIN");
        modern_call["method"] = "tools/list";
        auto mismatch = client.Post("/mcp", modern_headers, modern_call.dump(), "application/json");
        check(mismatch && mismatch->status == 400, "modern method header enforced");
        modern_headers.erase("Mcp-Method"); modern_headers.emplace("Mcp-Method", "tools/list");
        auto modern_tools = client.Post("/mcp", modern_headers, modern_call.dump(), "application/json");
        check(modern_tools && modern_tools->status == 200 &&
              Json::parse(modern_tools->body)["result"]["tools"].size() == 8 &&
              Json::parse(modern_tools->body)["result"]["resultType"] == "complete", "modern tool discovery");
        modern_call["method"] = "tools/call"; modern_call["params"]["name"] = "list_projects";
        modern_call["params"]["arguments"] = Json::object();
        modern_headers.erase("Mcp-Method"); modern_headers.emplace("Mcp-Method", "tools/call");
        auto missing_name = client.Post("/mcp", modern_headers, modern_call.dump(), "application/json");
        check(missing_name && missing_name->status == 400, "modern tool name header enforced");
        modern_headers.emplace("Mcp-Name", "list_projects");
        auto modern_result = client.Post("/mcp", modern_headers, modern_call.dump(), "application/json");
        check(modern_result && modern_result->status == 200 &&
              Json::parse(modern_result->body)["result"]["resultType"] == "complete", "modern native tool call");
        modern_call["params"].erase("_meta");
        auto missing_meta = client.Post("/mcp", modern_headers, modern_call.dump(), "application/json");
        check(missing_meta && missing_meta->status == 400, "modern protocol metadata required");
        auto unsupported_headers = authorization;
        unsupported_headers.emplace("MCP-Protocol-Version", "2099-01-01");
        auto unsupported = client.Post("/mcp", unsupported_headers, ping, "application/json");
        check(unsupported && unsupported->status == 400 &&
              Json::parse(unsupported->body)["error"]["code"] == -32022, "unsupported protocol version negotiation");
        auto public_headers = authorization; public_headers.emplace("Origin", issuer);
        auto public_origin = client.Post("/mcp", public_headers, ping, "application/json");
        check(public_origin && public_origin->status == 200, "canonical public origin accepted");
        auto legacy = client.Post("/mcp", authorization,
            R"({"jsonrpc":"2.0","id":5,"method":"initialize","params":{"protocolVersion":"2025-11-25"}})", "application/json");
        check(legacy && Json::parse(legacy->body)["result"]["protocolVersion"] == "2025-11-25", "legacy ChatGPT initialization");
        auto notification = client.Post("/mcp", authorization, R"({"jsonrpc":"2.0","method":"notifications/initialized"})", "application/json");
        check(notification && notification->status == 202 && notification->body.empty(), "HTTP notification 202");
        auto invalid = client.Post("/mcp", authorization, "{", "application/json");
        check(invalid && Json::parse(invalid->body)["error"]["code"] == -32700, "HTTP parse recovery");
        auto external = authorization; external.emplace("Origin", "https://evil.example.org");
        auto forbidden = client.Post("/mcp", external, ping, "application/json");
        check(forbidden && forbidden->status == 403, "external MCP Origin rejected");
        auto local = authorization; local.emplace("Origin", "https://chatgpt.com");
        auto native_chatgpt = client.Post("/mcp", local, ping, "application/json; charset=utf-8");
        check(native_chatgpt && native_chatgpt->status == 200, "ChatGPT Origin and JSON charset accepted");
        auto get = client.Get("/mcp", authorization);
        check(get && get->status == 405, "authenticated GET unsupported");
        auto wrong_type = client.Post("/mcp", authorization, ping, "application/jsonevil");
        check(wrong_type && wrong_type->status == 415, "incorrect JSON media type rejected");
        auto oversized = client.Post("/mcp", authorization, std::string(max_message_bytes + 1, 'x'), "application/json");
        check(oversized && oversized->status == 413, "oversized request rejected");
        auto duplicate_headers = authorization; duplicate_headers.emplace("Authorization", "Bearer other");
        auto duplicate = client.Post("/mcp", duplicate_headers, ping, "application/json");
        check(duplicate && duplicate->status == 401, "duplicate authorization rejected");
        OAuth restored(oauth_config, [&] { return now.load(); });
        httplib::Request restore_request; restore_request.method = "GET";
        for (const auto& pair : auth) restore_request.params.emplace(pair.first, pair.second);
        httplib::Response restore_response; restored.handle("authorize", restore_request, restore_response);
        check(restore_response.status == 200, "DCR client preserved across restart");
        std::map<std::string, std::string> refresh{
            {"grant_type", "refresh_token"}, {"client_id", client_id},
            {"refresh_token", tokens["refresh_token"].get<std::string>()}, {"resource", resource}};
        auto bad_refresh = refresh; bad_refresh["resource"] += "/other";
        auto audience = client.Post("/oauth/token", form(bad_refresh), content_type);
        check(audience && audience->status == 400, "refresh resource cannot change");
        auto refreshed = client.Post("/oauth/token", form(refresh), content_type);
        check(refreshed && refreshed->status == 200 &&
              Json::parse(refreshed->body)["refresh_token"] != tokens["refresh_token"], "refresh rotates credentials");
        const auto rotated = Json::parse(refreshed->body);
        const httplib::Headers rotated_auth{{"Authorization", "Bearer " + rotated["access_token"].get<std::string>()}};
        auto rotated_ok = client.Post("/mcp", rotated_auth, ping, "application/json");
        check(rotated_ok && rotated_ok->status == 200, "rotated access works");
        auto replay = client.Post("/oauth/token", form(refresh), content_type);
        auto revoked = client.Post("/mcp", rotated_auth, ping, "application/json");
        check(replay && replay->status == 400 && revoked && revoked->status == 401, "refresh replay revokes token family");
        auto fresh = approve(get_transaction());
        exchange["code"] = between(callback_target(fresh), "?code=", "&");
        auto fresh_tokens = client.Post("/oauth/token", form(exchange), content_type);
        const auto fresh_doc = Json::parse(fresh_tokens->body);
        const httplib::Headers expires_auth{{"Authorization", "Bearer " + fresh_doc["access_token"].get<std::string>()}};
        now += 3601;
        auto expired = client.Post("/mcp", expires_auth, ping, "application/json");
        check(expired && expired->status == 401, "expired access denied");
        transaction = get_transaction(); now += 301;
        auto expired_form = approve(transaction);
        check(expired_form && expired_form->status == 403, "expired PIN window denied");
        transaction = get_transaction();
        int last_status = 0;
        for (int i = 0; i < 9; ++i) {
            auto attempt = post_authorization({{"transaction", transaction}, {"pin", "wrong-pin"}, {"decision", "approve"}});
            last_status = attempt->status;
        }
        check(last_status == 429, "PIN guesses rate limited");
        const auto stdio = run_process({fs::absolute(fs::u8path(argv[1])).u8string(), "--transport", "stdio", "--root", root.u8string()},
            15, initialize + "\n" + R"({"jsonrpc":"2.0","method":"notifications/initialized"})" + "\n" + ping + "\n");
        check(stdio.exit_code == 0 && !stdio.timed_out, "internal stdio regression");
        std::istringstream lines(stdio.output); std::string line; std::vector<Json> responses;
        while (std::getline(lines, line)) responses.push_back(Json::parse(line));
        check(responses.size() == 2 && responses[1]["id"] == 1, "internal stdio JSON framing");
        bool rejected = false;
        try { OAuth missing(OAuthConfig{}); } catch (const std::exception&) { rejected = true; }
        check(rejected, "HTTP cannot start without PIN");
        std::cout << count << " PIN/OAuth/HTTP/stdio checks passed\n";
        return 0;
    } catch (const std::exception& error) { std::cerr << "FAIL " << error.what() << '\n'; return 1; }
}
