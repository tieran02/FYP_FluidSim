#include "PCISPHSolverGPU.h"

#include "util/Log.h"
#include "util/Util.h"

//Due to memory restriction on the GPU need to limit the number of max neighbours for a given point known as K
constexpr size_t K = 256;

PCISPHSolverGPU::PCISPHSolverGPU(float timeStep, size_t particleCount, const BoxCollider& boxCollider, OpenCLContext& context) :
	PCISPHSolverCPU(timeStep,particleCount,boxCollider), 
	m_context(context),
	m_particlePoints(particleCount)
{
	compileKernels();
	createBuffers();
}

void PCISPHSolverGPU::BeginTimeStep()
{
	//copy over positions into state
	m_context.Queue().enqueueCopyBuffer(m_positiionBuffer,m_statePositionBuffer,0,0,sizeof(ParticlePoint) * m_particles.Size());
	m_context.Queue().enqueueCopyBuffer(m_velocityBuffer,m_stateVelocityBuffer,0,0,sizeof(ParticlePoint) * m_particles.Size());

	computeNeighborList();

    computeDensities();
}

void PCISPHSolverGPU::ApplyForces()
{
	//check if the OpenCL context has the Brute NN search program
	OpenCLProgram* program = m_context.GetProgram("sph");
	CORE_ASSERT(program, "Failed to find OpenCLProgram for sph (Make sure its compiled)");
	cl::Kernel* nonPressureForcesKernel = program->GetKernel("ApplyNonPressureForces");
	CORE_ASSERT(nonPressureForcesKernel, "Failed to find OpenCL Kernel ('ApplyNonPressureForces') for sph (Make sure its compiled)");
	cl::Kernel* pressureForcesKernel = program->GetKernel("ApplyPressureForces");
	CORE_ASSERT(pressureForcesKernel, "Failed to find OpenCL Kernel ('ApplyPressureForces') for sph (Make sure its compiled)");

	try
	{
		nonPressureForcesKernel->setArg(0, m_neighborBuffer);
		nonPressureForcesKernel->setArg(1, m_positiionBuffer);
		nonPressureForcesKernel->setArg(2, m_velocityBuffer);
		nonPressureForcesKernel->setArg(3, m_densitiyBuffer);
		nonPressureForcesKernel->setArg(4, m_forcesBuffer);

		//reset kernel sums
		m_context.Queue().enqueueNDRangeKernel(*nonPressureForcesKernel,
			0,
			cl::NDRange(m_particles.Size()),
			cl::NDRange(1));

		pressureForcesKernel->setArg(0, m_neighborBuffer);
		pressureForcesKernel->setArg(1, m_positiionBuffer);
		pressureForcesKernel->setArg(2, m_velocityBuffer);
		pressureForcesKernel->setArg(3, m_densitiyBuffer);
		pressureForcesKernel->setArg(4, m_forcesBuffer);
		pressureForcesKernel->setArg(5, m_pressureBuffer);
		pressureForcesKernel->setArg(6, m_densityErrorBuffer);
		pressureForcesKernel->setArg(7, m_estimateDensityBuffer);

		m_context.Queue().enqueueFillBuffer(m_densityErrorBuffer, 0.0f, 0, sizeof(cl_float) * m_particles.Size());
		
		for (size_t i = 0; i < m_maxItterations; i++)
		{
			m_context.Queue().enqueueNDRangeKernel(*pressureForcesKernel,
				0,
				cl::NDRange(m_particles.Size()),
				cl::NDRange(1));
		}

	}
	catch (cl::Error& err)
	{
		LOG_CORE_ERROR("OpenCL Error: {0}, {1}", err.what(), Util::GetCLErrorString(err.err()));
		throw;
	}
}

void PCISPHSolverGPU::Integrate()
{

}

void PCISPHSolverGPU::ResolveCollisions()
{

}

void PCISPHSolverGPU::EndTimeStep()
{

}

