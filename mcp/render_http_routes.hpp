#pragma once
#include "render_viewer.hpp"

namespace gptframes::mcp {
inline void configure_render_http(httplib::Server& server,Service& service,
                                  const std::shared_ptr<OAuth>& oauth){
 const auto cookie=[&service](const httplib::Request& request,const std::string& id){
  std::istringstream input(request.get_header_value("Cookie"));
  std::string part,secret;
  while(std::getline(input,part,';')){
   part=trim(part);
   if(part.compare(0,14,"gptframes_job=")==0)secret=part.substr(14);
  }
  return service.render_jobs().authorized(id,secret);
 };
 const auto headers=[](httplib::Response& response){
  response.set_header("Cache-Control","no-store");
  response.set_header("X-Content-Type-Options","nosniff");
  response.set_header("Referrer-Policy","no-referrer");
 };
 server.Get(R"(/renders/([A-Za-z0-9_-]{22}))",
  [headers](const httplib::Request&,httplib::Response& response){
   headers(response);
   response.set_header("Content-Security-Policy","default-src 'none'; style-src 'unsafe-inline'; script-src 'unsafe-inline'; connect-src 'self'; media-src 'self'; img-src 'self'; base-uri 'none'; frame-ancestors 'none'");
   response.set_content(viewer_html(),"text/html; charset=utf-8");
  });
 server.Post(R"(/renders/([A-Za-z0-9_-]{22})/session)",
  [&service,oauth,headers](const httplib::Request& request,httplib::Response& response){
   headers(response);
   const auto id=request.matches[1].str();
   if(request.body.size()>256||request.get_header_value("X-GPTFrames-Action")!="session"||
      (!request.get_header_value("Origin").empty()&&!oauth->owns_origin(request.get_header_value("Origin")))){
    response.status=403;return;
   }
   const auto data=Json::parse(request.body,nullptr,false);
   const auto token=data.is_object()?data.value("token",Json{}):Json{};
   if(!token.is_string()||!service.render_jobs().authorized(id,token.get<std::string>())){
    response.status=403;return;
   }
   response.set_header("Set-Cookie","gptframes_job="+token.get<std::string>()+
     "; Path=/renders/"+id+"; HttpOnly; Secure; SameSite=Strict; Max-Age=86400");
   response.set_content(R"({"ok":true})","application/json");
  });
 server.Get(R"(/renders/([A-Za-z0-9_-]{22})/status)",
  [&service,cookie,headers](const httplib::Request& request,httplib::Response& response){
   headers(response);
   const auto id=request.matches[1].str();
   if(!cookie(request,id)){response.status=403;return;}
   try{response.set_content(service.render_jobs().status(id).dump(),"application/json");}
   catch(const std::exception&){response.status=404;}
  });
 server.Post(R"(/renders/([A-Za-z0-9_-]{22})/cancel)",
  [&service,cookie,headers](const httplib::Request& request,httplib::Response& response){
   headers(response);
   const auto id=request.matches[1].str();
   if(!cookie(request,id)||request.get_header_value("X-GPTFrames-Action")!="cancel"){
    response.status=403;return;
   }
   try{response.set_content(service.render_jobs().cancel(id).dump(),"application/json");}
   catch(const std::exception&){response.status=404;}
  });
 server.Get(R"(/renders/([A-Za-z0-9_-]{22})/video)",
  [&service,cookie,headers](const httplib::Request& request,httplib::Response& response){
   headers(response);
   const auto id=request.matches[1].str();
   if(!cookie(request,id)){response.status=403;return;}
   const auto path=service.render_jobs().video(id);
   if(path.empty()||!fs::is_regular_file(path)){response.status=404;return;}
   if(request.has_param("download"))
    response.set_header("Content-Disposition","attachment; filename=\"gptframes-video.mp4\"");
   response.set_header("Accept-Ranges","bytes");
   response.set_file_content(path.u8string(),"video/mp4");
  });
}
} // namespace gptframes::mcp
