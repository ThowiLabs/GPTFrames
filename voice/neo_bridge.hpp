#pragma once
// Puente de inferencia con Neo Lite. La inferencia se ejecuta en el motor oficial
// Neo Lite; GPTFrames valida y adapta su salida PCM al compositor de vídeo.
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cmath>
#include <cerrno>
#include <cstring>
#include <cstdint>
#include <algorithm>
#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif
namespace gptvoice {
namespace fs=std::filesystem;
inline void run(const std::vector<std::string>& args) {
 if(args.empty())throw std::runtime_error("Ejecutable vacío");
 for(const auto&a:args)if(a.find('\0')!=std::string::npos)throw std::runtime_error("Argumento inválido");
#ifdef _WIN32
 std::vector<const char*> v;for(auto&a:args)v.push_back(a.c_str());v.push_back(nullptr);
 int rc=_spawnvp(_P_WAIT,args[0].c_str(),v.data());
 if(rc!=0)throw std::runtime_error("Neo Lite falló (código "+std::to_string(rc)+")");
#else
 pid_t pid=fork();
 if(pid<0)throw std::runtime_error("No se pudo iniciar Neo Lite");
 if(pid==0){
  std::vector<char*>v;for(const auto&a:args)v.push_back(const_cast<char*>(a.c_str()));v.push_back(nullptr);
  execvp(v[0],v.data());_exit(127);
 }
 int st=0;while(waitpid(pid,&st,0)<0){if(errno!=EINTR)throw std::runtime_error("Error esperando Neo Lite");}
 if(!WIFEXITED(st)||WEXITSTATUS(st)!=0)throw std::runtime_error("Neo Lite falló (estado "+std::to_string(st)+")");
#endif
}
inline uint16_t le16(const unsigned char*p){return uint16_t(p[0])|(uint16_t(p[1])<<8);}
inline uint32_t le32(const unsigned char*p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
inline void w16(std::ostream& f,uint16_t n){f.put(char(n));f.put(char(n>>8));}
inline void w32(std::ostream& f,uint32_t n){for(int i=0;i<4;++i)f.put(char(n>>(8*i)));}
struct PCM {int sampleRate=0;std::vector<int16_t> samples;};
inline PCM readWav(const fs::path&filename){
 if(!fs::is_regular_file(filename)||fs::file_size(filename)>120*1024*1024)throw std::runtime_error("WAV ausente o demasiado grande");
 std::ifstream f(filename,std::ios::binary);
 char head[12];f.read(head,12);
 if(!f||std::memcmp(head,"RIFF",4)||std::memcmp(head+8,"WAVE",4))throw std::runtime_error("RIFF/WAVE inválido");
 PCM out;int channels=0,bits=0,format=0;bool dataFound=false;
 while(f&&!dataFound){
  char tag[4];unsigned char sizeBytes[4];f.read(tag,4);f.read(reinterpret_cast<char*>(sizeBytes),4);if(!f)break;
  uint32_t len=le32(sizeBytes);
  if(len>120*1024*1024)throw std::runtime_error("Chunk WAV demasiado grande");
  if(std::memcmp(tag,"fmt ",4)==0){
   if(len<16||len>4096)throw std::runtime_error("Formato WAV no admitido");
   std::vector<unsigned char>b(len);f.read(reinterpret_cast<char*>(b.data()),len);
   if(!f)throw std::runtime_error("Cabecera WAV truncada");
   format=le16(b.data());channels=le16(b.data()+2);out.sampleRate=int(le32(b.data()+4));bits=le16(b.data()+14);
  } else if(std::memcmp(tag,"data",4)==0){
   if(format!=1||channels!=1||bits!=16||out.sampleRate<8000||out.sampleRate>96000||len%2)
    throw std::runtime_error("Requiere WAV PCM mono de 16 bits");
   std::vector<unsigned char>b(len);f.read(reinterpret_cast<char*>(b.data()),len);
   if(!f)throw std::runtime_error("Datos WAV truncados");
   out.samples.resize(len/2);
   for(size_t i=0;i<out.samples.size();++i)out.samples[i]=int16_t(le16(b.data()+i*2));
   dataFound=true;
  }else f.seekg(len+(len&1),std::ios::cur);
 }
 if(!dataFound||out.samples.empty())throw std::runtime_error("Sin audio PCM");
 return out;
}
inline void write24k(const PCM&src,const fs::path&filename){
 constexpr int rate=24000;
 size_t count=size_t(std::llround(double(src.samples.size())*rate/src.sampleRate));
 if(count==0||count>size_t(rate)*600)throw std::runtime_error("Audio no puede exceder 10 minutos");
 std::ofstream f(filename,std::ios::binary|std::ios::trunc);
 if(!f)throw std::runtime_error("No se pudo crear WAV final");
 uint32_t bytes=uint32_t(count*2);
 f.write("RIFF",4);w32(f,36+bytes);f.write("WAVEfmt ",8);w32(f,16);
 w16(f,1);w16(f,1);w32(f,rate);w32(f,rate*2);w16(f,2);w16(f,16);
 f.write("data",4);w32(f,bytes);
 for(size_t i=0;i<count;++i){
  double srcIndex=double(i)*src.sampleRate/rate;
  auto lo=std::min(size_t(srcIndex),src.samples.size()-1);
  auto hi=std::min(lo+1,src.samples.size()-1);
  double frac=std::clamp(srcIndex-double(lo),0.,1.);
  auto sample=int(std::lround((1-frac)*src.samples[lo]+frac*src.samples[hi]));
  w16(f,uint16_t(int16_t(std::clamp(sample,-32768,32767))));
 }
 if(!f)throw std::runtime_error("Error escribiendo WAV final");
}
inline void synthesize(const fs::path&model,const fs::path&textFile,const fs::path&out,const std::string&binary="neo-lite"){
 if(!fs::is_regular_file(model)||!(model.extension()==".onnx"||model.extension()==".neo"))
  throw std::runtime_error("Modelo Neo Lite .onnx/.neo no encontrado");
 if(!fs::is_regular_file(textFile)||fs::file_size(textFile)>65536)
  throw std::runtime_error("Texto de voz ausente o mayor de 64KB");
 if(out.extension()!=".wav")throw std::runtime_error("Salida de voz requiere .wav");
 if(!out.parent_path().empty())fs::create_directories(out.parent_path());
 fs::path tmp=out;tmp+=".neo-part.wav";
 if(fs::exists(tmp))throw std::runtime_error("Existe un WAV temporal; no sobrescribir");
 try{
  run({binary,"infer","--model",model.string(),"--text-file",textFile.string(),"--out",tmp.string(),"--device","cpu"});
  write24k(readWav(tmp),out);
  fs::remove(tmp);
 }catch(...){std::error_code ec;fs::remove(tmp,ec);throw;}
}
} // namespace gptvoice
