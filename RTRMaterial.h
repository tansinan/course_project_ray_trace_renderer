#ifndef RTRMATERIAL_H
#define RTRMATERIAL_H

#include <QColor>

/**
 * @brief RTRMaterial类定义了物体的一个表面材质
 */
class RTRMaterial
{
public:
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
signals:

public slots:
};

#endif // RTRMATERIAL_H
