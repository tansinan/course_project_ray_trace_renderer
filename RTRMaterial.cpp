#include "RTRMaterial.h"
#include <QColor>

RTRMaterial::RTRMaterial(const QColor& _diffuse, const QColor &_specular)
{
	diffuse = _diffuse;
	specular = _specular;
}

