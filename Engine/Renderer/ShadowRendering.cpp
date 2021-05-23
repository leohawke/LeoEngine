#include "ShadowRendering.h"
#include "Render/BuiltInShader.h"
#include "Render/ShaderParamterTraits.hpp"
#include "Render/ShaderParameterStruct.h"
#include "Math/TranslationMatrix.h"
#include "Math/RotationMatrix.h"
#include "Math/ScaleMatrix.h"
#include "Math/ShadowProjectionMatrix.h"
#include "Math/TranslationMatrix.h"

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


lr::GraphicsPipelineStateInitializer ProjectedShadowInfo::SetupShadowDepthPass(lr::CommandList& CmdList)
{
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