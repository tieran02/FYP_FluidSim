#include "platform/Window.h"
#include "renderer/Renderer.h"
#include "Simulation.h"
#include <util/Log.h>
#include <structures/KDTree.h>
#include "util/Stopwatch.h"

std::vector<KDPair<3,size_t>> randomPoints(size_t count)
{
	std::vector<KDPair<3,size_t>> points(count);
	for (int i = 0; i < count; ++i)
	{
		points[i].first.x = 0 + rand() % (( 100 + 1 ) - 0);
		points[i].first.y = 0 + rand() % (( 100 + 1 ) - 0);
		points[i].first.z = 0 + rand() % (( 100 + 1 ) - 0);
		points[i].second = i;
	}
	return points;
}

int main()
{
	Log::Init();

	std::vector<KDPair<3,size_t>> points = randomPoints(100000);

	Stopwatch sw;

	sw.Start();
	KDTree<3,size_t> tree(points, 1);
	sw.Stop();
	LOG_CORE_INFO(sw.Time());

	sw.Start();
	size_t index = 0;
	bool found = tree.FindNearestNeighbor(glm::vec3(20,41,4), index);

	std::vector<size_t*> elements;
	bool foundElements = tree.FindNearestNeighbors(glm::vec3(20,41,4),5, elements);
	sw.Stop();
	LOG_CORE_INFO(sw.Time());

	/*constexpr uint32_t WIDTH{1920}, HEIGHT{1080};
	Window window{"Fluid Simulation", WIDTH, HEIGHT};
	Renderer renderer{WIDTH,HEIGHT};

	Simulation simulation(renderer);

	window.SetKeyCallback(std::bind(&Simulation::KeyCallback, &simulation,
		std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
	window.SetCursorCallback(std::bind(&Simulation::CursorCallback, &simulation,
		std::placeholders::_1,std::placeholders::_2));
	window.SetMouseButtonCallback(std::bind(&Simulation::MouseButtonCallback, &simulation,
		std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
	window.Run(std::bind(&Simulation::Update, &simulation));*/
}
