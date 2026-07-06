#pragma once
#include "Types.h"
#include <glad/glad.h>

namespace Grass {
	enum class BufferUsage : u32
	{
		// Upload once or rarely update
		Static = GL_STATIC_DRAW,
		// Upload frequently
		Dynamic = GL_DYNAMIC_DRAW 
	};
}