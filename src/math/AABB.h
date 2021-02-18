#pragma once
#include <glm.hpp>
#include <vector>
#include "Collider.h"

struct AABB
{
 public:
	AABB(const glm::vec3& Min, const glm::vec3& Max);
	bool IsPointInside(const glm::vec3& point) const;
	bool IsPointOutside(const glm::vec3& point) const;
	std::vector<glm::vec3> Intersection(const glm::vec3& point, const glm::vec3& dir) const;
 private:
	glm::vec3 m_min, m_max;
};
