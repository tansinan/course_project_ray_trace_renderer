#include <random>
#include <vector>
#include <QVector>
#include "RTRKdTree.h"
#include "RTRRadianceRenderer.h"
#include "RTRRenderElement.h"

void RTRRadianceRenderer::renderPhoton(
    RTRVector3D location, RTRVector3D direction, QVector<Photon*> &result,
    RTRRenderElement *emissionElement, bool causticOnly)
{
    RTRColor photonColor = RTRColor(1.0, 1.0, 1.0);
    photonColor = photonColor * qAbs(direction.z());
    bool refracInAir = true;
    RTRRenderElement* intersectElement = emissionElement;
    Photon::Type photonType = Photon::Type::DIRECT;
    for(int i = 0; i < 10; i++)
    {
        //qDebug() << location.x() << location.y() << location.z();
        //qDebug() << "D" << direction.x() << direction.y() << direction.z();

        RTRRay ray(location, direction, RTRRay::CREATE_FROM_POINT_AND_DIRECTION);
        elementsCache->search(intersectElement, ray, intersectElement);
        if(intersectElement == nullptr)
        {
            //qDebug() << "No element";
            /*if(i != 0)
            {
                result.append(photon);
            }*/
            return;
        }
        RTRVector3D intersectPoint;
        RTRVector3D intersectNormal;
        RTRColor intersectColor;
        intersectElement->intersect(ray, intersectPoint, intersectNormal, intersectColor);

        double reflectionRate = intersectElement->material->reflectionRate;
        RTRColor mtlReflColor= intersectElement->material->reflectionColor;
        double mtlReflGloss = 1.0;
        RTRColor mtlSpecColor;
        double mtlRefracRate = intersectElement->material->refractionRate;
        double mtlRefracIndex = intersectElement->material->refractionIndex;
        RTRColor mtlRefracColor(1.0, 1.0, 1.0);
        double mtlRefracGloss = 1.0;
        double mtlDiffuseRate = 0.0;

        if (intersectElement->material->getPropertyType("reflection_glossiness") == RTRMaterial::TYPE_COLOR)
            mtlReflGloss = intersectElement->material->getColorAt("reflection_glossiness", 0, 0).r();

        if (intersectElement->material->getPropertyType("specular") == RTRMaterial::TYPE_COLOR)
            mtlSpecColor = intersectElement->material->getColorAt("specular", 0, 0);

        if (intersectElement->material->getPropertyType("refraction_color") == RTRMaterial::TYPE_COLOR)
            mtlRefracColor = intersectElement->material->getColorAt("refraction_color", 0, 0);
        if (intersectElement->material->getPropertyType("refraction_glossiness") == RTRMaterial::TYPE_COLOR)
            mtlRefracGloss = intersectElement->material->getColorAt("refraction_glossiness", 0, 0).r();

        mtlDiffuseRate = intersectElement->material->getColorAt("Kd", 0, 0).r();
        //Decide which kind of intersection is happening
        if (reflectionRate > 0.00001 && rand() / (float)RAND_MAX < reflectionRate)
        {
            if (intersectNormal.dotProduct(ray.direction) < 0)
            {
                intersectNormal = intersectNormal * (-1.0);
            }
            RTRVector3D reflectionDirection(0.0, 0.0, 0.0);
            reflectionDirection = (intersectNormal * 2 * ray.direction.dotProduct(intersectNormal) - ray.direction)*-1;
            /*if (mtlReflGloss < 0.99999)
            {
                reflectionDirection.x() *= (1 + (qrand() / (double)RAND_MAX * 2 - 1)*(1 - mtlReflGloss));
                reflectionDirection.y() *= (1 + (qrand() / (double)RAND_MAX * 2 - 1)*(1 - mtlReflGloss));
                reflectionDirection.z() *= (1 + (qrand() / (double)RAND_MAX * 2 - 1)*(1 - mtlReflGloss));
            }*/
            location = intersectPoint;
            direction = reflectionDirection;
            photonColor = RTRColor(
                        photonColor.r() * mtlReflColor.r(),photonColor.g() * mtlReflColor.g(),photonColor.b() * mtlReflColor.b());
            if(photonType == Photon::Type::DIRECT || photonType == Photon::Type::SPECULAR)
                photonType = Photon::Type::SPECULAR;
            else
                photonType = Photon::Type::DIFFUSE_INTERREFLECTION;
        }
        else if (mtlRefracRate > 0.00001 && rand() / (float)RAND_MAX < mtlRefracRate)
        {
            double IOR = refracInAir ? mtlRefracIndex : 1 / mtlRefracIndex;
            RTRVector3D refractionNormal = intersectNormal;
            /*if (mtlRefracGloss < 0.99999)
            {
                refractionNormal.x() *= (1 + (qrand() / (double)RAND_MAX * 2 - 1)*(1 - mtlRefracGloss));
                refractionNormal.y() *= (1 + (qrand() / (double)RAND_MAX * 2 - 1)*(1 - mtlRefracGloss));
                refractionNormal.z() *= (1 + (qrand() / (double)RAND_MAX * 2 - 1)*(1 - mtlRefracGloss));
            }*/
            if (refractionNormal.dotProduct(ray.direction) > 0) refractionNormal = refractionNormal * (-1);
            double c = -ray.direction.dotProduct(refractionNormal);
            double r = IOR;// 1 / IOR;
            double temp = 1 - r*r*(1 - c*c);
            if(photonType == Photon::Type::DIRECT || photonType == Photon::Type::SPECULAR)
                photonType = Photon::Type::SPECULAR;
            else
                photonType = Photon::Type::DIFFUSE_INTERREFLECTION;
            if (temp > 0)
            {
                RTRVector3D refractionDirection = ray.direction * r + refractionNormal*(r*c - qSqrt(temp));
                location = intersectPoint;
                direction = refractionDirection;
                refracInAir = !refracInAir;
                photonColor = RTRColor(
                    photonColor.r() * mtlRefracColor.r(),
                    photonColor.g() * mtlRefracColor.g(),
                    photonColor.b() * mtlRefracColor.b()
                );
            }
            else return;
        }
        else
        {
            if(photonType == Photon::Type::SPECULAR)
                photonType = Photon::Type::CAUSTIC;
            else if(photonType == Photon::Type::DIRECT)
                photonType = Photon::Type::DIFFUSE;
            else
                photonType = Photon::Type::DIFFUSE_INTERREFLECTION;
            if((causticOnly && photonType == Photon::Type::CAUSTIC) ||
               (!causticOnly /*&& photonType != Photon::Type::CAUSTIC*/))
            {
                Photon *photon = new Photon();
                photon->color = photonColor;
                photon->direction = direction;
                photon->location = intersectPoint;
                photon->type = photonType;

                result.append(photon);

                RTRColor diffuseColor(0.0, 0.0, 0.0);
                RTRColor specColor;
                double decay = direction.dotProduct(intersectNormal);
                int sym1 = sgn(decay);
                int sym2 = sgn((intersectPoint - ray.beginningPoint).dotProduct(intersectNormal));
                decay = decay > 0 ? decay : -decay;
                if (sym1 == sym2)
                {
                    diffuseColor = intersectColor * photonColor * decay;
                    RTRVector3D specularDirection = (intersectNormal * 2 * ray.direction.dotProduct(intersectNormal) - ray.direction)*-1;
                    specularDirection.vectorNormalize();
                    double spec = qAbs(specularDirection.dotProduct(direction));
                    specColor = mtlSpecColor*photonColor*qPow(spec, 2);
                }

                photonColor = RTRColor(
                            photonColor.r() * diffuseColor.r(),
                            photonColor.g() * diffuseColor.g(),
                            photonColor.b() * diffuseColor.b()
                        );
            }
            if(causticOnly/* && photonType != Photon::Type::SPECULAR*/)
            {
                return;
            }
            RTRVector3D nextPhotonDirection;
            //qDebug() << "N" << intersectNormal.x() << intersectNormal.y() << intersectNormal.z();
            if(intersectNormal.dotProduct(direction) > 0)
            {
                intersectNormal = intersectNormal * -1.0;
            }
            for(;;)
            {
                nextPhotonDirection = sampler->generateRandomDirection();
                if(nextPhotonDirection.dotProduct(intersectNormal) > 0) break;
            }
            //qDebug() << "DIFFUSE";
            location = intersectPoint;
            direction = nextPhotonDirection;
            //if(sampler->generateRandomNumber() > 0.9)
            //{
            //    return;
            //}
            //result.append(photon);
        }
    }
}

