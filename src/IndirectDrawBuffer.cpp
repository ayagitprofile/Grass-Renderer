#include "IndirectDrawBuffer.h"

#include <glad/glad.h>
#include "BufferUsage.h"

Grass::IndirectDrawBuffer::IndirectDrawBuffer() : _id(0) {
	glGenBuffers(1, &_id);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, _id);
}

Grass::IndirectDrawBuffer::~IndirectDrawBuffer() {
	if (_id)
		glDeleteBuffers(1, &_id);	
}

void Grass::IndirectDrawBuffer::SetData(DrawElementsIndirectCommand data) {
	glNamedBufferData(_id, sizeof(data), &data, (u32)BufferUsage::Dynamic);
}

void Grass::IndirectDrawBuffer::UpdateData(DrawElementsIndirectCommand data) {
	glNamedBufferSubData(_id, 0, sizeof(data), &data);
}

void Grass::IndirectDrawBuffer::SetSSBOBinding(i32 bindingIndex) {
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingIndex, _id);
}

void Grass::IndirectDrawBuffer::Clear(size_t offset, size_t size) {
	const byte zero = 0;
	glClearNamedBufferSubData(_id, GL_R8, offset, size, GL_RED, GL_UNSIGNED_BYTE, &zero);
}
