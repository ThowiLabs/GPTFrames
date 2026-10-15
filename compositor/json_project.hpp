#pragma once
// Compositor declarativo de escenas C++17. Coordenadas de diseño: 720 x 1280.
// Salida MP4 dimensionada mediante output::resize, sin navegador.
#include <filesystem>
#include <memory>
#include <cstdint>
#include <cstring>
#include <map>
#include <set>
#include <sstream>
#include "../vendor/browser/json.hpp"
#ifndef HFC_WITH_BROWSER_CAPTURE
#define STB_IMAGE_IMPLEMENTATION
#include "../vendor/browser/stb_image.h"
#endif

namespace project {
using json=nlohmann::json;
namespace fs=std::filesystem;
constexpr double pi=3.14159265358979323846;
struct Color {unsigned char r=0,g=0,b=0;};
struct Pose {double x=0,y=0,w=200,h=100,opacity=1,scale=1,rotation=0;};
struct Key {double at;Pose pose;std::string easing="linear";};
struct Image {int width=0,height=0;std::vector<unsigned char> rgba;};
struct Layer {
 std::string type,text,file;
 Color color{240,247,252},stroke{240,247,252};
 Pose base;
 int fontSize=62;bool bold=true,underline=false;
 double from=0,duration=0,strokeWidth=0,radius=0;
 Color shadowColor{0,0,0}; double shadowOpacity=0,shadowX=5,shadowY=8;
 std::vector<Key> keys;
 std::shared_ptr<Image> image;
};
struct Transition {std::string type="cut";double duration=0;};
struct Scene {
 double start=0,duration=1;
 Color background{6,17,33},gradient{6,17,33};
 bool hasGradient=false;
 Transition transition;
 std::vector<Layer> layers;
};
struct Audio {fs::path file;double start=0,gain=1;};
struct Config {
 std::vector<Scene> scenes;
 std::vector<Audio> audio;
 int width=720,height=1280,fps=24;
 double duration=0;
};
inline double bounded(double n,double low,double high,const std::string& label){
 if(!std::isfinite(n)||n<low||n>high)throw std::runtime_error("JSON "+label+" fuera de rango");
 return n;
}
inline double num(const json& j,const char*name,double defaultVal,double low=-10000,double high=10000){
 if(!j.contains(name))return defaultVal;
 if(!j.at(name).is_number())throw std::runtime_error(std::string("JSON ")+name+" requiere número");
 return bounded(j.at(name).get<double>(),low,high,name);
}
inline std::string str(const json&j,const char*name,const std::string& fallback=""){
 if(!j.contains(name))return fallback;
 if(!j.at(name).is_string())throw std::runtime_error(std::string("JSON ")+name+" requiere texto");
 return j.at(name).get<std::string>();
}
inline bool flag(const json&j,const char*name,bool def){
 if(!j.contains(name))return def;
 if(!j.at(name).is_boolean())throw std::runtime_error(std::string("JSON ")+name+" requiere booleano");
 return j.at(name).get<bool>();
}
inline Color parseColor(const std::string&s) {
 if(s.size()!=7||s[0]!='#')throw std::runtime_error("Color esperado #RRGGBB: "+s);
 auto hx=[](char c)->int{if(c>='0'&&c<='9')return c-'0';if(c>='a'&&c<='f')return c-'a'+10;if(c>='A'&&c<='F')return c-'A'+10;throw std::runtime_error("Color hexadecimal inválido");};
 return {static_cast<unsigned char>(hx(s[1])*16+hx(s[2])),static_cast<unsigned char>(hx(s[3])*16+hx(s[4])),static_cast<unsigned char>(hx(s[5])*16+hx(s[6]))};
}
inline Color colorField(const json&j,const char*key,Color def){
 return j.contains(key)?parseColor(str(j,key)):def;
}
inline fs::path safeAsset(const fs::path& root,const std::string&relative){
 fs::path p(relative);
 if(relative.empty()||relative.size()>240||p.is_absolute()||p.has_root_name())
   throw std::runtime_error("Ruta de recurso inválida");
 for(const auto&part:p)if(part==".."||part==".")throw std::runtime_error("Ruta de recurso fuera del proyecto");
 auto base=fs::weakly_canonical(root);
 auto path=fs::weakly_canonical(root/p);
 auto first=std::mismatch(base.begin(),base.end(),path.begin(),path.end());
 if(first.first!=base.end())throw std::runtime_error("Enlace de recurso fuera del proyecto");
 if(!fs::is_regular_file(path))throw std::runtime_error("Recurso no encontrado: "+path.string());
 return path;
}
inline std::shared_ptr<Image> readImage(const fs::path&p) {
 auto sz=fs::file_size(p);
 if(sz>25*1024*1024)throw std::runtime_error("Imagen mayor de 25MB");
 int w=0,h=0,c=0;
 if(!stbi_info(p.string().c_str(),&w,&h,&c)||w<1||h<1||w>4096||h>4096||static_cast<int64_t>(w)*h>10000000)
   throw std::runtime_error("Imagen no válida o demasiado grande");
 int decodedW=0,decodedH=0;
 unsigned char*rgba=stbi_load(p.string().c_str(),&decodedW,&decodedH,&c,4);
 if(!rgba)throw std::runtime_error("No se pudo decodificar imagen PNG/JPEG");
 auto result=std::make_shared<Image>();
 result->width=decodedW;result->height=decodedH;
 result->rgba.assign(rgba,rgba+size_t(decodedW)*decodedH*4);
 stbi_image_free(rgba);return result;
}
inline Pose readPose(const json& j,Pose defaults){
 defaults.x=num(j,"x",defaults.x,-3000,3000);
 defaults.y=num(j,"y",defaults.y,-3000,3000);
 defaults.w=num(j,"width",defaults.w,1,3500);
 defaults.h=num(j,"height",defaults.h,1,3500);
 defaults.opacity=num(j,"opacity",defaults.opacity,0,1);
 defaults.scale=num(j,"scale",defaults.scale,0.01,12);
 defaults.rotation=num(j,"rotation",defaults.rotation,-1440,1440);
 return defaults;
}
inline bool nearTime(double a,double b){return std::abs(a-b)<0.000001;}
inline Config load(const std::string&filename){
 fs::path path(filename);
 if(!fs::is_regular_file(path)||fs::file_size(path)>1024*1024)throw std::runtime_error("JSON ausente o mayor de 1 MB");
 std::ifstream in(path,std::ios::binary);
 json doc=json::parse(in);
 if(!doc.is_object()||doc.value("version",0)!=1)throw std::runtime_error("Se requiere version:1");
 Config out;
 if(doc.contains("canvas")){
  const auto& c=doc.at("canvas");
  if(!c.is_object())throw std::runtime_error("canvas debe ser objeto");
  out.width=static_cast<int>(num(c,"width",720,144,3840));
  out.height=static_cast<int>(num(c,"height",1280,256,6840));
  // Diseñamos a 720x1280; se admite salida reescalada.
  if(out.width*16!=out.height*9 || out.width%16!=0 || out.height%16!=0)
    throw std::runtime_error("Canvas vertical 9:16 alineado a 16 píxeles");
 }
 out.fps=static_cast<int>(num(doc,"fps",24,1,60));
 if(!doc.contains("scenes")||!doc.at("scenes").is_array()||doc.at("scenes").empty()||doc.at("scenes").size()>64)
   throw std::runtime_error("Se requieren 1..64 escenas");
 auto root=path.parent_path();if(root.empty())root=".";
 std::map<std::string,std::shared_ptr<Image>> images;
 for(const auto& item:doc.at("scenes")){
  if(!item.is_object())throw std::runtime_error("Cada escena debe ser objeto");
  Scene s;
  s.start=num(item,"start",out.duration,0,240);
  s.duration=num(item,"duration",1,.05,240);
  if(!nearTime(s.start,out.duration))throw std::runtime_error("Escenas deben ser contiguas y ordenadas");
  s.background=colorField(item,"background",s.background);
  s.gradient=colorField(item,"gradient",s.background);
  s.hasGradient=item.contains("gradient");
  if(item.contains("transition")){
    auto& tr=item.at("transition");
    if(!tr.is_object())throw std::runtime_error("transition debe ser objeto");
    s.transition.type=str(tr,"type","cut");
    if(s.transition.type!="cut"&&s.transition.type!="fade"&&s.transition.type!="wipe"&&s.transition.type!="iris"&&s.transition.type!="slide")
      throw std::runtime_error("Transición desconocida: "+s.transition.type);
    s.transition.duration=num(tr,"duration",.45,0,4);
    if(s.transition.duration>s.duration||s.transition.duration>(out.scenes.empty()?s.duration:out.scenes.back().duration))
      throw std::runtime_error("Transición más larga que la escena");
  }
  if(!item.contains("layers")||!item.at("layers").is_array()||item.at("layers").size()>100)
    throw std::runtime_error("layers debe ser arreglo de máximo 100");
  for(const auto& l:item.at("layers")){
   if(!l.is_object())throw std::runtime_error("Capa inválida");
   Layer layer;
   layer.type=str(l,"type");
   static const std::set<std::string> types={"rect","ellipse","text","image","line","ring","glow","grid"};
   if(!types.count(layer.type))throw std::runtime_error("Capa desconocida: "+layer.type);
   layer.base=readPose(l,layer.base);
   layer.color=colorField(l,"color",layer.color);
   layer.stroke=colorField(l,"stroke",layer.color);
   layer.strokeWidth=num(l,"strokeWidth",0,0,100);
   layer.radius=num(l,"radius",0,0,500);
   layer.shadowColor=colorField(l,"shadowColor",{0,0,0});
   layer.shadowOpacity=num(l,"shadowOpacity",0,0,1);
   layer.shadowX=num(l,"shadowX",5,-150,150);
   layer.shadowY=num(l,"shadowY",8,-150,150);
   layer.from=num(l,"start",0,0,s.duration);
   layer.duration=num(l,"duration",s.duration-layer.from,.01,s.duration);
   if(layer.from+layer.duration>s.duration+.001)throw std::runtime_error("Capa excede duración de escena");
   if(layer.type=="text"){
    layer.text=str(l,"text");
    if(layer.text.size()>1200)throw std::runtime_error("Texto demasiado largo");
    layer.fontSize=static_cast<int>(num(l,"fontSize",62,8,260));
    layer.bold=flag(l,"bold",true);
    layer.underline=flag(l,"underline",false);
    if(layer.base.rotation!=0)throw std::runtime_error("Rotación de texto no disponible; utilice posición y escala");
   }
   if(layer.type=="image"){
    layer.file=str(l,"src");
    fs::path img=safeAsset(root,layer.file);
    auto key=img.lexically_normal().string();
    if(!images.count(key))images[key]=readImage(img);
    layer.image=images[key];
   }
   if(l.contains("keyframes")){
    const auto& arr=l.at("keyframes");
    if(!arr.is_array()||arr.size()>128)throw std::runtime_error("Máximo 128 keyframes");
    Pose prev=layer.base;double last=-1;
    for(const auto& k:arr){
     if(!k.is_object())throw std::runtime_error("keyframe inválido");
     double at=num(k,"time",-1,0,layer.duration);
     if(at<=last)throw std::runtime_error("Keyframes deben ir en orden temporal ascendente");
     Key key{at,readPose(k,prev),str(k,"easing","linear")};
     static const std::set<std::string> easing={"linear","easeIn","easeOut","easeInOut","back","step"};
     if(!easing.count(key.easing))throw std::runtime_error("Easing inválido");
     if(layer.type=="text"&&key.pose.rotation!=0)throw std::runtime_error("Texto no soporta rotación");
     layer.keys.push_back(key);prev=key.pose;last=at;
    }
   }
   s.layers.push_back(std::move(layer));
  }
  out.duration=s.start+s.duration;
  if(out.duration>240)throw std::runtime_error("Máximo 240 segundos por proyecto");
  out.scenes.push_back(std::move(s));
 }
 if(doc.contains("audio")){
  const auto& aud=doc.at("audio");
  if(!aud.is_array()||aud.size()>12)throw std::runtime_error("Audio debe ser arreglo de máximo 12 pistas");
  for(const auto&entry:aud){
   if(!entry.is_object())throw std::runtime_error("Pista de audio inválida");
   Audio track;
   track.file=safeAsset(root,str(entry,"src"));
   if(track.file.extension()!=".wav")throw std::runtime_error("Audio v1 requiere .wav PCM mono");
   if(fs::file_size(track.file)>30*1024*1024)throw std::runtime_error("Audio excede 30 MB");
   track.start=num(entry,"start",0,0,out.duration);
   track.gain=num(entry,"gain",1,0,2);
   out.audio.push_back(track);
  }
 }
 return out;
}
inline double ease(const std::string&e,double t) {
 t=std::clamp(t,0.,1.);
 if(e=="step")return t>=1?1:0;
 if(e=="easeIn")return t*t*t;
 if(e=="easeOut")return 1-std::pow(1-t,3);
 if(e=="easeInOut")return t<.5?4*t*t*t:1-std::pow(-2*t+2,3)/2;
 if(e=="back"){const double c=1.70158;return 1+(c+1)*std::pow(t-1,3)+c*std::pow(t-1,2);}
 return t;
}
inline Pose lerp(const Pose&a,const Pose&b,double t){
 auto f=[&](double x,double y){return x+(y-x)*t;};
 return {f(a.x,b.x),f(a.y,b.y),f(a.w,b.w),f(a.h,b.h),f(a.opacity,b.opacity),f(a.scale,b.scale),f(a.rotation,b.rotation)};
}
inline Pose sample(const Layer&l,double local) {
 if(l.keys.empty())return l.base;
 if(local<l.keys.front().at){
  if(l.keys.front().at<=0)return l.keys.front().pose;
  return lerp(l.base,l.keys.front().pose,ease(l.keys.front().easing,local/l.keys.front().at));
 }
 for(size_t i=0;i+1<l.keys.size();++i)
  if(local<=l.keys[i+1].at){
   double t=(local-l.keys[i].at)/(l.keys[i+1].at-l.keys[i].at);
   return lerp(l.keys[i].pose,l.keys[i+1].pose,ease(l.keys[i+1].easing,t));
  }
 return l.keys.back().pose;
}
inline void blend(int x,int y,Color c,double a){
 if(x<0||x>=W||y<0||y>=H||a<=0)return;
 a=std::clamp(a,0.,1.);
 const int alpha=static_cast<int>(a*255+.5), inv=255-alpha;
 auto i=(size_t(y)*W+x)*3;
 pixels[i]=static_cast<unsigned char>((pixels[i]*inv+c.b*alpha+127)/255);
 pixels[i+1]=static_cast<unsigned char>((pixels[i+1]*inv+c.g*alpha+127)/255);
 pixels[i+2]=static_cast<unsigned char>((pixels[i+2]*inv+c.r*alpha+127)/255);
}
inline Color mix(Color a,Color b,double t){
 t=std::clamp(t,0.,1.);
 return {static_cast<unsigned char>(a.r*(1-t)+b.r*t),static_cast<unsigned char>(a.g*(1-t)+b.g*t),static_cast<unsigned char>(a.b*(1-t)+b.b*t)};
}
inline void backdrop(const Scene&s){
 for(int y=0;y<H;++y){
  auto c=s.hasGradient?mix(s.background,s.gradient,double(y)/(H-1)):s.background;
  for(int x=0;x<W;++x){auto i=(size_t(y)*W+x)*3;pixels[i]=c.b;pixels[i+1]=c.g;pixels[i+2]=c.r;}
 }
}
inline double coverage(double d){return std::clamp(d+.7,0.,1.);}
inline double shapeAlpha(const Layer&l,double u,double v,double w,double h) {
 if(l.type=="glow"){
  double rx=(u-w*.5)/(w*.5),ry=(v-h*.5)/(h*.5);
  double d=rx*rx+ry*ry;
  if(d>=1.)return 0.;
  double k=1-d;
  return k*k*k;
 }
 if(l.type=="ring"){
  double rx=(u-w*.5)/(w*.5),ry=(v-h*.5)/(h*.5);
  double d=std::sqrt(rx*rx+ry*ry);
  double width=std::max(1.,l.strokeWidth)/std::min(w,h)*2.;
  return coverage((1-std::abs(d-(1-width*.5))/std::max(.001,width*.5))*.75*std::min(w,h)/2);
 }
 if(l.type=="ellipse"){
  double nx=(u-w/2)/(w/2),ny=(v-h/2)/(h/2);
  return coverage((1-std::sqrt(nx*nx+ny*ny))*std::min(w,h)/2);
 }
 if(l.type=="line"){
  double distance=l.strokeWidth*.5-std::abs(v-h*.5);
  return coverage(distance)*coverage(std::min(u,w-u));
 }
 if(l.type=="grid"){
  double pitch=std::max(10.,l.strokeWidth);
  double du=std::abs(std::remainder(u,pitch)),dv=std::abs(std::remainder(v,pitch));
  return coverage(.1-std::min(du,dv));
 }
 double outer=std::min(std::min(u,w-u),std::min(v,h-v));
 if(l.type=="rect" && l.radius>0){
  double r=std::min({l.radius,w/2,h/2});
  double qx=std::abs(u-w/2)-(w/2-r),qy=std::abs(v-h/2)-(h/2-r);
  double d=std::hypot(std::max(qx,0.),std::max(qy,0.))+std::min(std::max(qx,qy),0.)-r;
  outer=-d;
 }
 double alpha=coverage(outer);
 if(l.type=="rect"&&l.strokeWidth>0){
  double inside=outer-l.strokeWidth;
  alpha*=1.-coverage(inside);
 }
 return alpha;
}
inline void drawPixels(const Layer& l,const Pose& p) {
 const double w=p.w*p.scale,h=p.h*p.scale;
 if(w<.5||h<.5||p.opacity<.001)return;
 double radians=p.rotation*pi/180,cs=std::cos(radians),sn=std::sin(radians);
 const double cx=p.x+w/2,cy=p.y+h/2;
 const double extentX=std::abs(cs)*w/2+std::abs(sn)*h/2+2,extentY=std::abs(sn)*w/2+std::abs(cs)*h/2+2;
 const int x0=std::max(0,int(std::floor(cx-extentX))),x1=std::min(W,int(std::ceil(cx+extentX)));
 const int y0=std::max(0,int(std::floor(cy-extentY))),y1=std::min(H,int(std::ceil(cy+extentY)));
 for(int y=y0;y<y1;y++)for(int x=x0;x<x1;x++){
  double dx=x+.5-cx,dy=y+.5-cy;
  double u=cs*dx+sn*dy+w/2,v=-sn*dx+cs*dy+h/2;
  double aa=shapeAlpha(l,u,v,w,h);
  if(aa<=0)continue;
  if(l.type=="image"){
   const auto& img=*l.image;
   double sx=(u/w)*img.width-.5,sy=(v/h)*img.height-.5;
   int bx=std::clamp(int(std::floor(sx)),0,img.width-1),by=std::clamp(int(std::floor(sy)),0,img.height-1);
   int nx=std::min(bx+1,img.width-1),ny=std::min(by+1,img.height-1);
   double fx=std::clamp(sx-bx,0.,1.),fy=std::clamp(sy-by,0.,1.);
   auto a=img.rgba.data();auto index=[&](int xx,int yy){return (size_t(yy)*img.width+xx)*4;};
   auto interp=[&](int k){auto q00=a[index(bx,by)+k],q10=a[index(nx,by)+k],q01=a[index(bx,ny)+k],q11=a[index(nx,ny)+k];return (q00*(1-fx)+q10*fx)*(1-fy)+(q01*(1-fx)+q11*fx)*fy;};
   Color c{static_cast<unsigned char>(interp(0)),static_cast<unsigned char>(interp(1)),static_cast<unsigned char>(interp(2))};
   blend(x,y,c,aa*p.opacity*interp(3)/255.);
  }else blend(x,y,l.color,aa*p.opacity);
 }
}
inline void drawText(FontPainter&font,const Layer&l,const Pose&p){
 int sz=std::clamp(int(std::lround(l.fontSize*p.scale)),8,250),y=int(std::round(p.y));
 size_t pos=0;int count=0;
 while(pos<=l.text.size()&&count<20){
  auto end=l.text.find('\n',pos);
  if(end==std::string::npos)end=l.text.size();
  std::string line=l.text.substr(pos,end-pos);
  if(l.shadowOpacity>0){
   font.draw(int(std::round(p.x+l.shadowX)),int(std::round(y+l.shadowY)),line,sz,{l.shadowColor.r,l.shadowColor.g,l.shadowColor.b},l.bold,int(p.w*p.scale),float(p.opacity*l.shadowOpacity));
  }
  font.draw(int(std::round(p.x)),y,line,sz,{l.color.r,l.color.g,l.color.b},l.bold,int(p.w*p.scale),float(p.opacity));
  if(l.underline){
   int width=std::min(int(font.measure(line,sz,l.bold)),int(p.w*p.scale));
   for(int yy=y+sz+2;yy<y+sz+5;yy++)for(int xx=int(p.x);xx<int(p.x)+width;xx++)blend(xx,yy,l.color,p.opacity);
  }
  if(end==l.text.size())break;
  pos=end+1;y+=int(sz*1.17);count++;
 }
}
inline void drawScene(const Scene&s,double local){
 backdrop(s);
 static FontPainter font;
 for(const auto&l:s.layers){
  if(local+1e-7<l.from||local>=l.from+l.duration)continue;
  Pose p=sample(l,local-l.from);
  if(l.type=="text")drawText(font,l,p);
  else drawPixels(l,p);
 }
}
inline int sceneIndex(const Config& c,double seconds) {
 for(int i=int(c.scenes.size())-1;i>=0;--i)
  if(seconds>=c.scenes[i].start-0.0000001)return i;
 return 0;
}
inline void frame(const Config&c,double seconds){
 if(seconds<0||seconds>=c.duration)throw std::runtime_error("Tiempo fuera de proyecto");
 int index=sceneIndex(c,seconds);const Scene&s=c.scenes[index];
 double local=seconds-s.start;
 if(index==0||s.transition.type=="cut"||s.transition.duration<=0||local>=s.transition.duration){
  drawScene(s,std::max(0.,local));return;
 }
 const auto& prev=c.scenes[index-1];
 drawScene(prev,std::max(0.,prev.duration-1./c.fps));
 std::vector<unsigned char> before=pixels;
 drawScene(s,std::max(0.,local));
 double t=std::clamp(local/s.transition.duration,0.,1.);
 const double cx=W/2.,cy=H/2.,rad=std::hypot(cx,cy);
 for(int y=0;y<H;y++)for(int x=0;x<W;x++){
  double visibility=t;
  if(s.transition.type=="wipe") visibility=std::clamp((t-double(x)/W)*24+.5,0.,1.);
  if(s.transition.type=="slide")visibility=std::clamp((t-double(y)/H)*24+.5,0.,1.);
  if(s.transition.type=="iris")visibility=std::clamp((t-std::hypot(x-cx,y-cy)/rad)*24+.5,0.,1.);
  int alpha=int(visibility*255+.5), inv=255-alpha;
  size_t off=(size_t(y)*W+x)*3;
  for(int k=0;k<3;k++)pixels[off+k]=static_cast<unsigned char>((before[off+k]*inv+pixels[off+k]*alpha+127)/255);
 }
}
inline std::vector<int16_t> sound(const Config& cfg){
 const auto samples=static_cast<size_t>(std::ceil(cfg.duration*SR));
 std::vector<int32_t> mixBuffer(samples,0);
 for(const auto&track:cfg.audio){
  std::ifstream f(track.file,std::ios::binary);
  char riff[12];f.read(riff,12);
  if(f.gcount()!=12||std::memcmp(riff,"RIFF",4)||std::memcmp(riff+8,"WAVE",4))
   throw std::runtime_error("Audio no es WAV: "+track.file.string());
  bool format=false,data=false;
  while(f&&!data){
   char tag[4];f.read(tag,4);if(f.gcount()!=4)break;
   unsigned char b[4];f.read(reinterpret_cast<char*>(b),4);if(f.gcount()!=4)break;
   uint32_t len=uint32_t(b[0])|(uint32_t(b[1])<<8)|(uint32_t(b[2])<<16)|(uint32_t(b[3])<<24);
   if(len>30*1024*1024)throw std::runtime_error("Chunk WAV demasiado grande");
   if(std::memcmp(tag,"fmt ",4)==0){
    if(len<16||len>1024)throw std::runtime_error("Formato WAV no soportado");
    std::vector<unsigned char> header(len);f.read(reinterpret_cast<char*>(header.data()),len);
    if(!f||header[0]!=1||header[1]!=0||header[2]!=1||header[3]!=0||
       header[4]!=0xc0||header[5]!=0x5d||header[6]!=0||header[7]!=0||
       header[14]!=16||header[15]!=0)
       throw std::runtime_error("Requiere WAV PCM mono, 24 kHz, 16 bits");
    format=true;
   }else if(std::memcmp(tag,"data",4)==0){
    if(!format||len%2!=0)throw std::runtime_error("WAV PCM inválido");
    auto offset=static_cast<size_t>(std::llround(track.start*SR));
    auto available=offset<samples?samples-offset:0;
    auto count=std::min(available,size_t(len/2));
    std::vector<int16_t> pcm(count);
    if(count){f.read(reinterpret_cast<char*>(pcm.data()),count*2);
      if(size_t(f.gcount())!=count*2)throw std::runtime_error("WAV truncado");
    }
    for(size_t i=0;i<count;i++)mixBuffer[offset+i]+=int(std::lround(pcm[i]*track.gain));
    data=true;
   }else f.seekg(len+(len&1u),std::ios::cur);
  }
  if(!data)throw std::runtime_error("WAV sin muestras");
 }
 std::vector<int16_t> out(samples);
 for(size_t i=0;i<samples;i++)out[i]=int16_t(std::clamp(mixBuffer[i],-32768,32767));
 return out;
}
} // namespace project
