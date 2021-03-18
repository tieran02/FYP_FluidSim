#include "GpuBruteForceNN.h"
#include <util/Log.h>
#include <util/Util.h>


constexpr cl_uint MAX_NEIGHBOURS = 512;

GpuBruteForceNN::GpuBruteForceNN(const std::vector<point4_t>& points, OpenCLContext& context) :
	m_openCLContext(context),
	m_pointBuffer(nullptr),
	m_neighboursBuffer(nullptr)
{

}

void GpuBruteForceNN::Build(const std::vector<point4_t>& points)
{
	//check if the OpenCL context has the Brute NN search program
	OpenCLProgram* bruteForceNNProgram = m_openCLContext.GetProgram("bruteForceNN");
	CORE_ASSERT(bruteForceNNProgram, "Failed to find OpenCLProgram for BruteForceNN (Make sure its compiled)");



	cl_int err = CL_SUCCESS;
	try
	{
		//build point openCL buffer
		if(!m_pointBuffer)
		{
			m_pointBuffer = std::make_unique<cl::Buffer>(m_openCLContext.Context(),
				CL_MEM_READ_ONLY,
				points.size() * sizeof(point4_t),
				nullptr,
				&err);
		}
		m_openCLContext.Queue().enqueueWriteBuffer(*m_pointBuffer, CL_TRUE, 0, points.size() * sizeof(point4_t), points.data());

		if(!m_neighboursBuffer)
		{
			m_neighboursBuffer = std::make_unique<cl::Buffer>(m_openCLContext.Context(),
				CL_MEM_READ_WRITE,
				points.size() * sizeof(float),
				nullptr,
				&err);
		}
	}
	catch (cl::Error& err)
	{
		LOG_CORE_ERROR("OpenCL Error: {0}, {1}", err.what(), Util::GetCLErrorString(err.err()));
		throw;
	}

	m_pointCount = points.size();
}

bool GpuBruteForceNN::FindNearestNeighbor(const point4_t& point, uint32_t& index)
{
	return false;
}

bool GpuBruteForceNN::FindNearestNeighbors(const point4_t& point, float radius, std::vector<uint32_t>& indices)
{
	//The NN search is off-loaded to the GPU with OpenCL and follows the following algorithm:
	//1 Compute all the distances between the point and all other points
	//2 Sort the computed distances
	//3 Select all the points with the radius of the euclidean distance
	//Note this is an expensive brute force approach but due to parallelisation on the GPU its not as bad
	//Note the first element of the neighbors buffer is the number of neighbors for the queryPoint

	OpenCLProgram* bruteForceNNProgram = m_openCLContext.GetProgram("bruteForceNN");
	CORE_ASSERT(bruteForceNNProgram, "Failed to find OpenCLProgram for BruteForceNN (Make sure its compiled)");
	cl::Kernel* kernel = bruteForceNNProgram->GetKernel("FindNearestNeighbors");
	CORE_ASSERT(kernel, "Failed to find OpenCL Kernel ('FindNearestNeighbors') for BruteForceNN (Make sure its compiled)");

	std::vector<uint32_t> neighbours(m_pointCount);
	cl_int err = CL_SUCCESS;
	try
	{
		kernel->setArg(0, *m_pointBuffer);
		kernel->setArg(1, *m_neighboursBuffer);
		kernel->setArg(2, cl_float3{point.x,point.y,point.z});
		kernel->setArg(3, m_pointCount);
		kernel->setArg(4, MAX_NEIGHBOURS);
		kernel->setArg(5, radius);

		m_openCLContext.Queue().enqueueNDRangeKernel(
			*kernel,
			cl::NDRange(0),
			cl::NDRange(m_pointCount),
			cl::NDRange(16));

		cl::Event event;
		m_openCLContext.Queue().enqueueReadBuffer(*m_neighboursBuffer,
			true,
			0,
			m_pointCount * sizeof(uint32_t),
			neighbours.data(),
			nullptr,
			&event);
		//wait for event to finish
		event.wait();
	}catch (cl::Error& err)
	{
		LOG_CORE_ERROR("OpenCL Error: {0}, {1}", err.what(), Util::GetCLErrorString(err.err()));
		throw;
	}

	return false;
}

bool GpuBruteForceNN::FindAllNearestNeighbors(const std::vector<point4_t>& points,
	float radius,
	std::vector<std::vector<uint32_t>>& indices)
{
	return false;
}
