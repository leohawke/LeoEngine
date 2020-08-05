#pragma once

#include "ICommandList.h"

namespace platform::Render {
	void SetGraphicsPipelineState(CommandList& cmdlist, const GraphicsPipelineStateInitializer& initializer);


	namespace PipelineStateCache
	{
		class FGraphicsPipelineState;

		FGraphicsPipelineState* GetAndOrCreateGraphicsPipelineState(CommandList& cmdlist, const GraphicsPipelineStateInitializer& initializer);
	}
}