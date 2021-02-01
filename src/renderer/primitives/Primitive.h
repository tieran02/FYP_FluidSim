#pragma once

#include <vector>
#include <renderer/Vertex.h>
#include "Mesh.h"

class Primitive
{
 public:
	Primitive();
	virtual ~Primitive();
	virtual void Build() = 0;

	Mesh& GetMesh();
 protected:
	Mesh m_mesh;
};
