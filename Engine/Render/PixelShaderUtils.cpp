#include "PixelShaderUtils.h"
#include "CommonRenderResources.h"

using namespace platform::Render;

void PixelShaderUtils::InitFullscreenPipelineState(CommandList& CmdList, const RenderShader& PixelShader, GraphicsPipelineStateInitializer& GraphicsPSOInit)
{
	CmdList.FillRenderTargetsInfo(GraphicsPSOInit);

	GraphicsPSOInit.BlendState = {};
	GraphicsPSOInit.RasterizerState.cull = Render::CullMode::None;
	GraphicsPSOInit.DepthStencilState.depth_enable = false;
	GraphicsPSOInit.DepthStencilState.depth_func = Render::CompareOp::Pass;
	GraphicsPSOInit.Primitive = Render::PrimtivteType::TriangleList;

	GraphicsPSOInit.ShaderPass.VertexDeclaration = GFilterVertexDeclaration();
	GraphicsPSOInit.ShaderPass.PixelShader = PixelShader.GetPixelShader();
}

void PixelShaderUtils::DrawFullscreenTriangle(CommandList& RHICmdList, uint32 InstanceCount)
{
	RHICmdList.SetVertexBuffer(0, GScreenRectangleVertexBuffer().get());

	RHICmdList.DrawIndexedPrimitive(
		GScreenRectangleIndexBuffer().get(),
		/*BaseVertexIndex=*/ 0,
		/*MinIndex=*/ 0,
		/*NumVertices=*/ 3,
		/*StartIndex=*/ 6,
		/*NumPrimitives=*/ 1,
		/*NumInstances=*/ InstanceCount);
}
