#pragma once

#include "Window.h"
#include "Camera.h"
#include "UniformBufferObject.h"
#include <memory>

namespace Grass {
class App
{
private:
	u32 _frameCount = 0;
	bool _initializedSuccessfully;
	Grass::Window _window;
	std::shared_ptr<Camera> _camera;
	UniformBufferObject _uniformBuffer;

public:
	bool RenderGrass = true;
	App();
	int Run();
	~App();
};
}