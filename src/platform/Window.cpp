#include "Window.h"
#include <iostream>
#include <assert.h>

Window::Window(std::string&& title, uint32_t width, uint32_t height, std::function<void()>&& update) :
	m_width(width),
	m_height(height),
	m_updateFunc(update)
{
	std::cout << "Starting GLFW context, OpenGL 3.3" << std::endl;
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Create a GLFW window object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(m_width, m_height, title.c_str(), NULL, NULL);
	glfwMakeContextCurrent(window);

	assert(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);

	assert(gladLoadGLLoader((GLADloadproc) glfwGetProcAddress));

	// Define the viewport dimensions
	glViewport(0, 0, m_width, m_height);

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Check if any events have been activated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();

		if(update)
			update();

		// Swap the screen buffers
		glfwSwapBuffers(window);
	}

	// Terminates GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
}

void Window::Close()
{
	glfwSetWindowShouldClose(m_window,true);
}

void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	std::cout << key << std::endl;
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}
