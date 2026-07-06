#include "ShaderStorageBuffer.h"
#include <glad/glad.h>
#include <iostream>

using namespace Grass;

static i64 FetchBufferSize(u32 id) {
	i64 size = 0;
	glGetNamedBufferParameteri64v(id, GL_BUFFER_SIZE, &size);
	return size;
}

void Grass::ShaderStorageBuffer::SetBinding(i32 bindingIndex) {
	if (_id == 0) return;

	_bindingIndex = bindingIndex;

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingIndex, _id);
}

void Grass::ShaderStorageBuffer::SetData(void* data, size_t size, BufferUsage bufferUsage) {
	glNamedBufferData(_id, size, data, (u32)bufferUsage);
}

void Grass::ShaderStorageBuffer::UpdateData(void* data, size_t size) {
	SetSubData(0, size, data);
}

void Grass::ShaderStorageBuffer::SetSubData(size_t offset, size_t size, void* data) {
	glNamedBufferSubData(_id, offset, size, data);
}

void Grass::ShaderStorageBuffer::SetUInt(size_t index, u32 value) {
	u32 v = value;
	SetSubData(sizeof(u32) * index, sizeof(u32), &v);
}

u32 Grass::ShaderStorageBuffer::FetchUInt(size_t index) {
	u32 value = 0;
	glGetNamedBufferSubData(_id, index * sizeof(u32), sizeof(u32), &value);
	return value;
}

i64 Grass::ShaderStorageBuffer::FetchBytesize() const {
	return FetchBufferSize(_id);
}

void Grass::ShaderStorageBuffer::FetchData(std::span<byte> buffer) const {
	i64 bytesize = FetchBufferSize(_id);

	if (buffer.size_bytes() < bytesize) {
		std::cerr << "[Shader Storage Buffer] Fetch Error: size of provided buffer: " << buffer.size_bytes() << " is less than size of data: " << bytesize << '\n';
		return;
	}

	glGetNamedBufferSubData(_id, 0, bytesize, buffer.data());
}

void Grass::ShaderStorageBuffer::FetchData(void* buffer, size_t size) const {
	i64 bytesize = FetchBufferSize(_id);

	if (size < bytesize) {
		std::cerr << "[Shader Storage Buffer] Fetch Error: size of provided buffer: " << size << " is less than size of data: " << bytesize << '\n';
		return;
	}

	glGetNamedBufferSubData(_id, 0, bytesize, buffer);
}

u32 Grass::ShaderStorageBuffer::FetchUInt(size_t index) const {
	u32 value = 0;
	glGetNamedBufferSubData(_id, index * sizeof(u32), sizeof(u32), &value);
	return value;
}

void Grass::ShaderStorageBuffer::Clear() {
	const byte zero = 0;

	glClearNamedBufferData(
		_id,
		GL_R8,          // internal format (1 byte per element)
		GL_RED,         // format of input data
		GL_UNSIGNED_BYTE,
		&zero
	);
}

void Grass::ShaderStorageBuffer::Clear(size_t offset, size_t size) {
	const byte zero = 0;
	glClearNamedBufferSubData(_id, GL_R8, offset, size, GL_RED, GL_UNSIGNED_BYTE, &zero);
}

Grass::ShaderStorageBuffer::ShaderStorageBuffer()
{
	glGenBuffers(1, &_id);
}

Grass::ShaderStorageBuffer::~ShaderStorageBuffer() {
	if (_id != 0)
		glDeleteBuffers(1, &_id);
}
