#include "GpuSpatialHash.h"
#include <util/Log.h>
#include <util/Util.h>

constexpr uint32_t MAX_NEIGHBORS = 256;

GpuSpatialHash::GpuSpatialHash(const std::vector<point4_t>& points, const AABB& aabb, OpenCLContext& context) :
	m_pointCount(points.size()),
	m_openCLContext(context),
	m_aabb(aabb),
	m_subdivisions(16)
{

}

void GpuSpatialHash::Build(const std::vector<point4_t>& points)
{
	//check if the OpenCL context has the Brute NN search program
	OpenCLProgram* NNProgram = m_openCLContext.GetProgram("spatialHash");
	CORE_ASSERT(NNProgram, "Failed to find OpenCLProgram for spatialHash (Make sure its compiled)");
	cl::Kernel* buildKernel = NNProgram->GetKernel("Build");
	CORE_ASSERT(buildKernel, "Failed to find OpenCL Kernel ('Build') for spatialHash (Make sure its compiled)");
	cl::Kernel* indexSizeKernel = NNProgram->GetKernel("GetStartSize");
	CORE_ASSERT(indexSizeKernel, "Failed to find OpenCL Kernel ('indexSizeKernel') for spatialHash (Make sure its compiled)");

	m_pointCount = points.size();

	glm::vec4 lowerBound = glm::vec4(m_aabb.Min(),1.0f);
	glm::vec4 upperBound = glm::vec4(m_aabb.Max(),1.0f);
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

		cl_uint maxValue = std::numeric_limits<cl_uint>::max();
		if(!m_cellStartIndexBuffer)
		{
			m_cellStartIndexBuffer = std::make_unique<cl::Buffer>(m_openCLContext.Context(),
				CL_MEM_READ_WRITE,
				m_subdivisions * m_subdivisions * m_subdivisions * sizeof(cl_uint),
				nullptr,
				&err);
		}
		m_openCLContext.Queue().enqueueFillBuffer(*m_cellStartIndexBuffer, maxValue,0,m_subdivisions * m_subdivisions * m_subdivisions * sizeof(cl_uint));

		if(!m_cellSizeIndexBuffer)
		{
			m_cellSizeIndexBuffer = std::make_unique<cl::Buffer>(m_openCLContext.Context(),
				CL_MEM_READ_WRITE,
				m_subdivisions * m_subdivisions * m_subdivisions * sizeof(cl_uint),
				nullptr,
				&err);
		}
		//m_openCLContext.Queue().enqueueFillBuffer(*m_cellSizeIndexBuffer, &maxValue,0,sizeof(cl_uint));
	}
	catch (cl::Error& err)
	{
		LOG_CORE_ERROR("OpenCL Error: {0}, {1}", err.what(), Util::GetCLErrorString(err.err()));
		throw;
	}



	std::vector<GpuHashPoint> hashPoints(m_pointCount);
