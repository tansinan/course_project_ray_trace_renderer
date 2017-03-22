#ifndef RENDERINGOPTIONS_H
#define RENDERINGOPTIONS_H

#include <stdint.h>

class RenderingOptions
{
public:
    enum DIEstimationMode
    {
        FROM_LIGHT_SOURCE,
        FROM_OBJECT
    };
public:
    DIEstimationMode diEstimationMode;
    
    // Options about photon map generation
    uint32_t photonMapMaxBounceCount;
    uint32_t diffusePhotonCount;
    bool useCausticPhoton;
    uint32_t causticPhotonCount;
    
    // Options about radiance estimation
    uint32_t diffuseEstimationPhotonCount;
    uint32_t causticEstimationPhotonCount;
    RenderingOptions();
};

#endif // RENDERINGOPTIONS_H
