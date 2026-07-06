#include "Terrain.h"
#include "Const.h"
#include <stb_image/stb_image.h>
#include <iostream>

#if _DEBUG
constexpr auto TERRAIN_MESH = "terrain/unitPlane.obj";
#else
constexpr auto TERRAIN_MESH = "terrain/unitPlaneHP.obj";
#endif

Grass::Terrain::Terrain(glm::vec2 area) 
	: 
	_area(area),
	_renderingMaterial(Path(Const::ShadersPath) / "TerrainShader.glsl"),
	_waterMesh(Path(Const::MeshesPath) / "terrain/unitPlane.obj", MeshCreationFlags::CalculateTangents),
	_terrainMesh(Path(Const::MeshesPath) / TERRAIN_MESH, MeshCreationFlags::CalculateTangents)
{
	_center = { 0, 0,0 };

	_renderingMaterial.SetTexture("u_colorTexture", std::make_shared<Texture>(Path(Const::TexturesPath) / "grass/color.jpg"));
	_renderingMaterial.SetTexture("u_normalMap", std::make_shared<Texture>(Path(Const::TexturesPath) / "grass/normal.jpg"));

	_renderingMaterial.SetTexture("u_sandColorMap", std::make_shared<Texture>(Path(Const::TexturesPath) / "sand/color.jpg"));
	_renderingMaterial.SetTexture("u_sandNormalMap", std::make_shared<Texture>(Path(Const::TexturesPath) / "sand/normal.jpg"));

	_terrainHeight = std::make_shared<Texture>(Path(Const::TexturesPath) / "terrain/height.jpg");
	_terrainNormal = std::make_shared<Texture>(Path(Const::TexturesPath) / "terrain/normal.jpg");

	_renderingMaterial.SetTexture("u_heightMap", _terrainHeight);

	_renderingMaterial.SetFloat("u_specPow", 4); 
	_renderingMaterial.SetFloat("u_specStr", 0.2);
	
	_waterMaterial = Material(Path(Const::ShadersPath) / "WaterShader.glsl");

	auto waterNormalMap = std::make_shared<Texture>(Path(Const::TexturesPath) / "water/normal.jpg");
	waterNormalMap->SetWrappingMode(TextureWrappingMode::Repeat);
	_waterMaterial.SetTexture("u_normalMap", waterNormalMap);
	_waterMaterial.SetTexture("u_colorMap", std::make_shared<Texture>(Path(Const::TexturesPath) / "water/color.jpg"));
	_waterMaterial.FaceCullingMode = FaceCullingMode::Disabled;

	_waterMaterial.SetFloat("u_specPow", 128);
	_waterMaterial.SetFloat("u_specStr", 1);

	Transform.SetScale(glm::vec3(_area.x, 1, _area.y));

	_cube = Model(
		std::make_shared<Mesh>(Path(Const::MeshesPath) / "cube.obj", MeshCreationFlags::CalculateTangents), std::make_shared<Material>(_waterMaterial));
	_cube.Transform.SetPosition(glm::vec3(7, 12, 0));
	_cube.Transform.SetScale(glm::vec3(10, 10, 10));
}

void Grass::Terrain::Render(Renderer& renderer) {
	const float WaterSize = 50;
	const glm::vec3 WaterCenter = { 22, 4, 3 };
	static glm::mat4 waterModelMatrix = glm::scale(glm::translate(glm::mat4(1.0), WaterCenter), glm::vec3(WaterSize, WaterSize, WaterSize));
	
	auto mm = this->_cube.Transform.GetModelMatrix();
	auto m = this->_cube.Mesh.get();


	renderer.RenderMesh(this->_terrainMesh, this->_renderingMaterial, this->Transform.GetModelMatrix());
	_waterMaterial.FaceCullingMode = FaceCullingMode::Back;
	_waterMaterial.DepthWriting = true;
	renderer.RenderTransparentMesh(m, &this->_waterMaterial, &mm);
	//renderer.RenderTransparentMesh(&this->_waterMesh, &this->_waterMaterial, &waterModelMatrix);
}
