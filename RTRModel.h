#ifndef RTRMODEL_H
#define RTRMODEL_H

#include <QVector>
#include <QString>
#include "RTRMatrix.h"
#include "RTRVector2D.h"
#include "RTRVector3D.h"
#include "RTRMaterial.h"

class QPointF;
class QImage;
class QColor;

class RTRFace
{
public:
	QString objectName;
	QString groupName;
	QString materialName;
	QList<int> vertices;
	QList<int> normals;
	QList<int> uvCoordinates;
	void addVertex(int vertex)
	{
		vertices.append(vertex);
	}
};

class RTRModel
{
public:
	QVector<RTRVector3D> vertexPositions;
	QVector<RTRVector3D> vertexNormals;
	QVector<RTRVector2D> vertexUVPositions;
	QVector<RTRFace> faces;
	QMap<QString, RTRMaterial*> materialLibrary;
	QString modelPath;
public:
	bool loadModelFromObjFile(const QString& filePath);
	bool loadMaterialLibraryFromMtlFile(const QString& filePath);
	RTRModel();
	~RTRModel();
};

#endif // RTRMODEL_H
