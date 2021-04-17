#pragma once

#include <renderer/Vertex.h>
#include <vector>
#include <NonCopyable.h>
#include <NonMovable.h>
#include <renderer/Buffer.h>
#include <math/Transform.h>
#include <renderer/Shader.h>

class Mesh : NonCopyable, NonMovable
{
 public:
	Mesh() = default;
	Mesh(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices);
	void Build(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices);
	void Draw(const Transform& transform, const Shader& shader) const;
	void Draw() const;

	const Buffer& VBO() const;
	const Buffer& EBO() const;

	const std::vector<Vertex>& Vertices() const;
	const std::vector<uint32_t>& Indices() const;
 private:
	std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_indices;
	Buffer m_VBO{BufferType::VERTEX_BUFFER};
	Buffer m_EBO{BufferType::ELEMENT_BUFFER};
};
