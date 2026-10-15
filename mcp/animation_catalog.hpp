#pragma once
#include "common.hpp"
#include <set>
namespace gptframes::mcp {
inline Json animation_library(const fs::path& root){
    auto file=root/"animaciones"/"catalogo.json";
    if(!fs::is_regular_file(file)||fs::is_symlink(file)||fs::file_size(file)>262144)
        throw std::runtime_error("Biblioteca Animaciones no disponible");
    Json doc=Json::parse(read_file(file,262144));
    if(!doc.is_object()||doc.value("version",0)!=1||
       !doc.contains("presets")||!doc["presets"].is_array()||
       doc["presets"].size()>128)
        throw std::runtime_error("Estructura de animaciones inválida");
    std::set<std::string> ids;
    for(const auto& a:doc["presets"]){
        if(!a.is_object()||!a.contains("id")||!a["id"].is_string()||
           !a.contains("number")||!a["number"].is_number_integer()||
           !a.contains("name")||!a["name"].is_string()||
           !a.contains("description")||!a["description"].is_string()||
           !a.contains("category")||!a["category"].is_string()||
           !a.contains("style")||!a["style"].is_string()||
           !a.contains("preset_file")||!a["preset_file"].is_string())
            throw std::runtime_error("Registro de animación inválido");
        auto id=a["id"].get<std::string>();
        if(id.empty()||id.size()>64||
           !std::all_of(id.begin(),id.end(),[](unsigned char c){return ascii_alnum(c)||c=='-';})||
           !ids.insert(id).second)throw std::runtime_error("ID de animación inválido");
        auto relative=fs::path(a["preset_file"].get<std::string>());
        if(relative.parent_path()!=fs::path("animaciones/presets")||
           relative.filename()!=id+".html"||relative.is_absolute())
            throw std::runtime_error("Ruta de animación inválida");
        auto path=root/relative;
        if(!fs::is_regular_file(path)||fs::is_symlink(path)||
           fs::file_size(path)>1024*1024)
            throw std::runtime_error("Archivo de animación ausente o no verificado");
    }
    // Discover new local assets on every read. Unknown HTML is indexed but not
    // executable: it requires a reviewed catalog registration before rendering.
    std::error_code ec;
    const auto presets=root/"animaciones"/"presets";
    if(fs::is_directory(presets,ec) && !fs::is_symlink(presets,ec)){
        std::vector<std::string> discovered;
        for(fs::directory_iterator it(presets,fs::directory_options::skip_permission_denied,ec),end;
            !ec && it!=end && discovered.size()<128;it.increment(ec)){
            const auto& entry=*it;
            if(entry.is_symlink(ec)||!entry.is_regular_file(ec)||
               entry.path().extension()!=".html"||entry.file_size(ec)>1024*1024)continue;
            auto id=entry.path().stem().u8string();
            if(id.empty()||id.size()>64||
               !std::all_of(id.begin(),id.end(),[](unsigned char c){return ascii_alnum(c)||c=='-';})||
               ids.count(id))continue;
            discovered.push_back(id);
        }
        std::sort(discovered.begin(),discovered.end());
        for(const auto& id:discovered){
            ids.insert(id);
            doc["presets"].push_back({{"number",doc["presets"].size()+1},
                {"id",id},{"name",id},{"category","Sin clasificar"},{"type","animación"},
                {"description","HTML detectado; requiere registro y revisión antes de renderizar."},
                {"style","pendiente"},{"preset_file","animaciones/presets/"+id+".html"},
                {"status","detected-unverified"},{"renderable",false},
                {"tags",Json::array({"nuevo","sin revisar"})},{"motion",Json::array()}});
        }
    }
    doc["total"]=doc["presets"].size();
    return doc;
}
inline Json list_animations(const fs::path& root,const std::string& category){
    auto data=animation_library(root);
    Json found=Json::array();
    for(const auto& a:data["presets"])
        if(category.empty()||a["category"]==category)found.push_back(a);
    return {{"collection","animaciones"},{"total",found.size()},{"presets",found}};
}
inline Json describe_animation(const fs::path& root,const std::string& id){
    auto data=animation_library(root);
    for(const auto& a:data["presets"])if(a["id"]==id)return a;
    throw std::invalid_argument("Animación no encontrada");
}
}
