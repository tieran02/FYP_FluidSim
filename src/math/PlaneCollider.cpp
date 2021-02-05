#include "PlaneCollider.h"

PlaneCollider::PlaneCollider(const Plane& plane) : Collider(Shape::PLANE), m_plane(plane)
{

}

bool PlaneCollider::CollisionOccured(const PlaneCollider& collider, const glm::vec3& velocity, CollisionData& collisionData)
{
	//TODO: implement plane to plane collision
	return false;
}

bool PlaneCollider::CollisionOccured(const glm::vec3& point, const glm::vec3& velocity, CollisionData& collisionData)
{
	return false;
}

void PlaneCollider::SetTransform(const Transform& transform)
{
	glm::mat4 model = transform.ModelMatrix();
	m_plane.A = glm::vec4(m_plane.A,1) * model;
	m_plane.B = glm::vec4(m_plane.B,1) * model;
	m_plane.C = glm::vec4(m_plane.C,1) * model;
}
