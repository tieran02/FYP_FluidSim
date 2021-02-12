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
	KERNEL_RADIUS(0.2f)
{

}

void SPHSolverCPU::Setup()
{
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<float> dist(0.25f, 1.25f);

	const int perRow = static_cast<int>(floor(sqrt(64)));
	constexpr float spacing = 1.25f;

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
}

void SPHSolverCPU::ResolveCollisions()
{
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
				pos = collisionData.ContactPoint;
				vel = glm::vec3(0, 0, 0);
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
	const auto& p = m_particles.Pressures;
	const auto& f = m_particles.Forces;

}
