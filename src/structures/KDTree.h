#pragma once
#include <glm.hpp>
#include <gtx/norm.hpp>
#include <vector>
#include <memory>
#include <algorithm>
#include "INearestNeighbor.h"

#pragma region KDNode
template<size_t K>
struct KDNode
{
	KDNode(const point_t<K>& point, size_t index);
	size_t Index;
	point_t<K> Point;
	std::unique_ptr<KDNode> Left;
	std::unique_ptr<KDNode> Right;
};

template<size_t K>
KDNode<K>::KDNode(const point_t<K>& point, size_t index) : Point(point), Index(index)
{
}
#pragma endregion

template<size_t K>
class KDTree : public INearestNeighbor<K>
{
 public:
	KDTree() = default;
	KDTree(const std::vector<point_t<K>>& points, size_t leafLimit = 1);
	const std::unique_ptr<KDNode<K>>& Root() const { return m_root; }

	void Build(const std::vector<point_t<K>>& points) override;
	bool FindNearestNeighbor(const point_t<K>& point, uint32_t& index) override;
	bool FindNearestNeighbors(const point_t<K>& point, float radius, std::vector<uint32_t>& indices) override;
	bool FindAllNearestNeighbors(const std::vector<point_t<K>>& points,
		float radius,
		std::vector<std::vector<uint32_t>>& indices) override;
 private:
	typedef std::pair<point_t<K>,size_t> KDPair;
	typedef typename std::vector<KDPair>::iterator point_Iterator;
	std::unique_ptr<KDNode<K>> m_root;
	const size_t m_leafLimit{1};

	std::unique_ptr<KDNode<K>> buildTree(const point_Iterator& begin, const point_Iterator& end, int currentLevel);
	KDNode<K>* findNearestNode(KDNode<K>* branch,
		const point_t<K>& point,
		size_t depth,
		KDNode<K>* best,
		float bestDistance) const;

	void findNearestNodesWithinRadius(KDNode<K>* branch,
		const point_t<K>& point,
		size_t depth,
		std::vector<uint32_t>& nearestNodes,
		float radius2) const;
};

template<size_t K>
KDTree<K>::KDTree(const std::vector<point_t<K>>& points, size_t leafLimit) : m_leafLimit(leafLimit)
{
	Build(points);
}

template<size_t K>
void KDTree<K>::Build(const std::vector<point_t<K>>& points)
{
	if(m_root)
		m_root.reset();

	//pair points with index, this also creates a copy of the points so the source vector is unchanged
	std::vector<KDPair> pairedPoints(points.size());
	for (size_t i = 0; i < points.size(); ++i)
	{
		pairedPoints[i] = std::make_pair(points[i],i);
	}

	m_root = buildTree(pairedPoints.begin(),pairedPoints.end(),0);
}


template<size_t K>
std::unique_ptr<KDNode<K>> KDTree<K>::buildTree(const KDTree::point_Iterator& begin,
	const KDTree::point_Iterator& end,
	int currentLevel)
{
	if(begin >= end)
		return nullptr;

	size_t axis = currentLevel % K;

	//find the mid point
	std::size_t len = end - begin;
	auto mid = begin + len / 2;

	auto cmp = [axis](const KDPair& p1, const KDPair& p2)
	{
		return p1.first[axis] < p2.first[axis];
	};

	std::nth_element(begin, mid, end, cmp); // linear time partition

	while (mid > begin && (mid - 1)->first[axis] == mid->first[axis])
	{
		--mid;
	}

	std::unique_ptr<KDNode<K>> node = std::make_unique<KDNode<K>>(mid->first,mid->second);
	node->Left = buildTree(begin,mid, currentLevel+1);
	node->Right = buildTree(mid + 1 ,end, currentLevel+1);
	return node;
}

template<size_t K>
KDNode<K>* KDTree<K>::findNearestNode(KDNode<K>* branch,
	const point_t<K>& point,
	size_t depth,
	KDNode<K>* best,
	float bestDistance) const
{
	float d, dx, dx2;
	if(!branch)
		return nullptr;

	const point_t<K>& branchPoint = branch->Point;

	d = glm::distance2(branchPoint,point);
	dx = branchPoint[depth] - point[depth];
	dx2 = dx * dx;

	KDNode<K>* best_l = best;
	float best_dist_l = bestDistance;
	if (d < bestDistance) {
		best_dist_l = d;
		best_l = branch;
	}

	size_t next_lv = (depth + 1) % K;
	KDNode<K>* section = nullptr;
	KDNode<K>* other = nullptr;

	// select which branch makes sense to check
	if (dx > 0) {
		section = branch->Left.get();
		other = branch->Right.get();
	} else {
		section = branch->Right.get();
		other = branch->Left.get();
	}

	KDNode<K>* further = findNearestNode(section, point, next_lv, best_l, best_dist_l);
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

template<size_t K>
bool KDTree<K>::FindNearestNeighbor(const point_t<K>& point, uint32_t& index)
{
	auto node = findNearestNode(m_root.get(),point,0, nullptr,std::numeric_limits<float>::infinity());

	if(node)
	{
		index = node->Index;
		return true;
	}
	return false;
}

template<size_t K>
bool KDTree<K>::FindNearestNeighbors(const point_t<K>& point, float radius, std::vector<uint32_t>& indices)
{
	findNearestNodesWithinRadius(m_root.get(),point,0,indices,radius * radius);
	return !indices.empty();
}

template<size_t K>
void KDTree<K>::findNearestNodesWithinRadius(KDNode<K>* branch,
	const point_t<K>& point,
	size_t depth,
	std::vector<uint32_t>& nearestNodes,
	float radius2) const
{

	float d, dx, dx2;
	if(!branch)
		return;

	const point_t<K>& branchPoint = branch->Point;

	d = glm::distance2(branchPoint, point);
	dx = branchPoint[depth] - point[depth];
	dx2 = dx * dx;

	if(d <= radius2)
		nearestNodes.push_back(branch->Index);

	size_t next_lv = (depth + 1) % K;
	KDNode<K>* section = nullptr;
	KDNode<K>* other = nullptr;

	// select which branch makes sense to check
	if (dx > 0) {
		section = branch->Left.get();
		other = branch->Right.get();
	} else {
		section = branch->Right.get();
		other = branch->Left.get();
	}

	findNearestNodesWithinRadius(section, point, next_lv, nearestNodes, radius2);

	if (dx2 <= radius2) {
		findNearestNodesWithinRadius(other, point, next_lv, nearestNodes, radius2);
	}
}

template<size_t K>
bool KDTree<K>::FindAllNearestNeighbors(const std::vector<point_t<K>>& points,
	float radius,
	std::vector<std::vector<uint32_t>>& indices)
{
	return false;
}
