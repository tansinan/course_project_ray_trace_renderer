#include "RTRRenderThread.h"
#include "RTRRenderer.h"
#include "Light/RTRLightPoint.h"
#include <QImage>
#include <qdebug.h>
#include <ctime>
#include <cmath>
#include "RTRRadianceRenderer.h"

const static double PI = 3.1415926536;

RTRRenderThread::RTRRenderThread(RTRRenderer* _renderer, int _threadIndex)
{
    renderer = _renderer;
    threadIndex = _threadIndex;
    renderResult = new RTRColor[renderer->image->width()*renderer->image->height()];
}


static double circleAreaI(const RTRVector3D &p1, const RTRVector3D &p2, const RTRVector3D &p3, double radius2)
{
    RTRVector3D p1ToP2 = p1 - p2;
    RTRVector3D p1ToP3 = p1 - p3;
    RTRVector3D p2ToP3 = p3 - p2;
    RTRVector3D p2ToP3Norm = p2ToP3;
    p2ToP3Norm.vectorNormalize();
    double dist = qSqrt(p1ToP2.vectorLengthSquared() - qPow(p1ToP2.dotProduct(p2ToP3Norm), 2));
    bool longEdgeIsP1P3 = true;
    double edgeShort = p1ToP2.vectorLength();
    double edgeLong = p1ToP3.vectorLength();
    if(edgeLong < edgeShort)
    {
        std::swap(edgeLong, edgeShort);
        longEdgeIsP1P3 = false;
    }
    if(radius2 >= edgeLong * edgeLong)
    {
        return p1ToP2.crossProduct(p1ToP3).vectorLength() / 2.0;
    }
    double angle = acos(p1ToP2.dotProduct(p1ToP3) / p1ToP2.vectorLength() / p1ToP3.vectorLength());
    if(radius2 <= dist * dist)
    {
        return angle / 2 * radius2;
    }
    
    if(radius2 < edgeShort * edgeShort)
    {
        // The arc and triangle edge could have one or two intersect(s)
        if(longEdgeIsP1P3)
        {
            if(p1ToP2.dotProduct(p2ToP3) > 0)
            {
                double l = sqrt(radius2 - dist * dist);
                double angleInner = acos(dist / sqrt(radius2));
                return l * dist + (angle - angleInner * 2) / 2 * radius2;       
            }
            else return angle / 2 * radius2;
        }
        else
        {
            if(p1ToP3.dotProduct(p2ToP3) < 0)
            {
                double l = sqrt(radius2 - dist * dist);
                double angleInner = acos(dist / sqrt(radius2));
                return l * dist + (angle - angleInner * 2) / 2 * radius2;       
            }
            else return angle / 2 * radius2;
        }
    }
    else
    {
        double distanceAlongEdge = sqrt(radius2 - dist * dist);
        RTRVector3D predPoint = p2 + p2ToP3Norm * p1ToP2.dotProduct(p2ToP3Norm);
        RTRVector3D possibleIntersect1 = predPoint + p2ToP3Norm * distanceAlongEdge;
        RTRVector3D possibleIntersect2 = predPoint - p2ToP3Norm * distanceAlongEdge;
        RTRVector3D v;
        if(RTRVector3D(possibleIntersect1 - p2).dotProduct(possibleIntersect1 - p3) < 0)
        {
            v = p1 - possibleIntersect1;
        }
        else
        {
            v = p1 - possibleIntersect2;
        }
        if(longEdgeIsP1P3)
        {
            return acos(v.dotProduct(p1ToP3) / v.vectorLength() / p1ToP3.vectorLength())
                / 2 * radius2 + v.crossProduct(p1ToP2).vectorLength() / 2;
        }
        else
        {
            return acos(v.dotProduct(p1ToP2) / v.vectorLength() / p1ToP2.vectorLength())
                / 2 * radius2 + v.crossProduct(p1ToP3).vectorLength() / 2;
        }
    }
    qDebug() << "This is impossible";
    return angle / 2 * radius2;
}

