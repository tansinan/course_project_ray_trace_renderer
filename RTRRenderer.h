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
public:
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
	bool render(RTRModel* model, RTRCamera* camera, int pass);
	void renderPixel(int x, int y, double z, const RTRColor &color);
};

#endif // RTRRENDERER_H
