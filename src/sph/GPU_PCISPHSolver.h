#pragma once
#define __CL_ENABLE_EXCEPTIONS
#define CL_TARGET_OPENCL_VERSION 300
#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif

#include "PCISPHSolverCPU.h"
#include "opencl/OpenCLContext.h"

class GPU_PCISPHSolver : public PCISPHSolverCPU
{
public:
	GPU_PCISPHSolver(float timeStep, size_t particleCount, const BoxCollider& boxCollider, OpenCLContext& context);
protected:
	void BeginTimeStep() override;
private:
	void createBuffers();
	
	OpenCLContext& m_context;
	cl::Buffer m_positiionBuffer;
	cl::Buffer m_velocityBuffer;
	cl::Buffer m_forcesBuffer;
	cl::Buffer m_densitiyBuffer;
	cl::Buffer m_pressureBuffer;
};
