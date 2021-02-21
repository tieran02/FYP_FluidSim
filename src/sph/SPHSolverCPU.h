#pragma once

#include <math/PlaneCollider.h>
#include <math/BoxCollider.h>
#include "Solver.h"
#include "ParticleSet.h"
#include "structures/KDTree.h"
#include "structures/SpartialHash.h"

class SPHSolverCPU : public Solver
{
 public:
	SPHSolverCPU(float timeStep, size_t particleCount, const BoxCollider& boxCollider);
	virtual ~SPHSolverCPU() = default;

	void Setup() override;
	void Reset() override;
	const ParticleSet& Particles() const;
 protected:
	void BeginTimeStep() override;
	void ApplyForces() override;
	void Integrate() override;
	void ResolveCollisions() override;
	void EndTimeStep() override;

	void computeNeighborList();
	float sumOfKernelNearby(size_t pointIndex) const;
	void computeDensities();
	virtual void pressureForces();
	void accumlatePressureForces(const std::vector<glm::vec3>& positions,const std::vector<float>& densities, std::vector<float>& pressures, const std::vector<glm::vec3>& forces);
	float computePressure(float density, float targetDensity, float eosScale, float eosExponent, float negativePressureScale) const;

	void viscosityForces();

	void resolveCollisions(std::vector<glm::vec3>& positions,std::vector<glm::vec3>& velocities);

	const BoxCollider m_boxCollider;

	const size_t PARTICLE_COUNT;
	const float PARTICLE_RADIUS;
	const float KERNEL_RADIUS;
	ParticleSet m_particles;
	ParticleState m_state;
	std::vector<std::vector<size_t>> m_neighborList;

	const glm::vec3 GRAVITY{0.0f,-9.81f,0.0f};
	const float MASS{0.4f};
	const float TargetDensitiy{1000.0f};
	const float speedOfSound{1000.0f};
	const float viscosityCoefficient = 0.5f;
	const float pseudoViscosityCoefficient = 0.25f;
	const float negativePressureScale = 0.0f;

	KDTree<3> m_tree{ };

 private:
	void fakeViscosity();
};
