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
}

bool RTRRenderElement::intersect(const RTRRay& ray, RTRVector3D& result) const
{

}