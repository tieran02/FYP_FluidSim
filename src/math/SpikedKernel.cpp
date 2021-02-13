#include "SpikedKernel.h"
#include "gtc/constants.hpp"

SpikedKernel::SpikedKernel(float kernelRadius)
{
	h = kernelRadius;
	h2 = h * h;
	h3 = h2 * h;
	h4 = h3 * h;
	h5 = h4 * h;
}

float SpikedKernel::Value(float distance) const
{
	if (distance >= h)
		return 0.0f;
	else
	{
		float x = 1.0f - distance / h;
		return 15.0f / (glm::pi<float>() * h3) * x * x * x;
	}
}

float SpikedKernel::FirstDerivative(float distance) const
{
	if (distance >= h)
	{
		return 0.0f;
	}
	else
	{
		float x = 1.0f - distance  / h;
		return -45.0f / (glm::pi<float>() * h4)  * x * x;
	}
}

float SpikedKernel::SecondDerivative(float distance) const
{
	if (distance >= h)
		return 0.0f;
	else
	{
		float x = 1.0f - distance / h;
		return 90.0f / (glm::pi<float>() * h5) * x;
	}
}

glm::vec3 SpikedKernel::Gradiant(float distance, const glm::vec3& direction) const
{
	return -FirstDerivative(distance) * direction;
}
