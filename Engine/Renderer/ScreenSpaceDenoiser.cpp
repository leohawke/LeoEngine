#include "ScreenSpaceDenoiser.h"
#include "Engine/Render/BuiltInShader.h"
#include "Engine/Render/ShaderParamterTraits.hpp"
#include "Engine/Render/ShaderParameterStruct.h"
#include "Engine/Render/ShaderTextureTraits.hpp"
#include "Engine/Render/DrawEvent.h"
#include "Engine/Render/ShaderPermutation.h"
#include "Engine/Render/IContext.h"
#include "Engine/Render/RenderObject.h"

using namespace platform;

constexpr auto TILE_SIZE = 8;

/** Different signals to denoise. */
enum class SignalProcessing
{
	// Denoise a shadow mask.
	ShadowVisibilityMask,

	MAX
};

/** Returns whether a signal processing support upscaling. */
static bool SignalSupportsUpscaling(SignalProcessing SignalProcessing)
{
	return false;
}

static bool SignalUsesPreConvolution(SignalProcessing SignalProcessing)
{
	return
		SignalProcessing == SignalProcessing::ShadowVisibilityMask;
}

/** Returns whether a signal can denoise multi sample per pixel. */
static bool SignalSupportMultiSPP(SignalProcessing SignalProcessing)
{
	return (
		SignalProcessing == SignalProcessing::ShadowVisibilityMask
		);
}

/** Returns whether a signal have a code path for 1 sample per pixel. */
static bool SignalSupport1SPP(SignalProcessing SignalProcessing)
{
	return (
		false);
}

// Permutation dimension for the type of signal being denoised.
class FSignalProcessingDim : SHADER_PERMUTATION_ENUM_CLASS("DIM_SIGNAL_PROCESSING", SignalProcessing);

// Permutation dimension for the number of signal being denoised at the same time.
class FSignalBatchSizeDim : SHADER_PERMUTATION_RANGE_INT("DIM_SIGNAL_BATCH_SIZE", 1, 4);

// Permutation dimension for denoising multiple sample at same time.
class FMultiSPPDim : SHADER_PERMUTATION_BOOL("DIM_MULTI_SPP");

class SSDSpatialAccumulationCS : public Render::BuiltInShader
{
public:
	enum class Stage
	{
		// Spatial kernel used to process raw input for the temporal accumulation.
		ReConstruction,

		MAX
	};

	class FStageDim : SHADER_PERMUTATION_ENUM_CLASS("DIM_STAGE", Stage);
	class FUpscaleDim : SHADER_PERMUTATION_BOOL("DIM_UPSCALE");

	using FPermutationDomain = Render::TShaderPermutationDomain<FSignalProcessingDim, FStageDim, FUpscaleDim, FSignalBatchSizeDim, FMultiSPPDim>;

	static bool ShouldCompilePermutation(const Render::FBuiltInShaderPermutationParameters& Parameters)
	{
		FPermutationDomain PermutationVector(Parameters.PermutationId);
		SignalProcessing SignalProcessing = PermutationVector.Get<FSignalProcessingDim>();

		// Only reconstruction have upscale capability for now.
		if (PermutationVector.Get<FUpscaleDim>() &&
			PermutationVector.Get<FStageDim>() != Stage::ReConstruction)
		{
			return false;
		}

		// Only upscale is only for signal that needs it.
		if (PermutationVector.Get<FUpscaleDim>() &&
			!SignalSupportsUpscaling(SignalProcessing))
		{
			return false;
		}

		//// Only compile pre convolution for signal that uses it.
		//if (!SignalUsesPreConvolution(SignalProcessing) &&
		//	PermutationVector.Get<FStageDim>() == Stage::PreConvolution)
		//{
		//	return false;
		//}

		// Only compile multi SPP permutation for signal that supports it.
		if (PermutationVector.Get<FStageDim>() == Stage::ReConstruction &&
			PermutationVector.Get<FMultiSPPDim>() && !SignalSupportMultiSPP(SignalProcessing))
		{
			return false;
		}

		// Compile out the shader if this permutation gets remapped.
		if (RemapPermutationVector(PermutationVector) != PermutationVector)
		{
			return false;
		}

		return true;
	}

	static FPermutationDomain RemapPermutationVector(FPermutationDomain PermutationVector)
	{
		SignalProcessing SignalProcessing = PermutationVector.Get<FSignalProcessingDim>();

		if (PermutationVector.Get<FStageDim>() == Stage::ReConstruction)
		{
			// force use the multi sample per pixel code path.
			if (!SignalSupport1SPP(SignalProcessing))
			{
				PermutationVector.Set<FMultiSPPDim>(true);
			}
		}
		else
		{
			PermutationVector.Set<FMultiSPPDim>(true);
		}

		return PermutationVector;
	}


