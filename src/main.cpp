#include <iostream>
#include "platform/Window.h"
#include "Renderer.h"

void update();

int main()
{
	constexpr uint32_t WIDTH{1280}, HEIGHT{720};
	Window window{"Fluid Simulation", WIDTH, HEIGHT};
	Renderer renderer{WIDTH,HEIGHT};

	window.Run(std::bind(&Renderer::Draw, &renderer));
}
