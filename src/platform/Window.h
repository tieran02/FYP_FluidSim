#pragma once

#ifdef _WIN32
#include <windows.h>
#else
#define APIENTRY
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "glad/gl.h"


#include <string>
#include <functional>

class Window
{
 public:
	Window(std::string&& title, uint32_t width, uint32_t height);
	~Window();
	void Run(const std::function<void()>& draw);
	void Close();

	static uint32_t Width();
	static uint32_t Height();
 private:
	void init();
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
	static const Window* m_instance;

	GLFWwindow* m_window{};
	std::string m_title;
	uint32_t m_width, m_height;
	std::function<void()> m_updateFunc;
};
