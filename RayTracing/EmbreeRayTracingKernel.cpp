#include <embree2/rtcore_ray.h>
#include "EmbreeRayTracingKernel.h"

EmbreeRayTracingKernel::EmbreeRayTracingKernel()
{
    
}

EmbreeRayTracingKernel::~EmbreeRayTracingKernel()
{
    
}

void EmbreeRayTracingKernel::buildIndex(const QVector<RTRRenderElement*>& elementTable)
{
    this->elementTable = elementTable;
    embreeDevice = rtcNewDevice(nullptr);
    //error_handler(rtcDeviceGetError(embreeDevice));

    /* set error handler */
    //rtcDeviceSetErrorFunction(embreeDevice, error_handler);

    /* create scene */
    embreeScene = rtcDeviceNewScene(embreeDevice, RTC_SCENE_STATIC,RTC_INTERSECT1);

    int vertexCount = elementTable.size() * 3;
    int triangleCount = elementTable.size();
    
    struct Vertex { float x, y, z, a; };
    struct Triangle 
    {
        unsigned v0, v1, v2, materialID;
    };
    
    unsigned int mesh = rtcNewTriangleMesh(
        embreeScene, RTC_GEOMETRY_STATIC, triangleCount, vertexCount);
    Vertex* vertices = (Vertex*)rtcMapBuffer(embreeScene, mesh, RTC_VERTEX_BUFFER);
    for(int i = 0; i < triangleCount; i++)
    {
        auto renderElement = elementTable[i];
        elementToIndex[renderElement] = i;
        for(int j = 0; j < 3; j++)
        {
            vertices[3 * i + j].x = (float)renderElement->triangle3D->vertices[j].x();
            vertices[3 * i + j].y = (float)renderElement->triangle3D->vertices[j].y();
            vertices[3 * i + j].z = (float)renderElement->triangle3D->vertices[j].z();
        }
    }
    rtcUnmapBuffer(embreeScene, mesh, RTC_VERTEX_BUFFER);

    Triangle* triangles = (Triangle*)rtcMapBuffer(embreeScene, mesh, RTC_INDEX_BUFFER);
    for(int i = 0; i < triangleCount; i++)
    {
        triangles[i].v0 = 3 * i;
        triangles[i].v1 = 3 * i + 1;
        triangles[i].v2 = 3 * i + 2;
    }
    rtcUnmapBuffer(embreeScene, mesh, RTC_INDEX_BUFFER);

    // commit changes to scene
    rtcCommit(embreeScene);
}

void EmbreeRayTracingKernel::intersect(
    RTRRenderElement*& searchResult, const RTRRay& ray,
    const RTRRenderElement* elementFrom) const
{
    RTCRay embreeRay;
    embreeRay.org[0] = ray.beginningPoint(0);
    embreeRay.org[1] = ray.beginningPoint(1);
    embreeRay.org[2] = ray.beginningPoint(2);
    RTRVector3D dir = ray.direction;
    dir.vectorNormalize();
    embreeRay.dir[0] = dir(0);
    embreeRay.dir[1] = dir(1);
    embreeRay.dir[2] = dir(2);
    embreeRay.tnear = 0.0001f;
    embreeRay.tfar = std::numeric_limits<float>::infinity();
    embreeRay.geomID = RTC_INVALID_GEOMETRY_ID;
    embreeRay.primID = RTC_INVALID_GEOMETRY_ID;
    embreeRay.mask = -1;
    embreeRay.time = 0;
    rtcIntersect(embreeScene, embreeRay);
    qDebug() << embreeRay.geomID << embreeRay.primID;
}
