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
	bool IsSphereInside(const glm::vec3& point, float radius) const;
	bool IsSphereOutside(const glm::vec3& point, float radius) const;
	std::vector<std::pair<glm::vec3,glm::vec3>> Intersection(const glm::vec3& point, const glm::vec3& dir) const;
	std::vector<std::pair<glm::vec3, glm::vec3>> Intersection(const glm::vec3& point, const glm::vec3& vel, float radius) const;
	std::pair<glm::vec3, glm::vec3> GetClosestPoint(const glm::vec3& point, bool flipNormal) const;

	void Expand(float expandScalar);

	const glm::vec3& Min() const;
	const glm::vec3& Max() const;

	float Height() const;
	float Width() const;
	float Depth() const;
 private:
	glm::vec3 m_min, m_max;
	glm::vec3 getNormal(const glm::vec3 boundedPoint) const;

	void getNearFar(const glm::vec3& point, const glm::vec3& dir, float& near, float& far, glm::vec3& diff) const;
};
