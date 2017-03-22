#ifndef __TINY_RENDERER_PHOTON__
#define __TINY_RENDERER_PHOTON__

#include <Eigen/Dense>
using namespace Eigen;
#include "RTRVector3D.h"
#include "RTRColor.h"

class RTRRenderElement;

struct Photon
{
public:
    enum Type {
        DIRECT,
        DIFFUSE,
        SPECULAR,
        CAUSTIC,
        DIFFUSE_INTERREFLECTION,
    };
public:
    RTRRenderElement *intersectElement;
    RTRVector3D location;
    RTRVector3D direction;
    RTRColor color;
    Type type;
};

#endif // __TINY_RENDERER_PHOTON__
