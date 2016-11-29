#ifndef __TINYRENDERER_RTRRAIANCERENDERER_H__
#define __TINYRENDERER_RTRRAIANCERENDERER_H__

#include <QVector>
#include "RTRVector3D.h"
#include "Photon.h"
#include "SamplingUtil.h"

class RTRModel;
class RTRKdTree;
class RTRCamera;
class QImage;
class RTRRenderElement;

class RTRRadianceRenderer
{
public:
    RTRModel* model;
    QVector<RTRRenderElement*> elements;
    RTRKdTree* elementsCache;
    RTRCamera* camera;
    QImage* image;
    SamplingUtil* sampler;
public:
    void renderPhoton(RTRVector3D location, RTRVector3D direction, QVector<Photon> &result, bool causticOnly);
};

#endif // __TINYRENDERER_RTRRAIANCERENDERER_H__
