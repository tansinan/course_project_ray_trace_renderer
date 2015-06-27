#include "RTRViewer.h"
#include "RTRModel.h"
#include "RTRCamera.h"
#include "RTRRenderer.h"
#include <QPainter>
#include <QDebug>

RTRViewer::RTRViewer(QWidget *parent) : QWidget(parent)
{
	model = new RTRModel();
	model->loadModelFromObjFile(QString("D:\\Documents\\SimpleGlass.obj"));
	setFixedSize(800,600);
	renderResult =  new QImage(800, 600, QImage::Format_ARGB32);
}

RTRViewer::~RTRViewer()
{
	qDebug() << "Destructor called!" <<endl;
	delete model;
	delete renderResult;
	renderResult = NULL;
}

void RTRViewer::paintEvent(QPaintEvent* event)
{
	static bool rendered = false;
		QPainter painter(this);
	if(rendered)
	{
		painter.drawImage(0,0,*renderResult);
		return;
	}
	rendered = true;
	if(renderResult==NULL) return;

	RTRCamera camera;
	camera.cameraAngle = RTRVector(63.6, 0.6, 46.7);
	//camera.cameraAngle = RTRVector(0, 0, 0);
	camera.cameraPosition = RTRVector(7.5, -6.5, 5.3);
	//camera.cameraPosition = RTRVector(0, 0, 0);
	camera.focalLength = 1500;
	camera.offset.x() = 400;
	camera.offset.y() = 300;
	//camera.focalLength = 500;
	//camera.offset.x() = 100;
	//camera.offset.y() = 100;
	camera.evaluateRotationMatrix();
	RTRRenderer renderer(renderResult);
	renderer.model = model;
	renderer.camera = &camera;
	renderer.render();
	painter.drawImage(0,0,*renderResult);
}

void RTRViewer::mousePressEvent(QMouseEvent * event)
{

}
