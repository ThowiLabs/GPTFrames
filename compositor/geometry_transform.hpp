#pragma once
// Layout coordinates describe unscaled bounding boxes. Rendering geometry
// uses the *center* of that box as its anchor for scale and camera moves.
// This keeps icons, text, circles and sibling elements visually registered.
#include <cmath>
#include <stdexcept>

namespace gptframes_geometry {
template<class Pose>
Pose center_anchored(Pose p,double canvasWidth,double canvasHeight,
                     double cameraZoom=1.,double cameraOffsetX=0.,
                     double cameraOffsetY=0.) {
    if(!std::isfinite(p.x)||!std::isfinite(p.y)||!std::isfinite(p.w)||
       !std::isfinite(p.h)||!std::isfinite(p.scale)||
       !std::isfinite(cameraZoom)||cameraZoom<=0||
       !std::isfinite(cameraOffsetX)||!std::isfinite(cameraOffsetY)||
       p.w<=0||p.h<=0||p.scale<=0||canvasWidth<=0||canvasHeight<=0)
        throw std::invalid_argument("Non-finite or invalid geometry");
    const double originalCenterX=p.x+p.w*.5;
    const double originalCenterY=p.y+p.h*.5;
    const double transformedCenterX=
        (originalCenterX-canvasWidth*.5)*cameraZoom+
        canvasWidth*.5+cameraOffsetX;
    const double transformedCenterY=
        (originalCenterY-canvasHeight*.5)*cameraZoom+
        canvasHeight*.5+cameraOffsetY;
    p.scale*=cameraZoom;
    p.x=transformedCenterX-p.w*p.scale*.5;
    p.y=transformedCenterY-p.h*p.scale*.5;
    return p;
}
} // namespace gptframes_geometry
