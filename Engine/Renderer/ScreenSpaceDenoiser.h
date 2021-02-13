#pragma once

#include <Engine/Render/ICommandList.h>
#include "Engine/Render/ShaderParamterTraits.hpp"
#include "Engine/Render/ShaderParameterStruct.h"

namespace platform
{
	class ComputeShaderUtils
	{
	public:
		template <class T>
		static T DivideAndRoundUp(T Dividend, T Divisor)
		{
			return (Dividend + Divisor - 1) / Divisor;
		}

		static leo::math::int3 GetGroupCount(const leo::math::int2& ThreadCount, int GroupSize)
		{
			return { DivideAndRoundUp((int)ThreadCount.x ,GroupSize) ,DivideAndRoundUp((int)ThreadCount.y, GroupSize),1 };
		}

		template<typename TShaderClass>
		static void Dispatch(Render::CommandList& CmdList, const Render::ShaderRef<TShaderClass>& ComputeShader, const typename TShaderClass::Parameters& Parameters, leo::math::int3 GroupCount)
		{
			auto ShaderRHI = ComputeShader->GetComputeShader();
			CmdList.SetComputeShader(ShaderRHI);
			Render::SetShaderParameters(CmdList, ComputeShader, ShaderRHI, Parameters);
			CmdList.DispatchComputeShader(GroupCount.x, GroupCount.y, GroupCount.z);
		}
	};

	class ScreenSpaceDenoiser
	{
	public:
		struct ShadowVisibilityInput
		{
			Render::Texture2D* Mask;
			Render::Texture* SceneDepth;
			Render::Texture2D* WorldNormal;

			float LightHalfRadians;
		};

		struct ShadowVisibilityOutput
		{
			Render::Texture2D* Mask;
			Render::UnorderedAccessView* MaskUAV;
		};

		struct ShadowViewInfo
		{
			int StateFrameIndex;
			leo::math::float4x4 ScreenToTranslatedWorld;
			leo::math::float4x4 ViewToClip;
			leo::math::float4x4 TranslatedWorldToView;
			leo::math::float4x4 InvProjectionMatrix;
			leo::math::float4 InvDeviceZToWorldZTransform;
		};

		static void DenoiseShadowVisibilityMasks(
			Render::CommandList& CmdList,
			const ShadowViewInfo& ViewInfo,
			const ShadowVisibilityInput& InputParameters,
			const ShadowVisibilityOutput& Output
		);
	};
}