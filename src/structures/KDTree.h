#pragma once
#include <glm.hpp>
#include <vector>
#include <memory>

struct KDNode
{
	KDNode(const glm::vec3& point);
	const glm::vec3& Point;
	std::unique_ptr<KDNode> Left;
	std::unique_ptr<KDNode> Right;
};


class KDTree
{
 public:
	KDTree(std::vector<glm::vec3>& points, size_t leafLimit);
 private:
	typedef std::vector<glm::vec3> POINT_VECTOR;
	std::unique_ptr<KDNode> m_root;

	std::unique_ptr<KDNode> buildTree(const std::vector<glm::vec3>::iterator& begin, const std::vector<glm::vec3>::iterator& end, int currentLevel);
	std::vector<glm::vec3>::iterator findMedian(const std::vector<glm::vec3>::iterator& begin, const std::vector<glm::vec3>::iterator& end, size_t axis);
};
