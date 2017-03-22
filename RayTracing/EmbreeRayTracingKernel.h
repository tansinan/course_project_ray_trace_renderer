#ifndef __TINY_RENDERER_EMBREE_RAY_TRACING_KERNEL__
#define __TINY_RENDERER_EMBREE_RAY_TRACING_KERNEL__

#include <embree2/rtcore.h>
#include <embree2/rtcore_scene.h>
#include <QMap>
#include "RTRRenderElement.h"
#include "AbstractRayTracingKernel.h"

class EmbreeRayTracingKernel : public AbstractRayTracingKernel
{
protected:
    QVector<RTRRenderElement*> elementTable;
    QMap<RTRRenderElement*, int> elementToIndex;
public:
    RTCDevice embreeDevice = nullptr;
    RTCScene embreeScene = nullptr;
    EmbreeRayTracingKernel();
    ~EmbreeRayTracingKernel() override;
    void buildIndex(const QVector<RTRRenderElement*>& elementTable) override;
    void intersect(
        RTRRenderElement*& searchResult, const RTRRay& ray,
        const RTRRenderElement* elementFrom) const override;
};

#endif // __TINY_RENDERER_EMBREE_RAY_TRACING_KERNEL__
