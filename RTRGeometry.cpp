#include "RTRGeometry.h"
#include <qmath.h>
#include <cmath>

RTRPlane::RTRPlane(RTRVector3D _onePoint, RTRVector3D _normal)
{
	onePoint = _onePoint;
	normal = _normal;
	normal.vectorNormalize();
}

RTRSegment::RTRSegment(RTRVector3D param1, RTRVector3D param2, int createMode)
{
	switch(createMode)
	{
	case CREATE_FROM_POINTS:
		beginningPoint = param1;
		endPoint = param2;
		direction = endPoint - beginningPoint;
		direction.vectorNormalize();
		break;
	case CREATE_FROM_POINT_AND_DIRECTION:
		beginningPoint = param1;
		direction = param2;
		direction.vectorNormalize();
		endPoint = beginningPoint + direction;
	}
}

RTRVector3D RTRSegment::pointAt(int coordinate, double val) const
{
	//TODO Not finished... I want to sleep!!
	double ratio = (val - beginningPoint(coordinate)) / (endPoint(coordinate) - beginningPoint(coordinate));
	RTRVector3D ret = beginningPoint + (endPoint - beginningPoint) * ratio;
	ret(coordinate) = val;
	return ret;
}

RTRSegment2D::RTRSegment2D(const RTRVector2D &param1, const RTRVector2D& param2, int createMode)
{
	Q_ASSERT(param1.isVector()&&param1.getSize()==2);
	Q_ASSERT(param2.isVector()&&param2.getSize()==2);
	switch(createMode)
	{
	case CREATE_FROM_POINTS:
		beginningPoint = param1;
		endPoint = param2;
		direction = endPoint - beginningPoint;
		direction.vectorNormalize();
		break;
	case CREATE_FROM_POINT_AND_DIRECTION:
		beginningPoint = param1;
		direction = param2;
		direction.vectorNormalize();
		endPoint = beginningPoint + direction;
		break;
	default:
		Q_ASSERT(false);
	}
	//y =slopeY * x + intersectY;

	slopeY = direction.y()/direction.x();
	intersectY = beginningPoint.y() - slopeY * beginningPoint.x();

	//x = slopeX * y + intersectX
	slopeX = direction.x()/direction.y();
	intersectX = beginningPoint.x() - slopeX * beginningPoint.y();
}

void RTRSegment2D::reset(const RTRVector2D &param1, const RTRVector2D& param2, int createMode)
{
	Q_ASSERT(param1.isVector()&&param1.getSize()==2);
	Q_ASSERT(param2.isVector()&&param2.getSize()==2);
	switch(createMode)
	{
	case CREATE_FROM_POINTS:
		beginningPoint = param1;
		endPoint = param2;
		direction = beginningPoint - endPoint;
		direction.vectorNormalize();
		break;
	case CREATE_FROM_POINT_AND_DIRECTION:
		beginningPoint = param1;
		direction = param2;
		direction.vectorNormalize();
		endPoint = beginningPoint + direction;
		break;
	default:
		Q_ASSERT(false);
	}
	//y =slopeY * x + intersectY;

	slopeY = direction.y()/direction.x();
	intersectY = beginningPoint.y() - slopeY * beginningPoint.x();

	//x = slopeX * y + intersectX
	slopeX = direction.x()/direction.y();
	intersectX = beginningPoint.x() - slopeX * beginningPoint.y();
}


int RTRSegment2D::pointSign(const RTRVector2D& point) const
{
	if((!std::isinf(slopeY)) && (!std::isnan(slopeY))/* && abs(slopeY )< 1.0e10*/)
	{
		return point.y() > point.x()*slopeY + intersectY ? 1 : -1;
	}
	else return point.x() > intersectX ? 1 : -1;
}

RTRTriangle3D::RTRTriangle3D(const RTRVector3D& vert1, const RTRVector3D& vert2, const RTRVector3D& vert3)
	:plane(vert1, RTRVector3D(vert3-vert1).crossProduct(vert2-vert1))
{
	vertices[0] = vert1;
	vertices[1] = vert2;
	vertices[2] = vert3;
}

RTRTriangle2D::RTRTriangle2D(const RTRVector2D& vert1, const RTRVector2D& vert2, const RTRVector2D& vert3)
	//:edges(vert1,vert2,RTRSegment2D::CREATE_FROM_POINTS)
	/*:edges{{vert1,vert2,RTRSegment2D::CREATE_FROM_POINTS},
{vert2,vert3,RTRSegment2D::CREATE_FROM_POINTS},
		  {vert3,vert1,RTRSegment2D::CREATE_FROM_POINTS}}*/
{
	edges[0] = RTRSegment2D(vert1,vert2,RTRSegment2D::CREATE_FROM_POINTS);
	edges[1] = RTRSegment2D(vert2,vert3,RTRSegment2D::CREATE_FROM_POINTS);
	edges[2] = RTRSegment2D(vert3,vert1,RTRSegment2D::CREATE_FROM_POINTS);
	vertices[0] = vert1;
	vertices[1] = vert2;
	vertices[2] = vert3;
	centroid = (vert1+vert2+vert3)/3;
	for(int i=0;i<3;i++)
	{
		centroidSignes[i] = edges[i].pointSign(centroid);
	}
}
//static bool pointInsideTriange(RTRTriangle3D, RTRVector);
//static bool intersect(RTRTriangle3D, RTRSegment);

