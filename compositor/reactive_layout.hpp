#pragma once
// Runtime-resolved auto-layout with atomic, timed groups and semantic voice cues.
// Group children participate in layout only when visible. The audio event
// determines when each unit appears; backgrounds and labels share a timestamp.
#include "auto_layout.hpp"
#include <map>
#include <cmath>
namespace gptframes_reactive {
using json=nlohmann::json;
inline double number(const json& a,const char* key,double fallback){
 return gptframes_layout::quantity(a,key,fallback);
}
inline void assign(json& a,const json& words,int& id,double inherited,double parentEnd,
                   double audioOffset=.10){
 if(!a.is_object())throw std::invalid_argument("Invalid reactive node");
 double start=number(a,"start",inherited);
 if(a.contains("cueIndex")){
  if(!a.at("cueIndex").is_number_integer())throw std::invalid_argument("cueIndex must be integer");
  const int idx=a.at("cueIndex").get<int>();
  if(idx<0||idx>=static_cast<int>(words.size()))throw std::invalid_argument("cueIndex out of range");
  start=words[idx].at("start").get<double>()+audioOffset+number(a,"cueOffset",.04);
 }
 start=std::max(start,inherited);
 const double givenDuration=number(a,"duration",parentEnd-start);
 double end=std::min(parentEnd,start+givenDuration);
 if(!std::isfinite(start)||!std::isfinite(end)||start>=parentEnd||end<=start)
  throw std::invalid_argument("Reactive event exceeds scene bounds");
 a["start"]=start;
 a["duration"]=end-start;
 a.erase("cueIndex");a.erase("cueOffset");
 if(a.value("type",std::string{})=="group"){
  if(!a.contains("children")||!a["children"].is_array())throw std::invalid_argument("Group requires children");
  for(auto& child:a["children"])assign(child,words,id,start,end,audioOffset);
 }else a["__layoutId"]=id++;
}
inline json timed(const json& nodes,double now){
 json visible=json::array();
 for(const auto& node:nodes){
  const auto start=node.value("start",0.);
  const auto duration=node.value("duration",1e9);
  if(now+.000001<start||now>=start+duration)continue;
  json child=node;
  if(child.value("type",std::string{})=="group"){
   child["children"]=timed(child.at("children"),now);
   if(child["children"].empty())continue;
  }
  visible.push_back(std::move(child));
 }
 return visible;
}
using Placed=std::map<int,json>;
inline Placed solve(const json& root,double time){
 Placed positions;
 const auto visible=timed(root,time);
 if(visible.empty())return positions;
 for(auto& layer:gptframes_layout::flattened(visible))
  if(layer.contains("__layoutId"))positions.emplace(layer["__layoutId"].get<int>(),std::move(layer));
 return positions;
}
inline double lastEvent(const json& root,double time,double previous=0){
 for(const auto& a:root){
  const double at=a.value("start",0.);
  if(at<=time+.000001&&at>previous)previous=at;
  if(a.value("type",std::string{})=="group")
   previous=lastEvent(a.at("children"),time,previous);
 }
 return previous;
}
struct Resolved {Placed current,previous;double blend=1;};
inline Resolved layout(const json& root,double local,double transition=.28){
 Resolved out;
 out.current=solve(root,local);
 const double event=lastEvent(root,local);
 if(event>0&&local-event<transition){
  out.previous=solve(root,std::max(0.,event-.001));
  const double t=std::clamp((local-event)/transition,0.,1.);
  out.blend=t*t*(3-2*t);
 }
 return out;
}
} // namespace gptframes_reactive
