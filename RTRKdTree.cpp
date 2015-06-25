#include "RTRKdTree.h"

RTRKdTree::RTRKdTree()
{

}


void RTRKdTree::construct(Node* parent, const QVector<RTRRenderElement*>& elementTable, int depth, int maxDepth)
{
	//Thrink the size first
	RTRBoundingBox boundingBox;
	for (int i = 0; i<3; i++)
	{
		boundingBox.point1(i) = 1e100;
		boundingBox.point2(i) = -1e100;
	}
	for (int i = 0; i<elementTable.size(); i++)
	{
		for (int j = 0; j<3; j++)
		{
			if (elementTable[i]->getBoundingBox().point1(j)<boundingBox.point1(j))
				boundingBox.point1(j) = elementTable[i]->getBoundingBox().point1(j);
			if (elementTable[i]->getBoundingBox().point2(j)>boundingBox.point2(j))
				boundingBox.point2(j) = elementTable[i]->getBoundingBox().point2(j);
		}
	}

	for (int i = 0; i < 3; i++)
	{
		if (boundingBox.point1(i) > parent->boundingBox.point1(i))
		{
			parent->boundingBox.point1(i) = boundingBox.point1(i);
		}
		if (boundingBox.point2(i) < parent->boundingBox.point2(i))
		{
			parent->boundingBox.point2(i) = boundingBox.point2(i);
		}
	}

	if (elementTable.size() <= 2 || depth >= maxDepth)
	{
		parent->small = parent->large = NULL;
		parent->splitMethod = Node::SPLIT_NONE;
		parent->data = elementTable;
		return;
	}
	//int splitMethod = parent->splitMethod;

	QVector<RTRRenderElement*> bestNewTableSmall;
	QVector<RTRRenderElement*> bestNewTableLarge;
	double bestMid = 0.0;
	int bestSplitMethod = 0;
	int minDuplicate = INT_MAX;

	int splitMethod = -1;

	/*for (int i = 0; i < elementTable.size(); i++)
	{
	if (elementTable[i]->getBoundingBox().contain(parent->boundingBox))
	{
	parent->data.append(elementTable[i]);
	}
	}*/

	for (int method = 0; method < 3; method++)
	{
		int duplicate = 0;
		QVector<RTRRenderElement*> newTableSmall;
		QVector<RTRRenderElement*> newTableLarge;
		QVector<double> divisionList;
		for (int i = 0; i<elementTable.size(); i++)
		{
			divisionList.append(
				(elementTable[i]->getBoundingBox().point1(method) + elementTable[i]->getBoundingBox().point2(method)) / 2);
			//divisionList.append(elementTable[i]->getBoundingBox().point1(method));
			//divisionList.append(elementTable[i]->getBoundingBox().point2(method));
		}
		qSort(divisionList);
		double mid = divisionList.size() % 2 == 0 ?
			(divisionList[divisionList.size() / 2] + divisionList[divisionList.size() / 2 - 1]) / 2 :
			divisionList[divisionList.size() / 2];
		for (int i = 0; i < elementTable.size(); i++)
		{
			double valSmall = elementTable[i]->getBoundingBox().point1(method);
			double valLarge = elementTable[i]->getBoundingBox().point2(method);
			/*if (elementTable[i]->getBoundingBox().contain(parent->boundingBox))
			{
			//parent->data.append(elementTable[i]);
			continue;
			}*/
			if (valSmall < mid)
			{
				newTableSmall.append(elementTable[i]);
			}
			if (valLarge >= mid)
			{
				newTableLarge.append(elementTable[i]);
			}
			if (valSmall < mid && valLarge >= mid)
			{
				duplicate++;
			}
		}
		if (duplicate < minDuplicate)
		{
			minDuplicate = duplicate;
			bestNewTableLarge = newTableLarge;
			bestNewTableSmall = newTableSmall;
			splitMethod = parent->splitMethod = method;
			bestMid = mid;
		}
	}

	if (minDuplicate / (double)elementTable.size() > 0.6)
	{
		depth = maxDepth + 1;
	}
	Node* nodeSmall = new Node();
	Node* nodeLarge = new Node();
	parent->small = nodeSmall;
	parent->large = nodeLarge;
	nodeSmall->boundingBox = nodeLarge->boundingBox = parent->boundingBox;
	nodeSmall->boundingBox.point2(splitMethod) = bestMid;
	nodeLarge->boundingBox.point1(splitMethod) = bestMid;
	//nodeSmall->splitMethod = nodeLarge->splitMethod = (parent->splitMethod+1) % 3;
	construct(nodeSmall, bestNewTableSmall, depth + 1, maxDepth);
	construct(nodeLarge, bestNewTableLarge, depth + 1, maxDepth);
}

