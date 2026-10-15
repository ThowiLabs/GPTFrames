#pragma once
#include "../vendor/browser/json.hpp"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <locale>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <windows.h>
#endif

namespace gptframes::mcp {
namespace fs = std::filesystem;
using Json = nlohmann::json;
constexpr std::size_t max_message_bytes = 1000000;

inline std::string environment(const char* name, const std::string& fallback = {}) {
    const auto* value = std::getenv(name);
    return value && *value ? value : fallback;
}
inline bool ascii_alnum(unsigned char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
}
inline std::string slug(const Json& value) {
    if (!value.is_string()) throw std::invalid_argument("Nombre de proyecto requerido");
    auto s = value.get<std::string>();
    if (s.empty() || s.size() > 40 || !ascii_alnum(static_cast<unsigned char>(s[0])) ||
        !std::all_of(s.begin(), s.end(), [](unsigned char c) { return ascii_alnum(c) || c == '-' || c == '_'; }))
        throw std::invalid_argument("El nombre admite 1-40 caracteres: letras, numeros, guion y guion bajo");
    return s;
}
inline std::size_t utf8_length(const std::string& text) {
    return static_cast<std::size_t>(std::count_if(text.begin(), text.end(), [](unsigned char c) {
        return (c & 0xc0) != 0x80;
    }));
}
inline bool blank(const std::string& text) {
    return text.empty() || std::all_of(text.begin(), text.end(), [](unsigned char c) {
        return c == ' ' || c == '\t' || c == '\r' || c == '\n';
    });
}
inline std::string read_file(const fs::path& path, std::size_t maximum = max_message_bytes) {
    if (!fs::is_regular_file(path) || fs::file_size(path) > maximum)
        throw std::runtime_error("Archivo ausente o demasiado grande: " + path.filename().string());
    std::ifstream input(path, std::ios::binary);
    if (!input) throw std::runtime_error("No se pudo abrir archivo");
    std::string result;
    char block[8192];
    while (input) {
        input.read(block, sizeof(block));
        const auto count = static_cast<std::size_t>(input.gcount());
        if (count > maximum - result.size()) throw std::runtime_error("Archivo demasiado grande");
        result.append(block, count);
    }
    if (!input.eof()) throw std::runtime_error("Error leyendo archivo");
    return result;
}
inline bool within(const fs::path& root, const fs::path& path) {
    auto a = root.begin(), b = path.begin();
    for (; a != root.end(); ++a, ++b) if (b == path.end() || *a != *b) return false;
    return true;
}
inline void safe_output(const fs::path& root, const fs::path& path) {
    if (!within(fs::canonical(root), fs::canonical(path.parent_path())) || fs::is_symlink(path))
        throw std::runtime_error("Enlace o salida fuera del proyecto");
}
inline void safe_directory(const fs::path& root, const fs::path& directory) {
    const auto base = fs::canonical(root);
    if (!within(base, fs::weakly_canonical(directory)))
        throw std::runtime_error("Directorio fuera del proyecto");
    fs::create_directories(directory);
    if (!within(base, fs::canonical(directory))) throw std::runtime_error("Directorio fuera del proyecto");
}
inline std::string unique_token() {
    static std::atomic<unsigned long long> sequence{0};
    return std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + "-" +
           std::to_string(sequence.fetch_add(1));
}
struct TemporaryDirectory {
    fs::path path;
    explicit TemporaryDirectory(const fs::path& parent, const std::string& prefix = ".gptframes-") {
        for (int i = 0; i < 64; ++i) {
            auto candidate = parent / (prefix + unique_token());
            std::error_code error;
            if (fs::create_directory(candidate, error)) {
                path = candidate;
                fs::permissions(path, fs::perms::owner_all, fs::perm_options::replace, error);
                return;
            }
            if (error && error != std::errc::file_exists) throw fs::filesystem_error("Crear temporal", candidate, error);
        }
        throw std::runtime_error("No se pudo reservar directorio temporal");
    }
    TemporaryDirectory(const TemporaryDirectory&) = delete;
    TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;
    ~TemporaryDirectory() {
        if (!path.empty()) { std::error_code error; fs::remove_all(path, error); }
    }
};
inline void write_file(const fs::path& path, const std::string& data) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out || !out.write(data.data(), static_cast<std::streamsize>(data.size())) || !out.flush())
        throw std::runtime_error("No se pudo escribir archivo: " + path.filename().string());
}
inline void atomic_write(const fs::path& path, const std::string& data, bool replace = true) {
    const auto parent = path.parent_path().empty() ? fs::path(".") : path.parent_path();
    if (fs::is_symlink(path) || (!replace && fs::exists(path)))
        throw std::runtime_error("El archivo ya existe o es un enlace");
    TemporaryDirectory temporary(parent);
    const auto staged = temporary.path / "content";
    write_file(staged, data);
    if (!replace && fs::exists(path)) throw std::runtime_error("El archivo ya existe");
#ifdef _WIN32
    if (!MoveFileExW(staged.wstring().c_str(), path.wstring().c_str(),
                    MOVEFILE_WRITE_THROUGH | (replace ? MOVEFILE_REPLACE_EXISTING : 0)))
        throw std::runtime_error("No se pudo publicar el archivo");
#else
    fs::rename(staged, path);
#endif
}
inline std::string number_text(double number) {
    if (!std::isfinite(number)) throw std::invalid_argument("Numero no finito");
    std::ostringstream text;
    text.imbue(std::locale::classic());
    text << std::setprecision(17) << number;
    return text.str();
}
inline bool constant_time_equal(const std::string& a, const std::string& b) {
    std::size_t mismatch = a.size() ^ b.size();
    for (std::size_t i = 0; i < std::max(a.size(), b.size()); ++i)
        mismatch |= static_cast<unsigned char>(i < a.size() ? a[i] : 0) ^
                    static_cast<unsigned char>(i < b.size() ? b[i] : 0);
    return mismatch == 0;
}
} // namespace gptframes::mcp
