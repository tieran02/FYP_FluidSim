#include <util/Log.h>
#include <Simulation.h>
#include "SPHSolverCPU.h"
#include <omp.h>
#include <iterator>
#include <iostream>
#include <random>
#include "math/SmoothedKernel.h"
#include "math/SpikedKernel.h"

SPHSolverCPU::SPHSolverCPU(float timeStep, size_t particleCount, const BoxCollider& boxCollider) :
	Solver(timeStep),
	PARTICLE_COUNT(particleCount),
	m_particles(particleCount),
	m_neighborList(particleCount),
	PARTICLE_RADIUS(0.1f),
	KERNEL_RADIUS(PARTICLE_RADIUS*4),
	m_boxCollider(boxCollider)
{

}

void SPHSolverCPU::Setup()
{
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<float> dist(0.25f, 1.25f);

	const int perRow = (m_boxCollider.GetAABB().Max().x * 2) / KERNEL_RADIUS;
	constexpr float spacing = 0.25f;

	for (int i = 0; i < PARTICLE_COUNT; i++)
	{
		float x = ((i % perRow) * spacing) - (perRow / 2) + dist(mt);
		float y = ((0 * spacing) + (float)(((i / perRow) / perRow) * spacing) * 0.5f);
		float z = (((i / perRow) % perRow) * spacing) - (perRow / 2) + dist(mt);

		x+= -m_boxCollider.GetAABB().Max().x + perRow*0.5f;
		z+= -m_boxCollider.GetAABB().Max().z + perRow*0.5f;
		y+= m_boxCollider.GetAABB().Min().y;
		m_particles.Positions[i] = glm::vec3(x, y, z);
	}
}

void SPHSolverCPU::Reset()
{
	m_particles.Reset();
	Setup();
}

void SPHSolverCPU::BeginTimeStep()
{
	m_state = ParticleState(m_particles);
	m_tree.Build(m_particles.Positions);
	//LOG_CORE_INFO(glm::to_string(m_state.Positions[0]));

	computeNeighborList();

	computeDensities();
}

void SPHSolverCPU::ApplyForces()
{
	#pragma omp parallel
	{
		#pragma omp for
		for (int i = 0; i < m_particles.Size(); ++i)
			m_particles.Forces[i] = GRAVITY;
	}

	viscosityForces();
	pressureForces();
}

void SPHSolverCPU::Integrate()
{
	#pragma omp parallel
	{
		#pragma omp for
		for (int i = 0; i < PARTICLE_COUNT; i++)
		{
			//integrate velocity
			glm::vec3& newVelocity = m_state.Velocities[i];
			newVelocity = m_particles.Velocities[i] + TIMESTEP * m_particles.Forces[i];

			//integrate position
			glm::vec3& newPosition = m_state.Positions[i];
			newPosition = m_particles.Positions[i] + TIMESTEP * newVelocity;
		}
	}
}

void SPHSolverCPU::ResolveCollisions()
{
	resolveCollisions(m_state.Positions, m_state.Velocities);
}

void SPHSolverCPU::EndTimeStep()
{
	//move state into particles
	m_particles.Integrate(m_state);

	//fakeViscosity();
}

const ParticleSet& SPHSolverCPU::Particles() const
{
	return m_particles;
}

void SPHSolverCPU::computeNeighborList()
{
	//find all neighbours of all elements
	#pragma omp parallel for
	for (int i = 0; i < m_particles.Positions.size(); ++i)
	{
		std::vector<size_t> e;
		e.reserve(1000);
		bool foundE = m_tree.FindNearestNeighbors(m_particles.Positions[i], KERNEL_RADIUS*KERNEL_RADIUS, e);
		m_neighborList[i] = e;
	}
}

void SPHSolverCPU::computeDensities()
{
	const auto& p = m_particles.Positions;
	auto& d = m_particles.Densities;

	#pragma omp parallel for
	for (int i = 0; i < m_particles.Positions.size(); ++i)
	{
		float sum = sumOfKernelNearby(i);
		d[i] = MASS * sum;
	}
}

float SPHSolverCPU::sumOfKernelNearby(size_t pointIndex) const
{
	float sum = 0.0f;
	SmoothedKernel kernal = SmoothedKernel(KERNEL_RADIUS);

	for(const auto& neighbor : m_neighborList[pointIndex])
	{
		if(neighbor == pointIndex)
			continue;

		float dist = glm::distance(m_particles.Positions[pointIndex], m_particles.Positions[neighbor]); //TODO distance2
		float weight = kernal.Value(dist);
		sum += weight;
	}

	return sum;
}

void SPHSolverCPU::pressureForces()
{

	const auto& x = m_particles.Positions;
	const auto& d = m_particles.Densities;
	auto& p = m_particles.Pressures;
	const auto& f = m_particles.Forces;

	float targetDensity = TargetDensitiy;
	float eosScale = targetDensity * (speedOfSound * speedOfSound);
	constexpr float eosExponent = 7.0f;

	#pragma omp parallel for
	for (int i = 0; i < m_particles.Pressures.size(); ++i)
	{
		p[i] = computePressure(d[i], targetDensity, eosScale, eosExponent, negativePressureScale);
		//p[index] = 50.0f * (d[index] - 82.0f);
	}


	accumlatePressureForces(x,d,p,f);
}

