#pragma once

#include "Types.h"
#include <string>
#include <filesystem>

namespace Grass {
	
class Shader
{
protected:
	u32 _id;
public:
	Shader();
	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;

	Shader(Shader&& other) noexcept;
	Shader& operator=(Shader&& other) noexcept;
	
	Shader(std::filesystem::path shaderSourceFile);
	Shader(const std::string& shaderSource, const Path& includePath);

	Shader(const std::string& vertexSource, const std::string& fragmentSource);

	u32 GetID() const { return _id; }

	void Bind() const;
};
}
