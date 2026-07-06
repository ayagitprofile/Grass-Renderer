#pragma once

#include "Shader.h"
#include "Texture.h"

namespace Grass{
class ComputeShader : public Shader
{
private:
	void BindTextures();
	Ref<Texture> _computeDataTexture = nullptr;
public:
	ComputeShader();
	ComputeShader(Path source) : Shader(source) {};
	ComputeShader(const std::string& source, const Path& includePath) : Shader(source, includePath) {};
	void DispatchSync(u32 numOfGroups);
	void DispatchSync(u32 numOfGroupsX, u32 numOfGroupsY);
	void SetDataTexture(const std::string& name, Ref<Texture> texture);
};
}
