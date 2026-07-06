#pragma once

#include "Mesh.h"
#include "Material.h"
#include "Renderer.h"
#include "ShaderStorageBuffer.h"
#include "Camera.h"
#include "ComputeShader.h"
#include "IndirectDrawBuffer.h"
#include <vector>
#include <memory>

namespace Grass {
class GrassRenderer
{
private:
	ComputeShader CreateDataGenerationCompute();
	ComputeShader CreateCullingComputeShader(i32);

	struct GPUAABB { 
		glm::vec4 minpoint;
		glm::vec4 maxpoint;
	};

	size_t GetAABBBufferSize() const { return sizeof(GPUAABB) * _numberOfChunks; }
	size_t GetVisibilityBufferSize() const { return sizeof(u32) * (1 + _numberOfChunks); }

	Mesh _grassBladeMesh;
	Material _grassMaterial;

	ShaderStorageBuffer _instanceDataBuffer; // per instance model matrices 
	ShaderStorageBuffer _sharedStateBuffer; // shared state (BladesPerChunk)
	ShaderStorageBuffer _chunkCullingBuffer; // how many chunks are visible and offsets to visible chunks data inside instance data buffer
	ShaderStorageBuffer _chunkBoundingBoxesBuffer; // contains bounding boxes

	DrawElementsIndirectCommand _drawCommand;

	IndirectDrawBuffer _indirectDrawBuffer;

	ComputeShader _cullingComputeShader;

	glm::vec4 _wind;
	float _windSpeed = 1;
	i32 _bladesPerRow = 10;
	i32 _bladesPerChunk = 0;
	glm::vec2 _grassArea = { 0, 0 };
	i32 _instanceCount = 0;
	i32 _chunksPerRow = 0;
	i32 _numberOfChunks = 0;
	glm::vec2 _chunkSize = {};
	glm::vec3 _center = { 0,0,0 };
	i32 _visibleChunks = 0;
	float _cullingTime = 0.f;

	static struct SharedShaderStateData {
		u32 BladesPerChunk;
		float GrassRenderingDistance;
	};

	SharedShaderStateData _sharedShaderStateData;

public:
	GrassRenderer();
	~GrassRenderer();
	void GenerateData(Ref<Texture> terrainHeightTexture, glm::vec2 area, i32 density);
	void Update(float deltaTime, const Camera& camera);
	void Render(Renderer& renderer);

	void SetWindDirection(glm::vec3 direction);
	void SetWindStrength(float strength);

	void SetWindSpeed(float speed) { _windSpeed = speed; };

	i32 GetChunkSize() const;
	glm::ivec2 ChunkedDimensions() const { return { _chunksPerRow, _chunksPerRow }; };

	i32 FetchVisibleChunks() const;
	i32 CalculateInstanceCount(i32 visibleChunks) const { return visibleChunks * _instanceCount; }

	float GetCullingTime() const { return _cullingTime; }
	float GetWindStrength() const { return _wind.w; }
	glm::vec3 GetWindDirection() const { return glm::vec3(_wind.x, _wind.y, _wind.z); }
	size_t GetInstanceBufferSize() const { return sizeof(glm::mat4) * _instanceCount; }

	void SetRenderingDistance(float distance);
	float GetRenderingDistance() const { return _sharedShaderStateData.GrassRenderingDistance; }

	bool EnableChunkCulling = true;
};
}