//以下为几何解算模块的代码。

RTRVector3D RTRGeometry::intersect(const RTRPlane& plane, const RTRSegment& segment)
{
	double ratio = 0.0;
	double d1 = distance(segment.beginningPoint,plane);
	double d2 = distance(segment.endPoint, plane);
	ratio = d1 / (d1 - d2);
	return segment.beginningPoint + (segment.endPoint-segment.beginningPoint) * ratio;
}

RTRVector2D RTRGeometry::project(const RTRVector &point, const RTRCamera &camera)
{
	return camera.transformPoint(point);
}

RTRSegment RTRGeometry::project(const RTRSegment& segment, const RTRCamera& camera)
{
	return RTRSegment(camera.transformPoint(segment.beginningPoint),
					  camera.transformPoint(segment.endPoint),
					  RTRSegment::CREATE_FROM_POINTS);
}

RTRTriangle2D RTRGeometry::project(const RTRTriangle3D& triangle3D, const RTRCamera& camera)
{
	return RTRTriangle2D(camera.transformPoint(triangle3D.vertices[0]),
			camera.transformPoint(triangle3D.vertices[1]),
			camera.transformPoint(triangle3D.vertices[2])
			);
}

RTRMatrix RTRGeometry::rotationMatrix(double angleX, double angleY, double angleZ)
{
	RTRMatrix rotateX(3,3), rotateY(3,3), rotateZ(3,3);
	rotateX.fill(0);
	rotateX(0, 0) = 1;
	rotateX(2,2) = rotateX(1, 1) = qCos(qDegreesToRadians(angleX));
	rotateX(1,2) = qSin(qDegreesToRadians(angleX));
	rotateX(2,1) = -rotateX(1,2);

	rotateY.fill(0);
	rotateY(1, 1) = 1;
	rotateY(0,0) = rotateY(2, 2) = qCos(qDegreesToRadians(angleY));
	rotateY(2,0) = qSin(qDegreesToRadians(angleY));
	rotateY(0,2) = -rotateY(2,0);

	rotateZ.fill(0);
	rotateZ(2, 2) = 1;
	rotateZ(0, 0) = rotateZ(1, 1) = qCos(qDegreesToRadians(angleZ));
	rotateZ(0, 1) = qSin(qDegreesToRadians(angleZ));
	rotateZ(1, 0) = -rotateZ(0, 1);
	return rotateX*rotateY*rotateZ;
}

RTRMatrix RTRGeometry::inverseRotationMatrix(double angleX, double angleY, double angleZ)
{
	RTRMatrix rotateX(3,3), rotateY(3,3), rotateZ(3,3);
	rotateX.fill(0);
	rotateX(0, 0) = 1;
	rotateX(2,2) = rotateX(1, 1) = qCos(qDegreesToRadians(-angleX));
	rotateX(1,2) = qSin(qDegreesToRadians(-angleX));
	rotateX(2,1) = -rotateX(1,2);

	rotateY.fill(0);
	rotateY(1, 1) = 1;
	rotateY(0,0) = rotateY(2, 2) = qCos(qDegreesToRadians(-angleY));
	rotateY(2,0) = qSin(qDegreesToRadians(-angleY));
	rotateY(0,2) = -rotateY(2,0);

	rotateZ.fill(0);
	rotateZ(2, 2) = 1;
	rotateZ(0, 0) = rotateZ(1, 1) = qCos(qDegreesToRadians(-angleZ));
	rotateZ(0, 1) = qSin(qDegreesToRadians(-angleZ));
	rotateZ(1, 0) = -rotateZ(0, 1);
	return rotateZ*rotateY*rotateX;
}

RTRSegment RTRGeometry::invertProject(const RTRVector &point, const RTRCamera &camera)
{
	return camera.inverseProject(point);
}

double RTRGeometry::distance(RTRVector point, RTRPlane plane)
{
	return (point - plane.onePoint).dotProduct(plane.normal);
}


bool RTRGeometry::pointInsideTriangle(const RTRTriangle2D &triangle2D, const RTRVector2D& point)
{
	for(int i=0;i<3;i++)
	{
		if(triangle2D.edges[i].pointSign(point)!=triangle2D.centroidSignes[i])
			return false;
	}
	return true;
}

bool RTRGeometry::intersect(const RTRBoundingBox& boundingBox, RTRSegment &segment)
{
	for (int i = 0; i < 3; i++)
	{
		if (segment.beginningPoint(i) > segment.endPoint(i)) std::swap(segment.beginningPoint, segment.endPoint);
		if (segment.beginningPoint(i) > boundingBox.point2(i) || segment.endPoint(i) < boundingBox.point1(i)) return false;
		if (segment.beginningPoint(i) < boundingBox.point1(i)) segment.beginningPoint = segment.pointAt(i, boundingBox.point1(i));
		if (segment.endPoint(i) > boundingBox.point2(i)) segment.endPoint = segment.pointAt(i, boundingBox.point2(i));
	}
	return true;
}

RTRBoundingBox::RTRBoundingBox()
	:point1(3), point2(3)
{

}

bool RTRBoundingBox::contain(const RTRBoundingBox& other) const
{
	for (int i = 0; i < 3; i++)
	{
		if (point1(i)>other.point1(i) || point2(i) < other.point2(i))
		{
			return false;
		}
	}
	return true;
}