#ifndef RTRVECTOR3D_H
#define RTRVECTOR3D_H

#include "RTRMatrix.h"

class QColor;

class RTRVector3D : public RTRMatrix
{
public:
	RTRVector3D();
	RTRVector3D(double _x, double _y, double _z);
	RTRVector3D(const RTRMatrix& matrix);
	double& x();
	double x()const;
	double& y();
	double y() const;
	double& z();
	double z() const;
	RTRVector3D crossProduct(const RTRVector3D &other) const;
};

#endif