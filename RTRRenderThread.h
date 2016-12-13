#ifndef RTRRENDERTHREAD_H
#define RTRRENDERTHREAD_H

#include <QThread>
#include "RTRRenderer.h"
#include "SamplingUtil.h"
#include "RTRRadianceRenderer.h"

class RTRRadianceRenderer;

class RTRRenderThread : public QThread
{
    Q_OBJECT
public:
    int xBegin;
    int xEnd;
    int yBegin;
    int yEnd;
    int threadIndex;
    RTRRenderer* renderer;
    RTRRenderThread(RTRRenderer* _renderer, int _threadIndex);
    RTRColor* renderResult;
    SamplingUtil sampler;
    RTRRadianceRenderer *radianceRenderer;

public slots:
    void start(int _xBegin, int _xEnd, int _yBegin, int _yEnd);

protected:
    virtual RTRColor estimateRadianceByPhotonMap(PhotonKdTree* photonMap,
        const std::vector<Photon*>& photons,
        RTRVector3D location, RTRVector3D normal, RTRColor color);

    virtual void run() Q_DECL_OVERRIDE;
    /**
    * @brief renderRay实现了光线追踪的核心算法——它渲染一个光路的“颜色”。
    * @param ray需要进行追踪的光线
    * @param iterationCount迭代次数，为方便调用，默认参数值为0
    * @param elementFrom 发出该逆向光线的渲染元素。该参数主要用于防止发生光线求交误判为和自己想交的情况。
    * @return 获得的颜色数值。
    */
    RTRColor renderRay(const RTRRay& ray, int iterationCount = 0, const RTRRenderElement* elementFrom = NULL, double refracInAir = true, int diffuseCount = 0);

signals:
    void renderFinished(int _threadIndex);
};

#endif
