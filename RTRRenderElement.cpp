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
		qMax(std::abs(triangle3D->vertices[1].z() - triangle3D->vertices[2].z()),
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

bool RTRRenderElement::intersect(const RTRRay& ray, RTRVector3D& result, RTRVector3D& normal, RTRColor& color) const
{
	result = RTRGeometry::intersect(triangle3D->plane, ray);
	RTRVector2D temp(2);
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
	if (useSmoothShading)
	{
		double a1, a2, a3;
		RTRVector3D line12 = triangle3D->vertices[1] - triangle3D->vertices[0];
		RTRVector3D line13 = triangle3D->vertices[2] - triangle3D->vertices[0];
		RTRVector3D line1p = result - triangle3D->vertices[0];

		if (orthProjectDirection == 0)
		{
			double det = line13.y() * line12.z() - line12.y() * line13.z();
			a2 = line1p.y() * line13.z() - line13.y() * line1p.z();
			a2 = abs(a2 / det);
			a3 = line1p.y() * line12.z() - line12.y() * line1p.z();
			a3 = abs(a3 / det);
			a1 = 1 - a2 - a3;
		}
		else if (orthProjectDirection == 1)
		{
			double det = line13.x() * line12.z() - line12.x() * line13.z();
			a2 = line1p.x() * line13.z() - line13.x() * line1p.z();
			a2 = abs(a2 / det);
			a3 = line1p.x() * line12.z() - line12.x() * line1p.z();
			a3 = abs(a3 / det);
			a1 = 1 - a2 - a3;
		}
		else if (orthProjectDirection == 2)
		{
			double det = line13.x() * line12.y() - line12.x() * line13.y();
			a2 = line1p.x() * line13.y() - line13.x() * line1p.y();
			a2 = abs(a2 / det);
			a3 = line1p.x() * line12.y() - line12.x() * line1p.y();
			a3 = abs(a3 / det);
			a1 = 1 - a2 - a3;
		}
		normal = vertexNormals[0] * a1 + vertexNormals[1] * a2 + vertexNormals[2] * a3;
	}
	else
	{
		normal = triangle3D->plane.normal;
	}
	if (objectName != "Plane")
	{
		if (material != NULL)
		{
			color = material->getColorAt("diffuse", 0, 0);
		}
		else color.r() = color.g() = color.b() = 1.0;
	}
	else
	{
		int x = qRound(result.x());
		int y = qRound(result.y());
		if ((x + y) % 2 == 0)
		{
			color.r() = color.g() = color.b() = 0.25;
		}
		else
		{
			color.r() = color.g() = color.b() = 0.75;
		}
	}
	return ret;
}
