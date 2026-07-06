#pragma once
#include "Types.h"
#include "Window.h"
#include <glm/vec2.hpp>
#include <memory>
#include <unordered_map>
#include "KeyCodes.h"

namespace Grass {

	class Input {
	private:
		struct KeyPressRecord { u32 Frame; i32 KeyState; };

		bool _isCursorVisible = false;
		Window& _window;

		glm::vec2 _mousePos;
		glm::vec2 _mouseDelta;

		std::unordered_map<KeyCode, KeyPressRecord> _recordedKeyStates;
		
		static void KeyboardKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

		float _scroll = 0;

	public:

		void Update(u32 frameCount);

		glm::vec2 MouseDelta() const;
		
		void EnableCursor();
		void DisableCursor();
		bool GetKeyDown(KeyCode key);
		bool GetKey(KeyCode key);
		bool GetKeyUp(KeyCode key);

		bool GetMouseKey(i32 key);

		float ScrollDelta() const { return _scroll; };

		bool IsCursorEnabled() const { return _isCursorVisible; }

		Input(Window& window);
	};
}