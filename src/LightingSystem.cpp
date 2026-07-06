#include "LightingSystem.h"
#include "Const.h"

#include <glm/vec4.hpp>

using namespace Grass;

// Opengl uniform buffer uses layout std140, in which vector3 is 16 bytes, so for each vector a 4 byte padding is needed

static struct LightData
{
	glm::vec4 DirectionalLightDirection;
	glm::vec4 DirectionalLightColor;
	glm::vec4 AmbientLight;
	glm::vec4 FogRange;
};

Grass::LightingSystem::LightingSystem() 
	: _unifromDataBuffer(Const::LightDataUBOBindingIndex)
{
	LightData data{};
	_unifromDataBuffer.SetData(&data, sizeof(data));
}

void Grass::LightingSystem::SetSkyboxMaterial(Ref<Material> material) {
	_skyboxMaterial = material;
	_skyboxMaterial->FaceCullingMode = FaceCullingMode::Front;
	_skyboxMaterial->DepthWriting = true;
	_skyboxMaterial->DepthTesting = true;
	_skyboxMaterial->DepthTestFunction = DepthTestFunction::LessEqual;
}

void Grass::LightingSystem::RenderSkybox(Renderer& renderer) const {
	renderer.RenderMesh(*_skyboxMesh, *_skyboxMaterial, glm::mat4(1.0f));
}

void Grass::LightingSystem::SetFogRange(float start, float end) {
	if (start > end)
		start = end;
	if (end < start)
		end = start;

	_fogStart = start;
	_fogEnd = end;
}

void Grass::LightingSystem::UploadData(float deltaTime) {
	LightData data = {};
	static float time = 0;

	time += deltaTime;

	data.DirectionalLightDirection = glm::vec4(MainLight.Direction, 0.0f);
	data.DirectionalLightColor = glm::vec4(MainLight.DiffuseColor, 0.0f);
	data.AmbientLight = glm::vec4(AmbientLight, 0.0f);
	data.FogRange = glm::vec4(_fogStart, _fogEnd, time, deltaTime);
	
	_unifromDataBuffer.UpdateData(&data, sizeof(data));
}
