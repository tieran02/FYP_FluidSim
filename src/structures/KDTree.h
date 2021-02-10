#pragma once
#include <glm.hpp>
#include <vector>
#include <memory>
#include "INearestNeighbor .h"

template<size_t K>
using point_t = glm::vec<K, float, glm::defaultp>;

template<size_t K, typename ElementType>
using KDPair = std::pair<point_t<K>,ElementType>;

#pragma region KDNode
template<size_t K, typename ElementType>
struct KDNode
{
	KDNode(const point_t<K>& point, ElementType element);
	ElementType Element;
	point_t<K> Point;
	std::unique_ptr<KDNode> Left;
	std::unique_ptr<KDNode> Right;
};

template<size_t K, typename ElementType>
KDNode<K, ElementType>::KDNode(const point_t<K>& point, ElementType element) : Point(point), Element(element)
{
}
#pragma endregion

template<size_t K, typename ElementType>
class KDTree : public INearestNeighbor<K,ElementType>
{
 public:
	KDTree(const std::vector<point_t<K>>& points, size_t leafLimit = 1);
	const std::unique_ptr<KDNode<K,ElementType>>& Root() const { return m_root; }

	void Build(const std::vector<point_t<K>>& points) override;
	bool FindNearestNeighbor(const point_t<K>& point, ElementType& element) override;
	bool FindNearestNeighbors(const point_t<K>& point, float radius, std::vector<ElementType>&) override;
 private:
	typedef KDPair<K,ElementType> pair;
	typedef typename std::vector<pair>::iterator point_Iterator;
	std::unique_ptr<KDNode<K,ElementType>> m_root;
	const size_t m_leafLimit;

	std::unique_ptr<KDNode<K,ElementType>> buildTree(const point_Iterator& begin, const point_Iterator& end, int currentLevel);
	KDNode<K, ElementType>* findNearestNode(KDNode<K, ElementType>* branch,
		const point_t<K>& point,
		size_t depth,
		KDNode<K, ElementType>* best,
		float bestDistance) const;

	void findNearestNodesWithinRadius(KDNode<K, ElementType>* branch,
		const point_t<K>& point,
		size_t depth,
		std::vector<ElementType>& nearestNodes,
		float radius) const;
};

template<size_t K, typename ElementType>
KDTree<K, ElementType>::KDTree(const std::vector<point_t<K>>& points, size_t leafLimit) : m_leafLimit(leafLimit)
{
	Build(points);
}

template<size_t K, typename ElementType>
void KDTree<K, ElementType>::Build(const std::vector<point_t<K>>& points)
{
	if(m_root)
		m_root.reset();

	//pair points with index, this also creates a copy of the points so the source vector is unchanged
	std::vector<KDPair<K,ElementType>> pairedPoints(points.size());
	for (size_t i = 0; i < points.size(); ++i)
	{
		pairedPoints[i] = std::make_pair(points[i],i);
	}

	m_root = buildTree(pairedPoints.begin(),pairedPoints.end(),0);
}


template<size_t K, typename ElementType>
std::unique_ptr<KDNode<K, ElementType>> KDTree<K, ElementType>::buildTree(const KDTree::point_Iterator& begin,
	const KDTree::point_Iterator& end,
	int currentLevel)
{
	if(begin >= end)
		return nullptr;

	size_t axis = currentLevel % K;

	//find the mid point
	std::size_t len = end - begin;
	auto mid = begin + len / 2;

	//TODO implement leaf size
	//if(len <= m_leafLimit)
	//	return nullptr;

	auto cmp = [axis](const std::pair<point_t<K>,ElementType>& p1, const std::pair<point_t<K>,ElementType>& p2)
	{
		return p1.first[axis] < p2.first[axis];
	};

	std::nth_element(begin, mid, end, cmp); // linear time partition

	while (mid > begin && (mid - 1)->first[axis] == mid->first[axis])
	{
		--mid;
	}

	std::unique_ptr<KDNode<K,ElementType>> node = std::make_unique<KDNode<K,ElementType>>(mid->first,mid->second);
	node->Left = buildTree(begin,mid, currentLevel+1);
	node->Right = buildTree(mid + 1 ,end, currentLevel+1);
	return node;
}

