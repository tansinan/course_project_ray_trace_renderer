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
	int beginTime = clock();
	if(model == NULL || camera == NULL) return;

	qDebug() << "Scan";
	//扫描所有需要渲染的多边形
	foreach(const RTRModelPolygen* face, model->polygens)
	{
		if (face->objectName == "Plane") continue;
		//TODO : 只适用于凸多边形！
		//三角形的三个顶点
		RTRVector point1(3), point2(3), point3(3);

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
	qDebug() << "Begin Kd-tree";
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

RTRColor RTRRenderer::renderRay(const RTRRay& ray, int iterationCount, const RTRRenderElement* elementFrom)
{
	int beginTime, endTime;
	//RTRVector* vec3D = new RTRVector(3)[100];
	//RTRLightPoint lightPoint(RTRVector(4.07625,1.00545,5.90386),RTRColor(1,1,1),7.5);
	RTRLightPoint lightPoint(RTRVector(2.3 + (rand() % 10 - 5) / 5.0
		, -0.9 + (rand() % 10 - 5) / 5.0
		, 6.7 + (rand() % 10 - 5) / 5.0), RTRColor(1, 1, 1), 7.5);

	beginTime = clock();
	//QSet<RTRRenderElement*> possibleElements;
	RTRRenderElement* possibleElements;
	elementsCache->search(possibleElements, ray, elementFrom);

	endTime = clock();
	//TODO: check 3D Z Order Algorithm

	searchTime += ((float)endTime - beginTime) / CLOCKS_PER_SEC;

	RTRColor diffuseColor(0.0, 0.0, 0.0);
	double minZ = 1e50;
	const RTRRenderElement* frontElement = NULL;

	RTRVector3D intersectPoint(0.0, 0.0, 0.0);
	RTRVector3D intersectNormal(0.0, 0.0, 0.0);
	RTRColor intersectColor(0.0, 0.0, 0.0);
	//foreach(const RTRRenderElement* element, possibleElements)
	if (possibleElements != NULL)
	{
		RTRRenderElement* element = possibleElements;
		RTRVector3D point(0.0, 0.0, 0.0);
		RTRVector3D normal(0.0, 0.0, 0.0);
		RTRColor objColor(0.0, 0.0, 0.0);
		element->intersect(ray, point, normal, objColor);
		//if (!element->intersect(ray, point, normal, objColor));continue;
		double zValue = ray.direction.dotProduct(point - ray.beginningPoint);
		//if (zValue < 0) continue;
		//double zValue = abs(point.z() - ray.beginningPoint.z());
		//if (zValue > minZ) continue;
		//if (element == elementFrom) continue;
		frontElement = element;
		intersectPoint = point;
		intersectColor = objColor;
		intersectNormal = normal;
		minZ = zValue;
		RTRVector v =lightPoint.directionAt(point);
		double decay = lightPoint.directionAt(point).dotProduct(normal);
		int sym1 = sgn(decay);
		int sym2 = sgn((point - ray.beginningPoint).dotProduct(normal));
		decay = decay>0?decay:-decay;
		if(sym1 == sym2)
		{
			RTRColor lightColor = lightPoint.colorAt(point);
			diffuseColor.r() = objColor.r()*lightColor.r()*decay;
			diffuseColor.g() = objColor.g()*lightColor.g()*decay;
			diffuseColor.b() = objColor.b()*lightColor.b()*decay;
		}
		else
		{
			diffuseColor.r() = 0.0;
			diffuseColor.g() = 0.0;
			diffuseColor.b() = 0.0;
		}
		//处理一般阴影
		RTRRenderElement* directLight = NULL;
		elementsCache->search(directLight, RTRRay(point, lightPoint.getPosition(), RTRRay::CREATE_FROM_POINTS), element);
		if (directLight != NULL)
		{
			diffuseColor.r() = 0.0;
			diffuseColor.g() = 0.0;
			diffuseColor.b() = 0.0;
		}
	}

	RTRColor reflectionColor(0.0,0.0,0.0);
	if (frontElement != NULL && frontElement->objectName == "Plane" /*&& (frontElement->objectName == "Sphere" || frontElement->objectName == "Cube_Cube.001")*/ && iterationCount<0)
	{
		RTRVector3D reflectionDirection(0.0, 0.0, 0.0);
		reflectionDirection = (intersectNormal * 2 * ray.direction.dotProduct(intersectNormal) - ray.direction)*-1;
		for (int i = 0; i < 2; i++)
		{
			RTRVector3D rd = reflectionDirection;
			rd.x() *= (1 + (rand() % 10-5) / 100.0);
			rd.y() *= (1 + (rand() % 10-5) / 100.0);
			rd.z() *= (1 + (rand() % 10-5) / 100.0);
			RTRRay reflectionRay(intersectPoint, rd, RTRRay::CREATE_FROM_POINT_AND_DIRECTION);
			reflectionColor = reflectionColor + renderRay(reflectionRay, iterationCount + 1, frontElement) * 0.5;
		}
		//RTRRay reflectionRay(intersectPoint,reflectionDirection,RTRRay::CREATE_FROM_POINT_AND_DIRECTION);
		//reflectionColor = renderRay(reflectionRay, iterationCount + 1, frontElement);
		return reflectionColor*0.5 + diffuseColor*0.5;
	}
	else
	{
		return diffuseColor;
	}
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