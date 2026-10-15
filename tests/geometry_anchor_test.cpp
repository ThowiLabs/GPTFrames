#include "../compositor/geometry_transform.hpp"
#include <stdexcept>
#include <cmath>
#include <iostream>
#include <stdexcept>

struct Pose {double x,y,w,h,opacity,scale,rotation;};
static void check(bool okay){if(!okay)throw std::runtime_error("Geometry anchor regression");}
static bool near(double a,double b){return std::abs(a-b)<1e-7;}
static double cx(const Pose& p){return p.x+p.w*p.scale*.5;}
static double cy(const Pose& p){return p.y+p.h*p.scale*.5;}

int main(){
    const Pose ring{760,360,200,200,1.,1.,0.};
    const Pose arrow{810,410,100,100,1.,1.,0.};
    for(int i=0;i<=100;i++){
        const double phase=6.283185307179586*i/100.;
        const double scale=.9+.15*std::sin(phase);
        Pose shape=ring;shape.scale=scale;
        Pose marker=arrow;marker.scale=scale;
        shape=gptframes_geometry::center_anchored(shape,1920,1080);
        marker=gptframes_geometry::center_anchored(marker,1920,1080);
        check(near(cx(shape),860));check(near(cy(shape),460));
        check(near(cx(marker),860));check(near(cy(marker),460));
        check(near(cx(shape),cx(marker)));
        check(near(cy(shape),cy(marker)));
    }
    // Center of the unscaled bounding box moves with the camera transform,
    // while the point of scale application remains the object's own center.
    Pose moved=gptframes_geometry::center_anchored(ring,1920,1080,1.2,20,-10);
    check(near(cx(moved),(860-960)*1.2+960+20));
    check(near(cy(moved),(460-540)*1.2+540-10));
    check(near(moved.w*moved.scale,240));
    Pose unscaled=gptframes_geometry::center_anchored(ring,1920,1080);
    check(near(unscaled.x,ring.x));check(near(unscaled.y,ring.y));
    check(near(unscaled.scale,1.));
    bool rejected=false;
    try {gptframes_geometry::center_anchored(Pose{0,0,12,12,1,0,0},1920,1080);}
    catch(const std::invalid_argument&){rejected=true;}
    check(rejected);
    std::cout<<"PASS 101 pulse sizes, aligned icon/ring, camera center and legacy unscaled pose\n";
}
