#ifndef RTRLIGHTSOURCE_H
#define RTRLIGHTSOURCE_H

#include "RTRGeometry.h"
#include <QColor>

class RTRLightAbstract
{
public:
	virtual RTRColor colorAt(RTRVector point) = 0;
};

#endif // RTRLIGHTSOURCE_H
