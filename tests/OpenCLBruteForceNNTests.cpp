#include <util/Log.h>
#include <util/Stopwatch.h>
#include "OpenCLBruteForceNNTests.h"
#include "structures/GpuBruteForceNN.h"

OpenCLBruteForceNNTests::OpenCLBruteForceNNTests()
{
	buildProgram();
}

void OpenCLBruteForceNNTests::BuildTests(const std::vector<glm::vec4>& points)
{
	GpuBruteForceNN gpuNN(points, m_context);
	gpuNN.Build(points);

	std::vector<uint32_t> indices;

	Stopwatch sw;
	sw.Start();
	gpuNN.FindNearestNeighbors(glm::vec4(0,0,0,0),1000.0f,indices);
	sw.Stop();

	LOG_CORE_INFO("OpenCLBruteForceNNTests find all neighbors of point: {0}", sw.Time());
}

void OpenCLBruteForceNNTests::buildProgram()
{
	m_context.AddProgram("bruteForceNN", "resources/kernels/bruteForceNN.cl");
	m_context.GetProgram("bruteForceNN")->AddKernel("FindNearestNeighbors");
}
