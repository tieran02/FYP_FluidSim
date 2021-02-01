#pragma once

#include <vector>
#include <renderer/Vertex.h>
#include <glad/gl.h>

class Primitive
{
 public:
	Primitive();
	virtual ~Primitive();
	virtual void Build() = 0;
 private:
	std::vector<Vertex> m_vertices;
	GLuint m_vbo;
};
