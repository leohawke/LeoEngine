#include "PipleState.h"
#include "Convert.h"
#include "FrameBuffer.h"
#include "InputLayout.hpp"
#include "ShaderCompose.h"
#include "Context.h"

namespace platform_ex::Windows::D3D12 {
	PipleState::PipleState(const base & state)
		:base(state)
	{
		graphics_ps_desc.BlendState = Convert(base::BlendState);
		graphics_ps_desc.RasterizerState = Convert(base::RasterizerState);
		graphics_ps_desc.DepthStencilState = Convert(base::DepthStencilState);
	}
	COMPtr<ID3D12PipelineState> PipleState::RetrieveGraphicsPSO(const platform::Render::InputLayout & _layout, ShaderCompose & shader_compose, const std::shared_ptr<platform::Render::FrameBuffer>& _frame, bool HasTessellation) const
	{
		auto& layout = static_cast<const InputLayout&>(_layout);
		auto& frame = std::static_pointer_cast<FrameBuffer>(_frame);

		size_t hash_val = 0;
		hash_combine(hash_val, shader_compose.sc_template);
		for (auto & input_elem_desc : layout.GetInputDesc()) {
			auto p = reinterpret_cast<char const*>(&input_elem_desc);
			hash_val = hash(hash_val, p, p + sizeof(input_elem_desc));
		}
		hash_combine(hash_val, layout.GetIndexFormat());
		hash_combine(hash_val, layout.GetTopoType());

		for (auto i = FrameBuffer::Target0; i != FrameBuffer::DepthStencil; i =static_cast<FrameBuffer::Attachment>(i+1)) {
			if (auto view = frame->Attached(i))
				hash_combine(hash_val, view->Format());
		}
		if (auto view = frame->Attached(FrameBuffer::DepthStencil))
			hash_combine(hash_val, view->Format());
		
		auto iter = psos.find(hash_val);
		if (iter == psos.end()) {
			auto pso_desc = graphics_ps_desc;

			pso_desc.pRootSignature = shader_compose.RootSignature();
			pso_desc.VS << shader_compose.sc_template->VertexShader;
			pso_desc.PS << shader_compose.sc_template->PixelShader;

			//TODO StreamOutput
			pso_desc.StreamOutput.pSODeclaration = nullptr;
			pso_desc.StreamOutput.NumEntries = 0;
			pso_desc.StreamOutput.pBufferStrides = nullptr;
			pso_desc.StreamOutput.NumStrides = 0;
			pso_desc.StreamOutput.RasterizedStream = shader_compose.sc_template->rasterized_stream;

			auto& input_desc = layout.GetInputDesc();
			pso_desc.InputLayout.pInputElementDescs = input_desc.empty()?nullptr: leo::addressof(input_desc[0]);
			pso_desc.InputLayout.NumElements = static_cast<UINT>(input_desc.size());
			pso_desc.IBStripCutValue = (EF_R16UI == layout.GetIndexFormat()) ?
				D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF : D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF;

			auto tt = layout.GetTopoType();
			//TODO tessellation support
			pso_desc.PrimitiveTopologyType = Convert<D3D12_PRIMITIVE_TOPOLOGY_TYPE>(tt);


			for (auto i = std::size(pso_desc.RTVFormats) - 1; i >= 0; --i) {
				if (frame->Attached((FrameBuffer::Attachment)(FrameBuffer::Target0 + i))) {
					pso_desc.NumRenderTargets =static_cast<UINT>(i + 1);
					break;
				}
			}
			for (auto i = 0; i != pso_desc.NumRenderTargets; ++i) {
				pso_desc.RTVFormats[i] = Convert(frame->Attached((FrameBuffer::Attachment)(FrameBuffer::Target0+i))->Format());
			}
			for (auto i = pso_desc.NumRenderTargets; i != std::size(pso_desc.RTVFormats); ++i)
				pso_desc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
			if (auto view = frame->Attached(FrameBuffer::DepthStencil))
				pso_desc.DSVFormat = Convert(view->Format());
			else
				pso_desc.DSVFormat = DXGI_FORMAT_UNKNOWN;
			pso_desc.SampleDesc.Count = 1;
			pso_desc.SampleDesc.Quality = 0;

			ID3D12PipelineState* pso;
			CheckHResult(Context().GetDevice()->CreateGraphicsPipelineState(&pso_desc, IID_ID3D12PipelineState, reinterpret_cast<void**>(&pso)));

			iter = psos.emplace(hash_val, COMPtr<ID3D12PipelineState>(pso)).first;
		}
		return iter->second;
	}
}