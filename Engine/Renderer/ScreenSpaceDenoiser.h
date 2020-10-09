#pragma once

#include <Engine/Render/ICommandList.h>


namespace platform
{
	class ScreenSpaceDenoiser
	{
	public:
		struct ShadowVisibilityInput
		{
			Render::Texture2D* Mask;
		};

		struct ShadowVisibilityOutput
		{
			Render::Texture2D* Mask;
			Render::UnorderedAccessView* MaskUAV;
		};

		struct ShadowViewInfo
		{

		};

		static void DenoiseShadowVisibilityMasks(
			Render::CommandList& CmdList,
			const ShadowViewInfo& ViewInfo,
			const ShadowVisibilityInput& InputParameters,
			const ShadowVisibilityOutput& Output
		);
	};
}