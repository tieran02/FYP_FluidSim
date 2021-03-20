#include <math/SpikedKernel.h>
#include "PCISPHSolverCPU.h"

PCISPHSolverCPU::PCISPHSolverCPU(float timeStep, size_t particleCount, const BoxCollider& boxCollider) : SPHSolverCPU(
	timeStep,
	particleCount,
	boxCollider),
	m_tempPositions(particleCount),
	m_tempVelocities(particleCount),
	m_pressureForces(particleCount),
	m_densitiyErrors(particleCount)
{
	m_mass = 2.0f;
	m_targetDensitiy = 1000.0f;
	m_viscosityCoefficient = 0.01f;
	deltaDensitity = computeDeltaPressure();
}

void PCISPHSolverCPU::pressureForces()
{
	auto& p = m_particles.Pressures;
	auto& d = m_particles.Densities;
	auto& x = m_particles.Positions;
	auto& v = m_particles.Velocities;
	auto& f = m_particles.Forces;

	//estimated density
	std::vector<float> ds(PARTICLE_COUNT, 0.0f);
	SpikedKernel kernel(KERNEL_RADIUS);

	//init vars
	#pragma omp parallel for
	for (int i = 0; i < m_particles.Size(); ++i) //Maybe we could just do a mem copy?
	{
		p[i] = 0.0f;
		m_pressureForces[i].vec = glm::vec3(0.0f);
		m_densitiyErrors[i] = 0.0f;
		ds[i] = d[i];
	}

	unsigned int maxNumIter = 0;
	float maxDensityError;
	float densityErrorRatio = 0.0f;
	for (int i = 0; i < m_maxItterations; ++i)
	{
		//predict velocity and pos
		#pragma omp parallel for
		for (int i = 0; i < m_particles.Size(); ++i) //Maybe we could just do a mem copy?
		{
			m_tempVelocities[i].vec
				= v[i].vec
				+ TIMESTEP / m_mass
				* (f[i].vec + m_pressureForces[i].vec);

			auto newPos = x[i].vec + TIMESTEP * m_tempVelocities[i].vec;

			//resolveCollision(m_tempPositions[i],newPos,m_tempVelocities[i]);

			m_tempPositions[i].vec = newPos;
		}

		//resolve collisions
		resolveCollisions(m_tempPositions,m_tempVelocities);

		//calculate pressure from densitiy error
		#pragma omp parallel for
		for (int i = 0; i < m_particles.Size(); ++i)
		{
			float weightSum = 0.0f;

			for(const auto& neighbor : m_neighborList[i])
			{
				float dist = glm::distance(m_tempPositions[neighbor].vec, m_tempPositions[i].vec);
				weightSum += kernel.Value(dist);
			}
			weightSum += kernel.Value(0.0f);

			float density = m_mass * weightSum;
			float densityError = (density - m_targetDensitiy);
			float pressure = deltaDensitity * densityError;

			if(pressure < 0.0f)
			{
				pressure *= m_negativePressureScale;
				densityError *= m_negativePressureScale;
			}

			p[i] += pressure;
			ds[i] = density;
			m_densitiyErrors[i] = densityError;
		}

		//pressure gradiant force
		std::fill(m_pressureForces.begin(), m_pressureForces.end(), ParticlePoint());
		SPHSolverCPU::accumlatePressureForces(x,ds,p,m_pressureForces);

		//max densitiy error
		maxDensityError = 0.0f;
		for (int j = 0; j < PARTICLE_COUNT; ++j)
		{
			maxDensityError = std::max(maxDensityError, fabs(m_densitiyErrors[i]));
		}

		densityErrorRatio = maxDensityError / m_targetDensitiy;
		maxNumIter = i + 1;

		//float maxDensityError =
		float densitiyErrorRatio = m_maxErrorRatio / m_targetDensitiy;

		if(fabs(densitiyErrorRatio) < m_maxErrorRatio)
		{
			break;
		}
	}

	//accumlate pressure force
	#pragma omp parallel for
	for (int i = 0; i < m_particles.Size(); ++i)
	{
		f[i].vec += m_pressureForces[i].vec;
	}

}

float PCISPHSolverCPU::computeDeltaPressure()
{
	const float targetSpacing = 0.08f;
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
		float integratedDensity2 = (m_mass * TIMESTEP / m_targetDensitiy) * (m_mass * TIMESTEP / m_targetDensitiy);
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

