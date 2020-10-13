#include "ScreenSpaceDenoiser.h"
#include "Engine/Render/BuiltInShader.h"
#include "Engine/Render/ShaderParamterTraits.hpp"
#include "Engine/Render/ShaderParameterStruct.h"
#include "Engine/Render/DrawEvent.h"

using namespace platform;

namespace Shadow
{
	using namespace platform::Render;

	class SSDInjectCS : public BuiltInShader
	{
	public:
		BEGIN_SHADER_PARAMETER_STRUCT(Parameters)
			SHADER_PARAMETER(leo::math::float4, ThreadIdToBufferUV)
			SHADER_PARAMETER(leo::math::float4, BufferBilinearUVMinMax)
			SHADER_PARAMETER(leo::math::float2, BufferUVToOutputPixelPosition)
			SHADER_PARAMETER(float, HitDistanceToWorldBluringRadius)
			END_SHADER_PARAMETER_STRUCT();

		EXPORTED_BUILTIN_SHADER(SSDInjectCS);
	};

	IMPLEMENT_BUILTIN_SHADER(SSDInjectCS, "SSD/Shadow/SSDInject.lsl", "MainCS", platform::Render::ComputeShader);

	constexpr auto TILE_SIZE = 8;
}

void platform::ScreenSpaceDenoiser::DenoiseShadowVisibilityMasks(Render::CommandList& CmdList, const ShadowViewInfo& ViewInfo, const ShadowVisibilityInput& InputParameters, const ShadowVisibilityOutput& Output)
{
	auto FullResW = InputParameters.Mask->GetWidth(0);
	auto FullResH = InputParameters.Mask->GetHeight(0);

	//Injest
	{
		SCOPED_GPU_EVENT(CmdList, ShadowInjest);

		auto InjestShader = Render::GetGlobalShaderMap()->GetShader<Shadow::SSDInjectCS>();

		Shadow::SSDInjectCS::Parameters Parameters;
		Parameters.ThreadIdToBufferUV.x = 1.0f / FullResW;
		Parameters.ThreadIdToBufferUV.y = 1.0f / FullResH;
		Parameters.ThreadIdToBufferUV.z = 0.5f / FullResW;
		Parameters.ThreadIdToBufferUV.w = 0.5f / FullResH;

		Parameters.BufferBilinearUVMinMax = leo::math::float4(
			0.5f / FullResW,0.5f/ FullResH,
			(FullResW-0.5f)/ FullResW, (FullResH - 0.5f) / FullResH
		);

		Parameters.BufferUVToOutputPixelPosition = leo::math::float2(FullResW, FullResH);
		Parameters.HitDistanceToWorldBluringRadius = 1;

		ComputeShaderUtils::Dispatch(CmdList, InjestShader, Parameters,
			ComputeShaderUtils::GetGroupCount(leo::math::int2(FullResW,FullResH),Shadow::TILE_SIZE));

	}
}
