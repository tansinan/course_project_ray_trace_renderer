#include "RTRCamera.h"
#include <qmath.h>
#include <QDebug>

RTRCamera::RTRCamera()
	:cameraAngle(3)
	,cameraPosition(3)
	,rotationMatrix(3,3)
	,offset(2)
{

}

void RTRCamera::evaluateRotationMatrix()
{
	rotationMatrix = RTRGeometry::rotationMatrix(cameraAngle.x(),cameraAngle.y(),cameraAngle.z());
	inverseRotationMatrix
			= RTRGeometry::inverseRotationMatrix(cameraAngle.x(),cameraAngle.y(),cameraAngle.z());
	/*RTRMatrix rotateX(3,3), rotateY(3,3), rotateZ(3,3);
	rotateX.fill(0);
	rotateX(0, 0) = 1;
	rotateX(2,2) = rotateX(1, 1) = qCos(qDegreesToRadians(cameraAngle.x()));
	rotateX(1,2) = qSin(qDegreesToRadians(cameraAngle.x()));
	rotateX(2,1) = -rotateX(1,2);

	rotateY.fill(0);
	rotateY(1, 1) = 1;
	rotateY(0,0) = rotateY(2, 2) = qCos(qDegreesToRadians(cameraAngle.y()));
	rotateY(2,0) = qSin(qDegreesToRadians(cameraAngle.y()));
	rotateY(0,2) = -rotateY(2,0);

	rotateZ.fill(0);
	rotateZ(2, 2) = 1;
	rotateZ(0, 0) = rotateZ(1, 1) = qCos(qDegreesToRadians(cameraAngle.z()));
	rotateZ(0, 1) = qSin(qDegreesToRadians(cameraAngle.z()));
	rotateZ(1, 0) = -rotateZ(0, 1);
	rotationMatrix = rotateX*rotateY*rotateZ;*/
}

RTRVector RTRCamera::transformPoint(RTRVector point) const
{
	RTRVector ret(2);
	RTRVector relativePosition = rotationMatrix*(point-cameraPosition);
	//rotationMatrix.printDebugInfo();
	//qDebug() << ret.x() << ret.y() << ret.z();
	ret.x() = - relativePosition.x()/relativePosition.z()*focalLength;
	ret.y() = -  relativePosition.y()/relativePosition.z()*focalLength;
	ret = ret + offset;
	return ret;
}

RTRSegment RTRCamera::inverseProject(RTRVector point) const
{
	point = point - offset;
	//RTRVector point1(point.x()/focalLength,point.y()/focalLength,-1);
	RTRVector point2(point.x(),point.y(),-focalLength);
	//point1 = inverseRotationMatrix * point1;
	point2 = inverseRotationMatrix * point2;
	point2 = point2 + cameraPosition;
	return RTRSegment(cameraPosition, point2, RTRSegment::CREATE_FROM_POINTS);
}
