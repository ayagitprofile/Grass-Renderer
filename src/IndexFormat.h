#pragma once

#include "Types.h"
#include <glad/glad.h>
namespace Grass {
enum class IndexFormat : u32
{
	Byte = GL_UNSIGNED_BYTE,
	Ushort = GL_UNSIGNED_SHORT,
	UInt = GL_UNSIGNED_INT
};
inline size_t GetIndexFormatSize(IndexFormat format) {
	switch (format) {
	case Grass::IndexFormat::Byte: return sizeof(byte);
	case Grass::IndexFormat::Ushort: return sizeof(u16);
	case Grass::IndexFormat::UInt: return sizeof(u32);
	}
	return 0;
}
}