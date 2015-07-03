#include "RTRLightPoint.h"

RTRLightPoint::RTRLightPoint(RTRVector3D _position, RTRColor _color, double _multiplier)
{
	position = _position;
	color = _color;
	multiplier = _multiplier;
}

const RTRColor &RTRLightPoint::getColor()
{
	return color;
}

const RTRVector3D& RTRLightPoint::getPosition()
{
	return position;
}

double RTRLightPoint::getMultiplier()
{
	return multiplier;
}

RTRColor RTRLightPoint::colorAt(RTRVector3D point)
{
	double decay = multiplier/(point-position).vectorLength();
	return color * decay;
}

RTRVector3D RTRLightPoint::directionAt(RTRVector3D point)
{
	RTRVector3D ret = point-position;
	ret.vectorNormalize();
	return ret;
}
