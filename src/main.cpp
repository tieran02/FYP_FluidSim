#include <iostream>
#include "platform/Window.h"

void update();

int main()
{
	Window window("Fluid Simulation", 1280, 720, update);
}

void update()
{
	// Render
	// Clear the colorbuffer
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}
