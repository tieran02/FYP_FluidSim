#pragma once
#include <math/Plane.h>
#include "Primitive.h"
#include "glm.hpp"

class QuadPrimitive : public Primitive
{
 public:
	QuadPrimitive(const glm::vec2& Size);
	~QuadPrimitive() override;
	void Build() override;

	//Calculate and return the plane in local space
	Plane GetPlane() const;
 private:
	glm::vec2 m_size;
};
