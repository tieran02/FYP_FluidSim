#include <util/Log.h>
#include <Simulation.h>
#include "SPHSolverCPU.h"
#include <omp.h>
#include <iterator>
#include <iostream>

SPHSolverCPU::SPHSolverCPU(float timeStep, size_t particleCount, const PlaneCollider& CollisionPlane) :
	Solver(timeStep),
	PARTICLE_COUNT(particleCount),
	m_particles(particleCount),
	m_collisionPlane(CollisionPlane)
{

}

void SPHSolverCPU::Setup()
{
	constexpr int perRow = 64;
	constexpr float spacing = 1.25f;

	for (int i = 0; i < PARTICLE_COUNT; i++)
	{
		float x = ((i % perRow) * spacing) - (perRow / 2);
		float y = ((0 * spacing) + (float)(((i / perRow) / perRow) * spacing) * 1.1f) + 10.0f;
		float z = (((i / perRow) % perRow) * spacing) - (perRow / 2);
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
