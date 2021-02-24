#include <util/Log.h>
#include <structures/KDTree.h>
#include <structures/SpartialHash.h>
#include "util/Stopwatch.h"
#include "PlaneTests.h"
#include "AABBTests.h"
#include <random>

std::vector<glm::vec3> randomPoints(size_t count)
{
	std::vector<glm::vec3> points(count);

	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int> dist(-100000, 100000);

	for (int i = 0; i < count; ++i)
	{
		points[i].x = dist(mt);
		points[i].y = dist(mt);
		points[i].z = dist(mt);
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
	size_t index = 0;
	bool found = tree.FindNearestNeighbor(glm::vec3(20,41,4), index);
	sw.Stop();
	LOG_CORE_INFO("KD-Tree find closest index to point time: {}", sw.Time());

	sw.Start();
	std::vector<size_t> elements;
	elements.reserve(1000);
	bool foundElements = tree.FindNearestNeighbors(glm::vec3(50,60,60),20*20, elements);
	sw.Stop();
	LOG_CORE_INFO("KD-Tree find nearest neighbors to point within radius time: {}", sw.Time());

	sw.Start();
	//find all neighbours of all elements
	std::vector<std::vector<size_t>> neighbors(points.size());
	#pragma omp parallel for
	for (int i = 0; i < points.size(); ++i)
	{
		std::vector<size_t> e;
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
	size_t index = 0;
	bool found = spartialHash.FindNearestNeighbor(glm::vec3(20,41,4), index);
	sw.Stop();
	LOG_CORE_INFO("Spatial hash find closest index to point time: {}", sw.Time());


	sw.Start();
	std::vector<size_t> elements;
	found = spartialHash.FindNearestNeighbors(glm::vec3(0,0,0), 100*100,elements);
	sw.Stop();
	LOG_CORE_INFO("Spatial hash find nearest neighbors to point within radius time: {}", sw.Time());

	sw.Start();
	//find all neighbours of all elements
	std::vector<std::vector<size_t>> neighbors(points.size());
	#pragma omp parallel for
	for (int i = 0; i < points.size(); ++i)
	{
		std::vector<size_t> e;
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
	AABBTests::IntersectionTests();

	return 0;
}