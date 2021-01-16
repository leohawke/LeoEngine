#pragma once

#include "ICommandList.h"

namespace platform::Render {
	struct PixelShaderUtils
	{
		/** Initialize a pipeline state object initializer with almost all the basics required to do a full viewport pass. */
		static void InitFullscreenPipelineState(
			CommandList& RHICmdList,
			const ShaderRef<RenderShader>& PixelShader,
			GraphicsPipelineStateInitializer& GraphicsPSOInit);

		/** Draw a single triangle on the entire viewport. */
		static void DrawFullscreenTriangle(CommandList& RHICmdList, uint32 InstanceCount = 1);
	};

}