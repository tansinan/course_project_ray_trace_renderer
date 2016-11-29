#include "RTRRenderThread.h"
#include "RTRRenderer.h"
#include "Light/RTRLightPoint.h"
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
	qsrand(clock()^time(0));
	double apertureSize = 0.0;
	double focus = 0.0;
	if (renderer->model->materialLibrary.find("@@world") != renderer->model->materialLibrary.end())
	{
		RTRMaterial* worldMaterial = renderer->model->materialLibrary["@@world"];
		if (worldMaterial->getPropertyType("fod_aperture") == RTRMaterial::TYPE_COLOR && worldMaterial->getPropertyType("fod_focus") == RTRMaterial::TYPE_COLOR)
		{
			apertureSize = worldMaterial->getColorAt("fod_aperture", 0, 0).r();
			focus = worldMaterial->getColorAt("fod_focus", 0, 0).r();;
		}
	}
	for (int i = xBegin; i <= xEnd; i++)
	{
		for (int j = yBegin; j <= yEnd; j++)
		{
			RTRColor result(0.0, 0.0, 0.0);

			//Ϊ��ʵ��Anti-alias���еĶ��ڹ����ϵ��Ĳ�������������
			double antiAliasOffsetX = (qrand() / (double)RAND_MAX - 0.5);
			double antiAliasOffsetY = (qrand() / (double)RAND_MAX - 0.5);

			//����

			RTRRay ray = RTRGeometry::invertProject(RTRVector2D(i + antiAliasOffsetX, j + antiAliasOffsetY), *renderer->camera);
			if (apertureSize > 0.0000001 && focus > 0.0000001)
			{
				ray.endPoint = ray.beginningPoint + (ray.endPoint - ray.beginningPoint) / (renderer->camera->imageDistance / focus);
				RTRVector3D vec1 = renderer->camera->rotationMatrix * RTRVector3D(0, 1, 0);
				RTRVector3D vec2 = renderer->camera->rotationMatrix * RTRVector3D(1, 0, 0);
				double apertureOffsetX, apertureOffsetY;
				for (;;)
				{
					apertureOffsetX = (qrand() / (double)RAND_MAX * 2 * apertureSize - apertureSize);
					apertureOffsetY = (qrand() / (double)RAND_MAX * 2 * apertureSize - apertureSize);
					if (apertureOffsetX*apertureOffsetX + apertureOffsetY*apertureOffsetY < apertureSize*apertureSize)
						break;
				}
				ray.beginningPoint = ray.beginningPoint + vec1*apertureOffsetX;
				ray.beginningPoint = ray.beginningPoint + vec2*apertureOffsetY;
				ray = RTRRay(ray.beginningPoint, ray.endPoint, RTRRay::CREATE_FROM_POINTS);
			}
			result = renderRay(ray);
			renderResult[i*renderer->image->height() + j] = result;
		}
	}
	emit renderFinished(threadIndex);
}

