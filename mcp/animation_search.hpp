#pragma once
#include "animation_catalog.hpp"
#include <map>
#include <cctype>
#include <sstream>
namespace gptframes::mcp {
inline std::string animation_fold(std::string in){
    static const std::vector<std::pair<std::string,std::string>> folding={
      {"Á","a"},{"É","e"},{"Í","i"},{"Ó","o"},{"Ú","u"},{"Ü","u"},{"Ñ","n"},
      {"á","a"},{"é","e"},{"í","i"},{"ó","o"},{"ú","u"},{"ü","u"},{"ñ","n"}
    };
    for(const auto& replacement:folding){
      auto pos=in.find(replacement.first);
      while(pos!=std::string::npos){
        in.replace(pos,replacement.first.size(),replacement.second);
        pos=in.find(replacement.first,pos+replacement.second.size());
      }
    }
    for(char& c:in){
      if(static_cast<unsigned char>(c)<128)c=static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return in;
}
inline std::string safe_string(const Json& obj,const char* key){
    if(!obj.contains(key)||!obj[key].is_string())return {};
    return obj[key].get<std::string>();
}
inline bool animation_matches_filter(const Json& item,const Json& filter){
    for(const auto& field:{"category","type"}){
      auto value=safe_string(filter,field);
      if(!value.empty()&&animation_fold(safe_string(item,field))!=animation_fold(value))
        return false;
    }
    for(const auto& field:{"tag","motion"}){
      auto value=safe_string(filter,field);
      if(value.empty())continue;
      auto key=field==std::string("tag")?"tags":"motion";
      if(!item.contains(key)||!item[key].is_array())return false;
      bool exists=false;
      for(const auto& word:item[key])
        if(word.is_string()&&animation_fold(word.get<std::string>())==animation_fold(value)){
          exists=true;break;
        }
      if(!exists)return false;
    }
    return true;
}
inline Json list_animations_filtered(const fs::path& root,const Json& filters){
    const auto data=animation_library(root);
    Json results=Json::array();
    const int limit=filters.value("limit",100);
    for(const auto& item:data.at("presets")){
        if(!animation_matches_filter(item,filters))continue;
        if(static_cast<int>(results.size())<limit)results.push_back(item);
    }
    int total=0;
    for(const auto& item:data.at("presets"))
      if(animation_matches_filter(item,filters))++total;
    return {{"collection","animaciones"},{"total",total},{"returned",results.size()},
            {"presets",std::move(results)}};
}
inline Json animation_facets(const fs::path& root){
    auto data=animation_library(root);
    std::map<std::string,std::map<std::string,int>> counts;
    for(const auto& item:data.at("presets")){
      for(const auto& field:{"category","type"}){
        const auto value=safe_string(item,field);
        if(!value.empty())counts[field][value]++;
      }
      for(const auto& field:{"tags","motion"}){
        if(item.contains(field)&&item[field].is_array()){
          std::set<std::string> unique;
          for(const auto& tag:item[field])
            if(tag.is_string())unique.insert(tag.get<std::string>());
          for(const auto& word:unique)counts[field][word]++;
        }
      }
    }
    Json result;
    for(const auto& [kind,values]:counts){
      Json arr=Json::array();
      for(const auto& [name,total]:values)
        arr.push_back({{"value",name},{"count",total}});
      result[kind]=std::move(arr);
    }
    result["total"]=data.at("presets").size();
    return result;
}
inline std::vector<std::string> split_search_tokens(const std::string& query){
    std::string folded=animation_fold(query);
    for(char& c:folded)
      if(!(std::isalnum(static_cast<unsigned char>(c))||
           static_cast<unsigned char>(c)>127))c=' ';
    std::istringstream stream(folded);
    std::vector<std::string> out;
    std::string token;
    while(stream>>token){
      if(token.size()<2)continue;
      if(std::find(out.begin(),out.end(),token)==out.end())out.push_back(token);
      if(out.size()>12)throw std::invalid_argument("Demasiados términos de búsqueda");
    }
    return out;
}
inline int search_score(const Json& item,const std::vector<std::string>& tokens,const std::string& phrase){
    const std::string name=animation_fold(safe_string(item,"name"));
    const std::string id=animation_fold(safe_string(item,"id"));
    const std::string category=animation_fold(safe_string(item,"category"));
    const std::string type=animation_fold(safe_string(item,"type"));
    const std::string desc=animation_fold(safe_string(item,"description"));
    const std::string style=animation_fold(safe_string(item,"style"));
    std::vector<std::string> tags,motions,usage,aliases;
    for(auto [key,into]:std::vector<std::pair<const char*,std::vector<std::string>*>>{
      {"tags",&tags},{"motion",&motions},{"usage",&usage},{"aliases",&aliases}}){
      if(item.contains(key)&&item[key].is_array())
        for(const auto& word:item[key])if(word.is_string())
          into->push_back(animation_fold(word.get<std::string>()));
    }
    int sum=0;
    if(!phrase.empty()){
      if(id==phrase)sum+=120;
      if(name==phrase)sum+=90;
      if(name.find(phrase)!=std::string::npos)sum+=45;
      if(std::find(tags.begin(),tags.end(),phrase)!=tags.end())sum+=40;
    }
    for(const auto& token:tokens){
      int best=0;
      auto check=[&](const std::string& text,int importance){
        if(text==token)best=std::max(best,importance+12);
        else if(text.find(token)!=std::string::npos)best=std::max(best,importance);
      };
      check(name,24);check(id,28);check(type,15);
      check(category,12);check(desc,6);check(style,5);
      for(const auto& value:tags)check(value,20);
      for(const auto& value:motions)check(value,16);
      for(const auto& value:usage)check(value,10);
      for(const auto& value:aliases)check(value,19);
      if(best==0)return 0; // All query words must match.
      sum+=best;
    }
    return sum;
}
inline Json search_animations(const fs::path& root,const Json& request){
    const std::string query=request.at("query").get<std::string>();
    const auto tokens=split_search_tokens(query);
    if(tokens.empty())throw std::invalid_argument("La búsqueda necesita palabras significativas");
    const auto catalog=animation_library(root);
    struct Hit{int score;int number;Json item;};
    std::vector<Hit> matches;
    for(const auto& item:catalog.at("presets")){
      if(!animation_matches_filter(item,request))continue;
      const int score=search_score(item,tokens,animation_fold(query));
      if(score>0)matches.push_back({score,item.value("number",999),item});
    }
    std::sort(matches.begin(),matches.end(),[](const Hit& a,const Hit& b){
      if(a.score!=b.score)return a.score>b.score;
      return a.number<b.number;
    });
    const int limit=request.value("limit",12);
    Json hits=Json::array();
    for(const auto& match:matches){
      if(static_cast<int>(hits.size())>=limit)break;
      auto record=match.item;
      record["score"]=match.score;
      hits.push_back(std::move(record));
    }
    return {{"query",query},{"total",matches.size()},{"returned",hits.size()},
            {"results",std::move(hits)},{"collection","animaciones"}};
}
} // namespace gptframes::mcp
