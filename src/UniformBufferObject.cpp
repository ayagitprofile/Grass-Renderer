#include "UniformBufferObject.h"
#include <glad/glad.h>

Grass::UniformBufferObject::UniformBufferObject(u32 binding) 
	: _bindingIndex(binding)
{
	Reset(binding);
}

Grass::UniformBufferObject::~UniformBufferObject() {
	glDeleteBuffers(1, &_id);
}

void Grass::UniformBufferObject::Reset(u32 binding) {
	_bindingIndex = binding;
	glGenBuffers(1, &_id);
	glBindBuffer(GL_UNIFORM_BUFFER, _id);
	glBindBufferBase(GL_UNIFORM_BUFFER, _bindingIndex, _id);
}

void Grass::UniformBufferObject::SetData(void* data, size_t size) {
	glBindBuffer(GL_UNIFORM_BUFFER, _id);
	glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
}

void Grass::UniformBufferObject::UpdateData(void* data, size_t size) {
	glBindBuffer(GL_UNIFORM_BUFFER, _id);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
}
