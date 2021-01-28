#include "Renderer.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cassert>

Renderer::Renderer(uint32_t viewportWidth, uint32_t viewportHeight)
{
	assert(gladLoadGLLoader((GLADloadproc) glfwGetProcAddress));

	// Define the viewport dimensions
	glViewport(0, 0, viewportWidth, viewportHeight);
}

void Renderer::DrawFrame()
{
	// Render
	// Clear the colorbuffer
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}
