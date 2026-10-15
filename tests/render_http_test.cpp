#include "../mcp/http_server.hpp"
#include <thread>
#include <cassert>
#include <iostream>

using namespace gptframes::mcp;
int main(int argc,char**argv){
 if(argc<3)return 2;
 TemporaryDirectory tmp(fs::temp_directory_path(),"gptframes-render-http-");
 const auto root=fs::canonical(argv[2]);
 Config c{root,tmp.path/"projects",tmp.path/"models",fs::absolute(argv[1]),"neo-lite"};
 Service service(c);
 const std::string origin="https://gptframes.example.test";
 auto oauth=std::make_shared<OAuth>(OAuthConfig{"example-secure-pin",origin,{},tmp.path/"oauth.json"});
 httplib::Server server;
 configure_http(server,service,oauth);
 const auto port=server.bind_to_any_port("127.0.0.1");
 if(port<0)return 1;
 std::thread worker([&]{server.listen_after_bind();});
 struct Shutdown{httplib::Server& s;std::thread& t;~Shutdown(){s.stop();if(t.joinable())t.join();}} shutdown{server,worker};
 server.wait_until_ready();
 httplib::Client client("127.0.0.1",port);
 client.set_read_timeout(15);
 const auto job=service.render_jobs().start("sample",root/"examples/json-compositor/showcase.json",1.0,origin);
 const auto id=job.at("id").get<std::string>();
 const auto url=job.at("viewer_url").get<std::string>();
 const auto token=url.substr(url.find("#access=")+8);
 const auto path="/renders/"+id;
 auto public_page=client.Get(path);
 assert(public_page&&public_page->status==200);
 assert(public_page->body.find("GPTFrames")!=std::string::npos);
 assert(public_page->body.find(token)==std::string::npos);
 auto unauth=client.Get(path+"/status");
 assert(unauth&&unauth->status==403);
 auto wrong=client.Post(path+"/session",{{"X-GPTFrames-Action","session"}},
    Json{{"token",std::string(43,'X')}}.dump(),"application/json");
 assert(wrong&&wrong->status==403);
 auto good=client.Post(path+"/session",{{"X-GPTFrames-Action","session"}},
    Json{{"token",token}}.dump(),"application/json");
 assert(good&&good->status==200);
 const auto set=good->get_header_value("Set-Cookie");
 assert(set.find("HttpOnly")!=std::string::npos && set.find("Secure")!=std::string::npos);
 httplib::Headers cookies{{"Cookie","gptframes_job="+token}};
 auto status=client.Get(path+"/status",cookies);
 assert(status&&status->status==200);
 bool ready=false;
 for(int i=0;i<200;i++){
  const auto s=service.render_jobs().status(id);
  if(s["status"]=="completed"){ready=true;break;}
  if(s["status"]=="failed")throw std::runtime_error(s.dump());
  std::this_thread::sleep_for(std::chrono::milliseconds(80));
 }
 assert(ready);
 const auto video=client.Get(path+"/video",cookies);
 assert(video&&video->status==200&&video->body.size()>1024);
 assert(video->get_header_value("Content-Type")=="video/mp4");
 assert(video->body.find("ftyp")!=std::string::npos);
 auto rejected=client.Get(path+"/video");
 assert(rejected&&rejected->status==403);
 const httplib::Headers range_headers{{"Cookie","gptframes_job="+token},{"Range","bytes=0-127"}};
 auto range=client.Get(path+"/video",range_headers);
 assert(range&&(range->status==206||range->status==200));
 std::cout<<"PASS private viewer, fragment exchange, protected status, MP4 playback, range access\n";
}
