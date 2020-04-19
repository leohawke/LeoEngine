#include "ShaderCore.h"

using namespace platform::Render;
using namespace platform::Render::Shader;

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
