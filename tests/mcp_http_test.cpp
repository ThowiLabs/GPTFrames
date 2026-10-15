#include "../mcp/http_server.hpp"
#include <thread>
using namespace gptframes::mcp;
int main(int argc, char** argv) {
    if (argc != 3) { std::cerr << "Usage: gptframes-mcp-http-test MCP_BINARY SOURCE_ROOT\n"; return 2; }
    TemporaryDirectory temp(fs::temp_directory_path(), "gptframes-http-tests-");
    const auto root = fs::canonical(fs::u8path(argv[2]));
    Config configuration{root, temp.path / "projects", temp.path / "models", temp.path / "missing-engine", "neo-lite"};
    Service service(configuration);
    httplib::Server server;
    const std::string token = "test-token-with-at-least-24-characters";
    configure_http(server, service, token);
    const int port = server.bind_to_any_port("127.0.0.1");
    if (port < 0) { std::cerr << "Cannot bind test loopback port\n"; return 1; }
    std::thread thread([&] { server.listen_after_bind(); });
    struct Stop {
        httplib::Server& server;
        std::thread& thread;
        ~Stop() { server.stop(); if (thread.joinable()) thread.join(); }
    } stop{server, thread};
    server.wait_until_ready();
    httplib::Client client("127.0.0.1", port);
    client.set_read_timeout(5);
    const httplib::Headers authorization{{"Authorization", "Bearer " + token}};
    const auto ping = Json{{"jsonrpc", "2.0"}, {"id", 1}, {"method", "ping"}}.dump();
    int count = 0;
    auto check = [&](bool condition, const char* label) {
        if (!condition) throw std::runtime_error(label);
        ++count; std::cout << "PASS " << label << '\n';
    };
    try {
        const auto health = client.Get("/health");
        check(health && health->status == 200 && Json::parse(health->body)["status"] == "ok", "loopback health");
        const auto unauthorized = client.Post("/mcp", ping, "application/json");
        check(unauthorized && unauthorized->status == 401 && unauthorized->has_header("WWW-Authenticate"), "missing bearer rejected");
        const auto wrong = client.Post("/mcp", {{"Authorization", "Bearer wrong-token"}}, ping, "application/json");
        check(wrong && wrong->status == 401, "wrong bearer rejected");
        auto external = authorization; external.emplace("Origin", "https://evil.example.org");
        const auto forbidden = client.Post("/mcp", external, ping, "application/json");
        check(forbidden && forbidden->status == 403, "external origin rejected");
        auto local = authorization; local.emplace("Origin", "http://localhost:3000");
        const auto allowed = client.Post("/mcp", local, ping, "application/json");
        check(allowed && allowed->status == 200 && Json::parse(allowed->body)["id"] == 1, "allowed local origin");
        const auto initialize = Json{{"jsonrpc", "2.0"}, {"id", 2}, {"method", "initialize"}}.dump();
        const auto initialized = client.Post("/mcp", authorization, initialize, "application/json");
        check(initialized && initialized->status == 200 &&
              Json::parse(initialized->body)["result"]["serverInfo"]["name"] == "gptframes", "HTTP initialize");
        const auto notification = client.Post("/mcp", authorization,
            R"({"jsonrpc":"2.0","method":"notifications/initialized"})", "application/json");
        check(notification && notification->status == 202 && notification->body.empty(), "HTTP notification 202");
        const auto invalid = client.Post("/mcp", authorization, "{", "application/json");
        check(invalid && invalid->status == 200 && Json::parse(invalid->body)["error"]["code"] == -32700, "HTTP parse error");
        const auto tools = client.Post("/mcp", authorization,
            R"({"jsonrpc":"2.0","id":3,"method":"tools/list"})", "application/json");
        check(tools && tools->status == 200 && Json::parse(tools->body)["result"]["tools"].size() == 8, "HTTP tool list");
        const auto get = client.Get("/mcp");
        check(get && get->status == 405, "unsupported GET returns 405");
        const auto wrong_type = client.Post("/mcp", authorization, ping, "text/plain");
        check(wrong_type && wrong_type->status == 415, "non JSON content rejected");
        const auto oversized = client.Post("/mcp", authorization, std::string(max_message_bytes + 1, 'x'), "application/json");
        check(oversized && oversized->status == 413, "oversized request rejected");
        auto duplicates = authorization; duplicates.emplace("Authorization", "Bearer another");
        const auto duplicate = client.Post("/mcp", duplicates, ping, "application/json");
        check(duplicate && duplicate->status == 401, "duplicate authorization rejected");
        httplib::Server rejected;
        bool failed = false;
        try { configure_http(rejected, service, "short"); } catch (const std::exception&) { failed = true; }
        check(failed, "short configured bearer rejected");
        const auto stdio = run_process({fs::absolute(fs::u8path(argv[1])).u8string(), "--root", root.u8string()},
            15, initialize + "\n" + R"({"jsonrpc":"2.0","method":"notifications/initialized"})" + "\n" + ping + "\n");
        check(stdio.exit_code == 0 && !stdio.timed_out, "compiled MCP stdio starts");
        std::istringstream output(stdio.output); std::string line; std::vector<Json> responses;
        while (std::getline(output, line)) responses.push_back(Json::parse(line));
        check(responses.size() == 2 && responses[0]["result"]["serverInfo"]["name"] == "gptframes" &&
              responses[1]["id"] == 1, "compiled MCP clean JSON framing");
        std::cout << count << " HTTP/stdio integration checks passed\n";
        return 0;
    } catch (const std::exception& error) { std::cerr << "FAIL " << error.what() << '\n'; return 1; }
}
