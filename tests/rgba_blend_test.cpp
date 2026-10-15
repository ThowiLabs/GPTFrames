#include "../compositor/rgba_blend.hpp"
#include <cassert>
int main(){
 std::vector<unsigned char> image(4*4*3,0);
 for(size_t i=0;i<image.size();i+=3){image[i]=30;image[i+1]=60;image[i+2]=90;}
 std::vector<unsigned char> overlay={255,0,0,0, 255,0,0,255,
                                     0,255,0,128, 0,0,255,255};
 gptframes_rgba::over(image,4,4,overlay,2,2,1,1);
 auto px=[&](int x,int y,int c){return image[(size_t(y)*4+x)*3+c];};
 assert(px(1,1,2)==90);            // Transparent does not alter BGR.
 assert(px(2,1,2)==255);           // Opaque red layer.
 assert(px(2,1,0)==0);
 assert(px(1,2,1)>145&&px(1,2,1)<190); // Half-green alpha.
 assert(px(3,3,0)==30);            // Clipped layer leaves background untouched.
 gptframes_rgba::over(image,4,4,overlay,2,2,-1,-1,.5);
 assert(px(3,3,2)==90);
 return 0;
}
