#pragma once

#include "Types.h"
#include "UniformBufferObject.h"
#include <glm/vec3.hpp>
#include "Mesh.h"
#include "Material.h"
#include "Renderer.h"

namespace Grass {
struct DirectionalLight
{
	glm::vec3 Direction;
	glm::vec3 DiffuseColor;
};

class LightingSystem
{
private:
	UniformBufferObject _unifromDataBuffer;
	Ref<Mesh> _skyboxMesh;
	Ref<Material> _skyboxMaterial;
	float _fogStart = 20;
	float _fogEnd = 80;

public:
	LightingSystem();
	
	DirectionalLight MainLight = {};
	glm::vec3 AmbientLight = {};
	
	void SetSkyboxMaterial(Ref<Material> material);
	void SetSkyboxMesh(Ref<Mesh> mesh) { _skyboxMesh = mesh; }

	void RenderSkybox(Renderer& renderer) const;
	void SetFogRange(float start, float end);
	glm::vec2 GetFogRange() const { return {_fogStart, _fogEnd }; }

	void UploadData(float deltaTime);
};
}