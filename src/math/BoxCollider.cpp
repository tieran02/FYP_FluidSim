#include "BoxCollider.h"

BoxCollider::BoxCollider(const glm::vec3& Min, const glm::vec3& Max) : Collider(Collider::Shape::BOX), m_aabb(Min, Max)
{

}

bool BoxCollider::CollisionOccured(const PlaneCollider& collider,
	const glm::vec3& velocity,
	CollisionData& collisionData) const
{
	//TODO
	return false;
}

bool BoxCollider::CollisionOccured(const glm::vec3& point,
	const glm::vec3& velocity,
	CollisionData& collisionData) const
{
	auto intersections = m_aabb.Intersection(point,velocity,0.1f);
	if(!intersections.empty())
	{
		// TODO: collisionData.CollisionNormal
		collisionData.ContactPoint = intersections[0].first;
		collisionData.CollisionNormal = intersections[0].second;
		collisionData.Distance = glm::distance(point,collisionData.ContactPoint);

		if(m_aabb.IsPointInside(collisionData.ContactPoint) || collisionData.Distance < 0.1f)
			return true;

	}
	return false;
}

void BoxCollider::SetTransform(const Transform& transform)
{
	//TODO
}
