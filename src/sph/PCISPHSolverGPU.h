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
#include <optional>

class Buffer;

class PCISPHSolverGPU : public PCISPHSolverCPU
{
public:
	PCISPHSolverGPU(float timeStep, size_t particleCount, const BoxCollider& boxCollider, OpenCLContext& context,
		const Buffer& storagePostionBuffer, const Buffer& storagePressureBuffer);
	void Setup(Scenario scenario) override;
protected:
	void BeginTimeStep() override;
	void ApplyForces() override;
	void Integrate() override;
	void ResolveCollisions() override;
	void EndTimeStep() override;

	void computeNeighborList() override;
	void computeDensities() override;
 private:
	void createBuffers();
	void compileKernels() const;

	void resolveCollisions(cl::Buffer& positions, cl::Buffer& velocities);
	void copyIntoState();
	void copyToHost();
	void copyToGPU();
	
	OpenCLContext& m_context;
	int m_localWorkGroupSize;

	const Buffer& m_storagePostionBuffer;
	const Buffer& m_storagePressureBuffer;
	std::vector<cl::Memory> m_openGLBuffers;
	
	std::optional<cl::Buffer> m_positiionBuffer;
	std::optional<cl::Buffer> m_velocityBuffer;
	std::optional<cl::Buffer> m_forcesBuffer;
	std::optional<cl::Buffer> m_densitiyBuffer;
	std::optional<cl::Buffer> m_pressureBuffer;

	//state buffers
	std::optional<cl::Buffer> m_statePositionBuffer;
	std::optional<cl::Buffer> m_stateVelocityBuffer;

	std::optional<cl::Buffer> m_neighborBuffer;
	std::vector<uint32_t> hostNeighbors;

	std::optional<cl::Buffer> m_kernelSumBuffer;

	//PCI SPH buffers
	std::optional<cl::Buffer> m_densityErrorBuffer;
	std::optional<cl::Buffer> m_estimateDensityBuffer;
	std::optional<cl::Buffer> m_pressureForcesBuffer;
	std::optional<cl::Buffer> m_viscosityForcesBuffer;
	std::optional<cl::Buffer> m_tempPositionBuffer;
	std::optional<cl::Buffer> m_tempVelocityBuffer;

	//TEMP copy of position data for NN search
	std::vector<ParticlePoint> m_particlePoints;
};
