#pragma once
// Bounded, durable native render queue. No shell, Python, Node or FFmpeg.
#include "process.hpp"
#include "oauth_crypto.hpp"
#include <condition_variable>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <atomic>
#include <thread>
#include <ctime>
#include <cstdio>

namespace gptframes::mcp {
class RenderQueue {
 struct Job {
  std::string id,hash,project,status="queued",error;
  fs::path directory;
  double percent=0,seconds=0;
  std::int64_t created=0;
  std::atomic_bool cancel{false};
 };
 fs::path directory_,engine_;
 mutable std::mutex mutex_;
 std::condition_variable condition_;
 std::map<std::string,std::shared_ptr<Job>> jobs_;
 std::deque<std::string> pending_;
 std::thread worker_;
 bool stopping_=false;
 static std::int64_t now(){return static_cast<std::int64_t>(std::time(nullptr));}
 static bool valid_id(const std::string& id){
  return id.size()==22&&std::all_of(id.begin(),id.end(),[](unsigned char c){
   return ascii_alnum(c)||c=='-'||c=='_';});
 }
 static Json summary(const Job& j){
  return {{"id",j.id},{"project",j.project},{"status",j.status},
   {"progress",j.percent},{"seconds",j.seconds},{"created",j.created},
   {"error",j.error},{"ready",j.status=="completed"}};
 }
 void save(const Job& j)const{
  Json record=summary(j);
  record["token_hash"]=j.hash;
  atomic_write(j.directory/"state.json",record.dump(2)+"\n");
  fs::permissions(j.directory/"state.json",fs::perms::owner_read|fs::perms::owner_write,
                  fs::perm_options::replace);
 }
 void process(const std::shared_ptr<Job>& j){
  std::string buffer;
  try{
   auto output=[&](const std::string& data){
    buffer+=data;
    while(true){
     const auto at=buffer.find('\n');
     if(at==std::string::npos)break;
     const auto line=buffer.substr(0,at);
     buffer.erase(0,at+1);
     int done=-1,total=-1;
     if(std::sscanf(line.c_str(),"JSON project %d/%d",&done,&total)==2&&total>0){
      std::lock_guard<std::mutex> lock(mutex_);
      auto next=std::clamp(100.*done/total,0.,99.);
      if(next>=j->percent+2.){j->percent=next;save(*j);}
     }
    }
    if(buffer.size()>4096)buffer.clear();
   };
   const auto file=(j->directory/"project.json").u8string();
   const auto video=(j->directory/"video.mp4").u8string();
   std::vector<std::string> argv{engine_.u8string(),"--project","render",file,video};
   if(j->seconds>0){
    std::ostringstream value;value<<std::setprecision(8)<<j->seconds;
    argv.push_back(value.str());
   }
   const auto result=run_process(argv,7200,{},output,&j->cancel);
   std::lock_guard<std::mutex> lock(mutex_);
   if(result.cancelled||j->cancel.load()){j->status="cancelled";j->error="Cancelled by owner";}
   else if(result.exit_code==0&&fs::is_regular_file(video)&&fs::file_size(video)>1024){
    j->status="completed";j->percent=100.;
   }else{
    j->status="failed";j->error=result.timed_out?"Rendering timeout":
      (result.output.empty()?"Native renderer failed":result.output.substr(result.output.size()>700?result.output.size()-700:0));
   }
   if(j->status!="completed"){std::error_code ec;fs::remove(video,ec);}
   save(*j);
  }catch(const std::exception& error){
   std::lock_guard<std::mutex> lock(mutex_);
   j->status=j->cancel?"cancelled":"failed";
   j->error=std::string(error.what()).substr(0,400);
   save(*j);
  }
 }
 void run(){
  while(true){
   std::shared_ptr<Job> job;
   {
    std::unique_lock<std::mutex> lock(mutex_);
    condition_.wait(lock,[&]{return stopping_||!pending_.empty();});
    if(stopping_)return;
    const auto id=pending_.front();pending_.pop_front();
    job=jobs_.at(id);
    if(job->cancel){job->status="cancelled";save(*job);continue;}
    job->status="running";save(*job);
   }
   process(job);
  }
 }
 static void owner_only(const fs::path& path){
  fs::permissions(path,fs::perms::owner_all,fs::perm_options::replace);
 }
 public:
 explicit RenderQueue(const fs::path& root,const fs::path& engine)
   :directory_(root/"build/runtime/render-jobs"),engine_(engine){
  fs::create_directories(directory_);
  owner_only(directory_);
  for(const auto& entry:fs::directory_iterator(directory_)){
   if(!entry.is_directory()||entry.is_symlink()||!valid_id(entry.path().filename().string()))continue;
   try{
    const auto record=Json::parse(read_file(entry.path()/"state.json",8192));
    auto j=std::make_shared<Job>();
    j->id=entry.path().filename().string();
    j->hash=record.at("token_hash").get<std::string>();
    j->project=record.at("project").get<std::string>();
    j->status=record.at("status").get<std::string>();
    j->created=record.at("created").get<std::int64_t>();
    j->percent=record.at("progress").get<double>();
    j->seconds=record.value("seconds",0.);
    j->error=record.value("error","");
    j->directory=entry.path();
    if(j->status=="running"||j->status=="queued"){
     j->status="interrupted";j->error="Server restarted; launch a new render.";save(*j);
    }
    if(j->created+86400>now())jobs_[j->id]=j;
   }catch(const std::exception&){ /* Ignore invalid/partial job state. */ }
  }
  worker_=std::thread([this]{run();});
 }
 RenderQueue(const RenderQueue&)=delete;
 RenderQueue& operator=(const RenderQueue&)=delete;
 ~RenderQueue(){
  {
   std::lock_guard<std::mutex> lock(mutex_);
   stopping_=true;
   for(auto& [id,j]:jobs_)if(j->status=="running")j->cancel=true;
  }
  condition_.notify_all();
  if(worker_.joinable())worker_.join();
 }
 Json start(const std::string& project,const fs::path& file,double seconds,const std::string& origin){
  const auto secret=oauth_random_token();
  auto job=std::make_shared<Job>();
  job->id=oauth_base64url(oauth_random_bytes(16));
  job->hash=oauth_digest(secret);
  job->project=project;
  job->seconds=seconds;
  job->created=now();
  job->directory=directory_/job->id;
  {
   std::lock_guard<std::mutex> lock(mutex_);
   if(pending_.size()>=4)throw std::runtime_error("Render queue full (4 pending maximum)");
   fs::create_directory(job->directory);
   owner_only(job->directory);
   // Freeze the validated composition file for predictable rendering.
   fs::copy_file(file,job->directory/"project.json");
   fs::permissions(job->directory/"project.json",fs::perms::owner_read|fs::perms::owner_write,
                   fs::perm_options::replace);
   // Snapshot local assets into the private job directory. Never allow a
   // symlink escape or a changing source during an active render.
   const auto assets=file.parent_path()/"assets";
   if(fs::is_directory(assets)){
    std::uintmax_t bytes=0;
    for(const auto& entry:fs::recursive_directory_iterator(assets)){
     if(entry.is_symlink())throw std::runtime_error("Render assets cannot contain symlinks");
     if(entry.is_regular_file()){
      bytes+=entry.file_size();
      if(bytes>512*1024*1024)throw std::runtime_error("Render assets exceed 512 MB");
     }
    }
    fs::copy(assets,job->directory/"assets",fs::copy_options::recursive);
   }
   save(*job);
   jobs_[job->id]=job;
   pending_.push_back(job->id);
  }
  condition_.notify_one();
  auto info=summary(*job);
  info["viewer_url"]=origin+"/renders/"+job->id+"#access="+secret;
  return info;
 }
 Json status(const std::string& id)const{
  std::lock_guard<std::mutex> lock(mutex_);
  const auto it=jobs_.find(id);
  if(it==jobs_.end())throw std::runtime_error("Unknown or expired render job");
  return summary(*it->second);
 }
 bool authorized(const std::string& id,const std::string& secret)const{
  if(!valid_id(id)||secret.size()!=43)return false;
  std::lock_guard<std::mutex> lock(mutex_);
  auto it=jobs_.find(id);
  if(it==jobs_.end()||it->second->created+86400<now())return false;
  const std::string digest=oauth_digest(secret);
  if(digest.size()!=it->second->hash.size())return false;
  unsigned char diff=0;
  for(size_t i=0;i<digest.size();++i)diff|=static_cast<unsigned char>(digest[i]^it->second->hash[i]);
  return diff==0;
 }
 Json cancel(const std::string& id){
  std::lock_guard<std::mutex> lock(mutex_);
  auto it=jobs_.find(id);
  if(it==jobs_.end())throw std::runtime_error("Render job not found");
  auto& job=*it->second;
  if(job.status=="running"||job.status=="queued")job.cancel=true;
  if(job.status=="queued"){job.status="cancelled";save(job);}
  return summary(job);
 }
 fs::path video(const std::string& id)const{
  std::lock_guard<std::mutex> lock(mutex_);
  auto it=jobs_.find(id);
  if(it==jobs_.end()||it->second->status!="completed")return {};
  return it->second->directory/"video.mp4";
 }
};
} // namespace gptframes::mcp
