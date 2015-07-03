#include "RTRRenderer.h"
#include <QColor>
#include <QDebug>
#include <QImage>
#include <time.h>
#include "RTRGeometry.h"
#include "Light/RTRLightPoint.h"
#include "RTRRenderThread.h"

RTRRenderer::RTRRenderer(QImage *_image)
{
	model = NULL;
	camera = NULL;
	image = _image;
	renderResult = new RTRColor[image->width()*image->height()];
}

void RTRRenderer::render()
{
	if(model == NULL || camera == NULL) return;

	//扫描所有需要渲染的多边形
	foreach(const RTRModelPolygen* face, model->polygens)
	{
		//if (face->objectName == "Plane") continue;
		//TODO : 只适用于凸多边形！
		//三角形的三个顶点
		RTRVector3D point1, point2, point3;

		//将多边形的第一个顶点作为三角形的第一个顶m点
		point1 = face->vertices[0]->position;

		//而三角形的另外两个顶点从多边形剩下的顶点中取两两相邻的选取。
		for (int j = 1; j < face->vertices.size() - 1; j++)
		{
			//构建新的三角形
			point2 = face->vertices[j]->position;
			point3 = face->vertices[j + 1]->position;

			//TODO: 存在内存泄漏问题！
			RTRRenderElement* element = new RTRRenderElement(new RTRTriangle3D(point1, point2, point3), camera);
			element->objectName = face -> objectName;
			element->material = model->materialLibrary[face->materialName];
			if (face->uvMaps.size() > 0)
			{
				element->vertexUVMaps[0] = face->uvMaps[0];
				element->vertexUVMaps[1] = face->uvMaps[j];
				element->vertexUVMaps[2] = face->uvMaps[j + 1];
			}
			//element->material = &material;
			/*if (model->vertexNormals.size() > face.normals[j + 1] - 1 &&
				model->vertexNormals.size() > face.normals[j] - 1 &&
				model->vertexNormals.size() > face.normals[0] - 1)
			{
				element->useSmoothShading = true;
				element->vertexNormals[0] = model->vertexNormals[face.normals[0] - 1];
				element->vertexNormals[1] = model->vertexNormals[face.normals[j] - 1];
				element->vertexNormals[2] = model->vertexNormals[face.normals[j + 1] - 1];
			}*/
			//else
			{
				element->useSmoothShading = false;
			}
			/*if (face.materialName == "") element->material = NULL;
			else
			{
				element->material = model->materialLibrary[face.materialName];
			}*/

			//将三角形添加到需要渲染的三角形的列表之中
			elements.append(element);
		}
	}
	
	//创建Kd树以便加速搜索。
	elementsCache = RTRKdTree::create(elements);

	//遍历所有的像素点，从它们发出光线并计算渲染结果。
	/*for(int i=0;i<image->width();i++)
	{
		for(int j=0;j<image->height();j++)
		{
			RTRColor result(0.0, 0.0, 0.0);
			for (int pass = 0; pass < 1; pass++)
			{
				RTRRay ray = RTRGeometry::invertProject(RTRVector2D(i, j), *camera);
				RTRColor color = renderRay(ray);
				result = result + color;
			}
			result = result / 1.0;
			renderPixel(i,j,0,result);
		}
	}
	int endTime = clock();
	qDebug() << ((float)endTime - beginTime)/CLOCKS_PER_SEC;
	qDebug() << "Search:" << searchTime;*/

	renderThreads = new RTRRenderThread*[8];
	for (int i = 0; i < 8; i++)
	{
		renderThreads[i] = new RTRRenderThread(this, i);
		connect(renderThreads[i], SIGNAL(finished()), this, SLOT(onThreadFinished()));
		connect(renderThreads[i], SIGNAL(renderFinished(int)), this, SLOT(onRenderFinished(int)));
	}
	currentPass = 0;
	targetPass = 20;
	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 12; j++)
		{
			renderGridPass[i][j] = 0;
		}
	}
	allocateTask();
}

void RTRRenderer::renderPixel(int x, int y, double z, const RTRColor& color)
{
	image->setPixel(x,image->height()-1-y,color.toQtColor().rgb());
}

void RTRRenderer::allocateTask(int threadId)
{
	if (threadId == -1)
	{
		for (int i = 0; i < 8; i++)
		{
			allocateTask(i);
		}
		return;
	}
	if (renderThreads[threadId]->isRunning()) return;
	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 12; j++)
		{
			if (renderGridPass[i][j] <= currentPass)
			{
				renderGridPass[i][j]++;
				renderThreads[threadId]->start(50 * i, 50 * i + 49, 50 * j, 50 * j + 49);
				return;
			}
		}
	}
	//currentPass++;
	//TODO: this means render finished.
	if (currentPass == targetPass) return;
	currentPass++;
	renderGridPass[0][0]++;
	renderThreads[threadId]->start(0, 49, 0, 49);
}

void RTRRenderer::onRenderFinished(int threadId)
{
	//RTRRenderThread* thread = renderThreads[threadId];
}

void RTRRenderer::onThreadFinished()
{
	RTRRenderThread* thread = (RTRRenderThread*)QObject::sender();
	for (int i = thread->xBegin; i <= thread->xEnd; i++)
	{
		for (int j = thread->yBegin; j <= thread->yEnd; j++)
		{
			renderResult[i*image->height() + j] = renderResult[i*image->height() + j] + thread->renderResult[i*image->height() + j];
		}
	}
	allocateTask(thread->threadIndex);
	if(rand()%10==0) emit renderStatusChanged();
}