#ifndef RTRCOLOR_H
#define RTRCOLOR_H

#include "RTRMatrix.h"

class QColor;

class RTRColor : public RTRMatrix
{
public:
	RTRColor();
	RTRColor(double _r, double _g, double _b);
	RTRColor(double _r, double _g, double _b, double _a);
	RTRColor(const RTRMatrix& matrix);
	double& r();
	double r()const;
	double& g();
	double g() const;
	double& b();
	double b() const;
	double& a();
	double a() const;
	QColor toQtColor() const;
};

#endif