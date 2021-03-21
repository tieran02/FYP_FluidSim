#include "GPU_PCISPHSolver.h"

#include "util/Log.h"
#include "util/Util.h"

GPU_PCISPHSolver::GPU_PCISPHSolver(float timeStep, size_t particleCount, const BoxCollider& boxCollider, OpenCLContext& context) :
	PCISPHSolverCPU(timeStep,particleCount,boxCollider), 
	m_context(context)
{
	createBuffers();
}

void GPU_PCISPHSolver::BeginTimeStep()
{	
	SPHSolverCPU::BeginTimeStep();
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
	}
	catch (cl::Error& err)
	{
		LOG_CORE_ERROR("OpenCL Error: {0}, {1}", err.what(), Util::GetCLErrorString(err.err()));
		throw;
	}
	
}
