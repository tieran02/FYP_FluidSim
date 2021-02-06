#pragma once

#include <math/PlaneCollider.h>
#include "Solver.h"
#include "ParticleSet.h"

class SPHSolverCPU : public Solver
{
 public:
	SPHSolverCPU(float timeStep, size_t particleCount, const PlaneCollider& CollisionPlane);
	const ParticleSet& Particles() const;
 private:
	void BeginTimeStep() override;
	void ApplyForces() override;
	void Integrate() override;
	void ResolveCollisions() override;
	void EndTimeStep() override;

	//For now just have one collision plane
	const PlaneCollider& m_collisionPlane;

	const size_t PARTICLE_COUNT;
	ParticleSet m_particles;
	const glm::vec3 GRAVITY{0.0f,-9.81f,0.0f};
};
