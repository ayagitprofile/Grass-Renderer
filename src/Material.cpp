#include "Material.h"
#include <glad/glad.h>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

Grass::i32 Grass::Material::GetUniformLocation(const std::string& name) {
	_shader->Bind();
	return glGetUniformLocation(_shader->GetID(), name.c_str());
}

Grass::Material::Material(Ref<Shader> shader)
	: _shader(shader)
{
}

Grass::Material::Material(std::filesystem::path shaderSource) 
	: _shader(std::make_shared<Shader>(shaderSource))
{
}

void Grass::Material::SetTexture(const std::string& name, Ref<Texture> texture) {
	_textures[name] = texture;

	i32 index = 0;

	for (const auto& [name, texture] : _textures) {
		i32 bindingIndex = GL_TEXTURE0 + index;
		glActiveTexture(bindingIndex);
		glBindTexture(GL_TEXTURE_2D, texture->GetID());
		glUniform1i(glGetUniformLocation(_shader->GetID(), name.c_str()), index);

		index++;
	}
}

void Grass::Material::SetMat4(const std::string& name, const glm::mat4& matrix) {
	glUniformMatrix4fv(GetUniformLocation(name), 1, false, glm::value_ptr(matrix));
}

void Grass::Material::SetMat3(const std::string& name, const glm::mat3& matrix) {
	SetMat3(GetUniformLocation(name), matrix);
}

void Grass::Material::SetMat3(i32 id, const glm::mat3& matrix) {
	_shader->Bind();
	glUniformMatrix3fv(id, 1, false, glm::value_ptr(matrix));
}

void Grass::Material::SetFloat(const std::string& name, float value) {
	glUniform1f(GetUniformLocation(name), value);
}

void Grass::Material::SetVec4(const std::string& name, glm::vec4 value) {
	glUniform4fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Grass::Material::SetVec3(const std::string& name, glm::vec3 value) {
	glUniform3fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Grass::Material::SetVec2(const std::string& name, glm::vec2 value) {
	glUniform2fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

Grass::i32 Grass::Material::GetUniformID(const std::string& name) {
	return GetUniformLocation(name);
}

void Grass::Material::Bind() const {
	_shader->Bind();

	if (_textures.size() == 0)
		return;

	i32 index = 0;

	for (const auto& [name, texture] : _textures) {
		i32 bindingIndex = GL_TEXTURE0 + index;
		glActiveTexture(bindingIndex);
		glBindTexture(GL_TEXTURE_2D, texture->GetID());
		
		index++;
	}
}
