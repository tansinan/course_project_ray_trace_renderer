#include "RTRVector3D.h"
#include <QtCore>

RTRVector3D::RTRVector3D()
{
	x() = y() = z() = 0.0;
}

RTRVector3D::RTRVector3D(double _x, double _y, double _z)
{
	x() = _x;
	y() = _y;
	z() = _z;
}

RTRVector3D::RTRVector3D(const RTRMatrix<3, 1>& matrix)
{
	x() = matrix(0);
	y() = matrix(1);
	z() = matrix(2);
}

double& RTRVector3D::x()
{
	return elementAt(0);
}

double RTRVector3D::x() const
{
	return elementAt(0);
}

double& RTRVector3D::y()
{
	return elementAt(1);
}

double RTRVector3D::y() const
{
	return elementAt(1);
}

double& RTRVector3D::z()
{
	return elementAt(2);
}

double RTRVector3D::z() const
{
	return elementAt(2);
}

RTRVector3D RTRVector3D::crossProduct(const RTRVector3D& other) const
{
	return RTRVector3D(
		y()*other.z() - z()*other.y(), 
		z()*other.x() - x()*other.z(), 
		x()*other.y() - y()*other.x()
		);
}