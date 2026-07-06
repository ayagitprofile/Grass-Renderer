#include "Mesh.h"

#include <TinyObjLoader/tiny_obj_loader.h>
#include <fstream>
#include <glm/vec2.hpp>
#include <unordered_map>
#include <iostream>

#include "BufferView.h"
#include "ScopedTimer.h"
#include "ModelLoader.h"
#include <glm/geometric.hpp>
#include <glm/ext/vector_float4.hpp>

namespace Grass {
static size_t GetAttribFormatSize(VertexAttributeFormat format) {
	switch (format) {
	case VertexAttributeFormat::Float32: return sizeof(float);
	case VertexAttributeFormat::Float16: return 2;
	case VertexAttributeFormat::Int:     return sizeof(i32);
	case VertexAttributeFormat::UInt:    return sizeof(u32);
	}
	return 0;
}

static bool IsIntegerFormat(VertexAttributeFormat format) {
	switch (format) {
	case VertexAttributeFormat::Int:
	case VertexAttributeFormat::UInt:
		return true;
	}
	return false;
}

static size_t GetVertexBytesize(const std::vector<VertexAttributeDescriptor>& layout) {
	size_t size = 0;
	for (const auto& attrib : layout) {
		size += GetAttribFormatSize(attrib.Format) * attrib.Count;
	}
	return size;
}


void Mesh::SetVertexBufferLayout(const std::initializer_list<VertexAttributeDescriptor>& layout) {
	SetVertexBufferLayout(std::vector<VertexAttributeDescriptor>{layout});
}

void Mesh::SetVertexBufferLayout(const std::vector<VertexAttributeDescriptor>& layout) {
	size_t vertexSize = GetVertexBytesize(layout);
	size_t i = 0, offset = 0;
	for (const auto& attrib : layout) {
		glEnableVertexAttribArray(i);
		
		if (IsIntegerFormat(attrib.Format))
			glVertexAttribIPointer(i, attrib.Count, (u32)attrib.Format, vertexSize, (void*)offset);
		else
			glVertexAttribPointer(i, attrib.Count, (u32)attrib.Format, false, vertexSize, (void*)offset);
		
		++i;
		offset += GetAttribFormatSize(attrib.Format) * attrib.Count;
	}

	_layout = layout;
}

std::vector<glm::vec3> Mesh::GetVertices() const {
	if (_layout.empty() || _vaoID == 0 || _vertexBufferID == 0) {
		std::cerr << "[Mesh] Cant get vertices\n";
		return {};
	}

	auto it = std::find_if(
		_layout.begin(), _layout.end(), [](const Grass::VertexAttributeDescriptor& attrib) {
			return attrib.Attribute == VertexAttribute::Position && attrib.Count == 3 && attrib.Format == Grass::VertexAttributeFormat::Float32;
		});

	if (it == _layout.end()) {
		std::cout << "[Mesh] Cant get vertices, layout does not have 3D position attribute\n";
	}

	size_t vertexSize = GetVertexBytesize(_layout);
	size_t bufferSize = vertexSize * _vertexCount;

	auto buffer = std::make_unique<byte[]>(bufferSize);

	glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferID);
	glGetBufferSubData(GL_ARRAY_BUFFER, 0, bufferSize, buffer.get());

	size_t attribOffset = 0;
	for (auto it = _layout.begin(); it != _layout.end() && it->Attribute != VertexAttribute::Position; ++it)
		attribOffset += GetAttribFormatSize(it->Format) * it->Count;

	std::vector<glm::vec3> positions;

	for (size_t i = 0; i < _vertexCount; ++i) {
		size_t begin = i * vertexSize;
		size_t attrib = begin + attribOffset;
		glm::vec3 position;
		memcpy(&position, buffer.get() + attrib, sizeof(glm::vec3));
		positions.push_back(position);
	}

	return positions;
}

static u32 GenerateVAO() { u32 vaoid; glGenVertexArrays(1, &vaoid); return vaoid; }
static u32 GenerateBuffer() { u32 bufferid; glGenBuffers(1, &bufferid); return bufferid; }

void Mesh::Cleanup() {
	glDeleteVertexArrays(1, &_vaoID);
	glDeleteBuffers(1, &_vertexBufferID);
	glDeleteBuffers(1, &_indexBufferID);
}

