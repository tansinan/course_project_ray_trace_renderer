#ifndef RTRGEOMETRY_H
#define RTRGEOMETRY_H

#include "RTRMatrix.h"
#include "RTRCamera.h"
#include "RTRVector3D.h"
#include "RTRVector2D.h"

template<class T>
int sgn(T val)
{
	return val>0 ? 1 : -1;
}

class RTRPlane
{
public:
	RTRVector3D normal;
	RTRVector3D onePoint;
	RTRPlane(RTRVector3D _onePoint, RTRVector3D _normal);
};

/**
* @brief RTRSegment类定义了二维或三维空间中的一条线段或直线
*/
class RTRSegment
{
public:
	/**
	* @brief beginningPoint表示直线上的一点或者线段的起始点
	*/
	RTRVector3D beginningPoint;

	/**
	* @brief endPoint表示直线上的另一点或者线段的终止点
	*/
	RTRVector3D endPoint;

	/**
	* @brief direction表示线段的方向，是单位向量
	*/
	RTRVector3D direction;
public:
	static const int CREATE_FROM_POINTS = 0;
	static const int CREATE_FROM_POINT_AND_DIRECTION = 1;
	RTRSegment(){}
	RTRSegment(RTRVector3D param1, RTRVector3D param2, int createMode);
	RTRVector3D pointAt(int coordinate, double val) const;
};


class RTRSegment2D
{
public:
	/**
	* @brief beginningPoint表示直线上的一点或者线段的起始点
	*/
	RTRVector2D beginningPoint;

	/**
	* @brief endPoint表示直线上的另一点或者线段的终止点
	*/
	RTRVector2D endPoint;

	/**
	* @brief direction表示线段的方向，是单位向量
	*/
	RTRVector2D direction;

	double slopeY;
	double slopeX;
	double intersectY;
	double intersectX;
public:
	static const int CREATE_FROM_POINTS = 0;
	static const int CREATE_FROM_POINT_AND_DIRECTION = 1;
	RTRSegment2D(){}
	RTRSegment2D(const RTRVector2D& param1, const RTRVector2D &param2, int createMode);
	int pointSign(const RTRVector2D &point) const;
	void reset(const RTRVector2D &param1, const RTRVector2D& param2, int createMode);
};

/**
* @brief RTRRay是RTR Segment的一个别名。不过用作RTRRay时，有时可能只有beginningPoint会被考虑到。
*/
typedef RTRSegment RTRRay;

class RTRTriangle3D
{
public:
	RTRVector3D vertices[3];
	RTRPlane plane;
public:
	RTRTriangle3D(const RTRVector3D &vert1, const RTRVector3D &vert2, const RTRVector3D &vert3);
};

class RTRTriangle2D
{
public:
	RTRVector2D vertices[3];
	RTRSegment2D edges[3];
	RTRVector2D centroid;
	int centroidSignes[3];
public:
	RTRTriangle2D();
	RTRTriangle2D(const RTRVector2D &vert1, const RTRVector2D &vert2, const RTRVector2D &vert3);
};

class RTRBoundingBox
{
public:
	RTRVector3D point1;
	RTRVector3D point2;
public:
	RTRBoundingBox();
	bool contain(const RTRBoundingBox& other) const;
	bool contain(const RTRVector3D& point) const;
};


class RTRCamera;
class RTRGeometry
{
public:
	//static bool pointInsideTriange(RTRTriangle3D, RTRVector);
	static bool pointInsideTriangle(const RTRTriangle2D& triangle2D, const RTRVector2D &point);
	//static bool intersect(RTRTriangle3D, RTRSegment);
	static RTRVector3D intersect(const RTRPlane &plane, const RTRSegment &segment);
	static bool intersect(const RTRBoundingBox& boundingBox, RTRSegment &segment);
	static RTRVector2D project(const RTRVector3D& point, const RTRCamera& camera);
	static RTRTriangle2D project(const RTRTriangle3D &triangle3D, const RTRCamera &camera);
	static RTRMatrix33 rotationMatrix(double angleX, double angleY, double angleZ);
	static RTRMatrix33 inverseRotationMatrix(double angleX, double angleY, double angleZ);

	//将点反向投影回空间中的一条直线（由于R3->R2的变换本身是不可逆的，因此没有附加信息结果不可能是一个点）

	static RTRSegment invertProject(const RTRVector2D& point, const RTRCamera& camera);


	/**
	* @brief invertProjectTo将视平面上的一个点投影回一个空间三角形
	* @param point 要反向投影的点
	* @param camera 反向投影使用的摄像机
	* @param isInside 该引用会被修改，显示反向投影点是否在三角形内部。
	* @return 反向投影结果。注意如果isInside被设置为false，则该结果既不确定也没有意义。
	*/
	static RTRVector3D invertProjectTo(const RTRVector2D& point, const RTRCamera& camera, bool& isInside);

	static double distance(RTRVector3D point, RTRPlane plane);
};


#endif // RTRGEOMETRY_H