	BEGIN_SHADER_PARAMETER_STRUCT(Parameters)
		//TODO:support ViewParams
		SHADER_PARAMETER(leo::math::float2, ViewportMin)
		SHADER_PARAMETER(leo::math::float2, ViewportMax)
		SHADER_PARAMETER(leo::math::float4, ThreadIdToBufferUV)
		SHADER_PARAMETER(leo::math::float4, BufferBilinearUVMinMax)
		SHADER_PARAMETER(leo::math::float4, BufferSizeAndInvSize)
		SHADER_PARAMETER(leo::math::float4, BufferUVToScreenPosition)
		SHADER_PARAMETER(leo::math::float2, BufferUVToOutputPixelPosition)
		SHADER_PARAMETER(leo::uint32, StateFrameIndexMod8)
		SHADER_PARAMETER(leo::math::float4x4, ScreenToTranslatedWorld)
		SHADER_PARAMETER(leo::math::float4x4, ViewToClip)
		SHADER_PARAMETER(leo::math::float4x4, TranslatedWorldToView)
		SHADER_PARAMETER(leo::math::float4, InvDeviceZToWorldZTransform)
		//TODO:SceneParamets
		SHADER_PARAMETER_TEXTURE(Render::Texture2D, SceneDepthBuffer)
		SHADER_PARAMETER_SAMPLER(Render::TextureSampleDesc, SceneDepthBufferSampler)
		SHADER_PARAMETER_TEXTURE(Render::Texture2D, WorldNormalBuffer)
		SHADER_PARAMETER_SAMPLER(Render::TextureSampleDesc, WorldNormalSampler)

		SHADER_PARAMETER_SAMPLER(Render::TextureSampleDesc, GlobalPointClampedSampler)

		SHADER_PARAMETER(float, WorldDepthToPixelWorldRadius)
		//SignalFramework.h
		SHADER_PARAMETER(float, HitDistanceToWorldBluringRadius)

		SHADER_PARAMETER_TEXTURE(Render::Texture2D, SignalInput_Textures_0)
		SHADER_PARAMETER_TEXTURE(Render::Shader::RWTexture2D, SignalOutput_UAVs_0)
		
		END_SHADER_PARAMETER_STRUCT();

	EXPORTED_BUILTIN_SHADER(SSDSpatialAccumulationCS);
};

IMPLEMENT_BUILTIN_SHADER(SSDSpatialAccumulationCS, "SSD/Shadow/SSDSpatialAccumulation.lsl", "MainCS", platform::Render::ComputeShader);


namespace Shadow
{
	using namespace platform::Render;

	class SSDInjestCS : public BuiltInShader
	{
	public:
		BEGIN_SHADER_PARAMETER_STRUCT(Parameters)
			SHADER_PARAMETER(leo::math::float4, ThreadIdToBufferUV)
			SHADER_PARAMETER(leo::math::float4, BufferBilinearUVMinMax)
			SHADER_PARAMETER(leo::math::float2, BufferUVToOutputPixelPosition)
			SHADER_PARAMETER(leo::math::float4, InputBufferUVMinMax)
			SHADER_PARAMETER(float, HitDistanceToWorldBluringRadius)
			SHADER_PARAMETER_TEXTURE(Render::Texture2D, SignalInput_Textures_0)
			SHADER_PARAMETER_TEXTURE(Render::Shader::RWTexture2D, SignalOutput_UAVs_0)
			SHADER_PARAMETER_SAMPLER(Render::TextureSampleDesc, point_sampler)
			END_SHADER_PARAMETER_STRUCT();

		EXPORTED_BUILTIN_SHADER(SSDInjestCS);
	};

	IMPLEMENT_BUILTIN_SHADER(SSDInjestCS, "SSD/Shadow/SSDInjest.lsl", "MainCS", platform::Render::ComputeShader);

}

