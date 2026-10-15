#include "../mcp/common.hpp"
#include "../voice/neo_bridge.hpp"
#include <set>
using namespace gptframes::mcp;
static void scene(const fs::path& output) {
    std::ostringstream page;
    page << "<!doctype html>\n<html lang=\"es\"><head><meta charset=\"utf-8\">\n"
         << "<style>.upper{top:210px;left:65px}.kicker{height:52px;font-size:17px}\n"
         << ".headline{font-size:68px;line-height:1.08;margin-top:50px}\n"
         << ".sub{margin-top:42px;font-size:24px}</style></head><body>\n";
    for (int i = 0; i < 6; ++i)
        page << "<section class=\"scene scene-" << i + 1 << "\" data-start=\"" << i * 2
             << "\" data-duration=\"2\" style=\"--a:#47DEF5\">\n"
             << "<div class=\"kicker\">MOTOR DE VIDEO</div>\n"
             << "<div class=\"headline\"><div class=\"wordline\">VIDEO NATIVO " << i + 1 << "</div></div>\n"
             << "<div class=\"sub\">Prueba técnica reproducible</div>\n"
             << "<div class=\"barlabel\">SALIDA NATIVA</div>\n</section>\n";
    page << "</body></html>\n";
    atomic_write(output, page.str());
}
static void audio(const fs::path& directory) {
    fs::create_directories(directory);
    for (int i = 1; i <= 6; ++i) {
        gptvoice::PCM pcm;
        pcm.sampleRate = 24000;
        for (int n = 0; n < 8000; ++n) {
            const double time = static_cast<double>(n) / 24000;
            const double envelope = std::min(1.0, time * 20) * std::min(1.0, (1.0 / 3 - time) * 20);
            pcm.samples.push_back(static_cast<int16_t>(2500 * envelope * std::sin(2 * 3.14159265358979323846 * (320 + 20 * i) * time)));
        }
        std::ostringstream name;
        name << "segment-" << std::setw(2) << std::setfill('0') << i << ".wav";
        gptvoice::write24k(pcm, directory / name.str());
    }
}
static void mutate(const fs::path& source, const fs::path& directory, const std::string& mode) {
    fs::create_directories(directory);
    const auto original = Json::parse(read_file(source));
    auto save = [&](const std::string& name, const Json& doc) { atomic_write(directory / name, doc.dump(2) + "\n"); };
    if (mode == "compositor") {
        auto doc = original;
        doc["scenes"][0]["layers"][1]["text"] = "TEXTO MODIFICADO"; save("modified.json", doc);
        doc = original; doc["scenes"][1]["start"] = 7; save("wrong-order.json", doc);
        doc = original; doc["scenes"][0]["layers"][4]["src"] = "../private.png"; save("traversal.json", doc);
    } else if (mode == "showcase") {
        std::set<std::string> kinds;
        for (const auto& s : original.at("scenes")) for (const auto& layer : s.at("layers"))
            kinds.insert(layer.at("type").get<std::string>());
        for (const auto* required : {"glow", "ring", "grid", "rect", "text", "line", "ellipse"})
            if (!kinds.count(required)) throw std::runtime_error("Falta geometria esperada");
        auto doc = original;
        bool changed = false;
        for (auto& layer : doc["scenes"][0]["layers"]) if (layer["type"] == "rect" && layer.contains("radius")) {
            layer["radius"] = 99999; changed = true; break;
        }
        if (!changed) throw std::runtime_error("Falta rectangulo con radio");
        save("invalid-radius.json", doc);
        doc = original; doc["scenes"][0]["background"] = "#GGGGGG"; save("invalid-color.json", doc);
    } else throw std::invalid_argument("Modo de fixture JSON desconocido");
}
int main(int argc, char** argv) {
    try {
        if (argc == 3 && std::string(argv[1]) == "scene") scene(fs::u8path(argv[2]));
        else if (argc == 3 && std::string(argv[1]) == "audio") audio(fs::u8path(argv[2]));
        else if (argc == 5 && std::string(argv[1]) == "json") mutate(fs::u8path(argv[2]), fs::u8path(argv[3]), argv[4]);
        else if ((argc == 6 || argc == 7) && std::string(argv[1]) == "replace") {
            auto text = read_file(fs::u8path(argv[2]), 4000000);
            const std::string from = argv[4], to = argv[5];
            if (from.empty()) throw std::invalid_argument("Texto de reemplazo vacio");
            const std::size_t maximum = argc == 7 ? std::stoul(argv[6]) : static_cast<std::size_t>(-1);
            std::size_t index = 0, count = 0;
            while (count < maximum && (index = text.find(from, index)) != std::string::npos) {
                text.replace(index, from.size(), to); index += to.size(); ++count;
            }
            if (!count) throw std::runtime_error("No se encontro texto para editar");
            atomic_write(fs::u8path(argv[3]), text);
        } else throw std::invalid_argument("Uso: gptframes-test-fixtures scene|audio SALIDA; json ENTRADA DIRECTORIO MODO; replace ENTRADA SALIDA ORIGINAL NUEVO [LIMITE]");
        return 0;
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
