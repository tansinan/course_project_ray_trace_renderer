#ifndef RTRVECTOR2D_H
#define RTRVECTOR2D_H

#include "RTRMatrix.h"

class QColor;

class RTRVector2D : public RTRMatrix<2,1>
{
public:
	RTRVector2D();
	RTRVector2D(double _x, double _y);
	RTRVector2D(const RTRMatrix<2,1>& matrix);
	double& x();
	double x()const;
	double& y();
	double y() const;
	double crossProduct(const RTRVector2D& other);
};

#endif