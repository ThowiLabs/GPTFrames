#pragma once
// Layout QA shared by command line and MCP. Non-mutating and deterministic.
#include "json_project.hpp"
#include <cmath>
#include <vector>
#include <string>

namespace gptframes_quality {
using json=nlohmann::json;
inline double luminance(project::Color c){
 auto f=[](unsigned char b){double s=b/255.0;return s<=.04045?s/12.92:std::pow((s+.055)/1.055,2.4);};
 return .2126*f(c.r)+.7152*f(c.g)+.0722*f(c.b);
}
inline double contrast(project::Color a,project::Color b){
 auto x=luminance(a),y=luminance(b);
 return (std::max(x,y)+.05)/(std::min(x,y)+.05);
}
inline json report(const project::Config& cfg){
 json issues=json::array();
 FontPainter font;
 for(size_t si=0;si<cfg.scenes.size();++si){
  const auto& scene=cfg.scenes[si];
  std::vector<std::pair<project::Pose,size_t>> textRects;
  for(size_t li=0;li<scene.layers.size();++li){
   const auto& l=scene.layers[li];
   const auto p=project::sample(l,std::max(0.,std::min(scene.duration/2.,l.duration)-l.from));
   auto warn=[&](const std::string&code,const std::string&detail){
    issues.push_back({{"scene",si+1},{"layer",li+1},{"code",code},{"detail",detail}});
   };
   if(l.type=="image" && l.image){
    const double bw=p.w*p.scale,bh=p.h*p.scale;
    const auto& img=*l.image;
    const double sourceFraction=gptframes_image_fit::visible_fraction_cover(bw,bh,img.width,img.height);
    if(l.imageFit=="cover" && sourceFraction<.80)
     warn("image_cropped","El modo cover oculta más del 20% del recurso: usar fit=contain o revisar el encuadre.");
    if(l.imageFit=="stretch" && sourceFraction<.88)
     warn("image_distorted","Stretch deforma la proporción original de la imagen: usar contain salvo intención explícita.");
    // An opaque card drawn later can cover almost all of a photograph.
    // This is different from deliberate, minimal accent-line overlays.
    const double imageArea=std::max(0.,bw*bh);
    if(imageArea>0)for(size_t later=li+1;later<scene.layers.size();++later){
     const auto& occluder=scene.layers[later];
     if(occluder.type!="rect")continue;
     const auto q=project::sample(occluder,std::max(0.,std::min(scene.duration/2.,occluder.duration)-occluder.from));
     if(q.opacity<.97)continue;
     const double ow=std::max(0.,std::min(p.x+bw,q.x+q.w*q.scale)-std::max(p.x,q.x));
     const double oh=std::max(0.,std::min(p.y+bh,q.y+q.h*q.scale)-std::max(p.y,q.y));
     if(ow*oh/imageArea>.55){
      warn("image_occluded","Un rectángulo posterior tapa más del 55% de la imagen; separar fotografía y tarjeta.");
      break;
     }
    }
   }
   if(l.type=="text"){
    const int px=std::max(8,int(std::round(l.fontSize*p.scale)));
    if(px<cfg.height/48) warn("small_text","Texto pequeño para la altura del video.");
    if(p.w*p.scale<32||p.h*p.scale<px*.95)warn("small_box","Caja de texto menor que su contenido.");
    // Wide strings can be wrapped/shrunk in the renderer. Advise earlier authoring review.
    if(!l.wrap){
     double maxLine=0;std::istringstream lines(l.text);
     std::string item;
     while(std::getline(lines,item))maxLine=std::max(maxLine,double(font.measure(item,px,l.bold)));
     if(maxLine>p.w*p.scale*1.05) warn("text_overflow","Texto excede el espacio asignado y tendrá que reducirse.");
    }
    // Test against the visible solid card behind the label, not only the
    // scene background. This avoids false warnings on dark text over bright
    // call-to-action buttons while preserving true low-contrast findings.
    project::Color backdrop=scene.background;
    const double cx=p.x+p.w*p.scale*.5,cy=p.y+p.h*p.scale*.5;
    for(size_t bi=li;bi>0;--bi){
     const auto& box=scene.layers[bi-1];
     if(box.type!="rect")continue;
     const auto q=project::sample(box,std::max(0.,std::min(scene.duration/2.,box.duration)-box.from));
     if(q.opacity<.99)continue;
     if(cx>=q.x && cx<=q.x+q.w*q.scale && cy>=q.y && cy<=q.y+q.h*q.scale){
      backdrop=box.color;break;
     }
    }
    if(contrast(l.color,backdrop)<3.1 && l.color.r!=0)
     warn("low_contrast","Contraste insuficiente sobre el fondo visible.");
    textRects.push_back({p,li});
   }
   if(l.type!="glow"&&l.type!="grid"&&l.type!="line"&&
      (p.x<0||p.y<0||p.x+p.w*p.scale>cfg.width+.5||p.y+p.h*p.scale>cfg.height+.5))
    warn("outside_canvas","El objeto rebasa la región visible.");
  }
  for(size_t i=0;i<textRects.size();++i)for(size_t j=i+1;j<textRects.size();++j){
   const auto& a=textRects[i].first;const auto& b=textRects[j].first;
   const double w=std::max(0.,std::min(a.x+a.w*a.scale,b.x+b.w*b.scale)-std::max(a.x,b.x));
   const double h=std::max(0.,std::min(a.y+a.h*a.scale,b.y+b.h*b.scale)-std::max(a.y,b.y));
   const double area=std::min(a.w*a.h*a.scale*a.scale,b.w*b.h*b.scale*b.scale);
   if(area>0 && w*h>area*.18)
    issues.push_back({{"scene",si+1},{"layer",textRects[j].second+1},
       {"code","text_collision"},{"detail","Posible superposición de cuadros de texto."}});
  }
 }
 return {{"passed",issues.empty()},{"scene_count",cfg.scenes.size()},
         {"warnings",issues},{"warning_count",issues.size()}};
}
} // namespace gptframes_quality
