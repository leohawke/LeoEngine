#include "ShadowRendering.h"
#include "Render/BuiltInShader.h"
#include "Render/ShaderParamterTraits.hpp"
#include "Render/ShaderParameterStruct.h"
#include "Engine/Render/ShaderTextureTraits.hpp"
#include "Render/DrawEvent.h"
#include "Render/ITexture.hpp"
#include "Render/IContext.h"
#include "Render/PipelineStateUtility.h"
#include "Renderer/ScreenRendering.h"
#include "Math/TranslationMatrix.h"
#include "Math/RotationMatrix.h"
#include "Math/ScaleMatrix.h"
#include "Math/ShadowProjectionMatrix.h"
#include "Math/TranslationMatrix.h"
#include <LBase/smart_ptr.hpp>

using namespace LeoEngine;
using namespace platform;

class ShadowDepthVS : public Render::BuiltInShader
{
public:
	BEGIN_SHADER_PARAMETER_STRUCT(Parameters)
		SHADER_PARAMETER(lm::float4x4, ViewMatrix)
		SHADER_PARAMETER(lm::float4x4, ProjectionMatrix)
		SHADER_PARAMETER(lm::float4, ShadowParams)
		END_SHADER_PARAMETER_STRUCT();

	EXPORTED_BUILTIN_SHADER(ShadowDepthVS);
};


class ShadowDepthPS : public Render::BuiltInShader
{
	EXPORTED_BUILTIN_SHADER(ShadowDepthPS);
};

IMPLEMENT_BUILTIN_SHADER(ShadowDepthVS, "ShadowDepthVertexShader.lsl", "Main", platform::Render::VertexShader);
IMPLEMENT_BUILTIN_SHADER(ShadowDepthPS, "ShadowDepthPixelShader.lsl", "Main", platform::Render::PixelShader);

constexpr float CSMShadowDepthBias = 10;
constexpr float CSMShadowSlopeScaleDepthBias = 3.0f;

void ProjectedShadowInfo::SetupWholeSceneProjection(const SceneInfo& scne, const WholeSceneProjectedShadowInitializer& initializer, uint32 InResolutionX, uint32 InResoultionY, uint32 InBorderSize)
{
	PreShadowTranslation = initializer.ShadowTranslation;

	ResolutionX = InResolutionX;
	ResolutionY = InResoultionY;
	BorderSize = InBorderSize;
	CascadeSettings = initializer.CascadeSettings;

	const auto WorldToLightScaled = initializer.WorldToLight * ScaleMatrix(initializer.Scales);

	MaxSubjectZ = lm::transformpoint(initializer.SubjectBounds.Origin, WorldToLightScaled).z + initializer.SubjectBounds.Radius;

	MinSubjectZ = (MaxSubjectZ - initializer.SubjectBounds.Radius * 2);

	SubjectAndReceiverMatrix = WorldToLightScaled * ShadowProjectionMatrix(MinSubjectZ, MaxSubjectZ, initializer.WAxis);

	auto FarZPoint = lm::transform(lm::float4(0, 0, 1, 0), lm::inverse(WorldToLightScaled)) * initializer.SubjectBounds.Radius;
	float MaxSubjectDepth = lm::transformpoint(
		initializer.SubjectBounds.Origin.yzx+FarZPoint.xyz
		, SubjectAndReceiverMatrix
	).z;

	InvMaxSubjectDepth = 1.0f / MaxSubjectDepth;

	ShadowBounds = Sphere(-initializer.ShadowTranslation, initializer.SubjectBounds.Radius);

	ShadowViewMatrix = initializer.WorldToLight;

	//ShadowDepthBias
	float DepthBias = 0;
	float SlopeSclaedDepthBias = 1;

	DepthBias = CSMShadowDepthBias / (MaxSubjectZ - MinSubjectZ);
	const float WorldSpaceTexelSize = ShadowBounds.W / ResolutionX;

	DepthBias = lm::lerp(DepthBias, DepthBias * WorldSpaceTexelSize, CascadeSettings.ShadowCascadeBiasDistribution);
	DepthBias *= 0.5f;

	SlopeSclaedDepthBias = CSMShadowSlopeScaleDepthBias;
	SlopeSclaedDepthBias *= 0.5f;

	ShaderDepthBias = std::max(DepthBias, 0.f);
	ShaderSlopeDepthBias = std::max(DepthBias * SlopeSclaedDepthBias, 0.0f);
	ShaderMaxSlopeDepthBias = 1.f;
}


