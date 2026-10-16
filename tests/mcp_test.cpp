#include "../mcp/service.hpp"
#include <functional>
#include <set>
using namespace gptframes::mcp;
namespace {
void require(bool condition, const std::string& message) { if (!condition) throw std::runtime_error(message); }
template<class F> void rejects(F&& function) {
    bool failed = false;
    try { function(); } catch (const std::exception&) { failed = true; }
    require(failed, "Expected operation to fail");
}
Json request(int id, const std::string& method, Json params = Json::object()) {
    return {{"jsonrpc", "2.0"}, {"id", id}, {"method", method}, {"params", std::move(params)}};
}
HttpResponse commons_fixture(const std::string&, std::size_t) {
    Json info{
        {"url", "https://upload.wikimedia.org/test.png"},
        {"thumburl", "https://upload.wikimedia.org/test.png"},
        {"descriptionurl", "https://commons.wikimedia.org/wiki/File:Test.png"},
        {"extmetadata", {
            {"Artist", {{"value", "<a>Ana &amp; Jos&#233;</a>"}}},
            {"LicenseShortName", {{"value", "CC BY 4.0"}}},
            {"LicenseUrl", {{"value", "https://creativecommons.org/licenses/by/4.0/"}}},
            {"AttributionRequired", {{"value", "true"}}}
        }}
    };
    return {200, Json{{"query", {{"pages", Json::array({{{"title", "File:Test.png"}, {"imageinfo", Json::array({info})}}})}}}}.dump(), {}, "application/json"};
}
}
int main(int argc, char** argv) {
    // Native subprocess fixtures; no real inference is claimed by these cases.
    if (argc > 1 && std::string(argv[1]) == "--echo") {
        std::vector<std::string> args;
        for (int i = 2; i < argc; ++i) args.emplace_back(argv[i]);
        std::cout << Json(args).dump(); return 0;
    }
    if (argc > 1 && std::string(argv[1]) == "--sleep") {
        std::this_thread::sleep_for(std::chrono::seconds(3)); return 0;
    }
    if (argc > 1 && std::string(argv[1]) == "--flood") {
        const std::string block(8192, 'x');
        const auto until = std::chrono::steady_clock::now() + std::chrono::seconds(3);
        while (std::chrono::steady_clock::now() < until) std::cout.write(block.data(), block.size()).flush();
        return 0;
    }
    if (argc > 1 && std::string(argv[1]) == "--stdin") {
        std::string text((std::istreambuf_iterator<char>(std::cin)), {});
        std::cout << Json{{"text", text}}.dump(); return 0;
    }
    if (argc > 1 && std::string(argv[1]) == "--fail") { std::cerr << "native failure"; return 7; }
    if (argc > 1 && std::string(argv[1]) == "--voice") {
        fs::path out, model;
        for (int i = 2; i + 1 < argc; ++i) {
            if (std::string(argv[i]) == "--out") out = fs::u8path(argv[i + 1]);
            if (std::string(argv[i]) == "--model") model = fs::u8path(argv[i + 1]);
        }
        if (out.empty()) return 9;
        gptvoice::PCM pcm{24000, std::vector<int16_t>(6000, 40)};
        gptvoice::write24k(pcm, out);
        if (model.filename() == "fail.onnx") return 17;
        if (model.filename() == "invalid.onnx") write_file(out, "not a WAV");
        return 0;
    }
    if (argc != 3) { std::cerr << "Usage: gptframes-mcp-test ENGINE SOURCE_ROOT\n"; return 2; }
    int passed = 0, failed = 0;
    auto test = [&](const std::string& name, const std::function<void()>& body) {
        try { body(); ++passed; std::cout << "PASS " << name << '\n'; }
        catch (const std::exception& error) { ++failed; std::cerr << "FAIL " << name << ": " << error.what() << '\n'; }
    };
    const auto source = fs::canonical(fs::u8path(argv[2]));
    TemporaryDirectory temp(fs::temp_directory_path(), "gptframes-native-tests-");
    Config config{source, temp.path / "projects", temp.path / "models", fs::absolute(fs::u8path(argv[1])), "neo-lite"};
    const auto self = fs::absolute(fs::u8path(argv[0]));
    fs::create_directories(config.models);
    Service service(config, MediaClient(commons_fixture));
    test("initialize and protocol version", [&] {
        const auto r = service.rpc(request(1, "initialize"));
        require(r && (*r)["result"]["serverInfo"]["name"] == "gptframes", "Wrong server");
        require((*r)["result"]["protocolVersion"] == "2025-06-18", "Wrong protocol");
    });
    test("all native tool contracts preserved", [&] {
        const auto r = service.rpc(request(2, "tools/list"));
        std::set<std::string> names;
        for (const auto& t : (*r)["result"]["tools"]) names.insert(t["name"].get<std::string>());
        require(names == std::set<std::string>{"resource_status","rescan_resources","search_animations","list_animation_facets","list_animations","describe_animation","list_asset_presets","describe_asset_preset","list_projects", "create_project", "validate_project", "preview_frame",
                "render_video", "synthesize_voice", "search_images", "download_commons_image",
                "list_voice_models", "review_layout", "start_render", "render_status", "cancel_render",
                "read_project", "save_project"}, "Tool mismatch");
    });
    test("hot discovery of voice models and unregistered HTML", [&] {
        auto before=service.run_tool("resource_status",Json::object());
        require(before["live"]==true,"Monitor should be live");
        const auto new_neo=config.models/"hot_reload_fixture.neo";
        write_file(new_neo,"synthetic voice fixture");
        auto list=service.run_tool("list_voice_models",Json::object());
        require(std::find(list["models"].begin(),list["models"].end(),"hot_reload_fixture.neo")!=list["models"].end(),
                "New voice must be visible without restarting Service");
        fs::remove(new_neo);
        list=service.run_tool("list_voice_models",Json::object());
        require(std::find(list["models"].begin(),list["models"].end(),"hot_reload_fixture.neo")==list["models"].end(),
                "Removed voice must disappear without restarting Service");
        auto refreshed=service.run_tool("rescan_resources",Json::object());
        require(refreshed["counts"]["animation_files"]>=95,"Assets scan missing");
    });
    test("unregistered HTML discovered without MCP restart", [&] {
        auto local=temp.path/"dynamic_animation_fixture";
        fs::create_directories(local/"animaciones"/"presets");
        write_file(local/"animaciones"/"catalogo.json",Json{{"version",1},{"presets",Json::array()}}.dump());
        require(animation_library(local)["presets"].empty(),"Initial catalog should be empty");
        auto added=local/"animaciones"/"presets"/"new-demo.html";
        write_file(added,"<!doctype html><style>.demo{opacity:.9}</style><div>local</div>");
        auto fresh=animation_library(local);
        require(fresh["total"]==1 && fresh["presets"][0]["id"]=="new-demo" &&
               fresh["presets"][0]["renderable"]==false,"New HTML must be discovered but never trusted");
        fs::remove(added);
        require(animation_library(local)["total"]==0,"Deleted preset should disappear live");
    });
    test("catalogue of numbered presets", [&] {
        const auto all=service.run_tool("list_asset_presets",Json::object());
        require(all["total"]==20,"Expected 20 original assets");
        require(all["presets"][0]["number"]==1,"Numbering must start at 1");
        require(all["presets"][19]["number"]==20,"Numbering must end at 20");
        const auto type=service.run_tool("list_asset_presets",{{"category","Seguridad"}});
        require(type["total"]==4,"Expected four security designs");
        const auto item=service.run_tool("describe_asset_preset",{{"id","orbit-rings"}});
        require(item["name"]=="Orbit Rings" && item["description"].is_string(),"Preset detail invalid");
        rejects([&] { service.run_tool("describe_asset_preset",{{"id","not-present"}}); });
    });
    test("deduplicated animation library", [&] {
        const auto all=service.run_tool("list_animations",Json::object());
        require(all["total"]==95,"Expected 95 unique animations");
        require(all["presets"][0]["number"]==1,"Expected numbered animation");
        require(all["presets"][5]["id"]=="biometric-scanner","Six original modules were displaced");
        require(all["presets"][22]["id"]=="shimmer-progress","Last imported animation mismatch");
        const auto group=service.run_tool("list_animations",{{"category","Seguridad"}});
        require(group["total"]==2,"Expected two initial security animations");
        const auto characters=service.run_tool("list_animations",{{"category","Personajes"}});
        require(characters["total"]==6,"Expected six new character assets");
        const auto cpu=service.run_tool("search_animations",{{"query","procesador Xeon"}});
        require(cpu["total"]>=1 && cpu["results"][0]["id"]=="editable-cpu","CPU rank failed");
        const auto rf=service.run_tool("search_animations",{{"query","antena rf"},{"category","Radiofrecuencia"}});
        require(rf["total"]==2,"RF filters failed");
        const auto marquee=service.run_tool("search_animations",{{"query","cintillo noticias"}});
        require(marquee["total"]>=1 && marquee["results"][0]["id"]=="breaking-news-ticker","News ticker rank failed");
        const auto terminals=service.run_tool("list_animations",{{"category","Terminales Linux"}});
        require(terminals["total"]==4,"Expected four detailed Linux terminals");
        const auto bash=service.run_tool("describe_animation",{{"id","terminal-bash-nginx"}});
        require(bash["editable"].contains("command") && bash["motion"].is_array(),
                "Terminal must support an editable command");
        const auto boot=service.run_tool("search_animations",{{"query","systemd arranque"}});
        require(boot["total"]>=1,"Boot assets should be searchable");
        for(const auto& cat:{"Arquitectura de software","Programación y ejecución","Lenguajes de programación"}){
            auto set=service.run_tool("list_animations",{{"category",cat}});
            require(set["total"]==10,"New collection must include ten independent animations");
        }
        auto py=service.run_tool("describe_animation",{{"id","language-python"}});
        require(py["editable"].contains("classname") && py["states"]==5,
                "Python module must expose editable code identifiers and stages");
        auto arch=service.run_tool("describe_animation",{{"id","architecture-microservicios"}});
        require(arch["source_module_id"]=="microservices",
                "Architecture asset must preserve source provenance");
        const auto types=service.run_tool("list_animations",{{"type","servidor"}});
        require(types["total"]>=2,"Asset type filter failed");
        const auto facets=service.run_tool("list_animation_facets",Json::object());
        require(facets["total"]==95 && facets["category"].is_array() && facets["tags"].is_array(),"Facet index invalid");
        rejects([&]{service.run_tool("search_animations",{{"query"," "}});});
        const auto item=service.run_tool("describe_animation",{{"id","smartphone-vibrate"}});
        require(item["name"]=="Teléfono vibrando","Wrong animation");
        rejects([&] {service.run_tool("describe_animation",{{"id","invalid"}});});
    });
    test("unknown method and invalid params", [&] {
        require((*service.rpc(request(3, "shutdown")))["error"]["code"] == -32601, "Unknown method accepted");
        require((*service.rpc(request(4, "tools/call", nullptr)))["error"]["code"] == -32602, "Invalid params accepted");
        require((*service.rpc(Json::array()))["error"]["code"] == -32600, "Array accepted as request");
        require((*service.rpc({{"jsonrpc", "2.0"}, {"id", true}, {"method", "ping"}}))["error"]["code"] == -32600, "Boolean id accepted");
    });
    test("notifications and resources", [&] {
        require(!service.rpc({{"jsonrpc", "2.0"}, {"method", "notifications/initialized"}}), "Notification response emitted");
        require((*service.rpc(request(5, "resources/list")))["result"]["resources"].empty(), "Unexpected resources");
    });
    test("reject traversal and invalid slugs", [&] {
        for (const auto* bad : {"../private", "/etc/passwd", "name/other", "", ".", "-leading", "a b"})
            rejects([&] { service.run_tool("create_project", {{"name", bad}}); });
        rejects([&] { slug(true); });
    });
    test("create editable template and reject duplicate", [&] {
        const auto r = service.run_tool("create_project", {{"name", "ejemplo"}});
        require(r["project"] == "ejemplo", "Wrong project");
        require(Json::parse(read_file(config.projects / "ejemplo/project.json"))["scenes"].size() == 4, "Wrong template");
        rejects([&] { service.run_tool("create_project", {{"name", "ejemplo"}}); });
        require(service.run_tool("list_projects", Json::object())["projects"] == Json::array({"ejemplo"}), "Wrong project list");
    });
    test("editable project ETags and pre-save validation", [&] {
        const auto current=service.run_tool("read_project",{{"project","ejemplo"}});
        auto document=current["document"];
        document["scenes"][0]["background"]="#082033";
        auto saved=service.run_tool("save_project",{{"project","ejemplo"},
            {"etag",current["etag"]},{"document",document}});
        require(saved["validated"]==true && saved["etag"]!=current["etag"],"Valid edit rejected");
        rejects([&]{service.run_tool("save_project",{{"project","ejemplo"},
            {"etag",current["etag"]},{"document",document}});});
        auto invalid=document;
        invalid["canvas"]["height"]=999;
        rejects([&]{service.run_tool("save_project",{{"project","ejemplo"},
            {"etag",saved["etag"]},{"document",invalid}});});
        auto reread=service.run_tool("read_project",{{"project","ejemplo"}});
        require(reread["etag"]==saved["etag"],"Invalid save overwrote valid project");
        rejects([&]{service.run_tool("save_project",{{"project","ejemplo"},
            {"etag",saved["etag"]},{"document",Json::array()}});});
    });
    test("validate with real native compositor", [&] {
        const auto r = service.run_tool("validate_project", {{"project", "ejemplo"}});
        require(r["message"].get<std::string>().find("4 escenas") != std::string::npos, "Template not validated");
    });
    test("real preview through MCP", [&] {
        service.run_tool("preview_frame", {{"project", "ejemplo"}, {"second", 3}});
        const auto png = read_file(config.projects / "ejemplo/preview.png", 4000000);
        require(png.compare(0, 8, "\x89PNG\r\n\x1a\n", 8) == 0, "Preview is not PNG");
    });
    test("missing model and unknown project errors", [&] {
        rejects([&] { service.run_tool("synthesize_voice", {{"project", "ejemplo"}, {"model", "missing.onnx"}, {"text", "Hola"}}); });
        const auto r = service.rpc(request(6, "tools/call", {{"name", "render_video"}, {"arguments", {{"project", "missing"}}}}));
        require((*r)["result"]["isError"] == true, "Missing project not reported");
    });
    test("argument types bounds and unknown fields", [&] {
        rejects([&] { service.run_tool("preview_frame", {{"project", "ejemplo"}, {"second", true}}); });
        rejects([&] { service.run_tool("render_video", {{"project", "ejemplo"}, {"seconds", 61}}); });
        rejects([&] { service.run_tool("search_images", {{"query", "cat"}, {"limit", 2.5}}); });
        rejects([&] { service.run_tool("create_project", {{"name", "x"}, {"unexpected", "value"}}); });
        rejects([&] { service.run_tool("search_images", {{"query", "cat"}, {"provider", "invalid"}}); });
        rejects([&] { service.run_tool("search_images", {{"query", std::string("cat\0secret", 10)}}); });
    });
    test("only HTTPS and permitted exact hosts", [&] {
        for (const auto* url : {"http://commons.wikimedia.org/a", "https://127.0.0.1/a",
              "https://upload.wikimedia.org.evil/a", "https://upload.wikimedia.org@evil/a",
              "https://upload.wikimedia.org:444/a", "file:///etc/passwd"}) {
            rejects([&] { permitted_url(url, {"upload.wikimedia.org", "commons.wikimedia.org"}); });
        }
    });
    test("redirect revalidated before network", [&] {
        int calls = 0;
        rejects([&] {
            fetch_https("https://upload.wikimedia.org/a", {"upload.wikimedia.org"}, 100,
                [&](const std::string&, std::size_t) { ++calls; return HttpResponse{302, "", "https://127.0.0.1/private", ""}; });
        });
        require(calls == 1, "Redirect contacted forbidden host");
        rejects([&] {
            fetch_https("https://upload.wikimedia.org/a", {"upload.wikimedia.org"}, 2,
                [](const std::string&, std::size_t) { return HttpResponse{200, "abc", "", ""}; });
        });
    });
    test("Wikimedia metadata and HTML entity attribution", [&] {
        const auto items = service.run_tool("search_images", {{"query", "capybara"}, {"limit", 2}})["results"];
        require(items.size() == 1 && items[0]["downloadable"] == true, "Commons not found");
        require(items[0]["author"] == "Ana & José", "Attribution damaged");
        require(items[0]["license"] == "CC BY 4.0", "License lost");
    });
    test("Openverse discovery remains search only", [&] {
        MediaClient media([](const std::string& url, std::size_t) {
            require(url.find("api.openverse.org/v1/images/") != std::string::npos, "Wrong endpoint");
            return HttpResponse{200, R"({"results":[{"id":"1","title":"Capybara","creator":"Ana","license":"by","foreign_landing_url":"https://example.org/a"}]})", "", "application/json"};
        });
        const auto items = media.search("capybara", "openverse", 1);
        require(items.size() == 1 && items[0]["downloadable"] == false, "Wrong Openverse behavior");
    });
    test("image download and attribution sidecar", [&] {
        const auto png = read_file(source / "examples/json-compositor/assets/orbit.png", 4000000);
        MediaClient media([&](const std::string& url, std::size_t limit) {
            if (url.find("commons.wikimedia.org/") != std::string::npos) return commons_fixture(url, limit);
            return HttpResponse{200, png, "", "image/png"};
        });
        const auto directory = temp.path / "download";
        const auto result = media.download("File:Test.png", directory, "capybara");
        require(read_file(directory / "capybara.png", 4000000) == png, "Download changed");
        require(Json::parse(read_file(directory / "capybara.attribution.json"))["author"] == "Ana & José", "Sidecar lost");
        require(result["license"] == "CC BY 4.0", "License not returned");
        rejects([&] { media.download("File:Test.png", directory, "capybara"); });
    });
    test("unsupported downloaded format never published", [&] {
        MediaClient media([](const std::string& url, std::size_t limit) {
            if (url.find("commons.wikimedia.org/") != std::string::npos) return commons_fixture(url, limit);
            return HttpResponse{200, "<svg/>", "", "image/svg+xml"};
        });
        const auto directory = temp.path / "bad-download";
        rejects([&] { media.download("File:Test.png", directory, "bad"); });
        require(!fs::exists(directory / "bad.png") && !fs::exists(directory / "bad.attribution.json"), "Partial download published");
    });
    test("subprocess exact arguments and stdin without shell", [&] {
        const std::vector<std::string> input{"a b", "$(touch forbidden)", "quote\"test", "áéí", "trailing\\"};
        std::vector<std::string> args{self.u8string(), "--echo"}; args.insert(args.end(), input.begin(), input.end());
        const auto r = run_process(args, 10);
        require(r.exit_code == 0 && Json::parse(r.output) == Json(input), "Arguments changed");
        const auto stdin_result = run_process({self.u8string(), "--stdin"}, 10, "voz\ncon acentos á\n");
        require(Json::parse(stdin_result.output)["text"] == "voz\ncon acentos á\n", "stdin changed");
        const auto failure = run_process({self.u8string(), "--fail"}, 10);
        require(failure.exit_code == 7 && failure.output == "native failure", "Exit or stderr lost");
    });
    test("subprocess timeout", [&] {
        const auto r = run_process({self.u8string(), "--sleep"}, 1);
        require(r.timed_out && r.exit_code == 124, "Timeout not enforced");
    });
    test("subprocess timeout with continuous output and bounded capture", [&] {
        const auto r = run_process({self.u8string(), "--flood"}, 1);
        require(r.timed_out && r.exit_code == 124, "Output bypassed timeout");
        require(!r.output.empty() && r.output.size() <= 65536, "Unbounded process output");
    });
    test("stdio framing parse errors notifications and recovery", [&] {
        std::istringstream input(request(10, "ping").dump() + "\n" +
            R"({"jsonrpc":"2.0","method":"notifications/initialized"})" + "\ninvalid\n" +
            std::string(max_message_bytes + 1, 'a') + "\n" + request(11, "ping").dump() + "\n");
        std::ostringstream output; serve_stdio(service, input, output);
        std::istringstream lines(output.str()); std::string line; std::vector<Json> responses;
        while (std::getline(lines, line)) responses.push_back(Json::parse(line));
        require(responses.size() == 4, "Wrong response count");
        require(responses[0]["id"] == 10 && responses[3]["id"] == 11, "Request ordering lost");
        require(responses[1]["error"]["code"] == -32700 && responses[2]["error"]["code"] == -32700, "Parse errors missing");
    });
    test("voice attach and failed inference rollback with native fixture", [&] {
        auto native = config; native.engine = self;
        Service voices(native);
        for (const auto* filename : {"ok.onnx", "fail.onnx", "invalid.onnx"}) write_file(config.models / filename, "fixture");
        const auto r = voices.run_tool("synthesize_voice", {{"project", "ejemplo"}, {"model", "ok.onnx"},
                                                          {"text", "Hola José"}, {"start", 0.5}, {"track_name", "voz"}});
        require(fs::is_regular_file(fs::u8path(r["wav_path"].get<std::string>())), "Voice not attached");
        const auto before = read_file(config.projects / "ejemplo/project.json");
        const auto document = Json::parse(before);
        require(document["audio"].back()["src"] == "assets/voices/voz.wav", "Voice track missing");
        for (const auto* filename : {"fail.onnx", "invalid.onnx"}) {
            rejects([&] { voices.run_tool("synthesize_voice", {{"project", "ejemplo"}, {"model", filename},
                       {"text", "Hola"}, {"track_name", "failed"}}); });
            require(read_file(config.projects / "ejemplo/project.json") == before, "Failed inference changed project");
            require(!fs::exists(config.projects / "ejemplo/assets/voices/failed.wav") &&
                    !fs::exists(config.projects / "ejemplo/assets/voices/failed.txt"), "Partial voice files leaked");
        }
        rejects([&] { voices.run_tool("synthesize_voice", {{"project", "ejemplo"}, {"model", "ok.onnx"},
                                                           {"text", "Otra"}, {"track_name", "voz"}}); });
    });
#ifndef _WIN32
    test("reject symlink project output asset directory and model escapes", [&] {
        const auto outside = temp.path / "outside"; fs::create_directory(outside);
        fs::create_directory_symlink(outside, config.projects / "escape");
        rejects([&] { service.run_tool("validate_project", {{"project", "escape"}}); });
        fs::create_symlink(outside / "preview", config.projects / "ejemplo/preview.png.tmp");
        fs::remove(config.projects / "ejemplo/preview.png");
        fs::rename(config.projects / "ejemplo/preview.png.tmp", config.projects / "ejemplo/preview.png");
        rejects([&] { service.run_tool("preview_frame", {{"project", "ejemplo"}, {"second", 1}}); });
        service.run_tool("create_project", {{"name", "asset_escape"}});
        fs::remove_all(config.projects / "asset_escape/assets");
        fs::create_directory_symlink(outside, config.projects / "asset_escape/assets");
        rejects([&] { service.run_tool("download_commons_image", {{"project", "asset_escape"},
                    {"file_title", "File:Test.png"}, {"asset_name", "bad"}}); });
        rejects([&] { service.run_tool("synthesize_voice", {{"project", "asset_escape"}, {"model", "ok.onnx"}, {"text", "Hola"}}); });
        require(!fs::exists(outside / "voices"), "Rejected voice created an external directory");
        write_file(outside / "voice.onnx", "fixture");
        fs::create_symlink(outside / "voice.onnx", config.models / "escape.onnx");
        rejects([&] { service.run_tool("synthesize_voice", {{"project", "ejemplo"}, {"model", "escape.onnx"}, {"text", "Hola"}}); });
    });
#endif
    std::cout << passed << " tests passed; " << failed << " failed\n";
    return failed ? 1 : 0;
}
