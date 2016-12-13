#pragma once

#include <vector>
#include <algorithm>
#include <queue>
#include <cmath>
#include <set>

template<class T>
static T myPow2(T val)
{
    return val * val;
}

class Point;

template<class TElem, class TCoord, int Dim, class TAccCoord>
class NearestSearchKdTree
{
protected:
    static TAccCoord accCoord;
public:
    class Node
    {
    public:
        int splitDirection;
        TCoord splitValue;
        std::vector<TElem> points;
        Node* smallChild;
        Node* largeChild;
        Node* parent;
        TCoord boundingBoxMin[Dim];
        TCoord boundingBoxMax[Dim];
    };
protected:
    Node* root;
public:
    static void construct(Node* node, std::vector<TElem> points, int divition,
        int allSameCount, TCoord boundingBoxMin[Dim], TCoord boundingBoxMax[Dim], Node* parent);
    static void construct(Node* node, std::vector<TElem> points);
    static NearestSearchKdTree* construct(std::vector<TElem> points);
    void insertTree(std::vector<TElem>& points, Node* node, TElem value, int count);
    void lookupNearest(std::vector<TElem> &resultCache, Node* root, TElem value, int count);
    std::vector<TElem> lookupNearest(TElem value, int count);
protected:
    //std::vector<TElem> resultCache;
    NearestSearchKdTree();
public:
    ~NearestSearchKdTree();
};

template<class TElem, class TCoord, int Dim, class TAccCoord>
TAccCoord NearestSearchKdTree<TElem, TCoord, Dim, TAccCoord>::accCoord;

template<class TElem, class TCoord, int Dim, class TAccCoord>
void NearestSearchKdTree<TElem, TCoord, Dim, TAccCoord>::construct(Node* node, std::vector<TElem> points, int divition,
    int allSameCount, TCoord boundingBoxMin[Dim], TCoord boundingBoxMax[Dim], Node* parent)
{
    node->parent = parent;
    for (int i = 0; i < Dim; i++)
    {
        node->boundingBoxMin[i] = boundingBoxMin[i];
        node->boundingBoxMax[i] = boundingBoxMax[i];
    }
    //If there are less than 5 points, stop divition.
    if (points.size() < 128)
    {
        node->splitDirection = -1;
        node->smallChild = node->largeChild = nullptr;
        node->points = points;
        return;
    }
    node->splitDirection = divition;
    std::sort(points.begin(), points.end(), [divition](TElem p1, TElem p2) {
        return accCoord(p1, divition) < accCoord(p2, divition);
    });
    int medianPos = points.size() / 2;
    //Divition rule: [left, medianPos), [medianPos, right)
    TCoord median = accCoord(points[medianPos], divition);
    int medianRangeLeft = medianPos;
    int medianRangeRight = medianPos;
    while (medianRangeLeft != 0 && accCoord(points[medianRangeLeft - 1], divition) == median)
    {
        medianRangeLeft--;
    }
    while (medianRangeRight != points.size() - 1 && accCoord(points[medianRangeRight + 1], divition) == median)
    {
        medianRangeRight++;
    }
    if (medianRangeLeft == 0 && medianRangeRight == points.size() - 1)
    {
        allSameCount++;
        //In a special case where all points are of the same value.
        if (allSameCount == Dim)
        {
            node->splitDirection = -1;
            node->smallChild = node->largeChild = nullptr;
            node->points = points;
            return;
        }
        divition++;
        divition %= Dim;
        construct(node, points, divition, allSameCount, boundingBoxMin, boundingBoxMax, parent);
        return;
        //TODO..... : 1.check reverse, 2. check different way
    }
    allSameCount = 0;
    if (points.size() - 1 - medianRangeRight > (unsigned int)medianRangeLeft)
    {
        medianPos = medianRangeRight + 1;
        median = accCoord(points[medianPos], divition);
    }
    else
    {
        medianPos = medianRangeLeft;
    }
    std::vector<TElem> smallData(points.begin(), points.begin() + medianPos);
    std::vector<TElem> largeData(points.begin() + medianPos, points.end());
    node->splitValue = median;
    node->smallChild = new Node();
    node->largeChild = new Node();
    int nextDivition = divition + 1;
    nextDivition %= Dim;
    TCoord temp = boundingBoxMax[divition];
    boundingBoxMax[divition] = node->splitValue;
    construct(node->smallChild, smallData, nextDivition, allSameCount, boundingBoxMin, boundingBoxMax, node);
    boundingBoxMax[divition] = temp;
    boundingBoxMin[divition] = node->splitValue;
    construct(node->largeChild, largeData, nextDivition, allSameCount, boundingBoxMin, boundingBoxMax, node);
    boundingBoxMin[divition] = node->boundingBoxMin[divition];
}

