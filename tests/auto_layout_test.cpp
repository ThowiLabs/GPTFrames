#include "../compositor/auto_layout.hpp"
#include <cassert>
#include <cmath>
#include <iostream>

using gptframes_layout::json;
static bool close(double a,double b){return std::abs(a-b)<0.001;}
int main(){
 const json content=json::array({{
  {"type","group"},{"x",100},{"y",200},{"width",400},{"height",160},
  {"direction","row"},{"padding",20},{"gap",16},{"align","center"},
  {"children",json::array({
   {{"type","ring"},{"width",90},{"height",90},{"color","#FFCE77"}},
   {{"type","text"},{"fontSize",40},{"text","ALERTA"},{"color","#FFFFFF"},{"width",220},{"height",60}}
  })}
 }});
 const auto flat=gptframes_layout::flattened(content);
 assert(flat.size()==2);
 assert(flat[0]["x"].get<double>() < flat[1]["x"].get<double>());
 assert(close(flat[0]["y"].get<double>(),235));
 assert(close(flat[1]["y"].get<double>(),250));
 assert(flat[1]["textAlign"]=="center");
 // Overlay: all elements share the true geometric center of the parent.
 const auto over=gptframes_layout::flattened(json::array({{
  {"type","group"},{"x",200},{"y",50},{"width",300},{"height",300},
  {"direction","overlay"},{"align","center"},{"justify","center"},
  {"children",json::array({
    {{"type","ring"},{"width",150},{"height",150}},
    {{"type","text"},{"text","!"},{"fontSize",90},{"width",90},{"height",120}}
  })}
 }}));
 assert(close(over[0]["x"].get<double>()+75,350));
 assert(close(over[0]["y"].get<double>()+75,200));
 assert(close(over[1]["x"].get<double>()+45,350));
 assert(close(over[1]["y"].get<double>()+60,200));
 // Regression: editorial side bar must stay on the left edge, never
 // become a centered vertical stripe when regrouped into an overlay.
 const auto accent=gptframes_layout::flattened(json::array({{
  {"type","group"},{"x",200},{"y",50},{"width",300},{"height",160},
  {"direction","overlay"},{"align","center"},
  {"children",json::array({
   {{"type","rect"},{"width",300},{"height",160}},
   {{"type","rect"},{"width",10},{"height",160},{"offsetX",-145}}
  })}
 }}));
 assert(accent.size()==2);
 assert(close(accent[1]["x"].get<double>(),200));
 assert(close(accent[1]["y"].get<double>(),50));
 assert(close(accent[1]["width"].get<double>(),10));
 // Grid cell positions remain stable and ordering is deterministic.
 const auto grid=gptframes_layout::flattened(json::array({{
  {"type","group"},{"width",620},{"height",320},{"direction","grid"},
  {"columns",3},{"gap",10},{"padding",5},{"align","stretch"},
  {"children",json::array({
    {{"type","rect"}},{{"type","rect"}},{{"type","rect"}},{{"type","rect"}}
  })}
 }}));
 assert(grid.size()==4);
 assert(grid[0]["x"].get<double>() < grid[1]["x"].get<double>());
 assert(close(grid[0]["x"].get<double>(),grid[3]["x"].get<double>()));
 assert(grid[3]["y"].get<double>() > grid[0]["y"].get<double>());
 bool rejected=false;
 try{gptframes_layout::flattened(json::array({{{"type","group"},{"width",100},{"height",90},{"padding",51},{"children",json::array()}}}));}
 catch(const std::invalid_argument&){rejected=true;}
 assert(rejected);
 std::cout<<"PASS native row, overlay, grid, centering and bounds\n";
}
