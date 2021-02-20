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
};
