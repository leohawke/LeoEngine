#include "ShadowRendering.h"
#include "Render/BuiltInShader.h"
#include "Render/ShaderParamterTraits.hpp"
#include "Render/ShaderParameterStruct.h"
#include "Math/TranslationMatrix.h"

using namespace LeoEngine;
using namespace platform;

class ShadowDepthVS : public Render::BuiltInShader
{
public:
	BEGIN_SHADER_PARAMETER_STRUCT(Parameters)
		SHADER_PARAMETER(lm::float4x4, ProjectionMatrix)
		END_SHADER_PARAMETER_STRUCT();

	EXPORTED_BUILTIN_SHADER(ShadowDepthVS);
};


class ShadowDepthPS : public Render::BuiltInShader
{
	EXPORTED_BUILTIN_SHADER(ShadowDepthPS);
};

IMPLEMENT_BUILTIN_SHADER(ShadowDepthVS, "ShadowDepthVertexShader.lsl", "Main", platform::Render::VertexShader);
IMPLEMENT_BUILTIN_SHADER(ShadowDepthPS, "ShadowDepthPixelShader.lsl", "Main", platform::Render::PixelShader);



lr::GraphicsPipelineStateInitializer LeoEngine::SetupShadowDepthPass(const ProjectedShadowInfo& ShadowInfo, lr::CommandList& CmdList)
{
	lr::GraphicsPipelineStateInitializer psoInit;

	CmdList.BeginRenderPass(*ShadowInfo.PassInfo, "ShadowDepth");

	CmdList.FillRenderTargetsInfo(psoInit);

	CmdList.SetViewport(ShadowInfo.X, ShadowInfo.Y, 0,ShadowInfo.ResolutionX, ShadowInfo.ResolutionY,1);

	auto VertexShader = Render::GetBuiltInShaderMap()->GetShader<ShadowDepthVS>();
	auto PixelShader = Render::GetBuiltInShaderMap()->GetShader<ShadowDepthPS>();

	ShadowDepthVS::Parameters Parameters;

	Parameters.ProjectionMatrix =lm::transpose(TranslationMatrix(ShadowInfo.PreShadowTranslation) * ShadowInfo.SubjectAndReceiverMatrix);

	auto ShadowDepthPassUniformBuffer = Render::CreateGraphicsBuffeImmediate(Parameters, Render::Buffer::Usage::SingleFrame);
	CmdList.SetShaderConstantBuffer(VertexShader->GetVertexShader(), 0, ShadowDepthPassUniformBuffer.Get());

	psoInit.ShaderPass.VertexShader = VertexShader->GetVertexShader();
	psoInit.ShaderPass.PixelShader = PixelShader->GetPixelShader();

	// Disable color writes
	psoInit.BlendState.color_write_mask[0] = 0;

	return psoInit;;
}