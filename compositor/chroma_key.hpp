#pragma once
// Chroma key for RGBA stills and future decoded video frames.
// Relative Euclidean RGB-distance provides a soft controllable edge.
#include <algorithm>
#include <cmath>
namespace gptframes_chroma {
struct Key {bool enabled=false;unsigned char r=0,g=255,b=0;double tolerance=.20,softness=.08;};
inline double alpha(const Key& k,double r,double g,double b){
 if(!k.enabled)return 1.;
 const double dr=(r-k.r)/255.,dg=(g-k.g)/255.,db=(b-k.b)/255.;
 const double d=std::sqrt((dr*dr+dg*dg+db*db)/3.);
 if(k.softness<=.000001)return d>k.tolerance?1.:0.;
 return std::clamp((d-k.tolerance)/k.softness,0.,1.);
}
}
