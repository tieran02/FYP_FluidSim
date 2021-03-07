#include "OpenCLHashmapTests.h"

#include "util/Log.h"
#include "util/Util.h"


OpenCLHashmapTests::OpenCLHashmapTests()
{
	compileProgram();
}

void OpenCLHashmapTests::compileProgram()
{
	m_context.AddProgram("hashmap", "resources/kernels/hashmap.cl");
	m_context.GetProgram("hashmap")->AddKernel("build_hashmap");

}

void OpenCLHashmapTests::InsertTests(const std::vector<glm::vec3>& points)
{
	std::vector<hash_pair> hashPairs(points.size());

	cl_int err = CL_SUCCESS;

	try
	{
		cl::Buffer bufferPoints(m_context.Context(), CL_MEM_READ_ONLY, points.size() * sizeof(glm::vec3), nullptr, &err);
		cl::Buffer bufferHashPairs(m_context.Context(), CL_MEM_WRITE_ONLY, points.size() * sizeof(hash_pair), nullptr, &err);

		m_context.Queue().enqueueWriteBuffer(bufferPoints, CL_TRUE, 0, points.size() * sizeof(glm::vec3), points.data());

		cl::Kernel hashKernal = *m_context.GetProgram("hashmap")->GetKernel("build_hashmap");
		hashKernal.setArg(0, bufferPoints);
		hashKernal.setArg(1, bufferHashPairs);
		hashKernal.setArg(2, (cl_int)points.size());
		hashKernal.setArg(3, 2000);

		//get max work-group size that can be used for kernel
		size_t localItemSize;
		hashKernal.getWorkGroupInfo(m_context.Device(), CL_KERNEL_WORK_GROUP_SIZE, &localItemSize);


		m_context.Queue().enqueueNDRangeKernel(
			hashKernal,
			cl::NDRange(0),
			cl::NDRange(points.size()),
			cl::NDRange(localItemSize));

		cl::Event event;
		m_context.Queue().enqueueReadBuffer(bufferHashPairs, true, 0, points.size() * sizeof(hash_pair), hashPairs.data(), nullptr, &event);
		//wait for event to finish
		event.wait();
	}
	catch (cl::Error& err)
	{
		LOG_CORE_ERROR("OpenCL Error: {0}, {1}", err.what(), Util::GetCLErrorString(err.err()));
		throw;
	}
}
