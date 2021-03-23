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

void PCISPHSolverGPU::Setup()
{
	SPHSolverCPU::Setup();
	createBuffers();
}

void PCISPHSolverGPU::BeginTimeStep()
{
	//copy over positions into state
	m_context.Queue().enqueueCopyBuffer(m_positiionBuffer.value(),m_statePositionBuffer.value(),0,0,sizeof(ParticlePoint) * m_particles.Size());
	m_context.Queue().enqueueCopyBuffer(m_velocityBuffer.value(),m_stateVelocityBuffer.value(),0,0,sizeof(ParticlePoint) * m_particles.Size());

	computeNeighborList();

    computeDensities();
}

bool argsSet = false;
void PCISPHSolverGPU::ApplyForces()
{
	//check if the OpenCL context has the Brute NN search program
	OpenCLProgram* program = m_context.GetProgram("sph");
	CORE_ASSERT(program, "Failed to find OpenCLProgram for sph (Make sure its compiled)");
	cl::Kernel* nonPressureForcesKernel = program->GetKernel("ApplyNonPressureForces");
	CORE_ASSERT(nonPressureForcesKernel, "Failed to find OpenCL Kernel ('ApplyNonPressureForces') for sph (Make sure its compiled)");
	cl::Kernel* pressureForcesKernel = program->GetKernel("ApplyPressureForces");
	CORE_ASSERT(pressureForcesKernel, "Failed to find OpenCL Kernel ('ApplyPressureForces') for sph (Make sure its compiled)");
	cl::Kernel* accumulateForcesKernel = program->GetKernel("AccumlateForces");
	CORE_ASSERT(accumulateForcesKernel, "Failed to find OpenCL Kernel ('AccumlateForces') for sph (Make sure its compiled)");

	try
	{
		if (!argsSet) {
			nonPressureForcesKernel->setArg(0, m_neighborBuffer.value());
			nonPressureForcesKernel->setArg(1, m_positiionBuffer.value());
			nonPressureForcesKernel->setArg(2, m_velocityBuffer.value());
			nonPressureForcesKernel->setArg(3, m_densitiyBuffer.value());
			nonPressureForcesKernel->setArg(4, m_forcesBuffer.value());

			pressureForcesKernel->setArg(0, m_neighborBuffer.value());
			pressureForcesKernel->setArg(1, m_positiionBuffer.value());
			pressureForcesKernel->setArg(2, m_velocityBuffer.value());
			pressureForcesKernel->setArg(3, m_densitiyBuffer.value());
			pressureForcesKernel->setArg(4, m_forcesBuffer.value());
			pressureForcesKernel->setArg(5, m_pressureBuffer.value());
			pressureForcesKernel->setArg(6, m_densityErrorBuffer.value());
			pressureForcesKernel->setArg(7, m_estimateDensityBuffer.value());
			pressureForcesKernel->setArg(8, m_pressureForcesBuffer.value());
			pressureForcesKernel->setArg(9, m_targetDensitiy);
			pressureForcesKernel->setArg(10, deltaDensitity);
			pressureForcesKernel->setArg(11, m_negativePressureScale);

			accumulateForcesKernel->setArg(0, m_pressureForcesBuffer.value());
			accumulateForcesKernel->setArg(1, m_forcesBuffer.value());

			argsSet = true;
		}
		
		//reset kernel sums
		m_context.Queue().enqueueNDRangeKernel(*nonPressureForcesKernel,
			0,
			cl::NDRange(m_particles.Size()),
			cl::NDRange(256));


		m_context.Queue().enqueueFillBuffer(m_densityErrorBuffer.value(), 0.0f, 0, sizeof(cl_float) * m_particles.Size());
		m_context.Queue().enqueueCopyBuffer(m_pressureBuffer.value(), m_estimateDensityBuffer.value(), 0, 0, sizeof(cl_float) * m_particles.Size());
		m_context.Queue().enqueueFillBuffer(m_pressureForcesBuffer.value(), ParticlePoint(), 0, sizeof(ParticlePoint) * m_particles.Size());

		//for (size_t i = 0; i < 5; i++)
		//{
		//	m_context.Queue().enqueueNDRangeKernel(*pressureForcesKernel,
		//		0,
		//		cl::NDRange(m_particles.Size()),
		//		cl::NDRange(256));
		//}

		//m_context.Queue().enqueueNDRangeKernel(*accumulateForcesKernel,
		//	0,
		//	cl::NDRange(m_particles.Size()),
		//	cl::NDRange(1));
	}
	catch (cl::Error& err)
	{
		LOG_CORE_ERROR("OpenCL Error: {0}, {1}", err.what(), Util::GetCLErrorString(err.err()));
		throw;
	}
}

