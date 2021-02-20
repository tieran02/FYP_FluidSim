#include "PCISPHSolverCPU.h"

PCISPHSolverCPU::PCISPHSolverCPU(float timeStep, size_t particleCount, const BoxCollider& boxCollider) : SPHSolverCPU(
	timeStep,
	particleCount,
	boxCollider)
{

}
void PCISPHSolverCPU::pressureForces()
{
	for (int i = 0; i < m_particles.Size(); ++i)
	{
		//init vars

		//predict velocity and pos

		//resolve collisions

		//compute density error

		//pressure gradiant force

		//max densitiy error
		//float maxDensityError =
		float densitiyErrorRatio = m_maxErrorRatio / TargetDensitiy;

		if(fabs(densitiyErrorRatio) < m_maxErrorRatio)
		{
			break;
		}
	}

	//accumlate pressure force

}