void platform::ScreenSpaceDenoiser::DenoiseShadowVisibilityMasks(Render::CommandList& CmdList, const ShadowViewInfo& ViewInfo, const ShadowVisibilityInput& InputParameters, const ShadowVisibilityOutput& Output)
{
	auto& Device = Render::Context::Instance().GetDevice();
	auto FullResW = InputParameters.Mask->GetWidth(0);
	auto FullResH = InputParameters.Mask->GetHeight(0);

	//Injest
	{
		SCOPED_GPU_EVENT(CmdList, ShadowInjest);

		auto InjestShader = Render::GetBuiltInShaderMap()->GetShader<Shadow::SSDInjestCS>();

		Shadow::SSDInjestCS::Parameters Parameters;
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

		Parameters.SignalInput_Textures_0 = InputParameters.Mask;
		Parameters.SignalOutput_UAVs_0 = Output.MaskUAV;

		Render::TextureSampleDesc point_sampler{};
		point_sampler.address_mode_u = point_sampler.address_mode_v = point_sampler.address_mode_w = Render::TexAddressingMode::Clamp;
		point_sampler.filtering = Render::TexFilterOp::Min_Mag_Mip_Point;

		Parameters.point_sampler = point_sampler;

		ComputeShaderUtils::Dispatch(CmdList, InjestShader, Parameters,
			ComputeShaderUtils::GetGroupCount(leo::math::int2(FullResW,FullResH),TILE_SIZE));
	}

	auto spatial_reconst =Render::shared_raw_robject(Device.CreateTexture(FullResW, FullResH,
		1, 1, Render::EF_ABGR16F, Render::EA_GPURead | Render::EA_GPUUnordered | Render::EA_GPUWrite, {}));
	// Spatial reconstruction with ratio estimator to be more precise in the history rejection.
	{
		SCOPED_GPU_EVENT(CmdList, SSDSpatialAccumulation_Reconstruction);

		//CreateUAV

		SSDSpatialAccumulationCS::Parameters Parameters;

		//ViewParams
		Parameters.ViewportMin = leo::math::float2(0, 0);
		Parameters.ViewportMax = leo::math::float2(FullResW, FullResH);

		Parameters.ThreadIdToBufferUV.x = 1.0f / FullResW;
		Parameters.ThreadIdToBufferUV.y = 1.0f / FullResH;
		Parameters.ThreadIdToBufferUV.z = 0.5f / FullResW;
		Parameters.ThreadIdToBufferUV.w = 0.5f / FullResH;

		Parameters.BufferBilinearUVMinMax = leo::math::float4(
			0.5f / FullResW, 0.5f / FullResH,
			(FullResW - 0.5f) / FullResW, (FullResH - 0.5f) / FullResH
		);

		Parameters.BufferUVToOutputPixelPosition = leo::math::float2(FullResW, FullResH);

		Parameters.BufferUVToScreenPosition.x = 2;
		Parameters.BufferUVToScreenPosition.y = -2;
		Parameters.BufferUVToScreenPosition.z = -1.0f;
		Parameters.BufferUVToScreenPosition.w = 1.0f;

		Parameters.StateFrameIndexMod8 = ViewInfo.StateFrameIndex % 8;
		Parameters.ScreenToTranslatedWorld = ViewInfo.ScreenToTranslatedWorld;
		Parameters.ViewToClip = ViewInfo.ViewToClip;
		Parameters.TranslatedWorldToView = ViewInfo.TranslatedWorldToView;
		Parameters.InvDeviceZToWorldZTransform = ViewInfo.InvDeviceZToWorldZTransform;

		//SceneParams
		Parameters.SceneDepthBuffer = static_cast<Render::Texture2D*>(InputParameters.SceneDepth);
		Parameters.WorldNormalBuffer = InputParameters.WorldNormal;

		Render::TextureSampleDesc point_sampler{};
		point_sampler.address_mode_u = point_sampler.address_mode_v = point_sampler.address_mode_w = Render::TexAddressingMode::Clamp;
		point_sampler.filtering = Render::TexFilterOp::Min_Mag_Mip_Point;

		Parameters.GlobalPointClampedSampler = point_sampler;
		Parameters.SceneDepthBufferSampler = point_sampler;
		Parameters.WorldNormalSampler = point_sampler;

		Parameters.WorldDepthToPixelWorldRadius = 1;
		Parameters.HitDistanceToWorldBluringRadius = 1;

		static auto uav = Device.CreateUnorderedAccessView(spatial_reconst.get());
		Parameters.SignalInput_Textures_0 = InputParameters.Mask;
		Parameters.SignalOutput_UAVs_0 = uav;


		SSDSpatialAccumulationCS::FPermutationDomain PermutationVector;
		PermutationVector.Set<FSignalProcessingDim>(SignalProcessing::ShadowVisibilityMask);
		PermutationVector.Set<FSignalBatchSizeDim>(1);
		PermutationVector.Set<SSDSpatialAccumulationCS::FStageDim>(SSDSpatialAccumulationCS::Stage::ReConstruction);
		PermutationVector.Set<SSDSpatialAccumulationCS::FUpscaleDim>(false);
		PermutationVector.Set<FMultiSPPDim>(true);

		Render::ShaderMapRef<SSDSpatialAccumulationCS> ComputeShader(Render::GetBuiltInShaderMap(), SSDSpatialAccumulationCS::RemapPermutationVector(PermutationVector));

		ComputeShaderUtils::Dispatch(CmdList, ComputeShader, Parameters,
			ComputeShaderUtils::GetGroupCount(leo::math::int2(FullResW, FullResH), TILE_SIZE));
	}
}
