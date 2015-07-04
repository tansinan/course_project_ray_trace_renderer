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
			/*ray.endPoint = ray.beginningPoint + (ray.endPoint - ray.beginningPoint) / 160;
			RTRVector3D vec1 = renderer->camera->rotationMatrix * RTRVector3D(0, 1, 0);
			RTRVector3D vec2 = renderer->camera->rotationMatrix * RTRVector3D(1, 0, 0);
			ray.beginningPoint = ray.beginningPoint + vec1*(rand() / (double)RAND_MAX * 2.0 - 1.0);
			ray.beginningPoint = ray.beginningPoint + vec2*(rand() / (double)RAND_MAX * 2.0 - 1.0);
			ray = RTRRay(ray.beginningPoint, ray.endPoint, RTRRay::CREATE_FROM_POINTS);*/
			result = renderRay(ray);
			renderResult[i*renderer->image->height() + j] = result;
		}
	}
	emit renderFinished(threadIndex);
}

RTRColor RTRRenderThread::renderRay(const RTRRay& ray, int iterationCount, const RTRRenderElement* elementFrom)
{
	srand(rand() ^ (clock() + time(0)));

	RTRColor mtlWorldAmbient;
	if (renderer->model->materialLibrary.find("@@world") != renderer->model->materialLibrary.end())
	{
		RTRMaterial* worldMaterial = renderer->model->materialLibrary["@@world"];
		qDebug() << "world";
		if (worldMaterial->getPropertyType("ambient") == RTRMaterial::TYPE_COLOR)
		{
			mtlWorldAmbient = worldMaterial->getColorAt("ambient", 0, 0);
		}
	}

	//面光源的随机采样
	RTRLightPoint lightPoint(RTRVector3D(2.3 + (rand() / (double)RAND_MAX * 2 - 1)
		, -0.9 + (rand() / (double)RAND_MAX * 2 - 1)
		, 6.7), RTRColor(1, 1, 1), 7.5);

	//求交元素
	RTRRenderElement* intersectElement = NULL;
	renderer->elementsCache->search(intersectElement, ray, elementFrom);

	//如果光线不和任何物体相交那肯定是空无一物了……
	if (intersectElement == NULL) return mtlWorldAmbient;

	//否则的话首先求交，并且计算直接漫反射照明。
	RTRVector3D intersectPoint(0.0, 0.0, 0.0);
	RTRVector3D intersectNormal(0.0, 0.0, 0.0);
	RTRColor intersectColor(0.0, 0.0, 0.0);
	intersectElement->intersect(ray, intersectPoint, intersectNormal, intersectColor);

	//如果可能，查找物体的反射率和反射颜色
	double reflectionRate = 0.0;
	RTRColor mtlReflColor(1.0, 1.0, 1.0);
	double mtlReflGloss = 1.0;
	RTRColor mtlSpecColor;
	if (intersectElement->material->getPropertyType("reflection_rate") == RTRMaterial::TYPE_COLOR)
		reflectionRate = intersectElement->material->getColorAt("reflection_rate", 0, 0).r();
	if (intersectElement->material->getPropertyType("reflection_color") == RTRMaterial::TYPE_COLOR)
		mtlReflColor = intersectElement->material->getColorAt("reflection_color", 0, 0);
	if (intersectElement->material->getPropertyType("reflection_glossiness") == RTRMaterial::TYPE_COLOR)
		mtlReflGloss = intersectElement->material->getColorAt("reflection_glossiness", 0, 0).r();
	if (intersectElement->material->getPropertyType("specular") == RTRMaterial::TYPE_COLOR)
		mtlSpecColor = intersectElement->material->getColorAt("specular", 0, 0);
	

	RTRColor mtlAmbientColor;
	if (intersectElement->material->getPropertyType("ambient") == RTRMaterial::TYPE_COLOR)
	{
		mtlAmbientColor = intersectElement->material->getColorAt("ambient", 0, 0);
	}


	RTRColor diffuseColor(0.0, 0.0, 0.0);
	RTRColor specColor;
	RTRVector3D lightDirection = lightPoint.directionAt(intersectPoint);
	double decay = lightDirection.dotProduct(intersectNormal);
	int sym1 = sgn(decay);
	int sym2 = sgn((intersectPoint - ray.beginningPoint).dotProduct(intersectNormal));
	decay = decay > 0 ? decay : -decay;
	RTRColor lightColor = lightPoint.colorAt(intersectPoint);
	if (sym1 == sym2)
	{
		diffuseColor = intersectColor * lightColor * decay;
		RTRVector3D specularDirection = (intersectNormal * 2 * ray.direction.dotProduct(intersectNormal) - ray.direction)*-1;
		specularDirection.vectorNormalize();
		double spec = specularDirection.dotProduct(lightDirection);
		specColor = mtlSpecColor*lightColor*qPow(spec, 2);
	}
	//处理阴影。
	RTRRenderElement* directLight = NULL;
	renderer->elementsCache->search(directLight, RTRRay(intersectPoint, lightPoint.getPosition(), RTRRay::CREATE_FROM_POINTS), intersectElement);
	if (directLight != NULL)
	{
		diffuseColor.r() = 0.0;
		diffuseColor.g() = 0.0;
		diffuseColor.b() = 0.0;
		specColor = RTRColor(0.0, 0.0, 0.0);
	}

	diffuseColor = diffuseColor + intersectColor * mtlAmbientColor;

	if (iterationCount < 5)
	{
		RTRVector3D reflectionDirection(0.0, 0.0, 0.0);
		reflectionDirection = (intersectNormal * 2 * ray.direction.dotProduct(intersectNormal) - ray.direction)*-1;
		if (mtlReflGloss < 0.99999)
		{
			reflectionDirection.x() *= (1 + (rand() / (double)RAND_MAX * 2 - 1)*(1 - mtlReflGloss));
			reflectionDirection.y() *= (1 + (rand() / (double)RAND_MAX * 2 - 1)*(1 - mtlReflGloss));
			reflectionDirection.z() *= (1 + (rand() / (double)RAND_MAX * 2 - 1)*(1 - mtlReflGloss));
		}
		RTRRay reflectionRay(intersectPoint, reflectionDirection, RTRRay::CREATE_FROM_POINT_AND_DIRECTION);
		RTRColor reflectionColor = renderRay(reflectionRay, iterationCount + 1, intersectElement)*mtlReflColor;
			return reflectionColor * reflectionRate + diffuseColor * (1 - reflectionRate) + specColor;
	}
	else
	{
		return diffuseColor + specColor;
	}
}