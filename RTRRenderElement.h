#ifndef RTRRENDERELEMENT_H
#define RTRRENDERELEMENT_H

#include "RTRGeometry.h"
#include "RTRMaterial.h"
#include "RTRColor.h"

class RTRRenderElement
{
protected:
	int orthProjectDirection;
	RTRTriangle2D* orthProjectTriangle;
public:
	bool useSmoothShading;
	RTRRenderElement();
	RTRRenderElement(RTRTriangle3D* _triangle3D, RTRCamera* camera);
	RTRTriangle3D* triangle3D;
	RTRTriangle2D* triangle2D;
	RTRMaterial* material;
	RTRBoundingBox boundingBox;
	RTRVector3D vertexNormals[3];
	virtual const RTRBoundingBox& getBoundingBox() const;
	virtual bool intersect(const RTRRay& ray, RTRVector3D& result, RTRVector3D& normal, RTRColor& color) const;
	virtual bool intersect(const RTRRay& ray, RTRVector3D& result);
	QString objectName;
	//virtual const RTRVector3D& getCentroid() const;
};

#endif // RTRRENDERELEMENT_H
