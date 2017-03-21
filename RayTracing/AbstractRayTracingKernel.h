#ifndef __TINY_RENDERER_ABSTRACT_RAY_TRACING_KERNEL__
#define __TINY_RENDERER_ABSTRACT_RAY_TRACING_KERNEL__

#include "RTRGeometry.h"
#include <QVector>

class RTRRenderElement;

class AbstractRayTracingKernel
{
public:
    AbstractRayTracingKernel();
    virtual ~AbstractRayTracingKernel();
    virtual void buildIndex(
        const QVector<RTRRenderElement*>& elementTable) = 0;
    virtual void intersect(
        RTRRenderElement*& searchResult,
        const RTRRay& ray, const RTRRenderElement* elementFrom) const = 0;
};

#endif// __TINY_RENDERER_ABSTRACT_RAY_TRACING_KERNEL__
