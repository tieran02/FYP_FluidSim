#include <util/Log.h>
#include "SPHSolverCPU.h"

SPHSolverCPU::SPHSolverCPU(double timeStep) : Solver(timeStep)
{

}

void SPHSolverCPU::BeginTimeStep()
{
	LOG_CORE_INFO("Begin Time Step");
}

void SPHSolverCPU::ApplyForces()
{

}

void SPHSolverCPU::Integrate()
{

}

void SPHSolverCPU::ResolveCollisions()
{

}

void SPHSolverCPU::EndTimeStep()
{

}
