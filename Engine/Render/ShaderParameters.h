#pragma once


#include "ITexture.hpp"
#include "IGraphicsBuffer.hpp"
#include "IGPUResourceView.h"
#include "TextureSampleDesc.h"

namespace platform::Render
{
	struct RayTracingShaderBindings
	{
		Texture* Textures[32] = {};
		ShaderResourceView* SRVs[32] = {};
		GraphicsBuffer* UniformBuffers[32] = {};
		TextureSampleDesc* Samplers[32] = {};
		UnorderedAccessView* UAVs[8] = {};
	};

	struct RayTracingShaderBindingsWriter : RayTracingShaderBindings
	{
		std::shared_ptr<GraphicsBuffer> RootUniformBuffer;

		void SetTexture(uint16 BaseIndex, Texture* Value)
		{
			Textures[BaseIndex] = Value;
		}

		void SetShaderSampler(uint16 BaseIndex, TextureSampleDesc* Value)
		{
			Samplers[BaseIndex] = Value;
		}

		void SetUAVParameter(uint16 BaseIndex, UnorderedAccessView* Value)
		{
			UAVs[BaseIndex] = Value;
		}

		void SetGraphicsBuffer(uint16 BaseIndex, GraphicsBuffer* Value)
		{
			UniformBuffers[BaseIndex] = Value;
		}
	};
}