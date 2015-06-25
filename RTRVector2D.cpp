#include "RTRVector2D.h"
#include <QtCore>

RTRVector2D::RTRVector2D()
	:RTRMatrix(2)
{
	x() = y() = 0.0;
}

RTRVector2D::RTRVector2D(double _x, double _y)
	: RTRMatrix(2)
{
	x() = _x;
	y() = _y;
}

RTRVector2D::RTRVector2D(const RTRMatrix& matrix)
	:RTRMatrix(2)
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
