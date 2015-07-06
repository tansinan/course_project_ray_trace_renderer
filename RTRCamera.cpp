#include "RTRCamera.h"
#include <qmath.h>
#include <QDebug>

RTRCamera::RTRCamera()
{

}

void RTRCamera::evaluateRotationMatrix()
{
	rotationMatrix = RTRGeometry::rotationMatrix(cameraAngle.x(),cameraAngle.y(),cameraAngle.z());
	inverseRotationMatrix
			= RTRGeometry::inverseRotationMatrix(cameraAngle.x(),cameraAngle.y(),cameraAngle.z());
}

RTRVector2D RTRCamera::transformPoint(RTRVector3D point) const
{
	RTRVector2D ret;
	RTRVector3D relativePosition = rotationMatrix*(point-cameraPosition);
	ret.x() = - relativePosition.x()/relativePosition.z()*imageDistance;
	ret.y() = -  relativePosition.y()/relativePosition.z()*imageDistance;
	ret = ret + offset;
	return ret;
}

RTRSegment RTRCamera::inverseProject(RTRVector2D point) const
{
	point = point - offset;
	RTRVector3D point2(point.x(),point.y(),-imageDistance);
	point2 = inverseRotationMatrix * point2;
	point2 = point2 + cameraPosition;
	return RTRSegment(cameraPosition, point2, RTRSegment::CREATE_FROM_POINTS);
}
