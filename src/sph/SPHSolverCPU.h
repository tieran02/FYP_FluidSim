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
	virtual void pressureForces();
	void accumlatePressureForces(const std::vector<glm::vec3>& positions,const std::vector<float>& densities, std::vector<float>& pressures, const std::vector<glm::vec3>& forces);
	void resolveCollisions(std::vector<glm::vec3>& positions,std::vector<glm::vec3>& velocities);

	const BoxCollider BOX_COLLIDER;
	const size_t PARTICLE_COUNT;
	const float PARTICLE_RADIUS;
	const float KERNEL_RADIUS;
	ParticleSet m_particles;
	std::vector<std::vector<size_t>> m_neighborList;
	float m_mass{ 1.0f};
	float m_targetDensitiy{ 200.0f};
	float m_viscosityCoefficient = 0.0074f;
	float m_negativePressureScale = 0.0f;
 private:
	void BeginTimeStep() override;
	void ApplyForces() override;
	void Integrate() override;
	void ResolveCollisions() override;
	void EndTimeStep() override;

	void computeNeighborList();
	float sumOfKernelNearby(size_t pointIndex) const;
	void computeDensities();
	float computePressure(float density, float targetDensity, float eosScale, float eosExponent, float negativePressureScale) const;
	void viscosityForces();

	void fakeViscosity();

	ParticleState m_state;
	const glm::vec3 GRAVITY{0.0f,-9.81f,0.0f};
	const float PSEUDO_VISCOSITY_COEFFICIENT = 1.0f;
	const float SPEED_OF_SOUND{ 500.0f};
	KDTree<3> m_tree{ };
};
