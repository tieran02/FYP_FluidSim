#include "platform/Window.h"
#include "renderer/Renderer.h"
#include "Simulation.h"
#include <util/Log.h>

void update();

int main()
{
	Log::Init();

	constexpr uint32_t WIDTH{1280}, HEIGHT{720};
	Window window{"Fluid Simulation", WIDTH, HEIGHT};
	Renderer renderer{WIDTH,HEIGHT};

	Simulation simulation(renderer);

	window.Run(std::bind(&Simulation::Update, &simulation));
}
