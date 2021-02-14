#pragma once
#include "Collider.h"
#include "Plane.h"

class PlaneCollider : public Collider
{
 public:
	PlaneCollider(const Plane& plane, bool IsInfinite = false );
	PlaneCollider(const Plane& plane, const Transform& transform, bool IsInfinite = false );

	bool CollisionOccured(const PlaneCollider& collider, const glm::vec3& velocity, CollisionData& collisionData) const override;
	bool CollisionOccured(const glm::vec3& point, const glm::vec3& velocity, CollisionData& collisionData) const override;

	//Set transform of the collider (Pos, Rotation, Scale)
	void SetTransform(const Transform& transform) override;
	const Plane& GetPlane() const;
 private:
	Plane m_plane;
	bool m_isInfinite;
};
