#include "PlaneCollider.h"
#include "gtx/intersect.hpp"

PlaneCollider::PlaneCollider(const Plane& plane, bool IsInfinite) : Collider(Shape::PLANE), m_plane(plane), m_isInfinite(IsInfinite)
{

}

PlaneCollider::PlaneCollider(const Plane& plane, const Transform& transform, bool IsInfinite) : Collider(Shape::PLANE), m_plane(plane), m_isInfinite(IsInfinite)
{
	SetTransform(transform);
}


bool PlaneCollider::CollisionOccured(const PlaneCollider& collider, const glm::vec3& velocity, CollisionData& collisionData) const
{
	//TODO: implement plane to plane collision
	return false;
}

bool PlaneCollider::CollisionOccured(const glm::vec3& point, const glm::vec3& velocity, CollisionData& collisionData) const
{
	glm::vec3 N = m_plane.GetNormal();

	float distance = 0.0f;

	bool collided = m_plane.LineIntersection(point,velocity,distance);
	if(collided)
	{
		collisionData.CollisionNormal = N;
		collisionData.ContactPoint = point - (velocity * distance);
		collisionData.Distance = distance;

		return m_plane.IsPointWithinPlane(collisionData.ContactPoint);
	}

	return false;
}

void PlaneCollider::SetTransform(const Transform& transform)
{
	//m_plane.SetTransform(transform);
}

const Plane& PlaneCollider::GetPlane() const
{
	return m_plane;
}
