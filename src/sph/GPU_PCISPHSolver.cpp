#include "GPU_PCISPHSolver.h"

#include "util/Log.h"
#include "util/Util.h"

//Due to memory restriction on the GPU need to limit the number of max neighbours for a given point known as K
constexpr size_t K = 256;

GPU_PCISPHSolver::GPU_PCISPHSolver(float timeStep, size_t particleCount, const BoxCollider& boxCollider, OpenCLContext& context) :
	PCISPHSolverCPU(timeStep,particleCount,boxCollider), 
	m_context(context),
	m_particlePoints(particleCount)
{
	createBuffers();
}

void GPU_PCISPHSolver::BeginTimeStep()
{
	//copy over positions into state
	m_context.Queue().enqueueCopyBuffer(m_positiionBuffer,m_statePositionBuffer,0,0,sizeof(ParticlePoint) * m_particles.Size());
	m_context.Queue().enqueueCopyBuffer(m_velocityBuffer,m_stateVelocityBuffer,0,0,sizeof(ParticlePoint) * m_particles.Size());

//	m_state = ParticleState(m_particles);
//
//	m_tree.Build(reinterpret_cast<std::vector<glm::vec4>&>(m_particles.Positions));
//	//LOG_CORE_INFO(glm::to_string(m_state.Positions[0]));
//
	computeNeighborList();
//
//	computeDensities();
}

void GPU_PCISPHSolver::ApplyForces()
{

}

void GPU_PCISPHSolver::Integrate()
{

}

void GPU_PCISPHSolver::ResolveCollisions()
{

}

void GPU_PCISPHSolver::EndTimeStep()
{

}

void GPU_PCISPHSolver::createBuffers()
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
	}
	catch (cl::Error& err)
	{
		LOG_CORE_ERROR("OpenCL Error: {0}, {1}", err.what(), Util::GetCLErrorString(err.err()));
		throw;
	}
}

void GPU_PCISPHSolver::computeNeighborList()
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
