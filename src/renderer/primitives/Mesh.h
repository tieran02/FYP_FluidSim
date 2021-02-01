#pragma once

#include <renderer/Vertex.h>
#include <vector>
#include <NonCopyable.h>
#include <NonMovable.h>
#include <renderer/Buffer.h>

class Mesh : NonCopyable, NonMovable
{
 public:
	Mesh() = default;
	Mesh(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices);
	void Build(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices);
	void Draw() const;
 private:
	std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_indices;
	Buffer m_VBO{BufferType::VERTEX_BUFFER};
	Buffer m_EBO{BufferType::ELEMENT_BUFFER};
};
