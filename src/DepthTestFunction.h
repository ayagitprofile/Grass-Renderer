#pragma once
#include <glad/glad.h>
#include "Types.h"
namespace Grass {
enum class DepthTestFunction : u32
{
	Less = GL_LESS,
	LessEqual = GL_LEQUAL,
	Equal = GL_EQUAL
};
}