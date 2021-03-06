#pragma once

#include <math/BoxCollider.h>
#include "Solver.h"
#include "ParticleSet.h"
#include "structures/KDTree.h"
#include "structures/SpartialHash.h"

class SPHSolverCPU : public Solver
{
public:
 public:
	SPHSolverCPU(float timeStep, size_t particleCount, const BoxCollider& boxCollider);
	virtual ~SPHSolverCPU() = default;

	void Setup(Scenario scenario) override;
	void Reset(Scenario scenario) override;
	const ParticleSet& Particles() const;
 protected:
	void BeginTimeStep() override;
	void ApplyForces() override;
	void Integrate() override;
	void ResolveCollisions() override;
	void EndTimeStep() override;

	virtual void computeDensities();
	virtual void pressureForces();
	void accumlatePressureForces(const std::vector<ParticlePoint>& positions,const std::vector<float>& densities, std::vector<float>& pressures, std::vector<ParticlePoint>& forces);
	void resolveCollisions(std::vector<ParticlePoint>& positions,std::vector<ParticlePoint>& velocities);
	void resolveCollision(const glm::vec3& startPos, glm::vec3& pos, glm::vec3& vel);
	virtual void computeNeighborList();


	const BoxCollider BOX_COLLIDER;
	const size_t PARTICLE_COUNT;
	const float PARTICLE_RADIUS;
	const float KERNEL_RADIUS;
	ParticleSet m_particles;
	std::vector<std::vector<uint32_t>> m_neighborList;
	float m_mass{ 1.0f};
	float m_targetDensitiy{ 200.0f};
	float m_viscosityCoefficient = 0.0074f;
	float m_negativePressureScale = 0.0f;
	KDTree<4> m_tree{ };
 private:
	float sumOfKernelNearby(size_t pointIndex) const;
	float computePressure(float density, float targetDensity, float eosScale, float eosExponent, float negativePressureScale) const;
	void viscosityForces();

	void fakeViscosity();

	void setupOneSidedParticles();
	void setupTwoSidedParticles();
	void setupFillParticles();

	ParticleState m_state;
	const glm::vec3 GRAVITY{0.0f,-9.81f,0.0f};
	const float PSEUDO_VISCOSITY_COEFFICIENT = 1.0f;
	const float SPEED_OF_SOUND{ 500.0f};
};