RTRKdTree* RTRKdTree::create(const QVector<RTRRenderElement*>& elementTable)
{
	int maxDepth = log2(elementTable.size()) + 3;
	RTRKdTree* ret = new RTRKdTree();
	if (elementTable.size() == 0) return NULL;
	RTRBoundingBox boundingBox;
	for (int i = 0; i<3; i++)
	{
		boundingBox.point1(i) = 1e100;
		boundingBox.point2(i) = -1e100;
	}
	for (int i = 0; i<elementTable.size(); i++)
	{
		for (int j = 0; j<3; j++)
		{
			if (elementTable[i]->getBoundingBox().point1(j)<boundingBox.point1(j))
				boundingBox.point1(j) = elementTable[i]->getBoundingBox().point1(j);
			if (elementTable[i]->getBoundingBox().point2(j)>boundingBox.point2(j))
				boundingBox.point2(j) = elementTable[i]->getBoundingBox().point2(j);
		}
	}
	for (int i = 0; i<3; i++)
	{
		boundingBox.point1(i) -= 0.001;
		boundingBox.point2(i) += 0.001;
	}
	ret->root = new Node();
	ret->root->boundingBox = boundingBox;
	ret->root->splitMethod = Node::SPLIT_BY_X;
	construct(ret->root, elementTable, 0, maxDepth);
	return ret;
}

void RTRKdTree::search(QSet<RTRRenderElement*>& searchResult, const RTRSegment& segment) const
{
	RTRVector newPoint1 = segment.pointAt(root->splitMethod, root->boundingBox.point1(root->splitMethod));
	RTRVector newPoint2 = segment.pointAt(root->splitMethod, root->boundingBox.point2(root->splitMethod));
	RTRSegment segment2(newPoint1, newPoint2, RTRSegment::CREATE_FROM_POINTS);
	search(root, searchResult, segment2);
}

void RTRKdTree::search(Node* node, QSet<RTRRenderElement*>& searchResult, RTRSegment& segment) const
{
	int splitMethod = node->splitMethod;
	if (!RTRGeometry::intersect(node->boundingBox, segment)) return;
	for (int i = 0; i < node->data.size(); i++)
	{
		searchResult.insert(node->data[i]);
	}
	if (node->large == NULL) return;
	if (segment.beginningPoint(splitMethod) > segment.endPoint(splitMethod))
	{
		std::swap(segment.beginningPoint, segment.endPoint);
	}
	//segment.beginningPoint(splitMethod)
	if (segment.endPoint(splitMethod) < node->large->boundingBox.point1(splitMethod) - (1e-5))
	{
		search(node->small, searchResult, segment);
		return;
	}
	if (segment.beginningPoint(splitMethod) > node->small->boundingBox.point2(splitMethod) + (1e-5))
	{
		search(node->large, searchResult, segment);
		return;
	}
	RTRSegment segmentSmallTemp, segmentLargeTemp;
	segmentSmallTemp.beginningPoint = segment.beginningPoint;
	segmentLargeTemp.endPoint = segment.endPoint;
	RTRVector midPoint = segment.pointAt(splitMethod, node->small->boundingBox.point2(splitMethod));
	segmentSmallTemp.endPoint = segmentLargeTemp.beginningPoint = midPoint;
	search(node->large, searchResult, segmentLargeTemp);
	search(node->small, searchResult, segmentSmallTemp);
}
