#pragma once
#include "common.hpp"
#include <cctype>
#include <functional>
#include <map>
#include <set>
#ifdef _WIN32
#include "process.hpp"
#include <winhttp.h>
#else
#include <curl/curl.h>
#endif

namespace gptframes::mcp {
struct HttpResponse {
    long status = 0;
    std::string body, location, content_type;
};
inline std::string trim(std::string value) {
    const auto whitespace = [](unsigned char c) { return c == ' ' || c == '\r' || c == '\n' || c == '\t'; };
    while (!value.empty() && whitespace(static_cast<unsigned char>(value.back()))) value.pop_back();
    std::size_t first = 0;
    while (first < value.size() && whitespace(static_cast<unsigned char>(value[first]))) ++first;
    return value.substr(first);
}
inline std::string lower(std::string s) {
    for (auto& c : s) if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    return s;
}
struct HttpsUrl { std::string host, resource; };
inline HttpsUrl permitted_url(const std::string& url, const std::set<std::string>& hosts) {
    if (url.compare(0, 8, "https://") || url.find_first_of("\r\n\t\\ ") != std::string::npos)
        throw std::invalid_argument("Se requiere una URL HTTPS permitida");
    const auto end = url.find_first_of("/?#", 8);
    auto authority = lower(url.substr(8, end == std::string::npos ? std::string::npos : end - 8));
    if (authority.size() > 4 && authority.compare(authority.size() - 4, 4, ":443") == 0)
        authority.resize(authority.size() - 4);
    if (!hosts.count(authority)) throw std::invalid_argument("Servidor de medios no autorizado");
    auto resource = end == std::string::npos ? "/" : url.substr(end);
    const auto fragment = resource.find('#');
    if (fragment != std::string::npos) resource.resize(fragment);
    if (resource.empty() || resource[0] != '/') resource = "/" + resource;
    return {authority, resource};
}
inline std::string encode_query(const std::string& input) {
    constexpr char digits[] = "0123456789ABCDEF";
    std::string output;
    for (unsigned char c : input) {
        if (ascii_alnum(c) || c == '-' || c == '_' || c == '.' || c == '~') output += static_cast<char>(c);
        else { output += '%'; output += digits[c >> 4]; output += digits[c & 15]; }
    }
    return output;
}
inline std::string query_url(const std::string& base, const std::map<std::string, std::string>& parameters) {
    std::string result = base + "?";
    bool first = true;
    for (const auto& [key, value] : parameters) {
        if (!first) result += '&';
        first = false; result += encode_query(key) + "=" + encode_query(value);
    }
    return result;
}
using HttpFetch = std::function<HttpResponse(const std::string&, std::size_t)>;
inline HttpResponse https_once(const std::string& url, std::size_t maximum) {
    HttpResponse response;
#ifdef _WIN32
    const auto parsed = permitted_url(url, {"commons.wikimedia.org", "api.openverse.org",
                                           "upload.wikimedia.org", "thumb.wikimedia.org"});
    struct InternetHandle {
        HINTERNET value = nullptr;
        ~InternetHandle() { if (value) WinHttpCloseHandle(value); }
    };
    InternetHandle session{WinHttpOpen(L"GPTFrames/0.2 (https://github.com/ThowiLabs/GPTFrames)",
                                      WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME,
                                      WINHTTP_NO_PROXY_BYPASS, 0)};
    if (!session.value) throw std::runtime_error("No se pudo iniciar HTTPS");
    WinHttpSetTimeouts(session.value, 10000, 10000, 25000, 25000);
    InternetHandle connection{WinHttpConnect(session.value, utf16(parsed.host).c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0)};
    if (!connection.value) throw std::runtime_error("No se pudo conectar a proveedor");
    InternetHandle request{WinHttpOpenRequest(connection.value, L"GET", utf16(parsed.resource).c_str(),
                                             nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
                                             WINHTTP_FLAG_SECURE)};
    if (!request.value) throw std::runtime_error("No se pudo crear consulta HTTPS");
    DWORD disabled = WINHTTP_DISABLE_REDIRECTS | WINHTTP_DISABLE_COOKIES;
    if (!WinHttpSetOption(request.value, WINHTTP_OPTION_DISABLE_FEATURE, &disabled, sizeof(disabled)))
        throw std::runtime_error("No se pudo desactivar redirecciones automaticas");
    if (!WinHttpSendRequest(request.value, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) ||
        !WinHttpReceiveResponse(request.value, nullptr))
        throw std::runtime_error("Fallo de consulta HTTPS");
    DWORD status = 0, status_bytes = sizeof(status);
    if (!WinHttpQueryHeaders(request.value, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                             WINHTTP_HEADER_NAME_BY_INDEX, &status, &status_bytes, WINHTTP_NO_HEADER_INDEX))
        throw std::runtime_error("Respuesta HTTP invalida");
    response.status = status;
    auto header = [&](DWORD key) {
        DWORD bytes = 0;
        WinHttpQueryHeaders(request.value, key, WINHTTP_HEADER_NAME_BY_INDEX, nullptr, &bytes, WINHTTP_NO_HEADER_INDEX);
        if (!bytes || bytes > 65536) return std::string{};
        std::wstring text(bytes / sizeof(wchar_t), L'\0');
        if (!WinHttpQueryHeaders(request.value, key, WINHTTP_HEADER_NAME_BY_INDEX, text.data(), &bytes, WINHTTP_NO_HEADER_INDEX))
            return std::string{};
        while (!text.empty() && !text.back()) text.pop_back();
        const int length = WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);
        std::string result(static_cast<std::size_t>(length), '\0');
        WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), result.data(), length, nullptr, nullptr);
        return result;
    };
    response.location = header(WINHTTP_QUERY_LOCATION);
    response.content_type = header(WINHTTP_QUERY_CONTENT_TYPE);
    while (true) {
        char block[8192]; DWORD received = 0;
        if (!WinHttpReadData(request.value, block, sizeof(block), &received)) throw std::runtime_error("Respuesta HTTPS interrumpida");
        if (!received) break;
        if (received > maximum - response.body.size()) throw std::runtime_error("Respuesta externa demasiado grande");
        response.body.append(block, received);
    }
