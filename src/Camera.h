#pragma once

#include "Transform.h"
#include <array>

namespace Grass {

struct FrustumPlane
{
	glm::vec3 normal;
	float d;

	float distance(const glm::vec3& p) const {
		return glm::dot(normal, p) + d;
	}
};

struct AABB {
	glm::vec3 min;
	glm::vec3 max;
};

typedef std::array<FrustumPlane, 6> CameraPlanes;

class Camera
{
private:
	float _aspectRatio;
	float _fov;
	float _zNear, _zFar;

	glm::mat4 _projection;

	void RecalculateProjectionMatrix();

public:
	static CameraPlanes CalculateFrustumPlanes(const glm::mat4& VPMatrix);
	CameraPlanes GetFrustumPlanes() const;
	Camera(float fov, float aspectRatio);
	Transform Transform;

	static bool IntersectFrustrumAABB(AABB aabb, const CameraPlanes& planes);

	void SetClipRange(float zNear, float zFar);
	
	glm::mat4 GetProjectionMatrix() const;
	
	void SetAspectRatio(float aspectRatio);
	float GetAspectRatio() const { return _aspectRatio; }
	
	void SetFov(float fov);
	float GetFov() const { return _fov; }

	glm::vec2 GetClippingRange() const { return { _zNear, _zFar }; }

	glm::mat4 GetViewMatrix() const;
};
}

