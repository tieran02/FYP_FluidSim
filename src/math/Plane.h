#pragma once
#include <glm.hpp>
#include "Transform.h"

/// Plane point layout
///A--------B
///|		|
///|		|
///C---------
///
struct Plane
{
	Plane(float nx, float ny, float nz, float d);
	Plane(const glm::vec4& plane);
	Plane(const glm::vec3& normal, float d);
	Plane(const glm::vec3& point, const glm::vec3& normal);
	Plane(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C);

	const glm::vec3& GetNormal() const;
	float GetD() const;

	float Dot(const glm::vec3& v, bool isPoint = false) const;
	bool IsPointWithinPlane(const glm::vec3& point) const;
	bool LineIntersection(const glm::vec3& point1, const glm::vec3& point2, float& distance) const;
	Plane TransformedPlane(const Transform& transform) const;

	float SignedDistance(const glm::vec3& point) const;
	float UnsignedDistance(const glm::vec3& point) const;

 private:
	glm::vec4 m_plane;
};
