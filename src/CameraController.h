#pragma once

#include "Camera.h"
#include "Input.h"
#include "UniformBufferObject.h"
#include <glm/vec3.hpp>
namespace Grass {

class CameraController
{
private:
	UniformBufferObject _unifromDataBuffer;
	Camera& _camera;
	Input& _input;

	float _yaw = 0.f;
	float _pitch = 0.f;

	void HandleMouseLook(float);
	void HandleMovement(float);
	void UploadCameraData();

	glm::vec3 _boundsMin = {-50, 0, -50 };
	glm::vec3 _boundsMax = { 50, 100, 50 };

public:
	bool ControlCameraRotation = true;
	void AddYaw(float yaw);
	void AddPitch(float pitch);
	CameraController(Camera& camera, Input& input);
	void Update(float deltaTime);
	void SetConstrainingBounds(glm::vec3 min, glm::vec3 max) { _boundsMin = min, _boundsMax = max; };
};
}