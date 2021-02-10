#include <util/Log.h>
#include <structures/KDTree.h>
#include <structures/SpartialHash.h>
#include "util/Stopwatch.h"
#include <random>

std::vector<glm::vec3> randomPoints(size_t count)
{
	std::vector<glm::vec3> points(count);

	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int> dist(-10000, 10000);

	for (int i = 0; i < count; ++i)
	{
		points[i].x = dist(mt);
		points[i].y = dist(mt);
		points[i].z = dist(mt);
	}
	return points;
}

int main()
{
	Log::Init();

	constexpr uint32_t POINT_COUNT{10000};
	auto points = randomPoints(POINT_COUNT);

	LOG_CORE_INFO("Number of points: {}", POINT_COUNT);
	Stopwatch sw;

	sw.Start();
	KDTree<3> tree(points, 1);
	sw.Stop();
	LOG_CORE_INFO("KD-Tree constuction time: {} seconds", sw.Time());

	sw.Start();
	size_t index = 0;
	bool found = tree.FindNearestNeighbor(glm::vec3(20,41,4), index);

	std::vector<size_t> elements;
	bool foundElements = tree.FindNearestNeighbors(glm::vec3(50,60,60),20*20, elements);
	sw.Stop();
	LOG_CORE_INFO("KD-Tree find closest index to point time: {} seconds", sw.Time());

	sw.Start();
	//find all neighbours of all elements
	std::vector<std::vector<size_t>> neighbors(points.size());
	#pragma omp parallel for
	for (int i = 0; i < points.size(); ++i)
	{
		std::vector<size_t> e;
		bool foundE = tree.FindNearestNeighbors(points[i],5000*5000, e);
		neighbors[i] = e;
	}
	sw.Stop();
	LOG_CORE_INFO("KD-Tree find all neighbors of all indices time: {} seconds", sw.Time());

	sw.Start();
	SpartialHash<3> spartialHash(points,100);
	sw.Stop();
	LOG_CORE_INFO("Spartial hash constuction time: {} seconds", sw.Time());

	return 0;
}