void RTRRadianceRenderer::execute()
{
    //If kd-tree exists, free them.
    if (diffusePhotonMap != nullptr)
    {
        delete diffusePhotonMap;
    }
    if (causticPhotonMap != nullptr)
    {
        delete causticPhotonMap;
    }

    std::vector<RTRRenderElement*> emissionElements;
    for (auto element : elements)
    {
        if (element->material->emissionStrength > 0.0001)
        {
            //qDebug() << "Emission object!";
            emissionElements.push_back(element);
        }
    }

    /*for (int i = 0; i < 20; i++)
    {
        qDebug() << sampler->generateInteger(0, emissionElements.size() - 1);
    }*/

    //QVector<Photon*> diffusePhotons;
    const int PHOTON_COUNT = 500000;

    while(allPhotons.size() < PHOTON_COUNT)
    {
        RTRVector3D lightSource(-1.2 + sampler->generateRandomNumber(-1.0, 1.0)
                                                        , 0 + sampler->generateRandomNumber(-2.0, 2.0)
                                                        , 4.99);
        RTRVector3D lightDirection;
        lightDirection = sampler->generateRandomDirection();
        /*do {
            lightDirection = sampler->generateRandomDirection();
        } while(lightDirection.z() > 0);*/
        lightDirection.vectorNormalize();
        renderPhoton(lightSource, lightDirection, allPhotons, false);
        if (allPhotons.size() % 1000 == 0)
        {
            qDebug() << allPhotons.size();
        }
    }
    int causticPhotonCount = 0;
    int diffusePhotonCount = 0;
    for(int i = 0; i < PHOTON_COUNT; i++)
    {
        if(allPhotons[i]->type == Photon::Type::CAUSTIC)
        {
            causticPhotonCount++;
        }
        else
        {
            diffusePhotonCount++;
        }
    }
    qDebug() << "CAUSTIC " << causticPhotonCount;
    qDebug() << "DIFFUSE " << diffusePhotonCount;
    stlDiffusePhotons.clear();
    stlDiffusePhotons = allPhotons.toStdVector();
    while(causticPhotons.size() < 5000)
    {
        RTRVector3D lightSource(-1.2 + sampler->generateRandomNumber(-1.0, 1.0)
                                                          , 0 + sampler->generateRandomNumber(-2.0, 2.0)
                                                          , 4.99);
        RTRVector3D lightDirection;
        do {
            lightDirection = sampler->generateRandomDirection();
        } while(lightDirection.z() > 0);
        lightDirection.vectorNormalize();
        renderPhoton(lightSource, lightDirection, causticPhotons, true);
        //qDebug() << causticPhotons.size();
        if (causticPhotons.size() > 0 && causticPhotons.size() % 1000 == 0)
        {
            qDebug() << causticPhotons.size();
        }
    }
    stlCausticPhotons.clear();
    stlCausticPhotons = causticPhotons.toStdVector();
    causticPhotonAdapter = new NanoFlannPhotonAdaptor(stlCausticPhotons);
    diffusePhotonAdapter = new NanoFlannPhotonAdaptor(stlDiffusePhotons);
    causticPhotonMap = new PhotonKdTree(3, *causticPhotonAdapter);
    causticPhotonMap->buildIndex();
    diffusePhotonMap = new PhotonKdTree(3, *diffusePhotonAdapter);
    diffusePhotonMap->buildIndex();
    //causticPhotonMap = NearestSearchKdTree<Photon*, double, 3, AccessRTRVector3D>::construct(causticPhotons.toStdVector());
    //diffusePhotonMap = NearestSearchKdTree<Photon, double, 3, AccessRTRVector3D>::construct(diffusePhotons.toStdVector());
    qDebug() << causticPhotons.size();
    qDebug() << stlDiffusePhotons.size();
}
