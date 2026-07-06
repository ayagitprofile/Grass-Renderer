#pragma once

#include "Types.h"
#include "Shader.h"
#include "FaceCullingMode.h"
#include "Texture.h"
#include "DepthTestFunction.h"

#include <memory>
#include <filesystem>
#include <unordered_map>
#include <glm/mat4x4.hpp>
#include <glm/mat3x3.hpp>

namespace Grass {
class Material
{
private:
	i32 GetUniformLocation(const std::string& name);
	std::unordered_map<std::string, Ref<Texture>> _textures;
	Ref<Shader> _shader;
public:
	Material() = default;
	FaceCullingMode FaceCullingMode = FaceCullingMode::Back;
	bool DepthWriting = true;
	bool DepthTesting = true;
	DepthTestFunction DepthTestFunction = DepthTestFunction::Less;
	Material(Ref<Shader> shader);
	Material(std::filesystem::path shaderSource);
	void SetTexture(const std::string& name, Ref<Texture> texture);
	void SetMat4(const std::string& name, const glm::mat4& matrix);
	void SetMat3(const std::string& name, const glm::mat3& matrix);
	void SetMat3(i32 id, const glm::mat3& matrix);
	void SetFloat(const std::string& name, float value);
	void SetVec4(const std::string& name, glm::vec4 value);
	void SetVec3(const std::string& name, glm::vec3 value);
	void SetVec2(const std::string& name, glm::vec2 value);
	i32 GetUniformID(const std::string& name);
	void Bind() const;
	const Shader& GetShader() const { return *_shader; }
};
}