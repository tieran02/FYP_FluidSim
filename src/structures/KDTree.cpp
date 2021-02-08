#include <functional>
#include <algorithm>
#include <util/Log.h>
#include "KDTree.h"

KDNode::KDNode(const glm::vec3& point) : Point(point)
{

}

KDTree::KDTree(std::vector<glm::vec3>& points, size_t leafLimit)
{
	m_root = buildTree(points.begin(),points.end(),0);
}

std::unique_ptr<KDNode> KDTree::buildTree(const std::vector<glm::vec3>::iterator& begin, const std::vector<glm::vec3>::iterator& end, int depth)
{

	//For the purpose of the project I only need to work in 3D so the KDTree will be hardcoded for 3 Dimensions
	constexpr size_t k = 3;

	if(begin >= end)
		return nullptr;

	size_t axis = depth % k;

	//sort the points
	auto mid = findMedian(begin,end,axis);

	//all equal point are to the right
	switch (axis)
	{
	case 0:
		while (mid > begin && (mid - 1)->x == mid->x) {
			--mid;
		}
		break;
	case 1:
		while (mid > begin && (mid - 1)->y == mid->y) {
			--mid;
		}
		break;
	case 2:
		while (mid > begin && (mid - 1)->z == mid->z) {
			--mid;
		}
		break;
	default:
	CORE_ASSERT(false, "KDTree::findMedian invalid axis")
		break;
	}

	std::unique_ptr<KDNode> node = std::make_unique<KDNode>(*mid);
	node->Left = buildTree(begin,mid, depth+1);
	node->Right = buildTree(mid + 1 ,end, depth+1);
	return node;
}

std::vector<glm::vec3>::iterator KDTree::findMedian(const std::vector<glm::vec3>::iterator& begin,
	const std::vector<glm::vec3>::iterator& end,
	size_t axis)
{
	std::size_t len = end - begin;
	auto mid = begin + len / 2;

	auto cmp = [axis](const glm::vec3& p1, const glm::vec3& p2)
	{
	  switch (axis)
	  {
	  case 0:
		  return p1.x< p2.x;
	  case 1:
		  return p1.y< p2.y;
	  case 2:
		  return p1.z< p2.z;
	  default:
	  	  CORE_ASSERT(false, "KDTree::findMedian invalid axis")
		  return p1.x< p2.x;
	  }
	};

	std::nth_element(begin,mid,end,cmp);
	return mid;
}