lr::GraphicsPipelineStateInitializer ProjectedShadowInfo::SetupShadowDepthPass(lr::CommandList& CmdList,lr::Texture2D* Target)
{
	DepthTarget = Target;

	lr::GraphicsPipelineStateInitializer psoInit;

	CmdList.FillRenderTargetsInfo(psoInit);

	CmdList.SetViewport(
		X + BorderSize, 
		Y + BorderSize,
		0, 
		X + BorderSize +ResolutionX,
		Y + BorderSize +ResolutionY,
		1);

	auto VertexShader = Render::GetBuiltInShaderMap()->GetShader<ShadowDepthVS>();
	auto PixelShader = Render::GetBuiltInShaderMap()->GetShader<ShadowDepthPS>();

	ShadowDepthVS::Parameters Parameters;

	Parameters.ProjectionMatrix =lm::transpose(TranslationMatrix(PreShadowTranslation) * SubjectAndReceiverMatrix);
	Parameters.ViewMatrix = lm::transpose(ShadowViewMatrix);
	Parameters.ShadowParams = lm::float4(
		ShaderDepthBias, ShaderSlopeDepthBias, ShaderMaxSlopeDepthBias,InvMaxSubjectDepth
	);

	auto ShadowDepthPassUniformBuffer = Render::CreateGraphicsBuffeImmediate(Parameters, Render::Buffer::Usage::SingleFrame);
	CmdList.SetShaderConstantBuffer(VertexShader->GetVertexShader(), 0, ShadowDepthPassUniformBuffer.Get());

	psoInit.ShaderPass.VertexShader = VertexShader->GetVertexShader();
	psoInit.ShaderPass.PixelShader = PixelShader->GetPixelShader();

	// Disable color writes
	psoInit.BlendState.color_write_mask[0] = 0;

	return psoInit;;
}

lm::vector2<int> ProjectedShadowInfo::GetShadowBufferResolution() const
{
	return { DepthTarget->GetWidth(0),DepthTarget->GetHeight(0) };
}

lm::float4x4 ProjectedShadowInfo::GetScreenToShadowMatrix(const SceneInfo& scene, uint32 TileOffsetX, uint32 TileOffsetY, uint32 TileResolutionX, uint32 TileResolutionY) const
{
	auto ShadowBufferRes = GetShadowBufferResolution();

	const auto InvResX = 1.0f / ShadowBufferRes.x;
	const auto InvResY = 1.0f / ShadowBufferRes.y;

	//pixel coord -> texel coord
	//[-1,1] -> [0,1]
	const auto ResFractionX = 0.5f * InvResX * TileResolutionX;
	const auto ResFractionY = 0.5f * InvResY * TileResolutionY;

	lm::float4x4 ViewDependeTransform =
		//Z of the position being transformed is actually view space Z
		lm::float4x4(
			lm::float4(1, 0, 0, 0),
			lm::float4(0, 1, 0, 0),
			lm::float4(0, 0, scene.Matrices.GetProjectionMatrix()[2][2], 1),
			lm::float4(0, 0, scene.Matrices.GetProjectionMatrix()[3][2], 0)
		) *
		scene.Matrices.GetInvViewProjectionMatrix();

	lm::float4x4 ShadowMapDependentTransform =
		// Translate to the origin of the shadow's translated world space
		TranslationMatrix(PreShadowTranslation)
		* SubjectAndReceiverMatrix*
		// Scale and translate x and y to be texture coordinates into the ShadowInfo's rectangle in the shadow depth buffer
		lm::float4x4(
			lm::float4(ResFractionX, 0, 0, 0),
			lm::float4(0, -ResFractionY, 0, 0),
			lm::float4(0, 0,1, 0),
			lm::float4(
				(TileOffsetX + BorderSize) * InvResX + ResFractionX,
				(TileOffsetY + BorderSize) * InvResY + ResFractionY,
				0, 1)
		)
		;

	return ViewDependeTransform * ShadowMapDependentTransform;
}

class ShadowProjectionVS : public Render::BuiltInShader
{
	EXPORTED_BUILTIN_SHADER(ShadowProjectionVS);
};


class ShadowProjectionPS : public Render::BuiltInShader
{
public:
	BEGIN_SHADER_PARAMETER_STRUCT(Parameters)
		SHADER_PARAMETER(lm::float4x4, ScreenToShadow)
		SHADER_PARAMETER(lm::float4, ProjectionDepthBiasParameters)
		SHADER_PARAMETER(float, FadePlaneOffset)
		SHADER_PARAMETER(float, InvFadePlaneLength)
		SHADER_PARAMETER_TEXTURE(lr::Texture2D, ShadowDepthTexture)
		SHADER_PARAMETER_SAMPLER(lr::TextureSampleDesc, ShadowDepthTextureSampler)

