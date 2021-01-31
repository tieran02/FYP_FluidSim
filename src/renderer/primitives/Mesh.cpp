#include "Mesh.h"
#include <cassert>

Mesh::Mesh(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices)
{
	Build(std::move(vertices), std::move(indices));
}

void Mesh::Build(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices)
{
	assert(!m_VBO.Valid() || !m_EBO.Valid());

	m_vertices = std::move(vertices);
	m_indices = std::move(indices);

	m_VBO.Build(m_vertices.data(), m_vertices.size() * sizeof(Vertex));
	m_EBO.Build(m_indices.data(), m_indices.size() * sizeof(uint32_t));
}

void Mesh::Draw()
{
	m_VBO.Bind();
	m_EBO.Bind();

	glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);
}
