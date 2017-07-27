#include "PipleState.h"
#include "Convert.h"
#include "FrameBuffer.h"
#include "InputLayout.hpp"
#include "ShaderCompose.h"

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

			ID3D12PipelineState* pso;
			iter = psos.emplace(hash_val, COMPtr<ID3D12PipelineState>(pso)).first;
		}
		return iter->second;
	}
}