#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#endif
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#include "html_composition.hpp"
#include "browser_detection.hpp"
using namespace std;
constexpr int W=720,H=1280,SR=24000;
using U=uint32_t;
struct RGB{unsigned char r,g,b;};
vector<unsigned char> pixels(W*H*3);
void p32(ostream&o,U v){for(int i=0;i<4;i++)o.put(char(v>>(8*i)));}
void p16(ostream&o,int v){o.put(char(v));o.put(char(v>>8));}
void four(ostream&o,const char*s){o.write(s,4);}
void patch(ostream&o,streampos pos,U v){auto at=o.tellp();o.seekp(pos);p32(o,v);o.seekp(at);}
void pixel(int x,int y,RGB c){if(x<0||y<0||x>=W||y>=H)return;auto i=(size_t(y)*W+x)*3;pixels[i]=c.b;pixels[i+1]=c.g;pixels[i+2]=c.r;}
void rect(int x,int y,int w,int h,RGB c){for(int yy=max(0,y);yy<min(H,y+h);++yy)for(int xx=max(0,x);xx<min(W,x+w);++xx)pixel(xx,yy,c);}
void circle(int cx,int cy,int r,RGB c,bool filled=false){for(int y=max(0,cy-r);y<min(H,cy+r+1);++y)for(int x=max(0,cx-r);x<min(W,cx+r+1);++x){int d=(x-cx)*(x-cx)+(y-cy)*(y-cy);if(filled?d<=r*r:abs(d-r*r)<r*2)pixel(x,y,c);}}
const char* glyph(char c){
 static const char* a[]={
 "011101000110001111111000110001", // A
 "111101000111110100011000111110", // B
 "011111000010000100001000001111", // C
 "111101000110001100011000111110", // D
 "111111000011110100001000011111", // E
 "111111000011110100001000010000", // F
 "011111000010111100011000101111", // G
 "100011000111111100011000110001", // H
 "111110010000100001000010011111", // I
 "001110001000010000101001001100", // J
 "100011001011100100101000110001", // K
 "100001000010000100001000011111", // L
 "100011101110101101011000110001", // M
 "100011100110101100111000110001", // N
 "011101000110001100011000101110", // O
 "111101000110001111101000010000", // P
 "011101000110001101011001001101", // Q
 "111101000110001111101001010001", // R
 "011111000001110000011000111110", // S
 "111110010000100001000010000100", // T
 "100011000110001100011000101110", // U
 "100011000110001100010101000100", // V
 "100011000110001101011101110001", // W
 "100011000101010001000101010001", // X
 "100011000101010001000010000100", // Y
 "111110000100010001000100011111", // Z
 "011101000110011101011100101110", // 0
 "001000110000100001000010001110", // 1
 "011101000100010001000100011111", // 2
 "111100000100110000011000111110", // 3
 "000100011001010111110001000010", // 4
 "111111000011110000011000111110", // 5
 "011111000011110100011000101110", // 6
 "111110000100010001000100001000", // 7
 "011101000101110100011000101110", // 8
 "011101000101111000011000111110"  // 9
 };
 if(c>='A'&&c<='Z')return a[c-'A'];
 if(c>='0'&&c<='9')return a[26+c-'0'];
 return nullptr;
}
void text(int x,int y,string s,int scale,RGB c){for(char ch:s){if(ch==' '){x+=scale*4;continue;}auto g=glyph(ch);if(g){for(int r=0;r<7;r++)for(int k=0;k<5;k++)if(g[r*5+k]=='1')rect(x+k*scale,y+r*scale,scale,scale,c);}else if(ch=='.')rect(x+2*scale,y+6*scale,scale,scale,c);else if(ch=='?'){rect(x,y,5*scale,scale,c);rect(x+4*scale,y+scale,scale,2*scale,c);rect(x+2*scale,y+3*scale,scale,scale,c);rect(x+2*scale,y+6*scale,scale,scale,c);}x+=6*scale;}}
#include "font_renderer.hpp"
#include "animation_timeline.hpp"
#include "output_scale.hpp"
#include "render_scene.hpp"
#include "render_news.hpp"
#ifdef HFC_WITH_BROWSER_CAPTURE
#include "browser_capture.hpp"
#endif
#ifdef HFC_WITH_LITEHTML
#include "html_scene_engine.hpp"
#endif
#include "avi_rle8.hpp"
#ifdef HFC_WITH_NATIVE_MP4
#include "native_mp4.hpp"
#endif
#define STB_IMAGE_WRITE_IMPLEMENTATION
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
#include "vendor/stb/stb_image_write.h"
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#include "remix/remix_renderer.hpp"
#include "compositor/json_project.hpp"
#include "voice/neo_bridge.hpp"
vector<int16_t> loadAudio(const string&dir,const vector<hf::Scene>&scenes,int seconds){
 vector<int16_t> result(size_t(seconds)*SR,0);
 if(dir=="-")return result;
 for(size_t j=0;j<scenes.size();j++){
  char name[40];snprintf(name,sizeof name,"/segment-%02zu.wav",j+1);
  ifstream f(dir+name,ios::binary);
  if(!f)throw runtime_error(string("Audio ausente: ")+name);
  char riff[12];f.read(riff,12);
  if(f.gcount()!=12||memcmp(riff,"RIFF",4)||memcmp(riff+8,"WAVE",4))throw runtime_error("RIFF WAV invalido");
  bool fmt=false,data=false;
  while(f&&!data){
   char tag[4];f.read(tag,4);if(f.gcount()!=4)break;
   unsigned char sz[4];f.read((char*)sz,4);if(f.gcount()!=4)break;
   U len=U(sz[0])|(U(sz[1])<<8)|(U(sz[2])<<16)|(U(sz[3])<<24);
   if(len>100000000)throw runtime_error("Chunk WAV demasiado grande");
   if(memcmp(tag,"fmt ",4)==0){
    if(len<16||len>1024)throw runtime_error("Formato WAV no soportado");
    vector<unsigned char> h(len);f.read((char*)h.data(),len);
    if(!f||h[0]!=1||h[2]!=1||h[4]!=0xc0||h[5]!=0x5d||h[14]!=16)throw runtime_error("Requiere PCM mono 24000Hz 16 bits");
    fmt=true;
   }else if(memcmp(tag,"data",4)==0){
    if(!fmt)throw runtime_error("Falta formato WAV");
    size_t at=size_t(scenes[j].start*SR+.5);
    if(at>=result.size()){data=true;break;}
    size_t count=min(size_t(len/2),result.size()-at);
    f.read((char*)(result.data()+at),count*2);
    if(size_t(f.gcount())!=count*2)throw runtime_error("Audio truncado");
    data=true;
   }else f.seekg(len+(len&1),ios::cur);
  }
  if(!data)throw runtime_error("Chunk de audio ausente");
 }
 return result;
}
std::vector<unsigned char> encodeJpeg(const std::vector<unsigned char>& bgr,int width,int height) {
 std::vector<unsigned char> rgb(bgr.size()), result;
 for(size_t i=0;i<bgr.size();i+=3){rgb[i]=bgr[i+2];rgb[i+1]=bgr[i+1];rgb[i+2]=bgr[i];}
 auto callback=[](void* context,void* data,int len) {
  auto& out=*static_cast<std::vector<unsigned char>*>(context);
  const auto* first=static_cast<const unsigned char*>(data);
  if(len>0)out.insert(out.end(),first,first+len);
 };
 if(!stbi_write_jpg_to_func(callback,&result,width,height,3,rgb.data(),89)||result.empty())
  throw std::runtime_error("No se pudo codificar fotograma JPEG");
 return result;
}
int main(int argc,char**argv){try{
 if(argc>=2 && string(argv[1])=="--voice"){
  if(argc!=9&&argc!=11)throw runtime_error("Uso: --voice synth --model voz.onnx|voz.neo --text-file texto.txt --out voz.wav [--neo-lite ruta]");
  if(string(argv[2])!="synth")throw runtime_error("Comando --voice admitido: synth");
  map<string,string> opts;
  for(int i=3;i<argc;i+=2){
   string key=argv[i];if(key!="--model"&&key!="--text-file"&&key!="--out"&&key!="--neo-lite")
    throw runtime_error("Opción de voz desconocida");
   if(!opts.emplace(key,argv[i+1]).second)throw runtime_error("Opción de voz duplicada");
  }
  for(auto required:{"--model","--text-file","--out"})
   if(!opts.count(required))throw runtime_error(string("Falta ")+required);
  const char* configured=getenv("GPTFRAMES_NEO_LITE");
  string engine=opts.count("--neo-lite")?opts.at("--neo-lite"):(configured?configured:"neo-lite");
  gptvoice::synthesize(opts.at("--model"),opts.at("--text-file"),opts.at("--out"),engine);
  cout<<"Voz generada con Neo Lite: "<<opts.at("--out")<<" (WAV PCM mono, 24000 Hz)\n";
  return 0;
 }
 if(argc>=4 && string(argv[1])=="--project"){
   string mode=argv[2],filename=argv[3];
   if(mode!="validate"&&mode!="frame"&&mode!="render")throw runtime_error("Modo project: validate, frame, render");
   const auto config=project::load(filename);
   if(mode=="validate"){
    if(argc!=4)throw runtime_error("Uso: --project validate proyecto.json");
    cout<<"Proyecto válido: "<<config.scenes.size()<<" escenas, "<<config.duration<<"s, "
        <<config.fps<<"fps, "<<config.width<<"x"<<config.height
        <<", "<<config.audio.size()<<" pistas WAV\n";
    return 0;
   }
   if(mode=="frame"){
    if(argc!=6)throw runtime_error("Uso: --project frame proyecto.json imagen.png segundo");
    size_t used=0;
    double sec=stod(argv[5],&used);
    if(used!=strlen(argv[5])||!isfinite(sec)||sec<0||sec>=config.duration)throw runtime_error("Segundo fuera de proyecto");
    project::frame(config,sec);
    vector<unsigned char> data=(config.width==W&&config.height==H)?pixels:output::resize(pixels,W,H,config.width,config.height);
    vector<unsigned char> rgb(data);
    for(size_t i=0;i<rgb.size();i+=3)std::swap(rgb[i],rgb[i+2]);
    if(!stbi_write_png(argv[4],config.width,config.height,3,rgb.data(),config.width*3))
      throw runtime_error("No se pudo guardar PNG");
    cout<<"Proyecto frame: "<<argv[4]<<" ("<<sec<<"s)\n";return 0;
   }
   if(argc!=5&&argc!=6)throw runtime_error("Uso: --project render proyecto.json salida.mp4 [duracion]");
#ifndef HFC_WITH_NATIVE_MP4
   throw runtime_error("Render MP4 requiere HFC_NATIVE_MP4=ON");
#else
   double duration=config.duration;
   if(argc==6){
    size_t used=0;
    duration=stod(argv[5],&used);
    if(used!=strlen(argv[5])||!isfinite(duration)||duration<=0||duration>config.duration)
      throw runtime_error("Duración del render fuera de rango");
   }
   const int count=int(std::ceil(duration*config.fps-0.00001));
   const string outputPath=argv[4];
   if(outputPath.size()<5||outputPath.substr(outputPath.size()-4)!=".mp4")
     throw runtime_error("La salida del proyecto requiere .mp4");
   auto audio=project::sound(config);
   audio.resize(size_t(std::ceil(duration*SR)),0);
   NativeMp4Writer writer(outputPath.c_str(),config.width,config.height,config.fps);
   vector<unsigned char> scaled;
   for(int i=0;i<count;i++){
    project::frame(config,std::min(config.duration-1e-6,double(i)/config.fps));
    if(config.width!=W||config.height!=H)scaled=output::resize(pixels,W,H,config.width,config.height);
    writer.addVideo((config.width==W&&config.height==H)?pixels:scaled);
    if(i%std::max(1,config.fps*4)==0)cerr<<"JSON project "<<i<<"/"<<count<<"\n";
   }
   writer.addAudio(audio);
   cout<<"Proyecto JSON renderizado: "<<outputPath<<" ("<<duration<<"s, "<<config.fps<<"fps)\n";
   return 0;
#endif
 }
 // El mismo ejecutable conserva la CLI HTML/AVI/browser y añade el compositor fusionado.
 if(argc==2 && string(argv[1])=="--remix-list"){
   for(size_t i=0;i<remix::scenes.size();++i){
    const auto& s=remix::scenes[i];
    cout<<i+1<<" | "<<s.start<<"s | "<<s.duration<<"s | "<<s.tag<<"\n";
   }
   return 0;
 }
 if(argc==4 && string(argv[1])=="--remix-frame"){
   size_t used=0;double sec=std::stod(argv[3],&used);
   if(used!=std::strlen(argv[3])||!std::isfinite(sec)||sec<0||sec>=148)
     throw runtime_error("Segundo remix invalido (0..148)");
   remix::init();
   remix::draw(std::min(3551,int(std::round(sec*24))));
   vector<unsigned char> rgb(pixels);
   for(size_t i=0;i<rgb.size();i+=3)std::swap(rgb[i],rgb[i+2]);
   if(!stbi_write_png(argv[2],W,H,3,rgb.data(),W*3))
     throw runtime_error("No se pudo guardar PNG remix");
   cout<<"Remix preview: "<<argv[2]<<" ("<<sec<<"s)\n";
   return 0;
 }
 if(argc>=5 && argc<=8 && string(argv[1])=="--fusion"){
#ifndef HFC_WITH_NATIVE_MP4
   throw runtime_error("Fusion requiere compilacion HFC_NATIVE_MP4=ON");
#else
   if(string(argv[4]).size()<5 || string(argv[4]).substr(std::strlen(argv[4])-4)!=".mp4")
     throw runtime_error("Fusion requiere salida .mp4");
   size_t used=0;
   int fps=24;
   if(argc>=6){fps=stoi(argv[5],&used);
     if(used!=std::strlen(argv[5]) || fps<1||fps>60)throw runtime_error("Fusion FPS 1..60");}
   string theme=argc>=7?argv[6]:"classic";
   if(theme!="classic"&&theme!="news"&&theme!="html"&&theme!="browser")
     throw runtime_error("Fusion theme: classic, news, html o browser");
#ifndef HFC_WITH_BROWSER_CAPTURE
   if(theme=="browser")throw runtime_error("Fusion browser requiere navegador/CDP");
#endif
#ifndef HFC_WITH_LITEHTML
   if(theme=="html")throw runtime_error("Fusion html requiere litehtml");
#endif
   auto dim=argc>=8?output::parseSize(argv[7]):output::Size{};
   const auto intro=hf::load(argv[2]);
   double introSeconds=intro.back().start+intro.back().duration;
   if(introSeconds>480||introSeconds<1)throw runtime_error("Intro fusion debe durar 1..480 segundos");
   const double fullDuration=introSeconds+81.;
   const int frames=int(std::ceil(fullDuration*fps-1e-5));
   const int samples=int(std::ceil(fullDuration))*SR;
   vector<int16_t> mixed(samples,0);
   const int voiceSeconds=int(std::ceil(introSeconds));
   auto initial=loadAudio(argv[3],intro,voiceSeconds);
   std::copy_n(initial.begin(),std::min(initial.size(),mixed.size()),mixed.begin());
   const auto score=remix::soundtrack("-",148);
   const size_t at=size_t(std::llround(introSeconds*SR));
   const size_t scoreStart=67u*SR;
   if(at<mixed.size()&&scoreStart<score.size()){
     const size_t count=std::min(mixed.size()-at,score.size()-scoreStart);
     std::copy_n(score.begin()+scoreStart,count,mixed.begin()+at);
   }
   remix::init();
#ifdef HFC_WITH_LITEHTML
   std::unique_ptr<HtmlSceneEngine> htmlScene;
   if(theme=="html")htmlScene=std::make_unique<HtmlSceneEngine>(argv[2],intro);
#endif
#ifdef HFC_WITH_BROWSER_CAPTURE
   std::unique_ptr<browser::Capture> browserScene;
   if(theme=="browser")browserScene=std::make_unique<browser::Capture>(argv[2],W,H);
#endif
   NativeMp4Writer video(argv[4],dim.width,dim.height,fps);
   vector<unsigned char> scaled;
   for(int n=0;n<frames;++n){
     double t=double(n)/fps;
     if(t<introSeconds){
#ifdef HFC_WITH_BROWSER_CAPTURE
       if(theme=="browser")browserScene->render(t,pixels);
       else
#endif
#ifdef HFC_WITH_LITEHTML
       if(theme=="html")htmlScene->frame(n,fps,intro);
       else
#endif
       if(theme=="news")renderNews(n,fps,intro,fullDuration);
       else render(n,fps,intro,fullDuration);
     }else{
       int source=1608+int(std::floor((t-introSeconds)*24.+.5));
       remix::draw(std::min(source,3551));
     }
     if(dim.width!=W||dim.height!=H)scaled=output::resize(pixels,W,H,dim.width,dim.height);
     video.addVideo((dim.width==W && dim.height==H)?pixels:scaled);
     if(n%std::max(1,5*fps)==0)cerr<<"Fusion "<<n<<"/"<<frames<<" frames\n";
   }
   video.addAudio(mixed);
   cout<<"MP4 fusionado: "<<argv[4]<<" ("<<fullDuration<<" s; "<<fps<<" fps; "<<intro.size()<<" escenas HyperFrames + 9 efectos)\n";
   return 0;
#endif
 }
 if(argc>=3 && string(argv[1])=="--remix" && argc<=7){
#ifndef HFC_WITH_NATIVE_MP4
   throw runtime_error("Remix MP4 requiere HFC_NATIVE_MP4=ON");
#else
   const std::string out=argv[2];
   if(out.size()<5||out.substr(out.size()-4)!=".mp4")throw runtime_error("Salida remix requiere .mp4");
   auto strictInteger=[](const char* value,int lo,int hi){
    size_t used=0;int n=std::stoi(value,&used);
    if(used!=std::strlen(value)||n<lo||n>hi)throw std::runtime_error("Numero remix invalido");
    return n;
   };
   const int fps=argc>=4?strictInteger(argv[3],1,60):24;
   const auto dim=argc>=5?output::parseSize(argv[4]):output::Size{};
   const int sec=argc>=6?strictInteger(argv[5],1,148):148;
   const std::string audioDir=argc>=7?argv[6]:"proyecto-remix/audio";
   remix::init();
   auto audio=remix::soundtrack(audioDir,sec);
   NativeMp4Writer writer(out.c_str(),dim.width,dim.height,fps);
   vector<unsigned char> scaled;
   const int frames=sec*fps;
   for(int n=0;n<frames;++n){
    remix::draw(std::min(remix::totalFrames()-1,int(std::llround(double(n)*24/fps))));
    if(dim.width!=W||dim.height!=H)scaled=output::resize(pixels,W,H,dim.width,dim.height);
    writer.addVideo((dim.width==W && dim.height==H)?pixels:scaled);
    if(n%std::max(1,fps*5)==0)std::cerr<<"Remix "<<n<<"/"<<frames<<" frames\n";
   }
   writer.addAudio(audio);
   cout<<"Remix MP4 generado: "<<out<<" ("<<sec<<"s, "<<fps<<"fps)\n";
   return 0;
#endif
 }
 #ifdef HFC_WITH_BROWSER_CAPTURE
 if(argc==5 && string(argv[1])=="--browser-frame"){
  browser::Capture cap(argv[2],W,H);
  std::vector<unsigned char> image;
  cap.render(std::stod(argv[3]),image);
  for(size_t i=0;i<image.size();i+=3)std::swap(image[i],image[i+2]);
  if(!stbi_write_png(argv[4],W,H,3,image.data(),W*3))
   throw runtime_error("Could not write browser preview PNG");
  cout<<"Browser frame captured at t="<<argv[3]<<": "<<argv[4]<<"\n";
  return 0;
 }
#endif
 if(argc==2 && string(argv[1])=="--browser-process-check"){
  const auto executable=browser::discover_running_process();
  if(executable.empty()){cerr<<"No running Chromium-family browser process found\n";return 3;}
  cout<<"Running browser executable: "<<executable<<"\n";return 0;
 }
 if(argc==2 && string(argv[1])=="--browser-check"){
  const auto executable=browser::discover();
  if(executable.empty()){cerr<<"Browser not found: install Chromium/Chrome/Edge or set HFC_BROWSER\n";return 3;}
  cout<<"Detected browser: "<<executable<<"\n";return 0;
 }
 if(argc==3 && string(argv[1])=="--inspect"){
  const auto data=hf::load(argv[2]);
  for(size_t i=0;i<data.size();++i){
   cout<<i+1<<" | "<<data[i].start<<" | "<<data[i].duration<<" | "<<data[i].tag<<" | "<<data[i].highlight<<"\n";
   for(const auto& line:data[i].lines)cout<<"    "<<line<<"\n";
  }
  return 0;
 }
 if(argc<4 || argc>9){cerr<<"Usage: hyperframes-cpp <composition.html> <WAV-dir|-> <output.avi|mp4> [fps 1..60] [mjpeg|rgb24|rle8|h264] [classic|news|html|browser] [WIDTHxHEIGHT] [clip-seconds]\n";return 2;}
 int fps=24; // 24 por defecto; 60 fps como maximo.
 if(argc>=5){
  size_t used=0;
  try{fps=stoi(argv[4],&used);}catch(...){throw runtime_error("fps debe ser entero de 1 a 60");}
  if(used!=strlen(argv[4]))throw runtime_error("fps debe ser entero de 1 a 60");
 }
 if(fps<1||fps>60)throw runtime_error("fps fuera de rango: se admite de 1 a 60");
 const std::string codec=argc>=6?argv[5]:"mjpeg";
 const std::string theme=argc>=7?argv[6]:"classic";
 if(theme!="classic" && theme!="news" && theme!="html" && theme!="browser")
   throw runtime_error("Tema no soportado: classic, news, html o browser");
#ifndef HFC_WITH_BROWSER_CAPTURE
 if(theme=="browser")throw runtime_error("Tema browser requiere compilar con HFC_BROWSER_CAPTURE");
#endif
#ifndef HFC_WITH_LITEHTML
 if(theme=="html")throw runtime_error("Tema html requiere compilar con litehtml: scripts/build-html-linux.sh");
#endif
 const auto target=argc==8?output::parseSize(argv[7]):output::Size{};
 const int videoW=target.width,videoH=target.height;
 const bool rle=codec=="rle8",jpeg=codec=="mjpeg",raw=codec=="rgb24",h264=codec=="h264";
 if(!rle&&!jpeg&&!raw&&!h264)throw runtime_error("Codec no soportado: usar mjpeg, rgb24, rle8 o h264");
#ifndef HFC_WITH_NATIVE_MP4
 if(h264)throw runtime_error("H264 requiere build nativo: scripts/build-native-mp4.sh");
#endif
 const bool compressed=rle||jpeg||h264;
 const auto scenes=hf::load(argv[1]);
 const double fullDuration=scenes.back().start+scenes.back().duration;
 double duration=fullDuration;
 if(argc==9){
  size_t used=0;
  try{duration=std::stod(argv[8],&used);
      if(used!=std::strlen(argv[8]) || !std::isfinite(duration))
       throw std::invalid_argument("invalid");}
  catch(...){throw std::runtime_error("clip-seconds must be a positive number");}
  if(duration<0.1||duration>fullDuration)
   throw std::runtime_error("clip-seconds outside composition duration");
 }
 const int seconds=int(std::ceil(duration-0.00001));
 const int frames=int(std::ceil(duration*fps-0.00001));
 const uint64_t estimated=uint64_t(videoW)*videoH*3*frames+uint64_t(seconds)*SR*2+uint64_t(frames)*40+4096;
 if(!compressed&&estimated>numeric_limits<U>::max())throw runtime_error("AVI RGB sin comprimir superaria 4 GiB; use rle8");

 auto audio=loadAudio(argv[2],scenes,seconds);
#ifdef HFC_WITH_LITEHTML
 std::unique_ptr<HtmlSceneEngine> htmlEngine;
 if(theme=="html")htmlEngine=std::make_unique<HtmlSceneEngine>(argv[1],scenes);
#endif
#ifdef HFC_WITH_BROWSER_CAPTURE
 std::unique_ptr<browser::Capture> browserCapture;
 if(theme=="browser")browserCapture=std::make_unique<browser::Capture>(argv[1],W,H);
#endif
 std::vector<unsigned char> scaled;
 auto renderFrame=[&](int n)->const std::vector<unsigned char>& {
#ifdef HFC_WITH_BROWSER_CAPTURE
  if(theme=="browser")browserCapture->render(double(n)/fps,pixels);
  else
#endif
#ifdef HFC_WITH_LITEHTML
  if(theme=="html")htmlEngine->frame(n,fps,scenes);
  else
#endif
  if(theme=="news")renderNews(n,fps,scenes,duration);
  else render(n,fps,scenes,duration);
  if(videoW==W && videoH==H)return pixels;
  scaled=output::resize(pixels,W,H,videoW,videoH);
  return scaled;
 };
#ifdef HFC_WITH_NATIVE_MP4
 if(h264){
  NativeMp4Writer mp4(argv[3],videoW,videoH,fps);
  for(int n=0;n<frames;n++){
   mp4.addVideo(renderFrame(n));
   if(n%120==0)cerr<<"H264 frame "<<n<<"/"<<frames<<"\n";
  }
  mp4.addAudio(audio);
  cout<<"MP4/H264/AAC generated: "<<argv[3]<<" ("<<fps<<" fps, "<<duration<<" seconds)\n";
  return 0;
 }
#endif
 ofstream o(argv[3],ios::binary);if(!o)throw runtime_error("No se puede escribir AVI");four(o,"RIFF");auto riff=o.tellp();p32(o,0);four(o,"AVI ");four(o,"LIST");auto hdrl=o.tellp();p32(o,0);four(o,"hdrl");
 four(o,"avih");p32(o,56);p32(o,1000000/fps);p32(o,(compressed?videoW*videoH:videoW*videoH*3)*fps+SR*2);p32(o,0);p32(o,0x10);p32(o,frames);p32(o,0);p32(o,2);p32(o,compressed?videoW*videoH:videoW*videoH*3);p32(o,videoW);p32(o,videoH);for(int i=0;i<4;i++)p32(o,0);
 four(o,"LIST");p32(o,rle?1140:116);four(o,"strl");four(o,"strh");p32(o,56);four(o,"vids");four(o,jpeg?"MJPG":(rle?"MRLE":"DIB "));p32(o,0);p16(o,0);p16(o,0);p32(o,0);p32(o,1);p32(o,fps);p32(o,0);p32(o,frames);p32(o,compressed?videoW*videoH:videoW*videoH*3);p32(o,0xffffffff);p32(o,0);p16(o,0);p16(o,0);p16(o,videoW);p16(o,videoH);
 four(o,"strf");p32(o,rle?1064:40);p32(o,40);p32(o,videoW);p32(o,videoH);p16(o,1);p16(o,rle?8:24);p32(o,rle?1:(jpeg?0x47504a4du:0));p32(o,compressed?videoW*videoH:videoW*videoH*3);p32(o,0);p32(o,0);p32(o,rle?256:0);p32(o,0);if(rle)writeRlePalette(o);
 four(o,"LIST");p32(o,92);four(o,"strl");four(o,"strh");p32(o,56);four(o,"auds");p32(o,0);p32(o,0);p16(o,0);p16(o,0);p32(o,0);p32(o,2);p32(o,SR*2);p32(o,0);p32(o,seconds*SR*2);p32(o,SR*2);p32(o,0xffffffff);p32(o,2);for(int i=0;i<4;i++)p16(o,0);
 four(o,"strf");p32(o,16);p16(o,1);p16(o,1);p32(o,SR);p32(o,SR*2);p16(o,2);p16(o,16);
 patch(o,hdrl,U(o.tellp()-hdrl-4));four(o,"LIST");auto movi=o.tellp();p32(o,0);four(o,"movi");auto movibase=o.tellp();struct Entry{char tag[4];U flags,offset,size;};vector<Entry> idx;idx.reserve(frames*2);
 for(int n=0;n<frames;n++){
  const auto& finalPixels=renderFrame(n);
  auto frame=o.tellp();U frameBytes=0;
  if(compressed){
   auto data=jpeg?encodeJpeg(finalPixels,videoW,videoH):encodeRle8(finalPixels,videoW,videoH);
   frameBytes=U(data.size());
   four(o,"00dc");p32(o,frameBytes);o.write((const char*)data.data(),frameBytes);
   if(frameBytes&1)o.put(0);
   if(uint64_t(o.tellp())+uint64_t((frames-n)*64)+4096>=numeric_limits<U>::max())
     throw runtime_error("Limite AVI RIFF 4 GiB alcanzado; reduzca FPS o resolucion");
   idx.push_back({{'0','0','d','c'},16,U(frame-movibase+4),frameBytes});
  }else{
   frameBytes=videoW*videoH*3;four(o,"00db");p32(o,frameBytes);
   for(int y=videoH-1;y>=0;y--)o.write((char*)finalPixels.data()+size_t(y)*videoW*3,videoW*3);
   idx.push_back({{'0','0','d','b'},16,U(frame-movibase+4),frameBytes});
  }auto at=size_t(n)*SR/fps;auto end=size_t(n+1)*SR/fps;auto bytes=U((end-at)*2);auto snd=o.tellp();four(o,"01wb");p32(o,bytes);o.write((char*)(audio.data()+at),bytes);idx.push_back({{'0','1','w','b'},0,U(snd-movibase+4),bytes});if(n%120==0)cerr<<"Fotograma "<<n<<"/"<<frames<<"\n";}
 patch(o,movi,U(o.tellp()-movi-4));four(o,"idx1");p32(o,U(idx.size()*16));for(auto&e:idx){o.write(e.tag,4);p32(o,e.flags);p32(o,e.offset);p32(o,e.size);}patch(o,riff,U(o.tellp()-std::streamoff(8)));o.close();if(!o)throw runtime_error("Error de escritura del AVI");cout<<"Video generado desde HTML: "<<argv[3]<<" ("<<fps<<" fps, "<<duration<<" s, "<<scenes.size()<<" escenas, "<<codec<<")\n";return 0;}catch(const exception&e){cerr<<"Error: "<<e.what()<<"\n";return 1;}}
