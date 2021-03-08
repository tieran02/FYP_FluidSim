#include <structures/GpuSpatialHash.h>
#include <util/Stopwatch.h>
#include "OpenCLHashmapTests.h"

#include "util/Log.h"
#include "util/Util.h"


OpenCLHashmapTests::OpenCLHashmapTests()
{
	buildProgram();
}

void OpenCLHashmapTests::InsertTests(const std::vector<glm::vec4>& points, const AABB& aabb)
{
	GpuSpatialHash gpuNN(points,aabb, m_context);

	Stopwatch sw;
	sw.Start();
	gpuNN.Build(points);
	sw.Stop();

	LOG_CORE_INFO("OpenCLHashmapTests build time: {0}", sw.Time());
}


void OpenCLHashmapTests::buildProgram()
{
	m_context.AddProgram("spatialHash", "resources/kernels/hashmap.cl");
	m_context.GetProgram("spatialHash")->AddKernel("Build");
}

