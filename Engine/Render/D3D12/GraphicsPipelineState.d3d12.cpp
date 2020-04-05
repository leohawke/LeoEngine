#include "GraphicsPipelineState.h"
#include "ShaderPass.h"
#include "RootSignature.h"
using namespace platform_ex::Windows::D3D12;

KeyGraphicsPipelineStateDesc GetKeyGraphicsPipelineStateDesc(
	platform::Render::GraphicsPipelineStateInitializer& initializer, RootSignature* RootSignature);

GraphicsPipelineState::GraphicsPipelineState(const platform::Render::GraphicsPipelineStateInitializer& initializer)
{
	//retrive RootSignature
	auto graphicspass = static_cast<GraphicsShaderPass*>(initializer.ShaderState);

	


}

void platform_ex::Windows::D3D12::GraphicsPipelineState::Create(const GraphicsPipelineStateCreateArgs& InCreationArgs)
{
}

KeyGraphicsPipelineStateDesc GetKeyGraphicsPipelineStateDesc(
	platform::Render::GraphicsPipelineStateInitializer& initializer, RootSignature* RootSignature)
{

}
