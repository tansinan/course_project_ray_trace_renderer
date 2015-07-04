#include "RTRViewer.h"
#include "RTRModel.h"
#include "RTRCamera.h"
#include "RTRRenderer.h"
#include <QPainter>
#include <QDebug>
#include <QApplication>

RTRViewer::RTRViewer(QWidget *parent) : QWidget(parent)
{
	model = new RTRModel();
	model->loadModelFromObjFile(QString("D:\\Documents\\FinalProject.obj"));
	//model->saveModelToObjFile(QString("D:\\RubikOutput.obj"));
	//QApplication::exit();
	setFixedSize(800,600);
	renderResult =  new QImage(800, 600, QImage::Format_ARGB32);
	renderer = new RTRRenderer(renderResult);
	//camera.cameraAngle = RTRVector3D(63.6, 0.6, 46.7);
	camera.cameraPosition = RTRVector3D(5.1, 2.6, 1.9);
	//camera.cameraPosition = RTRVector3D(7.5, -6.5, 5.3);
	camera.cameraAngle = RTRVector3D(83.6, 0.7, 117.9);
	camera.focalLength = 600;
	camera.offset.x() = 400;
	camera.offset.y() = 300;
	//camera.focalLength = 500;
	camera.evaluateRotationMatrix();
	renderer->model = model;
	renderer->camera = &camera;
	connect(renderer, SIGNAL(renderStatusChanged()), this, SLOT(onRenderStatusChanged()));
	connect(renderer, SIGNAL(renderFinished()), this, SLOT(onRenderFinished()));
	renderer->render();
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
	/*if(rendered)
	{
		painter.drawImage(0,0,*renderResult);
		return;
	}
	rendered = true;
	if(renderResult==NULL) return;*/
	for (int i = 0; i < renderer->image->width(); i++)
	{
		for (int j = 0; j < renderer->image->height(); j++)
		{
			int pass = renderer->renderGridPass[i / 50][j / 50];
			if (pass != 0)
			{
				renderer->renderPixel(i, j, 0, renderer->renderResult[i*renderer->image->height() + j] / pass);
			}
			else renderer->renderPixel(i, j, 0, RTRColor());
		}
	}
	painter.drawImage(0,0,*renderResult);
}

void RTRViewer::mousePressEvent(QMouseEvent * event)
{

}

void RTRViewer::onRenderStatusChanged()
{
	repaint();
}

void RTRViewer::onRenderFinished()
{
	for (int i = 0; i < renderer->image->width(); i++)
	{
		for (int j = 0; j < renderer->image->height(); j++)
		{
			int pass = renderer->renderGridPass[i / 50][j / 50];
			if (pass != 0)
			{
				renderer->renderPixel(i, j, 0, renderer->renderResult[i*renderer->image->height() + j] / pass);
			}
			else renderer->renderPixel(i, j, 0, RTRColor());
		}
	}
	renderer->image->save("MyRenderResult.bmp");
}