Mesh::Mesh(Mesh&& other) noexcept {
	_vaoID = other._vaoID;
	_vertexBufferID = other._vertexBufferID;
	_indexBufferID = other._indexBufferID;
	_indexCount = other._indexCount;
	_vertexCount = other._vertexCount;
	_layout = std::move(other._layout);

	other._indexBufferID = other._vertexBufferID = other._vaoID = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
	if (this == &other)
		return *this;
	Cleanup();

	_vaoID = other._vaoID;
	_vertexBufferID = other._vertexBufferID;
	_indexBufferID = other._indexBufferID;
	_indexCount = other._indexCount;
	_vertexCount = other._vertexCount;
	_layout = std::move(other._layout);

	other._indexBufferID = other._vertexBufferID = other._vaoID = 0;

	return *this;
}

Mesh::~Mesh() {
	Cleanup();
}

Mesh::Mesh()
	: _vaoID(GenerateVAO()), _vertexBufferID(GenerateBuffer()), _indexBufferID(GenerateBuffer()), _indexCount(0), _vertexCount(0)
{
	glBindVertexArray(_vaoID);

	glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBufferID);
}

static struct VertexPNUT 
{ 
	glm::vec3 position; 
	glm::vec3 normal; 
	glm::vec2 uv; 
	glm::vec4 tangent; // tangent = xyz, w = handedness
};

static void AccumulateTangent(
	VertexPNUT& v0,
	VertexPNUT& v1,
	VertexPNUT& v2) {
	const glm::vec3& p0 = v0.position;
	const glm::vec3& p1 = v1.position;
	const glm::vec3& p2 = v2.position;

	const glm::vec2& uv0 = v0.uv;
	const glm::vec2& uv1 = v1.uv;
	const glm::vec2& uv2 = v2.uv;

	glm::vec3 edge1 = p1 - p0;
	glm::vec3 edge2 = p2 - p0;

	glm::vec2 dUV1 = uv1 - uv0;
	glm::vec2 dUV2 = uv2 - uv0;

	float denom = (dUV1.x * dUV2.y - dUV2.x * dUV1.y);

	if (fabs(denom) < 1e-8f)
		return; // bad UVs

	float f = 1.0f / denom;

	glm::vec3 tangent = f * (
		dUV2.y * edge1 -
		dUV1.y * edge2
		);

	glm::vec3 bitangent = f * (
		-dUV2.x * edge1 +
		dUV1.x * edge2
		);

	float handedness =
		(glm::dot(glm::cross(v0.normal, tangent), bitangent) < 0.0f)
		? -1.0f
		: 1.0f;

	// accumulate tangent
	v0.tangent += glm::vec4(tangent, 0.0f);
	v1.tangent += glm::vec4(tangent, 0.0f);
	v2.tangent += glm::vec4(tangent, 0.0f);

	// accumulate handedness (sign consistency)
	v0.tangent.w += handedness;
	v1.tangent.w += handedness;
	v2.tangent.w += handedness;
}

