#ifndef RTRRENDERER_H
#define RTRRENDERER_H

#include "RTRModel.h"
#include "RTRCamera.h"
#include "RTRGeometry.h"
#include "RTRMaterial.h"
#include "RTRRenderElement.h"
#include "RTRKdTree.h"

class RTRRenderThread;

class RTRRenderer :public QObject
{
	friend class RTRViewer;
	Q_OBJECT
public:
	double searchTime = 0.0;
	double processTime = 0.0;
	RTRModel* model;
	QVector<RTRRenderElement*> elements;
	RTRKdTree* elementsCache;
	RTRCamera* camera;
	QImage* image;
protected:
	RTRColor* renderResult;
	RTRRenderThread** renderThreads;
	int renderGridPass[16][12];
	int currentPass;
	int targetPass;
protected:
	void allocateTask(int threadId = -1);
public slots:
	void onRenderFinished(int threadId);
	void onThreadFinished();
signals:
	void renderStatusChanged();
	void renderFinished();
public:
	RTRRenderer(QImage* _image);
	void render();
	void renderPixel(int x, int y, double z, const RTRColor &color);

	/**
	 * @brief renderRay实现了光线追踪的核心算法——它渲染一个光路的“颜色”。
	 * @param ray需要进行追踪的光线
	 * @param iterationCount迭代次数，为方便调用，默认参数值为0
	 * @param elementFrom 发出该逆向光线的渲染元素。该参数主要用于防止发生光线求交误判为和自己想交的情况。
	 * @return 获得的颜色数值。
	 */
};

#endif // RTRRENDERER_H