void SPHSolverCPU::accumlatePressureForces(const std::vector<glm::vec3>& positions,const std::vector<float>& densities, std::vector<float>& pressures, const std::vector<glm::vec3>& forces)
{
	const float massSquared = MASS * MASS;
	SpikedKernel kernal = SpikedKernel(KERNEL_RADIUS);

	//now accumlate pressure
	#pragma omp parallel for
	for (int i = 0; i < m_particles.Pressures.size(); ++i)
	{
		for(const auto& neighbor : m_neighborList[i])
		{
			if(neighbor == i)
				continue;

			float dist = glm::distance(m_particles.Positions[i], m_particles.Positions[neighbor]); //TODO distance2
			if(dist > 0.0f)
			{
				glm::vec3 direction = (m_particles.Positions[neighbor] - m_particles.Positions[i]) / dist;
				glm::vec3 force = massSquared
					* (m_particles.Pressures[i] / (m_particles.Densities[i] * m_particles.Densities[i])
						+ m_particles.Pressures[neighbor] / (m_particles.Densities[neighbor] * m_particles.Densities[neighbor]))
					* kernal.Gradiant(dist, direction);
				m_particles.Forces[i] -= force;
			}
		}
	}
}

float SPHSolverCPU::computePressure(float density,
	float targetDensity,
	float eosScale,
	float eosExponent,
	float negativePressureScale) const
{
	float p = eosScale / eosExponent
		* (pow((density / targetDensity), eosExponent) - 1.0f);

	//float p = 10.0f * (density - targetDensity);

	//negative pressue scaling
	if (p < 0)
		p *= negativePressureScale;
	return p;
}

void SPHSolverCPU::viscosityForces()
{
	const auto& x = m_particles.Positions;
	const auto& d = m_particles.Densities;
	const auto& v = m_particles.Velocities;
	auto& f = m_particles.Forces;

	float massSquared = MASS * MASS;
	SpikedKernel kernal = SpikedKernel(KERNEL_RADIUS);

	#pragma omp parallel for
	for (int i = 0; i < m_particles.Forces.size(); ++i)
	{
		for(const auto& neighbor : m_neighborList[i])
		{
			if(neighbor == i)
				continue;

			float dist = glm::distance(m_particles.Positions[i], m_particles.Positions[neighbor]); //TODO distance2

			f[i] += viscosityCoefficient * massSquared
				* (v[neighbor] - v[i]) / d[neighbor]
				* kernal.SecondDerivative(dist);
		}
	}
}


void SPHSolverCPU::resolveCollisions(std::vector<glm::vec3>& positions, std::vector<glm::vec3>& velocities)
{
	constexpr float damping = 0.2f;
	constexpr float RestitutionCoefficient = 0.2f;
	constexpr float frictionCoeffient = std::numeric_limits<float>::epsilon();

	#pragma omp parallel
	{
	#pragma omp for
		for (int i = 0; i < PARTICLE_COUNT; i++)
		{
			glm::vec3& pos = positions[i];
			glm::vec3& vel = velocities[i];

			CollisionData collisionData{};

			if(m_boxCollider.GetAABB().IsPointOutside(pos))
			{
				if (m_boxCollider.CollisionOccured(pos, -vel, collisionData))
				{
//					pos = collisionData.ContactPoint;
//					vel = glm::reflect(vel, collisionData.CollisionNormal) * damping;

					glm::vec3 targetNormal = collisionData.CollisionNormal;

					float normalDotRelativeVelocity = glm::dot(targetNormal, vel);
					glm::vec3 relativeVelocityNormal = normalDotRelativeVelocity * targetNormal;
					glm::vec3 relativeVelocityT = vel - relativeVelocityNormal;

					// Check if the velocity is facing opposite direction of the surface
					// normal
					if (normalDotRelativeVelocity < 0.0)
					{
						//Apply restitution coefficient to the surface normal component of the velocity
						glm::vec3 deltaRelativeVelocityNormal = (-RestitutionCoefficient - 1.0f) * relativeVelocityNormal;
						relativeVelocityNormal *= -RestitutionCoefficient;

						// Apply friction to the tangential component of the velocity
						if (glm::length2(relativeVelocityT) > 0.0f)
						{

							float frictionScale = std::max(1.0f - frictionCoeffient *
								glm::length(deltaRelativeVelocityNormal) /  glm::length(relativeVelocityT), 0.0f);
							relativeVelocityT *= frictionScale;
						}

						// Reassemble the components
						vel = relativeVelocityNormal + relativeVelocityT;
						vel *= damping;
					}
					pos = collisionData.ContactPoint;
				}
			}
		}
	}
}

glm::vec3 lerp(glm::vec3 x, glm::vec3 y, float t) {
	return x * (1.f - t) + y * t;
}


void SPHSolverCPU::fakeViscosity()
{
	const auto& x = m_particles.Positions;
	const auto& d = m_particles.Densities;
	auto& v = m_particles.Velocities;

	SpikedKernel kernel = SpikedKernel(KERNEL_RADIUS);
	std::vector<glm::vec3> smoothedVelocities(m_particles.Size());

	#pragma omp parallel for
	for (int i = 0; i < m_particles.Size(); ++i)
	{
		float weightSum = 0.0f;
		glm::vec3 smoothedVelocity = glm::vec3(0.0f);

		for(const auto& neighbor : m_neighborList[i])
		{
			float distance = glm::distance(x[i], x[neighbor]);
			float wj = MASS / d[neighbor] * kernel.Value(distance);
			weightSum += wj;
			smoothedVelocity += wj * v[neighbor];
		}

		float wi = MASS / d[i];
		weightSum += wi;
		smoothedVelocity += wi * v[i];

		if (weightSum > 0.0)
		{
			smoothedVelocity /= weightSum;
		}

		smoothedVelocities[i] = smoothedVelocity;
	}

	float factor = TIMESTEP * pseudoViscosityCoefficient;
	factor = std::max(0.0f, std::min(factor, 1.0f));
	#pragma omp parallel for
	for (int i = 0; i < m_particles.Size(); ++i)
	{
		v[1] = lerp(v[i],smoothedVelocities[i], factor);
	}
}

