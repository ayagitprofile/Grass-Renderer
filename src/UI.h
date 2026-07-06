#pragma once

#include <imgui.h>
#include "Input.h"
#include "GrassRenderer.h"
#include "LightingSystem.h"
#include "CameraController.h"
#include "App.h"
#include "Camera.h"

namespace Grass {
class UI
{
private:

	bool _isVisible = false;
	bool _rotateSun = true;
	
	ImGuiIO& _imGuiIO;
	LightingSystem& _lightingSystem;
	GrassRenderer& _grassRenderer;
	CameraController& _camCon;
	App& _app;
	Camera& _camera;

	float _deltaTime = 0;
	float _time = 0;
	
	float _windAngle = 0;
	float _windSpeed = 1;
	glm::vec2 _fogRange = {};
	glm::vec2 _sunEulerAngles = { 0.3, 0 };

	glm::vec3 _sunColor;
	glm::vec3 _ambientLightColor;

	float _sunLightStrength = 1.0;
	float _fov = 0;
	
public:
	UI(ImGuiIO& io, LightingSystem& ls, GrassRenderer& gr, App& app, Camera& camera, CameraController& cc);
	void Update(Input& input, float deltaTime);
	void Render();
};
}