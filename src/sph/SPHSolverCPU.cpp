#include <util/Log.h>
#include <Simulation.h>
#include "SPHSolverCPU.h"
#include <omp.h>
#include <iterator>
#include <iostream>
#include <random>
#include "math/SmoothedKernel.h"

SPHSolverCPU::SPHSolverCPU(float timeStep, size_t particleCount, const PlaneCollider& CollisionPlane) :
	Solver(timeStep),
	PARTICLE_COUNT(particleCount),
	m_particles(particleCount),
	m_neighborList(particleCount),
	m_collisionPlane(CollisionPlane),
	PARTICLE_RADIUS(0.1f),
	KERNEL_RADIUS(PARTICLE_RADIUS*4)
{

}

void SPHSolverCPU::Setup()
{
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<float> dist(0.25f, 1.25f);

	const int perRow = static_cast<int>(floor(sqrt(64)));
	constexpr float spacing = 0.8f;

	for (int i = 0; i < PARTICLE_COUNT; i++)
	{
		float x = ((i % perRow) * spacing) - (perRow / 2) + dist(mt);
		float y = ((0 * spacing) + (float)(((i / perRow) / perRow) * spacing) * 1.1f) + 10.0f;
		float z = (((i / perRow) % perRow) * spacing) - (perRow / 2) + dist(mt);
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
			newVelocity = m_particles.Velocities[i] + (TIMESTEP * m_particles.Forces[i]);

			//integrate position
			glm::vec3& newPosition = m_state.Positions[i];
			newPosition = m_particles.Positions[i] + (TIMESTEP * newVelocity);
		}
	}
	LOG_CORE_INFO("vel= {}", glm::to_string(m_particles.Velocities[0]));
}

void SPHSolverCPU::ResolveCollisions()
{
	constexpr float damping = 0.25f;
	#pragma omp parallel
	{
		#pragma omp for
		for (int i = 0; i < PARTICLE_COUNT; i++)
		{
			glm::vec3& pos = m_state.Positions[i];
			glm::vec3& vel = m_state.Velocities[i];

			CollisionData collisionData{};

			if (m_collisionPlane.CollisionOccured(pos, vel, collisionData))
			{
				vel = glm::reflect(vel,collisionData.CollisionNormal) * damping;;
				pos = collisionData.ContactPoint;

			}
		}
	}
}

void SPHSolverCPU::EndTimeStep()
{
	//move state into particles
	m_particles.Integrate(m_state);
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
		p[i] = computePressure(d[i], targetDensity, eosScale, eosExponent, 0.0f);
		//p[index] = 50.0f * (d[index] - 82.0f);
	}

	const float massSquared = MASS * MASS;
	SmoothedKernel kernal = SmoothedKernel(KERNEL_RADIUS); //TODO spiky kernel

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
