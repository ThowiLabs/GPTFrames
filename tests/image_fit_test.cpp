#include "../compositor/image_fit.hpp"
#include <cassert>
#include <cmath>
int main(){
 using gptframes_image_fit::map;
 auto side=map("contain",10,50,200,100,100,100);
 assert(!side.visible); // square source remains centered: left letterbox
 auto center=map("contain",100,50,200,100,100,100);
 assert(center.visible&&std::abs(center.x-49.5)<.001&&std::abs(center.y-49.5)<.001);
 auto top=map("contain",50,1,100,200,100,100);
 assert(!top.visible);
 auto cover=map("cover",0,50,200,100,100,100);
 assert(cover.visible&&cover.y>24 && cover.y<26); // tall source vertically cropped
 auto stretched=map("stretch",100,50,200,100,100,100);
 assert(stretched.visible&&std::abs(stretched.x-49.5)<.001);
 assert(gptframes_image_fit::visible_fraction_cover(200,100,100,100)==.5);
 assert(gptframes_image_fit::visible_fraction_cover(200,100,200,100)==1.);
}
