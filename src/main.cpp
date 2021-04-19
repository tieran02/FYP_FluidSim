#include "platform/Window.h"
#include "renderer/Renderer.h"
#include "Simulation.h"
#include <util/Log.h>

int main()
{
	Log::Init();

	constexpr uint32_t WIDTH{1920}, HEIGHT{1080};
	Window window{"Fluid Simulation", WIDTH, HEIGHT};
	Simulation simulation;
	simulation.Init();

	window.SetKeyCallback(std::bind(&Simulation::KeyCallback, &simulation,
		std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
	window.SetCursorCallback(std::bind(&Simulation::CursorCallback, &simulation,
		std::placeholders::_1,std::placeholders::_2));
	window.SetMouseButtonCallback(std::bind(&Simulation::MouseButtonCallback, &simulation,
		std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
	window.Run(std::bind(&Simulation::Update, &simulation));
}
