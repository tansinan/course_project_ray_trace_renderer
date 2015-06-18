#ifndef RTRPOINTLIGHT_H
#define RTRPOINTLIGHT_H

#include "RTRLightAbstract.h"

/**
 * @brief RTRLightPoint类定义了一个点光源
 */
class RTRLightPoint
{
protected:
	/**
	 * @brief position表示点光源的位置
	 */
	RTRVector position;

	/**
	 * @brief color表示点光源的颜色
	 */
	RTRColor color;

	/**
	 * @brief multiplier表示光的强度，默认值为1.0,，表示经过1单位的衰减，光的强度变为color表示的颜色
	 */
	double multiplier;
public:
	RTRLightPoint(RTRVector _position, RTRColor _color, double _multiplier);
	const RTRColor& getColor();
	const RTRVector& getPosition();
	double getMultiplier();
	RTRColor colorAt(RTRVector point);
	RTRVector directionAt(RTRVector point);
};

#endif // RTRPOINTLIGHT_H
