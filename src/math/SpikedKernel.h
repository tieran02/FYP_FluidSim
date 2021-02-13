#pragma once
#include <glm.hpp>

struct SpikedKernel
{
 public:
	SpikedKernel(float kernelRadius);
	float Value(float distance) const;
	float FirstDerivative(float distance) const;
	float SecondDerivative(float distance) const;
	glm::vec3 Gradiant(float distance, const glm::vec3& direction) const;
 private:
	float h, h2, h3, h4, h5;
};
