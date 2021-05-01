#pragma once
#include <glad/gl.h>

#ifdef _WIN32
#include <windows.h>
#else
#define APIENTRY
#endif

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <string>
#include <functional>

using WindowKeyCallBack = std::function<void(int,int,int)>;
using WindowCursorCallBack = std::function<void(double,double)>;
using WindowMouseButtonCallBack = std::function<void(int,int,int)>;

class Window
{
 public:
	Window(std::string&& title, uint32_t width, uint32_t height);
	~Window();
	void Run(const std::function<void()>& draw);
	void Close();

	static GLFWwindow* GetGLFWwindow();
	static uint32_t Width();
	static uint32_t Height();

	void SetKeyCallback(const WindowKeyCallBack& keyCallback);
	void SetCursorCallback(const WindowCursorCallBack& cursorCallback);
	void SetMouseButtonCallback(const WindowMouseButtonCallBack& mosueButtonCallback);
 private:
	void init();
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
	static void cursor_callback(GLFWwindow* window, double xPos, double yPos);
	static void mouseButton_callback(GLFWwindow* window, int button, int action, int mode);

	static const Window* m_instance;

	GLFWwindow* m_window{};
	std::string m_title;
	uint32_t m_width, m_height;
	std::function<void()> m_updateFunc;
	WindowKeyCallBack m_keyCallback;
	WindowCursorCallBack m_cursorCallback;
	WindowMouseButtonCallBack m_mouseButtonCallBack;
};