		SHADER_PARAMETER(lm::float4, InvDeviceZToWorldZTransform)

		SHADER_PARAMETER_STRUCT_INCLUDE(SceneParameters, SceneParameters)
		END_SHADER_PARAMETER_STRUCT();
	EXPORTED_BUILTIN_SHADER(ShadowProjectionPS);

	class FFadePlane : SHADER_PERMUTATION_BOOL("USE_FADE_PLANE");

	using FPermutationDomain = Render::TShaderPermutationDomain<FFadePlane>;

	void Set(lr::CommandList& CmdList, const SceneInfo& scene, const ProjectedShadowInfo* ShadowInfo)
	{
		Parameters Parameters;

		Parameters.ScreenToShadow = lm::transpose(ShadowInfo->GetScreenToShadowMatrix(scene));

		Parameters.ShadowDepthTexture = ShadowInfo->DepthTarget;

		Parameters.ShadowDepthTextureSampler.address_mode_u = Parameters.ShadowDepthTextureSampler.address_mode_v = lr::TexAddressingMode::Clamp;
		Parameters.ProjectionDepthBiasParameters = lm::float4(
			ShadowInfo->GetShaderDepthBias(), ShadowInfo->GetShaderSlopeDepthBias(), ShadowInfo->GetShaderReceiverDepthBias(), ShadowInfo->MaxSubjectZ - ShadowInfo->MinSubjectZ
		);

		Parameters.FadePlaneOffset = ShadowInfo->CascadeSettings.FadePlaneOffset;
		if(ShadowInfo->CascadeSettings.FadePlaneLength > 0)
			Parameters.InvFadePlaneLength = 1.0f /ShadowInfo->CascadeSettings.FadePlaneLength;

		Parameters.SceneParameters = scene.GetParameters();
		Parameters.InvDeviceZToWorldZTransform = CreateInvDeviceZToWorldZTransform(scene.Matrices.GetProjectionMatrix());

		lr::SetShaderParameters(CmdList,lr::ShaderRef<ShadowProjectionPS>(this), this->GetPixelShader(), Parameters);
	}
};

IMPLEMENT_BUILTIN_SHADER(ShadowProjectionVS, "ShadowProjectionVertexShader.lsl", "Main", platform::Render::VertexShader);
IMPLEMENT_BUILTIN_SHADER(ShadowProjectionPS, "ShadowProjectionPixelShader.lsl", "Main", platform::Render::PixelShader);

void ProjectedShadowInfo::RenderProjection(lr::CommandList& CmdList, const SceneInfo& scene)
{
	SCOPED_GPU_EVENTF(CmdList, "WholeScene split%d",CascadeSettings.ShadowSplitIndex);

	lr::GraphicsPipelineStateInitializer psoInit;

	CmdList.FillRenderTargetsInfo(psoInit);

	psoInit.DepthStencilState.depth_enable = false;
	psoInit.DepthStencilState.depth_func = lr::CompareOp::Pass;

	psoInit.BlendState.blend_enable[0] = true;
	psoInit.BlendState.src_blend[0] = lr::BlendFactor::Src_Alpha;
	psoInit.BlendState.dst_blend[0] = lr::BlendFactor::Inv_Src_Alpha;

	psoInit.Primitive = lr::PrimtivteType::TriangleStrip;
	psoInit.ShaderPass.VertexDeclaration =lr::VertexDeclarationElements { 
		lr::CtorVertexElement(0,0,lr::Vertex::Usage::Position,0,lr::EF_ABGR32F,sizeof(lm::float4))
	};

	//BindShadowProjectionShaders
	ShadowProjectionPS::FPermutationDomain PermutationVector;
	PermutationVector.Set < ShadowProjectionPS::FFadePlane>(CascadeSettings.FadePlaneLength > 0);

	auto PixelShader = lr::GetBuiltInShaderMap()->GetShader< ShadowProjectionPS>(PermutationVector);
	auto VertexShader = lr::GetBuiltInShaderMap()->GetShader< ShadowProjectionVS>();
	psoInit.ShaderPass.VertexShader = VertexShader->GetVertexShader();
	psoInit.ShaderPass.PixelShader = PixelShader->GetPixelShader();

	lr::SetGraphicsPipelineState(CmdList, psoInit);

	PixelShader->Set(CmdList, scene, this);

	CmdList.SetVertexBuffer(0, GFullScreenVertexBuffer().get());
	CmdList.DrawPrimitive(0, 0, 2, 1);
}
