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
	std::vector<std::pair<glm::vec3,glm::vec3>> Intersection(const glm::vec3& point, const glm::vec3& dir) const;

	const glm::vec3& Min() const {return m_min;}
	const glm::vec3& Max() const {return m_max;}
 private:
	glm::vec3 m_min, m_max;
	glm::vec3 getNormal(const glm::vec3 boundedPoint) const;
};