RTRColor RTRRenderThread::estimateRadianceByPhotonMap(PhotonKdTree* photonMap,
    const std::vector<Photon*>& photons,
    RTRVector3D location, RTRVector3D normal, RTRRenderElement *element)
{
    if(photonMap->size() == 0)
        return RTRColor(0.0, 0.0, 0.0);
    const int MIN_ESTIMATION_PHOTON_COUNT = 16;
    const int ESTIMATION_PHOTON_COUNT = 256;
    size_t photonIndex[ESTIMATION_PHOTON_COUNT];
    double photonDistance[ESTIMATION_PHOTON_COUNT];
    nanoflann::KNNResultSet<double> resultSet(ESTIMATION_PHOTON_COUNT);
    resultSet.init(photonIndex, photonDistance);
    double query_pt[3] = { location.x(),  location.y(),  location.z() };
    if (!photonMap->findNeighbors(resultSet, query_pt, nanoflann::SearchParams())) {
        std::cout << "error!" << std::endl;
    }
    RTRColor lastEstimation(0.0, 0.0, 0.0);
    RTRColor radioEst(0.0, 0.0, 0.0);
    radioEst = estimateRadianceByPhotonMapInternal(photonMap, location, normal, photons, photonIndex,
                                        photonDistance, ESTIMATION_PHOTON_COUNT);
    /*for(int i = MIN_ESTIMATION_PHOTON_COUNT; i <= ESTIMATION_PHOTON_COUNT; i *= 2)
    {
        radioEst = estimateRadianceByPhotonMapInternal(photonMap, location, normal, photons, photonIndex,
                                            photonDistance, i);
        if(i == MIN_ESTIMATION_PHOTON_COUNT)
        {
            lastEstimation = radioEst;
        }
        else
        {
            double l1 = lastEstimation.r() + lastEstimation.g() + lastEstimation.b();
            double l2 = radioEst.r() + radioEst.g() + radioEst.b();
            if(l2 > 1.2*l1 || l2 < 0.8*l1)
                return lastEstimation;
        }
    }*/
    double largestDistance = photonDistance[ESTIMATION_PHOTON_COUNT - 1];
    double area = circleAreaI(location, element->triangle3D->vertices[0],
            element->triangle3D->vertices[1], largestDistance)
        + circleAreaI(location, element->triangle3D->vertices[1],
            element->triangle3D->vertices[2], largestDistance)
        + circleAreaI(location, element->triangle3D->vertices[2],
            element->triangle3D->vertices[0], largestDistance);
    static bool aaa = true;
    if(xBegin == 0 && yBegin == 0 && aaa)
    for(int i = 0; i < 5000; i++)
    {
        std::cout << i << ","  << circleAreaI(RTRVector3D(0,0,0), RTRVector3D(-1,1,0),
            RTRVector3D(2,1,0), qPow(i * 0.001, 2)) << endl;
        aaa = false;
    }
    //exit(0);
    return radioEst / area;
}

RTRColor RTRRenderThread::estimateRadianceByPhotonMapInternal(PhotonKdTree* photonMap, RTRVector3D location, RTRVector3D normal,
    const std::vector<Photon*>& photons, size_t *photonIndex, double *photonDistance, int photonCount)
{
    RTRColor radioEst(0.0, 0.0, 0.0);
    double largestDistance = photonDistance[photonCount - 1];
    for (int i = 0; i < photonCount; i++)
    {
        auto index = photonIndex[i];
        auto photon = photons[index];
        photon->direction.vectorNormalize();
        if (qAbs((photon->location - location).dotProduct(normal)) < 0.001)
        {
            double filter = (1 - qSqrt(photonDistance[i] / largestDistance)) * 3;
            radioEst = radioEst + photon->color * filter;
        }
    }
    return radioEst;// * (1 / PI / largestDistance);
}

RTRColor RTRRenderThread::estimateDIBySamplingObject(RTRRenderElement* element, RTRVector3D location, RTRVector3D normal)
{
    RTRColor diEstimation(0.0, 0.0, 0.0);
    const int EMISSION_SAMPLING_COUNT = 32;
    for (int i = 0; i < EMISSION_SAMPLING_COUNT; i++)
    {
        RTRVector3D directLightDirection;
        for (;;)
        {
            directLightDirection = sampler.generateRandomDirection();
            if (directLightDirection.dotProduct(normal) > 0) break;
        }
        RTRRay diRay(location, directLightDirection, RTRRay::CREATE_FROM_POINT_AND_DIRECTION);
        RTRRenderElement* emissionElement = nullptr;
        renderer->rayTracingKernel->intersect(emissionElement, diRay, element);
        if (emissionElement != nullptr && emissionElement->material->emissionStrength > 0.0001)
        {
            auto e = emissionElement->material->emissionStrength;
            diEstimation = diEstimation + RTRColor(e, e, e)
                * qAbs(directLightDirection.dotProduct(normal));
        }
    }
    diEstimation /= EMISSION_SAMPLING_COUNT;
    return diEstimation;
}

