#pragma once


#include "ITexture.hpp"
#include "IGraphicsBuffer.hpp"
#include "IGPUResourceView.h"
#include "TextureSampleDesc.h"

namespace platform::Render
{
	struct RayTracingShaderBindings
	{
		platform::Render::Texture* Textures[32] = {};
		platform::Render::ShaderResourceView* SRVs[32] = {};
		platform::Render::GraphicsBuffer* UniformBuffers[32] = {};
		platform::Render::TextureSampleDesc* Samplers[32] = {};
		platform::Render::UnorderedAccessView* UAVs[8] = {};
	};
}