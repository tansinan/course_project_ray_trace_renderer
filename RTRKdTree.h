#ifndef RTRKDTREE_H
#define RTRKDTREE_H

#include "RTRGeometry.h"
#include "RTRRenderElement.h"
#include <QVector>
#include <QSet>

class RTRKdTree
{
protected:
	class Node
	{
	public:
		static const int SPLIT_BY_X = 0;
		static const int SPLIT_BY_Y = 1;
		static const int SPLIT_BY_Z = 2;
		static const int SPLIT_NONE = 100;
		int splitMethod;
		RTRBoundingBox boundingBox;
		Node* small;
		Node* large;
		QVector<RTRRenderElement*> data;
	};

protected:
	int nodeCount = 0;
	Node* root;
	RTRKdTree();

protected:
	void search(Node* node, RTRRenderElement*& searchResult, RTRSegment& segment,
		RTRRay& originalRay, double& minZ, const RTRRenderElement* elementFrom) const;
	void construct(Node* parent, const QVector<RTRRenderElement*>& elementTable, int depth = 0, int maxDepth = 20);
	void cleanUp(Node* node);

public:
	static RTRKdTree* create(const QVector<RTRRenderElement*>& elementTable);
	void search(RTRRenderElement*& searchResult, const RTRRay& ray, const RTRRenderElement* elementFrom) const;


};

#endif // RTRKDTREE_H
