#include "OpenCLHashmapTests.h"

#include "structures/GpuSpatialHash.h"
#include "util/Log.h"
#include "util/Stopwatch.h"
#include "TestData.h"


OpenCLHashmapTests::OpenCLHashmapTests()
{
	buildProgram();
}

void OpenCLHashmapTests::BuildTests(const std::vector<glm::vec4>& points, const AABB& aabb)
{
	GpuSpatialHash gpuNN(points,aabb, m_context);

	Stopwatch sw;
	sw.Start();
	gpuNN.Build(points);
	sw.Stop();

	LOG_CORE_INFO("OpenCLHashmapTests build time: {0}", sw.Time());
}

void OpenCLHashmapTests::NearestNeighborTests(const std::vector<glm::vec4>& points, const AABB& aabb)
{
	GpuSpatialHash gpuNN(points,aabb, m_context);
	gpuNN.Build(points);

	std::vector<glm::vec4> queryPoints{
		glm::vec4(0.0f)
	};

	constexpr int K = 16;
	std::vector<uint32_t> singleQueryPointNeighbors(queryPoints.size() * K);
	Stopwatch sw;
	sw.Start();

	//gpuNN.FindNearestNeighbors(glm::vec4(0.0f,0.0f,0.0f,1.0f), 30.0f,indices);
	gpuNN.KNN(points,queryPoints, aabb,K, 40.0f,singleQueryPointNeighbors);
	sw.Stop();

	size_t neighborCount = std::count_if(singleQueryPointNeighbors.begin(),singleQueryPointNeighbors.end(), [](int next_val){return next_val < std::numeric_limits<cl_uint>::max();});
	LOG_CORE_TRACE("FindNearestNeighbors target:{0} actual:{1}", 7, neighborCount);
	CORE_ASSERT(neighborCount == 7, "OpenCL hashmap FindNearestNeighbors returned wrong value");
	LOG_CORE_INFO("OpenCLHashmapTests NN for single point: {0}", sw.Time());

	//get all neighbours within range
	sw.Start();
	std::vector<std::vector<uint32_t>> allQueryPointNeighbors(points.size() * K);
	gpuNN.FindAllNearestNeighbors(points,40.0f,allQueryPointNeighbors);
	sw.Stop();

	auto neighborTestData = TestData::NeighborCount256();
	for (int i = 0; i < allQueryPointNeighbors.size(); ++i)
	{
		CORE_ASSERT(allQueryPointNeighbors[i].size() == neighborTestData[i], "OpenCL hashmap FindAllNearestNeighbors returned wrong neighbor count");
	}
	
	LOG_CORE_INFO("OpenCLHashmapTests NN for all points: {0}", sw.Time());
}

void OpenCLHashmapTests::buildProgram()
{
	m_context.AddProgram("spatialHash", "resources/kernels/hashmap.cl");
	m_context.GetProgram("spatialHash")->AddKernel("Build");
	m_context.GetProgram("spatialHash")->AddKernel("GetStartSize");
	m_context.GetProgram("spatialHash")->AddKernel("GetNearestNeighbours");
	m_context.GetProgram("spatialHash")->AddKernel("KNN");
}



