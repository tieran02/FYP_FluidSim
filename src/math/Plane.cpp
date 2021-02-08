#include "Plane.h"

glm::vec3 Plane::GetNormal() const
{
	return glm::normalize(glm::cross(AB(),AC()));
}

glm::vec3 Plane::AB() const
{
	return B- A;
}

glm::vec3 Plane::AC() const
{
	return C - A;
}

bool Plane::IsPointWithinPlane(const glm::vec3& point) const
{
	glm::vec3 N = GetNormal();

	float dist = glm::dot(N, point - A);
	if (dist > std::numeric_limits<float>::epsilon())
	{
		return false;
	}

	glm::vec3 coplanarPoint = point - (dist * N);
	glm::vec3 pa = coplanarPoint - A;

	glm::vec3 ab = AB();
	glm::vec3 ac = AC();

	//Get the U,V positions local to the plane by using the AB and AC vectors of the plane
	//If U or V are less than 0 or greater than 1 the point is outside of the finite plane
	float u = glm::dot(pa, ab) / glm::dot(ab, ab);
	float v = glm::dot(pa, ac) / glm::dot(ac, ac);

	if (u < 0.0f || u > 1.0f || v < 0.0f || v > 1.0f)
		return false;

	return true;
}

bool Plane::LineIntersection(const glm::vec3& point, const glm::vec3& line, float& distance) const
{
	glm::vec3 N = GetNormal();
	/*float d = glm::dot(N, A);

	if (glm::dot(N, line) == 0)
	{
		return false; // No intersection, the line is parallel to the plane
	}

	float x = (d - glm::dot(N, point)) / glm::dot(N, line);
	contactPoint = point + normalize(line)*x;*/

	glm::vec3 diff = point - A;
	float d = glm::dot(N,diff);
	float e = glm::dot(N,line);

	distance = d / e;

	float fDist = fabs(d);
	float fLength = fabs(glm::length(line));

	if (fabs(e) < std::numeric_limits<float>::epsilon())
	{
		return false;
	}
	if (distance < std::numeric_limits<float>::epsilon())
	{
		return false;
	}

	return true;
}
