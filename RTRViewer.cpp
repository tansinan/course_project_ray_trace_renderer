#include "RTRViewer.h"
#include "RTRModel.h"
#include "RTRCamera.h"
#include "RTRRenderer.h"
#include "MainWindow.h"
#include "RTRRenderThread.h"
#include <QPainter>
#include <QDebug>
#include <QApplication>
#include <ctime>

RTRViewer::RTRViewer(QWidget *parent, RTRRenderer* _renderer)
	:QWidget(parent)
{
	renderer = _renderer;
	setFixedSize(800,600);
}

RTRViewer::~RTRViewer()
{
}

void RTRViewer::paintEvent(QPaintEvent* event)
{
	static bool rendered = false;
	QPainter painter(this);
	if(renderer->image!=NULL)
	{
		painter.drawImage(0,0,*renderer->image);
	}
}

void RTRViewer::mousePressEvent(QMouseEvent * event)
{

}

void RTRViewer::updateDisplay(bool forced)
{
  static int lastUpdate = 0;
	if(clock() - lastUpdate < CLOCKS_PER_SEC/8 && !forced)
	{
		return;
	}
	lastUpdate = clock();
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
    for(int k=0;k<8;k++)
	{
		if(forced) break;
		if(renderer->renderThreads[k]->isFinished()) continue;
		for(int i =renderer->renderThreads[k]->xBegin,ii=0;i<=renderer->renderThreads[k]->xEnd;i++,ii++)
		{
			for(int j =renderer->renderThreads[k]->yBegin,jj=0;j<=renderer->renderThreads[k]->yEnd;j++,jj++)
			{
				int pass = renderer->renderGridPass[i / 50][j / 50];
				if(ii<8||ii>42||jj<8||jj>42)
				{
					if (pass != 0) renderer->renderPixel(i, j, 0, renderer->renderResult[i*renderer->image->height() + j] / pass * 2
							+ RTRColor(0.25,0.25,0.25));
					else renderer->renderPixel(i, j, 0, RTRColor());
				}
				else
				{
					if (pass != 0) renderer->renderPixel(i, j, 0, renderer->renderResult[i*renderer->image->height() + j] / pass / 2);
					else renderer->renderPixel(i, j, 0, RTRColor());
				}
			}
		}
    }
	repaint();
}
