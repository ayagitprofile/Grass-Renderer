#include "GrassRenderer.h"
#include "Const.h"
#include <glm/gtc/random.hpp>
#include "ScopedTimer.h"
#include "ComputeShaderSource.h"
#include "ComputeShader.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#define CLEAR_COMPUTE_CULLING_DATA

using namespace Grass;

static constexpr i32 InstanceDataBufferBindingIndex = 10;
static constexpr i32 BoundingBoxDataBufferBindingIndex = 11;
static constexpr i32 VisibilityBufferBindingIndex = 12;
static constexpr i32 IndirectDrawBufferBindingIndex = 13;
static constexpr i32 SharedStateBufferBindingIndex = 14;

static constexpr i32 GrassGridResolution = 1000;
static constexpr glm::vec2 GrassArea = { 200, 200 };
static constexpr u32 ChunkSize = 20;

static constexpr float MaxRandomOffset = 0.1f;

ComputeShader Grass::GrassRenderer::CreateCullingComputeShader(i32 numberOfThreads){
	ComputeShaderSource source(Path(Const::ShadersPath) / "FrustrumCullingCompute.glsl");
	source.InsertLine(std::format("#define BOUNDING_BOX_DATA_BINDING ({})", BoundingBoxDataBufferBindingIndex));
	source.InsertLine(std::format("#define VISIBILITY_DATA_BINDING ({})", VisibilityBufferBindingIndex));
	source.InsertLine(std::format("#define INDIRECT_BUFFER_BINDING ({})", IndirectDrawBufferBindingIndex));
	source.InsertLine(std::format("#define INSTANCE_DATA_BINDING ({})", InstanceDataBufferBindingIndex));
	source.InsertLine(std::format("#define SHARED_STATE_DATA_BINDING ({})", SharedStateBufferBindingIndex));

#ifndef CLEAR_COMPUTE_CULLING_DATA
	source.InsertLine("#define CLEAR_COMPUTE_CULLING_DATA");
#endif

	source.SetNumberOfThreads(numberOfThreads);
	ComputeShader compute = source.Compile();
	return compute;
}

Grass::GrassRenderer::GrassRenderer()
	: 
	_grassMaterial(Path(Const::ShadersPath) / "InstancedGrassShader.glsl"),
	_grassBladeMesh(Path(Const::MeshesPath) / "grassBlade.obj", MeshCreationFlags::CalculateTangents)
{
	_grassMaterial.FaceCullingMode = FaceCullingMode::Disabled;
	_grassMaterial.SetFloat("u_specPow", 4);
	_grassMaterial.SetFloat("u_specStr", 0.5);

	SetWindDirection(glm::normalize(glm::vec3(0.5, 0, -0.5)));
	SetWindStrength(0.5f);
}

Grass::GrassRenderer::~GrassRenderer() {
}

static std::string Vec3ToString(glm::vec3 val) {
	return std::format("({:.2f}, {:.2f}, {:.2f})", val.x, val.y, val.z);
}

static glm::vec3 AABBMax(glm::vec3 left, glm::vec3 right) {
	return
	{
		glm::max(left.x, right.x),
		glm::max(left.y, right.y),
		glm::max(left.z, right.z)
	};
}

static glm::vec3 AABBMin(glm::vec3 left, glm::vec3 right) {
	return
	{
		glm::min(left.x, right.x),
		glm::min(left.y, right.y),
		glm::min(left.z, right.z)
	};
}

ComputeShader Grass::GrassRenderer::CreateDataGenerationCompute() {
	glm::vec2 begin = -_grassArea * 0.5f;
	glm::vec2 end = _grassArea * 0.5f;

	ComputeShaderSource computeSource(Path(Const::ShadersPath) / "GrassDataGenerationCompute.glsl");
	computeSource.SetNumberOfThreads(1, 1); // 1 thread per each chunk
	computeSource.InsertLine(std::format("#define INSTANCE_DATA_BINDING ({})", InstanceDataBufferBindingIndex));
	computeSource.InsertLine(std::format("#define BOUNDING_BOX_DATA_BINDING ({})", BoundingBoxDataBufferBindingIndex));
	computeSource.InsertLine(std::format("#define SHARED_STATE_DATA_BINDING ({})", SharedStateBufferBindingIndex));
	computeSource.InsertLine(std::format("#define BLADES_PER_ROW ({})", _bladesPerRow));
	computeSource.InsertLine(std::format("#define BLADES_PER_CHUNK ({})", _bladesPerChunk));
	computeSource.InsertLine(std::format("#define CHUNKS_PER_ROW ({})", _chunksPerRow));
	computeSource.InsertLine(std::format("#define MAX_RANDOM_OFFSET ({})", MaxRandomOffset));

	computeSource.InsertLine(std::format("#define TERRAIN_BEGIN (vec2({}, {}))", begin.x, begin.y));
	computeSource.InsertLine(std::format("#define TERRAIN_AREA (vec2({}, {}))", _grassArea.x, _grassArea.y));
	computeSource.InsertLine(std::format("#define TERRAIN_END (vec2({}, {}))", end.x, end.y));
	computeSource.InsertLine(std::format("#define TERRAIN_CENTER (vec3({}, {}, {}))", _center.x, _center.y, _center.z));

	ComputeShader computeShader = computeSource.Compile();

	return computeShader;
}

