#include "../compositor/motion_symbols.hpp"
#include "../compositor/chroma_key.hpp"
#include <cassert>
int main(){
 using namespace gptframes_symbols;
 assert(supported("phone"));
 assert(supported("lock"));
 assert(supported("angry-person"));
 assert(mask("check",.65,.43,0)==0);
 assert(mask("check",.65,.43,1)>0);
 gptframes_chroma::Key key{true,0,255,0,.12,.1};
 assert(gptframes_chroma::alpha(key,0,255,0)<.001);
 assert(gptframes_chroma::alpha(key,255,0,0)>.99);
}
