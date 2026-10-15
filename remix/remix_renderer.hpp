#pragma once
// Compositor integrado: sólo reutiliza el framebuffer, fuente y codecs del motor HyperFrames.
namespace remix {
using Byte=unsigned char;
constexpr double PI=3.14159265358979323846;
static RGB white{249,253,255}, muted{219,231,239}, black{5,16,30};
static float limit(float a){return std::clamp(a,0.f,1.f);}
static double limit(double a){return std::clamp(a,0.,1.);}
static double ease(double v){v=limit(v);return 1-std::pow(1-v,3);}
static double mix(double a,double b,double t){return a+(b-a)*t;}
static float reveal(double local,double at,double length){return float(ease((local-at)/length));}
static float springAt(double frame,double delay=0.,double pace=.19){
 double t=std::max(0.,frame-delay);
 return float(std::clamp(1.-std::exp(-pace*t)*std::cos(t*.32),0.,1.075));
}
static RGB color(RGB a,RGB b,float t){t=limit(t);return {
 Byte(a.r*(1.f-t)+b.r*t), Byte(a.g*(1.f-t)+b.g*t), Byte(a.b*(1.f-t)+b.b*t)};}
static RGB hex(const char* code){
 unsigned n=0;std::sscanf(code+1,"%x",&n);
 return {Byte(n>>16),Byte((n>>8)&255),Byte(n&255)};
}
static void blend(int x,int y,RGB c,float opacity){
 if(unsigned(x)>=W||unsigned(y)>=H||opacity<=0)return;
 const int a=int(limit(opacity)*255+0.5f), inv=255-a;
 const auto i=(size_t(y)*W+x)*3;
 pixels[i]=Byte((int(pixels[i])*inv+int(c.b)*a+127)/255);
 pixels[i+1]=Byte((int(pixels[i+1])*inv+int(c.g)*a+127)/255);
 pixels[i+2]=Byte((int(pixels[i+2])*inv+int(c.r)*a+127)/255);
}
static void rect(int x,int y,int w,int h,RGB c,float a=1.f){
 if(w<=0||h<=0)return;
 const int x0=std::max(0,x),x1=std::min(W,x+w),y0=std::max(0,y),y1=std::min(H,y+h);
 if(x0>=x1||y0>=y1)return;
 if(a>=.999f){for(int yy=y0;yy<y1;yy++){auto pos=(size_t(yy)*W+x0)*3;for(int xx=x0;xx<x1;xx++){
 pixels[pos++]=c.b;pixels[pos++]=c.g;pixels[pos++]=c.r;}}return;}
 for(int yy=y0;yy<y1;yy++)for(int xx=x0;xx<x1;xx++)blend(xx,yy,c,a);
}
static void roundRect(int x,int y,int w,int h,int r,RGB c,float a=1.f){
 if(w<=0||h<=0)return;
 r=std::min({r,w/2,h/2});
 for(int yy=std::max(0,y);yy<std::min(H,y+h);++yy){
   for(int xx=std::max(0,x);xx<std::min(W,x+w);++xx){
     int dx=std::max({x+r-xx,0,xx-(x+w-r-1)});
     int dy=std::max({y+r-yy,0,yy-(y+h-r-1)});
     if(dx*dx+dy*dy<=r*r)blend(xx,yy,c,a);
   }
 }
}
static void outline(int x,int y,int w,int h,int r,RGB c,float a=1.f,int thick=2){
 roundRect(x,y,w,h,r,c,a);
 roundRect(x+thick,y+thick,w-2*thick,h-2*thick,std::max(0,r-thick),black,a);
}
static void line(int x0,int y0,int x1,int y1,RGB c,float a=1.f,int thick=1){
 int dx=std::abs(x1-x0),dy=-std::abs(y1-y0),sx=x0<x1?1:-1,sy=y0<y1?1:-1,err=dx+dy;
 while(true){
  for(int ox=-thick/2;ox<=thick/2;ox++)for(int oy=-thick/2;oy<=thick/2;oy++)blend(x0+ox,y0+oy,c,a);
  if(x0==x1&&y0==y1)break;
  int e2=2*err;if(e2>=dy){err+=dy;x0+=sx;}if(e2<=dx){err+=dx;y0+=sy;}
 }
}
static void ring(int cx,int cy,int radius,RGB c,float opacity=1.f,int thickness=2,double phase=0.,bool dash=false){
 const int segments=std::max(160,int(2*PI*radius/4));
 for(int i=0;i<segments;i++){
  if(dash&&(i/8)%2)continue;
  double a=(2*PI*i/segments)+phase;
  int x=int(cx+radius*std::cos(a)),y=int(cy+radius*std::sin(a));
  for(int u=-thickness/2;u<=thickness/2;u++)blend(x+u,y,c,opacity);
 }
}
static void glow(int cx,int cy,int rad,RGB tint,float strength){
 const int x0=std::max(0,cx-rad),x1=std::min(W,cx+rad),y0=std::max(0,cy-rad),y1=std::min(H,cy+rad);
 for(int y=y0;y<y1;y++)for(int x=x0;x<x1;x++){
  const float r2=float((x-cx)*(x-cx)+(y-cy)*(y-cy))/float(rad*rad);
  if(r2<1)blend(x,y,tint,strength*(1-r2)*(1-r2));
 }
}
struct Scene {
 double start,duration;
 const char* tag;
 const char* headline;
 const char* subtitle;
 const char* highlight;
 RGB accent,secondary,deep;
};
static const std::array<Scene,15> scenes{{
{0,11.1,"EL RUMOR","¿MÉXICO\nPROHIBIÓ LOS\nMEMES?","La respuesta no es tan simple.","NO EXACTAMENTE",hex("#4CF2D7"),hex("#13B8DB"),hex("#0D2736")},
{11.1,9.3,"LO QUE PASÓ","¿QUÉ\nAPROBÓ EL\nSENADO?","30 DE SEPTIEMBRE DE 2026","UNA REFORMA",hex("#F5C55B"),hex("#FF8D65"),hex("#292537")},
{20.4,10.9,"LA CONDUCTA","EL FOCO\nES EL ENGAÑO","Identidad gráfica, dominios y documentos oficiales.","CON INTENCIÓN DE COMETER UN ILÍCITO",hex("#FC786E"),hex("#FA497E"),hex("#282039")},
{31.3,11.1,"LA POSTURA OFICIAL","FRENAR\nFRAUDES CON IA","Voces clonadas, videos falsos y suplantación.","NO LA SÁTIRA",hex("#6DA9FF"),hex("#AD8CFF"),hex("#151F40")},
{42.4,13.8,"EL DEBATE","¿RIESGO PARA\nLA LIBERTAD\nDE EXPRESIÓN?","La oposición cuestiona la amplitud de la redacción.","EL CONTEXTO IMPORTA",hex("#E6A2FF"),hex("#FF6BBA"),hex("#241A39")},
{56.2,10.8,"EN POCAS PALABRAS","HACER UN MEME\\nNO ES UN\\nDELITO\\nAUTOMÁTICO","La interpretación y aplicación de la norma son clave.","INFÓRMATE ANTES DE COMPARTIR",hex("#8AF59B"),hex("#40DBB9"),hex("#16352F")},
{67,9,"DEMO 01 · TIPOGRAFÍA","TEXTO CINÉTICO\nY RESALTADOS","Entradas por palabra, subrayados y spring.","NOTACIONES EN MOVIMIENTO",hex("#40E4F2"),hex("#D9FCFF"),hex("#10303A")},
{76,9,"DEMO 02 · CAPAS","PROFUNDIDAD\nPARALLAX","Elementos superpuestos con velocidad variable.","CAPAS INDEPENDIENTES",hex("#FFB86C"),hex("#F9DE90"),hex("#303041")},
{85,9,"DEMO 03 · DATOS","GRÁFICOS\nDINÁMICOS","Contadores, indicadores y barras animadas.","VALORES EN TIEMPO REAL",hex("#F68ACB"),hex("#D7B5FF"),hex("#2C1F3D")},
{94,9,"DEMO 04 · REVEAL","MÁSCARAS Y\nREVELACIONES","Cortinillas, recortes y disoluciones.","WIPE / IRIS / DISSOLVE",hex("#5FEEC7"),hex("#A1FFDC"),hex("#123E38")},
{103,9,"DEMO 05 · SHAPES","GEOMETRÍA\nEN ACCIÓN","Gráficos vectoriales, órbitas y trayectorias.","FORMAS PROGRAMÁTICAS",hex("#FFD46E"),hex("#FF8A6C"),hex("#302630")},
{112,9,"DEMO 06 · SISTEMAS","PARTÍCULAS\nY ÓRBITAS","Animación determinista y efectos procedurales.","MOTION GENERATIVO",hex("#92A5FF"),hex("#83E9FF"),hex("#1B2755")},
{121,9,"DEMO 07 · CAMERA","ESPACIO 3D Y\nPERSPECTIVA","Transformaciones, profundidad y movimiento.","CÁMARA SIMULADA",hex("#DE9FFF"),hex("#FE8FC0"),hex("#35243F")},
{130,9,"DEMO 08 · AUDIO","ESPECTRO DE\nSONIDO","Visualización vinculada al ritmo del soundtrack.","AUDIO GENERATIVO",hex("#77F6CB"),hex("#57CCFF"),hex("#143B3A")},
{139,9,"DEMO 09 · FINAL","TRANSICIONES\nY COMPOSICIÓN","Una biblioteca de ideas llevadas a C++.","SHOWCASE COMPLETO",hex("#FFCA80"),hex("#FF7CB0"),hex("#422439")}
}};
static std::array<std::vector<Byte>,15> bgCache{};
static void cacheBackground(int index){
 auto &dst=bgCache[index];
 if(!dst.empty())return;
 dst.resize(pixels.size());
 const auto &s=scenes[index];
 for(int y=0;y<H;y++)for(int x=0;x<W;x++){
  float u=float(x)/W,v=float(y)/H;
  float glow1=std::max(0.f,1.f-((x-575.f)*(x-575.f))/(900.f*900.f)-((y-335.f)*(y-335.f))/(850.f*850.f));
  float glow2=std::max(0.f,1.f-((x+130.f)*(x+130.f))/(600.f*600.f)-((y-760.f)*(y-760.f))/(820.f*820.f));
  RGB c=color({4,11,21},s.deep,.60f+.23f*u+.12f*(1-v));
  c=color(c,s.accent,glow1*.19f);
  c=color(c,s.secondary,glow2*.10f);
  auto k=(size_t(y)*W+x)*3;
  dst[k]=c.b;dst[k+1]=c.g;dst[k+2]=c.r;
 }
}
static void field(int f,int sceneIndex){
 auto &s=scenes[sceneIndex];
 pixels=bgCache[sceneIndex];
 const int offset=int(27*std::sin(double(f)/80.+sceneIndex));
 glow(600+offset,285,400,s.accent,.13f);
 for(int k=0;k<9;k++){
  int x=130*k-(f/7%210)-300;
  line(x,0,x+390,1279,s.accent,.025f);
 }
 for(int x=64;x<W;x+=75)line(x,210,x,H-60,{77,118,142},.052f);
 for(int y=210;y<1120;y+=75)line(0,y,W-1,y,{77,118,142},.052f);
 const int shift=double(f)*1.7;
 for(int i=0;i<32;i++){
  int x=int(std::fmod(i*79.31+sceneIndex*13.+std::sin(f/36.+i*1.9)*21+1440.,W));
  int y=int(std::fmod(i*131.7+55*sceneIndex-shift*(.4+(i*3)%7/9.)+10000.,1460.))-65;
  roundRect(x,y,i%5==0?5:3,i%5==0?5:3,2,i%3==0?s.secondary:s.accent,.22f+.08f*(i%5));
 }
 for(int i=0;i<4;i++){
  int radius=140+i*82;
  ring(585,470,radius,s.accent,.11f+i*.015f,2,f*.005*(i%2?1:-1),i%2);
 }
 for(int i=0;i<4;i++)line(0,200+i*310,W,200+i*310,s.accent,.025f);
 int scan=int(f*10%1450)-60;
 remix::rect(0,scan,W,2,s.accent,.12f);
 if(f%79==0)remix::rect(0,520+f%115,W,3,s.accent,.30f);
 // Vignette: near-black transparent marginal shading.
 remix::rect(0,0,W,46,black,.18f);remix::rect(0,H-80,W,80,black,.25f);
}
static void label(FontPainter& font,int x,int y,const std::string& v,int sz,RGB c,float a=1.f,int maxW=610){
 font.draw(x,y,v,sz,c,true,maxW,a);
}
static void wrap(FontPainter& font,const std::string& str,int x,int y,int px,int limitW,RGB c,float a,int maxLines=3){
 std::string current;
 int row=0;
 for(size_t pos=0;pos<str.size()&&row<maxLines;){
  auto j=str.find(' ',pos);if(j==std::string::npos)j=str.size();
  auto word=str.substr(pos,j-pos);
  auto next=current.empty()?word:current+" "+word;
  if(!current.empty()&&font.measure(next,px,false)>limitW){
   font.draw(x,y+row*(px+8),current,px,c,false,limitW,a);row++;current=word;
  } else current=next;
  pos=j==str.size()?j:j+1;
 }
 if(!current.empty()&&row<maxLines)font.draw(x,y+row*(px+8),current,px,c,false,limitW,a);
}
static void kinetic(FontPainter& font,const Scene& s,double frame,int idx){
 std::string all=s.headline;
 size_t pos=0;int row=0;
 int px=idx==5?67:idx==2?73:idx>=6?71:72;
 while(pos<=all.size()){
  auto e=all.find('\n',pos);if(e==std::string::npos)e=all.size();
  std::string l=all.substr(pos,e-pos);
  int x=64, y=286+row*(idx==5?71:81);
  size_t wp=0;int word=0;
  int rowSz=px;
  while(rowSz>35&&font.measure(l,rowSz,true)>595)rowSz--;
  while(wp<l.size()){
   auto end=l.find(' ',wp);if(end==std::string::npos)end=l.size();
   auto part=l.substr(wp,end-wp);
   double when=12+row*7+word*4;
   float enter=springAt(frame,when,.15);
   float a=reveal(frame,when,13)*.99f;
   if(a>0){
    RGB c=row%3==1?s.accent:white;
    int offsetY=int((1-enter)*105),wmax=595-(x-64);
    if(wmax>15)font.draw(x,y+offsetY,part,rowSz,c,true,wmax,a);
   }
   x+=int(font.measure(part+" ",rowSz,true));
   wp=end<l.size()?end+1:end;
   word++;
  }
  if(e==all.size())break;
  pos=e+1;row++;
 }
}
static void card(int x,int y,int w,int h,int r,RGB edge,RGB tint,float visibility=1.f){
 roundRect(x+5,y+13,w,h,r,{0,3,10},.55f*visibility);
 roundRect(x,y,w,h,r,{7,23,39},.94f*visibility);
 // Subtle accent and dual-layer transparency.
 roundRect(x+3,y+3,w-6,h-6,r,edge,.08f*visibility);
 outline(x,y,w,h,r,edge,.81f*visibility,2);
 (void)tint;
}
static void panel(FontPainter& font,const Scene& s,int index,double local,int frame){
 float shown=springAt(local,34,.12);
 if(local<30)return;
 float a=limit((local-31)/17.);
 if(index==0){
  int x=91+int((1-shown)*390),y=757+int(std::sin(frame/18.)*4);
  card(x,y,546,255,27,s.accent,s.deep,a);
  label(font,x+34,y+30,"VERIFICACIÓN",23,s.accent,a,465);
  label(font,x+33,y+103,"MITO ≠ LEY",57,white,a,486);
  font.draw(x+35,y+191,"No todo contenido satírico es delito.",19,muted,false,470,a);
 } else if(index==1){
  int y=745+int((1-shown)*130);
  label(font,76,y,"FECHA DE REFERENCIA",22,s.accent,a,570);
  label(font,79,y+64,"30",146,white,a,288);
  label(font,365,y+83,"SEP",43,s.accent,a,250);
  label(font,365,y+143,"2026",40,s.accent,a,250);
  remix::rect(80,y+228,int(545*shown),5,s.accent,a);
  font.draw(80,y+252,"Debate legislativo en México",26,muted,false,550,a);
 } else if(index==2){
  int r=int(127*(.55+.45*shown));
  ring(356,893,r,s.accent,a,4,frame*.025);
  ring(356,893,int(r*.8),s.secondary,a,3,-frame*.043,true);
  label(font,329,809,"!",119,white,a,165);
  label(font,153,1050,"SUPLANTACIÓN / FRAUDE",25,s.accent,a,490);
 } else if(index==3){
  ring(355,884,int(136*(.65+.35*shown)),s.accent,a,3,frame*.009,true);
  ring(355,884,int(100*(.65+.35*shown)),s.secondary,a,2,-frame*.022);
  for(int i=0;i<12;i++){
   double t=i*2*PI/12.+frame*.012;
   int x=int(355+193*std::cos(t)),y=int(884+144*std::sin(t));
   roundRect(x,y,7,7,3,s.accent,a);
  }
  label(font,302,825,"IA",93,white,a,170);
  label(font,174,1044,"DETECTAR / PROTEGER",22,s.accent,a,430);
 } else if(index==4){
  int y=789+int((1-shown)*98);
  card(63,y,286,253,22,s.accent,s.deep,a);
  card(369,y+int(std::sin(frame/16.)*6),286,253,22,s.secondary,s.deep,a);
  label(font,86,y+31,"POSTURA A",19,s.accent,a,250);
  label(font,392,y+31,"POSTURA B",19,s.secondary,a,250);
  label(font,85,y+83,"EVITAR EL",28,white,a,255);
  label(font,85,y+126,"FRAUDE",28,white,a,255);
  label(font,391,y+83,"PROTEGER LA",27,white,a,254);
  label(font,391,y+126,"EXPRESIÓN",26,white,a,254);
 } else {
  int y=810+int((1-shown)*115)+int(std::sin(frame/18.)*5);
  card(65,y,589,192,27,s.accent,s.deep,a);
  label(font,96,y+26,"CONTEXTO",67,s.accent,a,530);
  font.draw(96,y+128,"Verifica antes de compartir",29,white,false,515,a);
 }
}
#include "showcase.hpp"
static void sceneFrame(FontPainter& font,int index,int globalFrame){
 const auto& s=scenes[index];
 double local=globalFrame-std::llround(s.start*24);
 field(globalFrame,index);
 label(font,65,68,"V I S U A L   E X P L A I N E R",17,white,.94f,480);
 label(font,569,60,(index<9?"0":"")+std::to_string(index+1),43,s.accent,1,80);
 label(font,626,77,"/ 15",21,muted,.8f,87);
 int kickerX=125+int((1-springAt(local,3))*(-112));
 remix::rect(64,163,43,7,s.accent,reveal(local,0,9));
 label(font,kickerX,153,s.tag,24,s.accent,reveal(local,3,14),555);
 kinetic(font,s,local,index);
 float a=reveal(local,34,22);
 wrap(font,s.subtitle,67,680,27,582,muted,a,3);
 if(index<6)panel(font,s,index,local,globalFrame);
 else showcaseEffect(font,s,index,local,globalFrame);
 float hi=reveal(local,43,22);
 int y=1126+int((1-hi)*110);
 remix::rect(65,1098,590,2,s.accent,hi);
 int hpx=index==2?20:index==5?20:23;
 int mw=std::min(590,int(font.measure(s.highlight,hpx,true))+65);
 roundRect(65,y,mw,64,12,s.accent,hi);
 label(font,88,y+18,"✦ "+std::string(s.highlight),hpx,black,hi,mw-37);
}
static void progressBar(int frame){
 const int y=1246;
 const double duration=148.;
 int x=64;
 for(size_t i=0;i<scenes.size();i++){
  int w=int(592*scenes[i].duration/duration)-3;
  RGB a=scenes[i].accent;
  roundRect(x,y,w,8,4,{51,72,85},1);
  auto dur=std::llround(scenes[i].duration*24);
  auto start=std::llround(scenes[i].start*24);
  int fill=int(w*limit((frame-double(start))/double(dur)));
  if(fill>0)roundRect(x,y,fill,8,4,a,1);
  x+=w+3;
 }
}
static int sceneAt(int frame){
 double sec=double(frame)/24;
 for(int i=int(scenes.size())-1;i>=0;--i)if(sec>=scenes[i].start-1e-7)return i;
 return 0;
}
static std::array<std::vector<Byte>,15> lastFrames;
static void renderFrame(FontPainter& painter,int frame){
 int scene=sceneAt(frame);
 sceneFrame(painter,scene,frame);
 if(scene>0){
  int local=frame-int(std::llround(scenes[scene].start*24));
  if(local>=0&&local<28){
   if(lastFrames[scene-1].empty()){
    auto current=pixels;
    sceneFrame(painter,scene-1,int(std::llround(scenes[scene].start*24))-5);
    lastFrames[scene-1]=pixels;
    pixels=std::move(current);
   }
   auto &old=lastFrames[scene-1];
   float revealT=limit((local+2)/24.f);
   int variant=scene%7;
   for(int y=0;y<H;y++)for(int x=0;x<W;x++){
    float u=0.f;
    switch(variant){
     case 0: u=float(x)/W;break; // horizontal slide
     case 1: u=std::sqrt(float((x-W/2)*(x-W/2)+(y-H/2)*(y-H/2)))/760.f;break; // iris
     case 2: u=float(x+(H-y)*.23f)/1030.f;break; // diagonal wipe
     case 3: u=.63f*(float(x%92)/92.f)+.37f*(float(x)/W);break; // shutters / blinds
     case 4: u=std::max(std::abs(float(x-W/2))/W,std::abs(float(y-H/2))/H)*2.f;break; // geometric zoom
     case 5: {unsigned xx=(unsigned(x/7)*374761393u+unsigned(y/7)*668265263u);
              xx=(xx^(xx>>13))*1274126177u;u=float(xx%1024u)/1024.f;break;} // pixel dissolve
     default: u=float(x)/W+.065f*std::sin(y*.037f+local*.44f);break; // ripple
    }
    float cutoff=revealT*1.47f-.19f;
    float newOpacity=limit((cutoff-u)*11.f+.5f);
    auto off=(size_t(y)*W+x)*3;
    if(newOpacity<1.f){
     for(int ch=0;ch<3;ch++)pixels[off+ch]=Byte(old[off+ch]*(1-newOpacity)+pixels[off+ch]*newOpacity);
    }
    const float edge=std::max(0.f,1.f-std::abs(cutoff-u)*35.f)*(.38f*(1-revealT));
    if(edge>0.f){
      pixels[off+0]=Byte(std::min(255.f,pixels[off+0]+scenes[scene].accent.b*edge));
      pixels[off+1]=Byte(std::min(255.f,pixels[off+1]+scenes[scene].accent.g*edge));
      pixels[off+2]=Byte(std::min(255.f,pixels[off+2]+scenes[scene].accent.r*edge));
    }
   }
   float flash=local==1?.30f:(local==3?.14f:0.f);
   if(flash)remix::rect(0,0,W,H,white,flash);
   for(int band=0;band<5;band++){
    int xx=int((local*96+band*107)%1400)-450;
    for(int yy=0;yy<H;yy+=3){
     int x=xx+yy/3;
     line(x,yy,x+11,yy,scenes[scene].accent,(1-revealT)*.23f);
    }
   }
  }
 }
 progressBar(frame);
}
#include "audio_score.hpp"
static std::vector<int16_t> loadAudio(const std::string& dir,int seconds){
 std::vector<int16_t> result(size_t(seconds)*SR);
 for(size_t i=0;i<6;i++){
  if(size_t(std::llround(scenes[i].start*SR))>=result.size())break;
  char filename[50];std::snprintf(filename,sizeof filename,"/segment-%02zu.wav",i+1);
  std::ifstream f(dir+filename,std::ios::binary);
  if(!f)throw std::runtime_error("No se encuentra WAV: "+dir+filename);
  char riff[12];f.read(riff,12);
  if(!f||std::memcmp(riff,"RIFF",4)||std::memcmp(riff+8,"WAVE",4))throw std::runtime_error("WAV inválido");
  bool fmt=false,haveData=false;
  while(f&&!haveData){
    char chunk[4];f.read(chunk,4);if(!f)break;
    uint8_t n[4];f.read(reinterpret_cast<char*>(n),4);if(!f)break;
    uint32_t size=uint32_t(n[0])|(uint32_t(n[1])<<8)|(uint32_t(n[2])<<16)|(uint32_t(n[3])<<24);
    if(size>100'000'000)throw std::runtime_error("WAV demasiado grande");
    if(std::memcmp(chunk,"fmt ",4)==0){
      if(size<16||size>512)throw std::runtime_error("Formato WAV no admitido");
      std::vector<Byte> b(size);f.read(reinterpret_cast<char*>(b.data()),size);
      int channels=b[2]|(b[3]<<8),sr=b[4]|(b[5]<<8)|(b[6]<<16)|(b[7]<<24);
      int bits=b[14]|(b[15]<<8);
      if(b[0]!=1||channels!=1||sr!=SR||bits!=16)throw std::runtime_error("Requiere PCM mono 24000 Hz, 16 bit");
      fmt=true;
    }else if(std::memcmp(chunk,"data",4)==0){
      if(!fmt||size%2)throw std::runtime_error("Datos WAV inválidos");
      const size_t at=size_t(std::llround(scenes[i].start*SR));
      size_t take=std::min(size_t(size/2),result.size()-at);
      f.read(reinterpret_cast<char*>(result.data()+at),take*2);
      if(size_t(f.gcount())!=take*2)throw std::runtime_error("Audio incompleto");
      haveData=true;
    }else f.seekg(size+(size&1),std::ios::cur);
  }
  if(!haveData)throw std::runtime_error("No se leyó WAV "+std::to_string(i+1));
 }
 addShowcaseScore(result);
 return result;
}

inline void init() {
 for(size_t i=0;i<scenes.size();++i)cacheBackground(int(i));
}
inline int totalFrames(){return 3552;}
inline double durationSeconds(){return 148.;}
inline void draw(int frame){
 static FontPainter font;
 if(frame<0||frame>=totalFrames())throw std::invalid_argument("Frame fuera del showcase");
 renderFrame(font,frame);
}
inline std::vector<int16_t> soundtrack(const std::string&dir,int seconds){
 if(seconds<=0||seconds>148)throw std::invalid_argument("Duracion remix fuera de rango");
 if(dir=="-"){
  std::vector<int16_t> silence(size_t(seconds)*SR);
  addShowcaseScore(silence);
  return silence;
 }
 return loadAudio(dir,seconds);
}
} // namespace remix
