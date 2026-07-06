#include "ModelLoader.h"
#include "ScopedTimer.h"
#include <TinyObjLoader/tiny_obj_loader.h>
#include <fstream>

static struct Vertex { glm::vec3 pos; glm::vec3 normal; glm::vec2 uv; };

static struct UniqueVertexKey {
	int v, n, t;

	bool operator==(const UniqueVertexKey& other) const {
		return v == other.v &&
			n == other.n &&
			t == other.t;
	}
};

static struct KeyHash {
	size_t operator()(const UniqueVertexKey& k) const {
		return ((k.v * 73856093) ^ (k.n * 19349663) ^ (k.t * 83492791));
	}
};

std::optional<Grass::ModelData> Grass::ModelLoader::LoadOBJ(std::filesystem::path filePath) {
	tinyobj::attrib_t attrib;

	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err;

	std::ifstream file(filePath);

	bool ok = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, &file);

	if (!err.empty() || !ok) {
		std::cerr << "[Model Loader] Failed to load model: " << filePath << " error: " << err << std::endl;
		return {};
	}

	size_t reserveSize = shapes[0].mesh.indices.size();

	std::vector<Vertex> vertexBuffer;
	vertexBuffer.reserve(reserveSize);
	std::vector<u32> indexBuffer;
	indexBuffer.reserve(reserveSize);
	u32 maxIndexValue = 0;

	std::unordered_map<UniqueVertexKey, u32, KeyHash> cache;
	cache.reserve(reserveSize);

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			UniqueVertexKey key{
				index.vertex_index,
				index.normal_index,
				index.texcoord_index
			};

			// If already exists → reuse index
			auto it = cache.find(key);
			if (it != cache.end()) {
				indexBuffer.push_back(it->second);
				continue;
			}

			// Otherwise create new vertex
			Vertex v{};

			// position
			v.pos[0] = attrib.vertices[3 * index.vertex_index + 0];
			v.pos[1] = attrib.vertices[3 * index.vertex_index + 1];
			v.pos[2] = attrib.vertices[3 * index.vertex_index + 2];

			// normal
			if (index.normal_index >= 0) {
				v.normal[0] = attrib.normals[3 * index.normal_index + 0];
				v.normal[1] = attrib.normals[3 * index.normal_index + 1];
				v.normal[2] = attrib.normals[3 * index.normal_index + 2];
			}

			// uv
			if (index.texcoord_index >= 0) {
				v.uv[0] = attrib.texcoords[2 * index.texcoord_index + 0];
				v.uv[1] = attrib.texcoords[2 * index.texcoord_index + 1];
			}

			unsigned int newIndex = (unsigned int)vertexBuffer.size();
			vertexBuffer.push_back(v);

			cache[key] = newIndex;

			maxIndexValue = std::max(maxIndexValue, newIndex);

			indexBuffer.push_back(newIndex);
		}
	}

	IndexFormat indexFormat = maxIndexValue <= std::numeric_limits<byte>::max() ? IndexFormat::Byte : maxIndexValue <= std::numeric_limits<u16>::max() ? IndexFormat::Ushort : IndexFormat::UInt;
	
	const size_t vertexCount = vertexBuffer.size();
	const size_t indexCount = indexBuffer.size();

	const size_t positionBytes = sizeof(glm::vec3) * vertexCount;
	const size_t normalBytes = attrib.normals.empty() ? 0 : sizeof(glm::vec3) * vertexCount;
	const size_t uvBytes = attrib.texcoords.empty() ? 0 : sizeof(glm::vec2) * vertexCount;
	const size_t indexBytes = Grass::GetIndexFormatSize(indexFormat) * indexCount;

	auto data = std::make_unique<byte[]>(
		positionBytes +
		normalBytes +
		uvBytes +
		indexBytes);

	size_t offset = 0;

	// Creates a span at the current offset and advances the offset.
	auto makeSpan = [&](auto& span, size_t count) {
		using T = typename std::remove_reference_t<decltype(span)>::element_type;

		span = std::span<T>(
			reinterpret_cast<T*>(data.get() + offset),
			count);

		offset += span.size_bytes();
		};

	// Writes an index buffer of the requested type.
	auto writeIndices = [&](auto& span) {
		using T = typename std::remove_reference_t<decltype(span)>::element_type;

		makeSpan(span, indexCount);

		for (size_t i = 0; i < indexCount; ++i)
			span[i] = static_cast<T>(indexBuffer[i]);
		};

	// Positions
	std::span<glm::vec3> positions;
	makeSpan(positions, vertexCount);

	for (size_t i = 0; i < vertexCount; ++i)
		positions[i] = vertexBuffer[i].pos;

	// Normals
	std::span<glm::vec3> normals;

	if (!attrib.normals.empty()) {
		makeSpan(normals, vertexCount);

		for (size_t i = 0; i < vertexCount; ++i)
			normals[i] = vertexBuffer[i].normal;
	}

	// UVs
	std::span<glm::vec2> uvs;

	if (!attrib.texcoords.empty()) {
		makeSpan(uvs, vertexCount);

		for (size_t i = 0; i < vertexCount; ++i)
			uvs[i] = vertexBuffer[i].uv;
	}

	// Indices
	std::span<byte> byteIndices;
	std::span<u16> ushortIndices;
	std::span<u32> uintIndices;

	switch (indexFormat) {
	case IndexFormat::Byte:
		writeIndices(byteIndices);
		break;

	case IndexFormat::Ushort:
		writeIndices(ushortIndices);
		break;

	case IndexFormat::UInt:
		writeIndices(uintIndices);
		break;
	}

	return ModelData(std::move(data), positions, normals, uvs, byteIndices, ushortIndices, uintIndices, indexFormat);
}

Grass::ModelData::ModelData() : IndexFormat() {
}

Grass::ModelData::ModelData(
	std::unique_ptr<byte[]> data,
	std::span<glm::vec3> positions, 
	std::span<glm::vec3> normals, 
	std::span<glm::vec2> uvs, 
	std::span<byte> byteIndices, 
	std::span<u16> ushortIndices, 
	std::span<u32> uintIndices,
	Grass::IndexFormat indexFormat
)
	: _data(std::move(data)), Positions(positions), Normals(normals), UVs(uvs), IndexFormat(indexFormat)
{
	switch (indexFormat) {
	case Grass::IndexFormat::Byte:
		ByteIndices = byteIndices;
		break;
	case Grass::IndexFormat::Ushort:
		UshortIndices = ushortIndices;
		break;
	case Grass::IndexFormat::UInt:
		UIntIndices = uintIndices;
		break;
	}
}

Grass::i32 Grass::ModelData::IndexCount() const {
	switch (this->IndexFormat){
	case IndexFormat::Byte: return ByteIndices.size();
	case IndexFormat::Ushort: return UshortIndices.size();
	case IndexFormat::UInt: return UIntIndices.size();
	default: return 0;
	}
	return 0;
}

Grass::i32 Grass::ModelData::GetIndexValue(i32 index) {
	switch (this->IndexFormat) {
	case IndexFormat::Byte: return ByteIndices[index];
	case IndexFormat::Ushort: return UshortIndices[index];
	case IndexFormat::UInt: return UIntIndices[index];
	default: return 0;
	}
	return 0;
}
