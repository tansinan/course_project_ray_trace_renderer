#include "RTRRenderElement.h"

RTRRenderElement::RTRRenderElement()
{

}

const RTRBoundingBox& RTRRenderElement::getBoundingBox() const
{
	return boundingBox;
}

RTRRenderElement::RTRRenderElement(RTRTriangle3D* _triangle3D, RTRCamera* camera)
{
	material = NULL;
	triangle3D = _triangle3D;
	for (int i = 0; i < 3; i++)
	{
		boundingBox.point1(i) = 1e50;
		boundingBox.point2(i) = -1e50;
	}
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			boundingBox.point1(j) = qMin(triangle3D->vertices[i](j), boundingBox.point1(j));
			boundingBox.point2(j) = qMax(triangle3D->vertices[i](j), boundingBox.point2(j));
		}
	}

	double areaZ = qAbs((triangle3D->vertices[1].x() - triangle3D->vertices[0].x())*(triangle3D->vertices[2].y() - triangle3D->vertices[0].y())
		- (triangle3D->vertices[1].y() - triangle3D->vertices[0].y())*(triangle3D->vertices[2].x() - triangle3D->vertices[0].x()));
	double areaX = qAbs((triangle3D->vertices[1].z() - triangle3D->vertices[0].z())*(triangle3D->vertices[2].y() - triangle3D->vertices[0].y())
		- (triangle3D->vertices[1].y() - triangle3D->vertices[0].y())*(triangle3D->vertices[2].z() - triangle3D->vertices[0].z()));
	double areaY = qAbs((triangle3D->vertices[1].z() - triangle3D->vertices[0].z())*(triangle3D->vertices[2].x() - triangle3D->vertices[0].x())
		- (triangle3D->vertices[1].x() - triangle3D->vertices[0].x())*(triangle3D->vertices[2].z() - triangle3D->vertices[0].z()));

	RTRVector2D vert1, vert2, vert3;

	if (areaX >= areaY&&areaX >= areaZ)
	{
		orthProjectDirection = 0;
		vert1.x() = triangle3D->vertices[0].y();
		vert1.y() = triangle3D->vertices[0].z();
		vert2.x() = triangle3D->vertices[1].y();
		vert2.y() = triangle3D->vertices[1].z();
		vert3.x() = triangle3D->vertices[2].y();
		vert3.y() = triangle3D->vertices[2].z();
	}
	else if (areaZ > areaX&&areaZ > areaY)
	{
		orthProjectDirection = 2;
		vert1.x() = triangle3D->vertices[0].x();
		vert1.y() = triangle3D->vertices[0].y();
		vert2.x() = triangle3D->vertices[1].x();
		vert2.y() = triangle3D->vertices[1].y();
		vert3.x() = triangle3D->vertices[2].x();
		vert3.y() = triangle3D->vertices[2].y();
	}
	else
	{
		orthProjectDirection = 1;
		vert1.x() = triangle3D->vertices[0].x();
		vert1.y() = triangle3D->vertices[0].z();
		vert2.x() = triangle3D->vertices[1].x();
		vert2.y() = triangle3D->vertices[1].z();
		vert3.x() = triangle3D->vertices[2].x();
		vert3.y() = triangle3D->vertices[2].z();
	}
	orthProjectTriangle = new RTRTriangle2D(vert1, vert2, vert3);
    auto AB = triangle3D->vertices[1] - triangle3D->vertices[0];
    auto AC = triangle3D->vertices[2] - triangle3D->vertices[0];
    areaDouble = (AB(0) * AC(1) - AB(1) * AC(0));
}