RTRColor RTRRenderThread::renderRay(const RTRRay& ray, int iterationCount, const RTRRenderElement* elementFrom, double refracInAir)
{
	qsrand(qrand() ^ (clock() + time(0)));

	RTRColor mtlWorldAmbient;
	if (renderer->model->materialLibrary.find("@@world") != renderer->model->materialLibrary.end())
	{
		RTRMaterial* worldMaterial = renderer->model->materialLibrary["@@world"];
		if (worldMaterial->getPropertyType("ambient") == RTRMaterial::TYPE_COLOR)
		{
			mtlWorldAmbient = worldMaterial->getColorAt("ambient", 0, 0);
		}
	}

	//����Դ����������
    RTRLightPoint lightPoint(RTRVector3D(-2.0 + (qrand() / (double)RAND_MAX * 2 - 1)
        , 0 + (qrand() / (double)RAND_MAX * 2 - 1)
        , 5.0), RTRColor(1, 1, 1), 7.5);

	//����Ԫ��
	RTRRenderElement* intersectElement = NULL;
	renderer->elementsCache->search(intersectElement, ray, elementFrom);

	//�������߲����κ������ཻ�ǿ϶��ǿ���һ���ˡ���
	if (intersectElement == NULL) return mtlWorldAmbient;

	//�����Ļ������󽻣����Ҽ���ֱ��������������
	RTRVector3D intersectPoint(0.0, 0.0, 0.0);
	RTRVector3D intersectNormal(0.0, 0.0, 0.0);
	RTRColor intersectColor(0.0, 0.0, 0.0);
	intersectElement->intersect(ray, intersectPoint, intersectNormal, intersectColor);

	//���������Ĳ������ԣ����������ʺͷ�����ɫ���߹⡢�����ʵ�
	double reflectionRate = 0.0;
	RTRColor mtlReflColor(1.0, 1.0, 1.0);
	double mtlReflGloss = 1.0;
	RTRColor mtlSpecColor;
	double mtlRefracRate = 0.0;
	double mtlRefracIndex = 1.0;
	RTRColor mtlRefracColor(1.0, 1.0, 1.0);
	double mtlRefracGloss = 1.0;

    //处理反射相关属性
	if (intersectElement->material->getPropertyType("reflection_rate") == RTRMaterial::TYPE_COLOR)
		reflectionRate = intersectElement->material->getColorAt("reflection_rate", 0, 0).r();
	if (intersectElement->material->getPropertyType("reflection_color") == RTRMaterial::TYPE_COLOR)
		mtlReflColor = intersectElement->material->getColorAt("reflection_color", 0, 0);
	if (intersectElement->material->getPropertyType("reflection_glossiness") == RTRMaterial::TYPE_COLOR)
		mtlReflGloss = intersectElement->material->getColorAt("reflection_glossiness", 0, 0).r();

	//�߹���������
	if (intersectElement->material->getPropertyType("specular") == RTRMaterial::TYPE_COLOR)
		mtlSpecColor = intersectElement->material->getColorAt("specular", 0, 0);

	//������������
	if (intersectElement->material->getPropertyType("refraction_rate") == RTRMaterial::TYPE_COLOR)
		mtlRefracRate = intersectElement->material->getColorAt("refraction_rate", 0, 0).r();
	if (intersectElement->material->getPropertyType("refraction_index") == RTRMaterial::TYPE_COLOR)
		mtlRefracIndex = intersectElement->material->getColorAt("refraction_index", 0, 0).r();
	if (intersectElement->material->getPropertyType("refraction_color") == RTRMaterial::TYPE_COLOR)
		mtlRefracColor = intersectElement->material->getColorAt("refraction_color", 0, 0);
	if (intersectElement->material->getPropertyType("refraction_glossiness") == RTRMaterial::TYPE_COLOR)
		mtlRefracGloss = intersectElement->material->getColorAt("refraction_glossiness", 0, 0).r();


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
		double spec = qAbs(specularDirection.dotProduct(lightDirection));
		specColor = mtlSpecColor*lightColor*qPow(spec, 2);
	}

	//������Ӱ��ֻ���Ǿֲ����գ������䲻���ĵط�û��Diffuse��Specular
	RTRRenderElement* directLight = NULL;
	renderer->elementsCache->search(directLight, RTRRay(intersectPoint, lightPoint.getPosition(), RTRRay::CREATE_FROM_POINTS), intersectElement);
	if (directLight != NULL)
	{
		diffuseColor = RTRColor(0.0, 0.0, 0.0);
		specColor = RTRColor(0.0, 0.0, 0.0);
	}

	diffuseColor = diffuseColor + intersectColor * mtlAmbientColor;

	//�����������Ƶ���ʱ�����������ͷ�������
	if (iterationCount >= 5) return diffuseColor + specColor;

	if (reflectionRate > 0.00001)
	{
		RTRVector3D reflectionDirection(0.0, 0.0, 0.0);
		reflectionDirection = (intersectNormal * 2 * ray.direction.dotProduct(intersectNormal) - ray.direction)*-1;
		if (mtlReflGloss < 0.99999)
		{
			reflectionDirection.x() *= (1 + (qrand() / (double)RAND_MAX * 2 - 1)*(1 - mtlReflGloss));
			reflectionDirection.y() *= (1 + (qrand() / (double)RAND_MAX * 2 - 1)*(1 - mtlReflGloss));
			reflectionDirection.z() *= (1 + (qrand() / (double)RAND_MAX * 2 - 1)*(1 - mtlReflGloss));
		}
		RTRRay reflectionRay(intersectPoint, reflectionDirection, RTRRay::CREATE_FROM_POINT_AND_DIRECTION);
		RTRColor reflectionColor = renderRay(reflectionRay, iterationCount + 1, intersectElement)*mtlReflColor;
		return reflectionColor * reflectionRate + diffuseColor * (1 - reflectionRate) + specColor;
	}
	else if (mtlRefracRate > 0.00001)
	{
		double IOR = refracInAir ? mtlRefracIndex : 1 / mtlRefracIndex;
		RTRVector3D refractionNormal = intersectNormal;
		if (mtlRefracGloss < 0.99999)
		{
			refractionNormal.x() *= (1 + (qrand() / (double)RAND_MAX * 2 - 1)*(1 - mtlRefracGloss));
			refractionNormal.y() *= (1 + (qrand() / (double)RAND_MAX * 2 - 1)*(1 - mtlRefracGloss));
			refractionNormal.z() *= (1 + (qrand() / (double)RAND_MAX * 2 - 1)*(1 - mtlRefracGloss));
		}
		if (refractionNormal.dotProduct(ray.direction) > 0) refractionNormal = refractionNormal * (-1);
		double c = -ray.direction.dotProduct(refractionNormal);
		double r = 1 / IOR;
		double temp = 1 - r*r*(1 - c*c);
		if (temp < 0)
		{
			return diffuseColor * (1 - mtlRefracRate) + specColor;
		}
		RTRVector3D refractionDirection = ray.direction * r + refractionNormal*(r*c - qSqrt(temp));
		RTRRay refractionRay(intersectPoint, refractionDirection, RTRRay::CREATE_FROM_POINT_AND_DIRECTION);
		RTRColor refractionColor = renderRay(refractionRay, iterationCount + 1, intersectElement, !refracInAir)*mtlRefracColor;
		return refractionColor * mtlRefracRate + diffuseColor * (1 - mtlRefracRate) + specColor;
	}
	else return diffuseColor + specColor;
}
