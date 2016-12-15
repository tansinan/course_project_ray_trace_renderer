#include "RTRRenderThread.h"
#include "RTRRenderer.h"
#include "Light/RTRLightPoint.h"
#include <QImage>
#include <qdebug.h>
#include <ctime>
#include "RTRRadianceRenderer.h"

const static double PI = 3.1415926536;

RTRRenderThread::RTRRenderThread(RTRRenderer* _renderer, int _threadIndex)
{
    renderer = _renderer;
    threadIndex = _threadIndex;
    renderResult = new RTRColor[renderer->image->width()*renderer->image->height()];
}

RTRColor RTRRenderThread::estimateRadianceByPhotonMap(PhotonKdTree* photonMap, 
    const std::vector<Photon*>& photons,
    RTRVector3D location, RTRVector3D normal, RTRColor color)
{
    const int ESTIMATION_PHOTON_COUNT = 512;
    size_t photonIndex[ESTIMATION_PHOTON_COUNT];
    double photonDistance[ESTIMATION_PHOTON_COUNT];
    nanoflann::KNNResultSet<double> resultSet(ESTIMATION_PHOTON_COUNT);
    resultSet.init(photonIndex, photonDistance);
    double query_pt[3] = { location.x(),  location.y(),  location.z() };
    if (!photonMap->findNeighbors(resultSet, query_pt, nanoflann::SearchParams())) {
        std::cout << "error!" << std::endl;
    }
    double largestDistance = photonDistance[ESTIMATION_PHOTON_COUNT - 1];
    RTRColor radioEst(0.0, 0.0, 0.0);
    for (int i = 0; i < ESTIMATION_PHOTON_COUNT; i++)
    {
        auto index = photonIndex[i];
        //qDebug() << index;
        auto photon = photons[index];
        photon->direction.vectorNormalize();
        if (qAbs((photon->location - location).dotProduct(normal)) < 0.001)
        {
            double filter = (1 - qSqrt(photonDistance[i] / largestDistance)) * 3;
            radioEst = radioEst + color * photon->color * filter;
        }
    }
    return radioEst * (1 / PI / largestDistance);
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
    qsrand(clock() ^ time(0));
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
    RTRVector3D pivot = renderer->camera->rotationMatrix * RTRVector3D(0, 0, 1);
    pivot = RTRVector3D(1, 0, 0);
    for (int i = xBegin; i <= xEnd; i++)
    {
        for (int j = yBegin; j <= yEnd; j++)
        {
            RTRColor result(0.0, 0.0, 0.0);

            double antiAliasOffsetX = sampler.generateRandomNumber() - 0.5;
            double antiAliasOffsetY = sampler.generateRandomNumber() - 0.5;

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
            //qDebug() << qAbs(ray.direction.dotProduct(pivot));
            //result = result * qAbs(ray.direction.dotProduct(pivot));
            renderResult[i*renderer->image->height() + j] = result;
        }
    }
    emit renderFinished(threadIndex);
}

