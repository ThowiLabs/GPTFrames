#pragma once
// Reusable image fit geometry. No network, no mutation, no browser dependency.
#include <algorithm>
#include <cmath>
#include <string_view>
namespace gptframes_image_fit {
struct Sample {
 double x=0,y=0; // position in source pixel units (half-pixel offset)
 bool visible=true;
};
inline Sample map(std::string_view fit,double u,double v,double boxW,double boxH,int srcW,int srcH){
 if(boxW<=0||boxH<=0||srcW<=0||srcH<=0)return {0,0,false};
 if(fit=="contain"){
  const double factor=std::min(boxW/srcW,boxH/srcH);
  const double displayedW=srcW*factor,displayedH=srcH*factor;
  const double left=(boxW-displayedW)/2,top=(boxH-displayedH)/2;
  // Padding remains transparent, preserving image proportions.
  if(u<left||v<top||u>=left+displayedW||v>=top+displayedH)return {0,0,false};
  return {(u-left)/factor-.5,(v-top)/factor-.5,true};
 }
 if(fit=="cover"){
  const double factor=std::max(boxW/srcW,boxH/srcH);
  const double shownW=boxW/factor,shownH=boxH/factor;
  return {(srcW-shownW)/2+u/factor-.5,(srcH-shownH)/2+v/factor-.5,true};
 }
 // stretch is explicitly requested, and may distort the source aspect ratio.
 return {u/boxW*srcW-.5,v/boxH*srcH-.5,true};
}
inline double visible_fraction_cover(double boxW,double boxH,int srcW,int srcH){
 if(boxW<=0||boxH<=0||srcW<=0||srcH<=0)return 0;
 const double sourceAspect=double(srcW)/srcH,boxAspect=boxW/boxH;
 return std::min(sourceAspect,boxAspect)/std::max(sourceAspect,boxAspect);
}
} // namespace gptframes_image_fit
