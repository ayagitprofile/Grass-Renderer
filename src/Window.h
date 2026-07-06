#pragma once

#include "glfwNoOpengl.h"
#include "Types.h"
#include <optional>
#include <functional>

namespace Grass {
class Window
{
private:
	GLFWwindow* _windowHandle;
	i32 _width, _height;
	bool _isOpen = false;

public:
	Window();
	Window(i32 width, i32 height);

	i32 GetWidth() const { return _width; }
	i32 GetHeight() const { return _height; } 

	bool GetKey(Key key);
	bool IsOpen();
	void Close() const;
	void SwapBuffers();
	void Clear();
	void* GetResourceHandle() const { return _windowHandle; }
	void SetTitle(const std::string& title);
	void SetResizeHandler(std::function<void(int, int)> handler);
};
}
