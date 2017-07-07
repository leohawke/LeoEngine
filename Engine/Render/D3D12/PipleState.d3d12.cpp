#include "PipleState.h"
#include "Convert.h"
#include "FrameBuffer.h"
#include "InputLayout.hpp"

namespace platform_ex::Windows::D3D12 {
	PipleState::PipleState(const base & state)
		:base(state)
	{
		union_desc.graphics_ps_desc.BlendState = Convert(base::BlendState);
		union_desc.graphics_ps_desc.RasterizerState = Convert(base::RasterizerState);
		union_desc.graphics_ps_desc.DepthStencilState = Convert(base::DepthStencilState);
	}
	COMPtr<ID3D12PipelineState> PipleState::RetrieveGraphicsPSO(const platform::Render::InputLayout & _layout, ShaderCompose & shader_compose, const std::shared_ptr<platform::Render::FrameBuffer>& _frame, bool HasTessellation)
	{
		auto& layout = static_cast<const InputLayout&>(_layout);
		auto& frame = std::static_pointer_cast<FrameBuffer>(_frame);
		return COMPtr<ID3D12PipelineState>();
	}
}