RTRColor RTRRenderThread::renderRay(const RTRRay& ray, int iterationCount,
    const RTRRenderElement* elementFrom, double refracInAir, int diffuseCount,
    bool directOnly)
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
    intersectNormal.vectorNormalize();

    //���������Ĳ������ԣ����������ʺͷ�����ɫ���߹⡢�����ʵ�

    double reflectionRate = intersectElement->material->reflectionRate;
    RTRColor mtlReflColor = intersectElement->material->reflectionColor;
    double mtlReflGloss = 1.0;
    RTRColor mtlSpecColor;
    double mtlRefracRate = intersectElement->material->refractionRate;
    double mtlRefracIndex = intersectElement->material->refractionIndex;
    RTRColor mtlRefracColor = intersectElement->material->refractionColor;
    double mtlRefracGloss = 1.0;
    double mtlEmissionStrength = intersectElement->material->emissionStrength;

    //处理反射相关属性

    //if (intersectElement->material->getPropertyType("reflection_glossiness") == RTRMaterial::TYPE_COLOR)
    //  mtlReflGloss = intersectElement->material->getColorAt("reflection_glossiness", 0, 0).r();

    //�߹���������

    //if (intersectElement->material->getPropertyType("specular") == RTRMaterial::TYPE_COLOR)
    //  mtlSpecColor = intersectElement->material->getColorAt("specular", 0, 0);

    //������������

    //if (intersectElement->material->getPropertyType("refraction_glossiness") == RTRMaterial::TYPE_COLOR)
    //mtlRefracGloss = intersectElement->material->getColorAt("refraction_glossiness", 0, 0).r();


    RTRColor mtlAmbientColor;
    if (intersectElement->material->getPropertyType("ambient") == RTRMaterial::TYPE_COLOR)
    {
        mtlAmbientColor = intersectElement->material->getColorAt("ambient", 0, 0);
    }


    //Radiosity estimation at a diffusion surface. The current algorithm is based on
    //Monte-Carlo sampling on the emission object.
    const int EMISSION_SAMPLING_COUNT = 32;
    RTRColor diffuseColor(0.0, 0.0, 0.0);
    /*for (int i = 0; i < EMISSION_SAMPLING_COUNT; i++)
    {
        //First, we randomly choose a point on the emission object.
        //TODO: This is currently hard-coded.
        RTRLightPoint lightPoint(RTRVector3D(-1.2 + sampler.generateRandomNumber(-1.0, 1.0)
            , 0 + sampler.generateRandomNumber(-2.0, 2.0)
            , 4.97), RTRColor(1.0, 1.0, 1.0), 10.0);

        //Then we evaluate the diffusion radiosity
        RTRColor outRadiositySample(0.0, 0.0, 0.0);

        //TODO: 4.99 is a hard-coded value.
        if (intersectPoint.z() < 4.99)
        {
            RTRVector3D lightDirection = lightPoint.directionAt(intersectPoint);

            //Evaluate the cosine as "decay"
            double sourceDecay = qAbs(lightDirection.dotProduct(RTRVector3D(0, 0, 1)));
            double decay = lightDirection.dotProduct(intersectNormal);
            int sym1 = sgn(decay);
            int sym2 = sgn((intersectPoint - ray.beginningPoint).dotProduct(intersectNormal));
            decay = decay > 0 ? decay : -decay;
            RTRColor lightColor = lightPoint.colorAt(intersectPoint);
            RTRRenderElement* lightBlocker = NULL;
            RTRRay blockTestRay = RTRRay(intersectPoint, lightPoint.getPosition(), RTRRay::CREATE_FROM_POINTS);
            renderer->elementsCache->search(lightBlocker, blockTestRay, intersectElement);
            double distanceToLight = (intersectPoint - lightPoint.getPosition()).vectorLengthSquared();
            RTRVector3D blkIntersectPoint(0.0, 0.0, 0.0);
            RTRVector3D blkIntersectNormal(0.0, 0.0, 0.0);
            RTRColor blkIntersectColor(0.0, 0.0, 0.0);
            double distanceToBlocker = 1e10;
            if (lightBlocker != nullptr)
            {
                lightBlocker->intersect(blockTestRay, blkIntersectPoint, blkIntersectNormal, blkIntersectColor);
                distanceToBlocker = (intersectPoint - blkIntersectPoint).vectorLengthSquared();
            }
            if (sym1 == sym2 && distanceToLight < distanceToBlocker)
            {
                outRadiositySample = intersectColor * lightColor * (decay * sourceDecay);
                //RTRVector3D specularDirection = (intersectNormal * 2 * ray.direction.dotProduct(intersectNormal) - ray.direction)*-1;
                //specularDirection.vectorNormalize();
                //double spec = qAbs(specularDirection.dotProduct(lightDirection));
                //specColor = mtlSpecColor*lightColor*qPow(spec, 2);
            }
            //outRadiositySample = lightPoint.directionAt(intersectPoint);
        }
        diffuseColor = diffuseColor + outRadiositySample;
    }
    diffuseColor /= EMISSION_SAMPLING_COUNT;*/

    //diffuseColor = RTRColor(0, 0, 0);


    /*diffuseColor =
        estimateRadianceByPhotonMap(radianceRenderer->causticPhotonMap,
            radianceRenderer->stlCausticPhotons,
            intersectPoint, intersectNormal, intersectColor) / 500;*/


    /*RTRLightPoint lightPoint(RTRVector3D(-2 + (qrand() / (double)RAND_MAX * 2 - 1)
    , 0 + (qrand() / (double)RAND_MAX * 2 - 1)
    , 4.97), RTRColor(1, 1, 1), 1.0);
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
    }*/

    //������Ӱ��ֻ���Ǿֲ����գ������䲻���ĵط�û��Diffuse��Specular
    //RTRRenderElement* directLight = NULL;
    //renderer->elementsCache->search(directLight, RTRRay(intersectPoint, lightPoint.getPosition(), RTRRay::CREATE_FROM_POINTS), intersectElement);
    /*if (directLight != NULL)
    {
    diffuseColor = RTRColor(0.0, 0.0, 0.0);
    specColor = RTRColor(0.0, 0.0, 0.0);
    }*/

    //diffuseColor = diffuseColor + intersectColor * mtlAmbientColor;

    //�����������Ƶ���ʱ�����������ͷ�������

    /*if (iterationCount >= 5) return estimateRadianceByPhotonMap(radianceRenderer->diffusePhotonMap,
        radianceRenderer->stlDiffusePhotons,
        intersectPoint, intersectNormal, intersectColor) / 200;*/// + specColor;

    if (mtlEmissionStrength > 0.0001)
    {
        return RTRColor(mtlEmissionStrength, mtlEmissionStrength, mtlEmissionStrength);
    }
    if (iterationCount >= 8) return diffuseColor;
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
        RTRColor reflectionColor = renderRay(reflectionRay, iterationCount + 1, intersectElement, refracInAir, diffuseCount)*mtlReflColor;
        return reflectionColor * reflectionRate + diffuseColor * (1 - reflectionRate);// + specColor;
    }
    else if (mtlRefracRate > 0.00001)
    {
        double IOR = refracInAir ? mtlRefracIndex : 1 / mtlRefracIndex;
        RTRVector3D refractionNormal = intersectNormal;
        /*if (mtlRefracGloss < 0.99999)
        {
        refractionNormal.x() *= (1 + (qrand() / (double)RAND_MAX * 2 - 1)*(1 - mtlRefracGloss));
        refractionNormal.y() *= (1 + (qrand() / (double)RAND_MAX * 2 - 1)*(1 - mtlRefracGloss));
        refractionNormal.z() *= (1 + (qrand() / (double)RAND_MAX * 2 - 1)*(1 - mtlRefracGloss));
        }*/
        refractionNormal.vectorNormalize();
        if (refractionNormal.dotProduct(ray.direction) > 0) refractionNormal = refractionNormal * (-1);
        double c = -ray.direction.dotProduct(refractionNormal);
        double r = 1 / IOR;
        double temp = 1 - r*r*(1 - c*c);
        if (temp < 0)
        {
            qDebug() << temp << r;
            return diffuseColor * (1 - mtlRefracRate);// + specColor;
        }
        RTRVector3D refractionDirection = ray.direction * r + refractionNormal*(r*c - qSqrt(temp));
        refractionDirection.vectorNormalize();
        RTRRay refractionRay(intersectPoint, refractionDirection, RTRRay::CREATE_FROM_POINT_AND_DIRECTION);
        RTRColor refractionColor = renderRay(refractionRay, iterationCount + 1, intersectElement, !refracInAir, diffuseCount)*mtlRefracColor;
        return refractionColor * mtlRefracRate + diffuseColor * (1 - mtlRefracRate);// + specColor;
    }
    else
    {
        if (diffuseCount >= 1) {
            return estimateRadianceByPhotonMap(radianceRenderer->diffusePhotonMap,
                radianceRenderer->stlDiffusePhotons,
                intersectPoint, intersectNormal, intersectColor);
        }
        RTRVector3D nextDirection;
        if (intersectNormal.dotProduct(ray.direction) > 0)
        {
            intersectNormal = intersectNormal * -1.0;
        }
        for (;;)
        {
            nextDirection = sampler.generateRandomDirection();
            if (nextDirection.dotProduct(intersectNormal) > 0) break;
        }
        RTRRay refractionRay(intersectPoint, nextDirection, RTRRay::CREATE_FROM_POINT_AND_DIRECTION);
        RTRColor directLightSampling(0, 0, 0);
        for (int i = 0; i < EMISSION_SAMPLING_COUNT; i++)
        {
            RTRVector3D directLightDirection;
            for (;;)
            {
                directLightDirection = sampler.generateRandomDirection();
                if (directLightDirection.dotProduct(intersectNormal) > 0) break;
            }
            RTRRay diRay(intersectPoint, directLightDirection, RTRRay::CREATE_FROM_POINT_AND_DIRECTION);
            RTRRenderElement* emissionElement = nullptr;
            renderer->elementsCache->search(emissionElement, diRay, intersectElement);
            RTRVector3D normal;
            if (emissionElement != nullptr && emissionElement->material->emissionStrength > 0.0001)
            {
                auto e = emissionElement->material->emissionStrength;
                directLightSampling = directLightSampling + RTRColor(e, e, e) 
                    * qAbs(directLightDirection.dotProduct(intersectNormal));
            }
        }
        directLightSampling /= EMISSION_SAMPLING_COUNT;
        RTRRenderElement* emissionElement = nullptr;
        renderer->elementsCache->search(emissionElement, refractionRay, intersectElement);
        if (emissionElement == nullptr || emissionElement->material->emissionStrength > 0.0001)
        {
            return directLightSampling * intersectColor * 2;
        }
        return directLightSampling * intersectColor * 2 +
            renderRay(refractionRay, iterationCount + 1, intersectElement, refracInAir, diffuseCount + 1) * intersectColor
            * qAbs(intersectNormal.dotProduct(refractionRay.direction)) * 2;
    }
}
