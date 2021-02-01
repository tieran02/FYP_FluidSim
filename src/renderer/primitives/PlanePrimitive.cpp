#include "PlanePrimitive.h"

PlanePrimitive::PlanePrimitive(const glm::vec2& Size) : m_size(Size)
{
	//Top Left Point
	glm::vec3 TL = glm::vec3(0.0f - Size.x, 0.0f + Size.y, 0.0f);
	//Top Right Point
	glm::vec3 TR = glm::vec3(0.0f + Size.x, 0.0f + Size.y, 0.0f);
	//Bottom Left
	glm::vec3 BL = glm::vec3(0.0f - Size.x, 0.0f - Size.y, 0.0f);

	m_plane = Plane{TL,TR,BL};
}


PlanePrimitive::~PlanePrimitive()
{

}

void PlanePrimitive::Build()
{
	glm::vec3 TL = m_plane.A;
	glm::vec3 TR = m_plane.B;
	glm::vec3 BL = m_plane.C;
	glm::vec3 BR = glm::vec3(0.0f + m_size.x, 0.0f - m_size.y, 0.0f);

	glm::vec3 normal = m_plane.GetNormal();

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
