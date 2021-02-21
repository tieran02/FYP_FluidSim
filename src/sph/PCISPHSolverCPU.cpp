#include <math/SpikedKernel.h>
#include "PCISPHSolverCPU.h"

PCISPHSolverCPU::PCISPHSolverCPU(float timeStep, size_t particleCount, const BoxCollider& boxCollider) : SPHSolverCPU(
	timeStep,
	particleCount,
	boxCollider),
	m_tempPositions(particleCount),
	m_tempVelocities(particleCount),
	m_pressureForces(particleCount)
{

}

void PCISPHSolverCPU::pressureForces()
{
	//init vars
	#pragma omp parallel for
	for (int i = 0; i < m_particles.Size(); ++i) //Maybe we could just do a mem copy?
	{
		m_pressureForces[i] = glm::vec3(0.0f);
	}

	for (int i = 0; i < m_maxItterations; ++i)
	{
		//predict velocity and pos
		#pragma omp parallel for
		for (int i = 0; i < m_particles.Size(); ++i) //Maybe we could just do a mem copy?
		{
			m_tempVelocities[i] = m_particles.Velocities[i]
				+ TIMESTEP / MASS
				+ (m_particles.Forces[i] + m_pressureForces[i]);

			m_tempPositions[i] = m_particles.Positions[i] + TIMESTEP * m_tempVelocities[i];
		}

		//resolve collisions
		resolveCollisions(m_tempPositions,m_tempVelocities);

		//compute density error
		float delta = computeDeltaPressure();

		//pressure gradiant force

		//max densitiy error
		//float maxDensityError =
		float densitiyErrorRatio = m_maxErrorRatio / TargetDensitiy;

		if(fabs(densitiyErrorRatio) < m_maxErrorRatio)
		{
			break;
		}
	}

	//accumlate pressure force

}

float PCISPHSolverCPU::computeDeltaPressure()
{
	const float targetSpacing = 0.6f;
	glm::vec3 originPoint(0.0f);

	//TODO fill an AABB box of particles where the size is 1.5 * Kernel radius and the spacing is equal to the target spacing
	AABB box(originPoint,originPoint);
	box.Expand(1.5f * KERNEL_RADIUS);
	std::vector<glm::vec3> points;
	fillAABBWithSpacing(box,targetSpacing, points);

	SpikedKernel kernel(KERNEL_RADIUS);

	float denom{0.0f};
	glm::vec3 denom1{0.0f};
	float denom2{0.0f};

	for (size_t i = 0; i < points.size(); ++i)
	{
		const glm::vec3& point = points[i];
		float distanceSquared = glm::length2(point);

		if(distanceSquared < KERNEL_RADIUS * KERNEL_RADIUS)
		{
			float distance = sqrtf(distanceSquared);
			glm::vec3 dir = (distance > 0.0f) ? point / distance : glm::vec3(0.0f);

			// gradiant of Wij
			glm::vec3 gradiant = kernel.Gradiant(distance,dir);
			denom1 += gradiant;
			denom2 += glm::dot(gradiant,gradiant);
		}
	}

	denom += glm::dot(-denom1,denom1) - denom2;

	if(fabs(denom) > std::numeric_limits<float>::epsilon())
	{
		float integratedDensity2 = (MASS * TIMESTEP / TargetDensitiy) * (MASS * TIMESTEP / TargetDensitiy);
		float beta = 2.0f * integratedDensity2;

		return -1.0f / (beta * denom);
	}
	return 0.0f;
}

void PCISPHSolverCPU::fillAABBWithSpacing(const AABB& boundingBox, float spacing, std::vector<glm::vec3>& points)
{
	const float halfSpace = spacing / 2.0f;
	const float width = boundingBox.Width();
	const float height = boundingBox.Height();
	const float depth = boundingBox.Depth();

	glm::vec3 position;
	bool hasOffset = false;
	for (int z = 0; z * halfSpace <= depth; ++z)
	{
		position.z = z * halfSpace + boundingBox.Min().z;

		float offset = (hasOffset) ? halfSpace : 0.0f;

		for (int y = 0; y * spacing + offset <= height; ++y)
		{
			position.y = y * spacing + offset + boundingBox.Min().y;

			for (int x = 0; x * spacing + offset <= width; ++x)
			{
				position.x = x * spacing + offset + boundingBox.Min().x;
				points.push_back(position);
			}
		}

		hasOffset = !hasOffset;
	}
}

