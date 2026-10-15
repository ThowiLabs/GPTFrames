#include "../mcp/render_jobs.hpp"
#include <iostream>
#include <cassert>
#include <chrono>
#include <thread>

using namespace gptframes::mcp;
int main(int argc,char** argv){
 if(argc<3)return 2;
 const fs::path root=fs::temp_directory_path()/("gptframes-queue-test-"+oauth_random_token());
 fs::create_directories(root);
 const fs::path engine=fs::absolute(argv[1]);
 const fs::path original=fs::absolute(argv[2])/"examples/json-compositor/showcase.json";
 const std::string origin="https://example.test";
 std::string id;
 try {
  {
   RenderQueue queue(root,engine);
   const auto job=queue.start("fixture",original,1.0,origin);
   id=job["id"].get<std::string>();
   const auto url=job["viewer_url"].get<std::string>();
   assert(url.find(origin+"/renders/"+id+"#access=")==0);
   const auto token=url.substr(url.find("#access=")+8);
   assert(queue.authorized(id,token));
   assert(!queue.authorized(id,std::string(43,'x')));
   assert(!queue.authorized("bad_id",token));
   bool completed=false;
   for(int i=0;i<400;i++){
    auto state=queue.status(id);
    if(state["status"]=="completed"){
     assert(state["progress"]==100.);
     assert(fs::is_regular_file(queue.video(id)));
     assert(fs::file_size(queue.video(id))>1024);
     completed=true;break;
    }
    if(state["status"]=="failed")throw std::runtime_error(state.dump());
    std::this_thread::sleep_for(std::chrono::milliseconds(40));
   }
   assert(completed);
   const auto second=queue.start("fixture",original,1.0,origin);
   const auto id2=second["id"].get<std::string>();
   queue.cancel(id2);
   for(int i=0;i<400;i++){
    auto state=queue.status(id2);
    if(state["status"]=="cancelled")break;
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
   }
   assert(queue.status(id2)["status"]=="cancelled");
   std::cout<<"PASS secure URLs, progress, native render, cancellation\n";
  }
  {
   RenderQueue restarted(root,engine);
   assert(restarted.status(id)["status"]=="completed");
   assert(fs::is_regular_file(restarted.video(id)));
   std::cout<<"PASS recovered persisted completed job\n";
  }
 }catch(const std::exception& e){
  std::cerr<<"FAIL "<<e.what()<<'\n';
  fs::remove_all(root);
  return 1;
 }
 fs::remove_all(root);
 return 0;
}
