#ifndef LE_RENDER_D3D12_PipeState_h
#define LE_RENDER_D3D12_PipeState_h 1

#include "../Effect/Effect.hpp"
#include "../IFrameBuffer.h"
#include "d3d12_dxgi.h"

namespace platform_ex::Windows::D3D12 {
	class ShaderCompose;
	class PipleState : public platform::Render::PipleState {
	public:
		using base = platform::Render::PipleState;
		PipleState(const base&);

		COMPtr<ID3D12PipelineState> RetrieveGraphicsPSO(const platform::Render::InputLayout & layout, ShaderCompose& shader_compose, const std::shared_ptr<platform::Render::FrameBuffer>&, bool HasTessellation);

	private:
		union PipelineStateUnion {
			D3D12_GRAPHICS_PIPELINE_STATE_DESC graphics_ps_desc;
			D3D12_COMPUTE_PIPELINE_STATE_DESC compute_ps_desc;
		} union_desc;

		mutable std::unordered_map<size_t, COMPtr<ID3D12PipelineState>> psos;
	};
}

#endif