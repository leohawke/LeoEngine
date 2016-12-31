#ifndef LE_RENDER_D3D12_PipeState_h
#define LE_RENDER_D3D12_PipeState_h 1

#include "../Effect/Effect.hpp"
#include "d3d12_dxgi.h"

namespace platform_ex::Windows::D3D12 {
	class PipleState : public platform::Render::PipleState {
	public:
		D3D12_BLEND_DESC BlendState;
		D3D12_RASTERIZER_DESC RasterizerState;
		D3D12_DEPTH_STENCIL_DESC DepthStencilState;
	};
}

#endif