Mesh::Mesh(Path filePath, MeshCreationFlags meshFlags) : Mesh() {
	std::string message = "\n[Mesh] Loading mesh: " + filePath.string() + " took: ";
	ScopedTimer timer(message);

	auto data = ModelLoader::LoadOBJ(filePath);
	if (data.has_value() == false)
		return;

	if (data->Normals.empty() || data->Positions.empty() || data->UVs.empty()) {
		std::cerr << "[Mesh] Only meshes with all 3 vertex data channels present are supported: " << filePath << std::endl;
		return;
	}

	if ((u32)meshFlags & (u32)MeshCreationFlags::CalculateTangents) {
		const auto layout = {
			(VertexAttributeDescriptor(Grass::VertexAttribute::Position, Grass::VertexAttributeFormat::Float32, 3)),
			(VertexAttributeDescriptor(Grass::VertexAttribute::Normal, Grass::VertexAttributeFormat::Float32, 3)),
			(VertexAttributeDescriptor(Grass::VertexAttribute::UV, Grass::VertexAttributeFormat::Float32, 2)),
			(VertexAttributeDescriptor(Grass::VertexAttribute::Tangent, Grass::VertexAttributeFormat::Float32, 4)),
		};

		SetVertexBufferLayout(layout);

		std::vector<VertexPNUT> vertexBuffer;
		vertexBuffer.reserve(data->Positions.size());

		for (size_t i = 0; i < data->VertexCount(); ++i) {
			VertexPNUT v;

			v.position = data->Positions[i];
			v.normal = data->Normals[i];
			v.uv = data->UVs[i];
			v.tangent = glm::vec4(0.0f);

			vertexBuffer.push_back(v);
		}

		for (size_t i = 0; i < data->IndexCount(); i += 3){
			//i32 begin = data->GetIndexValue(i);
			
			u32 i0 = data->GetIndexValue(i + 0);
			u32 i1 = data->GetIndexValue(i + 1);
			u32 i2 = data->GetIndexValue(i + 2);

			VertexPNUT& v0 = vertexBuffer[i0];
			VertexPNUT& v1 = vertexBuffer[i1];
			VertexPNUT& v2 = vertexBuffer[i2];

			AccumulateTangent(v0, v1, v2);
		}

		for (size_t i = 0; i < data->VertexCount(); ++i) {
			auto& v = vertexBuffer[i];

			glm::vec3 t = glm::vec3(v.tangent);

			// orthogonalize tangent vs normal
			t = t - v.normal * glm::dot(v.normal, t);

			if (glm::dot(t, t) > 0.0f)
				t = glm::normalize(t);

			// normalize handedness (majority vote)
			float w = (v.tangent.w >= 0.0f) ? 1.0f : -1.0f;

			v.tangent = glm::vec4(t, w);
		}

		SetVertexBufferData(vertexBuffer.data(), vertexBuffer.size() * sizeof(VertexPNUT));
	}
	else {
		const auto layout = {
			(VertexAttributeDescriptor(Grass::VertexAttribute::Position, Grass::VertexAttributeFormat::Float32, 3)),
			(VertexAttributeDescriptor(Grass::VertexAttribute::Normal, Grass::VertexAttributeFormat::Float32, 3)),
			(VertexAttributeDescriptor(Grass::VertexAttribute::UV, Grass::VertexAttributeFormat::Float32, 2))
		};
		SetVertexBufferLayout(layout);
		struct Vertex { glm::vec3 position; glm::vec3 normal; glm::vec2 uv; };

		std::vector<Vertex> vertexBuffer;
		vertexBuffer.reserve(data->Positions.size());

		for (size_t i = 0; i < data->VertexCount(); ++i) {
			Vertex v;

			v.position = data->Positions[i];
			v.normal = data->Normals[i];
			v.uv = data->UVs[i];

			vertexBuffer.push_back(v);
		}

		SetVertexBufferData(vertexBuffer.data(), vertexBuffer.size() * sizeof(Vertex));
	}

	switch (data->IndexFormat) {
	case IndexFormat::Byte:
		SetIndexBufferData(data->ByteIndices);
		break;
	case IndexFormat::Ushort:
		SetIndexBufferData(data->UshortIndices);
		break;
	case IndexFormat::UInt:
		SetIndexBufferData(data->UIntIndices);
		break;
	}
}

void Mesh::SetIndexBufferDataInternal(void* data, size_t size, BufferUsage bufferUsage) {
	glBindVertexArray(_vaoID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, (u32)bufferUsage);
}

void Mesh::SetIndexBufferData(std::span<u32> indices, BufferUsage bufferUsage) {
	_indexFormat = IndexFormat::UInt;
	_indexCount = indices.size();
	SetIndexBufferDataInternal(indices.data(), indices.size_bytes(), bufferUsage);
}

void Mesh::SetIndexBufferData(std::span<u16> indices, BufferUsage bufferUsage) {
	_indexFormat = IndexFormat::Ushort;
	_indexCount = indices.size();
	SetIndexBufferDataInternal(indices.data(), indices.size_bytes(), bufferUsage);
}

void Mesh::SetIndexBufferData(std::span<byte> indices, BufferUsage bufferUsage) {
	_indexFormat = IndexFormat::Byte;
	_indexCount = indices.size();
	SetIndexBufferDataInternal(indices.data(), indices.size_bytes(), bufferUsage);
}

void Mesh::SetIndexBufferData(void* data, size_t size, IndexFormat indexFormat, BufferUsage bufferUsage) {
	_indexFormat = indexFormat;
	_indexCount = size / GetIndexFormatSize(indexFormat);
	SetIndexBufferDataInternal(data, size, bufferUsage);
}

void Mesh::SetVertexBufferData(void* data, size_t size, BufferUsage bufferUsage) {
	if (_layout.empty()) {
		std::cerr << "[Mesh] Provide a vertex attribute layout before setting vertex buffer data\n";
		return;
	}

	size_t vertexSize = GetVertexBytesize(_layout);

	_vertexCount = size / vertexSize;

	glBindVertexArray(_vaoID);
	glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, size, data, (u32)bufferUsage);
}
}
