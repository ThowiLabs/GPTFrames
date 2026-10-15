#pragma once
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
namespace gptframes_asset_data {
namespace fs=std::filesystem;
inline std::string base64_encode(const std::string& input){
 static constexpr char chars[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
 std::string out;out.reserve(((input.size()+2)/3)*4);
 for(size_t i=0;i<input.size();i+=3){
  const unsigned a=static_cast<unsigned char>(input[i]);
  const unsigned b=i+1<input.size()?static_cast<unsigned char>(input[i+1]):0;
  const unsigned c=i+2<input.size()?static_cast<unsigned char>(input[i+2]):0;
  out.push_back(chars[a>>2]);
  out.push_back(chars[((a&3)<<4)|(b>>4)]);
  out.push_back(i+1<input.size()?chars[((b&15)<<2)|(c>>6)]:'=');
  out.push_back(i+2<input.size()?chars[c&63]:'=');
 }
 return out;
}
inline std::string safe_data_url(const fs::path& projectRoot,const std::string& relative){
 fs::path candidate=fs::u8path(relative);
 if(candidate.is_absolute()||candidate.parent_path()!=fs::path("assets")||
    candidate.filename().empty()||relative.find('\\')!=std::string::npos)
  throw std::invalid_argument("Images must use the local project assets/ directory");
 auto ext=candidate.extension().string();
 std::string mime;
 if(ext==".png")mime="image/png";
 else if(ext==".jpg"||ext==".jpeg")mime="image/jpeg";
 else if(ext==".webp")mime="image/webp";
 else throw std::invalid_argument("Unsupported editable image format");
 const auto base=projectRoot/"assets",file=projectRoot/candidate;
 if(!fs::is_directory(base)||fs::is_symlink(base)||fs::is_symlink(file)||
    !fs::is_regular_file(file)||fs::file_size(file)>3*1024*1024||
    fs::weakly_canonical(file).parent_path()!=fs::weakly_canonical(base))
  throw std::invalid_argument("Unsafe or missing local image");
 std::ifstream stream(file,std::ios::binary);
 if(!stream)throw std::runtime_error("Cannot load local image");
 const std::string bytes((std::istreambuf_iterator<char>(stream)),{});
 if(bytes.empty())throw std::invalid_argument("Empty image");
 return "data:"+mime+";base64,"+base64_encode(bytes);
}
}
