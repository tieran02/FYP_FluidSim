#pragma once
#include <math/Plane.h>
#include "Primitive.h"
#include "glm.hpp"

class PlanePrimitive : public Primitive
{
 public:
	PlanePrimitive(const glm::vec2& Size);
	~PlanePrimitive() override;
	void Build() override;
 private:
	Plane m_plane;
	glm::vec2 m_size;
};
