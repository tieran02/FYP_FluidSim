#pragma once

#include "Solver.h"

class SPHSolverCPU : public Solver
{
 public:
	SPHSolverCPU(double timeStep);
 private:
	void BeginTimeStep() override;
	void ApplyForces() override;
	void Integrate() override;
	void ResolveCollisions() override;
	void EndTimeStep() override;
};
