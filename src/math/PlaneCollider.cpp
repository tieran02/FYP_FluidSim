#include "PlaneCollider.h"

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
	glm::vec3 k = m_plane.A;
	glm::vec3 P = point - k;

	float dist = glm::dot(N, P);
	if (dist > std::numeric_limits<float>::epsilon())
	{
		return false;
	}

	//If the plane is finite, check if the colliers point is within the finite plane
	if ((!m_isInfinite && !m_plane.IsPointWithinPlane(point)) || dist > std::numeric_limits<float>::epsilon())
	{
		return false;
	}

	float d = glm::dot(N, P);
	float e = glm::dot(N, -velocity);

	if (e > std::numeric_limits<float>::epsilon())
	{
		collisionData.CollisionNormal = N;
		collisionData.ContactPoint = point + velocity * d / e;
		collisionData.Distance = d / e;
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
