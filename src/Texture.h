#pragma once
#include "Types.h"
#include <glm/vec4.hpp>
#include <span>

namespace Grass {
enum class TextureWrappingMode
{
	Clamp = 0, Repeat
};

class Texture
{
private:
	u32 _id = 0;
	i32 _width = 0;
	i32 _height = 0;
	i32 _numOfChannels = 0;
public:
	Texture(const Texture& other) = delete;
	Texture& operator=(const Texture& other) = delete;

	Texture(std::span<glm::u8vec4> pixels, i32 width, i32 height);

	Texture(std::span<glm::vec4> pixels, i32 width, i32 height);

	void SetWrappingMode(TextureWrappingMode wrappingMode);

	u32 GetID() const { return _id; }

	Texture();
	Texture(Path path);
	~Texture();
};
}

