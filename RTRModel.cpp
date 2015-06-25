#include "RTRModel.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QImage>
#include <QColor>
#include "RTRGeometry.h"

RTRModel::RTRModel()
{

}

RTRModel::~RTRModel()
{

}

bool RTRModel::loadModelFromObjFile(QString filePath)
{
	QFile objModelFile(filePath);
	if(!objModelFile.open(QIODevice::ReadOnly))
	{
		return false;
	}
	QTextStream objModelStream(&objModelFile);
	QString currentObjectName = "";
	while(!objModelStream.atEnd())
	{
		QString line = objModelStream.readLine().trimmed();
		if (line.isEmpty()) continue;
		if(line[0]=='#')
		{
			qDebug() << "Ignoring Comment Line:" << line;
			continue;
		}
		QStringList param = line.split(' ', QString::SkipEmptyParts);
		QString command = param[0];
		command.toLower();
		param.removeFirst();
		if (command == "o")
		{
			currentObjectName = param[0];
			//qDebug() << "Ignoring Unsupported Feature : Object" << objectName;
		}
		else if (command == "s")
		{
			//TODO : Smooth Shading Support 
		}
		else if (command == "v")
		{
			//TODO : Different Type Of Vertex Support
			vertexPositions.append(RTRVector3D(param[0].toDouble(), param[1].toDouble(), param[2].toDouble()));
			//rrqDebug() << "Vertex Loaded" << x<<y<<z;
		}
		else if (command == "vn")
		{
			vertexNormals.append(RTRVector3D(param[0].toDouble(), param[1].toDouble(), param[2].toDouble()));
			vertexNormals.back().vectorNormalize();
		}
		else if (command == "vt")
		{
			vertexUVPositions.append(RTRVector2D(param[0].toDouble(), param[1].toDouble()));
		}
		else if (command == "f")
		{
			RTRFace face;
			for (int i = 0; i < param.size(); i++)
			{
				QStringList list = param[i].split("/");
				switch (list.size())
				{
				case 1:
					face.addVertex(list[0].toInt());
					break;
				case 2:
					face.addVertex(list[0].toInt());
					face.uvCoordinates.append(list[1].toInt());
					break;
				case 3:
					face.addVertex(list[0].toInt());
					if(!list[1].isEmpty()) face.uvCoordinates.append(list[1].toInt());
					face.normals.append(list[2].toInt());
					break;
				default:
					break;
				}
			}
			faces.append(face);
		}
		else
		{
			qDebug() << "Ignoring Unsupported Line : " << command << param;
		}
	}
	objModelFile.close();
	return true;
}

void RTRModel::renderToImage(QImage* image)
{
	/*for(int i=0;i<faces.size();i++)
	{
		RTRFace& face = faces[i];
		for(int j=0;j<face.vertices.size();j++)
		{
			RTRVector point1(3), point2(3);
			point1 = vertices[face.vertices[j]-1];
			if(j==face.vertices.size()-1)
			{
				point2 = vertices[face.vertices[0]-1];
			}
			else
			{
				point2 = vertices[face.vertices[j+1]-1];
			}

			drawLineByDDA(image, 200 + point1.x()*100 + point1.z()*50,
						  200 + point1.y()*100+ point1.z()*50,
						  200 + point2.x()*100 + point2.z()*50,
						  200 + point2.y()*100 + point2.z()*50,
						  QColor(255,0,0));
		}
	}*/
}

void RTRModel::drawLineByDDA(QImage* image, int x1, int y1, int x2, int y2, const QColor &color)
{
	//This improved DDA Alogrithm handles x1/x2 in reverse order,
	//or \Delta Y>\Delta X cases.
	//Note: about the 0.5 problem: using qRound Function
	int deltaX = qAbs(x1 - x2);
	int deltaY = qAbs(y1 - y2);
	if (deltaX > deltaY)
	{
		if(x1 > x2)
		{
			qSwap(x1,x2);
			qSwap(y1,y2);
		}
		double slope = ((double)(y2-y1))/(x2-x1);
		double y = y1;
		for(int x = x1; x <= x2; x++, y += slope)
		{
			if(x>=0 && x<image->width() && y>=0 && y<image->height())
				image->setPixel(x, qRound(y), color.rgb());
		}
	}
	else
	{
		if(y1 > y2)
		{
			qSwap(x1,x2);
			qSwap(y1,y2);
		}
		double slope = ((double)(x2-x1))/(y2-y1);
		double x = x1;
		for(int y = y1; y <= y2; y++, x += slope)
		{
			if(x>=0 && x<image->width() && y>=0 && y<image->height())
				image->setPixel(qRound(x), y, color.rgb());
		}
	}
}
