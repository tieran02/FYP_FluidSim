#include "Mesh.h"
#include <cassert>
#include <util/Log.h>

Mesh::Mesh(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices)
{
	Build(std::move(vertices), std::move(indices));
}

void Mesh::Build(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices)
{
	CORE_ASSERT(!m_VBO.Valid() && !m_EBO.Valid(), "Mesh already has a VBO/EBO");

	m_vertices = std::move(vertices);
	m_indices = std::move(indices);

	m_VBO.Build(m_vertices.data(), m_vertices.size() * sizeof(Vertex));
	m_EBO.Build(m_indices.data(), m_indices.size() * sizeof(uint32_t));
}

void Mesh::Draw(const Transform& transform, const Shader& shader) const
{
	m_VBO.Bind();
	m_EBO.Bind();

	//NOTE: the model matrix is calculated each draw call, if the model doesn't change it still needs to be calculated
	//TODO: cache a copy of the model matrix in Transfrom and update it only when it changes
	glm::mat4 modelMatrix = transform.ModelMatrix();
	shader.SetMat4("model", modelMatrix,false);

	glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);

	m_VBO.Unbind();
	m_EBO.Unbind();

}

void Mesh::Draw() const
{
	m_VBO.Bind();
	m_EBO.Bind();
	
	glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);
	
	m_VBO.Unbind();
	m_EBO.Unbind();
}

const Buffer& Mesh::VBO() const
{
	return m_VBO;
}

const Buffer& Mesh::EBO() const
{
	return m_EBO;
}

const std::vector<Vertex>& Mesh::Vertices() const
{
	return m_vertices;
}

const std::vector<uint32_t>& Mesh::Indices() const
{
	return m_indices;
}
