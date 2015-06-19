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
	triangle3D = _triangle3D;
	triangle2D = new RTRTriangle2D(RTRGeometry::project(*triangle3D, *camera));
	for (int i = 0; i < 3; i++)
	{
		boundingBox.point1(i) = 1e50;
		boundingBox.point2(i) = -1e50;
	}
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			boundingBox.point1(j) = std::min(triangle3D->vertices[i](j), boundingBox.point1(j));
			boundingBox.point2(j) = std::max(triangle3D->vertices[i](j), boundingBox.point2(j));
		}
	}

	double rangeX = qMax(abs(triangle3D->vertices[0].x() - triangle3D->vertices[1].x()),
		qMax(abs(triangle3D->vertices[1].x() - triangle3D->vertices[2].x()),
		abs(triangle3D->vertices[0].x() - triangle3D->vertices[2].x())));

	double rangeY = qMax(abs(triangle3D->vertices[0].y() - triangle3D->vertices[1].y()),
		qMax(abs(triangle3D->vertices[1].y() - triangle3D->vertices[2].y()),
		abs(triangle3D->vertices[0].y() - triangle3D->vertices[2].y())));

	double rangeZ = qMax(abs(triangle3D->vertices[0].z() - triangle3D->vertices[1].z()),
		qMax(abs(triangle3D->vertices[1].z() - triangle3D->vertices[2].z()),
		abs(triangle3D->vertices[0].z() - triangle3D->vertices[2].z())));

	RTRVector2D vert1(2), vert2(2), vert3(2);

	if (rangeY > rangeX&&rangeZ > rangeX)
	{
		orthProjectDirection = 0;
		vert1.x() = triangle3D->vertices[0].y();
		vert1.y() = triangle3D->vertices[0].z();
		vert2.x() = triangle3D->vertices[1].y();
		vert2.y() = triangle3D->vertices[1].z();
		vert3.x() = triangle3D->vertices[2].y();
		vert3.y() = triangle3D->vertices[2].z();
	}
	else if (rangeX > rangeZ&&rangeY > rangeZ)
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
}

bool RTRRenderElement::intersect(const RTRRay& ray, RTRVector3D& result, RTRVector3D& normal) const
{
	result = RTRGeometry::intersect(triangle3D->plane, ray);
	normal = triangle3D->plane.normal;
	RTRVector2D temp(2);
	if (orthProjectDirection == 0)
	{
		temp.x() = result.y();
		temp.y() = result.z();
		return RTRGeometry::pointInsideTriangle(*orthProjectTriangle,temp);
	}
	if (orthProjectDirection == 1)
	{
		temp.x() = result.x();
		temp.y() = result.z();
		return RTRGeometry::pointInsideTriangle(*orthProjectTriangle, temp);
	}
	if (orthProjectDirection == 2)
	{
		temp.x() = result.x();
		temp.y() = result.y();
		return RTRGeometry::pointInsideTriangle(*orthProjectTriangle, temp);
	}
	return true;
}