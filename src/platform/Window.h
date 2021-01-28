#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <functional>

class Window
{
 public:
	Window(std::string&& title, uint32_t width, uint32_t height, std::function<void()>&& update);
	~Window();
	void Run();
	void Close();
 private:
	void init();
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

	GLFWwindow* m_window;
	std::string m_title;
	uint32_t m_width, m_height;
	std::function<void()> m_updateFunc;
};
