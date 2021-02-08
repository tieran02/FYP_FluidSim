#include "platform/Window.h"
#include "renderer/Renderer.h"
#include "Simulation.h"
#include <util/Log.h>
#include <structures/KDTree.h>

std::vector<glm::vec3> randomPoints(size_t count)
{
	std::vector<glm::vec3> points(count);
	for (int i = 0; i < count; ++i)
	{
		points[i].x = 0 + rand() % (( 100 + 1 ) - 0);
		points[i].y = 0 + rand() % (( 100 + 1 ) - 0);
		points[i].z = 0 + rand() % (( 100 + 1 ) - 0);
	}
	return points;
}

int main()
{
	Log::Init();

	std::vector<glm::vec3> points = randomPoints(2000);
	KDTree tree(points,1);

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
