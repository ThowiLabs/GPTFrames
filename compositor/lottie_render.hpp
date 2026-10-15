#pragma once
// Optional ThorVG CPU backend: native Lottie playback on the C++ timeline.
// Each render worker owns its ThorVG canvas; no browser or subprocess.
#ifdef HFC_WITH_THORVG
#include <thorvg.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>
namespace gptframes_lottie {
inline void initialize(){
 static std::once_flag once;
 std::call_once(once,[](){
  if(tvg::Initializer::init(0)!=tvg::Result::Success)
   throw std::runtime_error("ThorVG initialization failed");
 });
}
struct Playback {
 int width,height;
 std::unique_ptr<tvg::Animation> animation;
 std::unique_ptr<tvg::SwCanvas> canvas;
 std::vector<uint32_t> pixels;
 Playback(const std::string& path,int w,int h):width(w),height(h),pixels(size_t(w)*h,0){
  initialize();
  animation.reset(tvg::Animation::gen());
  if(!animation || animation->picture()->load(path.c_str())!=tvg::Result::Success)
   throw std::runtime_error("Lottie file cannot be loaded: "+path);
  if(animation->picture()->size(float(w),float(h))!=tvg::Result::Success)
   throw std::runtime_error("Lottie cannot be sized");
  canvas.reset(tvg::SwCanvas::gen());
  if(!canvas||canvas->target(pixels.data(),w,w,h,tvg::ColorSpace::ABGR8888S)!=tvg::Result::Success)
   throw std::runtime_error("Lottie raster target failed");
  if(canvas->add(animation->picture())!=tvg::Result::Success)
   throw std::runtime_error("Lottie canvas insert failed");
 }
 Playback(const Playback&)=delete;
 Playback& operator=(const Playback&)=delete;
 void render(double seconds,double speed,bool loop){
  double duration=animation->duration();
  if(!(duration>0&&std::isfinite(duration)))throw std::runtime_error("Lottie duration invalid");
  double t=std::max(0.,seconds*speed);
  if(loop)t=std::fmod(t,duration);
  else t=std::min(t,std::nextafter(duration,0.));
  const float frame=std::clamp(float(t/duration*animation->totalFrame()),
                         0.f,std::max(0.f,animation->totalFrame()-0.001f));
  animation->frame(frame);
  if(canvas->update()!=tvg::Result::Success)
   throw std::runtime_error("Lottie frame update failed");
  if(canvas->draw(true)!=tvg::Result::Success||canvas->sync()!=tvg::Result::Success)
   throw std::runtime_error("Lottie CPU frame failed");
 }
};
inline const Playback& draw(const std::string& path,int width,int height,
                            double seconds,double rate,bool loop){
 if(width<1||height<1||width>1600||height>1600||int64_t(width)*height>1900000)
  throw std::runtime_error("Lottie frame exceeds 1.9 Mpixels");
 const std::string token=path+"|"+std::to_string(width)+"|"+std::to_string(height);
 static thread_local std::map<std::string,std::unique_ptr<Playback>> cache;
 auto i=cache.find(token);
 if(i==cache.end()){
  if(cache.size()>=4)cache.clear();
  i=cache.emplace(token,std::make_unique<Playback>(path,width,height)).first;
 }
 i->second->render(seconds,rate,loop);
 return *i->second;
}
} // namespace gptframes_lottie
#endif
