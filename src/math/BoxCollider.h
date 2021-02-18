#pragma once

#include "Collider.h"
#include "AABB.h"

class BoxCollider : public Collider
{
 public:
	BoxCollider(const glm::vec3& Min, const glm::vec3& Max);
	bool CollisionOccured(const PlaneCollider& collider, const glm::vec3& velocity, CollisionData& collisionData) const override;
	bool CollisionOccured(const glm::vec3& point, const glm::vec3& velocity, CollisionData& collisionData) const override;
	void SetTransform(const Transform& transform) override;
	const AABB& GetAABB() const {return m_aabb;}
 private:
	AABB m_aabb;
};