RTRColor RTRRenderThread::estimateDIBySamplingLightSource(RTRRenderElement *element, RTRVector3D location, RTRVector3D normal)
{
    auto emissionElements = renderer->emissionElements;
    const int EMISSION_SAMPLING_COUNT = 8;
    RTRColor diEstimation(0.0, 0.0, 0.0);
    for(int i = 0; i < EMISSION_SAMPLING_COUNT; i++)
    {
        auto chosenElementIndex = sampler.generateInteger(0, emissionElements.size() - 1);
        auto chosenElement = emissionElements[chosenElementIndex];
        auto lightSource = sampler.generateRandomPointInTriangle(*chosenElement->triangle3D);
        RTRVector3D lightDirection = location - lightSource;
        //lightDirection.vectorNormalize();
        RTRRay diRay(lightSource, lightDirection, RTRRay::CREATE_FROM_POINT_AND_DIRECTION);
        RTRRenderElement* rayCastElement = nullptr;
        renderer->rayTracingKernel->intersect(rayCastElement, diRay, chosenElement);
        RTRVector3D emissionNormal = chosenElement->triangle3D->plane.normal;
        emissionNormal.vectorNormalize();
        if (rayCastElement == element)
        {
            double cosDecay = -normal.dotProduct(diRay.direction);
            if(cosDecay > 0.0)
            {
                auto e = chosenElement->material->emissionStrength;
                diEstimation = diEstimation +
                    RTRColor(e, e, e) / PI / lightDirection.vectorLengthSquared()
                    * qAbs(emissionNormal.dotProduct(diRay.direction))
                    * cosDecay * chosenElement->triangle3D->area()
                    * emissionElements.size();
            }
        }
    }
    diEstimation /= EMISSION_SAMPLING_COUNT;
    return diEstimation;
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
    const RTRRenderElement* elementFrom, double refracInAir, int diffuseCount, int specularCount,
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
    renderer->rayTracingKernel->intersect(intersectElement, ray, elementFrom);

    //�������߲����κ������ཻ�ǿ϶��ǿ���һ���ˡ���

    if (intersectElement == NULL) return mtlWorldAmbient;

    //�����Ļ������󽻣����Ҽ���ֱ��������������

    RTRVector3D intersectPoint(0.0, 0.0, 0.0);
    RTRVector3D intersectNormal(0.0, 0.0, 0.0);
    RTRColor intersectColor(0.0, 0.0, 0.0);
    intersectElement->intersect(ray, intersectPoint, intersectNormal, intersectColor);
    intersectNormal.vectorNormalize();
    //if(intersectNormal.dotProduct(ray.direction) > 0)
    //    intersectNormal = intersectNormal * -1.0;

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


    RTRColor mtlAmbientColor;
    /*if (intersectElement->material->getPropertyType("ambient") == RTRMaterial::TYPE_COLOR)
    {
        mtlAmbientColor = intersectElement->material->getColorAt("ambient", 0, 0);
    }*/


    //Radiosity estimation at a diffusion surface. The current algorithm is based on
    //Monte-Carlo sampling on the emission object.
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
            renderer->rayTracingKernel->search(lightBlocker, blockTestRay, intersectElement);
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
            radianceRenderer->causticPhotons,
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
    //renderer->rayTracingKernel->search(directLight, RTRRay(intersectPoint, lightPoint.getPosition(), RTRRay::CREATE_FROM_POINTS), intersectElement);
    /*if (directLight != NULL)
    {
    diffuseColor = RTRColor(0.0, 0.0, 0.0);
    specColor = RTRColor(0.0, 0.0, 0.0);
    }*/

    //diffuseColor = diffuseColor + intersectColor * mtlAmbientColor;

    //�����������Ƶ���ʱ�����������ͷ�������

    /*if (iterationCount >= 5) return estimateRadianceByPhotonMap(radianceRenderer->diffusePhotonMap,
        radianceRenderer->diffusePhotons,
        intersectPoint, intersectNormal, intersectColor) / 200;*/// + specColor;

    //处理发光材质，原则上来说如果有发光材质应当直接返回光源亮度．
    //但是，如果历史记录表明这条光线只经过一次diffuse，问题就不一样了．那说明这个渲染结果应当由caustic photon map来处理．
    //TODO: 添加caustic photon map的开关．
    if (mtlEmissionStrength > 0.0001)
    {
        if(diffuseCount == 1 && specularCount != 0)
        {
            return RTRColor(0.0, 0.0, 0.0);
        }
        else
        return RTRColor(mtlEmissionStrength, mtlEmissionStrength, mtlEmissionStrength);
    }
    if (iterationCount >= 8) return diffuseColor;

    //TODO: refrac/refl/diffuse on material should also be determined by probability.

    //TODO: in case of
    if (reflectionRate > 0.00001)
    {
        RTRVector3D reflectionDirection(0.0, 0.0, 0.0);
        reflectionDirection = (intersectNormal * 2 * ray.direction.dotProduct(intersectNormal) - ray.direction)*-1;
        //TODO: 重新考虑节点类型
        /*if (mtlReflGloss < 0.99999)
        {
            reflectionDirection.x() *= (1 + (qrand() / (double)RAND_MAX * 2 - 1)*(1 - mtlReflGloss));
            reflectionDirection.y() *= (1 + (qrand() / (double)RAND_MAX * 2 - 1)*(1 - mtlReflGloss));
            reflectionDirection.z() *= (1 + (qrand() / (double)RAND_MAX * 2 - 1)*(1 - mtlReflGloss));
        }*/
        RTRRay reflectionRay(intersectPoint, reflectionDirection, RTRRay::CREATE_FROM_POINT_AND_DIRECTION);
        RTRColor reflectionColor = renderRay(reflectionRay, iterationCount + 1, intersectElement, refracInAir, diffuseCount, specularCount + 1)*mtlReflColor;
        //TODO: 用MC方法处理漫反射？
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
        //处理全反射
        if (temp < 0)
        {
            return diffuseColor * (1 - mtlRefracRate);// + specColor;
        }
        RTRVector3D refractionDirection = ray.direction * r + refractionNormal*(r*c - qSqrt(temp));
        refractionDirection.vectorNormalize();
        RTRRay refractionRay(intersectPoint, refractionDirection, RTRRay::CREATE_FROM_POINT_AND_DIRECTION);
        RTRColor refractionColor = renderRay(refractionRay, iterationCount + 1, intersectElement, !refracInAir, diffuseCount, specularCount + 1)*mtlRefracColor;
        return refractionColor * mtlRefracRate + diffuseColor * (1 - mtlRefracRate);// + specColor;
    }
    else
    {
        if (diffuseCount >= 1) {
            //return estimateRadianceByPhotonMap(radianceRenderer->diffusePhotonMap,
            //    radianceRenderer->diffusePhotons,
            //    intersectPoint, intersectNormal) * intersectColor;
            return estimateRadianceByPhotonMap(radianceRenderer->elementDiffusePhotonMap[intersectElement],
                *radianceRenderer->elementDiffusePhotons[intersectElement],
                intersectPoint, intersectNormal, intersectElement) * intersectColor;
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
        //RTRColor diEstimation = estimateDIBySamplingObject(intersectElement, intersectPoint, intersectNormal);
        RTRColor diEstimation = estimateDIBySamplingLightSource(intersectElement, intersectPoint, intersectNormal);
        //return diEstimation * intersectColor;
        RTRRenderElement* emissionElement = nullptr;
        renderer->rayTracingKernel->intersect(emissionElement, refractionRay, intersectElement);
        if (emissionElement == nullptr || emissionElement->material->emissionStrength > 0.0001)
        {
            return (diEstimation + estimateRadianceByPhotonMap(radianceRenderer->elementCausticPhotonMap[intersectElement],
                    *radianceRenderer->elementCausticPhotons[intersectElement],
                    intersectPoint, intersectNormal, intersectElement)) * intersectColor;
        }
        //TODO: 只需要在第一层使用caustic photon map
        return (diEstimation +
            renderRay(refractionRay, iterationCount + 1, intersectElement, refracInAir, diffuseCount + 1, specularCount)
            * qAbs(intersectNormal.dotProduct(refractionRay.direction)) * 2
                + estimateRadianceByPhotonMap(radianceRenderer->elementCausticPhotonMap[intersectElement],
                    *radianceRenderer->elementCausticPhotons[intersectElement],
                    intersectPoint, intersectNormal, intersectElement)) * intersectColor;
    }
}
