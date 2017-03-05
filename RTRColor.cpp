#include "RTRColor.h"
#include <QtCore>
#include <QColor>

RTRColor::RTRColor()
{
	r() = g() = b() = 0.0;
	a() = 1.0;
}

RTRColor::RTRColor(double _r, double _g, double _b)
{
	r() = _r;
	g() = _g;
	b() = _b;
	a() = 1.0;
}

RTRColor::RTRColor(const RTRMatrix& matrix)
{
	r() = matrix(0);
	g() = matrix(1);
	b() = matrix(2);
	a() = matrix(3);
}

RTRColor::RTRColor(double _r, double _g, double _b, double _a)
{
	r() = _r;
	g() = _g;
	b() = _b;
	a() = _a;
}

double& RTRColor::r()
{
	return elementAt(0);
}

double RTRColor::r() const
{
	return elementAt(0);
}

double& RTRColor::g()
{
	return elementAt(1);
}

double RTRColor::g() const
{
	return elementAt(1);
}

double& RTRColor::b()
{
	return elementAt(2);
}

double RTRColor::b() const
{
	return elementAt(2);
}

QColor RTRColor::toQtColor() const
{
	QColor ret;
	if (r()<0) ret.setRed(0);
	else if (r()>1) ret.setRedF(1.0);
	else ret.setRedF(r());

	if (g()<0) ret.setGreen(0);
	else if (g()>1) ret.setGreenF(1.0);
	else ret.setGreenF(g());

	if (b()<0) ret.setBlue(0);
	else if (b()>1) ret.setBlueF(1.0);
	else ret.setBlueF(b());

	if (a()<0) ret.setAlphaF(0);
	else if (a()>1.0) ret.setAlphaF(1.0);
	else ret.setAlphaF(a());

	return ret;
}

double& RTRColor::a()
{
	return elementAt(3);
}

double RTRColor::a() const
{
	return elementAt(3);
}

RTRColor RTRColor::operator *(const RTRColor& other)
{
	return RTRColor(r()*other.r(), g()*other.g(), b()*other.b(), a()*other.a());
}

RTRColor RTRColor::operator +(const RTRColor& other)
{
    return RTRColor(r()+other.r(), g()+other.g(), b()+other.b(), a()+other.a());
}

RTRColor RTRColor::operator*(double ratio)
{
	return RTRColor(r()*ratio, g()*ratio, b()*ratio, a()*ratio);
}
