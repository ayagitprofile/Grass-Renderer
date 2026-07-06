#pragma once

#include "VertexAttribute.h"
#include "VertexAttributeFormat.h"

namespace Grass {
struct VertexAttributeDescriptor
{
	VertexAttributeDescriptor(VertexAttribute attribute, VertexAttributeFormat format, i32 count) 
		: Attribute(attribute), Format(format), Count(count) {}

	VertexAttribute Attribute;
	VertexAttributeFormat Format;
	i32 Count;
};
}