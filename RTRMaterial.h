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
    RTRColor diffuse;

    /**
     * @brief specular表示物体的高光色。
     */
    RTRColor specular;

    /**
     * @brief reflectionRate表示物体的反射强度
     */
    double reflectionRate = 0.0;

    /**
     * @brief reflectionColor表示物体的反射颜色
     */
    RTRColor reflectionColor;

    /**
     * @brief refractionRate表示物体的折射强度
     */
    double refractionRate = 0.0;

    /**
     * @brief refractionRate表示物体的折射颜色
     */
    RTRColor refractionColor;

    /**
     * @brief refractionIndex表示物体的折射率
     * 注意要求具有同一个折射率的表面必须围成封闭网格，否则会有奇怪的事情发生……
     */
    double refractionIndex = 1.0;

    double emissionStrength = 0.0;
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
