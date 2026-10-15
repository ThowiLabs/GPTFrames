#pragma once
// Música procedural original para capítulos 7..15, sin muestras de terceros.
// Mezcla mono PCM 24k en memoria con la narración de las primeras 6 escenas.
static double frac(double x){return x-std::floor(x);}
static double softSine(double phase){return std::sin(2*PI*phase);}
static void addShowcaseScore(std::vector<int16_t>& samples){
 const size_t from=size_t(67)*SR;
 if(samples.size()<=from)return;
 const double bpm=104., beat=60./bpm;
 const std::array<std::array<double,3>,4> chords{{
   {{110.,130.8128,164.8138}}, // Am
   {{87.307,110.,130.8128}}, // F
   {{98.0,123.4708,146.832}}, // G
   {{82.4069,103.826,123.4708}} // Em
 }};
 const double total=double(samples.size()-from)/SR;
 for(size_t i=from;i<samples.size();i++){
   const double t=double(i-from)/SR, beatN=t/beat;
   const int phrase=int(t/6.923),ch=phrase%4;
   const auto& notes=chords[ch];
   double pad=0;
   for(int j=0;j<3;j++){
     double f=notes[j]*(j==1?2.:1.);
     pad+=softSine(f*t+.08*softSine(.13*t+j))*.35;
     pad+=softSine(f*2*t+j*.3)*.085;
   }
   double arpDuration=beat/2.;
   int arpN=int(t/arpDuration);
   double f=notes[(arpN+phrase)%3]*(arpN%4==0?4.:2.);
   double arpPhase=frac(t/arpDuration);
   double arp=softSine(f*t)*std::exp(-arpPhase*5.)*.38;
   double beatPhase=frac(beatN);
   double bass=softSine(notes[0]*t*.5)*.27;
   double kick=softSine((48.+48*std::exp(-beatPhase*22.))*beatPhase*beat)*std::exp(-beatPhase*29.)*.40;
   double noise=frac(std::sin(double(i)*12.9898)*43758.5453123)*2.-1.;
   double hhPhase=frac(t/(beat/2.));
   double hat=noise*std::exp(-hhPhase*35.)*.065;
   double snPhase=frac((t+beat)/(beat*2.));
   double snare=(noise*.10+softSine(170*t)*.04)*std::exp(-snPhase*27.)*(beatPhase>.48?.8:.25);
   double sum=pad+arp+bass+kick+hat+snare;
   // Pulse every 9 seconds to underscore the change of chapter.
   const double scenePhase=frac(t/9.);
   if(scenePhase<.060)sum+=noise*.12*std::exp(-scenePhase*60.);
   double amp=std::min(1.,t/1.35)*std::min(1.,(total-t)/2.0);
   auto add=int(std::llround(sum*5000.*std::max(0.,amp)));
   int v=int(samples[i])+add;
   samples[i]=int16_t(std::clamp(v,-32768,32767));
 }
}
