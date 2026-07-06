#include "Window.h"
#include <iostream>
#include <glad/glad.h>

using namespace Grass;

static std::function<void(int, int)> s_onResize = nullptr;

Window::Window()
: _width(0), _height(0), _windowHandle(nullptr), _isOpen(false) 
{
}

Window::Window(i32 width, i32 height) : _width(width), _height(height) {
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 8);

	_windowHandle = glfwCreateWindow(_width, _height, "Window", nullptr, nullptr);

	if (_windowHandle == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		_isOpen = false;
		return;
	}

	glfwMakeContextCurrent(_windowHandle);

	glfwSetFramebufferSizeCallback(_windowHandle, [](GLFWwindow* window, int width, int height) {
		glViewport(0, 0, width, height);
		if (s_onResize)
			s_onResize(width, height);
	});
	
	_isOpen = true;
}

bool Window::GetKey(Key key) {
	return 	glfwGetKey(_windowHandle, GLFW_KEY_ESCAPE) == GLFW_PRESS;
}

bool Window::IsOpen(){
	_isOpen = !glfwWindowShouldClose(_windowHandle);
	return _isOpen;
}

void Window::Close() const {
	glfwSetWindowShouldClose(_windowHandle, true);
}

void Window::SwapBuffers() {
	glfwSwapBuffers(_windowHandle);
}

void Window::Clear() {
	//glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glClear(GL_DEPTH_BUFFER_BIT);
}
void Window::SetTitle(const std::string& title) {
	glfwSetWindowTitle(_windowHandle, title.c_str());
}
void Window::SetResizeHandler(std::function<void(int, int)> handler) {
	s_onResize = handler;
}

