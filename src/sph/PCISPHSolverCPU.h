#pragma once

#include "SPHSolverCPU.h"

class PCISPHSolverCPU : public SPHSolverCPU
{
 public:
	PCISPHSolverCPU(float timeStep, size_t particleCount, const BoxCollider& boxCollider);
	~PCISPHSolverCPU() override = default;
 protected:
	void pressureForces() override;
	uint32_t m_maxItterations{ 5 };
 private:
	float m_maxErrorRatio{0.1f};
	float deltaDensitity;

	std::vector<ParticlePoint> m_tempPositions;
	std::vector<ParticlePoint> m_tempVelocities;
	std::vector<ParticlePoint> m_pressureForces;
	std::vector<float> m_densitiyErrors;

	float computeDeltaPressure();

	/// Fill an AABB box of particles where the size is 1.5 * Kernel radius and the spacing is equal to the target spacing
	/// \param boundingBox bounding AABB that will be used to contain all the points
	/// \param spacing target spacing between each particle
	/// \param points a reference of a vector or points that will contain the spread out points.
	void fillAABBWithSpacing(const AABB& boundingBox, float spacing, std::vector<glm::vec3>& points);
};
