#include "PlaneCollider.h"
#include "gtx/intersect.hpp"

PlaneCollider::PlaneCollider(const Plane& plane, bool IsInfinite) : Collider(Shape::PLANE), m_plane(plane), m_isInfinite(IsInfinite)
{

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
		return true;
	}

	return false;
}

void PlaneCollider::SetTransform(const Transform& transform)
{
	glm::mat4 model = transform.ModelMatrix();
	m_plane.A = glm::vec4(m_plane.A,1) * model;
	m_plane.B = glm::vec4(m_plane.B,1) * model;
	m_plane.C = glm::vec4(m_plane.C,1) * model;
}

const Plane& PlaneCollider::GetPlane() const
{
	return m_plane;
}
