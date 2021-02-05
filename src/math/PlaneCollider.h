#pragma once
#include "Collider.h"
#include "Plane.h"

class PlaneCollider : public Collider
{
 public:
	PlaneCollider(const Plane& plane);

	bool CollisionOccured(const PlaneCollider& collider, const glm::vec3& velocity, CollisionData& collisionData) override;
	bool CollisionOccured(const glm::vec3& point, const glm::vec3& velocity, CollisionData& collisionData) override;

	//Set transform of the collider (Pos, Rotation, Scale)
	void SetTransform(const Transform& transform) override;
 private:
	Plane m_plane;
};
