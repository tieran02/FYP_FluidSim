#include <util/Log.h>
#include "Plane.h"

Plane::Plane(float nx, float ny, float nz, float d) : m_plane(glm::vec4{nx,ny,nz,d})
{

}

Plane::Plane(const glm::vec4& plane) : m_plane(plane)
{

}

Plane::Plane(const glm::vec3& normal, float d) : m_plane(glm::vec4(normal.x,normal.y,normal.z,d))
{

}

Plane::Plane(const glm::vec3& point, const glm::vec3& normal)
{
	glm::vec3 normalizedNormal = glm::normalize(normal);
	m_plane.x = normalizedNormal.x;
	m_plane.y = normalizedNormal.y;
	m_plane.z = normalizedNormal.z;
	m_plane.w = -glm::dot(point, normalizedNormal);
}

Plane::Plane(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C)
{
	glm::vec3 normal = glm::cross((B-A),(C-A));
	glm::vec3 normalizedNormal = glm::normalize(normal);
	m_plane.x = normalizedNormal.x;
	m_plane.y = normalizedNormal.y;
	m_plane.z = normalizedNormal.z;
	m_plane.w = -glm::dot(A, normalizedNormal);
}

const glm::vec3& Plane::GetNormal() const
{
	return (reinterpret_cast<const glm::vec3&>(m_plane));
}

float Plane::GetD() const
{
	return m_plane.w;
}

float Plane::Dot(const glm::vec3& v, bool isPoint) const
{
	if(isPoint)
		return (m_plane.x * v.x + m_plane.y * v.y + m_plane.z * v.z + m_plane.w);

	return (m_plane.x * v.x + m_plane.y * v.y + m_plane.z * v.z);
}


bool Plane::IsPointWithinPlane(const glm::vec3& point) const
{
//	glm::vec3 N = GetNormal();
//
//	float dist = glm::dot(N, point - m_A);
//
//	glm::vec3 coplanarPoint = point - (dist * N);
//	glm::vec3 pa = coplanarPoint - m_A;
//
//	glm::vec3 ab = AB();
//	glm::vec3 ac = AC();
//
//	//Get the U,V positions local to the plane by using the AB and AC vectors of the plane
//	//If U or V are less than 0 or greater than 1 the point is outside of the finite plane
//	float u = glm::dot(pa, ab) / glm::dot(ab, ab);
//	float v = glm::dot(pa, ac) / glm::dot(ac, ac);
//
//	if (u < 0.0f || u > 1.0f || v < 0.0f || v > 1.0f)
//		return false;
//


	return true;
}

bool Plane::LineIntersection(const glm::vec3& point, const glm::vec3& line, float& distance) const
{
	glm::vec3 planeVector = GetNormal() * m_plane.w;

	float fv = Dot(point + line);
	if (fv >= std::numeric_limits<float>::epsilon())
		return false;

	float d = SignedDistance(point);
	if(d > std::numeric_limits<float>::epsilon())
		return false;

	distance = d / fv;

	if(distance <= 0 || fabs(fv) < fabs(m_plane.w))
		return false;

	return true;
}

Plane Plane::TransformedPlane(const Transform& transform) const
{
	glm::mat4 model = transform.ModelMatrix();
	glm::vec4 O = glm::vec4(GetNormal() * GetD(), 1.0f);
	glm::vec4 N = glm::vec4(GetNormal(), 0.0f);

	O = model * O;
	N = glm::transpose(glm::inverse(model)) * N;

	float d = dot(glm::vec3(O), -glm::vec3(N));

//	auto p = Plane(O,N);
//	auto normal = p.GetNormal();

	glm::mat4 m = transform.ModelMatrix();
	auto p = Plane
	(
		m_plane.x * m[0][0] + m_plane.y * m[1][0] + m_plane.z * m[2][0],
		m_plane.x * m[0][1] + m_plane.y * m[1][1] + m_plane.z * m[2][1],
		m_plane.x * m[0][2] + m_plane.y * m[1][2] + m_plane.z * m[2][2],
		d
	);
	//m_plane.x * m[0][3] + m_plane.y * m[1][3] + m_plane.z * m[2][3] + m_plane.w
	return p;
}

float Plane::SignedDistance(const glm::vec3& point) const
{
	return Dot(point, true);
}

float Plane::UnsignedDistance(const glm::vec3& point) const
{
	return fabs(Dot(point, true));
}

