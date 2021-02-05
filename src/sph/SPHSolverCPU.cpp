#include <util/Log.h>
#include <Simulation.h>
#include "SPHSolverCPU.h"

SPHSolverCPU::SPHSolverCPU(float timeStep, size_t particleCount) : Solver(timeStep), PARTICLE_COUNT(particleCount), m_particles(particleCount)
{
	constexpr int perRow = 64;
	constexpr float spacing = 1.25f;

	for (int i = 0; i < PARTICLE_COUNT; i++)
	{
		float x = ((i % perRow) * spacing) - (perRow/2);
		float y = ((0 * spacing) + (float)(((i / perRow) / perRow) * spacing) * 1.1f) + 10.0f;
		float z = (((i /perRow) % perRow) * spacing) - (perRow/2);
		m_particles.Positions[i] = glm::vec3(x,y,z);
	}
}

void SPHSolverCPU::BeginTimeStep()
{

}

void SPHSolverCPU::ApplyForces()
{
	for (int i = 0; i < PARTICLE_COUNT; i++)
	{
		m_particles.Velocities[i] += (GRAVITY * TIMESTEP);
	}
}

void SPHSolverCPU::Integrate()
{
	for (int i = 0; i < PARTICLE_COUNT; i++)
	{
		m_particles.Positions[i] = m_particles.Positions[i] + TIMESTEP * m_particles.Velocities[i];
	}
}

void SPHSolverCPU::ResolveCollisions()
{

}

void SPHSolverCPU::EndTimeStep()
{

}

const ParticleSet& SPHSolverCPU::Particles() const
{
	return m_particles;
}
