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

bool AABB::IsSphereInside(const glm::vec3& point, float radius) const
{
	if(IsPointOutside(point))
		return false;

	//TODO closet point only only works when the point is outside the AABB
	auto closestPoint = GetClosestPoint(point, false);
	float distance2 = glm::distance2(closestPoint.first, point);

	if(distance2 <= radius * radius)
		return false;

	return true;
}

bool AABB::IsSphereOutside(const glm::vec3& point, float radius) const
{
	return !IsSphereInside(point,radius);
}

std::vector<std::pair<glm::vec3,glm::vec3>> AABB::Intersection(const glm::vec3& point, const glm::vec3& dir) const
{
	auto diff = (point + dir) - point;
	auto near = std::numeric_limits<float>::min();
	auto far = std::numeric_limits<float>::max();

	std::vector<std::pair<glm::vec3,glm::vec3>> intersections;

	getNearFar(point,dir,near,far, diff);

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

std::pair<glm::vec3, glm::vec3> AABB::GetClosestPoint(const glm::vec3& point, bool flipNormal) const
{
	//TODO closest point should use euclidean distance instead of a Manhattan approach with clamping on the axis
	auto result = point;
	if(IsPointOutside(point))
	{
		result.x = std::max(m_min.x, std::min(result.x ,m_max.x));
		result.y = std::max(m_min.y, std::min(result.y ,m_max.y));
		result.z = std::max(m_min.z, std::min(result.z ,m_max.z));
	}
	else
	{
		float max = std::numeric_limits<float>::min();
		float min = std::numeric_limits<float>::max();
		int minAxis = 0;
		int maxAxis = 0;

		for (int axis = 0; axis < 3; ++axis)
		{
			if(point[axis] < min)
			{
				min = point[axis];
				minAxis = axis;
			}
			if(point[axis] > max)
			{
				max = point[axis];
				maxAxis = axis;
			}
		}

		//check if closest to min or max
		if(fabs(min) > fabs(max))
			result[minAxis] = m_min[minAxis];
		else
			result[maxAxis] = m_max[maxAxis];
	}

	glm::vec3 intersectionNormal = flipNormal ? -getNormal(result) : getNormal(result);
	return std::make_pair(result,intersectionNormal);
}

void AABB::getNearFar(const glm::vec3& point, const glm::vec3& dir, float& near, float& far, glm::vec3& diff) const
{
	auto pointToMin = m_min - point;
	auto pointToMax = m_max - point;
	near = std::numeric_limits<float>::min();
	far = std::numeric_limits<float>::max();
	glm::vec3 normal{0.0f}; //TODO calculate hit normal

	std::vector<std::pair<glm::vec3,glm::vec3>> intersections;
	for (int axis = 0; axis < 3; ++axis)
	{
		if(fabs(diff[axis]) <= std::numeric_limits<float>::epsilon()) // point on axis is parallel
		{
			if(pointToMin[axis] > 0.0f || pointToMax[axis] < 0.0f)
				return;
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
		}
	}
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

