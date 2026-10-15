#pragma once
// Small native motion symbols used when external Lottie assets are not needed.
// All coordinates are normalized, deterministic and safe for multithreaded CPU render.
#include <algorithm>
#include <cmath>
#include <string>
namespace gptframes_symbols {
constexpr double pi=3.14159265358979323846;
inline double clamp(double v){return std::clamp(v,0.,1.);}
inline double length(double x,double y){return std::hypot(x,y);}
inline double segment(double x,double y,double x1,double y1,double x2,double y2,double thickness){
 const double dx=x2-x1,dy=y2-y1,den=dx*dx+dy*dy;
 const double a=den>0?clamp(((x-x1)*dx+(y-y1)*dy)/den):0;
 return length(x-x1-a*dx,y-y1-a*dy)<=thickness;
}
inline bool outline(double x,double y,double x1,double y1,double x2,double y2,double t){
 return x>=x1&&x<=x2&&y>=y1&&y<=y2&&
 (x<x1+t||x>x2-t||y<y1+t||y>y2-t);
}
inline double check(double x,double y,double progress,double thickness=.035){
 // Animate the tick as one continuous stroke in two segments.
 const double a=clamp(progress*1.9);
 const double b=clamp((progress-.43)*1.75);
 return segment(x,y,.30,.52,.30+.17*a,.52+.13*a,thickness) ||
        (b>0&&segment(x,y,.47,.65,.47+.28*b,.65-.34*b,thickness));
}
inline double mask(const std::string& symbol,double x,double y,double progress){
 if(!std::isfinite(x)||!std::isfinite(y)||!std::isfinite(progress))return 0;
 progress=clamp(progress);
 if(symbol=="check")return check(x,y,progress,.045);
 if(symbol=="phone"){
  const bool device=outline(x,y,.20,.05,.80,.95,.038);
  const bool earpiece=segment(x,y,.43,.115,.57,.115,.018);
  const bool home=length(x-.5,y-.88)<.025;
  const double tick=clamp((progress-.40)*2.2);
  return device||earpiece||home||(tick>0&&check(x,y,tick,.04));
 }
 if(symbol=="lock"){
  const bool body=(x>=.22&&x<=.78&&y>=.43&&y<=.88);
  const bool keyhole=(length(x-.50,y-.62)<.046 ||
                      (x>.48&&x<.52&&y>.62&&y<.77));
  const double close=clamp((progress-.15)*1.7);
  // The right shackle gradually moves into the locked position.
  const double right=.84-.18*close;
  const bool shackle=segment(x,y,.33,.47,.33,.31,.039)||
       segment(x,y,.33,.31,.49,.16,.039)||
       segment(x,y,.49,.16,right,.30,.039)||
       segment(x,y,right,.30,right,.43*close,.039);
  return body&&!keyhole || shackle;
 }
 if(symbol=="angry-person"){
  const bool head=length((x-.5)/.34,(y-.31)/.25)<=1.0;
  const bool torso=(x>=.18&&x<=.82&&y>=.70&&y<=.97&&
                  length((x-.50)/.50,(y-.82)/.43)<=1.);
  const bool face=length((x-.5)/.30,(y-.31)/.21)<=1.;
  const bool eye1=length(x-.38,y-.31)<.022,eye2=length(x-.62,y-.31)<.022;
  const bool brow1=segment(x,y,.28,.21,.43,.28,.020);
  const bool brow2=segment(x,y,.57,.28,.72,.21,.020);
  const bool mouth=(segment(x,y,.41,.47,.50,.44,.023)||
                    segment(x,y,.50,.44,.59,.47,.023));
  return torso||(head&&!face)||eye1||eye2||brow1||brow2||mouth;
 }
 return 0;
}
inline bool supported(const std::string& symbol){
 return symbol=="phone"||symbol=="check"||symbol=="lock"||symbol=="angry-person";
}
} // namespace gptframes_symbols
