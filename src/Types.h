#pragma once
#include <cstdint>
#include <memory>
#include <filesystem>

namespace Grass {
	
typedef std::int64_t i64;
typedef std::int32_t i32;
typedef std::int16_t i16;
typedef std::int8_t sbyte;

typedef std::uint64_t u64;
typedef std::uint32_t u32;
typedef std::uint16_t u16;
typedef std::uint8_t byte;

typedef i32 Key;
typedef std::filesystem::path Path;

template<typename T>
using Ref = std::shared_ptr<T>;
}