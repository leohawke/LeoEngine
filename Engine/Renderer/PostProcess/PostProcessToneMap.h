#pragma once

#include "Engine/Render/ITexture.hpp"
#include "Engine/Render/RenderPassInfo.h"

namespace platform
{
	struct TonemapInputs
	{
		// [Required] Render to the specified output. 
		// TODO: If invalid, a new texture is created and returned.
		Render::RenderTarget OverrideOutput;

		// [Required] HDR scene color to tonemap.
		Render::TexturePtr SceneColor;

		// [Required] Color grading texture used to remap colors.
		Render::TexturePtr ColorGradingTexture;
	};

	void TonemapPass(const TonemapInputs& Inputs);
}
