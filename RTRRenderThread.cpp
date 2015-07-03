#include "RTRRenderThread.h"
#include "RTRRenderer.h"
#include "Light\RTRLightPoint.h"
#include <QImage>
#include <qdebug.h>
#include <ctime>

RTRRenderThread::RTRRenderThread(RTRRenderer* _renderer, int _threadIndex)
{
	renderer = _renderer;
	threadIndex = _threadIndex;
	renderResult = new RTRColor[renderer->image->width()*renderer->image->height()];
}

void RTRRenderThread::start(int _xBegin, int _xEnd, int _yBegin, int _yEnd)
{
	xBegin = _xBegin;
	xEnd = _xEnd;
	yBegin = _yBegin;
	yEnd = _yEnd;
	QThread::start();
}

void RTRRenderThread::run()
{
	srand(clock());
	qDebug() << "Task Accepted:" << "(" << xBegin << "," << yBegin << ")" << "(" << xEnd << "," << yEnd << ")";
	for (int i = xBegin; i <= xEnd; i++)
	{
		for (int j = yBegin; j <= yEnd; j++)
		{
			RTRColor result(0.0, 0.0, 0.0);

			//为了实现Anti-alias进行的对于光屏上点的采样随机化处理
			double antiAliasOffsetX = (rand() / (double)RAND_MAX - 0.5);
			double antiAliasOffsetY = (rand() / (double)RAND_MAX - 0.5);

			//计算

			RTRRay ray = RTRGeometry::invertProject(RTRVector2D(i + antiAliasOffsetX, j + antiAliasOffsetY), *renderer->camera);
			result = renderRay(ray);
			renderResult[i*renderer->image->height() + j] = result;
		}
	}
	emit renderFinished(threadIndex);
}

RTRColor RTRRenderThread::renderRay(const RTRRay& ray, int iterationCount, const RTRRenderElement* elementFrom)
{
	int beginTime, endTime;
	//RTRVector* vec3D = new RTRVector(3)[100];
	//RTRLightPoint lightPoint(RTRVector(4.07625,1.00545,5.90386),RTRColor(1,1,1),7.5);

	//面光源的随机采样
	RTRLightPoint lightPoint(RTRVector3D(2.3 + (rand() / (double)RAND_MAX * 2 - 1)
		, -0.9 + (rand() / (double)RAND_MAX * 2 - 1)
		, 6.7 + (rand() / (double)RAND_MAX * 2 - 1)), RTRColor(1, 1, 1), 7.5);

	//求交元素
	RTRRenderElement* intersectElement = NULL;
	renderer->elementsCache->search(intersectElement, ray, elementFrom);

	//如果光线不和任何物体相交那肯定是空无一物了……
	if (intersectElement == NULL) return RTRColor(0.0, 0.0, 0.0);

	RTRVector3D intersectPoint(0.0, 0.0, 0.0);
	RTRVector3D intersectNormal(0.0, 0.0, 0.0);
	RTRColor intersectColor(0.0, 0.0, 0.0);
	intersectElement->intersect(ray, intersectPoint, intersectNormal, intersectColor);

	double reflectionRate = 0.0;
	if (intersectElement->material->getPropertyType("reflection_rate") == RTRMaterial::TYPE_COLOR)
	{
		reflectionRate = intersectElement->material->getColorAt("reflection_rate", 0, 0).r();
	}

	//if (rand() / (double)RAND_MAX > reflectionRate)
	//{
		RTRColor diffuseColor(0.0, 0.0, 0.0);
		double decay = lightPoint.directionAt(intersectPoint).dotProduct(intersectNormal);
		int sym1 = sgn(decay);
		int sym2 = sgn((intersectPoint - ray.beginningPoint).dotProduct(intersectNormal));
		decay = decay > 0 ? decay : -decay;
		if (sym1 == sym2)
		{
			RTRColor lightColor = lightPoint.colorAt(intersectPoint);
			diffuseColor.r() = intersectColor.r()*lightColor.r()*decay;
			diffuseColor.g() = intersectColor.g()*lightColor.g()*decay;
			diffuseColor.b() = intersectColor.b()*lightColor.b()*decay;
		}
		else
		{
			diffuseColor.r() = 0.0;
			diffuseColor.g() = 0.0;
			diffuseColor.b() = 0.0;
		}
		//处理一般阴影
		RTRRenderElement* directLight = NULL;
		renderer->elementsCache->search(directLight, RTRRay(intersectPoint, lightPoint.getPosition(), RTRRay::CREATE_FROM_POINTS), intersectElement);
		if (directLight != NULL)
		{
			diffuseColor.r() = 0.0;
			diffuseColor.g() = 0.0;
			diffuseColor.b() = 0.0;
		}
		//return diffuseColor;
	//}
	//else
	//{
	if (iterationCount < 5)
	{
		RTRVector3D reflectionDirection(0.0, 0.0, 0.0);
		reflectionDirection = (intersectNormal * 2 * ray.direction.dotProduct(intersectNormal) - ray.direction)*-1;
		reflectionDirection.x() *= (1 + (rand() % 10 - 5) / 100.0);
		reflectionDirection.y() *= (1 + (rand() % 10 - 5) / 100.0);
		reflectionDirection.z() *= (1 + (rand() % 10 - 5) / 100.0);
		RTRRay reflectionRay(intersectPoint, reflectionDirection, RTRRay::CREATE_FROM_POINT_AND_DIRECTION);
		RTRColor reflectionColor = renderRay(reflectionRay, iterationCount + 1, intersectElement);
		//RTRRay reflectionRay(intersectPoint,reflectionDirection,RTRRay::CREATE_FROM_POINT_AND_DIRECTION);
		//reflectionColor = renderRay(reflectionRay, iterationCount + 1, frontElement);
		return reflectionColor * reflectionRate + diffuseColor * (1 - reflectionRate);
	}
	else
	{
		return diffuseColor;
	}
}