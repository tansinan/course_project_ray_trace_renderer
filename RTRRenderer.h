#ifndef RTRRENDERER_H
#define RTRRENDERER_H

#include "RTRModel.h"
#include "RTRCamera.h"
#include "RTRGeometry.h"
#include "RTRMaterial.h"
#include "RTRRenderElement.h"
#include "RTRKdTree.h"

class RTRRenderer
{
public:
	RTRModel* model;
	QVector<RTRRenderElement*> elements;
	RTRKdTree* elementsCache;
	RTRCamera* camera;
	//由于启用了光线追踪，深度缓冲已被停用，但我依然会永远铭记它在调试过程中为我做出的巨大贡献。
	//double* zBuffer;
	QImage* image;
public:
	RTRRenderer(QImage* _image);
	void render();
	void renderPixel(int x, int y, double z, const RTRColor &color);
	void renderLineByDDA(int x1, int y1, int x2, int y2, const QColor &color);
	void renderLineByDDA(RTRVector2D p1, RTRVector2D p2, const QColor &color);
	void renderTriangle(const RTRTriangle3D& triangle, const RTRMaterial& material);

	/**
	 * @brief renderRay实现了光线追踪的核心算法——它渲染一个光路的“颜色”。
	 * @param ray需要进行追踪的光线
	 * @param iterationCount迭代次数，为方便调用，默认参数值为0
	 * @param elementFrom 发出该逆向光线的渲染元素。该参数主要用于防止发生光线求交误判为和自己想交的情况。
	 * @return 获得的颜色数值。
	 */
	RTRColor renderRay(const RTRRay& ray, int iterationCount = 0, const RTRRenderElement* elementFrom = NULL);
};

#endif // RTRRENDERER_H
