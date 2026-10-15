#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
namespace fs = std::filesystem;
static std::vector<unsigned char> font_bytes(const fs::path& file) {
    if (!fs::is_regular_file(file) || fs::file_size(file) > 2000000)
        throw std::runtime_error("Fuente ausente o mayor de 2 MB: " + file.u8string());
    std::ifstream input(file, std::ios::binary);
    std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(input)), {});
    if (!input.eof() && input.fail()) throw std::runtime_error("Error leyendo fuente");
    if (bytes.size() < 4 ||
        !((bytes[0] == 0 && bytes[1] == 1 && bytes[2] == 0 && bytes[3] == 0) ||
          (bytes[0] == 'O' && bytes[1] == 'T' && bytes[2] == 'T' && bytes[3] == 'O')))
        throw std::runtime_error("Fuente TTF/OTF inesperada: " + file.u8string());
    return bytes;
}
int main(int argc, char** argv) {
    try {
        if (argc > 3) throw std::invalid_argument("Uso: gptframes-embed-fonts [DIRECTORIO_FUENTES [SALIDA.hpp]]");
        const auto directory = argc > 1 ? fs::u8path(argv[1]) : fs::path("/usr/share/fonts/truetype/dejavu");
        const auto output = argc > 2 ? fs::u8path(argv[2]) : fs::path("font_embedded.hpp");
        const auto regular = font_bytes(directory / "DejaVuSans.ttf");
        const auto bold = font_bytes(directory / "DejaVuSans-Bold.ttf");
        if (!output.parent_path().empty()) fs::create_directories(output.parent_path());
        auto temporary = output;
        temporary += ".tmp-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        try {
            std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
            if (!file) throw std::runtime_error("No se pudo crear encabezado de fuentes");
            file << "#pragma once\n// Fuentes DejaVu originales embebidas. Consultar assets/fonts/LICENSE.txt.\nnamespace bundled {\n";
            const std::array<std::pair<const char*, const std::vector<unsigned char>*>, 2> fonts{{
                {"regular", &regular}, {"bold", &bold}
            }};
            for (const auto& font : fonts) {
                file << "static const unsigned char " << font.first << "[] = {\n";
                const auto& bytes = *font.second;
                for (std::size_t i = 0; i < bytes.size(); ++i) {
                    file << "0x" << std::hex << std::setfill('0') << std::setw(2) << static_cast<unsigned>(bytes[i]) << ',';
                    if ((i + 1) % 24 == 0) file << '\n';
                }
                file << "\n};\n";
            }
            file << "}\n";
            file.close();
            if (!file) throw std::runtime_error("Error escribiendo fuentes");
            std::error_code error;
            fs::rename(temporary, output, error);
            if (error) { fs::remove(output); fs::rename(temporary, output); }
        } catch (...) { std::error_code error; fs::remove(temporary, error); throw; }
        std::cout << "Encabezado generado: " << output.u8string() << '\n';
        return 0;
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
