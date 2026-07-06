#include "Texture.h"
#include <glad/glad.h>
#include <stb_image/stb_image.h>
#include <iostream>
#include "ScopedTimer.h"

static Grass::u32 GetTextureFormatFromNumOfChannel(Grass::i32 numOfChannels) {
	switch (numOfChannels) {
	case 1: return GL_RED;
	case 2: return GL_RG;
	case 3: return GL_RGB;
	case 4: return GL_RGBA;
	}
	return 0;
}

static void SetFilteringMode() {
	// Filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Wrapping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Aniso
	float maxSupported = 0.0f;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxSupported);

	GLfloat aniso = std::min(16.0f, maxSupported);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, aniso);
}

Grass::Texture::Texture(std::span<glm::vec4> pixels, i32 width, i32 height)
	: Texture() {
	_width = width;
	_height = height;
	_numOfChannels = 4;

	if (pixels.size() == 0 || pixels.data() == nullptr)
		return;

	ScopedTimer timer("[Texture] Loading unnamed texture took: ");
	glBindTexture(GL_TEXTURE_2D, _id);

	u32 internalFormat = GetTextureFormatFromNumOfChannel(4);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, _width, _height, 0, GL_RGBA, GL_FLOAT, pixels.data());

	SetFilteringMode();
	glGenerateMipmap(GL_TEXTURE_2D);
}

void Grass::Texture::SetWrappingMode(TextureWrappingMode wrappingMode) {
	glBindTexture(GL_TEXTURE_2D, _id);

	switch (wrappingMode) {
	case Grass::TextureWrappingMode::Clamp:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		break;
	case Grass::TextureWrappingMode::Repeat:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		break;
	}

}

Grass::Texture::Texture(std::span<glm::u8vec4> pixels, i32 width, i32 height) 
	: Texture()
{
	_width = width;
	_height = height;
	_numOfChannels = 4;

	if (pixels.size() == 0 || pixels.data() == nullptr)
		return;

	ScopedTimer timer("[Texture] Loading unnamed texture took: ");
	glBindTexture(GL_TEXTURE_2D, _id);

	u32 internalFormat = GetTextureFormatFromNumOfChannel(4);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

	SetFilteringMode();
	glGenerateMipmap(GL_TEXTURE_2D);
}

Grass::Texture::Texture() {
	glGenTextures(1, &_id);
}

Grass::Texture::Texture(Path path) : Texture() {
	glBindTexture(GL_TEXTURE_2D, _id);

	std::string message = "[Texture] Loading texture: " + path.string() + " took: ";
	ScopedTimer timer(message);
	stbi_set_flip_vertically_on_load(true);

	byte* data = stbi_load(path.string().c_str(), &_width, &_height, &_numOfChannels, 0);

	if (data == nullptr) {
		std::cerr << "[Texture] Failed to load texture: " << path << std::endl;
		return;
	}

	u32 format = GetTextureFormatFromNumOfChannel(_numOfChannels);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		format, // opengl format
		_width,
		_height,
		0, 
		format, // actual data format
		GL_UNSIGNED_BYTE,
		data
	);

	SetFilteringMode();

	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);
}

Grass::Texture::~Texture() {
	glDeleteTextures(1, &_id);
}