template<size_t K, typename ElementType>
KDNode<K, ElementType>* KDTree<K, ElementType>::findNearestNode(KDNode<K, ElementType>* branch,
	const point_t<K>& point,
	size_t depth,
	KDNode<K, ElementType>* best,
	float bestDistance) const
{
	float d, dx, dx2;
	if(!branch)
		return nullptr;

	const point_t<K>& branchPoint = branch->Point;

	d = glm::distance2(branchPoint,point);
	dx = branchPoint[depth] - point[depth];
	dx2 = dx * dx;

	KDNode<K, ElementType>* best_l = best;
	float best_dist_l = bestDistance;
	if (d < bestDistance) {
		best_dist_l = d;
		best_l = branch;
	}

	size_t next_lv = (depth + 1) % K;
	KDNode<K, ElementType>* section = nullptr;
	KDNode<K, ElementType>* other = nullptr;

	// select which branch makes sense to check
	if (dx > 0) {
		section = branch->Left.get();
		other = branch->Right.get();
	} else {
		section = branch->Right.get();
		other = branch->Left.get();
	}

	KDNode<K, ElementType>* further = findNearestNode(section, point, next_lv, best_l, best_dist_l);
	if(further)
	{
		float dl = glm::distance2(further->Point, point);

		if (dl < best_dist_l) {
			best_dist_l = dl;
			best_l = further;
		}
	}

	// only check the other branch if it makes sense to do so
	if (dx2 < best_dist_l) {
		further = findNearestNode(other, point, next_lv, best_l, best_dist_l);
		if (further)
		{
			float dl = glm::distance2(further->Point, point);
			if (dl < best_dist_l) {
				best_dist_l = dl;
				best_l = further;
			}
		}
	}

	return best_l;
}

template<size_t K, typename ElementType>
bool KDTree<K, ElementType>::FindNearestNeighbor(const point_t<K>& point, ElementType& element)
{
	auto node = findNearestNode(m_root.get(),point,0, nullptr,std::numeric_limits<float>::infinity());

	if(node)
	{
		element = node->Element;
		return true;
	}
	return false;
}

template<size_t K, typename ElementType>
bool KDTree<K, ElementType>::FindNearestNeighbors(const point_t<K>& point, float radius, std::vector<ElementType>& elements)
{
	elements = std::vector<ElementType>();

	findNearestNodesWithinRadius(m_root.get(),point,0,elements,radius);
	return false;
}

template<size_t K, typename ElementType>
void KDTree<K, ElementType>::findNearestNodesWithinRadius(KDNode<K, ElementType>* branch,
	const point_t<K>& point,
	size_t depth,
	std::vector<ElementType>& nearestNodes,
	float radius) const
{

	float d, dx, dx2;
	if(!branch)
		return;

	const point_t<K>& branchPoint = branch->Point;

	d = glm::distance2(branchPoint, point);
	dx = branchPoint[depth] - point[depth];
	dx2 = dx * dx;

	if(d <= radius)
		nearestNodes.push_back(branch->Element);

	size_t next_lv = (depth + 1) % K;
	KDNode<K, ElementType>* section = nullptr;
	KDNode<K, ElementType>* other = nullptr;

	// select which branch makes sense to check
	if (dx > 0) {
		section = branch->Left.get();
		other = branch->Right.get();
	} else {
		section = branch->Right.get();
		other = branch->Left.get();
	}

	findNearestNodesWithinRadius(section, point, next_lv, nearestNodes, radius);

	// only check the other branch if it makes sense to do so
	if (dx2 <= radius) {
		findNearestNodesWithinRadius(other, point, next_lv, nearestNodes, radius);
	}
}