template<class TElem, class TCoord, int Dim, class TAccCoord>
void NearestSearchKdTree<TElem, TCoord, Dim, TAccCoord>::construct(Node* node, std::vector<TElem> points)
{
    TCoord minBound[Dim];
    TCoord maxBound[Dim];
    for (int i = 0; i < Dim; i++)
    {
        minBound[i] = accCoord(points[0], i);
        maxBound[i] = accCoord(points[0], i);
    }
    for (auto i : points)
    {
        for (int j = 0; j < Dim; j++)
        {
            if (accCoord(i, j) < minBound[j])
            {
                minBound[j] = accCoord(i, j);
            }
            if (accCoord(i, j) > maxBound[j])
            {
                maxBound[j] = accCoord(i, j);
            }
        }
    }
    for (int i = 0; i < Dim; i++)
    {
        if (maxBound[i] != minBound[i])
        {
            maxBound[i] += (maxBound[i] - minBound[i]) / 10000.0;
        }
        else
        {
            maxBound[i] += 0.000001;
        }
    }
    construct(node, points, 0, 0, minBound, maxBound, nullptr);
}

template<class TElem, class TCoord, int Dim, class TAccCoord>
NearestSearchKdTree<TElem, TCoord, Dim, TAccCoord> *NearestSearchKdTree<TElem, TCoord, Dim, TAccCoord>::construct(std::vector<TElem> points)
{
    Node* root = new Node();
    construct(root, points);
    NearestSearchKdTree<TElem, TCoord, Dim, TAccCoord> *kdTree =
        new NearestSearchKdTree<TElem, TCoord, Dim, TAccCoord>();
    kdTree->root = root;
    return kdTree;
}

template<class TElem, class TCoord, int Dim, class TAccCoord>
void NearestSearchKdTree<TElem, TCoord, Dim, TAccCoord>::insertTree(std::vector<TElem>& points, typename NearestSearchKdTree<TElem, TCoord, Dim, TAccCoord>::Node* node, TElem value, int count)
{
    if (node->splitDirection == -1)
    {
        points.insert(points.end(), node->points.begin(), node->points.end());
        if(node->points.size() > 0)
        {
            std::sort(points.begin(), points.end(), [value](TElem p1, TElem p2) {
                TCoord length1 = 0.0, length2 = 0.0;
                for (int i = 0; i < Dim; i++)
                {
                    length1 += myPow2(accCoord(p1, i) - accCoord(value, i));
                    length2 += myPow2(accCoord(p2, i) - accCoord(value, i));
                }
                return length1 < length2;
            });
        }
        if(points.size() > count)
        {
            points.resize(count);
        }
    }
    else
    {
        insertTree(points, node->largeChild, value, count);
        insertTree(points, node->smallChild, value, count);
    }
}

template<class TElem, class TCoord, int Dim, class TAccCoord>
void NearestSearchKdTree<TElem, TCoord, Dim, TAccCoord>::lookupNearest(std::vector<TElem>& resultCache, Node * root, TElem value, int count)
{
    if(resultCache.size() >= count)
    {
      bool  finish = true;
      TElem furthest = resultCache[count - 1];
      TCoord length = 0.0;
      for (int i = 0; i < Dim; i++)
      {
          length += pow(accCoord(furthest, i) - accCoord(value, i), 2);
      }
      for (int i = 0; i < Dim; i++)
      {
          if (myPow2(root->boundingBoxMax[i] - accCoord(value, i)) <= length)
          {
              finish = false;
              break;
          }
          if (myPow2(root->boundingBoxMin[i] - accCoord(value, i)) <= length)
          {
              finish = false;
              break;
          }
      }
      if(finish)
      {
          return;
      }
    }
    Node* leaf = root;
    while (leaf->splitDirection != -1)
    {
        if (accCoord(value, leaf->splitDirection) >= leaf->splitValue)
        {
            leaf = leaf->largeChild;
        }
        else
        {
            leaf = leaf->smallChild;
        }
    }
    insertTree(resultCache, leaf, value, count);
    for (;;)
    {
        bool finish = true;
        if (resultCache.size() < count)
        {
            finish = false;
        }
        else
        {
            //resultCache.resize(count);
            TElem furthest = resultCache[count - 1];
            TCoord length = 0.0;
            for (int i = 0; i < Dim; i++)
            {
                length += pow(accCoord(furthest, i) - accCoord(value, i), 2);
            }
            //std::cout << length << std::endl;
            for (int i = 0; i < Dim; i++)
            {
                if (myPow2(leaf->boundingBoxMax[i] - accCoord(value, i)) <= length)
                {
                    finish = false;
                    break;
                }
                if (myPow2(leaf->boundingBoxMin[i] - accCoord(value, i)) <= length)
                {
                    finish = false;
                    break;
                }
            }
        }
        if (finish || leaf == root)
        {
            break;
        }
        auto oldLeaf = leaf;
        leaf = leaf->parent;
        if (leaf->smallChild == oldLeaf)
        {
            lookupNearest(resultCache, leaf->largeChild, value, count);
        }
        else
        {
            lookupNearest(resultCache, leaf->smallChild, value, count);
        }
    }
}

template<class TElem, class TCoord, int Dim, class TAccCoord>
std::vector<TElem> NearestSearchKdTree<TElem, TCoord, Dim, TAccCoord>::lookupNearest(TElem value, int count)
{
    std::vector<TElem> ret;
    lookupNearest(ret, root, value, count);
    return ret;
}

template<class TElem, class TCoord, int Dim, class TAccCoord>
NearestSearchKdTree<TElem, TCoord, Dim, TAccCoord>::NearestSearchKdTree()
{
}

template<class TElem, class TCoord, int Dim, class TAccCoord>
NearestSearchKdTree<TElem, TCoord, Dim, TAccCoord>::~NearestSearchKdTree()
{
}
