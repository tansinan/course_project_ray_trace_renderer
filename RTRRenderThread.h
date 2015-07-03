#ifndef RTRRENDERTHREAD_H
#define RTRRENDERTHREAD_H

#include <QThread>
#include "RTRRenderer.h"

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

public slots:
	void start(int _xBegin, int _xEnd, int _yBegin, int _yEnd);

protected:
	virtual void run() Q_DECL_OVERRIDE;
	RTRColor renderRay(const RTRRay& ray, int iterationCount = 0, const RTRRenderElement* elementFrom = NULL);

signals:
	void renderFinished(int _threadIndex);
};

#endif