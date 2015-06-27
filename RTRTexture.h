#ifndef RTRTEXTURE_H
#define RTRTEXTURE_H

#include <QImage>
#include "RTRColor.h"

class RTRTexture
{
public:
	QImage* image;
	RTRColor valueAt(double u, double v)
	{
		return RTRColor(0, 0, 0);
	}
	RTRTexture(const QString& filePath)
	{

	}
};

#endif