#include <random>
#include <math/AABB.h>
#include <structures/KDTree.h>
#include <structures/SpartialHash.h>
#include <util/Log.h>



#include "OpenCLHashmapTests.h"
#include "TestData.h"
#include "util/Stopwatch.h"

std::vector<glm::vec3> randomPoints(size_t count)
{
	std::vector<glm::vec3> points(count);

	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int> dist(-1000, 1000);

	for (int i = 0; i < count; ++i)
	{
		points[i].x = dist(mt);
		points[i].y = dist(mt);
		points[i].z = dist(mt);
	}
	return points;
}

std::vector<glm::vec4> randomPoints4(size_t count)
{
	std::vector<glm::vec4> points(count);

	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int> dist(-1000, 1000);

	for (int i = 0; i < count; ++i)
	{
		points[i].x = dist(mt);
		points[i].y = dist(mt);
		points[i].z = dist(mt);
		points[i].w = 0.0f;
	}
	return points;
}

constexpr uint32_t POINT_COUNT{10000};

void KDTests(uint32_t PointCount)
{
	LOG_CORE_INFO("--------------------------------------------------------------------------------------------------");
	LOG_CORE_INFO("------------------------------------------KD-Tree-------------------------------------------------");
	LOG_CORE_INFO("--------------------------------------------------------------------------------------------------");
	LOG_CORE_INFO("Number of points: {}", PointCount);

	auto points = randomPoints(PointCount);

	Stopwatch sw;

	sw.Start();
	KDTree<3> tree(points, 1);
	sw.Stop();
	double constructionTime = sw.Time();
	LOG_CORE_INFO("KD-Tree constuction time: {}", constructionTime);

	sw.Start();
	uint32_t index = 0;
	bool found = tree.FindNearestNeighbor(glm::vec3(20,41,4), index);
	sw.Stop();
	LOG_CORE_INFO("KD-Tree find closest index to point time: {}", sw.Time());

	sw.Start();
	std::vector<uint32_t> elements;
	elements.reserve(1000);
	bool foundElements = tree.FindNearestNeighbors(glm::vec3(50,60,60),20*20, elements);
	sw.Stop();
	LOG_CORE_INFO("KD-Tree find nearest neighbors to point within radius time: {}", sw.Time());

	sw.Start();
	//find all neighbours of all elements
	std::vector<std::vector<uint32_t>> neighbors(points.size());
	#pragma omp parallel for
	for (int i = 0; i < points.size(); ++i)
	{
		std::vector<uint32_t> e;
		e.reserve(1000);
		bool foundE = tree.FindNearestNeighbors(points[i],25*25, e);
		neighbors[i] = e;
	}
	sw.Stop();
	LOG_CORE_INFO("KD-Tree find all neighbors of all indices time: {}", sw.Time());
	LOG_CORE_INFO("KD-Tree find all neighbors of all indices time with construction: {}", sw.Time() + constructionTime);
}

void SpartialHashTests(uint32_t PointCount)
{
	LOG_CORE_INFO("--------------------------------------------------------------------------------------------------");
	LOG_CORE_INFO("-------------------------------------Spatial-Hash------------------------------------------------");
	LOG_CORE_INFO("--------------------------------------------------------------------------------------------------");
	LOG_CORE_INFO("Number of points: {}", PointCount);

	auto points = randomPoints(PointCount);

	Stopwatch sw;
	sw.Start();
	SpartialHash<3> spartialHash(points,100);
	sw.Stop();
	double constructionTime = sw.Time();
	LOG_CORE_INFO("Spatial hash constuction time: {}", sw.Time());

	sw.Start();
	uint32_t index = 0;
	bool found = spartialHash.FindNearestNeighbor(glm::vec3(20,41,4), index);
	sw.Stop();
	LOG_CORE_INFO("Spatial hash find closest index to point time: {}", sw.Time());


	sw.Start();
	std::vector<uint32_t> elements;
	found = spartialHash.FindNearestNeighbors(glm::vec3(0,0,0), 100*100,elements);
	sw.Stop();
	LOG_CORE_INFO("Spatial hash find nearest neighbors to point within radius time: {}", sw.Time());

	sw.Start();
	//find all neighbours of all elements
	std::vector<std::vector<uint32_t>> neighbors(points.size());
	#pragma omp parallel for
	for (int i = 0; i < points.size(); ++i)
	{
		std::vector<uint32_t> e;
		bool foundE = spartialHash.FindNearestNeighbors(points[i],100*100, e);
		neighbors[i] = e;
	}
	sw.Stop();
	LOG_CORE_INFO("Spatial hash find all neighbors of all indices time: {}", sw.Time());
	LOG_CORE_INFO("Spatial hash find all neighbors of all indices time with construction: {}", sw.Time() + constructionTime);

};

int main()
{
	Log::Init();
//
//	for (int i = 128; i <= 1048576; i *=2)
//	{
//		KDTests(i);
//	}
//
//	for (int i = 128; i <= 1048576; i *=2)
//	{
//		SpartialHashTests(i);
//	}


	//PlaneTests::NormalVectorTest();
	//AABBTests::IntersectionTests();

	auto points3 = TestData::Vec3Points256();
	auto points4 = TestData::Vec4Points256();
	auto randomPoints4f = randomPoints4(30720);
	//KD Tree
	KDTree<3> KDTree;
	KDTree.Build(points3);
	std::vector<uint32_t> indices;
	KDTree.FindNearestNeighbors(glm::vec3(0.0f,0.0f,0.0f),40.0f,indices);

	//Spatial hash
	SpartialHash<3> spartialHash(points3,50);
	spartialHash.Build(points3);
	std::vector<uint32_t> indices1;
	spartialHash.FindNearestNeighbors(glm::vec3(0.0f,0.0f,0.0f),40.0f,indices1);

	//OpenCL hash map tests
	OpenCLHashmapTests gpuHashmap;
	//Add 1% padding because if a point is max value the hash function exceeds the size.
	AABB m_aabb(glm::vec3(-110),glm::vec3(110));
	gpuHashmap.BuildTests(points4, m_aabb);
	gpuHashmap.NearestNeighborTests(points4, m_aabb);

	return 0;
}