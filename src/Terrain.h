#pragma once

#include "Mesh.h"
#include "Material.h"
#include "Renderer.h"
#include "Transform.h"
#include "Model.h"

namespace Grass {

class Terrain
{
private:
	Mesh _terrainMesh;
	Model _cube;
	Mesh _waterMesh;
	Material _renderingMaterial;
	Material _waterMaterial;
	std::vector<glm::vec4> _terrainTextureCPUSide;
	Ref<Texture> _terrainHeight = nullptr;
	Ref<Texture> _terrainNormal = nullptr;
	glm::vec2 _area = { 100, 100 };
	glm::vec3 _center = { 0, 0, 0 };
public:
	Transform Transform;
	Terrain(glm::vec2 area);
	glm::vec3 GetCenter() const { return { _center }; };
	glm::vec2 GetArea() const { return { _area.x, _area.y }; };
	void Render(Renderer& renderer);
	Ref<Texture> GetTerrainHeightTexture() const { return _terrainHeight; }
};
}