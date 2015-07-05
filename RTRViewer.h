#ifndef RTRVIEWER_H
#define RTRVIEWER_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "RTRCamera.h"

class RTRRenderer;
class RTRModel;

class RTRViewer : public QWidget
{
	Q_OBJECT
protected:
	RTRCamera camera;
	RTRRenderer* renderer;
	RTRModel* model;
	QHBoxLayout* mainLayout;
	QVBoxLayout* renderParamaterLayout;
	void paintEvent(QPaintEvent* event);
	void mousePressEvent(QMouseEvent * event);
	QImage* renderResult;
public:
	explicit RTRViewer(QWidget *parent = 0);
	RTRRenderer* getRenderer();
	~RTRViewer();
signals:
public slots :
	void onRenderStatusChanged();
	void onRenderFinished();
};

#endif // RTRVIEWER_H
