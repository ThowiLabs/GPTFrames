#include "../compositor/reactive_layout.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
static void require(bool yes,const char* err){if(!yes)throw std::runtime_error(err);}
int main(){
 using gptframes_reactive::json;
 json words=json::array({
  {{"word","Primero"},{"start",1.0},{"end",1.6}},
  {{"word","Luego"},{"start",3.0},{"end",3.5}}
 });
 json root=json::array({{
  {"type","group"},{"x",100},{"y",100},{"width",640},{"height",200},
  {"direction","row"},{"gap",20},{"justify","center"},{"align","center"},
  {"reactive",true},
  {"children",json::array({
   {{"type","group"},{"width",240},{"height",140},{"cueIndex",0},
     {"children",json::array({{{"type","rect"},{"width",240},{"height",140}},
                              {{"type","text"},{"text","Primero"},{"width",200},{"height",60}}})}},
   {{"type","group"},{"width",240},{"height",140},{"cueIndex",1},
     {"children",json::array({{{"type","rect"},{"width",240},{"height",140}},
                              {{"type","text"},{"text","Luego"},{"width",200},{"height",60}}})}}
  })}
 }});
 int id=0;for(auto& node:root)gptframes_reactive::assign(node,words,id,0,6.);
 require(id==4,"Expected four leaf items");
 require(gptframes_reactive::solve(root,1.0).empty(),"Reveal before voice cue");
 auto first=gptframes_reactive::solve(root,1.16);
 require(first.size()==2,"Card background and text must appear atomically");
 require(first.at(0)["start"]==first.at(1)["start"],"A blank background could appear without its label");
 auto second=gptframes_reactive::solve(root,3.16);
 require(second.size()==4,"Second card should be visible on second voice cue");
 const double oldX=first.at(0)["x"].get<double>();
 const double newX=second.at(0)["x"].get<double>();
 require(oldX!=newX,"Layout did not reorganize when another card arrived");
 auto animated=gptframes_reactive::layout(root,3.18);
 require(animated.blend>0&&animated.blend<1,"Smooth positional interpolation absent");
 auto final=gptframes_reactive::layout(root,3.5);
 require(final.blend==1,"Reflow must finish");
 std::cout<<"PASS voice cues, atomic image/text group, live reflow, interpolation\n";
}
