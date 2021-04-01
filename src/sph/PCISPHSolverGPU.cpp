#include "PCISPHSolverGPU.h"

#include "util/Log.h"
#include "util/Util.h"

//Due to memory restriction on the GPU need to limit the number of max neighbours for a given point known as K
constexpr size_t K = 256;

PCISPHSolverGPU::PCISPHSolverGPU(float timeStep, size_t particleCount, const BoxCollider& boxCollider, OpenCLContext& context) :
	PCISPHSolverCPU(timeStep,particleCount,boxCollider), 
	m_context(context),
	m_particlePoints(particleCount),
	m_localWorkGroupSize(std::min(64,static_cast<int>(particleCount)))
{
	m_negativePressureScale = 0.01f;
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
	std::array<cl::Event, 2> events;
	m_context.Queue().enqueueCopyBuffer(m_positiionBuffer.value(),m_statePositionBuffer.value(),0,0,sizeof(ParticlePoint) * m_particles.Size(),nullptr,&events[0]);
	m_context.Queue().enqueueCopyBuffer(m_velocityBuffer.value(),m_stateVelocityBuffer.value(),0,0,sizeof(ParticlePoint) * m_particles.Size(),nullptr, &events[1]);
	events[0].wait();
	events[1].wait();

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
	cl::Kernel* calculatePressureKernel = program->GetKernel("CalculatePressure");
	CORE_ASSERT(calculatePressureKernel, "Failed to find OpenCL Kernel ('CalculatePressure') for sph (Make sure its compiled)");
	cl::Kernel* accumulateForcesKernel = program->GetKernel("AccumlateForces");
	CORE_ASSERT(accumulateForcesKernel, "Failed to find OpenCL Kernel ('AccumlateForces') for sph (Make sure its compiled)");
	cl::Kernel* predictKernel = program->GetKernel("PredictPositionVelocity");
	CORE_ASSERT(predictKernel, "Failed to find OpenCL Kernel ('PredictPositionVelocity') for sph (Make sure its compiled)");
	cl::Kernel* accumalatePressureForcesKernel = program->GetKernel("AccumlatePressureForces");
	CORE_ASSERT(accumalatePressureForcesKernel, "Failed to find OpenCL Kernel ('AccumlatePressureForces') for sph (Make sure its compiled)");

	try
	{
		if (!argsSet) {
			nonPressureForcesKernel->setArg(0, m_neighborBuffer.value());
			nonPressureForcesKernel->setArg(1, m_positiionBuffer.value());
			nonPressureForcesKernel->setArg(2, m_velocityBuffer.value());
			nonPressureForcesKernel->setArg(3, m_densitiyBuffer.value());
			nonPressureForcesKernel->setArg(4, m_forcesBuffer.value());

			calculatePressureKernel->setArg(0, m_neighborBuffer.value());
			calculatePressureKernel->setArg(1, m_statePositionBuffer.value());
			calculatePressureKernel->setArg(2, m_stateVelocityBuffer.value());
			calculatePressureKernel->setArg(3, m_pressureBuffer.value());
			calculatePressureKernel->setArg(4, m_densityErrorBuffer.value());
			calculatePressureKernel->setArg(5, m_estimateDensityBuffer.value());
			calculatePressureKernel->setArg(6, m_pressureForcesBuffer.value());
			calculatePressureKernel->setArg(7, m_targetDensitiy);
			calculatePressureKernel->setArg(8, deltaDensitity);
			calculatePressureKernel->setArg(9, m_negativePressureScale);

			accumulateForcesKernel->setArg(0, m_pressureForcesBuffer.value());
			accumulateForcesKernel->setArg(1, m_forcesBuffer.value());

			predictKernel->setArg(0, m_positiionBuffer.value());
			predictKernel->setArg(1, m_velocityBuffer.value());
			predictKernel->setArg(2, m_forcesBuffer.value());
			predictKernel->setArg(3, m_pressureForcesBuffer.value());
			predictKernel->setArg(4, m_statePositionBuffer.value());
			predictKernel->setArg(5, m_stateVelocityBuffer.value());
			predictKernel->setArg(6, TIMESTEP);

			accumalatePressureForcesKernel->setArg(0, m_neighborBuffer.value());
			accumalatePressureForcesKernel->setArg(1, m_statePositionBuffer.value());
			accumalatePressureForcesKernel->setArg(2, m_stateVelocityBuffer.value());
			accumalatePressureForcesKernel->setArg(3, m_estimateDensityBuffer.value());
			accumalatePressureForcesKernel->setArg(4, m_pressureBuffer.value());
			accumalatePressureForcesKernel->setArg(5, m_pressureForcesBuffer.value());

			argsSet = true;
		}

		std::array<cl::Event, 10> events;
		//reset kernel sums
		m_context.Queue().enqueueNDRangeKernel(*nonPressureForcesKernel,
			0,
			cl::NDRange(m_particles.Size()),
			cl::NDRange(m_localWorkGroupSize),
			nullptr,
			&events[0]);
		events[0].wait();


		m_context.Queue().enqueueFillBuffer(m_pressureBuffer.value(), 0.0f, 0, sizeof(cl_float) * m_particles.Size(),nullptr, &events[1]);
		m_context.Queue().enqueueFillBuffer(m_pressureForcesBuffer.value(), ParticlePoint(), 0, sizeof(ParticlePoint) * m_particles.Size(), nullptr, &events[2]);
		m_context.Queue().enqueueFillBuffer(m_densityErrorBuffer.value(), 0.0f, 0, sizeof(cl_float) * m_particles.Size(), nullptr, &events[3]);
		m_context.Queue().enqueueCopyBuffer(m_densitiyBuffer.value(), m_estimateDensityBuffer.value(), 0, 0, sizeof(cl_float) * m_particles.Size(), nullptr, &events[4]);
		events[1].wait();
		events[2].wait();
		events[3].wait();
		events[4].wait();
		
		for (size_t i = 0; i < 1; i++)
		{
			m_context.Queue().enqueueNDRangeKernel(*predictKernel,
				0,
				cl::NDRange(m_particles.Size()),
				cl::NDRange(m_localWorkGroupSize),
				nullptr,
				&events[5]);
			events[5].wait();

			resolveCollisions(m_statePositionBuffer.value(),m_stateVelocityBuffer.value());

			m_context.Queue().enqueueNDRangeKernel(*calculatePressureKernel,
				0,
				cl::NDRange(m_particles.Size()),
				cl::NDRange(m_localWorkGroupSize),
				nullptr,
				&events[6]);
			events[6].wait();

			m_context.Queue().enqueueFillBuffer(m_pressureForcesBuffer.value(), ParticlePoint(), 0, sizeof(ParticlePoint) * m_particles.Size());
			m_context.Queue().enqueueNDRangeKernel(*accumalatePressureForcesKernel,
				0,
				cl::NDRange(m_particles.Size()),
				cl::NDRange(m_localWorkGroupSize),
				nullptr,
				&events[7]);
			events[7].wait();

			
			//max densitiy error
			//TODO ON GPU
			float maxDensityError = 0.0f;
			std::vector<float> densityErrors(PARTICLE_COUNT);
			m_context.Queue().enqueueReadBuffer(m_densityErrorBuffer.value(), CL_TRUE, 0, sizeof(cl_float) * m_particles.Size(), densityErrors.data(),nullptr,&events[8]);
			events[8].wait();
			
			#pragma omp parallel for
			for (int j = 0; j < PARTICLE_COUNT; ++j)
			{
				maxDensityError = std::max(maxDensityError, fabsf(densityErrors[i]));
			}

			float densityErrorRatio = maxDensityError / m_targetDensitiy;

			if (fabs(densityErrorRatio) < m_maxErrorRatio)
			{
				break;
			}
		}

		m_context.Queue().enqueueNDRangeKernel(*accumulateForcesKernel,
			0,
			cl::NDRange(m_particles.Size()),
			cl::NDRange(m_localWorkGroupSize),
			nullptr,
			&events[9]);
		events[9].wait();
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
	integrateKernel->setArg(3, TIMESTEP);
	
	try
	{
		std::array<cl::Event, 1> events;
		m_context.Queue().enqueueNDRangeKernel(*integrateKernel, 0, cl::NDRange(m_particles.Size()), cl::NDRange(m_localWorkGroupSize),nullptr, &events[0]);
		events[0].wait();
	}
	catch (cl::Error& err)
	{
		LOG_CORE_ERROR("OpenCL Error: {0}, {1}", err.what(), Util::GetCLErrorString(err.err()));
		throw;
	}
}

void PCISPHSolverGPU::ResolveCollisions()
{
	resolveCollisions(m_positiionBuffer.value(),m_velocityBuffer.value());
}

void PCISPHSolverGPU::resolveCollisions(cl::Buffer& positions, cl::Buffer& velocities)
{
	//check if the OpenCL context has the Brute NN search program
	OpenCLProgram* program = m_context.GetProgram("collisions");
	CORE_ASSERT(program, "Failed to find OpenCLProgram for collisions (Make sure its compiled)");
	cl::Kernel* collisionKernel = program->GetKernel("SphereAABBCollisions");
	CORE_ASSERT(collisionKernel, "Failed to find OpenCL Kernel ('SphereAABBCollisions') for collisions (Make sure its compiled)");

	glm::vec4 lower{ BOX_COLLIDER.GetAABB().Min(), 0.0f };
	glm::vec4 upper{ BOX_COLLIDER.GetAABB().Max(), 0.0f };

	try
	{
		collisionKernel->setArg(0, positions);
		collisionKernel->setArg(1, velocities);
		collisionKernel->setArg(2, lower);
		collisionKernel->setArg(3, upper);
		collisionKernel->setArg(4, PARTICLE_RADIUS);
		collisionKernel->setArg(5, TIMESTEP);

		std::array<cl::Event, 1> events;
		m_context.Queue().enqueueNDRangeKernel(*collisionKernel,
			0,
			cl::NDRange(m_particles.Size()),
			cl::NDRange(m_localWorkGroupSize),
			nullptr,
			&events[0]);

		events[0].wait();
	}
	catch (cl::Error& err)
	{
		LOG_CORE_ERROR("OpenCL Error: {0}, {1}", err.what(), Util::GetCLErrorString(err.err()));
		throw;
	}
}


void PCISPHSolverGPU::EndTimeStep()
{
	try
	{
		std::array<cl::Event, 1> events;
		m_context.Queue().enqueueReadBuffer(m_positiionBuffer.value(), CL_TRUE, 0, sizeof(ParticlePoint) * m_particles.Size(), m_particles.Positions.data(),nullptr,&events[0]);
		events[0].wait();
	}
	catch (cl::Error& err)
	{
		LOG_CORE_ERROR("OpenCL Error: {0}, {1}", err.what(), Util::GetCLErrorString(err.err()));
		throw;
	}
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
	m_context.GetProgram("sph")->AddKernel("CalculatePressure");
	m_context.GetProgram("sph")->AddKernel("AccumlateForces");
	m_context.GetProgram("sph")->AddKernel("integrate");
	m_context.GetProgram("sph")->AddKernel("PredictPositionVelocity");
	m_context.GetProgram("sph")->AddKernel("AccumlatePressureForces");

	m_context.AddProgram("collisions", "resources/kernels/collisions.cl");
	m_context.GetProgram("collisions")->AddKernel("SphereAABBCollisions");
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
		std::array<cl::Event, 1> events;
		m_context.Queue().enqueueWriteBuffer(m_neighborBuffer.value(), CL_TRUE, 0, sizeof(uint32_t) * m_particles.Size() * K, hostNeighbors.data(), nullptr, &events[0]);
		events[0].wait();
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

		std::array<cl::Event, 2> events;
		
		//reset kernel sums
		m_context.Queue().enqueueNDRangeKernel(*kernelSumKernel,
			0,
			cl::NDRange(m_particles.Size()),
			cl::NDRange(m_localWorkGroupSize),
			nullptr,
			&events[0]);
		events[0].wait();

		densityKernel->setArg(0, m_kernelSumBuffer.value());
		densityKernel->setArg(1, m_densitiyBuffer.value());
		m_context.Queue().enqueueNDRangeKernel(*densityKernel,
			0,
			cl::NDRange(m_particles.Size()),
			cl::NDRange(m_localWorkGroupSize),
			nullptr,
			&events[1]);
		events[1].wait();
	}
	catch (cl::Error& err)
	{
		LOG_CORE_ERROR("OpenCL Error: {0}, {1}", err.what(), Util::GetCLErrorString(err.err()));
		throw;
	}
}
