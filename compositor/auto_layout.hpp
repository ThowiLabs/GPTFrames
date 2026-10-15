#pragma once
// Native deterministic group layout. Geometry is resolved before rendering;
// no browser, JS, CSS engine, or external layout service is required.
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>
#include "../vendor/browser/json.hpp"

namespace gptframes_layout {
using json = nlohmann::json;

inline double quantity(const json& j, const char* key, double fallback) {
    if (!j.contains(key)) return fallback;
    if (!j.at(key).is_number()) throw std::invalid_argument(std::string("Layout requires numeric ") + key);
    double n = j.at(key).get<double>();
    if (!std::isfinite(n) || std::abs(n)>100000.) throw std::invalid_argument("Non-finite or oversized layout value");
    return n;
}
inline std::string option(const json& j, const char* key, const std::string& fallback) {
    if (!j.contains(key)) return fallback;
    if (!j.at(key).is_string()) throw std::invalid_argument("Layout option must be text");
    return j.at(key).get<std::string>();
}
inline bool enabled(const json& j, const char* key) {
    return j.contains(key) && j.at(key).is_boolean() && j.at(key).get<bool>();
}
inline double text_width(const json& item, double height) {
    const std::string text=option(item,"text","");
    const double size=quantity(item,"fontSize",std::min(height,48.));
    // Width only determines the flex item. Text is centered using actual
    // font metrics in the rasterizer.
    return std::max(12.,std::min(3500.,size*0.61*double(text.size())));
}
inline double preferred(const json& j, const char* field, double fallback) {
    return j.contains(field)?quantity(j,field,fallback):fallback;
}
inline std::vector<json> flattened(const json& children, double ox=0., double oy=0., int depth=0) {
    if (!children.is_array() || depth>8 || children.size()>100)
        throw std::invalid_argument("Layout requires <=100 children and depth <=8");
    std::vector<json> result;
    for (const auto& child:children) {
        if (!child.is_object()) throw std::invalid_argument("Invalid layout child");
        const std::string type=option(child,"type","");
        if (type!="group") {
            json item=child;
            item["x"]=quantity(child,"x",0.)+ox;
            item["y"]=quantity(child,"y",0.)+oy;
            result.push_back(std::move(item));
            continue;
        }
        if (!child.contains("children") || !child["children"].is_array())
            throw std::invalid_argument("A group requires children[]");
        const double x=ox+quantity(child,"x",0.), y=oy+quantity(child,"y",0.);
        const double w=quantity(child,"width",320.),h=quantity(child,"height",180.);
        const double p=quantity(child,"padding",0.),gap=quantity(child,"gap",0.);
        if (w<=0||h<=0||p<0||gap<0||2*p>=w||2*p>=h)
            throw std::invalid_argument("Invalid group geometry or padding");
        const double ix=x+p,iy=y+p,iw=w-2*p,ih=h-2*p;
        const auto& items=child["children"];
        const std::string direction=option(child,"direction","overlay");
        const std::string justify=option(child,"justify","center");
        const std::string align=option(child,"align","center");
        if (direction!="overlay"&&direction!="row"&&direction!="column"&&direction!="grid")
            throw std::invalid_argument("Layout direction must be overlay, row, column or grid");
        if (justify!="start"&&justify!="center"&&justify!="end"&&justify!="space-between")
            throw std::invalid_argument("Unknown layout justification");
        if (align!="start"&&align!="center"&&align!="end"&&align!="stretch")
            throw std::invalid_argument("Unknown layout alignment");
        if (items.empty()) continue;
        const int count=static_cast<int>(items.size());
        const int columns=std::clamp(static_cast<int>(quantity(child,"columns",2)),1,24);
        const int rows=(count+columns-1)/columns;
        const double cellw=(iw-gap*std::max(0,columns-1))/columns;
        const double cellh=(ih-gap*std::max(0,rows-1))/rows;
        if (cellw<=0||cellh<=0) throw std::invalid_argument("Insufficient grid space");
        const bool horizontal=direction=="row";
        const bool vertical=direction=="column";
        const double major=horizontal?iw:ih;
        const double minor=horizontal?ih:iw;
        double sizes=0, weights=0;
        std::vector<double> fixed(count,0);
        if (horizontal||vertical) for (int i=0;i<count;i++) {
            const auto& item=items[i];
            if (!item.is_object()) throw std::invalid_argument("Invalid layout child");
            const char* key=horizontal?"width":"height";
            if (item.contains(key)) { fixed[i]=quantity(item,key,0.);sizes+=fixed[i]; }
            else weights+=std::max(0.01,quantity(item,"grow",1.));
        }
        double available=major-sizes-gap*(count-1);
        if ((horizontal||vertical)&&available < -0.01)
            throw std::invalid_argument("Children exceed group main axis");
        if ((horizontal||vertical)&&weights>0)
            for (int i=0;i<count;i++) if(fixed[i]==0)
                fixed[i]=std::max(1.,available*std::max(0.01,quantity(items[i],"grow",1.))/weights);
        double occupied=(count-1)*gap;
        if (horizontal||vertical) for (double f:fixed) occupied+=f;
        double free=std::max(0.,major-occupied);
        double cursor=justify=="center"?free/2:justify=="end"?free:0.;
        double spacing=gap+(justify=="space-between"&&count>1?free/(count-1):0.);
        for (int i=0;i<count;i++) {
            json item=items[i];
            if (!item.is_object()) throw std::invalid_argument("Invalid layout child");
            const std::string kind=option(item,"type","");
            double bx=ix,by=iy,bw=iw,bh=ih;
            if (direction=="grid") {
                bx=ix+(i%columns)*(cellw+gap);
                by=iy+(i/columns)*(cellh+gap);
                bw=cellw;bh=cellh;
            } else if (horizontal) {bx=ix+cursor;bw=fixed[i];cursor+=bw+spacing;}
            else if (vertical) {by=iy+cursor;bh=fixed[i];cursor+=bh+spacing;}
            const double raww=preferred(item,"width",direction=="overlay"?
                  (kind=="text"?text_width(item,bh):bw):bw);
            const double rawh=preferred(item,"height",kind=="text"?
                  quantity(item,"fontSize",48.)*1.22:bh);
            const double cw=(horizontal?bw:(align=="stretch"&&!item.contains("width")?bw:raww));
            const double ch=(vertical?bh:(align=="stretch"&&!item.contains("height")?bh:rawh));
            if (cw<=0||ch<=0) throw std::invalid_argument("Layout child has non-positive dimensions");
            // On the main axis (row/column), distribution is handled by cursor.
            // The other axis, and both axes of overlay, use the parent's alignment.
            const bool align_x=(direction=="overlay"||vertical||direction=="grid");
            const bool align_y=(direction=="overlay"||horizontal||direction=="grid");
            const double offx=align=="center"?(bw-cw)/2:align=="end"?(bw-cw):0;
            const double offy=align=="center"?(bh-ch)/2:align=="end"?(bh-ch):0;
            bx+=align_x?offx:0;by+=align_y?offy:0;
            if (kind=="group") {
                item["width"]=cw;item["height"]=ch;
                item["x"]=bx-ox;item["y"]=by-oy;
                auto nested=flattened(json::array({item}),ox,oy,depth+1);
                if(child.contains("animation"))for(auto& sub:nested)
                    if(!sub.contains("animation"))sub["animation"]=child["animation"];
                result.insert(result.end(),nested.begin(),nested.end());
            } else {
                item["x"]=bx+quantity(item,"offsetX",0.);
                item["y"]=by+quantity(item,"offsetY",0.);
                item["width"]=cw;item["height"]=ch;
                item.erase("offsetX");item.erase("offsetY");item.erase("grow");
                if (kind=="text"&&!item.contains("textAlign"))item["textAlign"]="center";
                if (kind=="text"&&!item.contains("verticalAlign"))item["verticalAlign"]="center";
                result.push_back(std::move(item));
            }
        }
    }
    if (result.size()>100) throw std::invalid_argument("Flattened layout exceeds 100 layers");
    return result;
}
} // namespace gptframes_layout
