#pragma once
#include "media.hpp"
#include "process.hpp"
#include "render_jobs.hpp"
#include "tools.hpp"
#include "../voice/neo_bridge.hpp"
#include <optional>

namespace gptframes::mcp {
struct Config {
    fs::path root, projects, models, engine;
    std::string neo_lite = "neo-lite";
    static Config from_environment(const fs::path& executable, const fs::path& default_root) {
        Config c;
        c.root = fs::weakly_canonical(environment("GPTFRAMES_ROOT", default_root.u8string()));
        c.projects = fs::weakly_canonical(environment("GPTFRAMES_PROJECTS", (c.root / "projects").u8string()));
        c.models = fs::weakly_canonical(environment("GPTFRAMES_MODELS", (c.root / "models").u8string()));
        auto binary = executable.parent_path() /
#ifdef _WIN32
            "hyperframes-cpp-native.exe";
#else
            "hyperframes-cpp-native";
#endif
        if (!fs::is_regular_file(binary)) binary = c.root / "build" / "gptframes-linux" / binary.filename();
        c.engine = fs::absolute(environment("GPTFRAMES_ENGINE", binary.u8string()));
        c.neo_lite = environment("GPTFRAMES_NEO_LITE", "neo-lite");
        return c;
    }
};
class Service {
    Config config_;
    MediaClient media_;
    std::mutex mutex_;
    RenderQueue render_queue_;
    std::pair<fs::path, fs::path> project(const Json& name) const {
        const auto base = config_.projects / slug(name);
        if (!fs::is_directory(base) || fs::is_symlink(base) || fs::canonical(base).parent_path() != fs::weakly_canonical(config_.projects))
            throw std::runtime_error("Proyecto no encontrado");
        const auto file = base / "project.json";
        if (!fs::is_regular_file(file) || fs::is_symlink(file)) throw std::runtime_error("Falta project.json");
        return {base, file};
    }
    std::string engine(const std::vector<std::string>& parameters, int timeout = 75) const {
        if (!fs::is_regular_file(config_.engine)) throw std::runtime_error("Ejecutable GPTFrames no encontrado: compile el motor");
        std::vector<std::string> args{config_.engine.u8string()};
        args.insert(args.end(), parameters.begin(), parameters.end());
        auto result = run_process(args, timeout);
        if (result.timed_out) throw std::runtime_error("El ejecutable excedio el tiempo limite");
        if (result.output.size() > 1500) result.output.erase(0, result.output.size() - 1500);
        if (result.exit_code != 0) throw std::runtime_error("Motor fallo (" + std::to_string(result.exit_code) + "): " + result.output);
        return trim(result.output);
    }
    fs::path model(const std::string& name) const {
        if (name.empty() || name.size() > 106 || !ascii_alnum(static_cast<unsigned char>(name[0])) ||
            !std::all_of(name.begin(), name.end(), [](unsigned char c) { return ascii_alnum(c) || c == '-' || c == '_' || c == '.'; }))
            throw std::invalid_argument("Modelo admitido: nombre.onnx o nombre.neo");
        const fs::path filename(name);
        if (filename.extension() != ".onnx" && filename.extension() != ".neo")
            throw std::invalid_argument("Modelo admitido: nombre.onnx o nombre.neo");
        const auto path = config_.models / filename;
        if (!fs::is_regular_file(path) || fs::canonical(path).parent_path() != fs::weakly_canonical(config_.models))
            throw std::runtime_error("Modelo local no encontrado en GPTFRAMES_MODELS");
        return fs::canonical(path);
    }
    Json create(const std::string& name) const {
        fs::create_directories(config_.projects);
        const auto target = config_.projects / name;
        if (fs::exists(target) || fs::is_symlink(target)) throw std::runtime_error("El proyecto ya existe");
        const auto fixture = config_.root / "examples" / "json-showcase-v2";
        const auto source_assets = fixture / "assets";
        if (!fs::is_directory(source_assets)) throw std::runtime_error("Falta plantilla json-showcase-v2");
        TemporaryDirectory temporary(config_.projects);
        const auto staged = temporary.path / "project";
        fs::create_directory(staged);
        fs::create_directory(staged / "assets");
        for (const auto& entry : fs::recursive_directory_iterator(source_assets)) {
            if (entry.is_symlink()) throw std::runtime_error("La plantilla no admite enlaces");
            const auto destination = staged / "assets" / fs::relative(entry.path(), source_assets);
            if (entry.is_directory()) fs::create_directories(destination);
            else if (entry.is_regular_file()) fs::copy_file(entry.path(), destination);
            else throw std::runtime_error("Tipo de recurso de plantilla no admitido");
        }
        atomic_write(staged / "project.json", read_file(fixture / "showcase-v2.json"), false);
        fs::rename(staged, target);
        return {{"project", name}, {"project_json", (target / "project.json").u8string()},
                {"tip", "Personaliza project.json para animar textos, imagenes y keyframes."}};
    }
public:
    explicit Service(Config config, MediaClient media = MediaClient{})
        : config_(std::move(config)), media_(std::move(media)),
          render_queue_(config_.projects.parent_path(),config_.engine) {}
    RenderQueue& render_jobs(){return render_queue_;}
    const Config& config() const { return config_; }
    Json run_tool(const std::string& name, const Json& args) {
        validate_arguments(name, args);
        std::lock_guard<std::mutex> lock(mutex_);
        if (name == "list_projects") {
            std::vector<std::string> names;
            if (fs::is_directory(config_.projects))
                for (const auto& entry : fs::directory_iterator(config_.projects)) {
                    if (!entry.is_directory() || entry.is_symlink() || !fs::is_regular_file(entry.path() / "project.json") ||
                        fs::is_symlink(entry.path() / "project.json")) continue;
                    try { names.push_back(slug(entry.path().filename().u8string())); } catch (const std::invalid_argument&) {}
                }
            std::sort(names.begin(), names.end());
            if (names.size() > 100) names.resize(100);
            return {{"projects", names}};
        }
        if (name == "create_project") return create(slug(args.at("name")));
        if (name == "render_status") return render_queue_.status(args.at("job_id").get<std::string>());
        if (name == "cancel_render") return render_queue_.cancel(args.at("job_id").get<std::string>());
        if (name == "list_voice_models") {
            std::vector<std::string> models;
            if(fs::is_directory(config_.models))for(const auto& entry:fs::directory_iterator(config_.models)){
                if(!entry.is_regular_file()||entry.is_symlink())continue;
                auto ext=entry.path().extension().string();
                if(ext==".neo"||ext==".onnx")models.push_back(entry.path().filename().u8string());
            }
            std::sort(models.begin(),models.end());
            if(models.size()>100)models.resize(100);
            return {{"models",models}};
        }
        if (name == "search_images")
            return {{"results", media_.search(args.at("query").get<std::string>(), args.value("provider", "wikimedia"), args.value("limit", 6))}};
        const auto [base, file] = project(args.at("project"));
        if(name=="review_layout"){
            const auto result=engine({"--project","lint",file.u8string()},90);
            return Json::parse(result);
        }
        if(name=="start_render"){
            // Fail before queueing if the configured HTTPS origin is not ready.
            auto origin=environment("GPTFRAMES_PUBLIC_URL");
            if(origin.empty()){
                const auto origin_file=environment("GPTFRAMES_PUBLIC_URL_FILE");
                if(!origin_file.empty()&&fs::is_regular_file(fs::u8path(origin_file)))
                    origin=trim(read_file(fs::u8path(origin_file),512));
            }
            if(origin.rfind("https://",0)!=0||origin.size()>250)
                throw std::runtime_error("Configure a public HTTPS origin before starting remote render");
            engine({"--project","validate",file.u8string()},90);
            return render_queue_.start(base.filename().u8string(),file,args.value("seconds",0.0),origin);
        }
        if(name=="read_project"){
            const auto raw=read_file(file);
            return {{"project",base.filename().u8string()},
                    {"etag",oauth_digest(raw)},{"document",Json::parse(raw)}};
        }
        if(name=="save_project"){
            const auto raw=read_file(file);
            const auto expected=args.at("etag").get<std::string>();
            if(expected.size()!=43||oauth_digest(raw)!=expected)
                throw std::runtime_error("Project changed; read_project again before saving.");
            const auto document=args.at("document");
            if(!document.is_object())throw std::invalid_argument("document must be JSON object");
            const auto content=document.dump(2)+"\n";
            if(content.size()>max_message_bytes)throw std::invalid_argument("Project exceeds 1 MB");
            const auto candidate=base/(".validate-"+oauth_random_token()+".json");
            struct Cleanup {
                fs::path path;
                ~Cleanup(){std::error_code ec;fs::remove(path,ec);}
            } cleanup{candidate};
            atomic_write(candidate,content,false);
            engine({"--project","validate",candidate.u8string()},90);
            atomic_write(file,content);
            return {{"project",base.filename().u8string()},
                    {"etag",oauth_digest(content)},{"validated",true}};
        }
        if (name == "download_commons_image") {
            const auto directory = base / "assets";
            safe_directory(base, directory);
            safe_output(base, directory / ".asset");
            auto result = media_.download(args.at("file_title").get<std::string>(), directory, slug(args.at("asset_name")));
            result["src"] = "assets/" + fs::u8path(result["file"].get<std::string>()).filename().u8string();
            return result;
        }
        if (name == "validate_project") return {{"message", engine({"--project", "validate", file.u8string()})}};
        if (name == "preview_frame") {
            const auto output = base / "preview.png";
            safe_output(base, output);
            const auto message = engine({"--project", "frame", file.u8string(), output.u8string(),
                                         number_text(args.at("second").get<double>())});
            return {{"message", message}, {"preview_path", output.u8string()}};
        }
        if (name == "render_video") {
            const auto doc = Json::parse(read_file(file));
            const auto scenes = doc.value("scenes", Json::array());
            if (!scenes.is_array() || scenes.empty()) throw std::invalid_argument("El proyecto requiere escenas");
            const double total = scenes.back().value("start", 0.0) + scenes.back().value("duration", 0.0);
            if (!args.contains("seconds") && total > 60) throw std::invalid_argument("Proyecto de mas de 60s: indique una duracion menor");
            const auto output = base / "render.mp4";
            safe_output(base, output);
            std::vector<std::string> parameters{"--project", "render", file.u8string(), output.u8string()};
            if (args.contains("seconds")) parameters.push_back(number_text(args["seconds"].get<double>()));
            const auto message = engine(parameters, 240);
            if (!fs::is_regular_file(output)) throw std::runtime_error("El motor no creo el video");
            return {{"message", message}, {"video_path", output.u8string()}, {"bytes", fs::file_size(output)}};
        }
        if (name == "synthesize_voice") {
            const auto text = args.at("text").get<std::string>();
            if (blank(text)) throw std::invalid_argument("Texto de voz vacio");
            const auto source_model = model(args.at("model").get<std::string>());
            const auto voice = slug(args.value("track_name", "narracion"));
            auto doc = Json::parse(read_file(file));
            if (!doc.contains("audio")) doc["audio"] = Json::array();
            if (!doc["audio"].is_array() || doc["audio"].size() >= 12) throw std::invalid_argument("El proyecto admite como maximo 12 pistas");
            const auto directory = base / "assets" / "voices";
            safe_directory(base, directory);
            safe_output(base, directory / ".voice");
            const auto wav = directory / (voice + ".wav"), txt = directory / (voice + ".txt");
            safe_output(base, wav); safe_output(base, txt);
            if (fs::exists(wav) || fs::exists(txt)) throw std::runtime_error("Nombre de pista existente");
            atomic_write(txt, text, false);
            try {
                const auto message = engine({"--voice", "synth", "--model", source_model.u8string(),
                    "--text-file", txt.u8string(), "--out", wav.u8string(), "--neo-lite", config_.neo_lite}, 180);
                const auto pcm = gptvoice::readWav(wav);
                if (pcm.sampleRate != 24000) throw std::runtime_error("La voz requiere WAV PCM a 24kHz");
                doc["audio"].push_back({{"src", "assets/voices/" + wav.filename().u8string()},
                                       {"start", args.value("start", 0.0)}, {"gain", 1}});
                atomic_write(file, doc.dump(2) + "\n");
                return {{"message", message}, {"wav_path", wav.u8string()}, {"project", base.filename().u8string()}};
            } catch (...) {
                std::error_code error; fs::remove(txt, error); fs::remove(wav, error); throw;
            }
        }
        throw std::invalid_argument("Herramienta no reconocida");
    }
    std::optional<Json> rpc(const Json& request, const std::string& transport_version = "2025-06-18") {
        const bool valid_id = request.is_object() && (!request.contains("id") || request["id"].is_null() ||
                              request["id"].is_string() || request["id"].is_number_integer());
        if (!request.is_object() || request.value("jsonrpc", Json{}) != "2.0" ||
            !request.contains("method") || !request["method"].is_string() || !valid_id)
            return Json{{"jsonrpc", "2.0"}, {"id", nullptr}, {"error", {{"code", -32600}, {"message", "Invalid Request"}}}};
        if (!request.contains("id")) return std::nullopt;
        Json response{{"jsonrpc", "2.0"}, {"id", request["id"]}};
        const auto method = request["method"].get<std::string>();
        const bool modern = transport_version == "2026-07-28";
        if (method == "server/discover" && modern)
            response["result"] = {{"resultType", "complete"},
                {"supportedVersions", Json::array({"2026-07-28", "2025-11-25", "2025-06-18"})},
                {"capabilities", {{"tools", {{"listChanged", false}}}}},
                {"_meta", {{"io.modelcontextprotocol/serverInfo", {{"name", "gptframes"}, {"title", "GPTFrames"}, {"version", "0.2.0"}}}}},
                {"ttlMs", 30000}, {"cacheScope", "private"}};
        else if (method == "initialize") {
            auto version = transport_version.empty() ? std::string("2025-06-18") : transport_version;
            const auto params = request.value("params", Json::object());
            if (params.is_object() && params.contains("protocolVersion") && params["protocolVersion"].is_string()) {
                const auto requested = params["protocolVersion"].get<std::string>();
                version = (requested == "2025-03-26" || requested == "2025-06-18" || requested == "2025-11-25")
                    ? requested : "2025-11-25";
            }
            response["result"] = {{"protocolVersion", version}, {"capabilities", {{"tools", {{"listChanged", false}}}}},
                                  {"serverInfo", {{"name", "gptframes"}, {"version", "0.2.0"}}}};
        }
        else if (method == "ping") response["result"] = Json::object();
        else if (method == "tools/list") response["result"] = {{"tools", tool_definitions()}};
        else if (method == "resources/list") response["result"] = {{"resources", Json::array()}};
        else if (method == "tools/call") {
            const auto parameters = request.value("params", Json::object());
            if (!parameters.is_object() || !parameters.contains("name") || !parameters["name"].is_string()) {
                response["error"] = {{"code", -32602}, {"message", "Invalid params"}};
                return response;
            }
            try {
                const auto result = run_tool(parameters["name"].get<std::string>(), parameters.value("arguments", Json::object()));
                response["result"] = {{"content", Json::array({{{"type", "text"}, {"text", result.dump()}}})}, {"isError", false}};
            } catch (const std::exception& error) {
                response["result"] = {{"content", Json::array({{{"type", "text"}, {"text", std::string(error.what()).substr(0, 1500)}}})}, {"isError", true}};
            }
        } else response["error"] = {{"code", -32601}, {"message", "Method not found"}};
        if (modern && response.contains("result") &&
            (method == "tools/list" || method == "tools/call")) {
            response["result"]["resultType"] = "complete";
            if (method == "tools/list") {
                response["result"]["ttlMs"] = 30000;
                response["result"]["cacheScope"] = "private";
            }
        }
        return response;
    }
};
inline Json parse_error(const std::string& message = "Parse error") {
    return {{"jsonrpc", "2.0"}, {"id", nullptr}, {"error", {{"code", -32700}, {"message", message}}}};
}
inline void serve_stdio(Service& service, std::istream& input = std::cin, std::ostream& output = std::cout) {
    std::string line;
    bool oversized = false;
    char c;
    auto dispatch = [&] {
        if (oversized) output << parse_error("Request exceeds 1000000 bytes").dump() << '\n' << std::flush;
        else if (!line.empty()) {
            try {
                const auto response = service.rpc(Json::parse(line));
                if (response) output << response->dump() << '\n' << std::flush;
            } catch (const std::exception&) { output << parse_error().dump() << '\n' << std::flush; }
        }
        line.clear(); oversized = false;
    };
    while (input.get(c)) {
        if (c == '\n') dispatch();
        else if (!oversized) {
            if (line.size() == max_message_bytes) { line.clear(); oversized = true; }
            else line += c;
        }
    }
    if (!line.empty() || oversized) dispatch();
}
} // namespace gptframes::mcp
