#pragma once
#include "Types.h"
namespace Grass {

struct DrawElementsIndirectCommand
{
	u32 Count;         // Number of indices
	u32 InstanceCount; // Number of instances
	u32 FirstIndex;    // Starting index
	i32 BaseVertex;    // Added to each index
	u32 BaseInstance;  // Base instance ID
};

class IndirectDrawBuffer
{
public:
	IndirectDrawBuffer();
	~IndirectDrawBuffer();
	void SetData(DrawElementsIndirectCommand data);
	void UpdateData(DrawElementsIndirectCommand data);
	void SetSSBOBinding(i32 bindingIndex);
	void Clear(size_t offset, size_t size);
private:
	u32 _id = 0;
};
}