#pragma once
#include "common.hpp"
#include <condition_variable>
#include <thread>
#include <set>

namespace gptframes::mcp {
// Resource discovery never executes markup, reads voice model bytes, or visits remote URLs.
// It polls every two seconds; every user query can force a fresh atomic snapshot.
class ResourceMonitor {
    fs::path root_, models_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    bool stop_=false;
    std::thread worker_;
    Json current_=Json::object();
    static bool is_regular_file_safe(const fs::directory_entry& e){
        std::error_code ec;
        return !e.is_symlink(ec) && e.is_regular_file(ec);
    }
    static Json scan(const fs::path& root,const fs::path& models){
        Json resources={{"models",Json::array()},{"animation_files",Json::array()},
            {"animation_manifests",Json::array()},{"audio",Json::array()},
            {"images",Json::array()},{"video",Json::array()},{"lottie",Json::array()}};
        auto gather=[&](const fs::path& dir,const std::string& category,
                        const std::set<std::string>& allowed,bool recursive){
            std::error_code ec;
            if(!fs::is_directory(dir,ec)||fs::is_symlink(dir,ec))return;
            int seen=0;
            auto visit=[&](const fs::directory_entry& entry){
                if(++seen>4096||!is_regular_file_safe(entry))return;
                const std::string ext=entry.path().extension().string();
                if(!allowed.count(ext))return;
                if(entry.path().filename().u8string().size()>160)return;
                const auto size=entry.file_size(ec);
                if(ec||size==0||size>512ull*1024*1024)return;
                const auto name=entry.path().filename().u8string();
                resources[category].push_back(name);
            };
            if(recursive){
                for(fs::recursive_directory_iterator i(dir,fs::directory_options::skip_permission_denied,ec),end;
                    !ec&&i!=end && seen<4096;i.increment(ec)){
                    if(i->is_symlink(ec))i.disable_recursion_pending();
                    visit(*i);
                }
            }else{
                for(fs::directory_iterator i(dir,fs::directory_options::skip_permission_denied,ec),end;
                    !ec&&i!=end && seen<4096;i.increment(ec))visit(*i);
            }
        };
        gather(models,"models",{".neo",".onnx"},false);
        gather(root/"animaciones"/"presets","animation_files",{".html"},false);
        gather(root/"animaciones"/"collections","animation_manifests",{".json"},false);
        gather(root/"assets","audio",{".wav",".mp3",".flac",".ogg"},true);
        gather(root/"assets","images",{".png",".jpg",".jpeg",".webp",".svg"},true);
        gather(root/"assets","video",{".mp4",".webm",".mov"},true);
        gather(root/"assets","lottie",{".json",".tgs"},false);
        for(const auto& category:{"models","animation_files","animation_manifests","audio","images","video","lottie"}){
            auto& items=resources[category];
            std::sort(items.begin(),items.end());
            items.erase(std::unique(items.begin(),items.end()),items.end());
        }
        Json count=Json::object();
        for(const auto& [key,value]:resources.items())count[key]=value.size();
        auto now=std::chrono::system_clock::now().time_since_epoch();
        const auto timestamp=std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
        return {{"resources",std::move(resources)},{"counts",std::move(count)},
                {"checked_at_unix_ms",timestamp},{"watch_interval_seconds",2},
                {"discovery","local filesystem only"},{"live",true}};
    }
    void update(){
        auto data=scan(root_,models_);
        std::lock_guard<std::mutex> lk(mutex_);
        current_=std::move(data);
    }
public:
    ResourceMonitor(fs::path root,fs::path models):root_(std::move(root)),models_(std::move(models)){
        update();
        worker_=std::thread([this](){
            std::unique_lock<std::mutex> lock(mutex_);
            while(!stop_){
                if(cv_.wait_for(lock,std::chrono::seconds(2),[this]{return stop_;}))break;
                lock.unlock();
                try{update();}catch(const std::exception&){ /* keep last known valid snapshot */ }
                lock.lock();
            }
        });
    }
    ~ResourceMonitor(){
        {
          std::lock_guard<std::mutex> lk(mutex_);
          stop_=true;
        }
        cv_.notify_one();
        if(worker_.joinable())worker_.join();
    }
    ResourceMonitor(const ResourceMonitor&)=delete;
    ResourceMonitor& operator=(const ResourceMonitor&)=delete;
    Json snapshot(bool force=false){
        if(force)update();
        std::lock_guard<std::mutex> lk(mutex_);
        return current_;
    }
    Json models(bool force=true){return snapshot(force)["resources"]["models"];}
};
} // namespace gptframes::mcp
