#include "GpuSpatialHash.h"
#include <util/Log.h>
#include <util/Util.h>

GpuSpatialHash::GpuSpatialHash(const std::vector<point4_t>& points, const AABB& aabb, OpenCLContext& context) :
	m_pointCount(points.size()),
	m_openCLContext(context),
	m_aabb(aabb)
{

}

void GpuSpatialHash::Build(const std::vector<point4_t>& points)
{
	//check if the OpenCL context has the Brute NN search program
	OpenCLProgram* NNProgram = m_openCLContext.GetProgram("spatialHash");
	CORE_ASSERT(NNProgram, "Failed to find OpenCLProgram for spatialHash (Make sure its compiled)");
	cl::Kernel* kernel = NNProgram->GetKernel("Build");
	CORE_ASSERT(kernel, "Failed to find OpenCL Kernel ('Build') for spatialHash (Make sure its compiled)");

	m_pointCount = points.size();

	glm::vec4 lowerBound = glm::vec4(m_aabb.Min(),1.0f);
	glm::vec4 upperBound = glm::vec4(m_aabb.Max(),1.0f);
	int subdivisions = 16;

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

		if(!m_sortedPointBuffer)
		{
			m_sortedPointBuffer = std::make_unique<cl::Buffer>(m_openCLContext.Context(),
				CL_MEM_READ_WRITE,
				points.size() * sizeof(GpuHashPoint),
				nullptr,
				&err);
		}

		if(!m_cellStartIndexBuffer)
		{
			m_cellStartIndexBuffer = std::make_unique<cl::Buffer>(m_openCLContext.Context(),
				CL_MEM_READ_WRITE,
				subdivisions * subdivisions * subdivisions * sizeof(cl_uint),
				nullptr,
				&err);
		}

		if(!m_cellSizeIndexBuffer)
		{
			m_cellStartIndexBuffer = std::make_unique<cl::Buffer>(m_openCLContext.Context(),
				CL_MEM_READ_WRITE,
				subdivisions * subdivisions * subdivisions * sizeof(cl_uint),
				nullptr,
				&err);
		}
	}
	catch (cl::Error& err)
	{
		LOG_CORE_ERROR("OpenCL Error: {0}, {1}", err.what(), Util::GetCLErrorString(err.err()));
		throw;
	}



	std::vector<GpuHashPoint> hashPoints(m_pointCount);
	try
	{
		kernel->setArg(0, *m_pointBuffer);
		kernel->setArg(1, *m_sortedPointBuffer);
		kernel->setArg(2, *m_cellStartIndexBuffer);
		kernel->setArg(3, *m_cellSizeIndexBuffer);
		kernel->setArg(4, lowerBound);
		kernel->setArg(5, upperBound);
		kernel->setArg(6, subdivisions);

		m_openCLContext.Queue().enqueueNDRangeKernel(
			*kernel,
			cl::NDRange(0),
			cl::NDRange(m_pointCount),
			cl::NDRange(64));

//		cl::Event event;
//		m_openCLContext.Queue().enqueueReadBuffer(*m_sortedPointBuffer,
//			true,
//			0,
//			m_pointCount * sizeof(GpuHashPoint),
//			hashPoints.data(),
//			nullptr,
//			&event);
//		//wait for event to finish
//		event.wait();
	}catch (cl::Error& err)
	{
		LOG_CORE_ERROR("OpenCL Error: {0}, {1}", err.what(), Util::GetCLErrorString(err.err()));
		throw;
	}
}

bool GpuSpatialHash::FindNearestNeighbor(const point4_t& point, size_t& index)
{
	return false;
}

bool GpuSpatialHash::FindNearestNeighbors(const point4_t& point, float radius, std::vector<size_t>& indices)
{
	return false;
}

