#pragma once
#include <glm.hpp>
#include "Transform.h"

class PlaneCollider;

struct CollisionData
{
	glm::vec3 CollisionNormal;
	glm::vec3 ContactPoint;
	float Distance;
};

class Collider
{
 public:
	//TODO add shpere collider
	enum class Shape
	{
		PLANE
	};

 public:
	virtual bool CollisionOccured(const PlaneCollider& collider, const glm::vec3& velocity, CollisionData& collisionData) = 0;
	virtual bool CollisionOccured(const glm::vec3& point, const glm::vec3& velocity, CollisionData& collisionData) = 0;
	virtual void SetTransform(const Transform& transform) = 0;
 protected:
	explicit Collider(Shape shape);

	const Shape m_shape;
};
