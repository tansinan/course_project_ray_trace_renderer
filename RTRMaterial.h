#ifndef RTRMATERIAL_H
#define RTRMATERIAL_H

#include <QColor>
#include <QMap>
#include "RTRColor.h"

class RTRTexture;

/**
 * @brief RTRMaterial类定义了物体的一个表面材质
 */
class RTRMaterial
{
public:
	static const int TYPE_NONE = 0;
	static const int TYPE_COLOR = 1;
	static const int TYPE_TEXTURE = 2;
	QMap<QString, RTRColor> colorProperties;
	QMap<QString, RTRTexture*> textureProperties;
	/**
	 * @brief diffuse表示物体的漫反射颜色。
	 */
	QColor diffuse;

	/**
	 * @brief specular表示物体的高光色。
	 */
	QColor specular;
public:
	RTRMaterial(const QColor &_diffuse, const QColor& _specular);
	RTRMaterial();
	void setColorProperty(const QString& propertyName, const RTRColor& color);
	void setTextureProperty(const QString& propertyName, const QString& textureFilePath);
	int getPropertyType(const QString& propertyName);
	RTRColor getColorAt(const QString& propertyName, double u, double v);
signals:

public slots:
};

#endif // RTRMATERIAL_H