bool RTRRenderElement::intersect(const RTRRay& ray, RTRVector3D& result, RTRVector3D& normal, RTRColor& color) const
{
	result = RTRGeometry::intersect(triangle3D->plane, ray);
	RTRVector2D temp;
	bool ret = true;
	if (orthProjectDirection == 0)
	{
		temp.x() = result.y();
		temp.y() = result.z();
		ret = RTRGeometry::pointInsideTriangle(*orthProjectTriangle,temp);
	}
	else if (orthProjectDirection == 1)
	{
		temp.x() = result.x();
		temp.y() = result.z();
		ret =  RTRGeometry::pointInsideTriangle(*orthProjectTriangle, temp);
	}
	if (orthProjectDirection == 2)
	{
		temp.x() = result.x();
		temp.y() = result.y();
		ret = RTRGeometry::pointInsideTriangle(*orthProjectTriangle, temp);
	}
	if (!ret) return false;

	double a1, a2, a3;
	RTRVector3D line12 = triangle3D->vertices[1] - triangle3D->vertices[0];
	RTRVector3D line13 = triangle3D->vertices[2] - triangle3D->vertices[0];
	RTRVector3D line1p = result - triangle3D->vertices[0];

	if (orthProjectDirection == 0)
	{
		double det = line13.y() * line12.z() - line12.y() * line13.z();
		a2 = line1p.y() * line13.z() - line13.y() * line1p.z();
		a2 = qAbs(a2 / det);
		a3 = line1p.y() * line12.z() - line12.y() * line1p.z();
		a3 = qAbs(a3 / det);
		a1 = 1 - a2 - a3;
	}
	else if (orthProjectDirection == 1)
	{
		double det = line13.x() * line12.z() - line12.x() * line13.z();
		a2 = line1p.x() * line13.z() - line13.x() * line1p.z();
		a2 = qAbs(a2 / det);
		a3 = line1p.x() * line12.z() - line12.x() * line1p.z();
		a3 = qAbs(a3 / det);
		a1 = 1 - a2 - a3;
	}
	else if (orthProjectDirection == 2)
	{
		double det = line13.x() * line12.y() - line12.x() * line13.y();
		a2 = line1p.x() * line13.y() - line13.x() * line1p.y();
		a2 = qAbs(a2 / det);
		a3 = line1p.x() * line12.y() - line12.x() * line1p.y();
		a3 = qAbs(a3 / det);
		a1 = 1 - a2 - a3;
	}

	//处理Phong法线平滑着色

	if (useSmoothShading)
	{
		normal = vertexNormals[0] * a1 + vertexNormals[1] * a2 + vertexNormals[2] * a3;
	}
	else
	{
		normal = triangle3D->plane.normal;
	}


	//处理漫反射颜色贴图

	if (material != NULL)
	{
		if (material->getPropertyType("diffuse")==RTRMaterial::TYPE_COLOR) color = material->getColorAt("diffuse", 0, 0);
		else if (material->getPropertyType("diffuse") == RTRMaterial::TYPE_TEXTURE)
		{
			RTRVector2D uvPos = vertexUVMaps[0] * a1 + vertexUVMaps[1] * a2 + vertexUVMaps[2] * a3;
			color = material->getColorAt("diffuse", uvPos.x(), 1 - uvPos.y());
		}
	}
	else
	{
		color.r() = color.g() = color.b() = 1.0;
	}
	return ret;
}

bool RTRRenderElement::intersect(const RTRRay& ray, RTRVector3D& result)
{
    double ratio = 0.0;
    double d1 = RTRGeometry::distance(ray.beginningPoint, triangle3D->plane);
    double d2 = RTRGeometry::distance(ray.endPoint, triangle3D->plane);
    if (d1 > 0 && d2 > 0) return false;
    if (d1 < 0 && d2 < 0) return false;
    ratio = d1 / (d1 - d2);
    result = ray.beginningPoint + (ray.endPoint - ray.beginningPoint) * ratio;
	//result = RTRGeometry::intersect(triangle3D->plane, ray);
    auto PB = triangle3D->vertices[1] - result;
    auto PC = triangle3D->vertices[2] - result;
    auto alpha = (PB(0) * PC(1) - PB(1) * PC(0)) / areaDouble;
    if (alpha < 0 || alpha > 1) return false;
    auto PA = triangle3D->vertices[0] - result;
    auto beta = (PC(0) * PA(1) - PC(1) * PA(0)) / areaDouble;
    if (beta < 0 || beta > 1) return false;
    if (alpha + beta > 1) return false;
    return true;
}
