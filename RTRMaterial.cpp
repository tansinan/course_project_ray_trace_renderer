#include "RTRMaterial.h"
#include "RTRTexture.h"
#include <QColor>

RTRMaterial::RTRMaterial(const QColor& _diffuse, const QColor &_specular)
{
	diffuse = _diffuse;
	specular = _specular;
}

RTRMaterial::RTRMaterial()
{
}

void RTRMaterial::setColorProperty(const QString& propertyName, const RTRColor& color)
{
	colorProperties[propertyName] = color;
}

void RTRMaterial::setTextureProperty(const QString& propertyName, const QString& textureFilePath)
{
	textureProperties[propertyName] = new RTRTexture(textureFilePath);
}

RTRColor RTRMaterial::getColorAt(const QString& propertyName, double u, double v)
{
	if (textureProperties.find(propertyName) != textureProperties.end())
	{
		return textureProperties[propertyName]->valueAt(u, v);
	}
	else
	{
		return colorProperties[propertyName];
	}
}

int RTRMaterial::getPropertyType(const QString& propertyName)
{
	if (textureProperties.find(propertyName) != textureProperties.end()) return TYPE_TEXTURE;
	if (colorProperties.find(propertyName) != colorProperties.end()) return TYPE_COLOR;
	return TYPE_NONE;
}