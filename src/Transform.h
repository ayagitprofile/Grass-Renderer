#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Grass {
class Transform
{
private:
	glm::vec3 _position;
	glm::vec3 _scale;
	glm::quat _rotation;

public:
	Transform();
	inline glm::vec3 GetPosition() const { return _position; }
	inline glm::vec3 GetScale() const { return _scale; }

	void SetPosition(glm::vec3 position);
	void SetPosition(float x, float y, float z);
	void SetScale(glm::vec3 scale);

	inline void Translate(glm::vec3 offset) { SetPosition(GetPosition() + offset); }

	void SetEulerAngles(float x, float y, float z);
	glm::vec3 GetEulerAngles() const;

	glm::vec3 Forward() const;
	glm::vec3 Right() const;
	glm::vec3 Up() const;

	glm::mat4 GetModelMatrix() const;
	glm::quat GetRotation() const { return _rotation; }
};
}

