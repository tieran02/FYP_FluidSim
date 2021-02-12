#include "SmoothedKernel.h"
#include "gtc/constants.hpp"

SmoothedKernel::SmoothedKernel(float kernelRadius)
{
	h = kernelRadius;
	h2 = h * h;
	h3 = h2 * h;
	h5 = h2 * h3;
}

float SmoothedKernel::Value(float distance) const
{
	if (distance * distance >= h2)
		return 0.0f;
	else
	{
		float x = 1.0f - distance * distance / h2;
		return 315.0f / (64.0f * glm::pi<float>() * h3) * x * x * x;
	}
}

float SmoothedKernel::FirstDerivative(float distance) const
{
	if(distance >= h)
	{
		return 0.0f;
	}
	else
	{
		float x = 1.0f - distance * distance / h2;
		return -945.0f / (32.0f * glm::pi<float>() * h5) * distance * x * x;
	}
}

float SmoothedKernel::SecondDerivative(float distance) const
{
	if (distance * distance >= h2)
		return 0.0f;
	else
	{
		float x = distance * distance / h2;
		return 945.0f / (32.0f *  glm::pi<float>() * h5) * (1 - x) * (5 * x - 1);
	}
}

glm::vec3 SmoothedKernel::Gradiant(float distance, const glm::vec3& direction) const
{
	return -FirstDerivative(distance) * direction;
}
