#include "ShaderCore.h"

using namespace platform::Render;
using namespace platform::Render::Shader;

void ShaderParameterMap::AddParameterAllocation(const std::string& ParameterName, uint16 BufferIndex, uint16 BaseIndex, uint16 Size, ShaderParamClass ParameterType)
{
	ParameterAllocation Allocation;
	Allocation.BufferIndex = BufferIndex;
	Allocation.BaseIndex = BaseIndex;
	Allocation.Size = Size;
	Allocation.Class = ParameterType;
	ParameterMap.emplace(ParameterName, Allocation);
}

bool ShaderParameterMap::FindParameterAllocation(const std::string& ParameterName, uint16& OutBufferIndex, uint16& OutBaseIndex, uint16& OutSize) const
{
	auto itr = ParameterMap.find(ParameterName);
	if (itr != ParameterMap.end())
	{
		OutBufferIndex = itr->second.BufferIndex;
		OutBaseIndex = itr->second.BaseIndex;
		OutSize = itr->second.Size;

		return true;
	}

	return false;
}
