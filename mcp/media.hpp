#pragma once
#include "http_client.hpp"
#include <charconv>
#include <cstring>

namespace gptframes::mcp {
inline std::string string_field(const Json& object, const std::string& key, const std::string& fallback = {}) {
    const auto it = object.is_object() ? object.find(key) : object.end();
    return it != object.end() && it->is_string() ? it->get<std::string>() : fallback;
}
inline void append_utf8(std::string& output, unsigned codepoint) {
    if (codepoint == 0 || codepoint > 0x10ffff || (codepoint >= 0xd800 && codepoint <= 0xdfff)) codepoint = 0xfffd;
    if (codepoint < 0x80) output += static_cast<char>(codepoint);
    else if (codepoint < 0x800) {
        output += static_cast<char>(0xc0 | (codepoint >> 6));
        output += static_cast<char>(0x80 | (codepoint & 63));
    } else if (codepoint < 0x10000) {
        output += static_cast<char>(0xe0 | (codepoint >> 12));
        output += static_cast<char>(0x80 | ((codepoint >> 6) & 63));
        output += static_cast<char>(0x80 | (codepoint & 63));
    } else {
        output += static_cast<char>(0xf0 | (codepoint >> 18));
        output += static_cast<char>(0x80 | ((codepoint >> 12) & 63));
        output += static_cast<char>(0x80 | ((codepoint >> 6) & 63));
        output += static_cast<char>(0x80 | (codepoint & 63));
    }
}
inline std::string plain_text(const std::string& input) {
    std::string stripped;
    bool tag = false;
    for (char c : input) {
        if (c == '<') tag = true;
        else if (c == '>') tag = false;
        else if (!tag) stripped += c;
    }
    static const std::map<std::string, std::string> entities{
        {"amp", "&"}, {"lt", "<"}, {"gt", ">"}, {"quot", "\""}, {"apos", "'"},
        {"nbsp", " "}, {"copy", "\xc2\xa9"}, {"ndash", "\xe2\x80\x93"}, {"mdash", "\xe2\x80\x94"}
    };
    std::string decoded;
    for (std::size_t i = 0; i < stripped.size(); ++i) {
        if (stripped[i] == '&') {
            const auto end = stripped.find(';', i + 1);
            if (end != std::string::npos && end - i < 16) {
                const auto key = stripped.substr(i + 1, end - i - 1);
                const auto found = entities.find(key);
                if (found != entities.end()) { decoded += found->second; i = end; continue; }
                if (key.size() > 1 && key[0] == '#') {
                    const bool hex = key.size() > 2 && (key[1] == 'x' || key[1] == 'X');
                    const auto* first = key.data() + (hex ? 2 : 1);
                    const auto* last = key.data() + key.size();
                    unsigned codepoint = 0;
                    const auto result = std::from_chars(first, last, codepoint, hex ? 16 : 10);
                    if (result.ec == std::errc{} && result.ptr == last) {
                        append_utf8(decoded, codepoint); i = end; continue;
                    }
                }
            }
        }
        decoded += stripped[i];
    }
    std::string output;
    bool space = false;
    std::size_t count = 0;
    for (unsigned char c : decoded) {
        if (c == ' ' || c == '\n' || c == '\t' || c == '\r') { space = !output.empty(); continue; }
        if (space && count < 500) { output += ' '; ++count; }
        space = false;
        if ((c & 0xc0) != 0x80 && ++count > 500) break;
        output += static_cast<char>(c);
    }
    return output;
}
inline std::string metadata(const Json& info, const std::string& key) {
    if (!info.is_object() || !info.contains("extmetadata") || !info["extmetadata"].is_object()) return {};
    const auto found = info["extmetadata"].find(key);
    return found == info["extmetadata"].end() ? std::string{} : plain_text(string_field(*found, "value"));
}
class MediaClient {
    HttpFetch fetch_;
    Json get_json(const std::string& url) const {
        return Json::parse(fetch_https(url, {"commons.wikimedia.org", "api.openverse.org"}, 3000000, fetch_).body);
    }
public:
    explicit MediaClient(HttpFetch fetch = https_once) : fetch_(std::move(fetch)) {}
    Json search(const std::string& query, const std::string& provider = "wikimedia", int limit = 6) const {
        if (utf8_length(trim(query)) < 2 || utf8_length(trim(query)) > 120)
            throw std::invalid_argument("Consulta entre 2 y 120 caracteres");
        if (limit < 1 || limit > 15) throw std::invalid_argument("Limite 1-15");
        Json results = Json::array();
        if (provider == "wikimedia") {
            const auto data = get_json(query_url("https://commons.wikimedia.org/w/api.php", {
                {"action", "query"}, {"generator", "search"}, {"gsrsearch", query}, {"gsrnamespace", "6"},
                {"gsrlimit", std::to_string(limit)}, {"prop", "imageinfo"}, {"iiprop", "url|extmetadata"},
                {"iiurlwidth", "700"}, {"format", "json"}, {"formatversion", "2"}
            }));
            if (!data.contains("query") || !data["query"].contains("pages")) return results;
            for (const auto& page : data["query"]["pages"]) {
                if (!page.contains("imageinfo") || !page["imageinfo"].is_array() || page["imageinfo"].empty()) continue;
                const auto& info = page["imageinfo"][0];
                auto url = string_field(info, "thumburl", string_field(info, "url"));
                try { permitted_url(url, {"upload.wikimedia.org", "thumb.wikimedia.org"}); }
                catch (const std::invalid_argument&) { continue; }
                results.push_back({
                    {"provider", "wikimedia"}, {"title", string_field(page, "title")},
                    {"preview_url", url}, {"source_url", string_field(info, "descriptionurl")},
                    {"author", metadata(info, "Artist")}, {"license", metadata(info, "LicenseShortName")},
                    {"license_url", metadata(info, "LicenseUrl")}, {"credit", metadata(info, "Credit")},
                    {"attribution_required", metadata(info, "AttributionRequired")}, {"downloadable", true}
                });
            }
            return results;
        }
        if (provider == "openverse") {
            const auto data = get_json(query_url("https://api.openverse.org/v1/images/", {
                {"q", query}, {"page_size", std::to_string(limit)}
            }));
            if (!data.contains("results") || !data["results"].is_array()) return results;
            for (const auto& item : data["results"]) results.push_back({
                {"provider", "openverse"}, {"title", string_field(item, "title", "Untitled")},
                {"preview_url", string_field(item, "thumbnail")}, {"source_url", string_field(item, "foreign_landing_url")},
                {"author", string_field(item, "creator")}, {"license", string_field(item, "license")},
                {"license_url", string_field(item, "license_url")}, {"credit", string_field(item, "attribution")},
                {"downloadable", false}, {"id", string_field(item, "id")}
            });
            return results;
        }
        throw std::invalid_argument("Proveedor no reconocido");
    }
    Json download(const std::string& file_title, const fs::path& directory, const std::string& basename) const {
        slug(basename);
        if (file_title.compare(0, 5, "File:") || utf8_length(file_title) > 240 || file_title.find('\0') != std::string::npos)
            throw std::invalid_argument("Se requiere un titulo Commons File: de hasta 240 caracteres");
        const auto data = get_json(query_url("https://commons.wikimedia.org/w/api.php", {
            {"action", "query"}, {"titles", file_title}, {"prop", "imageinfo"},
            {"iiprop", "url|extmetadata"}, {"iiurlwidth", "1280"}, {"format", "json"}, {"formatversion", "2"}
        }));
        if (!data.contains("query") || !data["query"].contains("pages") || data["query"]["pages"].empty())
            throw std::runtime_error("Archivo Commons no encontrado");
        const auto& page = data["query"]["pages"][0];
        if ((page.contains("missing") && page["missing"] == true) || !page.contains("imageinfo") || page["imageinfo"].empty())
            throw std::runtime_error("Archivo Commons no encontrado");
        const auto& info = page["imageinfo"][0];
        const auto url = string_field(info, "thumburl", string_field(info, "url"));
        const auto response = fetch_https(url, {"upload.wikimedia.org", "thumb.wikimedia.org"}, 18 * 1024 * 1024, fetch_);
        std::string extension;
        if (response.body.size() >= 8 && !std::memcmp(response.body.data(), "\x89PNG\r\n\x1a\n", 8)) extension = ".png";
        else if (response.body.size() >= 3 && !std::memcmp(response.body.data(), "\xff\xd8\xff", 3)) extension = ".jpg";
        else throw std::invalid_argument("Solo PNG/JPEG; formato incompatible con el compositor");
        fs::create_directories(directory);
        const auto target = directory / (basename + extension);
        const auto sidecar = directory / (basename + ".attribution.json");
        safe_output(directory, target); safe_output(directory, sidecar);
        if (fs::exists(target) || fs::exists(sidecar)) throw std::runtime_error("La imagen o atribucion ya existe");
        Json attribution{
            {"filename", target.filename().u8string()}, {"source_title", string_field(page, "title", file_title)},
            {"source_url", string_field(info, "descriptionurl")}, {"author", metadata(info, "Artist")},
            {"license", metadata(info, "LicenseShortName")}, {"license_url", metadata(info, "LicenseUrl")},
            {"credit", metadata(info, "Credit")}, {"attribution_required", metadata(info, "AttributionRequired")}
        };
        atomic_write(target, response.body, false);
        try { atomic_write(sidecar, attribution.dump(2) + "\n", false); }
        catch (...) { std::error_code error; fs::remove(target, error); throw; }
        attribution["file"] = target.u8string(); attribution["attribution_file"] = sidecar.u8string();
        return attribution;
    }
};
} // namespace gptframes::mcp
