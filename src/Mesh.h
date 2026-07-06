#pragma once

#include <glm/vec3.hpp>
#include <vector>
#include <span>
#include <filesystem>

#include "Types.h"
#include "IndexFormat.h"
#include "VertexAttributeDescriptor.h"
#include "BufferUsage.h"

namespace Grass {

enum class MeshCreationFlags : u32 
{
	CalculateTangents = 1 << 0
};

class Mesh
{
private:
	IndexFormat _indexFormat = IndexFormat::Ushort;
	i32 _indexCount;
	i32 _vertexCount;

	u32 _vaoID;
	u32 _vertexBufferID;
	u32 _indexBufferID;

	void SetIndexBufferDataInternal(void* data, size_t size, BufferUsage bufferUsage);
	void Cleanup();
	std::vector<VertexAttributeDescriptor> _layout;

public:
	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;

	Mesh(Mesh&& other) noexcept;
	Mesh& operator=(Mesh&& other) noexcept;

	~Mesh();
	Mesh();
	Mesh(Path filePath, MeshCreationFlags meshFlags = {});
	
	void SetIndexBufferData(std::span<u32> indices, BufferUsage bufferUsage = BufferUsage::Static);
	void SetIndexBufferData(std::span<u16> indices, BufferUsage bufferUsage = BufferUsage::Static);
	void SetIndexBufferData(std::span<byte> indices, BufferUsage bufferUsage = BufferUsage::Static);
	void SetIndexBufferData(void* data, size_t size, IndexFormat indexFormat, BufferUsage bufferUsage = BufferUsage::Static);

	void SetVertexBufferData(void* data, size_t size, BufferUsage bufferUsage = BufferUsage::Static);

	void SetVertexBufferLayout(const std::initializer_list< VertexAttributeDescriptor>& layout);
	void SetVertexBufferLayout(const std::vector< VertexAttributeDescriptor>& layout);

	i32 GetIndexCount() const { return _indexCount; };

	std::vector<glm::vec3> GetVertices() const;

	void Bind() const { glBindVertexArray(_vaoID); };
	IndexFormat GetIndexFormat() const { return _indexFormat; }
};
}