void Grass::GrassRenderer::GenerateData(Ref<Texture> terrainHeightTexture, glm::vec2 area, i32 density) {
	ScopedTimer timer("[Grass Renderer] Instanced data generation took: ");

	_grassArea = area;
	
	_chunksPerRow = static_cast<i32>(area.x / ChunkSize);
	_bladesPerRow = ChunkSize * density;

	_numberOfChunks = _chunksPerRow * _chunksPerRow;
	_bladesPerChunk = _bladesPerRow * _bladesPerRow;

	_instanceCount = _numberOfChunks * _bladesPerChunk;
	_center = { 0, 0, 0 };
	_chunkSize = {_grassArea.x / (float)_chunksPerRow, _grassArea.y / (float)_chunksPerRow};

	_sharedStateBuffer.SetBinding(SharedStateBufferBindingIndex);

	_sharedShaderStateData.BladesPerChunk = (u32)_bladesPerChunk;
	_sharedShaderStateData.GrassRenderingDistance = 150;
	_sharedStateBuffer.SetData(&_sharedShaderStateData, sizeof(_sharedShaderStateData), BufferUsage::Static);

	_instanceDataBuffer.SetBinding(InstanceDataBufferBindingIndex);
	_chunkBoundingBoxesBuffer.SetBinding(BoundingBoxDataBufferBindingIndex);
	_chunkCullingBuffer.SetBinding(VisibilityBufferBindingIndex);

	_instanceDataBuffer.SetData(nullptr, sizeof(glm::mat4) * _instanceCount, BufferUsage::Dynamic);
	_chunkBoundingBoxesBuffer.SetData(nullptr, GetAABBBufferSize(), BufferUsage::Dynamic);

	_cullingComputeShader = CreateCullingComputeShader(_numberOfChunks);
	ComputeShader computeShader = CreateDataGenerationCompute();
	computeShader.SetDataTexture("TerrainHeightTexture", terrainHeightTexture);
	computeShader.DispatchSync(_chunksPerRow, _chunksPerRow);

	//_grassMaterial.SetTexture("u_heightMap", terrainHeightTexture);

	_chunkCullingBuffer.SetData(nullptr, GetVisibilityBufferSize(), BufferUsage::Dynamic);

	_drawCommand.BaseInstance = 0;
	_drawCommand.BaseVertex = 0;
	_drawCommand.Count = _grassBladeMesh.GetIndexCount();
	_drawCommand.FirstIndex = 0;
	_drawCommand.InstanceCount = _bladesPerChunk * _numberOfChunks;
	
	_indirectDrawBuffer.SetData(_drawCommand);
	_indirectDrawBuffer.SetSSBOBinding(IndirectDrawBufferBindingIndex);
}

void Grass::GrassRenderer::Update(float deltaTime, const Camera& camera) {
	static float time = 0.f;
	time += deltaTime;
	_grassMaterial.SetFloat("u_time", time * _windSpeed);

	if (EnableChunkCulling) {
	ScopedTimer timer;
#ifdef CLEAR_COMPUTE_CULLING_DATA
	_chunkCullingBuffer.Clear(0, sizeof(u32));
	_indirectDrawBuffer.Clear(offsetof(DrawElementsIndirectCommand, InstanceCount), sizeof(u32));
#endif
	_cullingComputeShader.DispatchSync(1);
	_cullingTime = timer.ElapsedTimeMS();
	}
}

void Grass::GrassRenderer::Render(Renderer& renderer) {
	if (EnableChunkCulling)
		renderer.RenderMeshIndirect(_grassBladeMesh, _grassMaterial, _indirectDrawBuffer);
	else
		renderer.RenderMeshInstanced(_grassBladeMesh, _grassMaterial, _bladesPerChunk * _numberOfChunks);
}

void Grass::GrassRenderer::SetWindDirection(glm::vec3 direction) {
	_wind = glm::vec4(direction, _wind.w);
	_grassMaterial.SetVec4("u_wind", _wind);
}

void Grass::GrassRenderer::SetWindStrength(float strength) {
	_wind = glm::vec4(glm::vec3(_wind.x, _wind.y, _wind.z), strength);
	_grassMaterial.SetVec4("u_wind", _wind);
}

i32 Grass::GrassRenderer::GetChunkSize() const {
	return ChunkSize;
}

i32 Grass::GrassRenderer::FetchVisibleChunks() const {
	return _chunkCullingBuffer.FetchUInt(0);
}

void Grass::GrassRenderer::SetRenderingDistance(float distance) {
	_sharedShaderStateData.GrassRenderingDistance = distance;
	_sharedStateBuffer.UpdateData(&_sharedShaderStateData, sizeof(_sharedShaderStateData));
}
