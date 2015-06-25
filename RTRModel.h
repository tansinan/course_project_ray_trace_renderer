#ifndef RTRMODEL_H
#define RTRMODEL_H

#include <QVector>
#include <QString>
#include "RTRMatrix.h"
#include "RTRVector2D.h"
#include "RTRVector3D.h"

class QPointF;
class QImage;
class QColor;

class RTRFace
{
public:
	QString objectName;
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
public:
	bool loadModelFromObjFile(QString filePath);
	RTRModel();
	~RTRModel();
	void renderToImage(QImage* image);
	void drawLineByDDA(QImage* image, int x1, int y1, int x2, int y2, const QColor &color);
};

#endif // RTRMODEL_H