#else
    static const int initialized = [] { return curl_global_init(CURL_GLOBAL_DEFAULT); }();
    if (initialized != CURLE_OK) throw std::runtime_error("No se pudo iniciar libcurl");
    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> handle(curl_easy_init(), &curl_easy_cleanup);
    if (!handle) throw std::runtime_error("No se pudo crear consulta HTTPS");
    struct Sink { HttpResponse* response; std::size_t maximum; bool overflow = false; } sink{&response, maximum};
    auto write = +[](char* data, std::size_t size, std::size_t count, void* context) -> std::size_t {
        auto& s = *static_cast<Sink*>(context);
        if (size && count > static_cast<std::size_t>(-1) / size) return 0;
        const auto bytes = size * count;
        if (bytes > s.maximum - s.response->body.size()) { s.overflow = true; return 0; }
        try { s.response->body.append(data, bytes); return bytes; } catch (...) { return 0; }
    };
    auto header = +[](char* data, std::size_t size, std::size_t count, void* context) -> std::size_t {
        const auto bytes = size * count;
        if (bytes > 65536) return 0;
        try {
            auto& r = *static_cast<HttpResponse*>(context);
            const std::string line(data, bytes);
            if (line.compare(0, 5, "HTTP/") == 0) { r.location.clear(); r.content_type.clear(); }
            const auto colon = line.find(':');
            if (colon != std::string::npos) {
                const auto key = lower(line.substr(0, colon));
                if (key == "location") r.location = trim(line.substr(colon + 1));
                if (key == "content-type") r.content_type = trim(line.substr(colon + 1));
            }
            return bytes;
        } catch (...) { return 0; }
    };
    auto* h = handle.get();
    curl_easy_setopt(h, CURLOPT_URL, url.c_str());
#if LIBCURL_VERSION_NUM >= 0x075500
    curl_easy_setopt(h, CURLOPT_PROTOCOLS_STR, "https");
#else
    curl_easy_setopt(h, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
#endif
    curl_easy_setopt(h, CURLOPT_FOLLOWLOCATION, 0L);
    curl_easy_setopt(h, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(h, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(h, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(h, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(h, CURLOPT_TIMEOUT, 25L);
    curl_easy_setopt(h, CURLOPT_LOW_SPEED_LIMIT, 1L);
    curl_easy_setopt(h, CURLOPT_LOW_SPEED_TIME, 15L);
    curl_easy_setopt(h, CURLOPT_USERAGENT, "GPTFrames/0.2 (https://github.com/ThowiLabs/GPTFrames; media research)");
    curl_easy_setopt(h, CURLOPT_WRITEFUNCTION, write);
    curl_easy_setopt(h, CURLOPT_WRITEDATA, &sink);
    curl_easy_setopt(h, CURLOPT_HEADERFUNCTION, header);
    curl_easy_setopt(h, CURLOPT_HEADERDATA, &response);
    const auto code = curl_easy_perform(h);
    if (sink.overflow) throw std::runtime_error("Respuesta externa demasiado grande");
    if (code != CURLE_OK) throw std::runtime_error("Fallo HTTPS: " + std::string(curl_easy_strerror(code)));
    curl_easy_getinfo(h, CURLINFO_RESPONSE_CODE, &response.status);
#endif
    return response;
}
inline HttpResponse fetch_https(std::string url, const std::set<std::string>& hosts,
                                std::size_t maximum, const HttpFetch& fetch = https_once) {
    for (int redirect = 0; redirect <= 5; ++redirect) {
        const auto parsed = permitted_url(url, hosts);
        auto response = fetch(url, maximum);
        if (response.body.size() > maximum) throw std::runtime_error("Respuesta externa demasiado grande");
        if (response.status >= 300 && response.status < 400) {
            if (response.location.empty() || redirect == 5) throw std::runtime_error("Redireccion no valida o excesiva");
            if (response.location.compare(0, 2, "//") == 0) url = "https:" + response.location;
            else if (response.location[0] == '/') url = "https://" + parsed.host + response.location;
            else if (response.location.find("://") != std::string::npos) url = response.location;
            else url = "https://" + parsed.host + parsed.resource.substr(0, parsed.resource.find_last_of('/') + 1) + response.location;
            continue;
        }
        if (response.status < 200 || response.status >= 300)
            throw std::runtime_error("Proveedor devolvio HTTP " + std::to_string(response.status));
        return response;
    }
    throw std::runtime_error("Demasiadas redirecciones");
}
} // namespace gptframes::mcp
