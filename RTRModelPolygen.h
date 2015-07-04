#ifndef RTRMODELPOLYGEN_H
#define RTRMODELPOLYGEN_H

#include <QVector>
#include "RTRVector3D.h"

class RTRModelVertex;

class RTRModelPolygen
{
public:
	QString objectName;
	QString groupName;
	QString materialName;
	bool smoothShading;
	QVector<RTRModelVertex*> vertices;
	QVector<RTRVector3D> normals;
	QVector<RTRVector2D> uvMaps;
};

#endif