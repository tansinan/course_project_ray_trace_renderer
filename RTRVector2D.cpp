#include "RTRVector2D.h"
#include <QtCore>

RTRVector2D::RTRVector2D()
{
	x() = y() = 0.0;
}

RTRVector2D::RTRVector2D(double _x, double _y)
{
	x() = _x;
	y() = _y;
}

RTRVector2D::RTRVector2D(const RTRMatrix& matrix)
{
	Q_ASSERT(matrix.isVector() && matrix.getSize() == 2);
	x() = matrix(0);
	y() = matrix(1);
}

double& RTRVector2D::x()
{
	return elementAt(0);
}

double RTRVector2D::x() const
{
	return elementAt(0);
}

double& RTRVector2D::y()
{
	return elementAt(1);
}

double RTRVector2D::y() const
{
	return elementAt(1);
}

double RTRVector2D::crossProduct(const RTRVector2D& other)
{
	return x()*other.y() - y()*other.x();
}