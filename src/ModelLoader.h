#pragma once

#include "Types.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <memory>
#include <span>
#include <filesystem>
#include <optional>

#include "IndexFormat.h"

namespace Grass {

class ModelLoader;

struct ModelData
{
private:
	std::unique_ptr<byte[]> _data = nullptr;

public:
	ModelData();

	ModelData(
		std::unique_ptr<byte[]> data,
		std::span<glm::vec3> positions, 
		std::span<glm::vec3> normals, 
		std::span<glm::vec2> uvs, 
		std::span<byte> byteIndices, 
		std::span<u16> ushortIndices, 
		std::span<u32> uintIndices,
		IndexFormat indexFormat);

	friend class ModelLoader;
	const std::span<glm::vec3> Positions;
	const std::span<glm::vec3> Normals;
	const std::span<glm::vec2> UVs;

	const IndexFormat IndexFormat;

	inline i32 VertexCount() const { return Positions.size(); }
	i32 IndexCount() const;
	i32 GetIndexValue(i32 index);

	union
	{
		std::span<u32> UIntIndices;
		std::span<u16> UshortIndices;
		std::span<byte> ByteIndices;
	};

};

class ModelLoader
{
public:
	static std::optional<ModelData> LoadOBJ(std::filesystem::path filePath);
};
}