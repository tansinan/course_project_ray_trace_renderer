#ifndef RTRGEOMETRY_H
#define RTRGEOMETRY_H

#include "RTRMatrix.h"
#include "RTRCamera.h"

class RTRPlane
{
public:
	RTRVector normal;
	RTRVector onePoint;
	RTRPlane(RTRVector _onePoint, RTRVector _normal);
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
	RTRVector beginningPoint;

	/**
	 * @brief endPoint表示直线上的另一点或者线段的终止点
	 */
	RTRVector endPoint;

	/**
	 * @brief direction表示线段的方向，是单位向量
	 */
	RTRVector direction;
public:
	static const int CREATE_FROM_POINTS = 0;
	static const int CREATE_FROM_POINT_AND_DIRECTION = 1;
	RTRSegment(){}
	RTRSegment(RTRVector param1, RTRVector param2, int createMode);
	RTRVector3D pointAt(int coordinate, double val) const;
};


class RTRSegment2D
{
public:
	/**
	 * @brief beginningPoint表示直线上的一点或者线段的起始点
	 */
	RTRVector beginningPoint;

	/**
	 * @brief endPoint表示直线上的另一点或者线段的终止点
	 */
	RTRVector endPoint;

	/**
	 * @brief direction表示线段的方向，是单位向量
	 */
	RTRVector direction;

	double slopeY;
	double slopeX;
	double intersectY;
	double intersectX;
public:
	static const int CREATE_FROM_POINTS = 0;
	static const int CREATE_FROM_POINT_AND_DIRECTION = 1;
	RTRSegment2D(){}
	RTRSegment2D(const RTRVector& param1, const RTRVector &param2, int createMode);
	int pointSign(const RTRVector2D &point) const;
		void reset(const RTRVector &param1, const RTRVector& param2, int createMode);
};

/**
 * @brief RTRRay是RTR Segment的一个别名。不过用作RTRRay时，有时可能只有beginningPoint会被考虑到。
 */
typedef RTRSegment RTRRay;

class RTRTriangle3D
{
public:
	RTRVector vertices[3];
	RTRPlane plane;
public:
	RTRTriangle3D(const RTRVector &vert1, const RTRVector &vert2, const RTRVector &vert3);
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
	RTRTriangle2D(const RTRVector &vert1, const RTRVector &vert2, const RTRVector &vert3);
};

class RTRBoundingBox
{
public:
	RTRVector3D point1;
	RTRVector3D point2;
public:
	RTRBoundingBox();
	bool contain(const RTRBoundingBox& other) const;
};


class RTRCamera;
class RTRGeometry
{
public:
	static bool pointInsideTriange(RTRTriangle3D, RTRVector);
	static bool pointInsideTriangle(const RTRTriangle2D& triangle2D, const RTRVector2D &point);
	//static bool intersect(RTRTriangle3D, RTRSegment);
	static RTRVector intersect(const RTRPlane &plane, const RTRSegment &segment);
	static RTRVector project(const RTRVector& point, const RTRCamera& camera);
	static RTRSegment project(const RTRSegment& segment, const RTRCamera& camera);
	static RTRTriangle2D project(const RTRTriangle3D &triangle3D, const RTRCamera &camera);
	static RTRMatrix rotationMatrix(double angleX, double angleY, double angleZ);
	static RTRMatrix inverseRotationMatrix(double angleX, double angleY, double angleZ);

	//将点反向投影回空间中的一条直线（由于R3->R2的变换本身是不可逆的，因此没有附加信息结果不可能是一个点）
	static RTRSegment invertProject(const RTRVector& point, const RTRCamera& camera);


	/**
	 * @brief invertProjectTo将视平面上的一个点投影回一个空间三角形
	 * @param point 要反向投影的点
	 * @param camera 反向投影使用的摄像机
	 * @param isInside 该引用会被修改，显示反向投影点是否在三角形内部。
	 * @return 反向投影结果。注意如果isInside被设置为false，则该结果既不确定也没有意义。
	 */
	static RTRVector3D invertProjectTo(const RTRVector& point, const RTRCamera& camera, bool& isInside);

	static double distance(RTRVector point, RTRPlane plane);
};


#endif // RTRGEOMETRY_H
