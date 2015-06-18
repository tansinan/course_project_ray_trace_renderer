#ifndef RTRMODEL_H
#define RTRMODEL_H

#include <QVector>
#include <QString>
#include "RTRMatrix.h"

class QPointF;
class QImage;
class QColor;

class RTRFace
{
public:
	QString objectName;
	QList<int> vertices;
	void addVertex(int vertex)
	{
		vertices.append(vertex);
	}
};

class RTRModel
{
public:
	QVector<RTRVector> vertices;
	QVector<RTRFace> faces;
public:
	bool loadModelFromObjFile(QString filePath);
	RTRModel();
	~RTRModel();
	void renderToImage(QImage* image);
	void drawLineByDDA(QImage* image, int x1, int y1, int x2, int y2, const QColor &color);
};

#endif // RTRMODEL_H
