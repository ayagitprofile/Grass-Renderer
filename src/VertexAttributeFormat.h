#pragma once

#include <glad/glad.h>
#include "Types.h"
namespace Grass {
enum class VertexAttributeFormat : u32
{
	Float32 = GL_FLOAT,
	Float16 = GL_HALF_FLOAT,
	Int = GL_INT,
	UInt = GL_UNSIGNED_INT
};
}