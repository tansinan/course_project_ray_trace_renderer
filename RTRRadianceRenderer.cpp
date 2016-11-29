#include <random>
#include <QVector>
#include "RTRKdTree.h"
#include "RTRRadianceRenderer.h"
#include "RTRRenderElement.h"

void RTRRadianceRenderer::renderPhoton(RTRVector3D location, RTRVector3D direction, QVector<Photon> &result, bool causticOnly)
{
    RTRColor photonColor = RTRColor(1.0, 1.0, 1.0);
    bool refracInAir = true;
    RTRRenderElement* intersectElement = nullptr;
    int causticStage = 0;
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

        double reflectionRate = 0.0;
        RTRColor mtlReflColor(1.0, 1.0, 1.0);
        double mtlReflGloss = 1.0;
        RTRColor mtlSpecColor;
        double mtlRefracRate = 0.0;
        double mtlRefracIndex = 1.0;
        RTRColor mtlRefracColor(1.0, 1.0, 1.0);
        double mtlRefracGloss = 1.0;
        double mtlDiffuseRate = 0.0;

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

        if (intersectElement->material->getPropertyType("reflection_rate") == RTRMaterial::TYPE_COLOR)
            reflectionRate = intersectElement->material->getColorAt("reflection_rate", 0, 0).r();
        if (intersectElement->material->getPropertyType("reflection_color") == RTRMaterial::TYPE_COLOR)
            mtlReflColor = intersectElement->material->getColorAt("reflection_color", 0, 0);
        if (intersectElement->material->getPropertyType("reflection_glossiness") == RTRMaterial::TYPE_COLOR)
            mtlReflGloss = intersectElement->material->getColorAt("reflection_glossiness", 0, 0).r();

        if (intersectElement->material->getPropertyType("specular") == RTRMaterial::TYPE_COLOR)
            mtlSpecColor = intersectElement->material->getColorAt("specular", 0, 0);

        if (intersectElement->material->getPropertyType("refraction_rate") == RTRMaterial::TYPE_COLOR)
            mtlRefracRate = intersectElement->material->getColorAt("refraction_rate", 0, 0).r();
        if (intersectElement->material->getPropertyType("refraction_index") == RTRMaterial::TYPE_COLOR)
            mtlRefracIndex = intersectElement->material->getColorAt("refraction_index", 0, 0).r();
        if (intersectElement->material->getPropertyType("refraction_color") == RTRMaterial::TYPE_COLOR)
            mtlRefracColor = intersectElement->material->getColorAt("refraction_color", 0, 0);
        if (intersectElement->material->getPropertyType("refraction_glossiness") == RTRMaterial::TYPE_COLOR)
            mtlRefracGloss = intersectElement->material->getColorAt("refraction_glossiness", 0, 0).r();

        mtlDiffuseRate = intersectElement->material->getColorAt("Kd", 0, 0).r();
        //Decide which kind of intersection is happening
        if (reflectionRate > 0.00001 && rand() / (float)RAND_MAX < reflectionRate)
        {
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
            if(causticStage == 0 || causticStage == 1)
                causticStage = 1;
            else
                causticStage = 3;
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
            double r = 1 / IOR;
            double temp = 1 - r*r*(1 - c*c);
            if(causticStage == 0 || causticStage == 1)
                causticStage = 1;
            else
                causticStage = 3;
            if (temp > 0)
            {
                RTRVector3D refractionDirection = ray.direction * r + refractionNormal*(r*c - qSqrt(temp));
                location = intersectPoint;
                direction = refractionDirection;
                refracInAir = !refracInAir;
                photonColor = RTRColor(
                    photonColor.r() * mtlReflColor.r(),
                    photonColor.g() * mtlReflColor.g(),
                    photonColor.b() * mtlReflColor.b()
                );
            }
            else return;
        }
        else
        {
            if(causticStage == 1)
                causticStage = 2;
            else
                causticStage = 3;
            if(causticStage == 3 && causticOnly)
            {
                return;
            }
            Photon photon;
            photon.color = photonColor;
            photon.direction = direction;
            photon.location = intersectPoint;
            result.append(photon);
            RTRVector3D nextPhotonDirection;
            photonColor = RTRColor(
                        photonColor.r() * diffuseColor.r(),
                        photonColor.g() * diffuseColor.g(),
                        photonColor.b() * diffuseColor.b()
                    );
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
            if(sampler->generateRandomNumber() > 0.9)
            {
                return;
            }
            //result.append(photon);
        }
    }
}
