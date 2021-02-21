#pragma once

#include "SPHSolverCPU.h"

class PCISPHSolverCPU : public SPHSolverCPU
{
 public:
	PCISPHSolverCPU(float timeStep, size_t particleCount, const BoxCollider& boxCollider);
	~PCISPHSolverCPU() override = default;

 protected:
	void pressureForces() override;

 private:
	float m_maxErrorRatio{0.01f};
	uint32_t m_maxItterations{5};

	std::vector<glm::vec3> m_tempPositions;
	std::vector<glm::vec3> m_tempVelocities;
	std::vector<glm::vec3> m_pressureForces;

	float computeDeltaPressure();

	/// Fill an AABB box of particles where the size is 1.5 * Kernel radius and the spacing is equal to the target spacing
	/// \param boundingBox bounding AABB that will be used to contain all the points
	/// \param spacing target spacing between each particle
	/// \param points a reference of a vector or points that will contain the spread out points.
	void fillAABBWithSpacing(const AABB& boundingBox, float spacing, std::vector<glm::vec3>& points);
};
