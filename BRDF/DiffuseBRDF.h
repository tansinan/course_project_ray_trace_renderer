#ifndef __TINY_RENDERER_BRDF__
#define __TINY_RENDERER_BRDF__

#include "RTRVector3D.h"

class DiffuseBRDF
{
public:
    static void sampleOutRay(SamplingUtil &sampler, const RTRVector3D &normal);
};

#endif // __TINY_RENDERER_BRDF__
