#include "Transform.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

Grass::Transform::Transform()
	: _position(glm::vec3(0.0f)), _scale(glm::vec3(1.0f, 1.0f, 1.0f)), _rotation(glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)))
{
}

void Grass::Transform::SetPosition(glm::vec3 position) {
	_position = position;
}

void Grass::Transform::SetPosition(float x, float y, float z) {
	_position = { x, y, z };
}

void Grass::Transform::SetScale(glm::vec3 scale) {
	_scale = scale;
}

void Grass::Transform::SetEulerAngles(float x, float y, float z) {
	_rotation = glm::quat(glm::radians(glm::vec3(x, y, z)));
}

glm::vec3 Grass::Transform::GetEulerAngles() const {
	return glm::degrees(glm::eulerAngles(_rotation));
}

glm::vec3 Grass::Transform::Forward() const {
	return _rotation * glm::vec3(0, 0, 1.0f);
}

glm::vec3 Grass::Transform::Right() const {
	return _rotation * glm::vec3(1.0f, 0, 0);
}

glm::vec3 Grass::Transform::Up() const {
	return _rotation * glm::vec3(0, 1.0f, 0);
}

glm::mat4 Grass::Transform::GetModelMatrix() const {
	glm::mat4 modelMatrix = glm::mat4(1.0f);

	modelMatrix = glm::translate(modelMatrix, _position);
	modelMatrix *= glm::toMat4(_rotation);
	modelMatrix = glm::scale(modelMatrix, _scale);

	return modelMatrix;
}