void PCISPHSolverGPU::Integrate()
{
	//check if the OpenCL context has the Brute NN search program
	OpenCLProgram* program = m_context.GetProgram("sph");
	CORE_ASSERT(program, "Failed to find OpenCLProgram for sph (Make sure its compiled)");
	cl::Kernel* integrateKernel = program->GetKernel("integrate");
	CORE_ASSERT(integrateKernel, "Failed to find OpenCL Kernel ('integrate') for sph (Make sure its compiled)");

	integrateKernel->setArg(0, m_forcesBuffer.value());
	integrateKernel->setArg(1, m_positiionBuffer.value());
	integrateKernel->setArg(2, m_velocityBuffer.value());
	
	try
	{
		m_context.Queue().enqueueNDRangeKernel(*integrateKernel, 0, cl::NDRange(m_particles.Size()), cl::NDRange(64));
		m_context.Queue().enqueueReadBuffer(m_positiionBuffer.value(), CL_TRUE, 0, sizeof(ParticlePoint) * m_particles.Size(), m_particles.Positions.data());
	}
	catch (cl::Error& err)
	{
		LOG_CORE_ERROR("OpenCL Error: {0}, {1}", err.what(), Util::GetCLErrorString(err.err()));
		throw;
	}
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
		if(!m_positiionBuffer.has_value())
			m_positiionBuffer = cl::Buffer(m_context.Context(), CL_MEM_READ_WRITE, sizeof(ParticlePoint) * m_particles.Size());
		m_context.Queue().enqueueWriteBuffer(m_positiionBuffer.value(),CL_TRUE,0,sizeof(ParticlePoint) * m_particles.Size(), m_particles.Positions.data());

		if (!m_velocityBuffer.has_value())
			m_velocityBuffer = cl::Buffer(m_context.Context(), CL_MEM_READ_WRITE, sizeof(ParticlePoint) * m_particles.Size());
		m_context.Queue().enqueueWriteBuffer(m_velocityBuffer.value(),CL_TRUE,0,sizeof(ParticlePoint) * m_particles.Size(), m_particles.Velocities.data());

		if (!m_forcesBuffer.has_value())
			m_forcesBuffer = cl::Buffer(m_context.Context(), CL_MEM_READ_WRITE, sizeof(ParticlePoint) * m_particles.Size());
		m_context.Queue().enqueueWriteBuffer(m_forcesBuffer.value(),CL_TRUE,0,sizeof(ParticlePoint) * m_particles.Size(), m_particles.Forces.data());

		if (!m_densitiyBuffer.has_value())
			m_densitiyBuffer = cl::Buffer(m_context.Context(), CL_MEM_READ_WRITE, sizeof(cl_float) * m_particles.Size());
		m_context.Queue().enqueueWriteBuffer(m_densitiyBuffer.value(),CL_TRUE,0,sizeof(cl_float) * m_particles.Size(), m_particles.Densities.data());

		if (!m_pressureBuffer.has_value())
			m_pressureBuffer = cl::Buffer(m_context.Context(), CL_MEM_READ_WRITE, sizeof(cl_float) * m_particles.Size());
		m_context.Queue().enqueueWriteBuffer(m_pressureBuffer.value(),CL_TRUE,0,sizeof(cl_float) * m_particles.Size(), m_particles.Pressures.data());

		//state buffers
		if (!m_statePositionBuffer.has_value())
			m_statePositionBuffer = cl::Buffer(m_context.Context(), CL_MEM_READ_WRITE, sizeof(ParticlePoint) * m_particles.Size());
		m_context.Queue().enqueueWriteBuffer(m_statePositionBuffer.value(),CL_TRUE,0,sizeof(ParticlePoint) * m_particles.Size(), m_particles.Positions.data());

		if (!m_stateVelocityBuffer.has_value())
			m_stateVelocityBuffer = cl::Buffer(m_context.Context(), CL_MEM_READ_WRITE, sizeof(ParticlePoint) * m_particles.Size());
		m_context.Queue().enqueueWriteBuffer(m_stateVelocityBuffer.value(),CL_TRUE,0,sizeof(ParticlePoint) * m_particles.Size(), m_particles.Velocities.data());

		//Neighbor Buffer
		if (!m_neighborBuffer.has_value())
			m_neighborBuffer = cl::Buffer(m_context.Context(), CL_MEM_READ_WRITE, sizeof(uint32_t) * m_particles.Size() * K);
		hostNeighbors = std::vector<uint32_t>(m_particles.Size() * K, std::numeric_limits<uint32_t>::max());

		//sum of kernel buffer
		if (!m_kernelSumBuffer.has_value())
			m_kernelSumBuffer = cl::Buffer(m_context.Context(), CL_MEM_READ_WRITE, sizeof(cl_float) * m_particles.Size());

		//density error buffer for PCI SPH
		if (!m_densityErrorBuffer.has_value())
			m_densityErrorBuffer = cl::Buffer(m_context.Context(), CL_MEM_READ_WRITE, sizeof(cl_float) * m_particles.Size());
		if (!m_estimateDensityBuffer.has_value())
			m_estimateDensityBuffer = cl::Buffer(m_context.Context(), CL_MEM_READ_WRITE, sizeof(cl_float) * m_particles.Size());
		if (!m_pressureForcesBuffer.has_value())
			m_pressureForcesBuffer = cl::Buffer(m_context.Context(), CL_MEM_READ_WRITE, sizeof(ParticlePoint) * m_particles.Size());
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
	m_context.GetProgram("sph")->AddKernel("AccumlateForces");
	m_context.GetProgram("sph")->AddKernel("integrate");
}

void PCISPHSolverGPU::computeNeighborList()
{
	//TODO compute KNN on GPU
	m_context.Queue().enqueueReadBuffer(m_positiionBuffer.value(),CL_TRUE,0,sizeof(ParticlePoint) * m_particles.Size(), m_particlePoints.data());
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
		m_context.Queue().enqueueWriteBuffer(m_neighborBuffer.value(), CL_TRUE, 0, sizeof(uint32_t) * m_particles.Size() * K, hostNeighbors.data());
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
		kernelSumKernel->setArg(0, m_positiionBuffer.value());
		kernelSumKernel->setArg(1, m_neighborBuffer.value());
		kernelSumKernel->setArg(2, m_kernelSumBuffer.value());

		//reset kernel sums
		m_context.Queue().enqueueNDRangeKernel(*kernelSumKernel,
			0,
			cl::NDRange(m_particles.Size()),
			cl::NDRange(1));

		densityKernel->setArg(0, m_kernelSumBuffer.value());
		densityKernel->setArg(1, m_densitiyBuffer.value());
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
