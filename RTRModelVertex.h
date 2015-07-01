#ifndef RTRMODELVERTEX_H
#define RTRMODELVERTEX_H

#include "RTRVector3D.h"
#include "RTRVector2D.h"
#include <QSet>

class RTRFace;

class RTRModelVertex
{
public:
	RTRVector3D position;
	//RTRVector3D normal;
	//RTRVector2D uvPosition;
	QSet<RTRFace*> referedByFaces;
};

#endif