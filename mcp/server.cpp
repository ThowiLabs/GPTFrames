#include "http_server.hpp"
#include <iostream>

#ifndef GPTFRAMES_DEFAULT_ROOT
#define GPTFRAMES_DEFAULT_ROOT "."
#endif

namespace {
gptframes::mcp::fs::path executable_path(const char* argument) {
    namespace fs = gptframes::mcp::fs;
#ifdef _WIN32
    std::wstring path(32768, L'\0');
    const auto size = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
    if (size && size < path.size()) { path.resize(size); return fs::path(path); }
#else
    std::error_code error;
    auto path = fs::read_symlink("/proc/self/exe", error);
    if (!error) return path;
#endif
    return fs::absolute(fs::u8path(argument));
}
}
int main(int argc, char** argv) {
    using namespace gptframes::mcp;
    try {
        std::string transport = "http";
        int port = 8769;
        fs::path root = fs::u8path(GPTFRAMES_DEFAULT_ROOT);
        const auto executable = executable_path(argv[0]);
        const auto packaged = executable.parent_path().parent_path();
        if (fs::is_regular_file(packaged / "examples" / "json-showcase-v2" / "showcase-v2.json")) root = packaged;
        for (int i = 1; i < argc; ++i) {
            const std::string option = argv[i];
            if (option == "--help" || option == "-h") {
                std::cout << "GPTFrames MCP C++ 0.2.0\n"
                          << "Usage: gptframes-mcp [--transport stdio|http] [--port 8769] [--root PROJECT_ROOT]\n"
                          << "Environment: GPTFRAMES_ROOT, GPTFRAMES_PROJECTS, GPTFRAMES_MODELS,\n"
                          << "GPTFRAMES_ENGINE, GPTFRAMES_NEO_LITE, GPTFRAMES_OAUTH_PIN_FILE,\n"
                          << "GPTFRAMES_PUBLIC_URL, GPTFRAMES_PUBLIC_URL_FILE, GPTFRAMES_OAUTH_STATE\n";
                return 0;
            }
            const auto equal = option.find('=');
            const auto key = option.substr(0, equal);
            std::string value;
            if (equal != std::string::npos) value = option.substr(equal + 1);
            else if (i + 1 < argc) value = argv[++i];
            else throw std::invalid_argument("Falta valor de opcion");
            if (key == "--transport") transport = value;
            else if (key == "--root") root = fs::u8path(value);
            else if (key == "--port") {
                const auto parsed = std::from_chars(value.data(), value.data() + value.size(), port);
                if (parsed.ec != std::errc{} || parsed.ptr != value.data() + value.size() || port < 1024 || port > 65535)
                    throw std::invalid_argument("Puerto fuera de rango");
            } else throw std::invalid_argument("Opcion no reconocida: " + key);
        }
        auto configuration = Config::from_environment(executable, root);
        Service service(std::move(configuration));
        if (transport == "stdio") { serve_stdio(service); return 0; }
        if (transport != "http") throw std::invalid_argument("Transporte admitido: stdio o http");
        httplib::Server server;
        auto oauth = std::make_shared<OAuth>(OAuthConfig::from_environment(service.config().root));
        configure_http(server, service, oauth);
        if (!server.listen("127.0.0.1", port)) throw std::runtime_error("No se pudo iniciar HTTP en loopback");
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "GPTFrames MCP: " << error.what() << '\n';
        return 1;
    }
}
