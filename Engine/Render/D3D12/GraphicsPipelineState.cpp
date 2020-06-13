#include "GraphicsPipelineState.h"

D3D12_GRAPHICS_PIPELINE_STATE_DESC platform_ex::Windows::D3D12::D3DGraphicsPipelineStateDesc::GraphicsDesc() const
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC D;
	D.Flags = this->Flags;
	D.NodeMask = this->NodeMask;
	D.pRootSignature = this->pRootSignature;
	D.InputLayout = this->InputLayout;
	D.IBStripCutValue = this->IBStripCutValue;
	D.PrimitiveTopologyType = this->PrimitiveTopologyType;
	D.VS = this->VS;
	D.GS = this->GS;
	D.HS = this->HS;
	D.DS = this->DS;
	D.PS = this->PS;
	D.BlendState = this->BlendState;
	D.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC1(this->DepthStencilState);
	D.DSVFormat = this->DSVFormat;
	D.RasterizerState = this->RasterizerState;
	D.NumRenderTargets = this->RTFormatArray.NumRenderTargets;
	std::memcpy(D.RTVFormats, this->RTFormatArray.RTFormats, sizeof(D.RTVFormats));
	std::memset(&D.StreamOutput, 0, sizeof(D.StreamOutput));
	D.SampleDesc = this->SampleDesc;
	D.SampleMask = this->SampleMask;
	D.CachedPSO = this->CachedPSO;
	return D;
}
