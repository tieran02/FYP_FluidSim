#pragma once

#include <glm.hpp>


struct Plane
{
	glm::vec3 A,B,C;

	glm::vec3 AB() const;
	glm::vec3 AC() const;
	glm::vec3 GetNormal() const;

	bool IsPointWithinPlane(const glm::vec3& point) const;
	bool LineIntersection(const glm::vec3& point, const glm::vec3& line, float& distance) const;
};
