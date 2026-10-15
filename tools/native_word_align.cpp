// Native approximate word timing from real PCM and the exact spoken script.
// This is NOT forced alignment and cannot claim phoneme/letter precision.
#include "../vendor/browser/json.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
using json=nlohmann::json;
static uint32_t le32(const unsigned char*p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
static uint16_t le16(const unsigned char*p){return uint16_t(p[0])|(uint16_t(p[1])<<8);}
struct Audio{int rate;std::vector<int16_t> pcm;};
static Audio readWav(const std::string& path){
 std::ifstream f(path,std::ios::binary);
 std::vector<unsigned char> bytes(std::istreambuf_iterator<char>(f),{});
 if(bytes.size()<44||bytes.size()>60*1024*1024||std::string(reinterpret_cast<char*>(bytes.data()),4)!="RIFF"||
    std::string(reinterpret_cast<char*>(bytes.data()+8),4)!="WAVE")throw std::runtime_error("PCM WAV inválido");
 int rate=0,channel=0,bits=0,format=0;
 size_t data=0, size=0;
 for(size_t p=12;p+8<=bytes.size();){
  const uint32_t n=le32(bytes.data()+p+4);
  if(n>bytes.size()-p-8)throw std::runtime_error("Invalid WAV chunk");
  const std::string tag(reinterpret_cast<char*>(bytes.data()+p),4);
  if(tag=="fmt "&&n>=16){
   format=le16(bytes.data()+p+8);channel=le16(bytes.data()+p+10);
   rate=int(le32(bytes.data()+p+12));bits=le16(bytes.data()+p+22);
  }
  if(tag=="data"){data=p+8;size=n;}
  p+=8+n+(n&1);
 }
 if(format!=1||bits!=16||channel!=1||rate<8000||rate>192000||!data||size%2)
  throw std::runtime_error("Solo WAV PCM16 mono");
 Audio out{rate,{}};
 out.pcm.reserve(size/2);
 for(size_t p=data;p<data+size;p+=2)out.pcm.push_back(int16_t(le16(bytes.data()+p)));
 return out;
}
struct Word{std::string value;int codepoints=0;int punctuation=0;};
static std::vector<Word> tokenize(const std::string& input){
 std::vector<Word> result;
 Word current;
 auto flush=[&]{if(!current.value.empty()){result.push_back(current);current={};}};
 for(size_t i=0;i<input.size();){
  unsigned char c=input[i],n=(c&0x80)==0?1:(c&0xE0)==0xC0?2:(c&0xF0)==0xE0?3:4;
  if(i+n>input.size())n=1;
  bool letter=(c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9')||c>=128;
  if(letter){current.value+=input.substr(i,n);current.codepoints++;i+=n;continue;}
  flush();
  if(!result.empty()){
   if(c=='.'||c=='!'||c=='?')result.back().punctuation=3;
   else if(c==','||c==';'||c==':')result.back().punctuation=1;
  }
  i++;
 }
 flush();
 return result;
}
static json align(const std::string& script,const Audio& wav){
 const auto words=tokenize(script);
 if(words.empty())throw std::runtime_error("Guion sin palabras");
 const int rate=wav.rate;
 const double total=double(wav.pcm.size())/rate;
 const size_t hop=std::max(1,rate/100);
 const size_t windows=(wav.pcm.size()+hop-1)/hop;
 std::vector<double> energy(windows,0);
 for(size_t w=0;w<windows;w++){
  const auto a=w*hop,b=std::min(a+hop,wav.pcm.size());
  for(size_t n=a;n<b;n++)energy[w]+=std::abs(int(wav.pcm[n]))/32768.;
  energy[w]/=std::max(size_t(1),b-a);
 }
 double weightSum=0;
 std::vector<double> weights;
 for(const auto& word:words){
  // Punctuation adds a short pause; letter count is a timing proxy only.
  double weight=1.+std::pow(double(word.codepoints),.82)+(word.punctuation==3?2.0:word.punctuation==1?.85:0.);
  weights.push_back(weight);weightSum+=weight;
 }
 json entries=json::array();
 double previous=0,cumulative=0;
 for(size_t i=0;i<words.size();i++){
  cumulative+=weights[i];
  const double nominal=(i+1==words.size()?total:total*cumulative/weightSum);
  double boundary=nominal;
  if(i+1<words.size()&&windows>10){
   const int expected=std::clamp(int(nominal*rate/hop),1,int(windows)-2);
   const int rad=std::clamp(int(rate*.11/hop),2,14);
   const int low=std::max(1,expected-rad),high=std::min(int(windows)-2,expected+rad);
   double mean=0;
   for(int k=low;k<=high;k++)mean+=energy[size_t(k)];
   mean/=(high-low+1);
   int best=expected;double lowest=std::numeric_limits<double>::max();
   for(int k=low;k<=high;k++){
    const double e=(energy[size_t(k-1)]+energy[size_t(k)]+energy[size_t(k+1)])/3.;
    const double normalized=double(k-expected)/rad;
    const double score=e+mean*.40*normalized*normalized;
    if(score<lowest){lowest=score;best=k;}
   }
   if(lowest<mean*.80)boundary=double(best*hop)/rate;
  }
  boundary=std::clamp(boundary,previous+.001,total);
  entries.push_back({{"word",words[i].value},{"start",previous},{"end",boundary},
       {"confidence","approximate"},{"method","weighted-text+pcm-energy"}});
  previous=boundary;
 }
 return entries;
}
int main(int argc,char**argv){
 try{
  if(argc!=4)throw std::runtime_error("Uso: gptframes-word-align guion.json carpeta_wav salida.json");
  std::ifstream input(argv[1]);json d=json::parse(input);
  json output;output["version"]=1;output["method"]="approximate-pcm-energy";
  output["accuracy"]="Segment WAV boundaries are exact. Word intervals are estimated, not phoneme forced alignment.";
  output["scenes"]=json::array();
  int n=0;
  for(const auto& part:d.at("parts")){
   ++n;const std::string name=(n<10?"0":"")+std::to_string(n);
   auto wav=readWav(std::string(argv[2])+"/"+name+".wav");
   const std::string text=part.at("text").get<std::string>();
   output["scenes"].push_back({{"scene",n},{"duration",double(wav.pcm.size())/wav.rate},
     {"sampleRate",wav.rate},{"words",align(text,wav)}});
  }
  std::ofstream target(argv[3]);if(!target)throw std::runtime_error("Cannot write JSON");
  target<<output.dump(2)<<'\n';
  std::cout<<"NATIVE_WORD_ALIGN_OK scenes="<<n<<'\n';
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
