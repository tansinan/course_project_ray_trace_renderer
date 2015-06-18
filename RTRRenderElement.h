#ifndef RTRRENDERELEMENT_H
#define RTRRENDERELEMENT_H

#include "RTRGeometry.h"
#include "RTRMaterial.h"

class RTRRenderElement
{
public:
	RTRRenderElement();
	RTRRenderElement(RTRTriangle3D* _triangle3D, RTRCamera* camera);
	RTRTriangle3D* triangle3D;
	RTRTriangle2D* triangle2D;
	RTRMaterial* material;
	RTRBoundingBox boundingBox;
	virtual const RTRBoundingBox& getBoundingBox() const;
	virtual bool intersect(const RTRRay& ray, RTRVector3D& result) const;
	QString objectName;
	//virtual const RTRVector3D& getCentroid() const;
};

#endif // RTRRENDERELEMENT_H
