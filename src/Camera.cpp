#include "Camera.h"
using namespace Grass;

glm::mat4 Grass::Camera::GetViewMatrix() const {
	glm::vec3 position = Transform.GetPosition();
	//glm::mat4 viewMat = glm::lookAtRH(position, position + Transform.Forward(), glm::vec3());

	return glm::lookAtLH(position, position + Transform.Forward(), Transform.Up());
}

void Grass::Camera::RecalculateProjectionMatrix() {
	_projection = glm::perspective(glm::radians(_fov), _aspectRatio, _zNear, _zFar);
}

CameraPlanes Grass::Camera::CalculateFrustumPlanes(const glm::mat4& VPMatrix) {
	std::array<FrustumPlane, 6> planes{};
	
	const glm::mat4& clip = VPMatrix;
	
	// Left
	planes[0].normal = glm::vec3(
		clip[0][3] + clip[0][0],
		clip[1][3] + clip[1][0],
		clip[2][3] + clip[2][0]
	);
	planes[0].d = clip[3][3] + clip[3][0];
	
	// Right
	planes[1].normal = glm::vec3(
		clip[0][3] - clip[0][0],
		clip[1][3] - clip[1][0],
		clip[2][3] - clip[2][0]
	);
	planes[1].d = clip[3][3] - clip[3][0];
	
	// Bottom
	planes[2].normal = glm::vec3(
		clip[0][3] + clip[0][1],
		clip[1][3] + clip[1][1],
		clip[2][3] + clip[2][1]
	);
	planes[2].d = clip[3][3] + clip[3][1];
	
	// Top
	planes[3].normal = glm::vec3(
		clip[0][3] - clip[0][1],
		clip[1][3] - clip[1][1],
		clip[2][3] - clip[2][1]
	);
	planes[3].d = clip[3][3] - clip[3][1];
	
	// Near
	planes[4].normal = glm::vec3(
		clip[0][3] + clip[0][2],
		clip[1][3] + clip[1][2],
		clip[2][3] + clip[2][2]
	);
	planes[4].d = clip[3][3] + clip[3][2];
	
	// Far
	planes[5].normal = glm::vec3(
		clip[0][3] - clip[0][2],
		clip[1][3] - clip[1][2],
		clip[2][3] - clip[2][2]
	);
	planes[5].d = clip[3][3] - clip[3][2];
	
	// Normalize planes
	for (auto& p : planes) {
		float len = glm::length(p.normal);
		p.normal /= len;
		p.d /= len;
	}
	
	return planes;
}

std::array<FrustumPlane, 6> Grass::Camera::GetFrustumPlanes() const {
	std::array<FrustumPlane, 6> planes{};

	glm::mat4 clip = GetProjectionMatrix() * GetViewMatrix();

	// Left
	planes[0].normal = glm::vec3(
		clip[0][3] + clip[0][0],
		clip[1][3] + clip[1][0],
		clip[2][3] + clip[2][0]
	);
	planes[0].d = clip[3][3] + clip[3][0];

	// Right
	planes[1].normal = glm::vec3(
		clip[0][3] - clip[0][0],
		clip[1][3] - clip[1][0],
		clip[2][3] - clip[2][0]
	);
	planes[1].d = clip[3][3] - clip[3][0];

	// Bottom
	planes[2].normal = glm::vec3(
		clip[0][3] + clip[0][1],
		clip[1][3] + clip[1][1],
		clip[2][3] + clip[2][1]
	);
	planes[2].d = clip[3][3] + clip[3][1];

	// Top
	planes[3].normal = glm::vec3(
		clip[0][3] - clip[0][1],
		clip[1][3] - clip[1][1],
		clip[2][3] - clip[2][1]
	);
	planes[3].d = clip[3][3] - clip[3][1];

	// Near
	planes[4].normal = glm::vec3(
		clip[0][3] + clip[0][2],
		clip[1][3] + clip[1][2],
		clip[2][3] + clip[2][2]
	);
	planes[4].d = clip[3][3] + clip[3][2];

	// Far
	planes[5].normal = glm::vec3(
		clip[0][3] - clip[0][2],
		clip[1][3] - clip[1][2],
		clip[2][3] - clip[2][2]
	);
	planes[5].d = clip[3][3] - clip[3][2];

	// Normalize planes
	for (auto& p : planes) {
		float len = glm::length(p.normal);
		p.normal /= len;
		p.d /= len;
	}

	return planes;
}

Grass::Camera::Camera(float fov, float aspectRatio)
	: _fov(fov), _aspectRatio(aspectRatio), _zNear(0.1f), _zFar(100.f)
{
	RecalculateProjectionMatrix();
}

bool Grass::Camera::IntersectFrustrumAABB(AABB aabb, const CameraPlanes& planes) {
	for (int i = 0; i < 6; i++) {
		const FrustumPlane& p = planes[i];

		// Select "positive vertex" (farthest in direction of plane normal)
		glm::vec3 positiveVertex = {
			(p.normal.x >= 0) ? aabb.max.x : aabb.min.x,
			(p.normal.y >= 0) ? aabb.max.y : aabb.min.y,
			(p.normal.z >= 0) ? aabb.max.z : aabb.min.z
		};

		// If positive vertex is outside, box is completely outside
		if (p.distance(positiveVertex) < 0)
			return false;
	}

	return true; // intersects or fully inside
}

void Grass::Camera::SetClipRange(float zNear, float zFar) {
	_zNear = zNear;
	_zFar = zFar;

	RecalculateProjectionMatrix();
}

glm::mat4 Grass::Camera::GetProjectionMatrix() const {
	return _projection;
}

void Grass::Camera::SetAspectRatio(float aspectRatio) {
	_aspectRatio = aspectRatio;
	RecalculateProjectionMatrix();
}

void Grass::Camera::SetFov(float fov) {
	_fov = fov;
	RecalculateProjectionMatrix();
}
