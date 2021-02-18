#include <vector>
#include "AABB.h"

AABB::AABB(const glm::vec3& Min, const glm::vec3& Max) : m_min(Min), m_max(Max)
{

}

bool AABB::IsPointInside(const glm::vec3& point) const
{
	return (point.x >= m_min.x && point.x <= m_max.x) &&
		(point.y >= m_min.y && point.y <= m_max.y) &&
		(point.z >= m_min.z && point.z <= m_max.z);
}

bool AABB::IsPointOutside(const glm::vec3& point) const
{
	return !IsPointInside(point);
}

std::vector<glm::vec3> AABB::Intersection(const glm::vec3& point, const glm::vec3& dir) const
{
	auto diff = (point + dir) - point;
	auto pointToMin = m_min - point;
	auto pointToMax = m_max - point;
	auto near = std::numeric_limits<float>::min();
	auto far = std::numeric_limits<float>::max();
	glm::vec3 normal{0.0f}; //TODO calculate hit normal

	std::vector<glm::vec3> intersections;
	for (int axis = 0; axis < 3; ++axis)
	{
		if(fabs(diff[axis]) <= std::numeric_limits<float>::epsilon()) // point on axis is parallel
		{
			if(pointToMin[axis] > 0.0f || pointToMax[axis] < 0.0f)
				return intersections; //segment is not on planes
		}
		else
		{
			float u = pointToMin[axis] / diff[axis];
			float v = pointToMax[axis] / diff[axis];
			float uvMin = std::min(u,v);
			float uvMax = std::max(u,v);

			if(uvMin > near)
			{
				near = uvMin;
			}
			if(uvMax < far)
			{
				far = uvMax;
			}
			if(near > far || far < 0.0f)
				return intersections;
		}
	}
	if (near >= std::numeric_limits<float>::epsilon() && near <= 1.0f)
	{
		intersections.push_back(point + diff * near);
	}
	if (far >= std::numeric_limits<float>::epsilon()  && far <= 1.0f)
	{
		intersections.push_back(point + diff * far);
	}

	return intersections;
}
