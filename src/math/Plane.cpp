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
