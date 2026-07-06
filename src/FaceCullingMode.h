#pragma once

#include "Types.h"

namespace Grass {
	enum class FaceCullingMode : byte
	{ 
		Back,
		Front,
		Both,
		Disabled
	};
}