#pragma once

#include "Types.h"
#include "BufferUsage.h"

#include <span>

namespace Grass {
class ShaderStorageBuffer
{
private:
	u32 _id = 0;
	i32 _bindingIndex = -1;
public:
	ShaderStorageBuffer(const ShaderStorageBuffer&) = delete;
	ShaderStorageBuffer& operator=(const ShaderStorageBuffer&) = delete;

	void SetBinding(i32 bindingIndex);
	void SetData(void* data, size_t size, BufferUsage bufferUsage);
	void UpdateData(void* data, size_t size);
	void SetSubData(size_t offset, size_t size, void* data);
	void SetUInt(size_t index, u32 value);
	u32 FetchUInt(size_t index);

	i64 FetchBytesize() const;
	void FetchData(std::span<byte> buffer) const;
	void FetchData(void* buffer, size_t size) const;
	u32 FetchUInt(size_t index) const;
	void Clear();
	void Clear(size_t offset, size_t size);

	//ShaderStorageBuffer(ShaderStorageBuffer&& other) noexcept;
	//ShaderStorageBuffer& operator=(ShaderStorageBuffer&& other) noexcept;

	ShaderStorageBuffer();
	~ShaderStorageBuffer();
};
}