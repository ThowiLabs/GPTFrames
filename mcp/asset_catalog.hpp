#pragma once
#include "common.hpp"
#include <set>
namespace gptframes::mcp {
// Read-only, curated HTML/CSS designs; never execute caller-provided markup.
inline Json asset_catalog(const fs::path& root){
    const fs::path path=root/"assets"/"preset-catalog.json";
    if(!fs::is_regular_file(path)||fs::is_symlink(path)||fs::file_size(path)>512*1024)
        throw std::runtime_error("Catálogo de presets local no disponible");
    Json doc=Json::parse(read_file(path,512*1024));
    if(!doc.is_object()||doc.value("version",0)!=1||
       !doc.contains("presets")||!doc["presets"].is_array()||
       doc["presets"].size()>128)
        throw std::runtime_error("Catálogo inválido");
    std::set<std::string> ids;
    for(const auto& item:doc["presets"]){
        if(!item.is_object()||!item.contains("id")||!item["id"].is_string()||
           !item.contains("number")||!item["number"].is_number_integer()||
           !item.contains("name")||!item["name"].is_string()||
           !item.contains("description")||!item["description"].is_string()||
           !item.contains("category")||!item["category"].is_string()||
           !item.contains("style")||!item["style"].is_string()||
           !item.contains("preset_file")||!item["preset_file"].is_string())
            throw std::runtime_error("Datos inválidos en preset");
        const auto id=item["id"].get<std::string>();
        if(id.empty()||id.size()>64||!std::all_of(id.begin(),id.end(),
                [](unsigned char ch){return ascii_alnum(ch)||ch=='-';})||!ids.insert(id).second)
            throw std::runtime_error("Identificador de preset duplicado/inválido");
        const auto relative=fs::path(item["preset_file"].get<std::string>());
        if(relative.is_absolute()||relative.extension()!=".html"||
           relative.parent_path()!=fs::path("assets/presets")||
           relative.filename()!=id+".html")
            throw std::runtime_error("Ruta de preset fuera del catálogo");
        const auto asset=root/relative;
        if(!fs::is_regular_file(asset)||fs::is_symlink(asset)||
           fs::file_size(asset)>1024*1024)
            throw std::runtime_error("Recurso de preset no verificado");
    }
    return doc;
}
inline Json list_asset_presets(const fs::path& root,const std::string& category){
    auto catalogue=asset_catalog(root);
    Json found=Json::array();
    for(const auto& item:catalogue["presets"])
        if(category.empty()||item["category"]==category)found.push_back(item);
    return {{"total",found.size()},{"presets",std::move(found)},
            {"note","Original CSS assets, browser backend, no external HTML execution."}};
}
inline Json describe_asset_preset(const fs::path& root,const std::string& id){
    const auto catalogue=asset_catalog(root);
    for(const auto& item:catalogue["presets"])
        if(item["id"]==id)return item;
    throw std::invalid_argument("Preset no encontrado");
}
} // namespace gptframes::mcp
