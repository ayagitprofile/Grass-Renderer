#include "Input.h"
#include "Types.h"

using namespace Grass;

static Input* s_inputInstance = nullptr;
static u32 s_frameCount = 0;

static constexpr Grass::i32 KEY_PRESSED = GLFW_PRESS;
static GLFWwindow* ToGLFWWindow(const Grass::Window& window) {
	return (GLFWwindow*)window.GetResourceHandle();
}

static glm::vec2 GetMousePos(GLFWwindow* window) {
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	return { x, y };
}

void Grass::Input::KeyboardKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (!(action == GLFW_PRESS || action == GLFW_RELEASE))
		return;

	KeyPressRecord record;
	record.Frame = s_frameCount;
	record.KeyState = action;
	s_inputInstance->_recordedKeyStates[(KeyCode)key] = record;
}

void Grass::Input::MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	s_inputInstance->_scroll = yoffset;
}

void Grass::Input::Update(u32 frameCount) {
	_scroll = 0;
	double x, y;
	glfwGetCursorPos(ToGLFWWindow(_window), &x, &y);

	_mouseDelta = glm::vec2(x, y) - _mousePos;

	_mousePos.x = x;
	_mousePos.y = y;

	s_frameCount = frameCount;
}

glm::vec2 Grass::Input::MouseDelta() const {
	//return _isCursorVisible ? glm::vec2(0) : _mouseDelta;
	return _mouseDelta;
}

void Grass::Input::EnableCursor() {
	_isCursorVisible = true;
	glfwSetInputMode(ToGLFWWindow(_window), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetInputMode(ToGLFWWindow(_window), GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
}

void Grass::Input::DisableCursor() {
	_isCursorVisible = false;
	glfwSetInputMode(ToGLFWWindow(_window), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetInputMode(ToGLFWWindow(_window), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
}

bool Grass::Input::GetKeyDown(KeyCode key) {
	if (_recordedKeyStates.contains(key) == false)
		return false;
	
	KeyPressRecord record = _recordedKeyStates[key];

	if (record.KeyState == GLFW_RELEASE)
		return false;

	u32 frameDifference = s_frameCount - record.Frame;

	return frameDifference == 1;
}

bool Grass::Input::GetKey(KeyCode key) {
	return glfwGetKey(ToGLFWWindow(_window), (u32)key) != GLFW_RELEASE;
}

bool Grass::Input::GetKeyUp(KeyCode key) {
	if (_recordedKeyStates.contains(key) == false)
		return false;

	KeyPressRecord record = _recordedKeyStates[key];

	if (record.KeyState == GLFW_PRESS)
		return false;

	u32 frameDifference = s_frameCount - record.Frame;

	return frameDifference == 1;
}

bool Grass::Input::GetMouseKey(i32 key) {
	return glfwGetMouseButton(ToGLFWWindow(_window), key);
}

Grass::Input::Input(Window& window)
	: _window(window), _mouseDelta(glm::vec2(0, 0))
{
	DisableCursor();
	_mousePos = GetMousePos(ToGLFWWindow(window));

	s_inputInstance = this;

	glfwSetKeyCallback(ToGLFWWindow(window), KeyboardKeyCallback);
	glfwSetScrollCallback(ToGLFWWindow(window), MouseScrollCallback);
}
