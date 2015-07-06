#ifndef RTRCAMERA_H
#define RTRCAMERA_H

#include "RTRMatrix.h"
#include "RTRVector3D.h"
#include "RTRVector2D.h"
#include "RTRGeometry.h"

class RTRSegment;
class RTRVector3D;
class RTRVector2D;

class RTRCamera
{
public:
	/**
	* @brief cameraPosition表示相机的坐标位置
	*/
	RTRVector3D cameraPosition;

	/**
	* @brief cameraAngle表示相机的旋转角度
	*/
	RTRVector3D cameraAngle;

	/**
	* @brief focalLength表示相机的焦距
	*/
	double imageDistance;

	/**
	* @brief offset是一个二维向量，表示图像的位置偏移,offset与透视效应无关。
	*/
	RTRVector2D offset;
	RTRCamera();
public:

	/**
	* @brief rotationMatrix是根据相机的旋转角度计算出来的旋转矩阵
	*/
	RTRMatrix33 rotationMatrix;

	/**
	* @brief inverseRotationMatrix是用于你变幻时使用的旋转矩阵
	*/
	RTRMatrix33 inverseRotationMatrix;

	/**
	* @brief evaluateRotationMatrix计算相机的旋转矩阵
	*/
	void evaluateRotationMatrix();

public:

	/**
	* @brief transformPoint将一个三维的点的坐标透视变换到平面上
	* @param point表示三维空间中的一个点
	* @return 点经过变换后在二维空间的位置
	*/
	RTRVector2D transformPoint(RTRVector3D point) const;

	/**
	* @brief invertProject将投影后的一个点转换回三维空间（但是对应一条直线）
	* @param point
	* @return
	*/
	RTRSegment inverseProject(RTRVector2D point) const;
};

#endif // RTRCAMERA_H
