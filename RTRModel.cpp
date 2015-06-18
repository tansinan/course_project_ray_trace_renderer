#include "RTRModel.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QPointF>
#include <QVector3D>
#include <QImage>
#include <QColor>

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
		QString str = objModelStream.readLine();
		if(str[0]=='#')
		{
			qDebug() << "Ignoring Comment Line:" << str;
		}
		else if(str.startsWith("mtllib "))
		{
			qDebug() << "Ignoring Unsupported Feature : Matrial Library" << str;
		}
		else if(str.startsWith("o "))
		{
			str[0] = ' ';
			currentObjectName = str.trimmed();
			//qDebug() << "Ignoring Unsupported Feature : Object" << objectName;
		}
		else if(str.startsWith("usemtl "))
		{
			qDebug() << "Ignoring Unsupported Feature : Use Material" << str;
		}
		else if(str.startsWith("s "))
		{
			qDebug() << "Ignoring Unsupported Feature : S(Shading?)" << str;
		}
		else if(str.startsWith("v "))
		{
			double x,y,z;
			QTextStream parser(&str);
			parser.seek(2);
			parser >> x >> y >> z;
			vertices.append(RTRVector(x,y,z));
			//rrqDebug() << "Vertex Loaded" << x<<y<<z;
		}
		else if(str.startsWith("f "))
		{
			RTRFace face;
			face.objectName = currentObjectName;
			int point;
			QTextStream parser(&str);
			parser.seek(2);
			while(!parser.atEnd())
			{
				parser >> point;
				face.addVertex(point);
			}
			faces.append(face);
		}
		else
		{
			qDebug() << "Ignoring Unsupported Line : " << str;
		}
	}
	objModelFile.close();
	return true;
}

void RTRModel::renderToImage(QImage* image)
{
	for(int i=0;i<faces.size();i++)
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
	}
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
