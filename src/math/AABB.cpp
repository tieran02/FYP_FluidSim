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

std::vector<std::pair<glm::vec3,glm::vec3>> AABB::Intersection(const glm::vec3& point, const glm::vec3& dir) const
{
	auto diff = (point + dir) - point;
	auto pointToMin = m_min - point;
	auto pointToMax = m_max - point;
	auto near = std::numeric_limits<float>::min();
	auto far = std::numeric_limits<float>::max();
	glm::vec3 normal{0.0f}; //TODO calculate hit normal

	std::vector<std::pair<glm::vec3,glm::vec3>> intersections;
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
		glm::vec3 intersectionPoint = point + diff * near;
		glm::vec3 intersectionNormal = getNormal(intersectionPoint);
		intersections.emplace_back(intersectionPoint,intersectionNormal);
	}
	if (far >= std::numeric_limits<float>::epsilon()  && far <= 1.0f)
	{
		glm::vec3 intersectionPoint = point + diff * far;
		glm::vec3 intersectionNormal = getNormal(intersectionPoint);
		intersections.emplace_back(intersectionPoint,intersectionNormal);
	}

	return intersections;
}

void AABB::Expand(float expandScalar)
{
	m_max += glm::vec3(expandScalar);
	m_min -= glm::vec3(expandScalar);
}

glm::vec3 AABB::getNormal(const glm::vec3 boundedPoint) const
{
	if(boundedPoint.y == m_min.y)  //bottom
		return glm::vec3(0.0f,1.0f,0.0f);
	else if(boundedPoint.y == m_max.y) //top
		return glm::vec3(0.0f,-1.0f,0.0f);
	else if(boundedPoint.x == m_min.x) //left
		return glm::vec3(1.0f,0.0f,0.0f);
	else if(boundedPoint.x == m_max.x) //right
		return glm::vec3(-1.0f,0.0f,0.0f);
	else if(boundedPoint.z == m_min.z) //front
		return glm::vec3(0.0f,0.0f,1.0f);
	else if(boundedPoint.z == m_max.z) //back
		return glm::vec3(0.0f,0.0f,-1.0f);

	return glm::vec3(0.0f,0.0f,0.0f);
}

const glm::vec3& AABB::Min() const
{
	return m_min;
}

const glm::vec3& AABB::Max() const
{
	return m_max;
}

float AABB::Height() const
{
	return m_max.y - m_min.y;
}
float AABB::Width() const
{
	return m_max.x - m_min.x;
}
float AABB::Depth() const
{
	return m_max.z - m_min.z;
}
