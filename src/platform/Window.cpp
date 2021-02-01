#include "Window.h"
#include <iostream>
#include <glad/glad.h>
#include <util/Log.h>

Window::Window(std::string&& title, uint32_t width, uint32_t height) :
	m_title{ title },
	m_width{ width },
	m_height{ height },
	m_window{ nullptr}
{
	LOG_CORE_INFO("Starting GLFW context");
	init();
}


Window::~Window()
{
	// Terminates GLFW, clearing any resources allocated by GLFW.
	LOG_CORE_INFO("Cleaning GLFW context");
	glfwTerminate();
}

void Window::Run(const std::function<void()>& draw)
{
	// Game loop
	while (!glfwWindowShouldClose(m_window))
	{
		// Check if any events have been activated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();

		if(draw)
			draw();

		// Swap the screen buffers
		glfwSwapBuffers(m_window);
	}
}

void Window::Close()
{
	glfwSetWindowShouldClose(m_window,true);
}

void Window::init()
{
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Create a GLFW window object that we can use for GLFW's functions
	m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), NULL, NULL);
	glfwMakeContextCurrent(m_window);

	CORE_ASSERT(m_window, "Failed to init GLFW window");

	// Set the required callback functions
	glfwSetKeyCallback(m_window, key_callback);
}

void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	std::cout << key << std::endl;
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}