void PCISPHSolverGPU::createBuffers()
{
	try
	{
		m_positiionBuffer = cl::Buffer(m_context.Context(), CL_MEM_READ_WRITE, sizeof(ParticlePoint) * m_particles.Size());
		m_context.Queue().enqueueWriteBuffer(m_positiionBuffer,CL_TRUE,0,sizeof(ParticlePoint) * m_particles.Size(), m_particles.Positions.data());

		m_velocityBuffer = cl::Buffer(m_context.Context(), CL_MEM_READ_WRITE, sizeof(ParticlePoint) * m_particles.Size());
		m_context.Queue().enqueueWriteBuffer(m_velocityBuffer,CL_TRUE,0,sizeof(ParticlePoint) * m_particles.Size(), m_particles.Velocities.data());

		m_forcesBuffer = cl::Buffer(m_context.Context(), CL_MEM_READ_WRITE, sizeof(ParticlePoint) * m_particles.Size());
		m_context.Queue().enqueueWriteBuffer(m_forcesBuffer,CL_TRUE,0,sizeof(ParticlePoint) * m_particles.Size(), m_particles.Forces.data());

		m_densitiyBuffer = cl::Buffer(m_context.Context(), CL_MEM_READ_WRITE, sizeof(cl_float) * m_particles.Size());
		m_context.Queue().enqueueWriteBuffer(m_densitiyBuffer,CL_TRUE,0,sizeof(cl_float) * m_particles.Size(), m_particles.Densities.data());

		m_pressureBuffer = cl::Buffer(m_context.Context(), CL_MEM_READ_WRITE, sizeof(cl_float) * m_particles.Size());
		m_context.Queue().enqueueWriteBuffer(m_pressureBuffer,CL_TRUE,0,sizeof(cl_float) * m_particles.Size(), m_particles.Pressures.data());

		//state buffers
		m_statePositionBuffer = cl::Buffer(m_context.Context(), CL_MEM_READ_WRITE, sizeof(ParticlePoint) * m_particles.Size());
		m_context.Queue().enqueueWriteBuffer(m_statePositionBuffer,CL_TRUE,0,sizeof(ParticlePoint) * m_particles.Size(), m_particles.Positions.data());

		m_stateVelocityBuffer = cl::Buffer(m_context.Context(), CL_MEM_READ_WRITE, sizeof(ParticlePoint) * m_particles.Size());
		m_context.Queue().enqueueWriteBuffer(m_stateVelocityBuffer,CL_TRUE,0,sizeof(ParticlePoint) * m_particles.Size(), m_particles.Velocities.data());

		//Neighbor Buffer
		m_neighborBuffer = cl::Buffer(m_context.Context(), CL_MEM_READ_WRITE, sizeof(uint32_t) * m_particles.Size() * K);
		hostNeighbors = std::vector<uint32_t>(m_particles.Size() * K, std::numeric_limits<uint32_t>::max());

		//sum of kernel buffer
		m_kernelSumBuffer = cl::Buffer(m_context.Context(), CL_MEM_READ_WRITE, sizeof(cl_float) * m_particles.Size());

		//density error buffer for PCI SPH
		m_densityErrorBuffer = cl::Buffer(m_context.Context(), CL_MEM_READ_WRITE, sizeof(cl_float) * m_particles.Size());
		m_estimateDensityBuffer = cl::Buffer(m_context.Context(), CL_MEM_READ_WRITE, sizeof(cl_float) * m_particles.Size());
	}
	catch (cl::Error& err)
	{
		LOG_CORE_ERROR("OpenCL Error: {0}, {1}", err.what(), Util::GetCLErrorString(err.err()));
		throw;
	}
}

void PCISPHSolverGPU::compileKernels() const
{
	m_context.AddProgram("sph", "resources/kernels/sph.cl");
	m_context.GetProgram("sph")->AddKernel("SumOfKernel");
	m_context.GetProgram("sph")->AddKernel("ComputeDensitites");
	m_context.GetProgram("sph")->AddKernel("ApplyNonPressureForces");
	m_context.GetProgram("sph")->AddKernel("ApplyPressureForces");
}

void PCISPHSolverGPU::computeNeighborList()
{
	//TODO compute KNN on GPU
	m_context.Queue().enqueueReadBuffer(m_positiionBuffer,CL_TRUE,0,sizeof(ParticlePoint) * m_particles.Size(), m_particlePoints.data());
	m_tree.Build(reinterpret_cast<std::vector<glm::vec4>&>(m_particles.Positions));

	//Set all neighbors to the max value
	memset(hostNeighbors.data(), std::numeric_limits<uint32_t>::max(), m_particles.Size() * K);

	//find all neighbours of all elements
	#pragma omp parallel for
	for (int i = 0; i < m_particles.Positions.size(); ++i)
	{
		std::vector<uint32_t> e;
		e.reserve(K);
		m_tree.FindNearestNeighbors(m_particles.Positions[i].vec4, KERNEL_RADIUS, e);

		size_t gid = i * K;
		memcpy(&hostNeighbors[gid],e.data(), sizeof(uint32_t) * std::min(e.size(),K));
	}

	try
	{
		m_context.Queue().enqueueWriteBuffer(m_neighborBuffer, CL_TRUE, 0, sizeof(uint32_t) * m_particles.Size() * K, hostNeighbors.data());
	}
	catch (cl::Error& err)
	{
		LOG_CORE_ERROR("OpenCL Error: {0}, {1}", err.what(), Util::GetCLErrorString(err.err()));
		throw;
	}
}

void PCISPHSolverGPU::computeDensities()
{
	//check if the OpenCL context has the Brute NN search program
	OpenCLProgram* program = m_context.GetProgram("sph");
	CORE_ASSERT(program, "Failed to find OpenCLProgram for sph (Make sure its compiled)");
	cl::Kernel* kernelSumKernel = program->GetKernel("SumOfKernel");
	CORE_ASSERT(kernelSumKernel, "Failed to find OpenCL Kernel ('SumOfKernel') for sph (Make sure its compiled)");
	cl::Kernel* densityKernel = program->GetKernel("ComputeDensitites");
	CORE_ASSERT(densityKernel, "Failed to find OpenCL Kernel ('ComputeDensitites') for sph (Make sure its compiled)");

	try
	{
		kernelSumKernel->setArg(0, m_positiionBuffer);
		kernelSumKernel->setArg(1, m_neighborBuffer);
		kernelSumKernel->setArg(2, m_kernelSumBuffer);

		//reset kernel sums
		m_context.Queue().enqueueNDRangeKernel(*kernelSumKernel,
			0,
			cl::NDRange(m_particles.Size()),
			cl::NDRange(1));

		densityKernel->setArg(0, m_kernelSumBuffer);
		densityKernel->setArg(1, m_densitiyBuffer);
		m_context.Queue().enqueueNDRangeKernel(*densityKernel,
			0,
			cl::NDRange(m_particles.Size()),
			cl::NDRange(1));
	}
	catch (cl::Error& err)
	{
		LOG_CORE_ERROR("OpenCL Error: {0}, {1}", err.what(), Util::GetCLErrorString(err.err()));
		throw;
	}
}
