#pragma once
#include "Types.h"
namespace Grass {
class UniformBufferObject
{
private:
	u32 _id;
	u32 _bindingIndex;
public:

	UniformBufferObject() = default;
	UniformBufferObject(u32 bindingIndex);
	~UniformBufferObject();

	UniformBufferObject(const UniformBufferObject&) = delete;
	UniformBufferObject& operator=(const UniformBufferObject&) = delete;

	void Reset(u32 binding);

	void SetData(void* data, size_t size);
	void UpdateData(void* data, size_t size);
	u32 BindingIndex() const { return _bindingIndex; }
};
}

