#include "RTRLightPoint.h"

RTRLightPoint::RTRLightPoint(RTRVector _position, RTRColor _color, double _multiplier)
{
	position = _position;
	color = _color;
	multiplier = _multiplier;
}

const RTRColor &RTRLightPoint::getColor()
{
	return color;
}

const RTRVector& RTRLightPoint::getPosition()
{
	return position;
}

double RTRLightPoint::getMultiplier()
{
	return multiplier;
}

RTRColor RTRLightPoint::colorAt(RTRVector point)
{
	double decay = multiplier/(point-position).vectorLength();
	return color * decay;
}

RTRVector RTRLightPoint::directionAt(RTRVector point)
{
	RTRVector ret = point-position;
	ret.vectorNormalize();
	return ret;
}
