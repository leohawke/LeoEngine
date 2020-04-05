#pragma once

#include "../IGraphicsPipelineState.h"
#include "../IDevice.h"
#include "d3d12_dxgi.h"
#include <LFramework/Win32/LCLib/COM.h>

namespace platform_ex::Windows::D3D12
{
	class RootSignature;

	struct D3DGraphicsPipelineStateDesc
	{
		ID3D12RootSignature* pRootSignature;
		D3D12_SHADER_BYTECODE VS;
		D3D12_SHADER_BYTECODE PS;
		D3D12_SHADER_BYTECODE DS;
		D3D12_SHADER_BYTECODE HS;
		D3D12_SHADER_BYTECODE GS;

		D3D12_BLEND_DESC BlendState;
		uint32 SampleMask;
		D3D12_RASTERIZER_DESC RasterizerState;
		D3D12_DEPTH_STENCIL_DESC1 DepthStencilState;

		D3D12_INPUT_LAYOUT_DESC InputLayout;
		D3D12_INDEX_BUFFER_STRIP_CUT_VALUE IBStripCutValue;
		D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTopologyType;
		D3D12_RT_FORMAT_ARRAY RTFormatArray;
		DXGI_FORMAT DSVFormat;
		DXGI_SAMPLE_DESC SampleDesc;
		uint32 NodeMask;
		D3D12_CACHED_PIPELINE_STATE CachedPSO;
		D3D12_PIPELINE_STATE_FLAGS Flags;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC GraphicsDesc() const;
	};

	struct KeyGraphicsPipelineStateDesc
	{
		const RootSignature* pRootSignature;
		D3DGraphicsPipelineStateDesc Desc;
	};

	class GraphicsPipelineStateCreateArgs
	{
		const KeyGraphicsPipelineStateDesc* Desc;
		ID3D12PipelineLibrary* Library;
	};

	class GraphicsPipelineState : public platform::Render::GraphicsPipelineState
	{
	public:
		explicit GraphicsPipelineState(const platform::Render::GraphicsPipelineStateInitializer& initializer);

	private:
		void Create(const GraphicsPipelineStateCreateArgs& InCreationArgs);
	private:
		COMPtr<ID3D12PipelineState> PipelineState;
	};
}