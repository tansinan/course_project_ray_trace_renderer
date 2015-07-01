#ifndef RTRMODEL_H
#define RTRMODEL_H

#include <QVector>
#include <QString>
#include "RTRMatrix.h"
#include "RTRVector2D.h"
#include "RTRVector3D.h"
#include "RTRMaterial.h"
#include "RTRModelVertex.h"
#include "RTRModelPolygen.h"

class QPointF;
class QImage;
class QColor;

class RTRModel
{
public:
	QSet<RTRModelVertex*> vertices;
	QSet<RTRModelPolygen*> polygens;
public:
	QMap<QString, RTRMaterial*> materialLibrary;
	QString modelPath;
public:
	bool loadModelFromObjFile(const QString& filePath);
	bool loadMaterialLibraryFromMtlFile(const QString& filePath);
	bool saveModelToObjFile(const QString& filePath);
	RTRModelVertex* addVertex(const RTRVector3D& coordinate);
	RTRModelVertex* addVertex(double _x, double _y, double _z);
	RTRModelPolygen* addPolygen(const QVector<RTRModelVertex*>& vertices);
	RTRModelPolygen* addPolygen(const QVector<RTRModelVertex*>& vertices, const QVector<RTRVector2D>& uvMaps);
	RTRModel();
	~RTRModel();
};

#endif // RTRMODEL_H
