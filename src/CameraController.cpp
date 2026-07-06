#include "CameraController.h"
#include "Const.h"

using namespace Grass;

static struct CameraUBOData
{
	glm::mat4 ViewMatrix;
	glm::mat4 ProjectionMatrix;
	glm::vec4 Position;
	glm::vec4 CameraForward;
	glm::mat4 VP;
	CameraPlanes CameraPlanes;
};

void Grass::CameraController::HandleMouseLook(float deltaTime) {
	const float sens = 0.1f;

	glm::vec2 mouseDelta = _input.MouseDelta() * (float)ControlCameraRotation;

	if (_input.GetKey(KeyCode::RIGHT))
		mouseDelta.x += 1000 * deltaTime;
	if (_input.GetKey(KeyCode::LEFT))
		mouseDelta.x -= 1000 * deltaTime;
	if (_input.GetKey(KeyCode::UP))
		mouseDelta.y -= 1000 * deltaTime;
	if (_input.GetKey(KeyCode::DOWN))
		mouseDelta.y += 1000 * deltaTime;

	_pitch -= mouseDelta.y * sens;
	_pitch = glm::clamp(_pitch, -89.f, 89.f);

	_yaw -= mouseDelta.x * sens;
	_yaw = glm::mod(_yaw, 360.f);

	_camera.Transform.SetEulerAngles(_pitch, _yaw, 0.f);
}

static glm::vec3 ApplyFriction(glm::vec3 velocity, float friction, float dt) {
	//Vector3 horizontal = player.velocity;
	//glm::vec3 horizontal = { velocity.x, 0, velocity.z };
	const float stopSpeed = 0.1f;

	float speed = glm::length(velocity);

	if (speed < 0.01f)
		return { 0, 0, 0};

	float control = speed < stopSpeed ? stopSpeed : speed;
	//float control = speed;

	float drop = control * friction * dt;

	float newSpeed = speed - drop;

	if (newSpeed < 0.0f)
		newSpeed = 0.0f;

	newSpeed /= speed;

	return velocity * newSpeed;

	//player.velocity.x *= newSpeed;
	//player.velocity.z *= newSpeed;
}

static glm::vec3 Accelerate(glm::vec3 velocity, glm::vec3 wishDir, float wishSpeed, float accel, float dt) {
	
	float currentSpeed = glm::dot(velocity, wishDir);

	float addSpeed = wishSpeed - currentSpeed;

	if (addSpeed <= 0.0f)
		return velocity;

	float accelSpeed = accel * wishSpeed * dt;

	if (accelSpeed > addSpeed)
		accelSpeed = addSpeed;

	return velocity + wishDir * accelSpeed;
}

void Grass::CameraController::HandleMovement(float deltaTime) {
	const float speed = _input.GetKey(KeyCode::LEFT_SHIFT) ? 30.f : _input.GetKey(KeyCode::LEFT_CONTROL) ? 4.0f : 7.f;

	static glm::vec3 velocity = { 0, 0, 0 };

	glm::vec3 movement = glm::vec3(0, 0, 0);

	if (_input.GetKey(KeyCode::A))
		movement.x -= 1;
	if (_input.GetKey(KeyCode::D))
		movement.x += 1;
	if (_input.GetKey(KeyCode::W))
		movement.z -= 1;
	if (_input.GetKey(KeyCode::S))
		movement.z += 1;
	if (_input.GetKey(KeyCode::E))
		movement.y += 1;
	if (_input.GetKey(KeyCode::Q))
		movement.y -= 1;

	if (glm::dot(movement, movement) > 0.01)
		movement = glm::normalize(movement);

	const float friction = 8;
	const float accel = 10;

	glm::vec3 wishDir = _camera.Transform.Right() * movement.x + _camera.Transform.Forward() * movement.z + _camera.Transform.Up() * movement.y;

	if (glm::dot(wishDir, wishDir) > 0.01)
		wishDir = glm::normalize(wishDir);

	velocity = ApplyFriction(velocity, friction, deltaTime);
	velocity = Accelerate(velocity, wishDir, speed, accel, deltaTime);

	_camera.Transform.Translate(velocity * deltaTime);

	glm::vec3 camPos = _camera.Transform.GetPosition();

	camPos.y = glm::clamp(camPos.y, _boundsMin.y, _boundsMax.y);
	camPos.x = glm::clamp(camPos.x, _boundsMin.x, _boundsMax.x);
	camPos.z = glm::clamp(camPos.z, _boundsMin.z, _boundsMax.z);

	_camera.Transform.SetPosition(camPos);
}

void Grass::CameraController::UploadCameraData() {
	CameraUBOData data{};
	data.CameraForward = glm::vec4(_camera.Transform.Forward(), 0.f);
	data.ProjectionMatrix = _camera.GetProjectionMatrix();
	data.ViewMatrix = _camera.GetViewMatrix();
	data.Position = glm::vec4(_camera.Transform.GetPosition(), 0.0f);
	data.VP = _camera.GetProjectionMatrix() * _camera.GetViewMatrix();
	data.CameraPlanes = _camera.GetFrustumPlanes();

	_unifromDataBuffer.UpdateData(&data, sizeof(CameraUBOData));
}

void Grass::CameraController::Update(float deltaTime) {
	HandleMouseLook(deltaTime);
	HandleMovement(deltaTime);
	UploadCameraData();
}

void Grass::CameraController::AddYaw(float yaw) {
	_yaw += yaw;
}

void Grass::CameraController::AddPitch(float pitch) {
	_pitch += pitch;
}

Grass::CameraController::CameraController(Camera& camera, Input& input)
	: _camera(camera), _input(input), _unifromDataBuffer(Const::CameraUBOBindingIndex)
{
	CameraUBOData data = {};
	_unifromDataBuffer.SetData(&data, sizeof(data));
}
