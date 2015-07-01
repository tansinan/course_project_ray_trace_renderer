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
		Q_ASSERT(u >= 0.0&&v >= 0.0&&u <= 1.0&&v <= 1.0);
		int x = qRound(u*image->width() - 1);
		int y = qRound(v*image->height() - 1);
		if (!image->isNull())
		{
			QRgb color = image->pixel(x, y);
			return RTRColor(qRed(color) / 255.0, qGreen(color) / 255.0, qBlue(color) / 255.0);
		}
		else return RTRColor(1.0, 1.0, 1.0);
	}
	RTRTexture(const QString& filePath)
	{
		image = new QImage(filePath);
	}
};

#endif