//	std::vector<cl_uint> index(subdivisions * subdivisions * subdivisions);
//	std::vector<cl_uint> sizes(subdivisions * subdivisions * subdivisions);
	try
	{
		buildKernel->setArg(0, *m_pointBuffer);
		buildKernel->setArg(1, *m_sortedPointBuffer);
		buildKernel->setArg(2, *m_cellStartIndexBuffer);
		buildKernel->setArg(3, *m_cellSizeIndexBuffer);
		buildKernel->setArg(4, lowerBound);
		buildKernel->setArg(5, upperBound);
		buildKernel->setArg(6, m_subdivisions);

		m_openCLContext.Queue().enqueueNDRangeKernel(
			*buildKernel,
			cl::NDRange(0),
			cl::NDRange(m_pointCount),
			cl::NDRange(64));

		cl::Event event;
		m_openCLContext.Queue().enqueueReadBuffer(*m_sortedPointBuffer,
			true,
			0,
			m_pointCount * sizeof(GpuHashPoint),
			hashPoints.data(),
			nullptr,
			&event);
		//wait for event to finish
		event.wait();

		//Temp replace with a radix sort on gpu
		std::sort(hashPoints.begin(),
			hashPoints.end(),
			[](GpuHashPoint& a, GpuHashPoint& b) {return a.Hash < b.Hash; });
		m_openCLContext.Queue().enqueueWriteBuffer(*m_sortedPointBuffer, CL_TRUE, 0, m_pointCount * sizeof(GpuHashPoint), hashPoints.data());

		//find index and size
		indexSizeKernel->setArg(0, *m_sortedPointBuffer);
		indexSizeKernel->setArg(1, *m_cellStartIndexBuffer);
		indexSizeKernel->setArg(2, *m_cellSizeIndexBuffer);

		m_openCLContext.Queue().enqueueNDRangeKernel(
			*indexSizeKernel,
			cl::NDRange(0),
			cl::NDRange(m_pointCount),
			cl::NDRange(64));

//		m_openCLContext.Queue().enqueueReadBuffer(*m_cellStartIndexBuffer,
//			true,
//			0,
//			subdivisions * subdivisions * subdivisions * sizeof(cl_uint),
//			index.data(),
//			nullptr,
//			&event);
//		//wait for event to finish
//
//		cl::Event event1;
//		m_openCLContext.Queue().enqueueReadBuffer(*m_cellSizeIndexBuffer,
//			true,
//			0,
//			subdivisions * subdivisions * subdivisions * sizeof(cl_uint),
//			sizes.data(),
//			nullptr,
//			&event1);
//		//wait for event to finish
//		event.wait();
//		event1.wait();

	}catch (cl::Error& err)
	{
		LOG_CORE_ERROR("OpenCL Error: {0}, {1}", err.what(), Util::GetCLErrorString(err.err()));
		throw;
	}
}

bool GpuSpatialHash::FindNearestNeighbor(const point4_t& point, size_t& index)
{
	throw std::logic_error{"FindNearestNeighbor not yet implemented."};
}

bool GpuSpatialHash::FindNearestNeighbors(const point4_t& point, float radius, std::vector<size_t>& indices)
{
	//check if the OpenCL context has the Brute NN search program
	OpenCLProgram* NNProgram = m_openCLContext.GetProgram("spatialHash");
	CORE_ASSERT(NNProgram, "Failed to find OpenCLProgram for spatialHash (Make sure its compiled)");
	cl::Kernel* nnKernel = NNProgram->GetKernel("GetNearestNeighbours");
	CORE_ASSERT(nnKernel, "Failed to find OpenCL Kernel ('GetNearestNeighbours') for spatialHash (Make sure its compiled)");

	try
	{
		std::vector<cl_uint> neighbours(MAX_NEIGHBORS);
		//create neighbour buffer
		cl::Buffer neighbourBuffer(m_openCLContext.Context(),CL_MEM_READ_WRITE, sizeof(cl_uint) * MAX_NEIGHBORS);
		glm::vec4 lowerBound = glm::vec4(m_aabb.Min(),1.0f);
		glm::vec4 upperBound = glm::vec4(m_aabb.Max(),1.0f);

		nnKernel->setArg(0, *m_pointBuffer);
		nnKernel->setArg(1, *m_sortedPointBuffer);
		nnKernel->setArg(2, *m_cellStartIndexBuffer);
		nnKernel->setArg(3, *m_cellSizeIndexBuffer);
		nnKernel->setArg(4, neighbourBuffer);
		nnKernel->setArg(5, lowerBound);
		nnKernel->setArg(6, upperBound);
		nnKernel->setArg(7, m_subdivisions);
		nnKernel->setArg(8, point);
		nnKernel->setArg(9, radius);

		m_openCLContext.Queue().enqueueNDRangeKernel(
			*nnKernel,
			cl::NDRange(0),
			cl::NDRange(1),
			cl::NDRange(1));

		cl::Event event;
		m_openCLContext.Queue().enqueueReadBuffer(neighbourBuffer,
			true,
			0,
			sizeof(cl_uint) * MAX_NEIGHBORS,
			neighbours.data(),
			nullptr,
			&event);
		//wait for event to finish
		event.wait();
	}
	catch (cl::Error& err)
	{
		LOG_CORE_ERROR("OpenCL Error: {0}, {1}", err.what(), Util::GetCLErrorString(err.err()));
		throw;
	}

	return !indices.empty();
}

