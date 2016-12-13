#ifndef __TINY_RENDERER_PHOTON__
#define __TINY_RENDERER_PHOTON__

#include "RTRVector3D.h"
#include "RTRColor.h"

struct Photon
{
    enum Type {
        DIRECT,
        DIFFUSE,
        SPECULAR,
        CAUSTIC,
        DIFFUSE_INTERREFLECTION,
    };
public:
    RTRVector3D location;
    RTRVector3D direction;
    RTRColor color;
    Type type;
};

#endif // __TINY_RENDERER_PHOTON__
