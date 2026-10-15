#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <vector>
namespace gptframes_rgba {
// Compose a browser RGBA surface above native BGR in source-over order.
inline void over(std::vector<unsigned char>& bgr,int width,int height,
                 const std::vector<unsigned char>& rgba,int rw,int rh,
                 int left,int top,double opacity=1.0){
 if(width<1||height<1||rw<1||rh<1||!std::isfinite(opacity)||
    opacity<0||opacity>1||bgr.size()!=size_t(width)*height*3||
    rgba.size()!=size_t(rw)*rh*4)
  throw std::invalid_argument("Invalid RGBA overlay dimensions");
 for(int y=std::max(0,top);y<std::min(height,top+rh);++y){
  for(int x=std::max(0,left);x<std::min(width,left+rw);++x){
   const size_t src=(size_t(y-top)*rw+(x-left))*4;
   const size_t dst=(size_t(y)*width+x)*3;
   const unsigned alpha=static_cast<unsigned>(std::lround(double(rgba[src+3])*opacity));
   if(!alpha)continue;
   const unsigned inv=255-alpha;
   bgr[dst+0]=static_cast<unsigned char>((bgr[dst+0]*inv+rgba[src+2]*alpha+127)/255);
   bgr[dst+1]=static_cast<unsigned char>((bgr[dst+1]*inv+rgba[src+1]*alpha+127)/255);
   bgr[dst+2]=static_cast<unsigned char>((bgr[dst+2]*inv+rgba[src+0]*alpha+127)/255);
  }
 }
}
}
