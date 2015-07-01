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
	Node* root;
	RTRKdTree();

protected:
	void search(Node* node, QSet<RTRRenderElement*>& searchResult, RTRSegment& segment) const;
	static void construct(Node* parent, const QVector<RTRRenderElement*>& elementTable, int depth = 0, int maxDepth = 20);

public:
	static RTRKdTree* create(const QVector<RTRRenderElement*>& elementTable);
	void search(QSet<RTRRenderElement*>& searchResult, const RTRSegment& segment) const;


};

#endif // RTRKDTREE_H
