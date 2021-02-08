#include "QuadPrimitive.h"

QuadPrimitive::QuadPrimitive(const glm::vec2& Size) : m_size(Size)
{

}


QuadPrimitive::~QuadPrimitive()
{

}

void QuadPrimitive::Build()
{
	glm::vec3 TL = glm::vec3(0.0f - m_size.x, 0.0f + m_size.y, 0.0f);
	glm::vec3 TR = glm::vec3(0.0f + m_size.x, 0.0f + m_size.y, 0.0f);
	glm::vec3 BL = glm::vec3(0.0f - m_size.x, 0.0f - m_size.y, 0.0f);
	glm::vec3 BR = glm::vec3(0.0f + m_size.x, 0.0f - m_size.y, 0.0f);

	Plane plane{TL,TR,BL};
	glm::vec3 normal = plane.GetNormal();

	std::vector<Vertex> verts =
	{
		{TL,normal,glm::vec2(0.0f,1.0f)}, //Top Left
		{TR,normal,glm::vec2(1.0f,1.0f)}, //Top Right
		{BR,normal,glm::vec2(1.0f,0.0f)}, //Bottom Right
		{BL,normal,glm::vec2(0.0f,0.0f)}  //Bottom Left
	};

	std::vector<uint32_t> indices = {
		0,2,3,
		0,1,2
	};

	m_mesh.Build(std::move(verts), std::move(indices));
}

Plane QuadPrimitive::GetPlane() const
{
	glm::vec3 TL = glm::vec3(0.0f - m_size.x, 0.0f + m_size.y, 0.0f);
	glm::vec3 TR = glm::vec3(0.0f + m_size.x, 0.0f + m_size.y, 0.0f);
	glm::vec3 BL = glm::vec3(0.0f - m_size.x, 0.0f - m_size.y, 0.0f);
	glm::vec3 BR = glm::vec3(0.0f + m_size.x, 0.0f - m_size.y, 0.0f);

	return Plane{TL,TR,BL};
}
