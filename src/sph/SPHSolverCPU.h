#pragma once

#include "Solver.h"
#include "ParticleSet.h"

class SPHSolverCPU : public Solver
{
 public:
	SPHSolverCPU(double timeStep, size_t particleCount);
	const ParticleSet& Particles() const;
 private:
	void BeginTimeStep() override;
	void ApplyForces() override;
	void Integrate() override;
	void ResolveCollisions() override;
	void EndTimeStep() override;

	const size_t PARTICLE_COUNT;
	ParticleSet m_particles;
};
