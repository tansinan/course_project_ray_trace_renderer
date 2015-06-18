#include "RTRRenderer.h"
#include <QColor>
#include <QDebug>
#include <QImage>
#include <time.h>
#include "RTRGeometry.h"
#include "Light/RTRLightPoint.h"

template<class T>
int sgn(T val)
{
	return val>0?1:-1;
}

RTRRenderer::RTRRenderer(QImage *_image)
{
	model = NULL;
	camera = NULL;
	image = _image;
	//zBuffer = new double[image->width()*image->height()];
}

void RTRRenderer::render()
{
	int beginTime = clock();
	//初始化深度缓冲，由于即将切换至光线追踪算法，下面的代码将不再被需要
	/*for(int i=0;i<image->width();i++)
	{
		for(int j=0;j<image->height();j++)
		{
			zBuffer[i*image->height()+j] = 1e50;
		}
	}*/

	if(model == NULL || camera == NULL) return;

	//扫描所有需要渲染的多边形
	for(int i=0;i<model->faces.size();i++)
	{
		RTRFace& face = model->faces[i];
		//三角形的三个顶点
		RTRVector point1(3), point2(3), point3(3);

		//将多边形的第一个顶点作为三角形的第一个顶m点
		point1 = model->vertices[face.vertices[0]-1];

		//而三角形的另外两个顶点从多边形剩下的顶点中取两两相邻的选取。
		for(int j=1;j<face.vertices.size()-1;j++)
		{
			//构建新的三角形
			point2 = model->vertices[face.vertices[j]-1];
			point3 = model->vertices[face.vertices[j+1]-1];

			QColor myBlue;
			myBlue.setRedF(0.5);
			myBlue.setGreenF(0.5);
			myBlue.setBlueF(0.5);

			RTRMaterial material(myBlue,Qt::black);


			//TODO: 存在内存泄漏问题！
			RTRRenderElement* element = new RTRRenderElement(new RTRTriangle3D(point1, point2, point3), camera);
			element->objectName = face.objectName;
			//element->triangle3D = new RTRTriangle3D(point1, point2, point3);
			//element->triangle2D = new RTRTriangle2D(RTRGeometry::project(*element->triangle3D, *camera));
			element->material = &material;

			//将三角形添加到需要渲染的三角形的列表之中
			elements.append(element);
			//myBlue.setRedF(0.214);
			//myBlue.setGreenF(0.390);
			//myBlue.setBlueF(0.698);
			//renderTriangle(*triangle, material);
		}
	}
	elementsCache = RTRKdTree::create(elements);
	RTRVector2D pointOnScreen(2);
	for(int i=0;i<image->width();i++)
	{
		for(int j=0;j<image->height();j++)
		{
			if (i == 40 && j == 40)
			{
				i++;
				i--;
			}
			pointOnScreen.x() = i;
			pointOnScreen.y() = j;
			RTRRay ray = RTRGeometry::invertProject(pointOnScreen,*camera);
			RTRColor color = renderRay(ray);
			if(i==400&&j==300)
			{
				int i=0;
				i++;
			}
			renderPixel(i,j,0,color);
		}
	}
	int endTime = clock();
	qDebug() << ((float)endTime - beginTime)/CLOCKS_PER_SEC;
}

void RTRRenderer::renderLineByDDA(int x1, int y1, int x2, int y2, const QColor &color)
{
	//x1 += 300;
	//x2 += 300;
	//y1 += 300;
	//y2 += 300;
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

void RTRRenderer::renderLineByDDA(RTRVector p1, RTRVector p2, const QColor &color)
{
	renderLineByDDA(p1.x(), p1.y(), p2.x(), p2.y(), color);
}


RTRColor RTRRenderer::renderRay(const RTRRay& ray, int iterationCount)
{
	//RTRVector* vec3D = new RTRVector(3)[100];
	RTRLightPoint lightPoint(RTRVector(4.07625,1.00545,5.90386),RTRVector(1,1,1),7.5);
	QColor myBlue;
	myBlue.setRedF(0.5);
	myBlue.setGreenF(0.5);
	myBlue.setBlueF(0.5);
	RTRMaterial material(myBlue,Qt::black);

	QSet<RTRRenderElement*> possibleElements;

	elementsCache->search(possibleElements, ray);

	//TODO:

	RTRColor color(3);
	color.r() = color.g() = color.b() = 0.0;
	double minZ = -1e50;
	foreach(const RTRRenderElement* element, possibleElements)
	{
		bool flag = false;
		if (element->objectName == "Plane")
		{
			flag = true;
		}
		RTRTriangle3D& triangle = *element->triangle3D;
		RTRVector2D point = RTRGeometry::intersect(triangle.plane,ray);
		RTRTriangle2D& triangle2D = *element->triangle2D;
		bool slowChkFail = !RTRGeometry::pointInsideTriangle(triangle2D, camera->transformPoint(point));
		if (slowChkFail) continue;
		if(point.z() < minZ)
		{
			continue;
		}
		minZ = point.z();
		RTRVector v =lightPoint.directionAt(point);
		double decay = lightPoint.directionAt(point).dotProduct(triangle.plane.normal);
		int sym1 = sgn(decay);
		int sym2 = sgn((point-camera->cameraPosition).dotProduct(triangle.plane.normal));
		decay = decay>0?decay:-decay;
		if(sym1 == sym2)
		{
			RTRColor lightColor = lightPoint.colorAt(point);
			color.r() = material.diffuse.redF()*lightColor.r()*decay;
			color.g() = material.diffuse.greenF()*lightColor.g()*decay;
			color.b() = material.diffuse.blueF()*lightColor.b()*decay;
		}
		else
		{
			color.r() = 0.0;
			color.g() = 0.0;
			color.b() = 0.0;
		}
	}

	return color;
}

void RTRRenderer::renderPixel(int x, int y, double z, const RTRColor& color)
{
	image->setPixel(x,image->height()-y,color.toQtColor().rgb());
}